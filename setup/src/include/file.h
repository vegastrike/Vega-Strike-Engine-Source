/***************************************************************************
 *                           file.h  -  description
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
#ifndef FILE_H
#define FILE_H

#include "central.h"

void LoadMainConfig(void);
void LoadConfig(void);
void Modconfig(int setting, char *name, char *group);
void EnableSetting(char *name, char *group);
void DisableSetting(char *name, char *group);

#endif
