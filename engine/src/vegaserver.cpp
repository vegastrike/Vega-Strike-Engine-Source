/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

/*
 *  Vegastrike Network Server Main - written by Stephane Vaxelaire <svax@free.fr>
 */

#include <iostream>
#include <stdlib.h>
#include "vs_globals.h"
#include "networking/netserver.h"
#include "cmd/script/mission.h"
#include "force_feedback.h"
#include "lin_time.h"
#include "cmd/unit_factory.h"

#include "options.h"
//#ifndef _WIN32
//#include <fenv.h>
//#endif
char SERVER = 1;
Universe *_Universe;
LeakVector<Mission *> active_missions;

vs_options game_options;

float SIMULATION_ATOM = 0.0f;
float AUDIO_ATOM = 0.0f;

void VSExit(int code)
{
    VSServer->closeAllSockets();
    exit(1);
}

// I dont think vegaserver makes use of this but it has to be defined.  
Unit *TheTopLevelUnit = NULL;

int main(int argc, char **argv)
{

    //#ifndef _WIN32
    //feenableexcept(FE_DIVBYZERO|FE_INVALID);//|FE_OVERFLOW|FE_UNDERFLOW)
    //#endif
    VSFileSystem::ChangeToProgramDirectory(argv[0]);

    //Stupid static variable somewhere, so need to initialize it twice.
    InitTime();
    setNewTime(0);
    setNewTime(((double) time(NULL)) - VEGA_EPOCH);
    VSServer = new NetServer;
    //Fake forcefeedback
    forcefeedback = new ForceFeedback();
    VSServer->start(argc, argv);

    delete VSServer;

    return 0;
}

