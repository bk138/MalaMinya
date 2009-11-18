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


// saves an image to a file
bool Util::ImageToFile(Magick::Image* image, const char* filename)
{
  try
    {
      image->write(filename);
      return true; 
    }
  catch (Magick::Exception &error)
    {
      ERR("Saving image %s failed.\n", filename);
      ERR("  -- Error: %s\n", error.what());
      return false;
    }
 
}




/**
 * Converts a Magick::Image to an XImage that can be displayed on the screen.
 * @param dpy The X Display.
 */
XImage* Util::ImageToXImage(Display* dpy, int screen, Magick::Image* image)
{
  try 
    {
      int numBufBytes= 4 * image->columns() * image->rows() * sizeof(char);
      char* buffer = (char*) malloc(numBufBytes);

      if (!buffer)
        {
	  ERR("Cannot create buffer.\n");
	  throw Magick::Exception("Cannot create XImage");
        }

      image->write(0, 0, image->columns(), image->rows(), "BGRA",
		   Magick::CharPixel,
		   buffer);

      //
      // now, create an ximage*, convert buffer to right depth if needed
      //
      XImage *ximage = NULL;
      int depth = DefaultDepth(dpy, screen);
      
      if (depth >= 24) 
	{
	  ximage = XCreateImage (dpy, 
				 CopyFromParent, depth, 
				 ZPixmap, 0, 
				 (char*)buffer,
				 image->columns(), image->rows(),
				 32, 0);
	}
      else
	if (depth >= 15) 
	  {
	    Visual *vis = DefaultVisual (dpy, screen);
	    double rRatio = vis->red_mask / 255.0;
	    double gRatio = vis->green_mask / 255.0;
	    double bRatio = vis->blue_mask / 255.0;

	    size_t numNewBufBytes = (2 * image->columns() * image->rows() * sizeof(char) );
	    u_int16_t *newBuf = (u_int16_t*) malloc(numNewBufBytes);
	  
	    int outIndex = 0;	
	    for (int i = 0; i < numBufBytes; ++i) 
	      {
		unsigned int r, g, b;
		
		b = (buffer[i] * bRatio);
		++i;
		g = (buffer[i] * gRatio);
		++i;
		r = (buffer[i] * rRatio);
		++i;
		
		r &= vis->red_mask;
		g &= vis->green_mask;
		b &= vis->blue_mask;
		
		newBuf[outIndex] = r | g | b;
		++outIndex;
	      }		
	    free(buffer);
		
	    ximage = XCreateImage (dpy,
				   CopyFromParent, depth,
				   ZPixmap, 0,
				   (char *) newBuf,
				   image->columns(), image->rows(),
				   16, 0);
	  } 
	else 
	  {
	    ERR("Converting image to display depth failed.\n");
	    throw Magick::Exception("Converting image to display depth failed");
	    return NULL;				
	  }
      
   
      if (!ximage)
        {
	  ERR("Cannot create XImage.\n");
	  throw Magick::Exception("Cannot create XImage");
        }
            
      return ximage;

    }
  catch (Magick::Exception &error)
    {
      ERR("Converting image failed.\n");
      ERR("  -- Error: %s\n", error.what());
      return NULL;
    }
}

       

Magick::Image* Util::XImageToImage(const XImage* ximage)
{
   try 
     {
       Magick::Image *image = new Magick::Image();
       image->modifyImage();
       image->type(Magick::TrueColorType);
       image->read(ximage->width, ximage->height, "BGRA", Magick::CharPixel, ximage->data);
       return image;
   }
   catch (Magick::Exception &error)
     {
       ERR("Converting image failed.\n");
       ERR("  -- Error: %s\n", error.what());
       return NULL;
     }
 
}
