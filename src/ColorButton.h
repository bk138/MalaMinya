/* $Id: ColorButton.h,v 1.4 2006/10/13 06:50:41 whot Exp $ */
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
#ifndef __COLORBUTTON_H___
#define __COLORBUTTON_H___

#include <X11/extensions/XI.h>
#include "XConn.h"

class ColorButton 
{
    private:
        XConn* x11;
        Window win;
        Window parent;
        XColor color;

    public:
        ColorButton(XConn* x11, Window parent, int r, int g, int b);
        void move(int x, int y);
        void resize(int w, int h);
        void registerEvent(XEventClass* evclass);
        bool hasWindow(Window win);

        XColor getColor();
};

#endif
