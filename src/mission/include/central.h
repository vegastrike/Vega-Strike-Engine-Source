/***************************************************************************
 *                           central.h  -  description
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
#ifndef CENTRAL_H
#define CENTRAL_H

#define MISSION_VERSION "1.0"
#define MISSION_PROGRAM "Vega Strike"

#include <expat.h>
#include <glob.h>
#include <gtk/gtk.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "general.h"
#include "display_gtk.h"
#include "file.h"
#include "easydom.h"
#include "xml_support.h"
#include "hashtable.h"

#ifndef MAX_READ
#define MAX_READ 1024	// Maximum number of characters to read from a line
#endif

#define EXT_MISSION "*.mission"
#define EXT_CAMPAIGN "*.campaign"

#define DIR_MISSION "./mission/"
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

#endif    //CENTRAL_H
