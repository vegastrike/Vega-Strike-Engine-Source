/***************************************************************************
 *                           display_console.h  -  description
 *                           ----------------------------
 *                           begin                : January 18, 2002
 *                           copyright            : (C) 2002 by David Ranger
 *                           email                : sabarok@start.com.au
 *                           copyright            : (C) 2023 by Benjamen R. Meyer
 *                           copyright            : (C) 2025 by Stephen G. Tuggy
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
#ifndef VEGA_STRIKE_ENGINE_SETUP_DONT_LINK_DISPLAY_CONSOLE_H
#define VEGA_STRIKE_ENGINE_SETUP_DONT_LINK_DISPLAY_CONSOLE_H

#include "central.h"

void InitGraphics();
void ShowMain();
void ShowChoices(int val);

#endif    //VEGA_STRIKE_ENGINE_SETUP_DONT_LINK_DISPLAY_CONSOLE_H
