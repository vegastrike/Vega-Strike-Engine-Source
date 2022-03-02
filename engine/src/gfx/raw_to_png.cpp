/**
 * raw_to_png.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#include <string.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>

unsigned int height = 256;
unsigned int width = 256;
int rowoffset = 0;
int coloffset = 0;

unsigned short *Rread(const char *myfile) {
    unsigned short *tm = (unsigned short *) malloc(512 * 512 * sizeof(unsigned short));
    int i;
    FILE *fp = VSFileSystem::vs_open(myfile, "rb");
    if (!fp) {
        return tm;
    }
    VSFileSystem::Read(tm, sizeof(unsigned short), 512 * 512, fp);
    VSFileSystem::vs_close(fp);
    unsigned short *smaller = (unsigned short *) malloc(256 * 256 * sizeof(unsigned short));
    for (i = 0; i < 256; i++) {
        memcpy(smaller + (i * 256), tm + rowoffset + ((i + coloffset) * 512), sizeof(unsigned short) * 256);
    }
    if (rowoffset) {
        for (i = 0; i < 256; i++) {
            smaller[i * 256] = tm[rowoffset + -1 + ((i + coloffset) * 512)];
        }
    }
    for (i = 0; i < 64 * 256; i++) {
        int count = 0;
        int j;
        smaller[i] = 256;
    }
    for (i = 0; i < 256; i++) {
        for (int j = 0; j < 64; j++) {
            smaller[i * 256 + j] = 256;
        }
    }
    /*
     *  while (count<256) {
     *  for (j=0;j<16;j++,count++) {
     *   if (smaller[i*256+count]!=0)
     *     j=0;
     *   if (count==256)
     *     break;
     *  }
     *  if (j>5&&count>0) {
     *   for (int k=count-1;k>=0;k--) {
     *     if (smaller[i*256+count]==0) {
     *       smaller[i*256+count]=05;
     *     }else {
     *       break;
     *     }
     *   }
     *  }
     *  }
     */
    if (coloffset) {
        memcpy(smaller, tm + rowoffset + (coloffset - 1) * 512, sizeof(unsigned short) * 256);
    }
    free(tm);
    return smaller;
}

void Wwrite(const char *myfile, unsigned short *data) {
    FILE *fp = VSFileSystem::vs_open(myfile, "wb");
    png_structp png_ptr = png_create_write_struct
            (PNG_LIBPNG_VER_STRING, (png_voidp) NULL, NULL, NULL);
    //user_error_fn, user_warning_fn);
    if (!png_ptr) {
        return;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr,
                (png_infopp) NULL);
        return;
    }
    if (setjmp(png_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        VSFileSystem::vs_close(fp);
        return;
    }
    png_init_io(png_ptr, fp);
    png_set_filter(png_ptr, 0,
            PNG_FILTER_NONE);
    png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

    /* set other zlib parameters */
    png_set_compression_mem_level(png_ptr, 8);
    png_set_compression_strategy(png_ptr,
            Z_DEFAULT_STRATEGY);
    png_set_compression_window_bits(png_ptr, 15);
    png_set_compression_method(png_ptr, 8);

    png_set_IHDR(png_ptr, info_ptr, width, height,
            16, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);
    png_set_swap(png_ptr);
    png_byte **row_pointers = new png_byte *[height];
    for (unsigned int i = 0; i < height; i++) {
        row_pointers[i] = (png_byte *) &data[i * width];
    }
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    //png_write_flush(png_ptr);
    VSFileSystem::vs_close(fp);
    free(data);
    delete[] row_pointers;
}

int main(int argc, char **argv) {
    if (argc > 3) {
        sscanf(argv[3], "%d", &rowoffset);
    }
    if (argc > 4) {
        sscanf(argv[4], "%d", &coloffset);
    }
    Wwrite(argv[2], Rread(argv[1]));
    return 0;
}

