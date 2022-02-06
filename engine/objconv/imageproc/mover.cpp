/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
#include <stdlib.h>
#include <stdio.h>
#ifndef png_jmpbuf
#  define png_jmpbuf( png_ptr ) ( (png_ptr)->jmpbuf )
#endif

#define FINAL_WIDTH 128
#define FINAL_HEIGHT 128
int PNG_HAS_PALETTE = 1;
int PNG_HAS_COLOR = 2;
int PNG_HAS_ALPHA = 4;

/* We can't write in volumes yet so this is useless now
 *  void PngWriteFunc(png_struct *Png, png_bytep bf, png_size_t size)
 *  {
 *       cerr<<"PNG DEBUG : preparing to write "<<size<<" bytes from PngFileBuffer"<<endl;
 *  }
 */

static void png_cexcept_error(png_structp png_ptr, png_const_charp msg)
{
    if (png_ptr) {
    }
#ifndef PNG_NO_CONSOLE_IO
    fprintf(stderr, "libpng error: %s\n", msg);
#endif
}

unsigned char *ReadPNG(FILE *fp,
                       unsigned int &sizeX,
                       unsigned int &sizeY,
                       int &img_depth,
                       int &img_color_type,
                       unsigned char ***row_pointer_ptr)
{
    png_structp png_ptr;
    png_bytepp row_pointers;
    png_infop info_ptr;
    int interlace_type;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                     NULL,
                                     (png_error_ptr) png_cexcept_error,
                                     (png_error_ptr) NULL);
    if (png_ptr == NULL) {
        exit(1);
        return NULL;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
        fprintf(stderr, "VSImage ERROR : PNG info_ptr == NULL !!!\n");
        exit(1);
        return NULL;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        /* Free all of the memory associated with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        /* If we get here, we had a problem reading the file */
        exit(1);
        return NULL;
    }
    png_init_io(png_ptr, fp);

    //png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);     /* read all PNG info up to image data */
    png_get_IHDR(png_ptr,
                 info_ptr,
                 (png_uint_32 *) &sizeX,
                 (png_uint_32 *) &sizeY,
                 &img_depth,
                 &img_color_type,
                 &interlace_type,
                 NULL,
                 NULL);

# if __BYTE_ORDER != __BIG_ENDIAN
    if (img_depth == 16) {
        png_set_swap(png_ptr);
    }
#endif
    if (img_depth == 16) {      //for now
        png_set_strip_16(png_ptr);
    }
    if (img_color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }
    if (img_color_type == PNG_COLOR_TYPE_GRAY && img_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    png_set_expand(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr,
                 info_ptr,
                 (png_uint_32 *) &sizeX,
                 (png_uint_32 *) &sizeY,
                 &img_depth,
                 &img_color_type,
                 &interlace_type,
                 NULL,
                 NULL);
    row_pointers = (unsigned char **) malloc(sizeof(unsigned char *) * sizeY);
    int numchan = 1;
    if (img_color_type & PNG_COLOR_MASK_COLOR) {
        numchan = 3;
    }
    if (img_color_type & PNG_COLOR_MASK_PALETTE) {
        numchan = 1;
    }
    if (img_color_type & PNG_COLOR_MASK_ALPHA) {
        numchan++;
    }
    unsigned long stride = numchan * sizeof(unsigned char) * img_depth / 8;
    unsigned char *image = (unsigned char *) malloc(stride * sizeX * sizeY);
    for (unsigned int i = 0; i < sizeY; i++) {
        row_pointers[i] = &image[i * stride * sizeX];
    }
    png_read_image(png_ptr, row_pointers);
    unsigned char *result;
    result = image;
    //free (row_pointers);
    *row_pointer_ptr = row_pointers;
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return result;
}

enum errort {
    BadFormat,
    Ok
};

errort WritePNG(FILE *fp, unsigned char *data, unsigned int sizeX, unsigned int sizeY, int img_depth, int img_alpha)
{
    png_structp png_ptr = png_create_write_struct
            (PNG_LIBPNG_VER_STRING, (png_voidp) NULL, NULL, NULL);
    if (!png_ptr) {
        return BadFormat;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
        return BadFormat;
    }
    if (setjmp(png_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return BadFormat;
    }
    png_init_io(png_ptr, fp);

    png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
    png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

    /* set other zlib parameters */
    png_set_compression_mem_level(png_ptr, 8);
    png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
    png_set_compression_window_bits(png_ptr, 15);
    png_set_compression_method(png_ptr, 8);

    png_set_IHDR(png_ptr,
                 info_ptr,
                 sizeX,
                 sizeY,
                 img_depth,
                 img_alpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);
# if __BYTE_ORDER != __BIG_ENDIAN
    if (img_depth == 16) {
        png_set_swap(png_ptr);
    }
#endif
    int stride = (img_depth / 8) * (img_alpha ? 4 : 3);
    png_byte **row_pointers = new png_byte *[sizeY];
    for (unsigned int i = 0; i < sizeY; i++) {
        row_pointers[i] = (png_byte *) &data[stride * i * sizeX];
    }
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);
    png_write_flush(png_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    //free (data);
    delete[] row_pointers;
    return Ok;
}

void ModifyImage(unsigned int sizex,
                 unsigned int sizey,
                 int img_depth,
                 int img_alpha,
                 unsigned char **row_pointers,
                 unsigned char output[FINAL_HEIGHT][FINAL_WIDTH][4],
                 int offsetx,
                 int offsety)
{
    memset(output, 0, sizeof(unsigned char) * FINAL_WIDTH * FINAL_HEIGHT * 4);
    int stride = (img_depth / 8) * (img_alpha ? 4 : 3);
    for (unsigned int i = 0; i < sizey; ++i) {
        for (unsigned int j = 0; j < sizex; ++j) {
            if (((int) i) + offsetx > 0 && ((int) j) + offsety > 0) {
                if (i + offsetx < FINAL_WIDTH && j + offsety < FINAL_HEIGHT && i + offsetx > 0 && j + offsety > 0) {
                    output[i + offsetx][j + offsety][0] = row_pointers[i][j * stride + 0];
                    output[i + offsetx][j + offsety][1] = row_pointers[i][j * stride + 1];
                    output[i + offsetx][j + offsety][2] = row_pointers[i][j * stride + 2];
                    output[i + offsetx][j + offsety][3] = row_pointers[i][j * stride + 3];
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc == 5) {
        FILE *fp = fopen(argv[3], "rb");
        if (fp) {
            int deltax = atoi(argv[1]);
            int deltay = atoi(argv[2]);
            unsigned int sizex, sizey;
            int img_depth, img_alpha;
            unsigned char **row_pointers;
            unsigned char *data = ReadPNG(fp, sizex, sizey, img_depth, img_alpha, &row_pointers);
            fclose(fp);
            if (data && (img_alpha & PNG_HAS_ALPHA)) {
                unsigned char output[FINAL_HEIGHT][FINAL_WIDTH][4] = {0};
                ModifyImage(sizex, sizey, img_depth, img_alpha & PNG_HAS_ALPHA, row_pointers, output, deltax, deltay);
                free(row_pointers);
                FILE *wr = fopen(argv[4], "wb");
                WritePNG(wr, (unsigned char *) output, FINAL_WIDTH, FINAL_HEIGHT, 8, PNG_HAS_ALPHA);
                fclose(wr);
            }
        }
    }
}

