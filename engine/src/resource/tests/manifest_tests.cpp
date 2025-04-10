/*
 * manifest.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include <gtest/gtest.h>
#include <iostream>

#include "resource/manifest.h"



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
