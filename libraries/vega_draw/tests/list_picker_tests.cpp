/*
 * list_picker_tests.cpp
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

#include "vega_draw/list_picker.h"
#include "vega_draw/picker.h"

using namespace vega_draw;

namespace {

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

ListPicker MakePicker() {
    ListPicker picker;
    picker.setRect(Rect{0.0f, 0.0f, 1000.0f, 1000.0f});
    picker.style().font_grid = 10.0f; // line height 10
    picker.style().row_padding_grid = 0.0f;
    picker.rows() = {{"a", 0, Color{}}, {"b", 0, Color{}}};
    return picker;
}

} // namespace

TEST(ListPicker, RowAtMapsPointToRow) {
    const FakeMeasurer measurer;
    const ListPicker picker = MakePicker();
    EXPECT_EQ(picker.rowAt(Point{150.0f, 5.0f}, kUnit, measurer), 0);
    EXPECT_EQ(picker.rowAt(Point{150.0f, 15.0f}, kUnit, measurer), 1);
    EXPECT_EQ(picker.rowAt(Point{150.0f, 25.0f}, kUnit, measurer), -1);
}

TEST(ListPicker, SelectionIsSetByIndex) {
    ListPicker picker = MakePicker();
    picker.setSelectedIndex(1);
    EXPECT_EQ(picker.selectedIndex(), 1);
}

// A row whose text wraps must be given the height of every line it wrapped to, and the rows must
// not overlap. This is the bug the main menu's Introduction text shows: the text wraps, no
// vertical space is allowed for the extra lines, and the wrapped remainder is drawn over the row
// below it.
TEST(ListPicker, WrappedRowGetsTheHeightOfAllItsLines) {
    const FakeMeasurer measurer;
    ListPicker picker;
    picker.setRect(Rect{0.0f, 0.0f, 1000.0f, 1000.0f});
    picker.style().font_grid = 10.0f;      // line height 10, char width 5
    picker.style().row_padding_grid = 0.0f;
    picker.style().wrap_rows = true;
    picker.style().region_w = 100.0f;      // a row of eight words is ~175px, so it must wrap
    picker.rows() = {
        {"short", 0, Color{}},
        {"aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa", 0, Color{}},
    };

    const PickerLayout layout = LayoutPicker(picker.style(), picker.rows(), kUnit, measurer, 0.0f);

    ASSERT_EQ(layout.visible_rows.size(), 2u);
    const PickerRowLayout &single = layout.visible_rows[0];
    const PickerRowLayout &wrapped = layout.visible_rows[1];
    EXPECT_FLOAT_EQ(single.height, 10.0f);                // one line is one line high
    EXPECT_GT(wrapped.text.lines.size(), 1u);             // the long row wrapped
    EXPECT_GT(wrapped.height, single.height);             // and was given all of its lines
    EXPECT_GE(wrapped.y, single.y + single.height);       // so it clears the row above it
    EXPECT_FLOAT_EQ(layout.content_height, single.height + wrapped.height);
}

// With wrapping off, a row is a single line - what a caller that does not want variable-height
// rows is asking for.
TEST(ListPicker, RowsAreASingleLineWhenWrappingIsOff) {
    const FakeMeasurer measurer;
    ListPicker picker;
    picker.setRect(Rect{0.0f, 0.0f, 1000.0f, 1000.0f});
    picker.style().font_grid = 10.0f;
    picker.style().row_padding_grid = 0.0f;
    picker.rows() = {
        {"short", 0, Color{}},
        {"aaaa aaaa aaaa aaaa aaaa aaaa aaaa aaaa", 0, Color{}},
    };

    const PickerLayout layout = LayoutPicker(picker.style(), picker.rows(), kUnit, measurer, 0.0f);

    ASSERT_EQ(layout.visible_rows.size(), 2u);
    EXPECT_EQ(layout.visible_rows[0].text.lines.size(), 1u);
    EXPECT_EQ(layout.visible_rows[1].text.lines.size(), 1u); // not wrapped: one line
    EXPECT_FLOAT_EQ(layout.visible_rows[1].height, layout.visible_rows[0].height);
}
