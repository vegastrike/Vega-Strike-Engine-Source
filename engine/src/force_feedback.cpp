/*
 * force_feedback.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; Alexander Rawass specifically
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

/*
 *  Force Feedback support by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include "force_feedback.h"

#include "vegastrike.h"
#include "vs_logging.h"
#include "vs_globals.h"

#include "config_xml.h"
#include "cmd/script/mission.h"
#include "options.h"

#define FF_DOIT 1

ForceFeedback::ForceFeedback() {
#if HAVE_FORCE_FEEDBACK
    init();
#else
    have_ff = false;
    VS_LOG(info, "Force feedback support disabled when compiled");
#endif
}

ForceFeedback::~ForceFeedback() {
    if (!have_ff) {
        return;
    }
#if HAVE_FORCE_FEEDBACK
    if (ff_fd != -1) {
        for (int i = 0; i < N_EFFECTS; i++) {
            stopEffect( i );
        }
        close( ff_fd );
    }
#endif
}

bool ForceFeedback::haveFF() {
    return have_ff;
}

void ForceFeedback::updateForce(float angle, float strength) {
    VS_LOG(info, (boost::format("update force %1% degrees %2%") % angle % strength));
}

void ForceFeedback::updateSpeedEffect(float strength) {
    VS_LOG(info, (boost::format("speed effect %1%") % strength));
}

void ForceFeedback::playHit(float angle, float strength) {
    VS_LOG(info, (boost::format("shield hit %1% degrees %2%") % angle % strength));
}

void ForceFeedback::playAfterburner(bool activate) {
    if (!have_ff) {
        return;
    }
#if HAVE_FORCE_FEEDBACK
    double nowtime = mission->getGametime();

    int    eff_nr  = eff_ab_jerk;
    if (activate == true) {
        eff_last_time[eff_nr] = nowtime;
    }
    if (activate == true && is_played[eff_nr] == false) {
        VS_LOG(info, "starting ab");
        playEffect( eff_ab_wiggle_x );
        playEffect( eff_ab_wiggle_y );
        playEffect( eff_ab_jerk );

        is_played[eff_nr] = true;
    } else if (activate == false && is_played[eff_nr] == true) {
        if (nowtime > eff_last_time[eff_nr]+min_effect_time) {
            VS_LOG(info, "stopped ab");
            stopEffect( eff_ab_wiggle_x );
            stopEffect( eff_ab_wiggle_y );
            stopEffect( eff_ab_jerk );

            is_played[eff_nr]     = false;
            eff_last_time[eff_nr] = nowtime;
        }
    }
#endif
}

void ForceFeedback::playLaser() {
    if (!have_ff) {
        return;
    }
#if HAVE_FORCE_FEEDBACK
    int    eff_nr  = eff_laser_jerk;

    double nowtime = mission->getGametime();
    if (nowtime < eff_last_time[eff_nr]+min_effect_time) {
        //to make sure that effects aren't done too fast after another
        return;
    }
    playEffect( eff_laser_jerk );
    playEffect( eff_laser_vibrate );

    eff_last_time[eff_nr] = nowtime;
#endif
}

void ForceFeedback::playDurationEffect(unsigned int eff_nr, bool activate) {
    if (!have_ff) {
        return;
    }
    return;

#if HAVE_FORCE_FEEDBACK
    if (activate == true && is_played[eff_nr] == false) {
        playEffect( eff_nr );
        is_played[eff_nr] = true;
    } else if (activate == false && is_played[eff_nr] == true) {
        stopEffect( eff_nr );
        is_played[eff_nr] = false;
    }
#endif
}

void ForceFeedback::playShortEffect(unsigned int eff_nr) {
    if (!have_ff) {
        return;
    }
    return;

#if HAVE_FORCE_FEEDBACK
    double nowtime = mission->getGametime();
    if (nowtime < eff_last_time[eff_nr]+min_effect_time) {
        //to make sure that effects aren't done too fast after another
        return;
    }
    playEffect( eff_nr );

    eff_last_time[eff_nr] = nowtime;
#endif
}

#if HAVE_FORCE_FEEDBACK

void ForceFeedback::playEffect( unsigned int eff_nr )
{
#if FF_DOIT
    play.type  = EV_FF;
    play.code  = effects[eff_nr].id;
    play.value = 1;
    if (write( ff_fd, (const void*) &play, sizeof (play) ) == -1) {
        perror( "ff: Play effect" );
        have_ff = false;
        close( ff_fd );
        return;
    }
#endif

    VS_LOG(info, (boost::format("played effect nr %1%") % eff_nr));
}

void ForceFeedback::stopEffect( unsigned int eff_nr )
{
#if FF_DOIT
    stop.type  = EV_FF;
    stop.code  = effects[eff_nr].id;
    stop.value = 0;
    if (write( ff_fd, (const void*) &stop, sizeof (stop) ) == -1) {
        perror( "ff: stop effect" );
        have_ff = false;
        close( ff_fd );
        return;
    }
#endif

    VS_LOG(info, (boost::format("stopped effect nr %1%") % eff_nr));
}

void ForceFeedback::init()
{
    if (!vs_options::instance().force_feedback) {
        VS_LOG(info, "force feedback disabled in config file");
        return;
    }
    char devname[200];
    sprintf( devname, "/dev/input/event%d", vs_options::instance().ff_device );

    ff_fd = open( devname, O_RDWR );
    if (ff_fd == -1) {
        perror( "error while opening force feedback device" );
        have_ff = false;
        return;
    }
    VS_LOG(info, (boost::format("Device %1% opened") % devname));
    /* Query device */
    if (ioctl( ff_fd, EVIOCGBIT( EV_FF, sizeof (unsigned long)*4 ), features ) == -1) {
        perror( "ff:Ioctl query" );
        have_ff = false;
        close( ff_fd );
        return;
    }
    VS_LOG(info, "Axes query: ");
    if ( test_bit( ABS_X, features ) ) {
        VS_LOG(info, "Axis X ");
    }
    if ( test_bit( ABS_Y, features ) ) {
        VS_LOG(info, "Axis Y ");
    }
    if ( test_bit( ABS_WHEEL, features ) ) {
        VS_LOG(info, "Wheel ");
    }
    VS_LOG(info, "\nEffects: ");
    if ( test_bit( FF_CONSTANT, features ) ) {
        VS_LOG(info, "Constant ");
    }
    if ( test_bit( FF_PERIODIC, features ) ) {
        VS_LOG(info, "Periodic ");
    }
    if ( test_bit( FF_SPRING, features ) ) {
        VS_LOG(info, "Spring ");
    }
    if ( test_bit( FF_FRICTION, features ) ) {
        VS_LOG(info, "Friction ");
    }
    if ( test_bit( FF_RUMBLE, features ) ) {
        VS_LOG(info, "Rumble ");
    }
    VS_LOG(info, "\nNumber of simultaneous effects: ");
    if (ioctl( ff_fd, EVIOCGEFFECTS, &n_effects ) == -1) {
        perror( "Ioctl number of effects" );
        have_ff = false;
        close( ff_fd );
        return;
    }
    VS_LOG(info, (boost::format("nr_effects: %1%") % n_effects));
    if (n_effects < N_EFFECTS) {
        VS_LOG(info, "not enough effects in device - ff disabled");
        close( ff_fd );
        have_ff = false;
        return;
    }

