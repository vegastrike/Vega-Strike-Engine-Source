/*
 * jpeg_memory.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021-2022 Stephen G. Tuggy
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#define JPEG_SUPPORT
#ifdef JPEG_SUPPORT   /* Always true? */
#ifndef __JPEG_MEMORY_H
#define __JPEG_MEMORY_H

#include <stdio.h>
#include <string.h>
#include <cstdint>
#if defined (_WIN32) && !defined (__CYGWIN__)
#ifndef NOMINMAX
#define NOMINMAX
#endif //tells VCC not to generate min/max macros
#include <windows.h>
#endif
#ifdef _WIN32
#define XMD_H
#endif

#if !defined(HAVE_BOOLEAN)
    typedef int32_t jpeg_bool;
#else
    typedef bool    jpeg_bool;
#endif

extern "C" {
//# define XMD_H
#   define DONT_USE_EXTERN_C
#   if !defined(HAVE_BOOLEAN)
#       define HAVE_BOOLEAN
#       define boolean      jpeg_bool
#       define TRUE         ((jpeg_bool)true)
#       define FALSE        ((jpeg_bool)false)
#       include <jconfig.h>
#       include <jpeglib.h>
#       undef boolean
#   else
#       include <jconfig.h>
#       include <jpeglib.h>
#   endif
}

/*--------------
 *  A hack to hijack JPEG's innards to write into a memory buffer
 *  ----------------
 *  /  this defines a new destination manager to store images in memory
 *  /  derived by jdatadst.c */
typedef struct
{
    struct jpeg_destination_mgr pub;     /* public fields */
    JOCTET *buffer;                                     /* start of buffer */
    int     bufsize;                                    /* buffer size */
    int     datacount;                                  /* finale data size */
} memory_destination_mgr;

typedef memory_destination_mgr*mem_dest_ptr;

/*----------------------------------------------------------------------------
 *  /  Initialize destination --- called by jpeg_start_compress before any data is actually written. */
void init_destination( j_compress_ptr cinfo );

/*----------------------------------------------------------------------------
 *  /  Empty the output buffer --- called whenever buffer fills up. */
jpeg_bool empty_output_buffer( j_compress_ptr cinfo );


/*----------------------------------------------------------------------------
 *  /  Terminate destination --- called by jpeg_finish_compress
 *  /  after all data has been written.  Usually needs to flush buffer. */
void term_destination( j_compress_ptr cinfo );


GLOBAL( void ) jpeg_memory_dest( j_compress_ptr cinfo, JOCTET*buffer, int bufsize );
int jpeg_compress( char *dst, char *src, int width, int height, int dstsize, int quality );
int jpeg_compress_to_file( char *src, char *file, int width, int height, int quality );
extern void jpeg_memory_src( j_decompress_ptr cinfo, unsigned char *ptr, size_t size );
void jpeg_decompress( unsigned char *dst, unsigned char *src, int size, int *w, int *h );
void jpeg_decompress_from_file( unsigned char *dst, char *file, int size, int *w, int *h );

#endif
#endif

