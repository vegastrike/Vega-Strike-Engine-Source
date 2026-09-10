/*
 * scroller_tests.cpp
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

#include "vega_draw/scroller.h"

using namespace vega_draw;

namespace {

ScrollerStyle VerticalTrack() {
    ScrollerStyle style;
    style.track = Rect{100.0f, 100.0f, 20.0f, 400.0f}; // y 100..500
    style.orientation = ScrollOrientation::Vertical;
    style.min_thumb_grid = 20.0f;
    return style;
}

} // namespace

TEST(Scroller, ThumbIsProportionalAndTracksPosition) {
    const ScrollerStyle style = VerticalTrack();
    ScrollerModel model;
    model.content_extent = 1000.0f;
    model.viewport_extent = 200.0f; // visible fraction 0.2 -> thumb length 80

    model.position = 0.0f;
    Rect thumb = ScrollerThumbRect(style, model);
    EXPECT_FLOAT_EQ(thumb.height, 80.0f);
    EXPECT_FLOAT_EQ(thumb.y, 100.0f);

    model.position = 400.0f; // half way -> offset (400-80)*0.5 = 160
    thumb = ScrollerThumbRect(style, model);
    EXPECT_FLOAT_EQ(thumb.y, 260.0f);

    model.position = 800.0f; // end -> offset 320
    thumb = ScrollerThumbRect(style, model);
    EXPECT_FLOAT_EQ(thumb.y, 420.0f);
}

TEST(Scroller, ThumbFillsTrackWhenContentFits) {
    const ScrollerStyle style = VerticalTrack();
    ScrollerModel model;
    model.content_extent = 100.0f;
    model.viewport_extent = 200.0f;

    const Rect thumb = ScrollerThumbRect(style, model);
    EXPECT_FLOAT_EQ(thumb.height, 400.0f);
    EXPECT_FLOAT_EQ(thumb.y, 100.0f);
}

TEST(Scroller, ThumbHasMinimumLength) {
    const ScrollerStyle style = VerticalTrack();
    ScrollerModel model;
    model.content_extent = 100000.0f;
    model.viewport_extent = 1.0f;

    const Rect thumb = ScrollerThumbRect(style, model);
    EXPECT_FLOAT_EQ(thumb.height, 20.0f);
}

TEST(Scroller, MaxPositionAndClamp) {
    ScrollerModel model;
    model.content_extent = 1000.0f;
    model.viewport_extent = 200.0f;
    EXPECT_FLOAT_EQ(ScrollerMaxPosition(model), 800.0f);
    EXPECT_FLOAT_EQ(ScrollerClampPosition(model, -50.0f), 0.0f);
    EXPECT_FLOAT_EQ(ScrollerClampPosition(model, 500.0f), 500.0f);
    EXPECT_FLOAT_EQ(ScrollerClampPosition(model, 2000.0f), 800.0f);
}

TEST(Scroller, MaxPositionIsZeroWhenContentFits) {
    ScrollerModel model;
    model.content_extent = 100.0f;
    model.viewport_extent = 100.0f;
    EXPECT_FLOAT_EQ(ScrollerMaxPosition(model), 0.0f);
}

TEST(Scroller, PositionForThumbCentreMapsTrackPoints) {
    const ScrollerStyle style = VerticalTrack(); // travel range 400 - 80 = 320
    ScrollerModel model;
    model.content_extent = 1000.0f;
    model.viewport_extent = 200.0f;

    EXPECT_FLOAT_EQ(ScrollerPositionForThumbCentre(style, model, 140.0f), 0.0f);
    EXPECT_FLOAT_EQ(ScrollerPositionForThumbCentre(style, model, 300.0f), 400.0f);
    EXPECT_FLOAT_EQ(ScrollerPositionForThumbCentre(style, model, 460.0f), 800.0f);
}

TEST(Scroller, HorizontalThumb) {
    ScrollerStyle style;
    style.track = Rect{100.0f, 100.0f, 400.0f, 20.0f}; // x 100..500
    style.orientation = ScrollOrientation::Horizontal;
    ScrollerModel model;
    model.content_extent = 1000.0f;
    model.viewport_extent = 200.0f;
    model.position = 400.0f;

    const Rect thumb = ScrollerThumbRect(style, model);
    EXPECT_FLOAT_EQ(thumb.width, 80.0f);
    EXPECT_FLOAT_EQ(thumb.x, 260.0f);
    EXPECT_FLOAT_EQ(thumb.y, 100.0f);
    EXPECT_FLOAT_EQ(thumb.height, 20.0f);
}
