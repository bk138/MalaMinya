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
#include <stdint.h>


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
XImage* Util::ImageToXImage(XConn* x11, Magick::Image* image)
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
           
      if (x11->depth >= 24) 
	{
	  ximage = XCreateImage (x11->dpy, 
				 CopyFromParent, x11->depth, 
				 ZPixmap, 0, 
				 (char*)buffer,
				 image->columns(), image->rows(),
				 32, 0);
	}
      else
	if (x11->depth >= 15) 
	  {
	    double rRatio = x11->vis->red_mask / 255.0;
	    double gRatio = x11->vis->green_mask / 255.0;
	    double bRatio = x11->vis->blue_mask / 255.0;

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
		
		r &= x11->vis->red_mask;
		g &= x11->vis->green_mask;
		b &= x11->vis->blue_mask;
		
		newBuf[outIndex] = r | g | b;
		++outIndex;
	      }		
	    free(buffer);
		
	    ximage = XCreateImage (x11->dpy,
				   CopyFromParent, x11->depth,
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

       

Magick::Image* Util::XImageToImage(XConn* x11, const XImage* ximage)
{
   try 
     {
       Magick::Image *image = new Magick::Image();
       image->modifyImage();
       image->type(Magick::TrueColorType);

       if(x11->depth == 32)
	 image->read(ximage->width, ximage->height, "BGRA", Magick::CharPixel, ximage->data);
       else if(x11->depth == 16)
	 {
	   // set size
	   image->size(Magick::Geometry(ximage->width, ximage->height));

	   image->modifyImage(); // get lock
	    
	   // get low level access
	   Magick::PixelPacket* pp = image->getPixels(0, 0, image->columns(), image->rows());

	   uint16_t* src = (uint16_t*)ximage->data;
	   for (int i = 0; i < ximage->width * ximage->height; ++i) 
	     {
	       // get the 16 bits of the current pixel
	       uint16_t r, g, b;
	       r = g = b = *src;
	       
	       // extract the color values
	       r &= x11->vis->red_mask;
	       r = r >> bitcount(x11->vis->green_mask | x11->vis->blue_mask);

	       g &= x11->vis->green_mask;
	       g = g >> bitcount(x11->vis->blue_mask);

	       b &= x11->vis->blue_mask;

	       
	       pp->red = r * (1 << (sizeof(Magick::Quantum)*8 - bitcount(x11->vis->red_mask)));
	       pp->green = g * (1 << (sizeof(Magick::Quantum)*8 - bitcount(x11->vis->green_mask)));
	       pp->blue = b * (1 << (sizeof(Magick::Quantum)*8 - bitcount(x11->vis->blue_mask)));

	       pp++;
	       src++;
	     }		
	 }
       else
	 { 
	   throw Magick::Exception("Converting ximage to image failed");
	   delete image;
	 }

       return image;
   }
   catch (Magick::Exception &error)
     {
       ERR("Converting image failed.\n");
       ERR("  -- Error: %s\n", error.what());
       return NULL;
     }
 
}


int Util::bitcount(unsigned int n) 
{
   int count = 0;
   while (n) 
     {
       count += n & 0x1u;
       n >>= 1;
     }
   return count;
}
