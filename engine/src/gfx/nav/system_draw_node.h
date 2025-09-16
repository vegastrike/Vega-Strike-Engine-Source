/*
 * system_draw_node.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically:
 * hellcatv, ace123, surfdargent, klaussfreire, jacks, pyramid3d
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
#ifndef VEGA_STRIKE_ENGINE_GFX_NAV_SYSTEM_DRAW_NODE_H
#define VEGA_STRIKE_ENGINE_GFX_NAV_SYSTEM_DRAW_NODE_H

#include <string>

#include "gfxlib_struct.h"

#include "gfx/nav/navscreenoccupied.h"

class SystemDrawNode {
    int type;
    float size;
    float x;
    float y;
    unsigned index;
    std::string source;
//Vector of indicies
//std::vector<int> *dest; //let's just hope that the iterator doesn't get killed during the frame, which shouldn't happen.
//std::vector<string> *stringdest; //let's just hope that the iterator doesn't get killed during the frame, which shouldn't happen.
    bool moused;
    char color;
    GFXColor race;
    navscreenoccupied *screenoccupation;

    friend bool operator<(const SystemDrawNode &lhs, const SystemDrawNode &rhs);
    friend bool operator==(const SystemDrawNode &lhs, const SystemDrawNode &rhs); 
public:
    SystemDrawNode(int type,
            float size,
            float x,
            float y,
            std::string source,
            unsigned index,
            navscreenoccupied *so,
            bool moused,
            GFXColor race);

    SystemDrawNode();

    void Draw(bool mouseover = false, bool willclick = false);
    unsigned GetIndex() const;
};

bool operator<(const SystemDrawNode &lhs, const SystemDrawNode &rhs);
bool operator==(const SystemDrawNode &lhs, const SystemDrawNode &rhs); 

#endif //VEGA_STRIKE_ENGINE_GFX_NAV_SYSTEM_DRAW_NODE_H