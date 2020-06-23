#include "galaxy.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace pt = boost::property_tree;

using std::string;
using std::map;
using std::vector;

Galaxy::Galaxy(string const &galaxy_file)
{
    pt::ptree tree;
    pt::read_xml(galaxy_file, tree);

    // Parse planet variables and planets
    string const planets_xpath = "galaxy.planets.";
    for (const auto& planet_iterator : tree.get_child(planets_xpath))
    {
        string type = planet_iterator.first;
        string name = planet_iterator.second.get<string>("<xmlattr>.name", "");

        // Parse planet variables
        if(type == "var") {
            string value = planet_iterator.second.get<string>("<xmlattr>.value", "");
            if(value == "") continue;
            variables[name] = value;
            //std::cout << "Parsed variable " << name << "=" << value << std::endl;
        } else if(type == "planet") { // Parse planets
            //std::cout << "Parsing planet " << name << std::endl;
            map<string, string> planet;
            pt::ptree inner_tree = planet_iterator.second;
            for (const auto& variable_iterator : inner_tree) {
                string name = variable_iterator.second.get<string>("<xmlattr>.name", "");
                string value = variable_iterator.second.get<string>("<xmlattr>.value", "");
                if(name == "" || value == "") continue;
                planet[name] = value;
                //std::cout << "Parsed variable " << name << "=" << value << std::endl;
            }
            planets[name] = planet;
        }
    }

    // Parse systems
    string const systems_xpath = "galaxy.systems.";
    for (const auto& sector_iterator : tree.get_child(systems_xpath))
    {
        string type = sector_iterator.first;
        string name = sector_iterator.second.get<string>("<xmlattr>.name", "");
        if(name == "") continue;

        // Parse sectors
//        std::cout << "Parsing sector " << name << std::endl;
        map<string, map<string, string>> sector;
        pt::ptree inner_tree = sector_iterator.second;
        for (const auto& system_iterator : inner_tree) {
            string name = system_iterator.second.get<string>("<xmlattr>.name", "");
            if(name == "") continue;
//            std::cout << "Parsing system " << name << std::endl;

            map<string, string> system;
            pt::ptree innermost_tree = system_iterator.second;
            for (const auto& variable_iterator : innermost_tree) {
                string name = variable_iterator.second.get<string>("<xmlattr>.name", "");
                string value = variable_iterator.second.get<string>("<xmlattr>.value", "");
                if(name == "" || value == "") continue;
                system[name] = value;
//                std::cout << "Parsed variable " << name << "=" << value << std::endl;
            }
            sector[name] = system;
        }
        systems[name] = sector;
    }
}
