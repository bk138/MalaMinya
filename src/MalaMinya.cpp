/* $Id: MalaMinya.cpp,v 1.14 2006/10/13 06:50:41 whot Exp $ */
/*--
 * Copyright (C) 2006 Peter Hutterer <peter@cs.unisa.edu.au>
 *
 * This file is part of MalaMinya.
 *
 * MalaMinya is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MalaMinya is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MalaMinya; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

  --*/

#include <string>

#include "MalaMinya.h"
#include "logger.h"
#include "Error.h"
#include "Util.h"


/**
 * Create MalaMinya and the top-level window.
 */
MalaMinya::MalaMinya(char* display)
{
    x11 = new XConn(display);

    width = WIDTH;
    height = HEIGHT;

    pen_width = 5;
    eraser_width = 15;

    XSetWindowAttributes attr;
    attr.background_pixel = x11->white;
    attr.event_mask = ExposureMask | StructureNotifyMask; 

    win = XCreateWindow(x11->dpy, 
            x11->root,
            0, 0,
            width, height,
            0, x11->depth,
            InputOutput,
            x11->vis,
            CWBackPixel | CWEventMask,
            &attr);

    if (!win)
        throw Error("Cannot create top-level window!");

    XSetWMProtocols(x11->dpy, win, &x11->wm_delete_window, 1);
 
    XTextProperty title_prop;
    const char *title[] = {"MalaMinya"};
    XStringListToTextProperty(const_cast<char**>(title), 1, &title_prop);
    XSetWMName(x11->dpy, win, &title_prop);

    XSizeHints* win_size_hints = XAllocSizeHints();
    if (!win_size_hints) 
      throw Error("XAllocSizeHints - out of memory\n");
       
    win_size_hints->flags = PMaxSize | PMinSize | PResizeInc | PAspect;
    win_size_hints->min_width = 200;
    win_size_hints->min_height = 200;
    win_size_hints->max_width = width;
    win_size_hints->max_height = height;
    win_size_hints->min_aspect.x = 1;
    win_size_hints->min_aspect.y = 1;
    win_size_hints->max_aspect.x = 1;
    win_size_hints->max_aspect.y = 1;
    win_size_hints->width_inc = 20;
    win_size_hints->height_inc = 20;

    XSetWMNormalHints(x11->dpy, win, win_size_hints);

    XFree(win_size_hints);

    XMapWindow(x11->dpy, win);
}

MalaMinya::~MalaMinya()
{
    XUnmapWindow(x11->dpy, win);
    XDestroyWindow(x11->dpy, win);

    XDeviceInfo* devices;
    int num;

    devices = XListInputDevices(x11->dpy, &num);


    std::map<int, Pointer*>::const_iterator it = pointers.begin();
    while (it != pointers.end())
    {
        XCloseDevice(x11->dpy, it->second->dev);
        delete it->second;
    }

    XCloseDisplay(x11->dpy);
}

/**
 * Initializes MalaMinya GUIs. 
 */
void MalaMinya::init()
{
    /* to make programming easer we create toolbars after initialising the
       devices. otherwise we have to go out of our way to get the tool buttons
       to listen to the right event classes */
    initGUI();
    initColorButtons();
    initDevices();
    initToolbars();
    registerEvents();

    XFlush(x11->dpy);
    XSync(x11->dpy, False);
}

/**
 * Initialize the GUI. One window for the toolbars, one window + GC for the
 * canvas.
 */
