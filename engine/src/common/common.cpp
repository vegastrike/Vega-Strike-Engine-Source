/*
 * common.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: jhunt
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <string>
#include <stdio.h>

#ifndef _WIN32
#include <unistd.h>
#endif

using std::string;
#include "common/common.h"

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

