/*
 * unit_customize.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_UNIT_CUSTOMIZE_H
#define VEGA_STRIKE_ENGINE_CMD_UNIT_CUSTOMIZE_H

#include "cmd/images.h"
#include "src/universe.h"
#include "src/vegastrike.h"
#include <stdio.h>
#include <stdlib.h>
#include "gfx/cockpit.h"
#include "vegadisk/savegame.h"
#include "src/config_xml.h"
#include "root_generic/xml_serializer.h"
#include "src/audiolib.h"
#include "root_generic/vs_globals.h"
#include "cmd/unit_const_cache.h"
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

#endif //VEGA_STRIKE_ENGINE_CMD_UNIT_CUSTOMIZE_H
