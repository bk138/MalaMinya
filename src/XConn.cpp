/* $Id: XConn.cpp,v 1.3 2006/10/11 09:03:33 whot Exp $ */
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

#include "XConn.h"
#include "Error.h"

/*
 * Connect to the given X Server and get all default settings.
 */
XConn::XConn(char* host) 
{
    dpy = XOpenDisplay(host);
    if (!dpy)
        throw Error("Could not connect to display");
    getDefaults();
}


/**
 * Get most used default settings and fill member variables.
 */
void XConn::getDefaults()
{
    screen = DefaultScreen(dpy);
    vis = DefaultVisual(dpy, screen);
    black = BlackPixel(dpy, screen);
    white = WhitePixel(dpy, screen);
    root = RootWindow(dpy, screen);
    depth = DefaultDepth(dpy, screen);
    cmap = DefaultColormap(dpy, screen);

    wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", false);
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", false);
}


XConn::~XConn()
{
    XCloseDisplay(dpy);
}
