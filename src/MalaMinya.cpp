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

    msg_shown = false;

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

    std::map<int, Pointer*>::const_iterator it = pointers.begin();
    while (it != pointers.end())
    {
      delete it->second;
      it++;
    }

    XCloseDisplay(x11->dpy);
}

/**
 * Initializes MalaMinya GUIs. 
 */
void MalaMinya::init()
{
    /* to make programming easier we create toolbars and backbuffers 
       after initialising the devices. */
    initGUI();
    initColorButtons();
    initDevices();
    initBackbuffers();
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

    canvasgc = XCreateGC(x11->dpy, canvaswin, GCForeground | GCBackground,
			 &gcvalues);

    canvasbackbuf = new Backbuffer(x11, canvaswin, width, height);


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
    XIDeviceInfo* devices;
    int num, ignore, num_used;
    int major = 2, minor = 0;

    // query XI and XI2
    if(! XQueryExtension (x11->dpy, "XInputExtension", &xi2opcode, &ignore, &ignore))
      throw Error("No XInput Extension!");

    if (XIQueryVersion(x11->dpy, &major, &minor) != Success)
      throw Error("XI2 not supported.\n");
    
    // clean up pointer map
    std::map<int, Pointer*>::const_iterator it = pointers.begin();
    while (it != pointers.end())
      {
	delete it->second;
	it++;
      }
    pointers.clear();

    // and query devices
    devices = XIQueryDevice(x11->dpy, XIAllMasterDevices, &num);

    num_used = 0;

    for(int i = 0; i < num; ++i)
    {
	TRACE("found master device: %d - %s \n", (int)devices[i].deviceid, devices[i].name); 

        if (devices[i].use == XIMasterPointer)
        {
          if(num_used >= NO_USERS)
             break;
        
	  TRACE(" -> adding device %d ...\n", (int)devices[i].deviceid); 

	  Pointer* p = new Pointer(devices[i].deviceid, num_used, x11);
	  pointers[p->getId()] = p;
	  p->setColor(cbuttons.at(0)->getColor());
	  ++num_used;
        }
    }
  
    XIFreeDeviceInfo(devices);

    // finally, register for device hierarchy changes
    XIEventMask mask;
    unsigned char bits[4] = {0};

    mask.mask = bits;
    mask.mask_len = sizeof(bits);
    mask.deviceid = XIAllDevices;
    XISetMask(bits, XI_HierarchyChanged);

    XISelectEvents(x11->dpy, x11->root, &mask, 1);
}

/**
 * Initialize a toolbar for each of the registered devices.
 */
void MalaMinya::initToolbars()
{
  vector<Toolbar*>::const_iterator it = toolbars.begin();
  while(it != toolbars.end())
    {
      delete *it;
      it++;
    }
    
  toolbars.clear();

    /* create toolbar for device */
    Toolbar* tb; 
    int i = 0;
    map<int, Pointer*>::const_iterator itp = pointers.begin();
    while(itp != pointers.end())
    {
        Pointer* p = itp->second;
        tb = new Toolbar(this, p->getId(), x11, menuswin, p->getImage());

	if(i%2)
	  tb->setVertical(true); 
	else
	  tb->setVertical(false); 

	toolbars.push_back(tb);
        itp++;
	++i;
    }

}


void MalaMinya::initBackbuffers()
{
  // clean up backbuffer map
  map<int, Backbuffer*>::const_iterator it = backbuffers.begin();
  while (it != backbuffers.end())
    {
      delete it->second;
      it++;
    }
  backbuffers.clear(); 


  // create backbuffer for each pointer 
  Backbuffer* bb;
  map<int, Pointer*>::const_iterator itp = pointers.begin();
  while(itp != pointers.end())
    {
      Pointer* p =  itp->second;

      bb = new Backbuffer(x11, canvaswin, width, height);
      backbuffers[p->getId()] = bb;
      
      itp++;
    }
}




