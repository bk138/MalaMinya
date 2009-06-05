/* $Id: Toolbar.h,v 1.10 2006/10/13 06:50:42 whot Exp $ */
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

#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__

#include <X11/Xlib.h>
#include "Pointer.h"
#include "XConn.h"
#include <Magick++.h>

const int BT_SIZE = 30;

class MalaMinya;

class Toolbar {
    private:
	int id;

        MalaMinya* parent;

        XConn* x11;
        Window menuswin; /* the parent window for the toolbars*/

        Window icon;
        GC gc_icon;
        XImage* ximg_icon;
        Magick::Image* img_icon;

        Window toolbar;
        GC gc_toolbar;

        Window pen;
        GC gc_pen;
        XImage* ximg_pen;
        Magick::Image* img_pen;

        Window save;
        GC gc_save;
        XImage* ximg_save;
        Magick::Image* img_save;

        Window wipe;
        GC gc_wipe;
        XImage* ximg_wipe;
        Magick::Image* img_wipe;

        XColor color_pen;
        XColor color_save;
        XColor color_wipe;
        
        bool vertical;
        int  btsize;

	int mydevice;
	bool is_restricted;

    public:
        Toolbar(MalaMinya* mm, int id, XConn* x11, Window menuswin, Magick::Image* icon);
	~Toolbar();
        void move(int x, int y);
	void registerEvent(int ev);
        bool hasWindow(Window win);
        void handleClick(Pointer* device, Window win);
        void restrictTo(int device);
        void setButtonSize(int size);

        void repaint();
        void setVertical(bool vertical);

    private:
        void rescale(Magick::Image* img, XImage** ximg, int size);
};

#endif
