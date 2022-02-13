/***************************************************************************
*                          common.cpp  -  description
*                             -------------------
*    begin                : Wed Jun 26 2002
*    copyright            : (C) 2002 by jhunt
*    email                : jhunt@jaja
*    copyright            : (C) 2022 by Stephen G. Tuggy
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include <string>
#include <stdio.h>

#ifndef _WIN32
#include <unistd.h>
#endif

using std::string;
#include "common.h"

#ifndef _WIN32

//Directories to look for data
const char *datadirs[] = {
        ".",
        "../data",
        "../../data",
//Added for MacOS X
        "../Resources/data",
#ifdef DATA_DIR
        DATA_DIR,
#endif
        "/usr/share/local/vegastrike/data",
        "/usr/local/share/vegastrike/data",
        "/usr/local/vegastrike/data",
        "/usr/share/vegastrike/data",
        "/usr/local/games/vegastrike/data",
        "/usr/games/vegastrike/data",
        "/opt/share/vegastrike/data",
        "/usr/share/local/vegastrike/data4.x",
        "/usr/local/share/vegastrike/data4.x",
        "/usr/local/vegastrike/data4.x",
        "/usr/share/vegastrike/data4.x",
        "/usr/local/games/vegastrike/data4.x",
        "/usr/games/vegastrike/data4.x",
        "/opt/share/vegastrike/data4.x",
};

string getdatadir() {
    string datadir;
    char tmppwd[65536];
    if (NULL == getcwd(tmppwd, 32768)) {
        tmppwd[0] = '\0';
    }
    unsigned int i = 0;
    for (; i < (sizeof(datadirs) / sizeof(datadirs[0])); i++) {
        if (chdir(datadirs[i])) {
            continue;
        }
        FILE *tfp = fopen("vegastrike.config", "r");
        if (tfp) {
            fclose(tfp);
            //We have found the data directory
            break;
        }
    }
    if (i >= sizeof(datadirs) / sizeof(datadirs[0])) {
        printf("Unable to find data directory\n");
        for (i = 0; i < (sizeof(datadirs) / sizeof(datadirs[0])); i++) {
            printf("Tried %s\n", datadirs[i]);
        }
        datadir = tmppwd;
        if (chdir(tmppwd)) {
            printf("Unable to set current directory to data directory\n");
        }
    }
        //Set data dir
    else if (datadirs[i][0] != '/') {
        //Was a relative path
        datadir = tmppwd;
        datadir += '/';
        datadir += datadirs[i];
    } else {
        datadir = datadirs[i];
    }
    return datadir;
}

#endif //!_WIN32

