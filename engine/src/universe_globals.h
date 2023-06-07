/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
#ifndef VEGA_STRIKE_ENGINE_UNIVERSE_GLOBALS_H
#define VEGA_STRIKE_ENGINE_UNIVERSE_GLOBALS_H

#include <vector>
using std::vector;

class Cockpit;
class StarSystem;

// Globals, in liueu of singletons
extern int _current_cockpit;
extern vector<Cockpit *> _cockpits;

// currently only 1 star system is stored
// TODO: make into a simple variable
extern vector<StarSystem *> _active_star_systems;

// the system currently used by the scripting
// TODO: understand why we can't use active_star_system instead
extern StarSystem *_script_system;

#endif //VEGA_STRIKE_ENGINE_UNIVERSE_GLOBALS_H
