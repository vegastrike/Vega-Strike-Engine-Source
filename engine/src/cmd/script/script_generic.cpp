/**
 * script_generic.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "xml_support.h"
#include "cmd/unit_generic.h"
#include "mission.h"

void Mission::call_unit_toxml(missionNode *node, int mode, varInst *ovi)
{
    Unit *my_object = getUnitObject(node, mode, ovi);
    if (my_object) {
        string fgid = my_object->getFgID();
    }
}

Unit *Mission::getUnitObject(missionNode *node, int mode, varInst *ovi)
{
    Unit *my_object = NULL;
    if (mode == SCRIPT_RUN) {
        my_object = (Unit *) ovi->object;
    }
    return my_object;
}

