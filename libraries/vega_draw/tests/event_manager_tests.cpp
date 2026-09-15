/*
 * event_manager_tests.cpp
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

#include "vega_draw/event_manager.h"

using namespace vega_draw;

namespace {

class CountingWidget : public Widget {
public:
    int downs = 0;
    int keys = 0;
    void draw(ImDrawList *, const Viewport &) override {
    }
    bool onMouseDown(const InputEvent &) override {
        ++downs;
        return true;
    }
    bool onKeyDown(const InputEvent &) override {
        ++keys;
        return true;
    }
};

InputEvent Event(InputType type) {
    InputEvent event;
    event.type = type;
    return event;
}

} // namespace

TEST(EventManager, RoutesToRoot) {
    CountingWidget root;
    EventManager manager;
    manager.setRoot(&root);

    EXPECT_TRUE(manager.dispatch(Event(InputType::MouseDown)));
    EXPECT_EQ(root.downs, 1);
    EXPECT_TRUE(manager.dispatch(Event(InputType::KeyDown)));
    EXPECT_EQ(root.keys, 1);
}

TEST(EventManager, ModalTakesPrecedenceOverRoot) {
    CountingWidget root;
    CountingWidget modal;
    EventManager manager;
    manager.setRoot(&root);
    manager.setModal(&modal);

    EXPECT_TRUE(manager.dispatch(Event(InputType::MouseDown)));
    EXPECT_EQ(modal.downs, 1);
    EXPECT_EQ(root.downs, 0);

    manager.setModal(nullptr);
    EXPECT_TRUE(manager.dispatch(Event(InputType::MouseDown)));
    EXPECT_EQ(root.downs, 1);
}

TEST(EventManager, NoTargetIsUnhandled) {
    EventManager manager;
    EXPECT_FALSE(manager.dispatch(Event(InputType::MouseDown)));
}