#if 0
    effects[1].type = FF_CONSTANT;
    effects[1].id   = -1;
    effects[1].u.constant.level = 0x2000;       /* Strength : 25 % */
    effects[1].direction = 0x6000;      /* 135 degrees */
    effects[1].u.constant.envelope.attack_length = 0x100;
    effects[1].u.constant.envelope.attack_level  = 0;
    effects[1].u.constant.envelope.fade_length   = 0x100;
    effects[1].u.constant.envelope.fade_level    = 0;
    effects[1].trigger.button   = 0;
    effects[1].trigger.interval = 0;
    effects[1].replay.length    = 2000;     /* 2 seconds */
    effects[1].replay.delay     = 0;

    /* download a periodic sinusoidal effect */
    effects[0].type = FF_PERIODIC;
    effects[0].id   = -1;
    effects[0].u.periodic.waveform  = FF_SINE;
    effects[0].u.periodic.period    = 0.4*0x100;        /* 0.1 second */
    effects[0].u.periodic.magnitude = 0x6000;           /* 0.5 * Maximum magnitude */
    effects[0].u.periodic.offset    = 0;
    effects[0].u.periodic.phase     = 0;
    effects[0].direction = 0x000;       /* Along X axis */
    effects[0].u.periodic.envelope.attack_length = 0x100;
    effects[0].u.periodic.envelope.attack_level  = 0;
    effects[0].u.periodic.envelope.fade_length   = 0x100;
    effects[0].u.periodic.envelope.fade_level    = 0;
    effects[0].trigger.button   = 0;
    effects[0].trigger.interval = 0;
    effects[0].replay.length    = 500;     /* 1 seconds */
    effects[0].replay.delay     = 0;

    /* download an condition spring effect */
    effects[2].type = FF_SPRING;
    effects[2].id   = -1;
    effects[2].u.condition[0].right_saturation = 0x7fff;
    effects[2].u.condition[0].left_saturation  = 0x7fff;
    effects[2].u.condition[0].right_coeff = 0x2000;
    effects[2].u.condition[0].left_coeff  = 0x2000;
    effects[2].u.condition[0].deadband    = 0x0;
    effects[2].u.condition[0].center = 0x0;
    effects[2].u.condition[1]   = effects[2].u.condition[0];
    effects[2].trigger.button   = 0;
    effects[2].trigger.interval = 0;
    effects[2].replay.length    = 0xFFFF;
    effects[2].replay.delay     = 0;

    /* download an condition damper effect */
    effects[3].type = FF_DAMPER;
    effects[3].id   = -1;
    effects[3].u.condition[0].right_saturation = 0x7fff;
    effects[3].u.condition[0].left_saturation  = 0x7fff;
    effects[3].u.condition[0].right_coeff = 0x2000;
    effects[3].u.condition[0].left_coeff  = 0x2000;
    effects[3].u.condition[0].deadband    = 0x0;
    effects[3].u.condition[0].center = 0x0;
    effects[3].u.condition[1]      = effects[3].u.condition[0];
    effects[3].trigger.button      = 0;
    effects[3].trigger.interval    = 0;
    effects[3].replay.length       = 2000;     /* 2 seconds */
    effects[3].replay.delay        = 0;
