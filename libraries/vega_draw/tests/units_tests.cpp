/*
 * units_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, Evert Vorster
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

#include "vega_draw/units.h"

using namespace vega_draw;

TEST(Units, GridMapsAcrossViewport) {
    const Viewport v{1920.0f, 1080.0f};
    EXPECT_FLOAT_EQ(GridToPixelX(500.0f, v), 960.0f);
    EXPECT_FLOAT_EQ(GridToPixelY(500.0f, v), 540.0f);
    EXPECT_FLOAT_EQ(GridToPixelX(1000.0f, v), 1920.0f);
    EXPECT_FLOAT_EQ(GridToPixelY(1000.0f, v), 1080.0f);
    EXPECT_FLOAT_EQ(GridToPixelW(1000.0f, v), 1920.0f);
    EXPECT_FLOAT_EQ(GridToPixelH(1000.0f, v), 1080.0f);
}

TEST(Units, FontHeightTiers) {
    EXPECT_FLOAT_EQ(ToFontGrid(FontHeight::ExtraSmall), 40.0f);
    EXPECT_FLOAT_EQ(ToFontGrid(FontHeight::Small), 45.0f);
    EXPECT_FLOAT_EQ(ToFontGrid(FontHeight::Medium), 50.0f);
    EXPECT_FLOAT_EQ(ToFontGrid(FontHeight::Large), 55.0f);
    EXPECT_FLOAT_EQ(ToFontGrid(FontHeight::ExtraLarge), 60.0f);
}

TEST(Units, FontSizeIsFractionOfHeightRoundedToWholePixel) {
    const Viewport v{1920.0f, 1080.0f};
    EXPECT_FLOAT_EQ(FontGridToPixel(20.0f, v), 22.0f); // 0.02 * 1080 = 21.6 -> 22
    EXPECT_FLOAT_EQ(FontGridToPixel(70.0f, v), 76.0f); // 0.07 * 1080 = 75.6 -> 76
    const Viewport square{1000.0f, 1000.0f};
    EXPECT_FLOAT_EQ(FontGridToPixel(20.0f, square), 20.0f);
    EXPECT_FLOAT_EQ(FontGridToPixel(1000.0f, square), 1000.0f);
}
