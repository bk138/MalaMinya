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

#include "Pointer.h"

int Pointer::xi_motion = 0;
int Pointer::xi_press = 0;
int Pointer::xi_release = 0;

/**
 * Creates a new pointer with a given ID. The evclasses has to be an array of
 * 3 event classes in the order motion, press and release.
 */
Pointer::Pointer(int id, XEventClass* evclasses, XImage* icon)
{
    this->id = id;
    this->x = this->y = -100;
    this->size = DFLT_POINTERSIZE;

    this->evclasses = new XEventClass[3];
    this->evclasses[XI_MOTION] = evclasses[XI_MOTION];
    this->evclasses[XI_PRESS] = evclasses[XI_PRESS];
    this->evclasses[XI_RELEASE] = evclasses[XI_RELEASE];

    this->icon = icon;
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

XEventClass* Pointer::getEventClass(int which)
{
    return &evclasses[which];
}

Pointer::~Pointer()
{
    delete icon;
    delete evclasses;
}
