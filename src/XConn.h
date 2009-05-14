/* $Id: XConn.h,v 1.3 2006/10/11 09:03:33 whot Exp $ */
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
#ifndef __XCONN_H__
#define __XCONN_H__
#include <X11/Xlib.h>

class XConn {
    public: 
        Display* dpy;
        int screen;
        Visual* vis;
        long black;
        long white;
        Window root;
        int depth;
        Colormap cmap;

        Atom wm_protocols;
        Atom wm_delete_window;

    public:
        /* Connect to given X Server, fill with default variables */
        XConn(char* display);

        ~XConn();

    private:
        void connect(char* display);
        void getDefaults();
};

#endif
