/*
 * widget_tests.cpp
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

#include "vega_draw/widget.h"

using namespace vega_draw;

namespace {

class FakeWidget : public Widget {
public:
    int downs = 0;
    void draw(ImDrawList *, const Viewport &) override {
    }
    bool onMouseDown(const InputEvent &) override {
        ++downs;
        return true;
    }
};

} // namespace

TEST(Widget, HitTestUsesHalfOpenRectAndRespectsHidden) {
    FakeWidget w;
    w.setRect(Rect{100.0f, 100.0f, 200.0f, 50.0f});
    EXPECT_TRUE(w.hitTest(Point{150.0f, 120.0f}));
    EXPECT_FALSE(w.hitTest(Point{99.0f, 120.0f}));
    EXPECT_FALSE(w.hitTest(Point{300.0f, 120.0f}));
    w.setHidden(true);
    EXPECT_FALSE(w.hitTest(Point{150.0f, 120.0f}));
}

TEST(Container, RoutesMouseToTopmostHitChild) {
    Container container;
    auto a = std::make_unique<FakeWidget>();
    a->setRect(Rect{0.0f, 0.0f, 100.0f, 100.0f});
    a->setId("a");
    auto b = std::make_unique<FakeWidget>();
    b->setRect(Rect{0.0f, 0.0f, 100.0f, 100.0f});
    b->setId("b");
    FakeWidget *pa = a.get();
    FakeWidget *pb = b.get();
    container.addChild(std::move(a));
    container.addChild(std::move(b));

    InputEvent event;
    event.type = InputType::MouseDown;
    event.loc = Point{50.0f, 50.0f};
    EXPECT_TRUE(container.onMouseDown(event));
    EXPECT_EQ(pb->downs, 1); // topmost (added last)
    EXPECT_EQ(pa->downs, 0);
}

TEST(Container, NoChildHandlesOutsideRect) {
    Container container;
    auto a = std::make_unique<FakeWidget>();
    a->setRect(Rect{0.0f, 0.0f, 100.0f, 100.0f});
    container.addChild(std::move(a));

    InputEvent event;
    event.loc = Point{500.0f, 500.0f};
    EXPECT_FALSE(container.onMouseDown(event));
}

TEST(Container, FindByIdFindsNested) {
    Container root;
    auto nested = std::make_unique<Container>();
    auto child = std::make_unique<FakeWidget>();
    child->setId("target");
    FakeWidget *pchild = child.get();
    nested->addChild(std::move(child));
    root.addChild(std::move(nested));

    EXPECT_EQ(root.findById("target"), pchild);
    EXPECT_EQ(root.findById("missing"), nullptr);
}