void MalaMinya::initGUI()
{
    XSetWindowAttributes attr;
    attr.background_pixel = x11->black;
    menuswin = XCreateWindow(x11->dpy, 
            win,
            0, 0,
            width, height,
            0, x11->depth,
            InputOutput,
            x11->vis,
            CWBackPixel,
            &attr);

    attr.background_pixel = x11->white;
    attr.event_mask = ExposureMask | PointerMotionMask; 
    canvaswin = XCreateWindow(x11->dpy, 
            menuswin,
            MENUHEIGHT, MENUHEIGHT,
            width - MENUHEIGHT * 2, height - MENUHEIGHT * 2,
            0, x11->depth,
            InputOutput,
            x11->vis,
            CWBackPixel | CWEventMask,
            &attr);

    XMapWindow(x11->dpy, canvaswin);
    XMapWindow(x11->dpy, menuswin);

    XGCValues gcvalues;
    gcvalues.foreground = x11->black;
    gcvalues.background = x11->white;

    canvas = XCreateGC(x11->dpy, canvaswin, GCForeground | GCBackground,
            &gcvalues);


    gcvalues.foreground = x11->white;
    backbuffer = XCreatePixmap(x11->dpy, canvaswin, width, height, x11->depth);
    buffer = XCreateGC(x11->dpy, backbuffer, GCForeground | GCBackground,
            &gcvalues);


    XFillRectangle(x11->dpy, backbuffer, buffer, 0, 0, width, height);

    XFlush(x11->dpy);

}

/**
 * Create buttons with randomized colors. Color buttons will be sprinkled
 * around the main window.
 */
void MalaMinya::initColorButtons()
{
    cbuttons.clear();

    ColorButton* cbutton;

    int no_buttons = 2 * WIDTH / BT_SIZE + 2 * HEIGHT / BT_SIZE;
    int x, y;
    x = y = 0;

    for (int i = 0; i < no_buttons; i++)
    {

        int r, g, b;
        r = rand() % 256;
        g = rand() % 256;
        b = rand() % 256;
        cbutton = new ColorButton(x11, menuswin, r, g, b);
        cbuttons.push_back(cbutton);
    }

}

/**
 * Initialize all the XINPUT devices. MalaMinya only has room for 8 toolbars,
 * so we take the first 8 XINPUT devices.
 */
void MalaMinya::initDevices()
{
    /* init input devices */
    XDeviceInfo* devices;
    int num, ignore, num_used;

    if(! XQueryExtension (x11->dpy, "XInputExtension", &ignore, &ignore, &ignore))
      throw Error("No XInputExtension!");

    // activate XI2
    XQueryInputVersion(x11->dpy, XI_2_Major, XI_2_Minor);

    devices = XListInputDevices(x11->dpy, &num);

    if (!num)
        throw Error("No XINPUT devices found!");

    pointers.clear();

    num_used = 0;

    while(num > 0)
    {
        XDevice* dev;
        XEventClass evclasses[3];

        num--;
        TRACE("found device: %d - %s \n", (int)devices[num].id,
                devices[num].name); 

        if (devices[num].use == IsXPointer)
        {
          if(num_used >= NO_USERS)
             break;
        
	  TRACE("   adding device %d ...\n", (int)devices[num].id); 

	  dev = XOpenDevice(x11->dpy, devices[num].id);
	  
	  DeviceMotionNotify(dev, Pointer::xi_motion, evclasses[0]);
	  DeviceButtonPress(dev, Pointer::xi_press, evclasses[1]);
	  DeviceButtonRelease(dev, Pointer::xi_release, evclasses[2]);
	  
	  XSelectExtensionEvent(x11->dpy, canvaswin, evclasses, 3);
	  Pointer* p = createPointer(num_used, evclasses);
	  p->dev = dev;
	  pointers[p->id] = p;
	  p->setColor(cbuttons.at(0)->getColor());
	  ++num_used;
        }
    }

    XFreeDeviceList(devices);
}

/**
 * Initialize a toolbar for each of the registered devices.
 */
void MalaMinya::initToolbars()
{
    toolbars.clear();

    /* create toolbar for device */
    Toolbar* tb; 

    map<int, Pointer*>::const_iterator it = pointers.begin();
    while(it != pointers.end())
    {
        Pointer* p = it->second;
        tb = createToolbar(getPointerImage(p->id));
        tb->id = p->id;

        switch(p->id) /* There are only 8 devices */
        {
            case 2:
            case 3:
            case 6:
            case 7:
                tb->setVertical(true);
                break;
            case 0:
            case 1:
            case 4:
            case 5:
                tb->setVertical(false);
                break;
        }

        toolbars.push_back(tb);
        it++;
    }

}

