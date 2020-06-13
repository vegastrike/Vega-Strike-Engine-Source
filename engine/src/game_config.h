#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <set>
#include <map>
#include <exception>
#include <iostream>

namespace pt = boost::property_tree;

using std::string;
using pt::ptree;



class GameConfig
{ 
private:
    static std::map<string, string> variables;
    static string _GetVariable(string const &section, string const &name);

public:
    static void LoadGameConfig(const string &filename);
    template <class T>
    static T GetVariable(string const &section, string const &name, T default_value) = delete;
};





template <>
float GameConfig::GetVariable(string const &section, string const &name, float default_value);

template <>
double GameConfig::GetVariable(string const &section, string const &name, double default_value);

template <>
int GameConfig::GetVariable(string const &section, string const &name, int default_value);

#endif // GAME_CONFIG_H
