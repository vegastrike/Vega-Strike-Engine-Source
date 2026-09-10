/*
 * scroll_bar_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#include "vega_draw/scroll_bar.h"

using namespace vega_draw;

namespace {

ScrollBar MakeVertical() {
    ScrollBar bar;
    bar.setRect(Rect{100.0f, 100.0f, 20.0f, 400.0f}); // y 100..500
    bar.setOrientation(ScrollOrientation::Vertical);
    bar.setMinThumbGrid(20.0f);
    bar.setContentExtent(1000.0f);
    bar.setViewportExtent(200.0f);
    return bar;
}

InputEvent Mouse(InputType type, float x, float y) {
    InputEvent event;
    event.type = type;
    event.loc = Point{x, y};
    return event;
}

} // namespace

TEST(ScrollBar, DragMovesPosition) {
    ScrollBar bar = MakeVertical();

    EXPECT_TRUE(bar.onMouseDown(Mouse(InputType::MouseDown, 110.0f, 140.0f)));
    EXPECT_FLOAT_EQ(bar.position(), 0.0f);

    EXPECT_TRUE(bar.onMouseDrag(Mouse(InputType::MouseDrag, 110.0f, 300.0f)));
    EXPECT_FLOAT_EQ(bar.position(), 400.0f);

    EXPECT_TRUE(bar.onMouseDrag(Mouse(InputType::MouseDrag, 110.0f, 460.0f)));
    EXPECT_FLOAT_EQ(bar.position(), 800.0f);

    EXPECT_TRUE(bar.onMouseUp(Mouse(InputType::MouseUp, 110.0f, 460.0f)));
    EXPECT_FALSE(bar.onMouseDrag(Mouse(InputType::MouseDrag, 110.0f, 300.0f)));
}

TEST(ScrollBar, PressOutsideIsIgnored) {
    ScrollBar bar = MakeVertical();
    EXPECT_FALSE(bar.onMouseDown(Mouse(InputType::MouseDown, 900.0f, 900.0f)));
}

TEST(ScrollBar, PositionClampsToRange) {
    ScrollBar bar = MakeVertical();
    bar.setPosition(9999.0f);
    EXPECT_FLOAT_EQ(bar.position(), 800.0f);
    bar.setPosition(-50.0f);
    EXPECT_FLOAT_EQ(bar.position(), 0.0f);
}

TEST(ScrollBar, MaxPositionZeroWhenContentFits) {
    ScrollBar bar = MakeVertical();
    bar.setContentExtent(100.0f);
    bar.setViewportExtent(200.0f);
    EXPECT_FLOAT_EQ(bar.maxPosition(), 0.0f);
}
