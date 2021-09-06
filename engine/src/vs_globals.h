/**
 * vs_globals.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020-2021 pyramid3d, Stephen G. Tuggy, and
 * other Vega Strike contributors
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


#ifndef __VS_GLOBALS_H_
#define __VS_GLOBALS_H_
#include <vector>

#include "command.h"

class Universe;

extern commandI *CommandInterpretor;
extern Universe *_Universe;


#ifdef WIN32
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

extern void cleanup();
extern void VSExit( int code );
extern bool   STATIC_VARS_DESTROYED;
extern int    loop_count;
extern double avg_loop;
extern int    nb_checks;
extern double last_check;
extern double cur_check;
extern bool   cleanexit;
extern bool   run_only_player_starsystem;

/* All global data is stored in a variable of this type */

// We will be removing all of these and using vs_options instead

typedef struct
{
    char  use_textures;
    char  use_animations;
    char  use_videos;
    char  use_sprites;
    char  use_logos;
    char  use_ship_textures;
    char  use_planet_textures;
    int   audio_frequency_mode;   //0==11025/8 1==22050/8 2==44100/8  3==11025/16 4==22050/16 5==44100/16
    int   sound_enabled;
    int   sound_volume;
    int   music_volume;
    int   max_sound_sources;
    int   warning_level;
    int   capture_mouse;
    float MouseSensitivityX;
    float MouseSensitivityY;
    float detaillevel;     /*how much the LOD calculation gets multiplied by...higher="bigger" */
    int   y_resolution;
    int   x_resolution;
    float znear;
    float zfar;
    float fov;
    float aspect;
    float difficulty;
    char  vsdebug;
} game_data_t;

extern game_data_t g_game;
class Music;
extern Music *muzak;
extern int    muzak_count;
class VegaConfig;
extern VegaConfig *vs_config;

class Mission;
extern Mission    *mission;
template < class MyType >
class LeakVector
{
    std::vector< MyType > *active_missions;
public:
    bool empty() const
    {
        return active_missions->empty();
    }
    void push_back( MyType mis )
    {
        active_missions->push_back( mis );
    }
    MyType back()
    {
        return active_missions->back();
    }
    LeakVector()
    {
        active_missions = new std::vector< MyType > ();
    }

    unsigned int size() const
    {
        return (active_missions)->size();
    }
    MyType operator[]( unsigned int i )
    {
        // stephengtuggy 2020-10-17: Enforce bounds checking
        return active_missions->at(i);
    }
    ~LeakVector()
    {
        /* DO NOTHING OR DIE INTENTIONAL LEAK We need this data after Exit*/
    }
    std::vector< MyType > * Get()
    {
        return active_missions;
    }
};

extern LeakVector< Mission* >active_missions;
class ForceFeedback;
extern ForceFeedback *forcefeedback;

extern double benchmark;

#endif

