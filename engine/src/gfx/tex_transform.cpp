/**
* tex_transform.cpp
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#include "vsimage.h"

#include <string.h>
#include <png.h>

unsigned char * texTransform( int &bpp, int &color_type, unsigned long &width, unsigned long &height, unsigned char **rp )
{
    unsigned char *data;
    unsigned int   row_size;
    assert( bpp == 8 );
    if ( (color_type&PNG_HAS_PALETTE) || ( !(color_type&PNG_HAS_COLOR) ) )
        row_size = width*(bpp/8)*sizeof (unsigned char)*( (color_type&PNG_HAS_ALPHA) ? 2 : 1 );
    else
        row_size = width*(bpp/8)*sizeof (unsigned char)*( (color_type&PNG_HAS_ALPHA) ? 4 : 3 );
    data = (unsigned char*) malloc( row_size*height );
    for (unsigned int i = 0; i < height; i++)
        memcpy( data+i*row_size, rp[i], row_size );
    return data;
}

unsigned char * heightmapTransform( int &bpp,
                                    int &color_type,
                                    unsigned long &width,
                                    unsigned long &height,
                                    unsigned char **row_pointers )
{
    unsigned short *dat = (unsigned short*) malloc( sizeof (unsigned short)*width*height );
    if ( (bpp == 8 && color_type
          == PNG_COLOR_TYPE_RGB_ALPHA) || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA ) {
        if (bpp == 8 && color_type == PNG_COLOR_TYPE_GRAY) {
            for (unsigned int i = 0; i < height; i++) {
                unsigned long iwid = i*width;
                for (unsigned int j = 0; j < width; j++)
                    dat[iwid+j] = row_pointers[i][j];
            }
        } else {
            if ( (bpp == 16 && color_type == PNG_COLOR_TYPE_GRAY) || (bpp == 8 && color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ) {
                for (unsigned int i = 0; i < height; i++)
                    memcpy( &dat[i*width], row_pointers[i], sizeof (unsigned short)*width );
            } else {
                //type is RGBA32 or GrayA32
                for (unsigned int i = 0; i < height; i++) {
                    unsigned long iwid = i*width;
                    for (unsigned int j = 0; j < width; j++)
                        dat[iwid+j] = ( ( (unsigned short*) row_pointers[i] )[j*2] );
                }
            }
        }
    } else {
        if (color_type == PNG_COLOR_TYPE_RGB) {
            unsigned int coloffset = (bpp == 8) ? 3 : 6;
            for (unsigned int i = 0; i < height; i++) {
                unsigned long iwid = i*width;
                for (unsigned int j = 0; j < width; j++)
                    dat[iwid+j] = *( (unsigned short*) ( &(row_pointers[i][j*coloffset]) ) );
            }
        } else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
            ///16 bit colors...take Red
            for (unsigned int i = 0; i < height; i++) {
                unsigned long iwid = i*width;
                for (unsigned int j = 0; j < width; j++)
                    dat[iwid+j] = ( ( (unsigned short*) row_pointers[i] )[j*4] );
            }
        }
    }
    bpp = 16;
    color_type = PNG_COLOR_TYPE_GRAY;
    return (unsigned char*) dat;
}

unsigned char * terrainTransform( int &bpp,
                                  int &color_type,
                                  unsigned long &width,
                                  unsigned long &height,
                                  unsigned char **row_pointers )
{
    unsigned char *dat = (unsigned char*) malloc( sizeof (unsigned char)*width*height );
    if ( (bpp == 8 && color_type
          == PNG_COLOR_TYPE_RGB_ALPHA) || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA ) {
        if (bpp == 8 && color_type == PNG_COLOR_TYPE_GRAY) {
            for (unsigned int i = 0; i < height; i++)
                memcpy( &dat[i*width], row_pointers[i], sizeof (unsigned char)*width );
        } else {
            if ( (bpp == 16 && color_type == PNG_COLOR_TYPE_GRAY) || (bpp == 8 && color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ) {
                for (unsigned int i = 0; i < height; i++) {
                    unsigned long iwid = i*width;
                    for (unsigned int j = 0; j < width; j++)
                        dat[iwid+j] = (row_pointers[i])[j*2];
                }
            } else {
                //type is RGBA32 or GrayA32
                for (unsigned int i = 0; i < height; i++) {
                    unsigned long iwid = i*width;
                    for (unsigned int j = 0; j < width; j++)
                        dat[iwid+j] = ( (row_pointers[i])[j*4] );
                }
            }
        }
    } else {
        if (color_type == PNG_COLOR_TYPE_RGB) {
            unsigned int coloffset = (bpp == 8) ? 3 : 6;
            for (unsigned int i = 0; i < height; i++) {
                unsigned long iwid = i*width;
                for (unsigned int j = 0; j < width; j++)
                    dat[iwid+j] = *( (unsigned char*) ( &(row_pointers[i][j*coloffset]) ) );
            }
        } else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
            ///16 bit colors...take Red
            for (unsigned int i = 0; i < height; i++) {
                unsigned long iwid = i*width;
                for (unsigned int j = 0; j < width; j++)
                    dat[iwid+j] = ( ( (unsigned short*) row_pointers[i] )[j*4] )/256;
            }
        }
    }
    bpp = 8;
    color_type = PNG_COLOR_TYPE_GRAY;

    return (unsigned char*) dat;
}

