/* $Id: logger.h,v 1.4 2006/10/04 07:42:52 whot Exp $ */
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

#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <stdio.h>

#define DEBUG
#define DEBUG_VERBOSE
#define ERROR

#ifdef DEBUG
#define DBG(...) printf("DBG:   " __VA_ARGS__)
#else
#define DBG(...) /* __VA_ARGS__ */
#endif

#ifdef DEBUG_VERBOSE
#define TRACE(...) printf("TRACE: " __VA_ARGS__)
#else
#define TRACE(...) /* __VA_ARGS__ */
#endif

#ifdef ERROR
#define ERR(...) fprintf(stderr, "ERROR: " __VA_ARGS__)
#else
#define ERR(...) /* __VA_ARGS__ */
#endif

#endif