#endif

    effects[eff_speed_damper].type = FF_DAMPER;
    effects[eff_speed_damper].id   = -1;
    effects[eff_speed_damper].u.condition[0].right_saturation = 0x7fff;
    effects[eff_speed_damper].u.condition[0].left_saturation  = 0x7fff;
    effects[eff_speed_damper].u.condition[0].right_coeff = _is( 3716 );
    effects[eff_speed_damper].u.condition[0].left_coeff  = _is( 3716 );
    effects[eff_speed_damper].u.condition[0].deadband    = 0x0;
    effects[eff_speed_damper].u.condition[0].center = 0x0;
    effects[eff_speed_damper].u.condition[1]   = effects[eff_speed_damper].u.condition[0];
    effects[eff_speed_damper].trigger.button   = 0;
    effects[eff_speed_damper].trigger.interval = 0;
    effects[eff_speed_damper].replay.length    = 0xffff;
    effects[eff_speed_damper].replay.delay     = 0;

    effects[eff_ab_wiggle_x].type = FF_PERIODIC;
    effects[eff_ab_wiggle_x].id   = -1;
    effects[eff_ab_wiggle_x].u.periodic.waveform  = FF_SINE;
    effects[eff_ab_wiggle_x].u.periodic.period    = 0.5*0x100;
    effects[eff_ab_wiggle_x].u.periodic.magnitude = _is( 2023 );
    effects[eff_ab_wiggle_x].u.periodic.offset    = 0;
    effects[eff_ab_wiggle_x].u.periodic.phase     = 0;
    effects[eff_ab_wiggle_x].direction = 0x4000;
    effects[eff_ab_wiggle_x].u.periodic.envelope.attack_length = 2.31*0x100;
    effects[eff_ab_wiggle_x].u.periodic.envelope.attack_level  = 0x7fff;
    effects[eff_ab_wiggle_x].u.periodic.envelope.fade_length   = 0x000;
    effects[eff_ab_wiggle_x].u.periodic.envelope.fade_level    = 0;
    effects[eff_ab_wiggle_x].trigger.button   = 0;
    effects[eff_ab_wiggle_x].trigger.interval = 0;
    effects[eff_ab_wiggle_x].replay.length    = 0xffff;
    effects[eff_ab_wiggle_x].replay.delay     = 0;

    effects[eff_ab_wiggle_y].type = FF_PERIODIC;
    effects[eff_ab_wiggle_y].id   = -1;
    effects[eff_ab_wiggle_y].u.periodic.waveform  = FF_SINE;
    effects[eff_ab_wiggle_y].u.periodic.period    = 0.5*0x100;
    effects[eff_ab_wiggle_y].u.periodic.magnitude = _is( 2023 );
    effects[eff_ab_wiggle_y].u.periodic.offset    = 0;
    effects[eff_ab_wiggle_y].u.periodic.phase     = 0x4000;
    effects[eff_ab_wiggle_y].direction = 0x0000;
    effects[eff_ab_wiggle_y].u.periodic.envelope.attack_length = 2.31*0x100;
    effects[eff_ab_wiggle_y].u.periodic.envelope.attack_level  = 0x7fff;
    effects[eff_ab_wiggle_y].u.periodic.envelope.fade_length   = 0x000;
    effects[eff_ab_wiggle_y].u.periodic.envelope.fade_level    = 0;
    effects[eff_ab_wiggle_y].trigger.button   = 0;
    effects[eff_ab_wiggle_y].trigger.interval = 0;
    effects[eff_ab_wiggle_y].replay.length    = 0xffff;
    effects[eff_ab_wiggle_y].replay.delay     = 0;

    effects[eff_ab_jerk].type = FF_CONSTANT;
    effects[eff_ab_jerk].id   = -1;
    effects[eff_ab_jerk].u.constant.level = 0x0;
    effects[eff_ab_jerk].direction = 0x0000;
    effects[eff_ab_jerk].u.constant.envelope.attack_length = 0.3*0x100;
    effects[eff_ab_jerk].u.constant.envelope.attack_level  = 0x7fff;
    effects[eff_ab_jerk].u.constant.envelope.fade_length   = 0;
    effects[eff_ab_jerk].u.constant.envelope.fade_level    = 0;
    effects[eff_ab_jerk].trigger.button   = 0;
    effects[eff_ab_jerk].trigger.interval = 0;
    effects[eff_ab_jerk].replay.length    = 0.3*0x100;
    effects[eff_ab_jerk].replay.delay     = 0;

    init_bogus( eff_ab_off_x );
    init_bogus( eff_ab_off_y );
