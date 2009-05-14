/* $Id: Toolbar.cpp,v 1.11 2006/10/13 06:50:42 whot Exp $ */
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

#include "mpgxlib.h"
#include <X11/Xutil.h>

#include "logger.h"
#include "Toolbar.h"
#include "Error.h"
#include "MalaMinya.h"
#include "Pointer.h"
#include "Util.h"

Toolbar::Toolbar(MalaMinya* mm, XConn* x11, Window menuswin, Magick::Image* img_icon)
{
    this->parent = mm;
    this->x11 = x11;
    this->menuswin = menuswin;
    this->img_icon = img_icon;
    this->vertical = false;
    this->ximg_icon = NULL;

    btsize = BT_SIZE;
    rescale(img_icon, &ximg_icon, btsize);
    
    ximg_pen = ximg_eraser = ximg_wipe = NULL;

    XColor exact;
    XAllocNamedColor(x11->dpy, x11->cmap, "green", &color_pen,
            &exact);
    XAllocNamedColor(x11->dpy, x11->cmap, "red", &color_eraser,
            &exact);
    XAllocNamedColor(x11->dpy, x11->cmap, "blue", &color_wipe,
            &exact);


    XSetWindowAttributes attr;

    attr.background_pixel = x11->white;
    toolbar = XCreateWindow(x11->dpy, 
            menuswin,
            0, 0,
            BT_SIZE * 4,
            BT_SIZE,
            0, x11->depth,
            InputOutput,
            x11->vis,
            CWBackPixel,
            &attr);
    if (!toolbar)
        throw Error("Could not create window!");

    icon = XCreateWindow(x11->dpy,
            toolbar,
            0, 0,
            BT_SIZE, BT_SIZE,
            0, x11->depth,
            InputOutput,
            x11->vis,
            CWBackPixel,
            &attr);
    if (!icon)
        throw Error("Could not create window!");

    XGCValues vals; 
    long mask = 0;
    gc_icon = XCreateGC(x11->dpy, icon, mask, &vals);
    XPutImage(x11->dpy, icon, gc_icon, ximg_icon, 0, 0, BT_SIZE, BT_SIZE, 16,
            16);

    /* create buttons for toolbar */
    attr.background_pixel = color_pen.pixel;
    pen = XCreateWindow(x11->dpy,
            toolbar,
            BT_SIZE, 0,
            BT_SIZE, BT_SIZE,
            0, x11->depth,
            InputOutput,
            x11->vis,
            CWBackPixel,
            &attr);
    if (!toolbar)
        throw Error("Could not create window!");

    attr.background_pixel = color_eraser.pixel;
    eraser = XCreateWindow(x11->dpy,
            toolbar,
            BT_SIZE * 2, 0,
            BT_SIZE, BT_SIZE,
            0, x11->depth,
            InputOutput,
            x11->vis,
            CWBackPixel,
            &attr);
    if (!toolbar)
        throw Error("Could not create window!");

    attr.background_pixel = color_wipe.pixel;
    wipe = XCreateWindow(x11->dpy,
            toolbar,
            BT_SIZE * 3, 0,
            BT_SIZE, BT_SIZE,
            0, x11->depth,
            InputOutput,
            x11->vis,
            CWBackPixel,
            &attr);
    if (!toolbar)
        throw Error("Could not create window!");

    /* create the images */
    img_pen = Util::ImageFromFile(IMAGEPATH "pen.png");
    if (img_pen)
    {
        XImage* ximage = Util::ImageToXImage(x11->dpy, x11->screen, img_pen);
        if (!ximage)
        {
            ERR("Could not create ximage for pen.\n");
        } else
        {
            XGCValues vals; 
            long mask = 0;
            gc_pen = XCreateGC(x11->dpy, pen, mask, &vals);
            XPutImage(x11->dpy, pen, gc_pen, ximage, 0, 0, 0, 0,
                    ximage->width, ximage->height); 
            ximg_pen = ximage;
        }
    }

    img_eraser = Util::ImageFromFile(IMAGEPATH "eraser.png");
    if (img_eraser)
    {
        XImage* ximage = Util::ImageToXImage(x11->dpy, x11->screen, img_eraser);
        if (!ximage)
        {
            ERR("Could not create ximage for pen.\n");
        } else
        {
            XGCValues vals; 
            long mask = 0;
            gc_eraser = XCreateGC(x11->dpy, eraser, mask, &vals);
            XPutImage(x11->dpy, pen, gc_eraser, ximage, 0, 0, 0, 0,
                    ximage->width, ximage->height); 
            ximg_eraser = ximage;
        }
    }

    img_wipe = Util::ImageFromFile(IMAGEPATH "wipe.png");
    if (img_wipe)
    {
        XImage* ximage = Util::ImageToXImage(x11->dpy, x11->screen, img_wipe);
        if (!ximage)
        {
            ERR("Could not create ximage for pen.\n");
        } else
        {
            XGCValues vals; 
            long mask = 0;
            gc_wipe = XCreateGC(x11->dpy, wipe, mask, &vals);
            XPutImage(x11->dpy, pen, gc_wipe, ximage, 0, 0, 0, 0,
                    ximage->width, ximage->height); 
            ximg_wipe = ximage;
        }
    }

    XMapWindow(x11->dpy, toolbar);
    XMapRaised(x11->dpy, pen);
    XMapRaised(x11->dpy, eraser);
    XMapRaised(x11->dpy, wipe);
    XMapRaised(x11->dpy, icon);
    
    XFlush(x11->dpy);
    XSync(x11->dpy, FALSE);
}

