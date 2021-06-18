#ifndef WEAPON_FACTORY_H
#define WEAPON_FACTORY_H

#include <string>
#include <boost/property_tree/ptree.hpp>

class WeaponFactory
{
public:
    WeaponFactory(std::string filename);

private:
    void parse(boost::property_tree::ptree tree);
};

#endif // WEAPON_FACTORY_H
