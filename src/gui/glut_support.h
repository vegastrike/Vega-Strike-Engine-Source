/***************************************************************************
 *                           glut_support.h  -  description
 *                           --------------------------
 *                           begin                : December 28, 2001
 *                           copyright            : (C) 2001 by David Ranger
 *                           email                : ussreliant@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 ***************************************************************************/

#ifndef GLUT_SUPPORT_H
#define GLUT_SUPPORT_H

/* If you have functions that perform the same operation, but use different parameters,
 * It may be best if you replace the following functions with wrappers to your own functions
 */
#if defined(__APPLE__) || defined(MACOSX)
    #include <OpenGL/gl.h>
    #include <GLUT/glut.h>
#else
    #include <GL/gl.h>
    #include <GL/glut.h>
#endif
#include <png.h>
#include <stdio.h>
#include <vector.h>
#include "gfx/png_texture.h"

struct Texture {
        unsigned int name;
        unsigned int wid, hei;
        Texture () {name=wid=hei=0;}
        Texture (unsigned int wid,unsigned int hei,unsigned int name) {this->wid=wid;this->hei=hei;this->name=name;}
};

Texture ReadTex(char *texfile);
char * readString (FILE * fp,char endchar);

void png_cexcept_error(png_structp png_ptr, png_const_charp msg);
unsigned char *readImage (FILE *fp, int & bpp, int &color_type, unsigned int &width, unsigned int &height, unsigned char * &palette);

void ShowColor(float x, float y, float wid, float hei, float red, float green, float blue, float alpha);
void ShowImage(float x, float y, float wid, float hei, struct Texture image, int tile_h, int tile_w);
void ShowText(float x, float y, float wid, int size, char *string, int no_end);
float WidthOfChar(char chr);

extern int HAS_ALPHA;

#endif    // GLUT_SUPPORT_H
