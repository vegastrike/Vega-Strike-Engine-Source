/*
#====================================
# @file   : display.h
# @version: 2020-02-11
# @created: 2002-07-18
# @author : David Ranger (sabarok@start.com.au)
# @author : pyramid
# @author : vegastrike development team
# @brief  : setup configuration 
#====================================
*/
/***************************************************************************
 *                           display_gtk  -  description
 *                           ----------------------------
 *                           begin                : January 18, 2002
 *                           copyright            : (C) 2002 by David Ranger
 *                           email                : sabarok@start.com.au
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 **************************************************************************/

// Check to see if we're already loaded
#ifndef DISPLAY_H
#define DISPLAY_H

#include "central.h"

void InitGraphics(int*,char***);
void ShowMain();

#define GET_TITLE char title[1000]="Vega Strike Settings"; \
  if (strlen(CONFIG.program_name)+strlen(VERSION)<800) { \
    sprintf(title, "Settings - %s - Version 0.9.0", \
    CONFIG.program_name); \
  }

#define GET_STATIC_TEXT char static_text[1000]="Vega Strike requires \
the latest drivers for your video card.\nIf you run into problems please \
upgrade your video drivers.\n\nTo adjust volume levels in-game, use F9/F10 \
for sound and F11/F12 for music."

#endif    //DISPLAY_H
