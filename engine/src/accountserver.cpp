/*
 * accountserver.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
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

/*
 *  Vegastrike Network Server Main - written by Stephane Vaxelaire <svax@free.fr>
 */

#include <iostream>
#include <stdlib.h>
#include <string>
using std::string;
#include "networking/acctserver.h"
#include "root_generic/lin_time.h"

bool STATIC_VARS_DESTROYED = false;

void VSExit(int code) {
    STATIC_VARS_DESTROYED = true;
    exit(code);
}

string getStarSystemSector(const string &in) {
    return string("");
}

string GetUnitDir(string filename) {
    return string("");
}

char SERVER = 2;
float simulation_atom_var = (float) 1.0 / 10.0;
float audio_atom_var = (float) 1.0 / 18.0;
class NetClient {};
NetClient *Network;

int main(int argc, char **argv) {
    InitTime();
    setNewTime(((double) time(NULL)) - VEGA_EPOCH);
    AccountServer *Server = new AccountServer;

    Server->start();

    delete Server;
    return 0;
}

