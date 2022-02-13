/**
 * unit_customize.h
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


#ifndef __UNIT_CUSTOMIZE_CPP__
#define __UNIT_CUSTOMIZE_CPP__

#include "images.h"
#include "universe.h"
#include "vegastrike.h"
#include <stdio.h>
#include <stdlib.h>
#include "gfx/cockpit.h"
#include "savegame.h"
#include "config_xml.h"
#include "xml_serializer.h"
#include "audiolib.h"
#include "vs_globals.h"
#include "unit_const_cache.h"
#ifdef _MSC_VER
#define strcasecmp stricmp
#endif
extern int GetModeFromName(const char *);

extern Unit *CreateGameTurret(std::string tur, int faction);

bool Unit::UpgradeSubUnits(const Unit *up,
        int subunitoffset,
        bool touchme,
        bool downgrade,
        int &numave,
        double &percentage) {
    bool bl = this->UpgradeSubUnitsWithFactory(up,
            subunitoffset,
            touchme,
            downgrade,
            numave,
            percentage,
            &CreateGameTurret);
    if (!up->SubUnits.empty()) {
        this->DisableTurretAI();
    }
    return bl;
}

extern char *GetUnitDir(const char *);

#endif