void MalaMinya::registerEvents()
{
    /* 
       we run through all pointers and toolbars and register all pointer's
       event classes on all toolbars. This way everyone can use anybody's
       toolbar and we can limit it later again with MPX floor control.
       This doesn't sound like the smart thing to do but allows to demo MPX
       floor control which is a bit more flexible than messing around with
       X event classes.
     */
    map<int, Pointer*>::const_iterator itp; /* iterator over pointers */
    vector<Toolbar*>::const_iterator ittb; /* iterator over allToolbars */
    itp = pointers.begin();

    while(itp != pointers.end())
    {
        Pointer* pt = itp->second;

        ittb = toolbars.begin();
        while(ittb != toolbars.end())
        {
            Toolbar* t = *ittb;
            t->registerForEvents(pt->getEventClass(XI_PRESS));
            ittb++;
        }
        itp++;
    }

    /*
      now set up floor control for the toolbars. We restrict each toolbar to
      one pointing device for now.
     */
    itp = pointers.begin();
    ittb = toolbars.begin();
    while(itp != pointers.end())
    {
        Toolbar* t = *ittb;
        Pointer* pt = itp->second;

        t->restrictTo(pt->id);
        itp++;
        ittb++;
    }

    /* now run through the color buttons and register them for all devices */

    itp = pointers.begin();
    vector<ColorButton*>::const_iterator itcb;

    while(itp != pointers.end())
    {
        Pointer* pt = itp->second;

        itcb = cbuttons.begin();
        while (itcb != cbuttons.end())
        {
            (*itcb)->registerEvent(pt->getEventClass(XI_PRESS));
            itcb++;
        }

        itp++;
    }
}

void MalaMinya::run()
{

    bool running = true;
    while(running)
    {
        XEvent e;
        XNextEvent(x11->dpy, &e);
        switch(e.type)
        {
            case Expose:
                repaintToolbars();
                repaintCanvas();
                break;
            case ClientMessage:
                if (e.xclient.message_type == x11->wm_protocols &&
                        (Atom) e.xclient.data.l[0] == x11->wm_delete_window)
                    running = false;
                break;
            case ConfigureNotify:
                handleConfigure(&e.xconfigure);
                break;
            default:
                if (e.type == Pointer::xi_motion)
                {
                    XDeviceMotionEvent* mev;
                    mev = (XDeviceMotionEvent*)&e;
                    handleMotionEvent(mev);
                } if (e.type == Pointer::xi_press || e.type == Pointer::xi_release)
                {
                    handleButtonEvent((XDeviceButtonEvent*)&e);
                }
        }
    }



}

/**
 * Handles configure events and resizes the window. 
 * Currently forces square window and window size that is a multitude of 20.
 */
