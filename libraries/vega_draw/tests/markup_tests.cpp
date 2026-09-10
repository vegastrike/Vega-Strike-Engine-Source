/*
 * markup_tests.cpp
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

#include "vega_draw/markup.h"

using namespace vega_draw;

TEST(ParseMarkup, PlainTextIsOneUnstyledRun) {
    const TextLines lines = ParseMarkup("hello world");
    ASSERT_EQ(lines.size(), 1u);
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "hello world");
    EXPECT_FALSE(lines[0].runs[0].style.bold);
    EXPECT_FALSE(lines[0].runs[0].style.color.set);
    EXPECT_FALSE(lines[0].manual_break);
}

TEST(ParseMarkup, ColorSpanCarriesValues) {
    const TextLines lines = ParseMarkup("<color=55ffff>blue</color> plain");
    ASSERT_EQ(lines.size(), 1u);
    ASSERT_EQ(lines[0].runs.size(), 2u);
    EXPECT_EQ(lines[0].runs[0].text, "blue");
    EXPECT_TRUE(lines[0].runs[0].style.color.set);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.r), 0x55);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.g), 0xff);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.b), 0xff);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.a), 0xff);
    EXPECT_EQ(lines[0].runs[1].text, " plain");
    EXPECT_FALSE(lines[0].runs[1].style.color.set);
}

TEST(ParseMarkup, AlphaHexIsAccepted) {
    const TextLines lines = ParseMarkup("<color=11223344>x</color>");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.r), 0x11);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.a), 0x44);
}

TEST(ParseMarkup, BoldSpan) {
    const TextLines lines = ParseMarkup("a <b>bold</b> c");
    ASSERT_EQ(lines[0].runs.size(), 3u);
    EXPECT_FALSE(lines[0].runs[0].style.bold);
    EXPECT_TRUE(lines[0].runs[1].style.bold);
    EXPECT_EQ(lines[0].runs[1].text, "bold");
    EXPECT_FALSE(lines[0].runs[2].style.bold);
}

TEST(ParseMarkup, NestedColorAndBold) {
    const TextLines lines = ParseMarkup("<color=ff0000><b>hot</b></color>");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "hot");
    EXPECT_TRUE(lines[0].runs[0].style.bold);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.r), 0xff);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.g), 0x00);
}

TEST(ParseMarkup, HardBreakSplitsLinesAndMarksManual) {
    const TextLines lines = ParseMarkup("line1<br>line2");
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_TRUE(lines[0].manual_break);
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "line1");
    EXPECT_FALSE(lines[1].manual_break);
    ASSERT_EQ(lines[1].runs.size(), 1u);
    EXPECT_EQ(lines[1].runs[0].text, "line2");
}

TEST(ParseMarkup, EntitiesAreDecoded) {
    const TextLines lines = ParseMarkup("1 &lt; 2 &amp;&amp; 3 &gt; 2");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "1 < 2 && 3 > 2");
}

TEST(ParseMarkup, UnrecognisedTagIsLiteralText) {
    const TextLines lines = ParseMarkup("Ship <foo> 4");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "Ship <foo> 4");
}

TEST(ParseMarkup, InvalidColorTagIsLiteralText) {
    const TextLines lines = ParseMarkup("<color=zzzzzz>x</color>");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_FALSE(lines[0].runs[0].style.color.set);
    EXPECT_EQ(lines[0].runs[0].text, "<color=zzzzzz>x");
}