void Toolbar::setButtonSize(int size)
{
    btsize = size;
    rescale(img_icon, &ximg_icon, size);
    XResizeWindow(x11->dpy, icon, size, size);

    rescale(img_pen, &ximg_pen, size);
    XResizeWindow(x11->dpy, pen, size, size);

    rescale(img_eraser, &ximg_eraser, size);
    XResizeWindow(x11->dpy, eraser, size, size);

    rescale(img_wipe, &ximg_wipe, size);
    XResizeWindow(x11->dpy, wipe, size, size);

    setVertical(vertical);
}

void Toolbar::rescale(Magick::Image *magickimg, XImage** ximg, int size)
{
    if (*ximg != NULL)
        XDestroyImage(*ximg);
    Magick::Image img = *magickimg;
    img.scale(Magick::Geometry(size, size, 0, 0, false, false));

    *ximg = Util::ImageToXImage(x11->dpy, x11->screen, &img);
}

void Toolbar::setVertical(bool vertical)
{
    this->vertical = vertical;

    if (vertical)
    {
        XResizeWindow(x11->dpy, toolbar, btsize, btsize * 4);
        XMoveWindow(x11->dpy, icon, 0, 0);
        XMoveWindow(x11->dpy, pen, 0, btsize);
        XMoveWindow(x11->dpy, eraser, 0, btsize * 2);
        XMoveWindow(x11->dpy, wipe, 0, btsize * 3);
    } else {
        XResizeWindow(x11->dpy, toolbar, btsize * 4, btsize);
        XMoveWindow(x11->dpy, icon, 0, 0);
        XMoveWindow(x11->dpy, pen, btsize, 0);
        XMoveWindow(x11->dpy, eraser, btsize * 2, 0);
        XMoveWindow(x11->dpy, wipe, btsize * 3, 0);

    }
}

void Toolbar::move(int x, int y)
{
    XMoveWindow(x11->dpy, toolbar, x, y);
}

void Toolbar::registerForEvents(XEventClass* evclass)
{
    XSelectExtensionEvent(x11->dpy, pen, evclass, 1);
    XSelectExtensionEvent(x11->dpy, eraser, evclass, 1);
    XSelectExtensionEvent(x11->dpy, wipe, evclass, 1);
}

bool Toolbar::hasWindow(Window win)
{
    return (win == pen || win == eraser || win == wipe);
}

void Toolbar::handleClick(Pointer* device, Window win)
{
    if (win == wipe)
    {
        parent->wipe();
        TRACE("WIPE!");
    } else if (win == eraser) {
        device->setMode(ERASER);
    } else if (win == pen) {
        device->setMode(PEN);
    }

}

/**
 * Limit this toolbar to only the given device.
 */
void Toolbar::restrictTo(int device)
{
    MPXACLAddPermDevices(toolbar, &device, 1);
}

void Toolbar::repaint()
{
    if (ximg_pen && gc_pen)
        XPutImage(x11->dpy, pen, gc_pen, ximg_pen, 0, 0, 0, 0, btsize,
                btsize);

    if (ximg_eraser && gc_eraser)
        XPutImage(x11->dpy, eraser, gc_eraser, ximg_eraser, 0, 0, 0, 0,
                btsize, btsize); 
    if (ximg_wipe && gc_wipe)
        XPutImage(x11->dpy, wipe, gc_wipe, ximg_wipe, 0, 0, 0, 0, btsize,
                btsize);

    XPutImage(x11->dpy, icon, gc_icon, ximg_icon, 0, 0, 0, 0, btsize, btsize);
}
