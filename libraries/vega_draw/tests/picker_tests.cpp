/*
 * picker_tests.cpp
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

#include "vega_draw/picker.h"

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

const Viewport kUnit{1000.0f, 1000.0f};

PickerStyle BaseStyle() {
    PickerStyle style;
    style.region_w = 1000.0f;
    style.region_h = 1000.0f;
    style.font_grid = 10.0f;
    style.row_padding_grid = 0.0f;
    style.indent_grid = 20.0f;
    return style;
}

} // namespace

TEST(Picker, RowsStackWithHeights) {
    const FakeMeasurer measurer;
    const PickerStyle style = BaseStyle();
    const std::vector<PickerRow> rows = {{"a", 0, Color{}}, {"bb", 0, Color{}}};

    const PickerLayout layout = LayoutPicker(style, rows, kUnit, measurer);
    ASSERT_EQ(layout.visible_rows.size(), 2u);
    EXPECT_FLOAT_EQ(layout.visible_rows[0].y, 0.0f);
    EXPECT_FLOAT_EQ(layout.visible_rows[0].height, 10.0f);
    EXPECT_FLOAT_EQ(layout.visible_rows[1].y, 10.0f);
    EXPECT_FLOAT_EQ(layout.content_height, 20.0f);
}

TEST(Picker, IndentByLevel) {
    const FakeMeasurer measurer;
    const PickerStyle style = BaseStyle();
    const std::vector<PickerRow> rows = {{"a", 1, Color{}}, {"b", 2, Color{}}};

    const PickerLayout layout = LayoutPicker(style, rows, kUnit, measurer);
    ASSERT_EQ(layout.visible_rows.size(), 2u);
    EXPECT_FLOAT_EQ(layout.visible_rows[0].indent_px, 20.0f);
    EXPECT_FLOAT_EQ(layout.visible_rows[1].indent_px, 40.0f);
}

TEST(Picker, PaddingIncreasesRowHeight) {
    const FakeMeasurer measurer;
    PickerStyle style = BaseStyle();
    style.row_padding_grid = 4.0f;

    const std::vector<PickerRow> rows = {{"a", 0, Color{}}};
    const PickerLayout layout = LayoutPicker(style, rows, kUnit, measurer);
    ASSERT_EQ(layout.visible_rows.size(), 1u);
    EXPECT_FLOAT_EQ(layout.visible_rows[0].height, 18.0f); // 10 text + 2 * 4 padding
    EXPECT_FLOAT_EQ(layout.visible_rows[0].padding_px, 4.0f);
}

TEST(Picker, VisibleRowsRespectViewportAndScroll) {
    const FakeMeasurer measurer;
    PickerStyle style = BaseStyle();
    style.region_h = 10.0f; // viewport fits one 10px row
    const std::vector<PickerRow> rows = {{"a", 0, Color{}}, {"b", 0, Color{}}};

    const PickerLayout at_top = LayoutPicker(style, rows, kUnit, measurer, 0.0f);
    ASSERT_EQ(at_top.visible_rows.size(), 1u);
    EXPECT_EQ(at_top.visible_rows[0].index, 0);

    const PickerLayout scrolled = LayoutPicker(style, rows, kUnit, measurer, 10.0f);
    ASSERT_EQ(scrolled.visible_rows.size(), 1u);
    EXPECT_EQ(scrolled.visible_rows[0].index, 1);
}

TEST(Picker, ScrollIsClampedToContent) {
    const FakeMeasurer measurer;
    PickerStyle style = BaseStyle();
    style.region_h = 10.0f;
    const std::vector<PickerRow> rows = {{"a", 0, Color{}}, {"b", 0, Color{}}};

    const PickerLayout layout = LayoutPicker(style, rows, kUnit, measurer, 9999.0f);
    EXPECT_FLOAT_EQ(layout.max_scroll_px, 10.0f);
    EXPECT_FLOAT_EQ(layout.scroll_px, 10.0f);
}

TEST(Picker, WrapRowsGrowHeight) {
    const FakeMeasurer measurer;
    PickerStyle style = BaseStyle();
    style.region_w = 30.0f; // inner width 30 -> 6 chars per line at font 10
    style.wrap_rows = true;

    const std::vector<PickerRow> rows = {{"aaaaaa bbbbbb", 0, Color{}}};
    const PickerLayout layout = LayoutPicker(style, rows, kUnit, measurer);
    ASSERT_EQ(layout.visible_rows.size(), 1u);
    EXPECT_FLOAT_EQ(layout.visible_rows[0].height, 20.0f); // two wrapped lines
}

TEST(Picker, NonWrappingRowStaysOneLine) {
    const FakeMeasurer measurer;
    PickerStyle style = BaseStyle();
    style.region_w = 30.0f;
    style.wrap_rows = false;

    const std::vector<PickerRow> rows = {{"aaaaaa bbbbbb", 0, Color{}}};
    const PickerLayout layout = LayoutPicker(style, rows, kUnit, measurer);
    ASSERT_EQ(layout.visible_rows.size(), 1u);
    EXPECT_FLOAT_EQ(layout.visible_rows[0].height, 10.0f);
}
