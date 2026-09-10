/*
 * button_tests.cpp
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

#include "vega_draw/button.h"

using namespace vega_draw;

namespace {

class CommandContainer : public Container {
public:
    EventCommand last;
    bool onCommand(const EventCommand &command, Widget *) override {
        last = command;
        return true;
    }
};

class CommandRecorder : public Widget {
public:
    EventCommand last;
    Widget *sender = nullptr;
    void draw(ImDrawList *, const Viewport &) override {
    }
    bool onCommand(const EventCommand &command, Widget *widget) override {
        last = command;
        sender = widget;
        return true;
    }
};

PushButton MakeButton() {
    PushButton button;
    button.setRect(Rect{100.0f, 100.0f, 200.0f, 60.0f});
    button.setCommand("commit");
    return button;
}

InputEvent MouseEvent(InputType type, float x, float y) {
    InputEvent event;
    event.type = type;
    event.loc = Point{x, y};
    return event;
}

} // namespace

TEST(PushButton, PressAndReleaseInsideFiresCommand) {
    CommandRecorder recorder;
    PushButton button = MakeButton();
    button.setCommandTarget(&recorder);

    EXPECT_TRUE(button.onMouseDown(MouseEvent(InputType::MouseDown, 150.0f, 120.0f)));
    EXPECT_TRUE(button.pressed());
    EXPECT_TRUE(button.onMouseUp(MouseEvent(InputType::MouseUp, 150.0f, 120.0f)));
    EXPECT_FALSE(button.pressed());
    EXPECT_EQ(recorder.last, "commit");
    EXPECT_EQ(recorder.sender, &button);
}

TEST(PushButton, DoesNotPressWhenDisabled) {
    CommandRecorder recorder;
    PushButton button = MakeButton();
    button.setCommandTarget(&recorder);
    button.setEnabled(false);
    EXPECT_FALSE(button.onMouseDown(MouseEvent(InputType::MouseDown, 150.0f, 120.0f)));
    EXPECT_FALSE(button.pressed());
}

TEST(PushButton, ReleaseOutsideDoesNotFire) {
    CommandRecorder recorder;
    PushButton button = MakeButton();
    button.setCommandTarget(&recorder);
    EXPECT_TRUE(button.onMouseDown(MouseEvent(InputType::MouseDown, 150.0f, 120.0f)));
    EXPECT_TRUE(button.onMouseUp(MouseEvent(InputType::MouseUp, 900.0f, 900.0f)));
    EXPECT_TRUE(recorder.last.empty());
}

TEST(PushButton, CommandBubblesToParentWhenNoTarget) {
    CommandContainer container;
    auto button = std::make_unique<PushButton>(MakeButton());
    PushButton *pbutton = button.get();
    container.addChild(std::move(button));

    EXPECT_TRUE(pbutton->onMouseDown(MouseEvent(InputType::MouseDown, 150.0f, 120.0f)));
    EXPECT_TRUE(pbutton->onMouseUp(MouseEvent(InputType::MouseUp, 150.0f, 120.0f)));
    EXPECT_EQ(container.last, "commit"); // no target -> bubbles to the parent
}

TEST(PushButton, ContainerCapturesMouseAcrossRelease) {
    CommandRecorder recorder;
    Container container;
    auto button = std::make_unique<PushButton>(MakeButton());
    button->setCommandTarget(&recorder);
    container.addChild(std::move(button));

    EXPECT_TRUE(container.onMouseDown(MouseEvent(InputType::MouseDown, 150.0f, 120.0f)));
    // Released away from the button: the container routes to the captured widget.
    EXPECT_TRUE(container.onMouseUp(MouseEvent(InputType::MouseUp, 900.0f, 900.0f)));
    EXPECT_TRUE(recorder.last.empty()); // released outside -> no command
}
