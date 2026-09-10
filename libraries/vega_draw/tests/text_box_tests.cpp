/*
 * text_box_tests.cpp
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

#include "vega_draw/text_box.h"

using namespace vega_draw;

namespace {

// Deterministic measurer: every char is 0.5 * font_px wide, one font_px tall.
class FakeMeasurer : public TextMeasurer {
public:
    TextMetrics Measure(const std::string &text, float font_px) const override {
        TextMetrics m;
        m.width = static_cast<float>(text.size()) * font_px * 0.5f;
        m.height = font_px;
        return m;
    }
};

// A viewport where 1 grid unit == 1 pixel, to keep expectations obvious.
const Viewport kUnit{1000.0f, 1000.0f};

} // namespace

TEST(TextBox, InnerRectAppliesMargins) {
    TextBox box;
    box.region_x = 0.0f;
    box.region_y = 0.0f;
    box.region_w = 1000.0f;
    box.region_h = 1000.0f;
    box.margin_x = 100.0f;
    box.margin_y = 50.0f;

    const BoxRect inner = TextBoxInnerRectPx(box, kUnit);
    EXPECT_FLOAT_EQ(inner.x, 100.0f);
    EXPECT_FLOAT_EQ(inner.y, 50.0f);
    EXPECT_FLOAT_EQ(inner.width, 800.0f);
    EXPECT_FLOAT_EQ(inner.height, 900.0f);
}

TEST(TextBox, WrapUsesInnerRegionWidth) {
    const FakeMeasurer measurer;
    TextBox box;
    box.region_w = 50.0f; // inner width 50 px -> 10 chars at font 10
    box.region_h = 1000.0f;
    box.font_grid = 10.0f;
    box.wrap = true;

    const TextBoxLayout result = LayoutTextBox(box, "aaaaa bbbbb", kUnit, measurer);
    ASSERT_EQ(result.layout.lines.size(), 2u);
}

TEST(TextBox, WrapsRegardlessOfHugeFontDefault) {
    const FakeMeasurer measurer;
    TextBox box;
    box.region_w = 50.0f;
    box.region_h = 1000.0f;
    box.font_grid = 10.0f;
    box.wrap = false; // no wrap -> one overflowing line

    const TextBoxLayout result = LayoutTextBox(box, "aaaaa bbbbb", kUnit, measurer);
    ASSERT_EQ(result.layout.lines.size(), 1u);
}

TEST(TextBox, VisibleLineCountFitsInnerHeight) {
    const FakeMeasurer measurer;
    TextBox box;
    box.region_h = 25.0f; // font 10 -> line height 10 -> 2 lines fit
    box.font_grid = 10.0f;

    const TextBoxLayout result = LayoutTextBox(box, "a<br>b<br>c", kUnit, measurer);
    ASSERT_EQ(result.layout.lines.size(), 3u);
    EXPECT_EQ(TextBoxVisibleLineCount(box, result.layout, kUnit, 0), 2); // a, b
    EXPECT_EQ(TextBoxVisibleLineCount(box, result.layout, kUnit, 1), 2); // b, c
    EXPECT_EQ(TextBoxVisibleLineCount(box, result.layout, kUnit, 2), 1); // c
}

TEST(TextBox, LineSpacingIncreasesAdvance) {
    const FakeMeasurer measurer;
    TextBox box;
    box.font_grid = 10.0f;
    box.line_spacing = 0.5f; // advance 15

    const TextBoxLayout result = LayoutTextBox(box, "a<br>b", kUnit, measurer);
    ASSERT_EQ(result.layout.lines.size(), 2u);
    EXPECT_FLOAT_EQ(result.layout.lines[1].y, 15.0f);
    EXPECT_FLOAT_EQ(result.layout.height, 30.0f);
}

TEST(TextBox, JustificationUsesInnerWidth) {
    const FakeMeasurer measurer;
    TextBox box;
    box.region_w = 100.0f;
    box.font_grid = 10.0f;
    box.justification = Justification::Center;

    const TextBoxLayout result = LayoutTextBox(box, "aa", kUnit, measurer); // width 10
    ASSERT_EQ(result.layout.lines.size(), 1u);
    EXPECT_FLOAT_EQ(result.layout.lines[0].runs[0].x, 45.0f); // (100 - 10) * 0.5
}

TEST(TextBox, AutofitShrinksFontToFitRegion) {
    const FakeMeasurer measurer;
    TextBox box;
    box.region_h = 20.0f;
    box.font_grid = 100.0f; // would not fit; autofit must shrink it
    box.autofit = true;
    box.autofit_min_grid = 1.0f;
    box.autofit_max_grid = 100.0f;

    const TextBoxLayout result = LayoutTextBox(box, "x", kUnit, measurer); // 1 line, height = font_px
    EXPECT_FLOAT_EQ(result.font_px, 20.0f); // largest font whose line height fits 20 px
}

TEST(TextBox, AutofitOffUsesGivenFont) {
    const FakeMeasurer measurer;
    TextBox box;
    box.region_h = 20.0f;
    box.font_grid = 80.0f;

    const TextBoxLayout result = LayoutTextBox(box, "x", kUnit, measurer);
    EXPECT_FLOAT_EQ(result.font_px, 80.0f);
}

TEST(TextBox, SingleLineBoxShowsOnlyFirstLine) {
    const FakeMeasurer measurer;
    TextBox box;
    box.multiline = false;
    box.font_grid = 10.0f;

    const TextBoxLayout result = LayoutTextBox(box, "a<br>b", kUnit, measurer);
    ASSERT_EQ(result.layout.lines.size(), 1u);
    ASSERT_EQ(result.layout.lines[0].runs.size(), 1u);
    EXPECT_EQ(result.layout.lines[0].runs[0].text, "a");
}
