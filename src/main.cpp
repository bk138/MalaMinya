/* $Id: main.cpp,v 1.3 2006/10/04 07:42:52 whot Exp $ */
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
#include "Error.h"
#include "MalaMinya.h"
#include <iostream>
#include <cstring>


int main(int argc, char** argv) 
{
    char* display = NULL;
    int w, h = 0;

    try 
    {
      while(argc > 1) 
	{
	  --argc;
	  if ( strcmp(argv[argc-1], "-display") == 0 ) 
	    {                            
	      display = (argv[argc]);                                 
	      --argc;
	    }      
	  else
	    if ( strcmp(argv[argc-1], "-width") == 0 ) 
	      {                            
		w = atoi(argv[argc]);
		if(w < 300)    
		  w = 300;
		--argc;
	      }      
	    else
	      if ( strcmp(argv[argc-1], "-height") == 0 ) 
		{                            
		  h = atoi(argv[argc]);
		  if(h < 300)    
		    h = 300;
		  --argc;
		}
	}

      MalaMinya mm(display, w, h);
      mm.init();
      mm.run();
      
    } catch (Error e) 
    {
        std::cout << e.getMessage() << "\n";
        return 1;
    }

    return 0;
}