#if 0
    effects[eff_laser_jerk].type = FF_CONSTANT;
    effects[eff_laser_jerk].id   = -1;
    effects[eff_laser_jerk].u.constant.level = 0x2000;
    effects[eff_laser_jerk].direction = 0x0000;
    effects[eff_laser_jerk].u.constant.envelope.attack_length = 0x000;
    effects[eff_laser_jerk].u.constant.envelope.attack_level  = 0;
    effects[eff_laser_jerk].u.constant.envelope.fade_length   = 0x000;
    effects[eff_laser_jerk].u.constant.envelope.fade_level    = 0;
    effects[eff_laser_jerk].trigger.button   = 0;
    effects[eff_laser_jerk].trigger.interval = 0;
    effects[eff_laser_jerk].replay.length    = 0.2*0x100;     /* 2 seconds */
    effects[eff_laser_jerk].replay.delay     = 0;
#endif

    effects[eff_laser_jerk].type = FF_PERIODIC;
    effects[eff_laser_jerk].id   = -1;
    effects[eff_laser_jerk].u.periodic.waveform  = FF_SQUARE;
    effects[eff_laser_jerk].u.periodic.period    = 0.5*0x100;
    effects[eff_laser_jerk].u.periodic.magnitude = _is( 4573 );
    effects[eff_laser_jerk].u.periodic.offset    = 0;
    effects[eff_laser_jerk].u.periodic.phase     = 0;
    effects[eff_laser_jerk].direction = 0x0;
    effects[eff_laser_jerk].u.periodic.envelope.attack_length = 0.043*0x100;
    effects[eff_laser_jerk].u.periodic.envelope.attack_level  = 0x7fff;
    effects[eff_laser_jerk].u.periodic.envelope.fade_length   = 0x000;
    effects[eff_laser_jerk].u.periodic.envelope.fade_level    = 0;
    effects[eff_laser_jerk].trigger.button   = 0;
    effects[eff_laser_jerk].trigger.interval = 0;
    effects[eff_laser_jerk].replay.length    = 0.2*0x100;
    effects[eff_laser_jerk].replay.delay     = 0;

    effects[eff_laser_vibrate].type = FF_PERIODIC;
    effects[eff_laser_vibrate].id   = -1;
    effects[eff_laser_vibrate].u.periodic.waveform  = FF_SINE;
    effects[eff_laser_vibrate].u.periodic.period    = 0.5*0x100;
    effects[eff_laser_vibrate].u.periodic.magnitude = _is( 5023 );
    effects[eff_laser_vibrate].u.periodic.offset    = 0;
    effects[eff_laser_vibrate].u.periodic.phase     = 0;
    effects[eff_laser_vibrate].direction = 0x4000;
    effects[eff_laser_vibrate].u.periodic.envelope.attack_length = 0.2*0x100;
    effects[eff_laser_vibrate].u.periodic.envelope.attack_level  = 0x7fff;
    effects[eff_laser_vibrate].u.periodic.envelope.fade_length   = 0x000;
    effects[eff_laser_vibrate].u.periodic.envelope.fade_level    = 0;
    effects[eff_laser_vibrate].trigger.button   = 0;
    effects[eff_laser_vibrate].trigger.interval = 0;
    effects[eff_laser_vibrate].replay.length    = 0.438*0x100;
    effects[eff_laser_vibrate].replay.delay     = 0;
