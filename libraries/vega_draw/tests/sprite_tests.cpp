/*
 * sprite_tests.cpp
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

#include <memory>

#include "vega_draw/sprite.h"

using namespace vega_draw;

namespace {

class Stub : public Widget {
public:
    void draw(ImDrawList *, const Viewport &) override {
    }
};

} // namespace

TEST(TextureRegion, DefaultsCoverWholeTexture) {
    const TextureRegion region;
    EXPECT_EQ(region.texture, 0u);
    EXPECT_FLOAT_EQ(region.u0, 0.0f);
    EXPECT_FLOAT_EQ(region.v0, 0.0f);
    EXPECT_FLOAT_EQ(region.u1, 1.0f);
    EXPECT_FLOAT_EQ(region.v1, 1.0f);
}

TEST(TexturePanel, IsAContainerForChildren) {
    TexturePanel panel;
    panel.setRect(Rect{0.0f, 0.0f, 100.0f, 100.0f});
    panel.setTexture(TextureRegion{42u, 0.0f, 0.0f, 1.0f, 1.0f});
    EXPECT_EQ(panel.texture().texture, 42u);

    auto child = std::make_unique<Stub>();
    child->setRect(Rect{0.0f, 0.0f, 50.0f, 50.0f});
    child->setId("child");
    Stub *pchild = child.get();
    panel.addChild(std::move(child));
    EXPECT_EQ(panel.findById("child"), pchild);
}
