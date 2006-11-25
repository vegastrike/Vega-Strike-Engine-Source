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

#define GET_TITLE char title[1000]="VegaStrike Configurator"; \
  if (strlen(CONFIG.program_name)+strlen(VERSION)<800) { \
    sprintf(title, "Program Configuration - %s - Version 0.4.0", CONFIG.program_name); \
  }

#define GET_STATIC_TEXT char static_text[1000]="Vega Strike requires the latest drivers for your video card.\nIf you run into problems please upgrade your video drivers."

#endif    //DISPLAY_H
