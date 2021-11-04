#ifndef UNITCSVFACTORY_H
#define UNITCSVFACTORY_H

#include <map>
#include <string>
#include <utility>


class UnitCSVFactory
{
public:
    static std::map<std::string, std::map<std::string, std::string>> units;

    UnitCSVFactory();

    void ProcessCSV(const std::string &d);
};

#endif // UNITCSVFACTORY_H
