#include "configuration/game_config.h"

// Global Variable Definitions
WeaponsConfig weapons_config;

WeaponsConfig::WeaponsConfig() {
    can_fire_in_spec  = GameConfig::GetVariable( "physics", "can_fire_in_spec", false);
    can_fire_in_cloak = GameConfig::GetVariable( "physics", "can_fire_in_cloak", false);
}


std::map<string, string> GameConfig::variables;

// This is probably unique enough to ensure no collision
string GameConfig::DEFAULT_ERROR_VALUE = "GameConfig::_GetVariable DEFAULT_ERROR_VALUE";

void GameConfig::LoadGameConfig(const string &filename)
{
    ptree tree;
    pt::read_xml(filename, tree);

    string const xpath = "vegaconfig.variables.";
    for (const auto& section_iterator : tree.get_child(xpath))
    {
        string section_name = section_iterator.second.get<string>("<xmlattr>.name", "");
        if(section_name == "") continue;

        ptree inner_tree = section_iterator.second;
        for (const auto& variable_iterator : inner_tree)
        {
            string name = variable_iterator.second.get<string>("<xmlattr>.name", "");
            string value = variable_iterator.second.get<string>("<xmlattr>.value", "");
            if(name == "") continue;

            string const key = section_name + "." + name;
            variables[key] = value;
        }
    }
}
