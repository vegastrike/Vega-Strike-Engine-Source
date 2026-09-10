/*
 * geometry_tests.cpp
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

#include "vega_draw/geometry.h"

using namespace vega_draw;

TEST(Geometry, EdgesAreTopLeftYDown) {
    const Rect r{100.0f, 200.0f, 300.0f, 400.0f};
    EXPECT_FLOAT_EQ(r.left(), 100.0f);
    EXPECT_FLOAT_EQ(r.right(), 400.0f);
    EXPECT_FLOAT_EQ(r.top(), 200.0f); // y-down: top is the smaller y
    EXPECT_FLOAT_EQ(r.bottom(), 600.0f);
    const Point c = r.center();
    EXPECT_FLOAT_EQ(c.x, 250.0f);
    EXPECT_FLOAT_EQ(c.y, 400.0f);
}

TEST(Geometry, InsideIsInclusive) {
    const Rect r{0.0f, 0.0f, 100.0f, 100.0f};
    EXPECT_TRUE(r.inside(Point{0.0f, 0.0f}));
    EXPECT_TRUE(r.inside(Point{100.0f, 100.0f}));
    EXPECT_TRUE(r.inside(Point{50.0f, 50.0f}));
    EXPECT_FALSE(r.inside(Point{-1.0f, 50.0f}));
    EXPECT_FALSE(r.inside(Point{50.0f, 101.0f}));
}

TEST(Geometry, InsetTrimsEveryEdge) {
    const Rect r{0.0f, 0.0f, 100.0f, 100.0f};
    const Rect inner = r.inset(Size{10.0f, 20.0f});
    EXPECT_FLOAT_EQ(inner.x, 10.0f);
    EXPECT_FLOAT_EQ(inner.y, 20.0f);
    EXPECT_FLOAT_EQ(inner.width, 80.0f);
    EXPECT_FLOAT_EQ(inner.height, 60.0f);
}

TEST(Geometry, GridRectMapsToPixels) {
    const Viewport v{1000.0f, 1000.0f}; // 1 grid unit == 1 px
    const Rect px = GridToPixelRect(Rect{100.0f, 200.0f, 300.0f, 400.0f}, v);
    EXPECT_FLOAT_EQ(px.x, 100.0f);
    EXPECT_FLOAT_EQ(px.y, 200.0f);
    EXPECT_FLOAT_EQ(px.width, 300.0f);
    EXPECT_FLOAT_EQ(px.height, 400.0f);

    const Viewport hd{1920.0f, 1080.0f};
    const Rect hp = GridToPixelRect(Rect{0.0f, 0.0f, 1000.0f, 1000.0f}, hd);
    EXPECT_FLOAT_EQ(hp.width, 1920.0f);
    EXPECT_FLOAT_EQ(hp.height, 1080.0f);
}

TEST(Geometry, GridPointMapsToPixels) {
    const Viewport hd{1920.0f, 1080.0f};
    const Point p = GridToPixelPoint(Point{500.0f, 500.0f}, hd);
    EXPECT_FLOAT_EQ(p.x, 960.0f);
    EXPECT_FLOAT_EQ(p.y, 540.0f);
}
