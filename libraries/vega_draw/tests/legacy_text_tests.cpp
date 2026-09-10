/*
 * legacy_text_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file.
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

#include "vega_draw/legacy_text.h"

using namespace vega_draw;

static TextLines ParseImGui(const std::string &source) {
    return ParseLegacyVegaText(source, LegacyTextDialect::ImGuiText);
}

static TextLines ParseTextPlane(const std::string &source) {
    return ParseLegacyVegaText(source, LegacyTextDialect::TextPlane);
}

TEST(ParseLegacyVegaText, PlainTextIsOneUnstyledRun) {
    const TextLines lines = ParseImGui("hello world");
    ASSERT_EQ(lines.size(), 1u);
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "hello world");
    EXPECT_FLOAT_EQ(lines[0].runs[0].style.weight, kWeightNormal);
    EXPECT_FALSE(lines[0].runs[0].style.color.set);
    EXPECT_FALSE(lines[0].manual_break);
}

TEST(ParseLegacyVegaText, EmptySourceIsOneEmptyLine) {
    const TextLines lines = ParseImGui("");
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_TRUE(lines[0].runs.empty());
}

TEST(ParseLegacyVegaText, HexColorCarriesValues) {
    const TextLines lines = ParseImGui("#55ffffblue");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "blue");
    EXPECT_TRUE(lines[0].runs[0].style.color.set);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.r), 0x55);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.g), 0xff);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.b), 0xff);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.a), 0xff);
}

TEST(ParseLegacyVegaText, BlackHexResetsToDefaultColour) {
    const TextLines lines = ParseImGui("#FF0000red#000000plain");
    ASSERT_EQ(lines[0].runs.size(), 2u);
    EXPECT_TRUE(lines[0].runs[0].style.color.set);
    EXPECT_FALSE(lines[0].runs[1].style.color.set);
    EXPECT_EQ(lines[0].runs[1].text, "plain");
}

TEST(ParseLegacyVegaText, SixHexBeatsReservedPrefixCodes) {
    // "#b0b0b0" must be a colour, not a '#b' stroke code starting with '0'.
    const TextLines lines = ParseImGui("#b0b0b0x");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_TRUE(lines[0].runs[0].style.color.set);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.r), 0xb0);
    EXPECT_FLOAT_EQ(lines[0].runs[0].style.weight, kWeightNormal);
}

TEST(ParseLegacyVegaText, PushColourCode) {
    const TextLines lines = ParseImGui("#c1:0:0#red");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "red");
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.r), 255);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.g), 0);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.b), 0);
}

TEST(ParseLegacyVegaText, PushColourCodeWithAlpha) {
    const TextLines lines = ParseImGui("#c1:0:0:0.5#x");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(static_cast<int>(lines[0].runs[0].style.color.a), 128);
}

TEST(ParseLegacyVegaText, PushStrokeCodeDefaultsToBold) {
    const TextLines lines = ParseImGui("a#b#bold");
    ASSERT_EQ(lines[0].runs.size(), 2u);
    EXPECT_FLOAT_EQ(lines[0].runs[0].style.weight, kWeightNormal);
    EXPECT_FLOAT_EQ(lines[0].runs[1].style.weight, kWeightBold);
    EXPECT_TRUE(IsBoldWeight(lines[0].runs[1].style.weight));
    EXPECT_EQ(lines[0].runs[1].text, "bold");
}

TEST(ParseLegacyVegaText, PushStrokeCodeWithWeight) {
    const TextLines lines = ParseImGui("#b2.0#x");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_FLOAT_EQ(lines[0].runs[0].style.weight, 2.0f);
}

TEST(ParseLegacyVegaText, PopStrokeCode) {
    const TextLines lines = ParseImGui("#b#bold#-bnormal");
    ASSERT_EQ(lines[0].runs.size(), 2u);
    EXPECT_FLOAT_EQ(lines[0].runs[0].style.weight, kWeightBold);
    EXPECT_FLOAT_EQ(lines[0].runs[1].style.weight, kWeightNormal);
    EXPECT_EQ(lines[0].runs[1].text, "normal");
}

TEST(ParseLegacyVegaText, ResetStrokeCode) {
    const TextLines lines = ParseImGui("#b#bold#!bnormal");
    ASSERT_EQ(lines[0].runs.size(), 2u);
    EXPECT_FLOAT_EQ(lines[0].runs[0].style.weight, kWeightBold);
    EXPECT_FLOAT_EQ(lines[0].runs[1].style.weight, kWeightNormal);
}

TEST(ParseLegacyVegaText, PopColourCode) {
    const TextLines lines = ParseImGui("#c1:0:0#red#-cplain");
    ASSERT_EQ(lines[0].runs.size(), 2u);
    EXPECT_TRUE(lines[0].runs[0].style.color.set);
    EXPECT_FALSE(lines[0].runs[1].style.color.set);
    EXPECT_EQ(lines[0].runs[1].text, "plain");
}

TEST(ParseLegacyVegaText, DoubleHashIsLiteralHash) {
    const TextLines lines = ParseImGui("a##b");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "a#b");
}

TEST(ParseLegacyVegaText, UnknownHashIsLiteralText) {
    const TextLines lines = ParseImGui("Ship #4");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "Ship #4");
}

TEST(ParseLegacyVegaText, NewlineBreaksLine) {
    const TextLines lines = ParseImGui("a\nb");
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_TRUE(lines[0].manual_break);
    EXPECT_EQ(lines[0].runs[0].text, "a");
    EXPECT_FALSE(lines[1].manual_break);
    EXPECT_EQ(lines[1].runs[0].text, "b");
}

TEST(ParseLegacyVegaText, ImGuiHashNBreaksLine) {
    const TextLines lines = ParseImGui("a#n#b");
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_TRUE(lines[0].manual_break);
    EXPECT_EQ(lines[0].runs[0].text, "a");
    EXPECT_EQ(lines[1].runs[0].text, "b");
}

TEST(ParseLegacyVegaText, HashNCarriesLineSpacing) {
    const TextLines lines = ParseImGui("a#n0.5#b");
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_FLOAT_EQ(lines[0].line_spacing, 0.5f);
    EXPECT_FLOAT_EQ(lines[1].line_spacing, 0.0f);
}

TEST(ParseLegacyVegaText, PermanentLineSpacing) {
    const TextLines lines = ParseImGui("#l0.25#a\nb");
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_FLOAT_EQ(lines[0].line_spacing, 0.25f);
    EXPECT_FLOAT_EQ(lines[1].line_spacing, 0.25f);
}

TEST(ParseLegacyVegaText, ImGuiBackslashIsALineBreak) {
    // "a\nb" written with a JSON-escaped newline reaches the parser as the four
    // characters a, backslash, n, b; the legacy ImGuiText parser broke the line.
    const TextLines lines = ParseImGui("a\\nb");
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0].runs[0].text, "a");
    EXPECT_EQ(lines[1].runs[0].text, "b");
}

TEST(ParseLegacyVegaText, TextPlaneBackslashIsLiteral) {
    const TextLines lines = ParseTextPlane("a\\nb");
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "a\\nb");
}

TEST(ParseLegacyVegaText, TextPlaneUnderscoreBecomesSpace) {
    const TextLines lines = ParseTextPlane("hello_world");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "hello world");
}

TEST(ParseLegacyVegaText, ImGuiUnderscoreIsLiteral) {
    const TextLines lines = ParseImGui("hello_world");
    ASSERT_EQ(lines[0].runs.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "hello_world");
}

TEST(ParseLegacyVegaText, TextPlaneResetsColourAtLineBreak) {
    const TextLines lines = ParseTextPlane("#FF0000red\nplain");
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_TRUE(lines[0].runs[0].style.color.set);
    EXPECT_FALSE(lines[1].runs[0].style.color.set);
}

TEST(ParseLegacyVegaText, ImGuiKeepsColourAcrossLineBreak) {
    const TextLines lines = ParseImGui("#FF0000red\nplain");
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_TRUE(lines[0].runs[0].style.color.set);
    EXPECT_TRUE(lines[1].runs[0].style.color.set);
}

TEST(ParseLegacyVegaText, TrailingBreakDoesNotAddEmptyLine) {
    const TextLines lines = ParseImGui("a\n");
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0].runs[0].text, "a");
}
