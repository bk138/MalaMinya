/* $Id: Pointer.cpp,v 1.6 2006/10/04 07:42:52 whot Exp $ */
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
#include "Util.h"
#include "MalaMinya.h"
#include "Pointer.h"

using namespace std;


/**
 * Creates a new pointer with a given ID. The evclasses has to be an array of
 * 3 event classes in the order motion, press and release.
 */
Pointer::Pointer(int device_id, int icon_nr, XConn* x11)
{
    this->id = device_id;

    this->x = this->y = -100;
    this->size = DFLT_POINTERSIZE;

    char iconId[2] = {'0' + icon_nr, '\0'};
    string file = IMAGEPATH "icon";
    file.append((const char*)iconId);
    file.append(".png");
    this->img = Util::ImageFromFile((char*)file.c_str()); 
    
    // copy in here
    Magick::Image icon_img = *img; 
    icon_img.scale(Magick::Geometry(16, 16, 0, 0, false, false));
    this->icon = Util::ImageToXImage(x11, &icon_img);
}


Pointer::~Pointer()
{
    delete icon;
    delete img;
}

int Pointer::getId()
{
  return id;
}

void Pointer::setSize(int size) 
{
    this->size = size;
}

int Pointer::getSize()
{
  return this->size;
}


void Pointer::setColor(XColor color)
{
    this->color = color;
}

long Pointer::getColorPixel()
{
    return color.pixel;
}


XImage* Pointer::getIcon()
{
  return this->icon;
}


Magick::Image* Pointer::getImage()
{
  return this->img;
}
