/*
 * vs_globals.cpp
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

#include "root_generic/vs_globals.h"
#include <time.h>
#include <stdio.h>
#include "src/vega_cast_utils.h"

/*
 * Globals
 */
game_data_t g_game;
ForceFeedback *forcefeedback = nullptr;
VegaConfig *vs_config = nullptr;
bool cleanexit = false;
bool run_only_player_starsystem = true;

FILE *fpread = nullptr;

float     simulation_atom_var = 1.0 / 10.0;
float     audio_atom_var      = 1.0 / 18.0;
Mission * mission             = nullptr;

double benchmark = -1.0;
bool STATIC_VARS_DESTROYED = false;
const char *mission_key = "unit_to_dock_with";

/* for speed test */
int loop_count = 0;
double avg_loop = 0;
int nb_checks = 1;
double last_check = 1;
double cur_check = 1;

const std::string kLocaleName = "C";
std::locale our_numeric_locale = std::locale().combine<std::numpunct<char>>(std::locale(kLocaleName));

