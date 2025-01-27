/*
 * base_maker_texture.cpp
 *
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "hashtable.h"
#include "base_maker_texture.h"
#include <string.h>
#include <stdio.h>
#include <png.h>
#ifdef _WIN32
#define XMD_H
#define HAVE_BOOLEAN
#endif
extern "C" {
//YUCK it doesn't even have extern c in the headers!
#include <jpeglib.h>
}

#define strip_16 true

static void png_cexcept_error(png_structp png_ptr, png_const_charp msg) {
    if (png_ptr) {
        //
    }
#ifndef PNG_NO_CONSOLE_IO
    fprintf(stderr, "libpng error: %s\n", msg);
#endif
}

unsigned int Texture::last_handle = (unsigned int) -1;
enum Format { FORMAT_PNG, FORMAT_BMP, FORMAT_JPG, FORMAT_OTHER };
static size_t bogus_return; //added by chuck_starchaser, to get rid of ignored return warnings

static inline bool readPng(FILE *fp, Texture::FileData *data, Texture::TextureTransform tt) {
    unsigned char sig[8];
    png_structp png_ptr;
    png_bytepp row_pointers;
    png_infop info_ptr;
    int interlace_type;
    bogus_return = fread(sig, 1, 8, fp);
/*	if (!png_check_sig(sig, 8)) {
 *               fprintf(stderr, "Invalid PNG image");
 *               return;
 *       }*/
    //The previous check could only be false if the image was modified after being opened (such as on *nix systems).  In that case, worse things will happen anyways.
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
            (png_error_ptr) png_cexcept_error,
            (png_error_ptr) NULL);
    if (png_ptr == NULL) {
        return false;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
        return false;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        /* Free all of the memory associated with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        /* If we get here, we had a problem reading the file */
        return false;
    }
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
#ifdef PNGDEBUG
    printf( "Loading Done. Decompressing\n" );
#endif
    int ctype;
    png_read_info(png_ptr, info_ptr);     /* read all PNG info up to image data */
    png_get_IHDR(png_ptr,
            info_ptr,
            (png_uint_32 *) &data->width,
            (png_uint_32 *) &data->height,
            &data->bpp,
            &ctype,
            &interlace_type,
            NULL,
            NULL);
# if __BYTE_ORDER != __BIG_ENDIAN
    if (data->bpp == 16) {
        png_set_swap(png_ptr);
    }
#endif
    if (data->bpp == 16 && strip_16) {
        png_set_strip_16(png_ptr);
    }
    if (strip_16 && ctype == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }
    if (ctype == PNG_COLOR_TYPE_GRAY && data->bpp < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    png_set_expand(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr,
            info_ptr,
            (png_uint_32 *) &data->width,
            (png_uint_32 *) &data->height,
            &data->bpp,
            &ctype,
            &interlace_type,
            NULL,
            NULL);
    row_pointers = (unsigned char **) malloc(sizeof(unsigned char *) * data->height);
    int numchan = 1;
    if (ctype & PNG_COLOR_MASK_COLOR) {
        numchan = 3;
    }
    if (ctype & PNG_COLOR_MASK_PALETTE) {
        numchan = 1;
    }
    if (ctype & PNG_COLOR_MASK_ALPHA) {
        numchan++;
    }
    unsigned long stride = numchan * sizeof(unsigned char) * data->bpp / 8;
    unsigned char *image = (unsigned char *) malloc(stride * data->width * data->height);
    for (unsigned int i = 0; i < data->height; i++) {
        row_pointers[i] = &image[i * stride * data->width];
    }
    png_read_image(png_ptr, row_pointers);
    //png_read_image(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND , NULL);
    //row_pointers = png_get_rows(png_ptr, info_ptr);
    switch (ctype) {
        case PNG_COLOR_TYPE_GRAY:
            data->color_type = Texture::COLOR_TYPE_GRAY;
            break;
        case PNG_COLOR_TYPE_PALETTE:
            data->color_type = Texture::COLOR_TYPE_PALETTE;
            break;
        case PNG_COLOR_TYPE_RGB:
            data->color_type = Texture::COLOR_TYPE_RGB;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            data->color_type = Texture::COLOR_TYPE_RGB_ALPHA;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            data->color_type = Texture::COLOR_TYPE_GRAY_ALPHA;
            break;
        default:
            data->color_type = Texture::COLOR_TYPE_UNKNOWN;
            break;
    }
/*	if (tt) {
 *               (*tt) (data,row_pointers);
 *               free (image);
 *       }else {
 */
    data->data = image;
    /*
     *  }
     */
    free(row_pointers);
    //png_infop end_info;
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
#ifdef PNGDEBUG
    printf( "Decompressing Done.\n" );
#endif
    /* close the file */
    return true;
}

