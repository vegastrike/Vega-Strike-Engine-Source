/*
 * ani_texture_utils.cpp
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


#include "gfx/ani_texture_utils.h"

#include <cctype>
#include <cstring>
#include <sstream>

/// Case-insensitive token comparison: returns true if `token` equals `word`.
static bool tokenIs(const char *token, const char *word) {
    for (; *word != '\0'; ++word, ++token) {
        if (tolower(static_cast<unsigned char>(*token)) != *word) {
            return false;
        }
    }
    return *token == '\0';
}

bool IsNoAlphamapToken(const char *token) {
    return token[0] == '\0' || strcmp(token, "-") == 0 || tokenIs(token, "true") || tokenIs(token, "false");
}

int ParseAniLine(const char *line, char *file_out, size_t file_size, char *alp_out, size_t alp_size, char *opt_out,
                 size_t opt_size) {
    std::istringstream iss(line);
    std::string        file_str, alp_str, opt_str;
    int                num_gets = 0;
    if (iss >> file_str) {
        ++num_gets;
        if (iss >> alp_str) {
            ++num_gets;
            std::getline(iss, opt_str);
            auto first_non_space = opt_str.find_first_not_of(" \t\r\n");
            if (first_non_space != std::string::npos) {
                opt_str = opt_str.substr(first_non_space);
                ++num_gets;
            }
        }
    }
    // Safely copy parsed strings into fixed-size buffers with bounds checking
    if (file_out && file_size > 0) {
        strncpy(file_out, file_str.c_str(), file_size - 1);
        file_out[file_size - 1] = '\0';
    }
    if (alp_out && alp_size > 0) {
        strncpy(alp_out, alp_str.c_str(), alp_size - 1);
        alp_out[alp_size - 1] = '\0';
    }
    if (opt_out && opt_size > 0) {
        strncpy(opt_out, opt_str.c_str(), opt_size - 1);
        opt_out[opt_size - 1] = '\0';
    }
    return num_gets;
}
