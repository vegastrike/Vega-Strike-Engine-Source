/*
 * msgcenter.cpp
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
 *  MessageCenter written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

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

#include "cmd/unit_generic.h"
#include "mission.h"
#include "easydom.h"

#include "msgcenter.h"
#include <algorithm>

void MessageCenter::add(string from, string to, string message, double delay) {
    gameMessage msg;

    msg.from = from;
    msg.to = to;
    msg.message = message;

    msg.time = mission->getGametime() + delay;

    messages.push_back(msg);
}

void MessageCenter::clear(const std::vector<std::string> &who, const std::vector<std::string> &whoNOT) {
    if (who.empty() && whoNOT.empty()) {
        messages.clear();
    }
    for (int i = messages.size() - 1; i >= 0; i--) {
        if (std::find(whoNOT.begin(), whoNOT.end(),
                messages[i].to.get()) == whoNOT.end()
                && (who.empty() || std::find(who.begin(), who.end(), messages[i].to.get()) != who.end())) {
            messages.erase(messages.begin() + i);
        }
    }
}

bool MessageCenter::last(unsigned int n,
        gameMessage &m,
        const std::vector<std::string> &who,
        const std::vector<std::string> &whoNOT) {
    if (who.empty() && whoNOT.empty()) {
        int size = messages.size();

        int index = size - 1 - n;
        if (index >= 0) {
            m = messages[index];
            return true;
        } else {
            return false;
        }
    } else {
        int j = 0;
        int i = 0;
        for (i = messages.size() - 1; i >= 0; i--) {
            if (std::find(whoNOT.begin(), whoNOT.end(),
                    messages[i].to.get()) == whoNOT.end()
                    && (who.empty() || std::find(who.begin(), who.end(), messages[i].to.get()) != who.end())) {
                if (j == (int) n) {
                    break;
                }
                j++;
            }
        }
        if (i < 0) {
            return false;
        }
        m = messages[i];
        return true;
    }
}

