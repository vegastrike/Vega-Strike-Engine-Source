/***************************************************************************
 *                           central.cpp  -  description
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

#include "central.h"

glob_t *MISSIONS;
struct mission_data DATA;

// Primary initialization function. Sets everything up and takes care of the program
char *Start(int run_vegastrike) {
	InitGraphics();

	DATA.name = NewString("");
	DATA.author = NewString("");
	DATA.description = NewString("");
	DATA.briefing = NewString("");
	DATA.path = NewString("");

	ShowMain(1);
	gtk_main();
	return DATA.path;
}

void RunMission(void) {
	if (DATA.path[0] == '\0') { cout << "No mission selected\n"; return; }
	cout << "Starting " << MISSION_PROGRAM << " with mission " << DATA.path << endl;
	execlp("./vegastrike", "./vegastrike", DATA.path, NULL);
}
