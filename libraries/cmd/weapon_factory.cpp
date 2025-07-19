/*
 * weapon_factory.cpp
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


#include "cmd/weapon_factory.h"

#include "cmd/weapon_info.h"
#include "cmd/role_bitmask.h"
#include "src/audiolib.h"

#include "src/hashtable.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/json.hpp>
#include <boost/algorithm/string.hpp>

#include "resource/json_utils.h"

extern Hashtable<std::string, WeaponInfo, 257> lookuptable;

// TODO: reenable this?
/*constexpr int color_step(49);
int counts = time( nullptr );

bool gamma_needed(int gamma, int count, int depth)
{
    return !(( count/(100*depth*gamma) ) % ( (6*(color_step*100+depth)/gamma-1)/3 )
                -100);
}*/

void ParseWeapon(const boost::json::object& weapon) {
    WeaponInfo wi;

    // Weapon Type
    const std::string weapon_type = JsonGetWithDefault(weapon, "type", std::string());
    wi.type = getWeaponTypeFromString(weapon_type);
    wi.file = getFilenameFromWeaponType(wi.type);

    // Name
    wi.name = JsonGetStringWithDefault(weapon, "name", "Unknown");

    // Mount Size
    wi.size = getMountSize(JsonGetStringWithDefault(weapon, "mountsize", "Unknown_mount"));

    // Energy
    wi.energy_rate = JsonGetWithDefault(weapon, "Energy.rate", wi.energy_rate);
    wi.stability = JsonGetWithDefault(weapon, "Energy.stability", wi.stability);
    wi.refire_rate = JsonGetWithDefault(weapon, "Energy.refire", wi.refire_rate);
    wi.lock_time = JsonGetWithDefault(weapon, "Energy.locktime", wi.lock_time);


    // Damage
    // TODO: weapon_list.xml laser has damage. Everything else has rate. Correct.
    wi.damage = JsonGetWithDefault(weapon, "Damage.rate", wi.damage);
    wi.phase_damage = JsonGetWithDefault(weapon, "Damage.phasedamage", wi.phase_damage);
    wi.radius = JsonGetWithDefault(weapon, "Damage.radius", wi.radius);
    wi.radial_speed = JsonGetWithDefault(weapon, "Damage.radialspeed", wi.radial_speed);
    wi.long_range = JsonGetWithDefault(weapon, "Damage.longrange", wi.long_range);


    // Distance
    wi.volume = JsonGetWithDefault(weapon, "Distance.volume", wi.volume);
    wi.speed = JsonGetWithDefault(weapon, "Distance.speed", wi.speed);
    wi.pulse_speed = JsonGetWithDefault(weapon, "Distance.pulsespeed", wi.pulse_speed);
    wi.range = JsonGetWithDefault(weapon, "Distance.range", wi.range);
    wi.length = JsonGetWithDefault(weapon, "Distance.length", wi.length);


    // TODO: this is a bug. It gets parsed into the same radius and radial_speed variables.
    wi.radius = JsonGetWithDefault(weapon, "Distance.radius", wi.radius);
    wi.radial_speed = JsonGetWithDefault(weapon, "Distance.radialspeed", wi.radial_speed);


    // TODO: detonation range not implemented. Incorrectly assigns to pulse_speed...
    //wi.bug = inner.get( "Energy.<xmlattr>.detonationrange", wi.bug );

    // TODO: is this really necessary???
    /*if(game_options()->gun_speed_adjusted_game_speed) {
        if (wi.speed < 1000) {
            wi.speed *= 1.0+gun_speed/1.25;
        } else if (wi.speed < 2000) {
            wi.speed *= 1.0+gun_speed/2.5;
        } else if (wi.speed < 4000) {
            wi.speed *= 1.0+gun_speed/6.0;
        } else if (wi.speed < 8000) {
            wi.speed *= 1.0+gun_speed/17.0;
        }
    }*/

    // Appearance
    wi.file = JsonGetWithDefault(weapon, "Appearance.file", wi.file);
    wi.r = JsonGetWithDefault(weapon, "Appearance.r", wi.r);
    wi.g = JsonGetWithDefault(weapon, "Appearance.g", wi.g);
    wi.b = JsonGetWithDefault(weapon, "Appearance.b", wi.b);
    wi.a = JsonGetWithDefault(weapon, "Appearance.a", wi.a);

    // TODO: reenable? No idea why or what this is...
    /*if ( (gamma > 0) && gamma_needed( gamma, counts, 32 ) ) {
        //approximate the color func
        wi.b = (wi.b+color_step*5)/255.;
        wi.g = (wi.g+color_step/5)/255.;
        wi.r = (wi.r+color_step*2)/255.;
    }*/

    // Sound
    std::string sound_wave = JsonGetStringWithDefault(weapon, "Appearance.soundwav", "");
    if (!sound_wave.empty()) {
        // Missiles don't play the sound in a loop. Others do.
        wi.sound = AUDCreateSoundWAV(sound_wave, wi.type != WEAPON_TYPE::PROJECTILE);
    }

    std::string sound_mp3 = JsonGetStringWithDefault(weapon, "Appearance.soundmp3", "");
    if (!sound_mp3.empty()) {
        // Missiles don't play the sound in a loop. Others do.
        wi.sound = AUDCreateSoundMP3(sound_wave, wi.type != WEAPON_TYPE::PROJECTILE);
    }

    // Add new WeaponInfo to weapons table
    lookuptable.Put(boost::to_upper_copy(wi.name), new WeaponInfo(wi));
}


WeaponFactory::WeaponFactory(std::string filename) {
    std::ifstream ifs(filename, std::ifstream::in);
    std::stringstream buffer;
    buffer << ifs.rdbuf();

    const std::string json_text = buffer.str();

    boost::json::value json_value = boost::json::parse(json_text);
    boost::json::array root_array = json_value.get_array();

    for(boost::json::value& weapon_value : root_array) {
        boost::json::object weapon = weapon_value.get_object();
        ParseWeapon(weapon);
    }
}




