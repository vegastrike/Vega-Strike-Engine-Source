/*
 * text_layout_tests.cpp
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

#include "vega_draw/markup.h"
#include "vega_draw/text_layout.h"

using namespace vega_draw;

namespace {

// Deterministic measurer for tests: every char is 0.5 * font_px wide, one font_px tall.
class FakeMeasurer : public TextMeasurer {
public:
    TextMetrics Measure(const std::string &text, float font_px) const override {
        TextMetrics m;
        m.width = static_cast<float>(text.size()) * font_px * 0.5f;
        m.height = font_px;
        return m;
    }
};

// Concatenated text of a laid-out line (a wrapped line is split into tokens).
std::string LineText(const LaidOutLine &line) {
    std::string text;
    for (std::size_t i = 0; i < line.runs.size(); ++i) {
        text += line.runs[i].text;
    }
    return text;
}

} // namespace

TEST(TextLayout, UnwrappedLineKeepsRunsAndAdvances) {
    const FakeMeasurer measurer;
    TextStyle style;
    style.font_px = 10.0f; // char width 5

    const TextLayout layout = LayoutText(ParseMarkup("hello"), style, measurer);
    ASSERT_EQ(layout.lines.size(), 1u);
    ASSERT_EQ(layout.lines[0].runs.size(), 1u);
    EXPECT_EQ(layout.lines[0].runs[0].text, "hello");
    EXPECT_FLOAT_EQ(layout.lines[0].runs[0].x, 0.0f);
    EXPECT_FLOAT_EQ(layout.lines[0].runs[0].width, 25.0f);
    EXPECT_FLOAT_EQ(layout.lines[0].width, 25.0f);
    EXPECT_FLOAT_EQ(layout.lines[0].height, 10.0f);
    EXPECT_FLOAT_EQ(layout.height, 10.0f);
}

TEST(TextLayout, HardBreakIsHonoured) {
    const FakeMeasurer measurer;
    TextStyle style;
    style.font_px = 10.0f;

    const TextLayout layout = LayoutText(ParseMarkup("a<br>b"), style, measurer);
    ASSERT_EQ(layout.lines.size(), 2u);
    EXPECT_EQ(layout.lines[0].runs[0].text, "a");
    EXPECT_EQ(layout.lines[1].runs[0].text, "b");
    EXPECT_FLOAT_EQ(layout.lines[1].y, 10.0f);
    EXPECT_FLOAT_EQ(layout.height, 20.0f);
}

TEST(TextLayout, WordWrapSplitsAtSpaces) {
    const FakeMeasurer measurer;
    TextStyle style;
    style.font_px = 10.0f;      // char width 5
    style.wrap = true;
    style.wrap_width_px = 25.0f; // 5 chars

    const TextLayout layout = LayoutText(ParseMarkup("aaaaa bbbbb"), style, measurer);
    ASSERT_EQ(layout.lines.size(), 2u);
    EXPECT_EQ(layout.lines[0].runs[0].text, "aaaaa");
    EXPECT_EQ(layout.lines[1].runs[0].text, "bbbbb");
    EXPECT_FLOAT_EQ(layout.lines[1].y, 10.0f);
}

TEST(TextLayout, WrapTrimsTrailingSpaceAndDoesNotStartWithSpace) {
    const FakeMeasurer measurer;
    TextStyle style;
    style.font_px = 10.0f;
    style.wrap = true;
    style.wrap_width_px = 25.0f;

    const TextLayout layout = LayoutText(ParseMarkup("aaaa bbbb"), style, measurer);
    ASSERT_EQ(layout.lines.size(), 2u);
    ASSERT_EQ(layout.lines[0].runs.size(), 1u);
    EXPECT_EQ(layout.lines[0].runs[0].text, "aaaa");
    ASSERT_EQ(layout.lines[1].runs.size(), 1u);
    EXPECT_EQ(layout.lines[1].runs[0].text, "bbbb");
}

TEST(TextLayout, WrapDoesNotReflowManualBreak) {
    const FakeMeasurer measurer;
    TextStyle style;
    style.font_px = 10.0f;
    style.wrap = true;
    style.wrap_width_px = 100.0f; // wide enough that only <br> splits

    const TextLayout layout = LayoutText(ParseMarkup("one two<br>three four"), style, measurer);
    ASSERT_EQ(layout.lines.size(), 2u);
    EXPECT_EQ(LineText(layout.lines[0]), "one two"); // not reflowed
    EXPECT_EQ(LineText(layout.lines[1]), "three four");
}

TEST(TextLayout, ColorStyleSurvivesWrap) {
    const FakeMeasurer measurer;
    TextStyle style;
    style.font_px = 10.0f;
    style.wrap = true;
    style.wrap_width_px = 15.0f; // 3 chars

    const TextLayout layout = LayoutText(ParseMarkup("<color=ff0000>aa</color>"), style, measurer);
    ASSERT_EQ(layout.lines.size(), 1u);
    ASSERT_EQ(layout.lines[0].runs.size(), 1u);
    EXPECT_TRUE(layout.lines[0].runs[0].style.color.set);
    EXPECT_EQ(static_cast<int>(layout.lines[0].runs[0].style.color.r), 0xff);
}

TEST(TextLayout, CenterJustificationShiftsWithinRegion) {
    const FakeMeasurer measurer;
    TextStyle style;
    style.font_px = 10.0f;
    style.region_width_px = 30.0f;
    style.justification = Justification::Center;

    const TextLayout layout = LayoutText(ParseMarkup("aa"), style, measurer); // width 10
    ASSERT_EQ(layout.lines.size(), 1u);
    EXPECT_FLOAT_EQ(layout.lines[0].runs[0].x, 10.0f); // (30 - 10) * 0.5
}

TEST(TextLayout, RightJustificationShiftsWithinRegion) {
    const FakeMeasurer measurer;
    TextStyle style;
    style.font_px = 10.0f;
    style.region_width_px = 30.0f;
    style.justification = Justification::Right;

    const TextLayout layout = LayoutText(ParseMarkup("aa"), style, measurer); // width 10
    ASSERT_EQ(layout.lines.size(), 1u);
    EXPECT_FLOAT_EQ(layout.lines[0].runs[0].x, 20.0f); // 30 - 10
}
