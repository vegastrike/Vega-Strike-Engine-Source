/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * This code taken from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

/*******************************************************************\
 								 
    Name:	Pic.h						 
 								 
    Purpose:							 
 	Type declarations and associated macros for use		 
 	with picture files (raw PPM files).				 
 								 
\*******************************************************************/


/*
 *  Data structures
 */

typedef unsigned char   boolean;

#ifndef FALSE
#define   FALSE   ((boolean) 0)
#define   TRUE    ((boolean) 1)
#endif

typedef unsigned char  Pic_byte;

typedef struct Pic_Pixel
{
    Pic_byte   r, g, b;
}
Pic_Pixel;

typedef struct
{
    FILE     *fptr;
    char     *filename;

    short     width;
    short     height;
    short     scanline;
}
Pic;


/*
 *  Memory allocation and other macros :
 */

#define StrAlloc(n)    ((char *) malloc((unsigned)(n)))
#define PixelAlloc(n)  ((Pic_Pixel *) malloc((unsigned)((n)*sizeof(Pic_Pixel))))
#define PixelFree(p)   ((void) free((char *)(p)))


/*
 *  General routines
 */

extern	Pic       *PicOpen(const char* filename, short width, short height );
extern	boolean    PicWriteLine(Pic* ppmFile, Pic_Pixel* pixels);
extern  void       PicClose(Pic* ppmFile);

/*** THE END ***/


#ifdef __cplusplus
} /* extern "C" */
#endif