void MalaMinya::registerEvents()
{
  XIEventMask mask;
  unsigned char bits[4] = {0};

  mask.mask = bits;
  mask.mask_len = sizeof(bits);
  // which ones?
  mask.deviceid = XIAllMasterDevices;
  // what?
  XISetMask(bits, XI_ButtonPress);
  XISetMask(bits, XI_ButtonRelease);
  XISetMask(bits, XI_Motion);

  XISelectEvents(x11->dpy, canvaswin, &mask, 1);


  /* 
     we run through all toolbars and register all master devices
  */
  vector<Toolbar*>::const_iterator ittb = toolbars.begin();
  
  while(ittb != toolbars.end())
    {
      Toolbar* t = *ittb;
      t->registerEvent(XI_ButtonPress);
      ittb++;
    }
  

  /*
    now set up floor control for the toolbars. We restrict each toolbar to
    one pointing device for now.
  */
  map<int, Pointer*>::iterator itp = pointers.begin();
  ittb = toolbars.begin();

  while(itp != pointers.end())
    {
      Toolbar* t = *ittb;
      Pointer* pt = itp->second;

      t->restrictTo(pt->getId());
      itp++;
      ittb++;
    }

  /* now run through the color buttons and register them for all devices */
  vector<ColorButton*>::const_iterator itcb = cbuttons.begin();;
  while(itcb != cbuttons.end())
    {
      (*itcb)->registerEvent(XI_ButtonPress);
      itcb++;
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
	  repaintCanvas(0, 0, width, height);
	  break;
	case ClientMessage:
	  if (e.xclient.message_type == x11->wm_protocols &&
	      (Atom) e.xclient.data.l[0] == x11->wm_delete_window)
	    running = false;
	  break;
	case ConfigureNotify:
	  handleConfigure(&e.xconfigure);
	  break;

	case GenericEvent:
	  XIEvent* xi_e = (XIEvent*)&e;
	  if(xi_e->extension == xi2opcode)
	    {
	      if (xi_e->evtype == XI_HierarchyChanged)
		handleHierarchyChangedEvent((XIHierarchyEvent*)xi_e);
	      
	      if (xi_e->evtype == XI_Motion)
		handleMotionEvent((XIDeviceEvent*)xi_e);
	      
	      if (xi_e->evtype == XI_ButtonPress || xi_e->evtype == XI_ButtonRelease)
		handleButtonEvent((XIDeviceEvent*)xi_e);
	    }

	  XIFreeEventData(xi_e);
	  break;
	}
    }
}


/**
 * Handles configure events and resizes the window. 
 * Currently forces square window and window size that is a multitude of 20.
 */
void MalaMinya::handleConfigure(XConfigureEvent* ev)
{
    width = (WIDTH > ev->width) ? ev->width : WIDTH;
    width -= width % 20;
    height = width; 
    XResizeWindow(x11->dpy, menuswin, width, height);

    // scale all pointer sizes
    map<int, Pointer*>::const_iterator itp = pointers.begin();
    while(itp != pointers.end())
      {       
	Pointer* p = itp->second;
	p->setSize(DFLT_POINTERSIZE * width/WIDTH);
	itp++;
      }
    

    int btwidth = (int) (width / (double)(cbuttons.size() / 4.0));
    int btheight = btwidth;

    XMoveResizeWindow(x11->dpy, canvaswin, btwidth, btheight, 
                        width - btheight * 2, 
                        height - btheight * 2);

    
    placeColorButtons();
    
    placeToolbars();
}

