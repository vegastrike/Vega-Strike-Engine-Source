/*
 * central.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: David Ranger
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#if defined(_WIN32) && _MSC_VER > 1'300
#    define __restrict
#endif
#include "mission/include/central.h"
#ifdef _WIN32
#    include <direct.h>
#    include <process.h>
#    include <windows.h>
#else
#    include <stdio.h>
#    include <unistd.h>

#    include <sys/dir.h>
#endif
glob_t             *MISSIONS;
struct mission_data DATA;

// Primary initialization function. Sets everything up and takes care of the program
char *Start(int run_vegastrike) {
    InitGraphics();

    DATA.name        = NewString("");
    DATA.author      = NewString("");
    DATA.description = NewString("");
    DATA.briefing    = NewString("");
    DATA.path        = NewString("");
    DATA.numplayers  = NewString("");

    ShowMain(1);
    gtk_main();
    return DATA.path;
}

void RunMission(void) {
    if (DATA.path[0] == '\0') {
        cout << "No mission selected\n";
        return;
    }
    cout << "Starting " << MISSION_PROGRAM << " with mission " << DATA.path << endl;
#ifdef _WIN32
    char execname[2'048];
    char mypath[1'500];
    _getcwd(mypath, 1'499);
    sprintf(execname, "%s\\Vegastrike", mypath);
    _spawnl(P_NOWAIT, execname, execname, (string("\"") + string(DATA.path) + string("\"")).c_str(),
            (string("-m") + DATA.numplayers).c_str(), NULL);
#else
    execlp("./vegastrike", "./vegastrike", (string("\"") + string(DATA.path) + string("\"")).c_str(),
           (string("-m") + DATA.numplayers).c_str(), NULL);
#endif
}
