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
#include <stdio.h>
#include <vector>
#include "gfxlib.h"
using std::vector;

void ShowColor(float x, float y, float wid, float hei, float red, float green, float blue, float alpha);
void ShowText(float x, float y, float wid, int size, const char *string, int no_end);
float WidthOfChar(char chr);
void SetSoftwareMousePosition (int x, int y);
void StartGUIFrame(GFXBOOL clear=GFXTRUE);
void DrawGlutMouse (int mousex,int mousey, class VSSprite * spr);
void EndGUIFrame(bool drawmouseover);
extern int HAS_ALPHA;

#endif    // GLUT_SUPPORT_H
