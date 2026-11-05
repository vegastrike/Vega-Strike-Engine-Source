/*
 * background.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: dan_w, klaussfreire
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
#ifndef VEGA_STRIKE_ENGINE_GFX_BACKGROUND_H
#define VEGA_STRIKE_ENGINE_GFX_BACKGROUND_H

#include "src/gfxlib_struct.h"

#include <string>
#include <vector>

class SphereMesh;
class Texture;
class StarVlist;

// A picture fixed in the sky of one star system: a galaxy, a nebula, a quasar.
// Declared in the system file as
//   <Backdrop file="backdrops/andromeda.png" direction="0.3 0.8 -0.5" size="12"/>
// It is drawn like the skybox, locked to the camera, so it never gets closer
// or moves as the ship flies, only as the ship turns.
struct BackdropSpec {
    std::string file;                    // image or .ani animation
    Vector direction{0, 0, 1};           // where it is in the sky, from the system's origin
    float size_degrees = 10;             // width across the sky
    float height_degrees = 0;            // height; 0 = keep the image's aspect ratio
    float rotation_degrees = 0;          // turns the picture counterclockwise
    GFXColor color{1, 1, 1, 1};          // multiplies the image
    BLENDFUNC blend_source = SRCALPHA;   // alpha="ONE ONE" adds the image to the sky
    BLENDFUNC blend_destination = INVSRCALPHA;
};

// Per-system settings for what is drawn behind everything else. Every field
// defaults to the behaviour systems had before these settings existed.
struct BackgroundSettings {
    bool draw_skybox = true;             // false: no skybox; clear to the fill color instead
    bool fill_color_set = false;         // false: the fill color is space_background
    GFXColor color{1, 1, 1, 1};          // the skybox tint, or the fill color without a skybox
    bool degamma = false;
    int num_stars = 0;                   // far stars
    float star_brightness_exponent = 1;  // >1 makes most stars faint and a few bright
    std::vector<BackdropSpec> backdrops;
};

class Background {
    bool Enabled;
    bool degamma;
    GFXColor color;
    bool draw_skybox;
    bool fill_color_set;
    StarVlist *stars;
    struct Backdrop {
        BackdropSpec spec;
        Texture *texture;
    };
    std::vector<Backdrop> backdrops;
    void DrawBackdrops();
    Texture *up;
    Texture *left;
    Texture *front;
    Texture *right;
    Texture *back;
    Texture *down;
    SphereMesh *SphereBackground;
public:
    Background(const char *file,
            float spread,
            const std::string &starfilename,
            const BackgroundSettings &settings);
    ~Background();
    void EnableBG(bool);
    void Draw();
    struct BackgroundClone {
        Texture *backups[7];
        void FreeClone();
    };
    BackgroundClone Cache();
};

#endif //VEGA_STRIKE_ENGINE_GFX_BACKGROUND_H