struct my_error_mgr {
    struct jpeg_error_mgr pub;     //"public" fields
    jmp_buf setjmp_buffer;     //for return to caller
};

METHODDEF(void) my_error_exit(j_common_ptr cinfo) {
    //cinfo->err really points to a my_error_mgr struct, so coerce pointer
    my_error_mgr *myerr = (my_error_mgr *) cinfo->err;
    //Always display the message.
    //We could postpone this until after returning, if we chose.
    (*cinfo->err->output_message)(cinfo);
    //Return control to the setjmp point
    longjmp(myerr->setjmp_buffer, 1);
}

static inline bool readJpg(FILE *fp, Texture::FileData *data, Texture::TextureTransform tt) {
    data->bpp = 8;
    jpeg_decompress_struct cinfo;
    my_error_mgr jerr;
    JSAMPARRAY row_pointers = NULL;   //Output row buffer
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
        //If we get here, the JPEG code has signaled an error.
        //We need to clean up the JPEG object, close the input file, and return.
        jpeg_destroy_decompress(&cinfo);
        data->data = NULL;
        return false;
    }
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src((j_decompress_ptr) &cinfo, fp);
    (void) jpeg_read_header(&cinfo, TRUE);
    data->width = cinfo.image_width;
    data->height = cinfo.image_height;
    (void) jpeg_start_decompress(&cinfo);
    data->color_type = Texture::COLOR_TYPE_RGB;
    if (cinfo.output_components == 1) {
        data->color_type = Texture::COLOR_TYPE_GRAY;
    } else if (cinfo.output_components == 4) {
        data->color_type = Texture::COLOR_TYPE_RGB_ALPHA;
    } else if (cinfo.output_components == 2) {
        data->color_type = Texture::COLOR_TYPE_GRAY_ALPHA;
    }
    row_pointers = (unsigned char **) malloc(sizeof(unsigned char *) * cinfo.image_height);
    data->bpp = 8;
    int numchan = cinfo.output_components;
    unsigned long stride = numchan * sizeof(unsigned char) * data->bpp / 8;
    unsigned char *image = (unsigned char *) malloc(stride * cinfo.image_width * cinfo.image_height);
    for (unsigned int i = 0; i < cinfo.image_height; i++) {
        row_pointers[i] = &image[i * stride * cinfo.image_width];
    }
    int count = 0;
    while (count < data->height) {
        count += jpeg_read_scanlines(&cinfo, &(row_pointers[count]), data->height - count);
    }
    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    data->data = image;
    if (tt) {
        (*tt)(data, row_pointers);
    }
    free(row_pointers);
    return true;
}

static inline bool readBmp(FILE *fp, Texture::FileData *data, Texture::TextureTransform tt) {
    fprintf(
            stderr,
            "Bitmap files are not supported due to lack of an alpha channel.  Use PNG files instead to get better compression.\n");
    return false;
}

