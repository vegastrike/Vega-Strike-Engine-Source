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


#include "vegastrike.h"
#include "screenshot.h"
#include "pic.h"
#include <sys/types.h>
#include <sys/stat.h>

static int screenshot_num = 0;

void screenshot()
{
    char buff[20], *p;
    sprintf( buff, "tux_sshot_%d.ppm", screenshot_num++ );
    p = take_screenshot( buff );
    if ( p != NULL ) {
        fprintf( stderr, "Couldn't save %s: %s\n", buff, p );
    } 
} 

char* take_screenshot ( char* filename ) {
    Pic *ppmFile;
    Pic_Pixel *scanline;
    int i, viewport[4];

    glGetIntegerv( GL_VIEWPORT, viewport );
    scanline = PixelAlloc(viewport[2]);

    ppmFile = PicOpen( filename, viewport[2], viewport[3] );
    glReadBuffer( GL_FRONT );

    for (i=viewport[3]-1; i>=0; i--) {
        glReadPixels(viewport[0], viewport[1]+i, viewport[2], 1, GL_RGB, 
		     GL_UNSIGNED_BYTE, scanline);

	/* Make sure write was successful */
        if (!PicWriteLine(ppmFile, scanline)) {
            PixelFree(scanline);
            return "Error writing ppm file.";
        } 
    } 

    PicClose( ppmFile );

    PixelFree(scanline);
    return (char *)0;
}
