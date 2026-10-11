/*
 * text_input_tests.cpp
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

#include "vega_draw/text_input.h"

using namespace vega_draw;

namespace {

InputEvent Key(unsigned int code) {
    InputEvent event;
    event.type = InputType::KeyDown;
    event.code = code;
    return event;
}

} // namespace

TEST(TextInput, TypingAppends) {
    TextInput input;
    EXPECT_TRUE(input.onKeyDown(Key('A')));
    EXPECT_TRUE(input.onKeyDown(Key('b')));
    EXPECT_EQ(input.text(), "Ab");
}

TEST(TextInput, BackspaceRemovesLast) {
    TextInput input;
    input.setText("abc");
    EXPECT_TRUE(input.onKeyDown(Key(8)));
    EXPECT_EQ(input.text(), "ab");
}

TEST(TextInput, DisallowedAndMaxLengthAreEnforced) {
    TextInput input;
    input.setDisallowed("/");
    input.setMaxLength(2);
    EXPECT_FALSE(input.onKeyDown(Key('/')));
    EXPECT_TRUE(input.onKeyDown(Key('a')));
    EXPECT_TRUE(input.onKeyDown(Key('b')));
    EXPECT_FALSE(input.onKeyDown(Key('c')));
    EXPECT_EQ(input.text(), "ab");
}

TEST(TextInput, NonPrintableIsIgnored) {
    TextInput input;
    EXPECT_FALSE(input.onKeyDown(Key(1)));
    EXPECT_TRUE(input.text().empty());
}
