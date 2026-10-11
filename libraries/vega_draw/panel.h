/*
 * panel.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_PANEL_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_PANEL_H

#include "vega_draw/markup.h"
#include "vega_draw/widget.h"

namespace vega_draw {

struct PanelStyle {
    Color background{0, 0, 0, 160, true};
    Color border{0, 0, 0, 0, true}; // clear -> no border
    float border_px = 1.0f;
};

// A container that draws a background (and optional border) behind its children.
class Panel : public Container {
public:
    PanelStyle &style() {
        return m_style;
    }
    void setStyle(const PanelStyle &style) {
        m_style = style;
    }

    void draw(ImDrawList *draw_list, const Viewport &viewport) override;

private:
    PanelStyle m_style;
};

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_PANEL_H
