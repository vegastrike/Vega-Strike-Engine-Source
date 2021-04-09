#include "weapon_factory.h"

#include "weapon_info.h"

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
    for (const auto& iterator : tree)
    {
        weapon_info wi;

        ptree inner = iterator.second;

        std::cout << "Type " << iterator.first << endl;
        std::cout << "Name " << inner.get( "<xmlattr>.name", "Unknown" ) << endl;
        std::cout << "Mount size " << inner.get( "<xmlattr>.mountsize", "Unknown_mount" ) << endl;

        std::cout << "Energy rate " << inner.get( "Energy.<xmlattr>.rate", "Unknown_rate" ) << endl;
        std::cout << "Energy stability " << inner.get( "Energy.<xmlattr>.stability", "Unknown_stability" ) << endl;
        std::cout << "Energy refire " << inner.get( "Energy.<xmlattr>.refire", "Unknown_refire" ) << endl;

        std::cout << "Damage size " << inner.get( "Damage.<xmlattr>.rate", "Unknown_damage" ) << endl;
        std::cout << "Damage phasedamage " << inner.get( "Damage.<xmlattr>.phasedamage", "Unknown_phasedamage" ) << endl;
        std::cout << "Damage long range " << inner.get( "Damage.<xmlattr>.longrange", "Unknown_longrange" ) << endl;

        std::cout << "Appearance file " << inner.get( "Appearance.<xmlattr>.file", "Unknown_damage" ) << endl;
        std::cout << "Appearance soundwav " << inner.get( "Appearance.<xmlattr>.soundwav", "Unknown_soundwav" ) << endl;
        std::cout << "Appearance r " << inner.get( "Appearance.<xmlattr>.r", "r" ) << endl;
        std::cout << "Appearance g " << inner.get( "Appearance.<xmlattr>.g", "g" ) << endl;
        std::cout << "Appearance b " << inner.get( "Appearance.<xmlattr>.b", "b" ) << endl;
        std::cout << "Appearance a " << inner.get( "Appearance.<xmlattr>.a", "a" ) << endl;
    }
}
