/***************************************************************************
 *                           selector.h  -  description
 *                           ----------------------------
 *                           begin                : February 20, 2002
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
#ifndef SELECTOR_H
#define SELECTOR_H

#define MISSION_VERSION "2.0"
#define MISSION_PROGRAM "Vega Strike"

#include <expat.h>
#if defined(__APPLE__) || defined(MACOSX)
    #include <OpenGL/gl.h>
    #include <GLUT/glut.h>
#else
    #include <GL/gl.h>
    #include <GL/glut.h>
#endif

#if defined(WITH_MACOSX_BUNDLE) && defined(__APPLE__) || defined(MACOSX) 
#include <sys/param.h> // For MAXPATHLEN
extern char macosxExecutablePath[MAXPATHLEN];
#endif

#include <iostream.h>
#include <map>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <unistd.h>
#include <windows.h>
#else
#include <stdio.h>
#include <sys/dir.h>
#include <unistd.h>
#endif    // _WIN32



#include "button.h"
#include "easydom.h"
#include "file.h"
#include "general.h"
#include "graphics.h"
#include "hashtable.h"
#include "text_area.h"
#include "xml_support.h"

#ifndef MAX_READ
#define MAX_READ 1024	// Maximum number of characters to read from a line
#endif

extern char* EXT_MISSION;
extern char* EXT_CAMPAIGN;

extern char* DIR_MISSION;
char *Start(int run_vegastrike);
void RunMission(void);

struct mission_data {
	char *name;
	char *author;
	char *description;
	char *briefing;
	char *path;
};

extern glob_t *MISSIONS;
extern struct mission_data DATA;

#endif    //SELECTOR_H
