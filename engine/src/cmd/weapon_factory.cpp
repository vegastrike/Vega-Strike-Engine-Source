#include "weapon_factory.h"

#include "weapon_info.h"
#include "options.h"
#include "role_bitmask.h"
#include "audiolib.h"

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


WeaponFactory::WeaponFactory(std::string filename)
{
    pt::ptree tree;
    pt::read_xml(filename, tree);

    // Iterate over root
    for (const auto& iterator : tree)
    {
        parse(iterator.second);

        // There should be only one root. Exiting
        break;
    }
}


void WeaponFactory::parse(ptree tree)
{
    static float  gun_speed     = game_options.gun_speed * (game_options.gun_speed_adjusted_game_speed ? game_options.game_speed : 1);
    static int    gamma = (int) ( 20*game_options.weapon_gamma );

    for (const auto& iterator : tree)
    {
        weapon_info wi;

        ptree inner = iterator.second;

        // Weapon Type
        wi.type = getWeaponTypeFromString(iterator.first);
        wi.file = getFilenameFromWeaponType(wi.type);
        std::cout << "Type " << iterator.first << endl << "File " << wi.file << endl;

        // Name
        wi.name = inner.get( "<xmlattr>.name", "Unknown" );
        std::cout << "Name " << wi.name << endl;


        // Mount Size
        std::cout << "Mount size " << inner.get( "<xmlattr>.mountsize", "Unknown_mount" ) << endl;

        // Energy
        wi.energy_rate = inner.get( "Energy.<xmlattr>.rate", wi.energy_rate );
        wi.stability = inner.get( "Energy.<xmlattr>.rate", wi.stability );
        wi.refire_rate = inner.get( "Energy.<xmlattr>.rate", wi.refire_rate );
        std::cout << "Energy rate " << wi.energy_rate << endl;
        std::cout << "Energy stability " << wi.stability << endl;
        std::cout << "Energy refire " << wi.refire_rate << endl;

        // Damage
        std::cout << "Damage size " << inner.get( "Damage.<xmlattr>.rate", "Unknown_damage" ) << endl;
        std::cout << "Damage phasedamage " << inner.get( "Damage.<xmlattr>.phasedamage", "Unknown_phasedamage" ) << endl;
        std::cout << "Damage long range " << inner.get( "Damage.<xmlattr>.longrange", "Unknown_longrange" ) << endl;

        // Appearance
        wi.file = inner.get( "Appearance.<xmlattr>.file", wi.file );
        wi.r = inner.get( "Appearance.<xmlattr>.r", wi.r );
        wi.g = inner.get( "Appearance.<xmlattr>.g", wi.g );
        wi.b = inner.get( "Appearance.<xmlattr>.b", wi.b );
        wi.a = inner.get( "Appearance.<xmlattr>.a", wi.a );

        std::cout << "Appearance file " << wi.file << endl;
        std::cout << "Appearance r " << wi.r << endl;
        std::cout << "Appearance g " << wi.g << endl;
        std::cout << "Appearance b " << wi.b << endl;
        std::cout << "Appearance a " << wi.a << endl;

        // Sound
        std::string sound_wave = inner.get( "Appearance.<xmlattr>.soundwav", "" );
        if(!sound_wave.empty()) {
            // Missiles don't play the sound in a loop. Others do.
            wi.sound = AUDCreateSoundWAV( sound_wave, wi.type != WEAPON_TYPE::PROJECTILE );
        }

        std::string sound_mp3 = inner.get( "Appearance.<xmlattr>.soundmp3", "" );
        if(!sound_mp3.empty()) {
            // Missiles don't play the sound in a loop. Others do.
            wi.sound = AUDCreateSoundMP3( sound_wave, wi.type != WEAPON_TYPE::PROJECTILE );
        }

        std::cout << "Appearance sound " << wi.sound << endl;
    }
}
