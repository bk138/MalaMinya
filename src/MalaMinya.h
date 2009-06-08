/* $Id: MalaMinya.h,v 1.11 2006/10/13 06:50:41 whot Exp $ */
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

#ifndef __MALAMINYA_H__
#define __MALAMINYA_H__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>
#include <map>
#include <vector>

#include "Pointer.h"
#include "Toolbar.h"
#include "ColorButton.h"
#include "XConn.h"

#ifdef MAKEDIST
#define IMAGEPATH "/opt/MPX/images/"
#else
#define IMAGEPATH "../images/"
#endif



const int MENUWIDTH = BT_SIZE * 4;
const int MENUHEIGHT = BT_SIZE;
const int NO_USERS = 8;
const int WIDTH = BT_SIZE * 20;
const int HEIGHT = WIDTH;
using namespace std;

class MalaMinya {
    private:
        /* UI stuff */
        XConn* x11;

        int width;
        int height;

        Window win;
        Window menuswin;
        Window canvaswin;
        Pixmap backbuffer;
        GC buffer;
        
        GC canvas;

	// XI2 opcode
	int xi2opcode;

        /* Pointer objects, one for each connected mouse. */
        map<int, Pointer*> pointers; 

        /* The toolbars, one for each connected mouse too. */
        vector<Toolbar*> toolbars;

        /* All color buttons */
        vector<ColorButton*> cbuttons;

    public: 
        MalaMinya(char* display);
        ~MalaMinya();
        void init(); /* show the GUI */
        void run();

        void wipe();
	bool save(int id); // id specifies who saves
	void pensize(Pointer* dev);

    private:
        void initGUI();
        void initToolbars();
        void initColorButtons();
        void initDevices(); /* initialize the input devices */
        void registerEvents();
      
        void handleMotionEvent(XIDeviceEvent* mev);
        void handleButtonEvent(XIDeviceEvent* bev);
        void handleConfigure(XConfigureEvent* ev);
	void handleHierarchyChangedEvent(XIHierarchyEvent* ev);

        void repaintCanvas();

	Pointer* findPointer(int id);
        void updatePointerIcons();

        Toolbar* findToolbarFromWindow(Window win);
        void repaintToolbars();
        void placeToolbars();

        ColorButton* findColorButton(Window win);
	void placeColorButtons();
};

#endif
