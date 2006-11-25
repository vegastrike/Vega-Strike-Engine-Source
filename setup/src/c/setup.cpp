/***************************************************************************
 *                           setup.cpp  -  description
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

#include "../include/central.h"
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/dir.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

#endif
#include <string>
using std::string;
char origpath[65536];
#if defined(_WINDOWS)&&defined(_WIN32)
typedef char FileNameCharType [65536];
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd) {
	char*argv0= new char[65535];
	char **argv=&argv0;
	int argc=0;
	strcpy(argv0,origpath);
	GetModuleFileName(NULL, argv0, 65534);
#else
int main(int argc, char *argv[]) {
#endif
#if 1
//def _WIN32
        int i;
	for (i=strlen(argv[0]);argv[0][i]!='\\'&&argv[0][i]!='/'&&i>=0;i--) {
	}
	argv[0][i+1]='\0';
	chdir(argv[0]);
#endif
#ifdef _WIN32
	chdir ("..");//gotta check outside bin dir
#endif	
	getcwd (origpath,65535);
	origpath[65535]=0;
	
#ifndef _WIN32
	struct passwd *pwent;
	pwent = getpwuid (getuid());
        string HOMESUBDIR=".vegastrike";
	FILE *version=fopen("Version.txt","r");
	if (!version)
		version=fopen("../Version.txt","r");
	if (version) {
		std::string hsd="";
		int c;
		while ((c=fgetc(version))!=EOF) {
			if (isspace(c))
				break;
			hsd+=(char)c;
		}
		fclose(version);
		if (hsd.length()) {
			HOMESUBDIR=hsd;
			//fprintf (STD_OUT,"Using %s as the home directory\n",hsd.c_str());
		}			
	}
	chdir (pwent->pw_dir);
	chdir (HOMESUBDIR.c_str());
#endif
#ifdef GTK
#endif    //GTK
	Start(&argc,&argv);
#if defined(_WINDOWS)&&defined(_WIN32)
	delete []argv0;
#endif
	return 0;
}
