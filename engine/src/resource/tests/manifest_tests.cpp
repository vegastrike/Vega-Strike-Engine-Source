/*
 * manifest_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include <random>

#include "manifest.h"


TEST(Manifest, Random) {
    int size = 5;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> int_dist(0,size-1); 

    for(int i=0;i<1000;i++) {
        int index = int_dist(rng);

        EXPECT_GE(index,0);
        EXPECT_LT(index,size);
    }
}


TEST(Manifest, MPL) {
    // TODO: reenable once we figure out how to find out the data folder location

    /*boost::filesystem::path full_path(boost::filesystem::current_path());
    std::cerr << "Current path is : " << full_path << std::endl;
    std::string path = boost::filesystem::current_path().c_str();
    path = path + "/../data/";

    boost::filesystem::current_path(path);

    Manifest mpl = Manifest::MPL();

    std::cerr << "MPL Size" << mpl.size() << std::endl << std::flush;

    EXPECT_GT(mpl.size(), 100);

    Manifest food = mpl.GetCategoryManifest("Natural_Products/Food/Confed");

    EXPECT_EQ(food.size(), 4);

    Cargo c = mpl.GetRandomCargoFromCategory("Natural_Products/Food/Confed");

    std::cout << c.GetName() << std::endl;

    EXPECT_GT(c.GetName().size(), 0);*/
}