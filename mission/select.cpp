/***************************************************************************
 *                           mission.cpp  -  description
 *                           ----------------------------
 *                           begin                : January 31, 2002
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

#include "selector.h"

int main(int argc, char *argv[]) {
	int i = 0;
	for (i=strlen(argv[0]);argv[0][i]!='\\'&&argv[0][i]!='/';i--) {
	}
	argv[0][i+1]='\0';
	chdir(argv[0]);
#ifdef _WIN32
	FreeConsole();
#endif
	glutInit(&argc, argv);
	Start(1);
	return 0;
}
