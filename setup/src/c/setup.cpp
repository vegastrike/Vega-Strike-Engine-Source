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
#include <vector>
#include <string>
using std::string;
using std::vector;
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
	getcwd (origpath,65535);
	origpath[65535]=0;
        int i;
	for (i=strlen(argv[0]);argv[0][i]!='\\'&&argv[0][i]!='/'&&i>=0;i--) {
	}
	argv[0][i+1]='\0';
	if (i>=0) {
		chdir(argv[0]);
	}
#endif
	
	{	
		vector<string>	data_paths;
#ifdef DATA_DIR
		data_paths.push_back( DATA_DIR);
#endif
		data_paths.push_back( origpath);
		data_paths.push_back( string(origpath)+"/..");
		data_paths.push_back( string(origpath)+"/../data4.x");
		data_paths.push_back( string(origpath)+"/../../data4.x");
		data_paths.push_back( string(origpath)+"/data4.x");
		data_paths.push_back( string(origpath)+"/data");
		data_paths.push_back( string(origpath)+"/../data");
		getcwd (origpath,65535);
		origpath[65535]=0;
		data_paths.push_back( ".");
		data_paths.push_back( "..");
		data_paths.push_back( "../data4.x");
		data_paths.push_back( "../../data4.x");
		data_paths.push_back( "../data");
		data_paths.push_back( "../../data");
		data_paths.push_back( "../Resources");
		data_paths.push_back( "../Resources/data");
		data_paths.push_back( "/usr/share/local/vegastrike/data");
		data_paths.push_back( "/usr/local/share/vegastrike/data");
		data_paths.push_back( "/usr/local/vegastrike/data");
		data_paths.push_back( "/usr/share/vegastrike/data");
		data_paths.push_back( "/usr/local/games/vegastrike/data");
		data_paths.push_back( "/usr/games/vegastrike/data");
		data_paths.push_back( "/opt/share/vegastrike/data");
		data_paths.push_back( "../Resources/data4.x");
		data_paths.push_back( "/usr/share/local/vegastrike/data4.x");
		data_paths.push_back( "/usr/local/share/vegastrike/data4.x");
		data_paths.push_back( "/usr/local/vegastrike/data4.x");
		data_paths.push_back( "/usr/share/vegastrike/data4.x");
		data_paths.push_back( "/usr/local/games/vegastrike/data4.x");
		data_paths.push_back( "/usr/games/vegastrike/data4.x");
		data_paths.push_back( "/opt/share/vegastrike/data4.x");
		
		// Win32 data should be "."
		char tmppath[16384];
		for( vector<string>::iterator vsit=data_paths.begin(); vsit!=data_paths.end(); vsit++)
		{
			// Test if the dir exist and contains config_file
			chdir(origpath);
			chdir((*vsit).c_str());
			FILE *setupcfg = fopen("setup.config","r");
			if (!setupcfg)
				continue;
			fclose(setupcfg);
			setupcfg = fopen("Version.txt","r");
			if (!setupcfg)
				continue;
			getcwd (origpath,65535);
			origpath[65535]=0;
			printf("Found data in %s\n",origpath);
			break;
		}
	}
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
