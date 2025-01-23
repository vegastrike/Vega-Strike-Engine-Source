/*
 * force_feedback.h
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
#ifndef VEGA_STRIKE_ENGINE_FORCE_FEEDBACK_H
#define VEGA_STRIKE_ENGINE_FORCE_FEEDBACK_H

/*
 *  Force Feedback support by Alexander Rawass <alexannika@users.sourceforge.net>
 */

//#define  HAVE_FORCE_FEEDBACK 1

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#endif //_WIN32

#include <stdio.h>

#if HAVE_FORCE_FEEDBACK

#include <linux/input.h>

#define BITS_PER_LONG (sizeof (long)*8)
#define OFF( x ) ( (x)%BITS_PER_LONG )
#define BIT( x ) ( 1UL<<OFF( x ) )
#define LONG( x ) ( (x)/BITS_PER_LONG )
#define test_bit( bit, array ) ( ( array[LONG( bit )]>>OFF( bit ) )&1 )

#define _is( x ) ( (int) ( 3.2768*( (float) x ) ) )

#endif //HAVE_FORCE_FEEDBACK

#define ALL_EFFECTS 1

enum vseffects {
    eff_speed_damper = 0,     //the higher the speed, the more damped
    eff_ab_wiggle_x, eff_ab_wiggle_y, eff_ab_jerk,     //afterburner on
    eff_ab_off_x, eff_ab_off_y,     //afterburner off
    eff_laser_jerk, eff_laser_vibrate,     //laser fired
#if ALL_EFFECTS
    eff_beam_on, eff_beam_off,     //beam
    eff_missile_jerk, eff_missile_vibrate,     //missile
    eff_hit_jerk, eff_hit_vibrate,     //shield hits
#endif //ALL_EFFECTS
    eff_force     //additional force
};

#if ALL_EFFECTS
#define N_EFFECTS 15
#else //ALL_EFFECTS
#define N_EFFECTS 9
#endif //ALL_EFFECTS

class ForceFeedback {
public:
    ForceFeedback();
    ~ForceFeedback();

    bool haveFF();

    void playDurationEffect(unsigned int eff_nr, bool activate);
    void playShortEffect(unsigned int eff_nr);

    void playHit(float angle, float strength);
    void updateForce(float angle, float strength);
    void updateSpeedEffect(float strength);
    void playAfterburner(bool activate);
    void playLaser();

private:
    bool have_ff;

#if HAVE_FORCE_FEEDBACK
    private:
        void init();

        void init_bogus( int i );
        void playEffect( unsigned int eff_nr );
        void stopEffect( unsigned int eff_nr );

        int device_nr;

        struct ff_effect   effects[N_EFFECTS];
        struct input_event play, stop;
        int    ff_fd;
        unsigned long features[4];
        int    n_effects;   /* Number of effects the device can play at the same time */

        double eff_last_time[N_EFFECTS];

        double min_effect_time;

        bool   is_played[N_EFFECTS];
#endif //HAVE_FORCE_FEEDBACK
};

#endif //VEGA_STRIKE_ENGINE_FORCE_FEEDBACK_H
