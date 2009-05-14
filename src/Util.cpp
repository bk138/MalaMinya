/* $Id: Util.cpp,v 1.2 2006/10/04 07:42:52 whot Exp $ */
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

#include "Util.h"


/**
 * Loads an image from a file.
 * @param file The filename of the image.
 */
Magick::Image* Util::ImageFromFile(const char* file)
{
    try
    {
        Magick::Image *image = new Magick::Image();
        image->read(file);
        image->modifyImage();
        image->type(Magick::TrueColorType);
        return image;
    }catch (Magick::Exception &error)
    {
        ERR("Loading image %s failed.\n", file);
        ERR("  -- Error: %s\n", error.what());
        return NULL;
    }
}

/**
 * Converts a Magick::Image to an XImage that can be displayed on the screen.
 * @param dpy The X Display.
 */
XImage* Util::ImageToXImage(Display* dpy, int screen, Magick::Image* image)
{

    try {
        char* buffer = 
            (char*) malloc(4 * image->columns() * image->rows() * sizeof(char));

        if (!buffer)
        {
            ERR("Cannot create buffer.\n");
            throw Magick::Exception("Cannot create XImage");
        }

        image->write(0, 0, image->columns(), image->rows(), "BGRA",
                Magick::CharPixel,
                buffer);

        XImage* ximage = XCreateImage(dpy, DefaultVisual(dpy, screen),
                              DefaultDepth(dpy, screen), ZPixmap, 
                              0, buffer, 
                              image->columns(), image->rows(), 
                              XBitmapPad(dpy), 0); 
        if (!ximage)
        {
            ERR("Cannot create XImage.\n");
            throw Magick::Exception("Cannot create XImage");
        }
            
        return ximage;
    }catch (Magick::Exception &error)
    {
        ERR("Converting image failed.\n");
        ERR("  -- Error: %s\n", error.what());
        return NULL;
    }
}