void MalaMinya::handleMotionEvent(XIDeviceEvent* mev)
{
  int id = mev->deviceid;

  map<int, Pointer*>::iterator it = pointers.find(id);
  if(it == pointers.end())
    return;
  Pointer* p = it->second;

  if (XIMaskIsSet(mev->buttons->mask, 1) ||
      XIMaskIsSet(mev->buttons->mask, 2) ||
      XIMaskIsSet(mev->buttons->mask, 3))
    {
      long mask = GCForeground | GCLineWidth;
      XGCValues vals;

      // Button 1
      if(XIMaskIsSet(mev->buttons->mask, 1))
	{
	  vals.foreground = p->getColorPixel();
	  vals.line_width = p->getSize();
	}
      else
	{
	  vals.foreground = x11->white;
	  vals.line_width = p->getSize() * 3;
	}
      // draw into the main backbuffer
      XChangeGC(x11->dpy, canvasbackbuf->gc, mask, &vals);
      XDrawLine(x11->dpy, canvasbackbuf->buf, canvasbackbuf->gc,
		p->x, p->y, mev->event_x, mev->event_y);

      // and also draw into our private one
      XChangeGC(x11->dpy, backbuffers[id]->gc, mask, &vals);
      XDrawLine(x11->dpy, backbuffers[id]->buf, backbuffers[id]->gc,
		p->x, p->y, mev->event_x, mev->event_y);

      // and: mark other private buffers with white
      vals.foreground = x11->white;
      Backbuffer* bb;
      map<int, Backbuffer*>::const_iterator it = backbuffers.begin();
      while (it != backbuffers.end())
	{
	  bb = it->second;
	  // mark the _others_
	  if(bb != backbuffers[id])
	    {
	      XChangeGC(x11->dpy, bb->gc, mask, &vals);
	      XDrawLine(x11->dpy, bb->buf, bb->gc,
			p->x, p->y, mev->event_x, mev->event_y);
	    }

	  it++;
	}
    }

  // if the pointer made a big jump (or we had a message), redraw the whole canvas,
  // otherwise just update the area around the cursor
  if(abs((int)mev->event_x - p->x) > 40 || abs((int)mev->event_y - p->y) > 40 || msg_shown)
    {
      repaintCanvas(0, 0, width, height);
      msg_shown = false;
    }
  else
    repaintCanvas(mev->event_x-75, mev->event_y-75, 150, 150);
    
  // store new position
  p->x = mev->event_x;
  p->y = mev->event_y;

  updatePointerIcons();
}

void MalaMinya::handleButtonEvent(XIDeviceEvent* bev)
{
    Toolbar* tb = findToolbarFromWindow(bev->event);

    map<int, Pointer*>::iterator it = pointers.find(bev->deviceid);
    if(it == pointers.end())
      return;
    Pointer* p = it->second;

    if (!tb)
    {
        ColorButton* cbt = findColorButton(bev->event);
        if (cbt)
        {
            TRACE("Selecting color\n");
            p->setColor(cbt->getColor());
        } else if (bev->evtype == XI_ButtonPress) /* start line */
        {
            p->x = bev->event_x;
            p->y = bev->event_y;
        }
    } else
    {
        // toolbar button release
        tb->handleClick(p, bev->event);
        TRACE("click on toolbar\n");
    }
}


void MalaMinya::handleHierarchyChangedEvent(XIHierarchyEvent* ev)
{
  TRACE("hierarchy changed\n");
  initDevices();
  initBackbuffers();
  initToolbars();
  registerEvents();

  placeToolbars();
  repaintToolbars();


  XFlush(x11->dpy);
}



/**
 * Clean the whole drawing area.
 */
void MalaMinya::wipe(int id)
{
  XClearWindow(x11->dpy, canvaswin);

  XGCValues vals;
  // change GC funtion to equiv !
  vals.function = GXequiv;
  XChangeGC(x11->dpy, backbuffers[id]->gc, GCFunction, &vals);
  
  XCopyArea(x11->dpy, backbuffers[id]->buf, canvasbackbuf->buf, backbuffers[id]->gc,
	    0, 0, width, height, 0, 0);

  // change back
  vals.function = GXcopy;
  vals.foreground = x11->white;
  XChangeGC(x11->dpy, backbuffers[id]->gc, GCFunction | GCForeground, &vals);

  // clear private backbuffer
  XFillRectangle(x11->dpy, backbuffers[id]->buf, backbuffers[id]->gc, 0, 0, width, height);

  repaintCanvas(0,0, width, height);

  TRACE("WIPE for device id %d!\n", id);
}


