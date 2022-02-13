/*
 * msgcenter.h
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) Alexander Rawass
 * Copyright (C) 2020 Stephen G. Tuggy, pyramid3d, and other Vega Strike contributors
 * Copyright (C) 2021-2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
 *  MessageCenter written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#ifndef _MSGCENTER_H_

#define _MSGCENTER_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <expat.h>
#include "xml_support.h"

#include "vegastrike.h"

#include "mission.h"
//#include "easydom.h"

//#include "vs_globals.h"
//#include "vegastrike.h"

#include "SharedPool.h"

class gameMessage {
public:
    StringPool::Reference from, to, message;
    double time;
};

class MessageCenter {
public:
    bool last(unsigned int n, gameMessage &m,
            const std::vector<std::string> &who = std::vector<std::string>(), const std::vector<std::string> &whoNOT =
    std::vector<std::string>());
    void add(std::string from, std::string to, std::string message, double delay = 0.0);
    void clear(
            const std::vector<std::string> &who = std::vector<std::string>(), const std::vector<std::string> &whoNOT =
    std::vector<std::string>());
    std::vector<gameMessage> messages;
};

#endif //_MSGCENTER_H_

