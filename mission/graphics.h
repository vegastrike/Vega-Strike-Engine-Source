/***************************************************************************
 *                           graphics.h  -  description
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

#ifndef GRAPHICS_H
#define GRAPHICS_H
#include "selector.h"
#include <unistd.h>

#define WIDTH 640
#define HEIGHT 480

#define DOUBLE_BUFFER

void InitGraphics(void);
void show_main(void);

void StartFrame(void);
void EndFrame(void);

void ProcessMouseClick(int button, int state, int x, int y);
void ProcessMouseMovePassive(int x, int y);
void ProcessMouseMoveActive(int x, int y);
void ChangeSize(int wid, int hei);

void AddMissionsToTree(char *path, char *parent);

#endif    // GRAPHICS_H
