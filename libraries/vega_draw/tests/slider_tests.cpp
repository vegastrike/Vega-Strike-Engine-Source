/*
 * slider_tests.cpp
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

#include "vega_draw/slider.h"

using namespace vega_draw;

namespace {

Slider MakeHorizontal() {
    Slider slider;
    slider.setRect(Rect{100.0f, 100.0f, 200.0f, 20.0f}); // x 100..300
    slider.setOrientation(ScrollOrientation::Horizontal);
    slider.style().thumb_grid = 20.0f;
    slider.setRange(0.0f, 100.0f);
    return slider;
}

InputEvent Mouse(InputType type, float x, float y) {
    InputEvent event;
    event.type = type;
    event.loc = Point{x, y};
    return event;
}

} // namespace

TEST(Slider, ValueIsClampedToRange) {
    Slider slider = MakeHorizontal();
    slider.setValue(-10.0f);
    EXPECT_FLOAT_EQ(slider.value(), 0.0f);
    slider.setValue(150.0f);
    EXPECT_FLOAT_EQ(slider.value(), 100.0f);
}

TEST(Slider, PressSetsValueFromPosition) {
    Slider slider = MakeHorizontal();
    EXPECT_TRUE(slider.onMouseDown(Mouse(InputType::MouseDown, 100.0f, 110.0f)));
    EXPECT_FLOAT_EQ(slider.value(), 0.0f);

    EXPECT_TRUE(slider.onMouseDrag(Mouse(InputType::MouseDrag, 200.0f, 110.0f)));
    EXPECT_FLOAT_EQ(slider.value(), 50.0f);

    EXPECT_TRUE(slider.onMouseDrag(Mouse(InputType::MouseDrag, 300.0f, 110.0f)));
    EXPECT_FLOAT_EQ(slider.value(), 100.0f);
}

TEST(Slider, ThumbRectTracksValue) {
    Slider slider = MakeHorizontal();
    slider.setValue(50.0f);
    const Rect thumb = slider.thumbRect();
    EXPECT_FLOAT_EQ(thumb.origin.x, 190.0f); // 100 + 0.5 * (200 - 20)
    EXPECT_FLOAT_EQ(thumb.size.width, 20.0f);
    EXPECT_FLOAT_EQ(thumb.origin.y, 100.0f);
    EXPECT_FLOAT_EQ(thumb.size.height, 20.0f);
}

TEST(Slider, PressOutsideIsIgnored) {
    Slider slider = MakeHorizontal();
    EXPECT_FALSE(slider.onMouseDown(Mouse(InputType::MouseDown, 900.0f, 900.0f)));
}
