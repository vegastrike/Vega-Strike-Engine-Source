/*
 * unit_xml.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_UNIT_XML_H
#define VEGA_STRIKE_ENGINE_CMD_UNIT_XML_H

#include "unit_generic.h"

void addShieldMesh(Unit::XML *xml, const char *filename, const float scale, int faction, class Flightgroup *fg);
void addRapidMesh(Unit::XML *xml, const char *filename, const float scale, int faction, class Flightgroup *fg);
void pushMesh(Unit::XML *xml,
        const char *filename,
        const float scale,
        int faction,
        class Flightgroup *fg,
        int startframe /*-1 is random frame*/,
        double texturestarttime);

VSSprite *createVSSprite(const char *file);
bool isVSSpriteLoaded(const VSSprite *);
void deleteVSSprite(VSSprite *);

void cache_ani(string s);

/* ADDED FOR extensible use of unit pretty print and unit loading */

enum UNITLOADTYPE { DEFAULT, NO_MESH };

extern UNITLOADTYPE current_unit_load_mode;

#endif //VEGA_STRIKE_ENGINE_CMD_UNIT_XML_H
