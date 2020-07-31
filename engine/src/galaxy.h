#ifndef GALAXY_H
#define GALAXY_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <map>
#include <vector>

using std::string;
using std::map;
using std::vector;

class Galaxy
{
public:
    Galaxy(string const &galaxy_file);

    map<string, string> variables;
    map<string, map<string, string>> planets; // planets-variables
    map<string, map<string, map<string, string>>> systems; // sectors-systems-variables
};

#endif // GALAXY_H
