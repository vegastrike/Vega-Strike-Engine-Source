/**
 * weapon_factory.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "weapon_factory.h"

#include "weapon_info.h"
#include "options.h"
#include "role_bitmask.h"
#include "audiolib.h"
#include "hashtable.h"

#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

namespace pt = boost::property_tree;
namespace alg = boost::algorithm;

using boost::property_tree::ptree;

using std::string;
using std::endl;

extern Hashtable<string, WeaponInfo, 257> lookuptable;

// TODO: reenable this?
/*constexpr int color_step(49);
int counts = time( nullptr );

bool gamma_needed(int gamma, int count, int depth)
{
    return !(( count/(100*depth*gamma) ) % ( (6*(color_step*100+depth)/gamma-1)/3 )
                -100);
}*/

WeaponFactory::WeaponFactory(std::string filename) {
    pt::ptree tree;
    pt::read_xml(filename, tree);

    // Iterate over root
    for (const auto &iterator : tree) {
        parse(iterator.second);

        // There should be only one root. Exiting
        break;
    }
}

void WeaponFactory::parse(ptree tree) {
//    static float gun_speed =
//            game_options()->gun_speed * (game_options()->gun_speed_adjusted_game_speed ? game_options()->game_speed : 1);
//    static int gamma = (int) (20 * game_options()->weapon_gamma);

    for (const auto &iterator : tree) {
        WeaponInfo wi;

        ptree inner = iterator.second;

        // Weapon Type
        wi.type = getWeaponTypeFromString(iterator.first);
        wi.file = getFilenameFromWeaponType(wi.type);

        // Name
        wi.name = inner.get("<xmlattr>.name", "Unknown");

        // Mount Size
        wi.size = getMountSize(inner.get("<xmlattr>.mountsize", "Unknown_mount"));

        // Energy
        wi.energy_rate = inner.get("Energy.<xmlattr>.rate", wi.energy_rate);
        wi.stability = inner.get("Energy.<xmlattr>.stability", wi.stability);
        wi.refire_rate = inner.get("Energy.<xmlattr>.refire", wi.refire_rate);
        wi.lock_time = inner.get("Energy.<xmlattr>.locktime", wi.lock_time);

        // Damage
        // TODO: weapon_list.xml laser has damage. Everything else has rate. Correct.
        wi.damage = inner.get("Damage.<xmlattr>.rate", wi.damage);
        wi.phase_damage = inner.get("Damage.<xmlattr>.phasedamage", wi.phase_damage);
        wi.radius = inner.get("Damage.<xmlattr>.radius", wi.radius);
        wi.radial_speed = inner.get("Damage.<xmlattr>.radialspeed", wi.radial_speed);
        wi.long_range = inner.get("Damage.<xmlattr>.longrange", wi.long_range);

        // Distance
        wi.volume = inner.get("Distance.<xmlattr>.volume", wi.volume);
        wi.speed = inner.get("Distance.<xmlattr>.speed", wi.speed);
        wi.pulse_speed = inner.get("Distance.<xmlattr>.pulsespeed", wi.pulse_speed);
        wi.range = inner.get("Distance.<xmlattr>.range", wi.range);
        wi.length = inner.get("Distance.<xmlattr>.length", wi.length);

        // TODO: this is a bug. It gets parsed into the same radius and radial_speed variables.
        wi.radius = inner.get("Distance.<xmlattr>.radius", wi.radius);
        wi.radial_speed = inner.get("Distance.<xmlattr>.radialspeed", wi.radial_speed);

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
        wi.file = inner.get("Appearance.<xmlattr>.file", wi.file);
        wi.a = inner.get("Appearance.<xmlattr>.a", wi.a);
        wi.r = inner.get("Appearance.<xmlattr>.r", wi.r);
        wi.g = inner.get("Appearance.<xmlattr>.g", wi.g);
        wi.b = inner.get("Appearance.<xmlattr>.b", wi.b);
        wi.a = inner.get("Appearance.<xmlattr>.a", wi.a);

        // TODO: reenable? No idea why or what this is...
        /*if ( (gamma > 0) && gamma_needed( gamma, counts, 32 ) ) {
            //approximate the color func
            wi.b = (wi.b+color_step*5)/255.;
            wi.g = (wi.g+color_step/5)/255.;
            wi.r = (wi.r+color_step*2)/255.;
        }*/

        // Sound
        std::string sound_wave = inner.get("Appearance.<xmlattr>.soundwav", "");
        if (!sound_wave.empty()) {
            // Missiles don't play the sound in a loop. Others do.
            wi.sound = AUDCreateSoundWAV(sound_wave, wi.type != WEAPON_TYPE::PROJECTILE);
        }

        std::string sound_mp3 = inner.get("Appearance.<xmlattr>.soundmp3", "");
        if (!sound_mp3.empty()) {
            // Missiles don't play the sound in a loop. Others do.
            wi.sound = AUDCreateSoundMP3(sound_wave, wi.type != WEAPON_TYPE::PROJECTILE);
        }

        // Add new WeaponInfo to weapons table
        lookuptable.Put(boost::to_upper_copy(wi.name), new WeaponInfo(wi));
    }
}