bool MalaMinya::save(int id)
{
  unsigned int w, h;
  unsigned int ignore;
  Window ign_window;

  XGetGeometry(x11->dpy, canvasbackbuf->buf, &ign_window, (int*)&ignore, (int*)&ignore, 
	       &w, &h, &ignore, &ignore);

  XImage* ximage = XGetImage(x11->dpy, canvasbackbuf->buf, 0, 0, w, h, AllPlanes, ZPixmap);
  Magick::Image* image = Util::XImageToImage(ximage);
  if(!image)
    {
      XFree(ximage);
      return false;
    }
  
  // get the user number
  int usr=1;
  map<int, Pointer*>::iterator itp = pointers.begin();
  while(itp != pointers.end())
    {
      Pointer* p = itp->second;
      if(p->getId() == id)
	break;
      itp++;
      ++usr;
    }

  
  // should be enough ... 
  char filename[138]; 
  char date[138]; 
  time_t t; time_t *tp = &t;  
  
  time(tp); 
  
  // convert to date-string  
  strftime(date, 138, "%d.%m.%Y-%H.%M.%S", localtime(tp)); 
  snprintf(filename, 138, "malaminya-save_%s_by-user%i.png", date, usr); 
  
  if(! Util::ImageToFile(image, filename))
    {
      delete image;
      XFree(ximage);
      return false;
    }
  
  delete image;
  XFree(ximage);


  string msg = "Screenshot taken!";
  XDrawString(x11->dpy, canvaswin, canvasgc, width/2 - 80, height/2 - 40,
	      msg.c_str(), msg.length());
  msg_shown = true;
  
  TRACE("Saved %s!\n", filename);
  
  return true;
}


void MalaMinya::pensize(Pointer *p)
{
  int size = p->getSize();
  
  if(size >= width / 30)
    p->setSize(DFLT_POINTERSIZE);
  else
    p->setSize(size * 2);
}



void MalaMinya::repaintCanvas(int x, int y, int w, int h)
{
  XCopyArea(x11->dpy, canvasbackbuf->buf, canvaswin, canvasgc, x, y, w, h, x, y);
  XFlush(x11->dpy);
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
        XPutImage(x11->dpy, canvaswin, canvasgc, p->getIcon(),
		  0, 0, p->x + 20, p->y + 10, 16, 16);
        it++;
    }
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

void MalaMinya::placeToolbars()
{
  int x,y;

  int btwidth = (int) (width / (double)(cbuttons.size() / 4.0));
  int btheight = btwidth;


  int i = 0;
  vector<Toolbar*>::const_iterator it = toolbars.begin();
  while(it != toolbars.end())
    {
      Toolbar* tb = *it;

      switch(i)
        {
	case 0:
	  x = width - 4 * btwidth - btheight; 
	  y = height - btheight;
	  break;
	case 1:
	  x = 0;
	  y = height - 4 * btwidth - btwidth;
	  break;
	case 2:
	  x = 4 * btwidth;
	  y = height - btheight;
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
	  x = width - btheight;
	  y = 4 * btwidth;
	  break;
	case 6:
	  x = width - 4 * btwidth - btwidth;
	  y = 0;
	  break;
	case 7:
	  x = width - btheight;
	  y = height - 4 * btwidth - btwidth;
	  break;
        }

      tb->setButtonSize(btwidth);
      tb->move(x, y);

      it++;
      ++i;
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


void MalaMinya::placeColorButtons()
{
  int btwidth = (int) (width / (double)(cbuttons.size() / 4.0));
  int btheight = btwidth;

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


}
