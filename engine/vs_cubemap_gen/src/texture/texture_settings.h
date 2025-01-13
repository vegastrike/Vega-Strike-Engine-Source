/*
 * texture_settings.h
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
#ifndef VEGA_STRIKE_ENGINE_CUBE_MAP_TEXTURE_SETTINGS_H
#define VEGA_STRIKE_ENGINE_CUBE_MAP_TEXTURE_SETTINGS_H

class texture_settings {
    //input params:
    mem_texture &source_tex_;
    float source_gamma_;
    float ideal_ambient_light_;
    float correction_power_;
    //computed params:
    float red_scaling;
    float grn_scaling;
    float blu_scaling;
public:
    texture_settings(
            mem_texture &source_tex, float source_gamma        //-e.g. 0.50
            , float ideal_ambient_light //-e.g. 0.25
            , float correction_power    //0.0 to 1.0
    )
            : source_tex_(source_tex),
            source_gamma_(source_gamma),
            ideal_ambient_light_(ideal_ambient_light),
            correction_power_(correction_power) {
    }

    void init();
};

#endif //VEGA_STRIKE_ENGINE_CUBE_MAP_TEXTURE_SETTINGS_H
