/* $Id: Util.h,v 1.2 2006/10/04 07:42:52 whot Exp $ */
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

/**
 * Utility functions.
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include<X11/X.h>
#include<X11/Xlib.h>
#include<Magick++.h>
#include "XConn.h"
#include "logger.h"

class Util 
{
  static int bitcount(unsigned int x);

 public:
  static Magick::Image* ImageFromFile(const char* filename);
  static bool ImageToFile(Magick::Image* image, const char* filename);

  static XImage* ImageToXImage(XConn* x11, Magick::Image* image);
  static Magick::Image* XImageToImage(XConn* x11, const XImage* ximage);
};

#endif
