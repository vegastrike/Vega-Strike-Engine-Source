/*
 * diskobject_tests.cpp
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
#include <string>

#include <gtest/gtest.h>

#include "vegadisk/diskobject.h"

class DiskObjectTestModifier : public vega_disk::DiskObject {
    public:
        DiskObjectTestModifier(int _error_code, std::string _filename, std::string _error_msg) {
            error_code = _error_code;
            filename = _filename;
            error_message = _error_msg;
        }
};

TEST(VegaDisk, DiskObjectDefault) {
    // clean disk object; just validate it's in the expected error-free state
    vega_disk::DiskObject diskObjectDefault{};
    int errorValue = diskObjectDefault.GetError();
    std::string errorMessage = diskObjectDefault.GetErrorString();
    EXPECT_EQ(errorValue, vega_disk::DiskObject::ERROR_OK);
    EXPECT_STREQ(errorMessage.c_str(), "");
}

TEST(VegaDisk, DiskObjectProperties) {
    // configure the disk object via the accessor class above that gives access
    // to the protected values behind the get methods.
    int diskObjectErrorValue = vega_disk::DiskObject::ERROR_ARCHIVE_FATAL_ERROR;
    std::string diskObjectFilename = "foobar.test";
    std::string diskObjectErrorMessage = "test error message value for foobar.test";
    DiskObjectTestModifier diskObjectTester(
        diskObjectErrorValue,
        diskObjectFilename,
        diskObjectErrorMessage
    );
    int errorValue = diskObjectTester.GetError();
    std::string errorMessage = diskObjectTester.GetErrorString();
    std::string filename = diskObjectTester.GetFilename();
    EXPECT_EQ(errorValue, diskObjectErrorValue);
    EXPECT_STREQ(errorMessage.c_str(), diskObjectErrorMessage.c_str());
    EXPECT_STREQ(filename.c_str(), diskObjectFilename.c_str());

    std::string newFilename = "barfoo.test";
    diskObjectTester.SetFilename(newFilename);
    std::string updatedFilename = diskObjectTester.GetFilename();
    EXPECT_STREQ(updatedFilename.c_str(), newFilename.c_str());

    diskObjectTester.ClearError();
    int updatedError = diskObjectTester.GetError();
    std::string updatedErrorMessage = diskObjectTester.GetErrorString();
    EXPECT_EQ(updatedError, vega_disk::DiskObject::ERROR_OK);
    EXPECT_NE(updatedError, errorValue);
    EXPECT_STREQ(updatedErrorMessage.c_str(), "");
    EXPECT_STRNE(updatedErrorMessage.c_str(), errorMessage.c_str());
}

// TODO:
// - ListEntries
// - ReadEntry
// - WriteEntry
