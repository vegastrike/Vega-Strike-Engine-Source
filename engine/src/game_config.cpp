#include "game_config.h"

std::map<string, string> GameConfig::variables;

// This is probably unique enough to ensure no collision
string const DEFAULT_ERROR_VALUE = "GameConfig::_GetVariable DEFAULT_ERROR_VALUE";

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

string GameConfig::_GetVariable(string const &section, string const &name)
{
    string const key = section + "." + name;
    if (variables.count(key))
        return variables[key];
    return DEFAULT_ERROR_VALUE;
}

template <>
float GameConfig::GetVariable(string const &section, string const &name, float default_value)
{
    string result = _GetVariable(section, name);
    if(result == DEFAULT_ERROR_VALUE) return default_value;
    return std::stof(result);
}

template <>
double GameConfig::GetVariable(string const &section, string const &name, double default_value)
{
    string result = _GetVariable(section, name);
    if(result == DEFAULT_ERROR_VALUE) return default_value;
    return std::stod(result);
}

template <>
int GameConfig::GetVariable(string const &section, string const &name, int default_value)
{
    string result = _GetVariable(section, name);
    if(result == DEFAULT_ERROR_VALUE) return default_value;
    return std::stoi(result);
}
