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

char* EXT_MISSION = "*.mission";
char* EXT_CAMPAIGN = "*.campaign";
char* DIR_MISSION = "mission";

#if defined(WITH_MACOSX_BUNDLE)
char macosxExecutablePath[MAXPATHLEN];
#endif

int main(int argc, char *argv[]) {
#if defined(WITH_MACOSX_BUNDLE)
        // We need to set the path back 2 to make everything ok.
        char parentdir[MAXPATHLEN];
        char *c;
        strncpy ( parentdir, argv[0], sizeof(parentdir) );
        c = (char*) parentdir;
    
        while (*c != '\0')     /* go to end */
            c++;
        
        while (*c != '/')      /* back up to parent */
            c--;
        
        *c++ = '\0';             /* cut off last part (binary name) */
    
        chdir (parentdir);/* chdir to the binary app's parent */
        chdir ("../Resources");/* chdir where the vegastrike aexe is */
        getcwd(macosxExecutablePath, MAXPATHLEN);
        chdir ("../../../");/* chdir where the data dir is */
#else
//	int i = 0;
//	for (i=strlen(argv[0]);argv[0][i]!='\\'&&argv[0][i]!='/';i--) {
//	}
//	argv[0][i+1]='\0';
//	chdir(argv[0]);
#endif
#ifdef _WIN32
//	FreeConsole();
#endif
#if defined(WITH_MACOSX_BUNDLE)
        // get the current working directory so when glut trashes it we can restore.
        char pwd[MAXPATHLEN];
        getcwd (pwd,MAXPATHLEN);
#endif
	glutInit(&argc, argv);
#if defined(WITH_MACOSX_BUNDLE)
        // Restore it
        chdir(pwd);
#endif
	Start(1);
	return 0;
}
