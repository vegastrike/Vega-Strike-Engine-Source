#ifndef FACTIONFACTORY_H
#define FACTIONFACTORY_H

#include <string>
#include <boost/property_tree/ptree.hpp>

class FactionFactory
{
public:
    FactionFactory(std::string filename);

private:
    void parse(boost::property_tree::ptree tree);
};

#endif // FACTIONFACTORY_H
