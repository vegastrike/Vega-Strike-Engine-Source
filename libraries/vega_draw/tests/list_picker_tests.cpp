/*
 * list_picker_tests.cpp
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

#include "vega_draw/list_picker.h"

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
