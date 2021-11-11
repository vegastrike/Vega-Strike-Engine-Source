#ifndef UNITCSVFACTORY_H
#define UNITCSVFACTORY_H

#include <map>
#include <string>
#include <utility>
#include <boost/algorithm/string.hpp>

#include <iostream>

class UnitCSVFactory
{
    static std::string DEFAULT_ERROR_VALUE;
    static std::map<std::string, std::map<std::string, std::string>> units;

    static inline std::string _GetVariable(std::string unit_key, std::string const &attribute_key)
    {
        if(units.count(unit_key) == 0) {
            return DEFAULT_ERROR_VALUE;
        }

        std::map<std::string, std::string> unit_attributes = UnitCSVFactory::units[unit_key];

        if (unit_attributes.count(attribute_key) == 0) {
            return DEFAULT_ERROR_VALUE;
        }


        return unit_attributes[attribute_key];
    }

public:
    static void ProcessCSV(const std::string &d, bool saved_game);

    template <class T>
    static inline T GetVariable(std::string unit_key, std::string const &attribute_key, T default_value) = delete;
};




// Template Specialization
template <>
inline std::string UnitCSVFactory::GetVariable(std::string unit_key, std::string const &attribute_key, std::string default_value)
{
    std::string result = _GetVariable(unit_key, attribute_key);
    if(result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }

    return result;
}

// Need this in because "abcd" is const char* and not std::string
/*template <>
inline const char* UnitCSVFactory::GetVariable(std::string unit_key, std::string const &attribute_key, const char* default_value)
{
    std::string result = _GetVariable(unit_key, attribute_key);
    if(result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }

    return result.c_str();
}*/

template <>
inline bool UnitCSVFactory::GetVariable(std::string unit_key, std::string const &attribute_key, bool default_value)
{
    std::string result = _GetVariable(unit_key, attribute_key);
    if(result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    boost::algorithm::to_lower(result);
    return (result == "true" || result == "1");
}

template <>
inline float UnitCSVFactory::GetVariable(std::string unit_key, std::string const &attribute_key, float default_value)
{
    std::string result = _GetVariable(unit_key, attribute_key);

    if(result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }

    try {
        return std::stof(result);
    }  catch (std::invalid_argument) {
        return default_value;
    }
}

template <>
inline double UnitCSVFactory::GetVariable(std::string unit_key, std::string const &attribute_key, double default_value)
{
    std::string result = _GetVariable(unit_key, attribute_key);
    if(result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    try {
        return std::stod(result);
    }  catch (std::invalid_argument) {
        return default_value;
    }
}

template <>
inline int UnitCSVFactory::GetVariable(std::string unit_key, std::string const &attribute_key, int default_value)
{
    std::string result = _GetVariable(unit_key, attribute_key);
    if(result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    try {
        return std::stoi(result);
    }  catch (std::invalid_argument) {
        return default_value;
    }
}

#endif // UNITCSVFACTORY_H
