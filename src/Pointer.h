/* $Id: Pointer.h,v 1.7 2006/10/11 09:03:33 whot Exp $ */
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

#ifndef __POINTER_H__
#define __POINTER_H__


#include<X11/Xlib.h>
#include<X11/extensions/XInput.h>
#include <Magick++.h>

const int DFLT_POINTERSIZE = 3;


enum event_classes {
    XI_MOTION = 0,
    XI_PRESS = 1,
    XI_RELEASE = 2
};
/**
 * Pointer represents one pointer. Each pointer has a position, selected
 * colors etc. A pointer is NOT a user as a user may have multiple pointers.
 */
class Pointer {
    public:
        static int xi_motion;
        static int xi_press;
        static int xi_release;

        int x; /* x position of pointer */
        int y; /* y position of pointer */
        int id ; /* id is the same as the device id */

	XDevice* dev;


    private:
	int size;
	XColor color;
	XEventClass* evclasses;
	XImage* icon; // the small XImage incarnation of img
        Magick::Image* img;

    public:
        Pointer(int device_id, XEventClass* evclasses, XImage* icon, Magick::Image* img);
        ~Pointer();

	void setSize(int size);
	int  getSize();

        long getColorPixel();
        void setColor(XColor color);

        XEventClass* getEventClass(int which);
	XImage* getIcon();         // get the small one as XImage
	Magick::Image* getImage(); // get the full one
};

#endif

