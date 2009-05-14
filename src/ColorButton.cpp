/* $Id: ColorButton.cpp,v 1.4 2006/10/13 06:50:41 whot Exp $ */
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

#include "ColorButton.h"
#include "Error.h"
#include "MalaMinya.h"

/**
 * Create a window with the given color. Color in RGB format in the 0 - 255
 * range.
 */
ColorButton::ColorButton(XConn* x11, Window parent, int r, int g, int b)
{
    this->x11 = x11;
    this->parent = parent;

    color.red = 256 * r;
    color.green = 256 * g;
    color.blue = 256 * b;
    if (!XAllocColor(x11->dpy, x11->cmap, &color))
        throw Error("Cannot create color");


    XSetWindowAttributes attr;
    attr.background_pixel = color.pixel;

    win = XCreateWindow(x11->dpy,
            parent,
            0, 0,
            BT_SIZE, BT_SIZE,
            0, x11->depth,
            InputOutput,
            x11->vis,
            CWBackPixel,
            &attr);

    if (!win)
        throw Error("Cannot create color window!");


    XMapWindow(x11->dpy, win);
}

void ColorButton::move(int x, int y)
{
    XMoveWindow(x11->dpy, win, x, y);
}

void ColorButton::resize(int w, int h)
{
    XResizeWindow(x11->dpy, win, w, h);
}
/**
 */
void ColorButton::registerEvent(XEventClass* evclass)
{
    XSelectExtensionEvent(x11->dpy, win, evclass, 1);

}

bool ColorButton::hasWindow(Window win)
{
    return (this->win == win);
}

XColor ColorButton::getColor()
{
    return color;
}