static inline Format getFormat(FILE *fp) {
    {
        /// Check for PNG file

        unsigned char sig[8];
        bogus_return = fread(sig, sizeof(char), 8, fp);
        fseek(fp, 0, SEEK_SET);
        if (!png_sig_cmp(sig, 0, 8)) {
            return FORMAT_PNG;
        }
    }
    {
        //Check for JPEG file.

        //First 4 aren't known to me
        //Next 2 bytes is length
        //Next 5 are JFIF\0
        //Next 2 are version numbers
        char sig[13];
        bogus_return = fread(sig, sizeof(char), 13, fp);
        fseek(fp, 0, SEEK_SET);
        if (strncmp(sig + 6, "JFIF", 4) == 0) {
            return FORMAT_JPG;
        }
    }
    {
        /// Check for bitmap file.
        char a = toupper(fgetc(fp));
        char b = toupper(fgetc(fp));
        fseek(fp, 0, SEEK_SET);
        if (a == 'B' && b == 'M') {
            return FORMAT_BMP;
        }
    }
    return FORMAT_OTHER;
}

bool Texture::getTextureData(FILE *fp, Texture::FileData *data, TextureTransform tt) {
    bool ret;
    switch (getFormat(fp)) {
        case FORMAT_PNG:
            /// Load PNG file.
            ret = readPng(fp, data, tt);
            break;
        case FORMAT_JPG:
            /// Load JPEG file.
            ret = readJpg(fp, data, tt);
            break;
        case FORMAT_BMP:
            /// Load bitmap file.
            ret = readBmp(fp, data, tt);
            break;
        case FORMAT_OTHER:
        default:
            return false;
    }
    return ret;
}

bool Texture::getTextureData(const std::string &file, Texture::FileData *data, TextureTransform tt) {
    FILE *fp = fopen(file.c_str(), "rb");
    bool ret = getTextureData(fp, data, tt);
    fclose(fp);
    if (!ret) {
        fprintf(stderr, "Error: file '%s' is not a valid PNG or JPG file.\n", file.c_str());
    }
    return ret;
}

void Texture::loadTexture(FILE *file, TextureTransform tt) {
    FileData data;
    if (!getTextureData(file, &data, tt)) {
        handle = (unsigned int) -1;
        return;
    }
#if defined(__APPLE__) && defined (__MACH__)
    glGenTextures( 1, (GLuint*) &handle );
#else
    glGenTextures(1, &handle);
#endif
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    int rgbif, rgbf;
    switch (data.color_type) {
        case Texture::COLOR_TYPE_RGB_ALPHA:
            rgbif = GL_RGBA8;
            rgbf = GL_RGBA;
            break;
        case Texture::COLOR_TYPE_RGB:
            rgbif = GL_RGB8;
            rgbf = GL_RGB;
            break;
        case Texture::COLOR_TYPE_GRAY_ALPHA:
            rgbif = GL_LUMINANCE8_ALPHA8;
            rgbf = GL_LUMINANCE_ALPHA;
            break;
        case Texture::COLOR_TYPE_GRAY:
            rgbif = GL_LUMINANCE8;
            rgbf = GL_LUMINANCE;
            break;
        case Texture::COLOR_TYPE_PALETTE:
        default:
            rgbif = GL_RGB8;
            rgbf = GL_RGB;
            break;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, rgbif, data.width, data.height, 0, rgbf, GL_UNSIGNED_BYTE, data.data);
}

void Texture::loadTexture(const std::string &file, TextureTransform tt) {
    FILE *fp = fopen(file.c_str(), "rb");
    if (fp) {
        loadTexture(fp, tt);
        fclose(fp);
    } else {
        fprintf(stderr, "Error: Not found!\n");
    }
}

Texture::Texture(const std::string &file, TextureTransform tt) {
    loadTexture(file, tt);
}

Texture::Texture(FILE *file, TextureTransform tt) {
    loadTexture(file, tt);
}

Texture::~Texture() {
    fprintf(stderr, "Texture %d deleted", handle);
    //glDeleteTextures(1,&handle);
}

void Texture::bind() {
//if (last_handle!=handle) {
    last_handle = handle;
    glBindTexture(GL_TEXTURE_2D, handle);
//}
}

