/*
 * unit_util.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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


#include <string>
#include "unit_generic.h"
#include "gfx/cockpit.h"
#include "planet.h"
#include "gfx/animation.h"
#include "config_xml.h"
#include "unit_util.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "pilot.h"
#include "universe.h"

using std::string;
namespace UnitUtil {

void switchFg(Unit *my_unit, string arg) {
    if (!my_unit) {
        return;
    }
    string type = my_unit->name;
    int nr_waves_left = 0;
    int nr_ships = 1;
    string order("default");
    Flightgroup *fg = my_unit->getFlightgroup();
    if (fg) {
        type = fg->type;
        nr_waves_left = fg->nr_waves_left;
        nr_ships = 1;
        fg->Decrement(my_unit);
        order = fg->ainame;
    }
    fg = Flightgroup::newFlightgroup(arg, type, FactionUtil::GetFaction(
            my_unit->faction), order, nr_ships, nr_waves_left, "", "", mission);
    my_unit->SetFg(fg, fg->nr_ships_left - 1);
}

int communicateTo(Unit *my_unit, Unit *other_unit, float mood) {
    if (!my_unit) {
        return 0;
    }
    unsigned char sex = 0;
    Cockpit *tmp;
    if ((tmp = _Universe->isPlayerStarship(my_unit))) {
        if (other_unit) {
            Animation *ani = other_unit->pilot->getCommFace(other_unit, mood, sex);
            if (NULL != ani) {
                tmp->SetCommAnimation(ani, other_unit);
            }
        }
    }
    return sex;
}

bool commAnimation(Unit *my_unit, string anim) {
    if (!my_unit) {
        return false;
    }
    Cockpit *tmp;
    if ((tmp = _Universe->isPlayerStarship(my_unit))) {
        static Hashtable<std::string, Animation, 63> AniHashTable;
        Animation *vid = AniHashTable.Get(anim);
        if (NULL == vid) {
            vid = new Animation(anim.c_str());
            AniHashTable.Put(anim, vid);
        }
        tmp->SetCommAnimation(vid, NULL);
        return true;
    } else {
        return false;
    }
}
}

