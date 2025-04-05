/*
 * my_unit_test_environment.h
 *
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MY_UNIT_TEST_ENVIRONMENT_H
#define MY_UNIT_TEST_ENVIRONMENT_H

#include <include/gtest/gtest.h>


class MyUnitTestEnvironment : public ::testing::Environment {
public:
    ~MyUnitTestEnvironment() override;
    void SetUp() override;
    void TearDown() override;
};

#endif //MY_UNIT_TEST_ENVIRONMENT_H
