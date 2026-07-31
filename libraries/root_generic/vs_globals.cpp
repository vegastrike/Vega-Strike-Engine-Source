/*
 * vs_globals.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#include <iostream>
#include <time.h>
#include <stdio.h>
#include <string>
#include <locale>
#include <ostream>

/*
 * Globals
 */
game_data_t g_game;
ForceFeedback *forcefeedback = nullptr;
VegaConfig *vs_config = nullptr;
bool cleanexit = false;
bool run_only_player_starsystem = true;

FILE *fpread = nullptr;

float simulation_atom_var = (float) (1.0 / 10.0);
float audio_atom_var = (float) (1.0 / 18.0);
Mission *mission = nullptr;

double benchmark = -1.0;

std::locale &GetGlobalLocale() {
    const auto kLocaleNames = {"C.UTF-8", "C.UTF8", "C.utf8", "C", "POSIX", "en_US.UTF8", "en_US.UTF8", "en_US.utf8"};
    static std::locale * our_locale = nullptr;
    if (our_locale == nullptr) {
        for (const auto & kLocaleName : kLocaleNames) {
            try {
                our_locale = new std::locale(kLocaleName);
                // If we get to this line, that means success! Found a locale we can use
                break;
            } catch (const std::bad_alloc &) {
                std::cerr << "Locale memory allocation failed. Exiting..." << std::endl;
                exit(-1);
            } catch (const std::runtime_error &) {
                // This locale didn't work. Try the next one
                continue;
            }
        }
        if (our_locale == nullptr) {
            std::cerr << "Unable to find a compatible locale. Exiting..." << std::endl;
            exit(-1);
        } else {
            std::cout << "Found a compatible locale: " << our_locale->name() << std::endl;
        }
    }
    return *our_locale;
}

bool STATIC_VARS_DESTROYED = false;
const char *mission_key = "unit_to_dock_with";

/* for speed test */
int loop_count = 0;
double avg_loop = 0;
int nb_checks = 1;
double last_check = 1;
double cur_check = 1;