void MalaMinya::handleConfigure(XConfigureEvent* ev)
{
    int width = (WIDTH > ev->width) ? ev->width : WIDTH;
    width -= width % 20;
    height = width; 
    XResizeWindow(x11->dpy, menuswin, width, height);

    pen_width = width/100;
    eraser_width = pen_width * 3;

    int btwidth = (int) (width / (double)(cbuttons.size() / 4.0));
    int btheight = btwidth;

    XMoveResizeWindow(x11->dpy, canvaswin, btwidth, btheight, 
                        width - btheight * 2, 
                        height - btheight * 2);

    int x = 0;
    int y = 0;

    vector<ColorButton*>::const_iterator it = cbuttons.begin();
    while(it != cbuttons.end())
    {
        ColorButton* cbutton = *it;
        cbutton->resize(btwidth, btheight);
        cbutton->move(x, y);

        if (y < (height - btwidth) && y > 0)
        {
            if (x == 0)
                x = width - btwidth; 
            else
            {
                x = 0;
                y += btwidth;
            }
        } else
        {
            x += btwidth;
        }

        if (x > width - btwidth)
        {
            y += btwidth;
            x = 0;
        }

        it++;
    }

    // toolbars
    vector<Toolbar*>::const_iterator it2 = toolbars.begin();

    while(it2 != toolbars.end())
    {
        Toolbar* tb = *it2;
        tb->setButtonSize(btwidth);

        switch(tb->id)
        {
            case 0:
                x = width - 4 * btwidth - btheight; 
                y = height - btheight;
                break;
            case 1:
                x = 4 * btwidth;
                y = height - btheight;
                break;
            case 2:
                x = 0;
                y = height - 4 * btwidth - btwidth;
                break;
            case 3:
                x = 0;
                y = 4 * btwidth;
                break;
            case 4:
                x = 4 * btwidth;
                y = 0;
                break;
            case 5:
                x = width - 4 * btwidth - btwidth;
                y = 0;
                break;
            case 6:
                x = width - btheight;
                y = 4 * btwidth;
                break;
            case 7:
                x = width - btheight;
                y = height - 4 * btwidth - btwidth;
                break;
        }
        tb->move(x, y);
        it2++;
    }

}

void MalaMinya::handleMotionEvent(XDeviceMotionEvent* mev)
{

    Pointer* p = findPointer(mev->deviceid);
    if (mev->state & (Button1Mask | Button2Mask | Button3Mask))
    {
        long mask = GCForeground | GCLineWidth;
        XGCValues vals;

	if(mev->state & Button1Mask)
	  {
	    vals.foreground = p->getColorPixel();
	    vals.line_width = pen_width;
	  }
	else
	  {
	    vals.foreground = x11->white;
	    vals.line_width = eraser_width;
	  }
        XChangeGC(x11->dpy, buffer, mask, &vals);
        XDrawLine(x11->dpy, backbuffer, buffer, p->x, p->y, mev->x, mev->y);
    }

    p->x = mev->x;
    p->y = mev->y;
    repaintCanvas();
    updatePointerIcons();
}

void MalaMinya::handleButtonEvent(XDeviceButtonEvent* bev)
{
    TRACE("button press event on window %x\n", (unsigned int)bev->window);
    Toolbar* tb = findToolbarFromWindow(bev->window);
    Pointer* p = findPointer(bev->deviceid);
    if (!tb)
    {
        ColorButton* cbt = findColorButton(bev->window);
        if (cbt)
        {
            TRACE("Selecting color\n");
            p->setColor(cbt->getColor());
        } else if (bev->type == Pointer::xi_press) /* start line */
        {
            p->x = bev->x;
            p->y = bev->y;
        }
    } else
    {
        // toolbar button release
        tb->handleClick(p, bev->window);
        TRACE("click on toolbar\n");
    }
}

/**
 * Clean the whole drawing area.
 */
void MalaMinya::wipe()
{
    XClearWindow(x11->dpy, canvaswin);
    long mask = GCForeground;
    XGCValues vals;
    vals.foreground = x11->white;
    XChangeGC(x11->dpy, buffer, mask, &vals);
    XFillRectangle(x11->dpy, backbuffer, buffer, 0, 0, width, height);
    XFlush(x11->dpy);
}


bool MalaMinya::save()
{
  unsigned int w, h;
  unsigned int ignore;
  Window ign_window;
 
  XGetGeometry(x11->dpy, canvaswin, &ign_window, (int*)&ignore, (int*)&ignore, &w, &h, &ignore, &ignore);

  XImage* ximage = XGetImage(x11->dpy, canvaswin, 0, 0, w, h, AllPlanes, ZPixmap);
  Magick::Image* image = Util::XImageToImage(ximage);
  if(!image)
    {
      XFree(ximage);
      return false;
    }

  // should be enough ... 
  char filename[138]; 
  char date[138]; 
  time_t t; time_t *tp = &t;  

  time(tp); 

  // convert to date-string  
  strftime(date, 138, "%d.%m.%Y-%H.%M.%S", localtime(tp)); 
  snprintf(filename, 138, "malaminya-save_%s.png", date); 
 
  if(! Util::ImageToFile(image, filename))
    {
      delete image;
      XFree(ximage);
      return false;
    }

  delete image;
  XFree(ximage);

  TRACE("Saved %s!\n", filename);
  return true;
}