#if ALL_EFFECTS
    init_bogus( eff_beam_on );
    init_bogus( eff_beam_off );

    init_bogus( eff_missile_jerk );
    init_bogus( eff_missile_vibrate );

    init_bogus( eff_hit_jerk );
    init_bogus( eff_hit_vibrate );
#endif
    effects[eff_force].type = FF_CONSTANT;
    effects[eff_force].id   = -1;
    effects[eff_force].u.constant.level = 0x0;     //dynamically
    effects[eff_force].direction = 0x0;     //dynamically
    effects[eff_force].u.constant.envelope.attack_length = 0x000;
    effects[eff_force].u.constant.envelope.attack_level  = 0;
    effects[eff_force].u.constant.envelope.fade_length   = 0x000;
    effects[eff_force].u.constant.envelope.fade_level    = 0;
    effects[eff_force].trigger.button   = 0;
    effects[eff_force].trigger.interval = 0;
    effects[eff_force].replay.length    = 0xfff;
    effects[eff_force].replay.delay     = 0;
    for (int i = 0; i < N_EFFECTS; i++) {
        VS_LOG(info, (boost::format("uploading effect %1%") % i));
        if (ioctl( ff_fd, EVIOCSFF, &effects[i] ) == -1) {
            perror( "error while uploading effect" );
            have_ff = false;
            close( ff_fd );
            return;
        }
        eff_last_time[i] = 0;
        is_played[i]     = false;
    }
    min_effect_time = 0.3;

    playEffect( eff_speed_damper );
}

void ForceFeedback::init_bogus( int i )
{
    effects[i].type = FF_CONSTANT;
    effects[i].id   = -1;
    effects[i].u.constant.level = 0x2000;       /* Strength : 25 % */
    effects[i].direction = 0x6000;      /* 135 degrees */
    effects[i].u.constant.envelope.attack_length = 0x100;
    effects[i].u.constant.envelope.attack_level  = 0;
    effects[i].u.constant.envelope.fade_length   = 0x100;
    effects[i].u.constant.envelope.fade_level    = 0;
    effects[i].trigger.button   = 0;
    effects[i].trigger.interval = 0;
    effects[i].replay.length    = 2000;     /* 2 seconds */
    effects[i].replay.delay     = 0;
}

#endif
