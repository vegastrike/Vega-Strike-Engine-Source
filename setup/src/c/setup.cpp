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
#include <sys/stat.h>
#include <sys/types.h>

#endif
#include <vector>
#include <string>
using std::string;
using std::vector;
char origpath[65536];

static void changeToProgramDirectory(char *argv0) {
    int ret = -1; /* Should it use argv[0] directly? */
    char *program = argv0;
#ifndef _WIN32
    char buf[65536];
    {
	char linkname[128]; /* /proc/<pid>/exe */
	linkname[0]='\0';
	pid_t pid;
	
	/* Get our PID and build the name of the link in /proc */
	pid = getpid();
	
	sprintf(linkname, "/proc/%d/exe", pid);
	ret = readlink(linkname, buf, 65535);
	if (ret <= 0) {
		sprintf(linkname, "/proc/%d/file", pid);
		ret = readlink(linkname, buf, 65535);
	}
	if (ret <= 0) {
		ret = readlink(program, buf, 65535);
	}
	if (ret > 0) {
		buf[ret]='\0';
		/* Ensure proper NUL termination */
		program = buf;
	}
    }
#endif

    char *parentdir;
    int pathlen=strlen(program);
    parentdir=new char[pathlen+1];
    char *c;
    strncpy ( parentdir, program, pathlen+1 );
    c = (char*) parentdir;
    while (*c != '\0')     /* go to end */
      c++;
    
    while ((*c != '/')&&(*c != '\\')&&c>parentdir)      /* back up to parent */
      c--;
    
    *c = '\0';             /* cut off last part (binary name) */
    if (strlen (parentdir)>0) {
      chdir (parentdir);/* chdir to the binary app's parent */
    }
    delete []parentdir;
}


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

	if (argc>1) {
		if (strcmp(argv[1], "--target")==0 && argc>2) {
			chdir(argv[2]);
		} else {
			fprintf(stderr,"Usage: vssetup [--target DATADIR]\n");
			return 1;
		}
	}
	getcwd (origpath,65535);
	origpath[65535]=0;
	
	changeToProgramDirectory(argv[0]);
	
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
		data_paths.push_back( string(origpath)+"/../Resources");
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
		data_paths.push_back( "../Resources/data4.x");
/*
		data_paths.push_back( "/usr/share/local/vegastrike/data");
		data_paths.push_back( "/usr/local/share/vegastrike/data");
		data_paths.push_back( "/usr/local/vegastrike/data");
		data_paths.push_back( "/usr/share/vegastrike/data");
		data_paths.push_back( "/usr/local/games/vegastrike/data");
		data_paths.push_back( "/usr/games/vegastrike/data");
		data_paths.push_back( "/opt/share/vegastrike/data");
		data_paths.push_back( "/usr/share/local/vegastrike/data4.x");
		data_paths.push_back( "/usr/local/share/vegastrike/data4.x");
		data_paths.push_back( "/usr/local/vegastrike/data4.x");
		data_paths.push_back( "/usr/share/vegastrike/data4.x");
		data_paths.push_back( "/usr/local/games/vegastrike/data4.x");
		data_paths.push_back( "/usr/games/vegastrike/data4.x");
		data_paths.push_back( "/opt/share/vegastrike/data4.x");
*/		
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
	
	string HOMESUBDIR;
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
	if (HOMESUBDIR.empty()) {
		fprintf(stderr,"Error: Failed to find Version.txt anywhere.\n");
		return 1;
	}
	chdir (pwent->pw_dir);

	mkdir(HOMESUBDIR.c_str()
#ifndef _WIN32
		, 0755
#endif
		);

#endif
	chdir (HOMESUBDIR.c_str());
	
	Start(&argc,&argv);
#if defined(_WINDOWS)&&defined(_WIN32)
	delete []argv0;
#endif
	return 0;
}