Magick::Image* MalaMinya::getPointerImage(int id)
{
    char cId[2] = {'0' + id, cId[1] = '\0'};
    string file = IMAGEPATH "icon";
    file.append((const char*)cId);
    file.append(".png");
    Magick::Image* img = Util::ImageFromFile((char*)file.c_str()); 
    return img;
}



/**
 * Creates a icon for each pointer. Currently up to 8 different ones.
 */
XImage* MalaMinya::createPointerIcon(int id)
{
    Magick::Image* img = getPointerImage(id);
    img->scale(Magick::Geometry(16, 16, 0, 0, false, false));
    XImage* ximage = Util::ImageToXImage(x11->dpy, x11->screen, img);
    delete img;
    return ximage;
}

void MalaMinya::repaintCanvas()
{
    XCopyArea(x11->dpy, backbuffer, canvaswin, canvas, 0, 0, width, height, 0, 0);
    XFlush(x11->dpy);
}

/** 
 * Creates a new pointer object with a specific icon.
 */
Pointer* MalaMinya::createPointer(int id, XEventClass* evclasses)
{
    XImage* img = createPointerIcon(id);
    Pointer* p = new Pointer(id, evclasses, img);
    return p;
}

/**
 * Copies the pointer icon to the given pointer's position.
 */
void MalaMinya::updatePointerIcons()
{
    map<int, Pointer*>::iterator it = pointers.begin();

    while(it != pointers.end())
    {
        Pointer* p = it->second;
        XPutImage(x11->dpy, canvaswin, canvas, p->icon, 0, 0, p->x + 10, p->y + 10, 16, 16);
        it++;
    }
}


/* returns the pointer object for a given device */
Pointer* MalaMinya::findPointer(int id)
{
    Pointer* p;
    p = pointers[id];
    if (p == NULL)
        throw Error("cannot find pointer instance for device!");

    return p;
}


void MalaMinya::pointerListDestroy()
{
    map<int, Pointer*>::iterator it = pointers.begin();

    while(it != pointers.end())
    {
        delete it->second;
        it++;
    }

    pointers.clear();
}


void MalaMinya::tbListDestroy()
{
    toolbars.clear();
}

/*
 * Create a new toolbar object.
 * If vertical is set, the buttons will be aligned vertically.
 */
Toolbar* MalaMinya::createToolbar(Magick::Image* icon)
{
    Toolbar* tb = new Toolbar(this, x11, menuswin, icon);
    return tb;
}

/**
 * Returns the toolbar this window belongs to or NULL if no toolbar could be
 * found.
 */
Toolbar* MalaMinya::findToolbarFromWindow(Window win)
{
    vector<Toolbar*>::iterator it = toolbars.begin();
    while(it != toolbars.end())
    {
        Toolbar* tb = *it;
        if (tb->hasWindow(win))
            return tb;
        it++;
    }

    return NULL;
}

void MalaMinya::repaintToolbars()
{
    vector<Toolbar*>::iterator it = toolbars.begin();
    while(it != toolbars.end())
    {
        (*it)->repaint();
        it++;
    }
}

ColorButton* MalaMinya::findColorButton(Window win)
{
    vector<ColorButton*>::const_iterator it = cbuttons.begin();

    while(it != cbuttons.end())
    {
        if ((*it)->hasWindow(win))
            return *it;
        it++;
    }
    return NULL;
}

