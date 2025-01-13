/*
 * file_io.h
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
#ifndef VEGA_STRIKE_ENGINE_CUBE_MAP_FILE_IO_H
#define VEGA_STRIKE_ENGINE_CUBE_MAP_FILE_IO_H

mem_texture &load_texture_png(std::string filename);
mem_texture &load_texture_set_png
        (
                std::string fn_front,
                std::string fn_back,
                std::string fn_left,
                std::string fn_right,
                std::string fn_up,
                std::string fn_down
        );

void save_texture_png(...);
void save_texture_set_png(...);
void nvcompress_png(...);
void nvcompress_set_png(...);

void pack_dds_mip(...);
void pack_dds_all(...);

#endif //VEGA_STRIKE_ENGINE_CUBE_MAP_FILE_IO_H
