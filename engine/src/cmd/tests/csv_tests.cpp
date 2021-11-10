#include <gtest/gtest.h>

#include "unit_csv_factory.h"
#include <iostream>
#include <fstream>
#include <iterator>

TEST(CSV, Sanity) {
    // This may not work for all deployments.
    // Consider standardizing this.
    std::ifstream ifs ("../../data/units/units.csv", std::ifstream::in);
    std::stringstream buffer;
    buffer << ifs.rdbuf();

    UnitCSVFactory factory;
    factory.ProcessCSV(buffer.str(), false);

    /*for (auto const& x : UnitCSVFactory::units)
    {
        std::string unit = x.first;
        std::map<std::string, std::string> unit_attributes = x.second;

        for (auto const& y : unit_attributes)
        {
            std::string key = y.first;
            std::string value = y.second;

            std::cout << x.first  << ':'
                      << y.first  << ':'
                      << y.second << std::endl;
        }
    }*/
}
