/*
 * button.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_BUTTON_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_BUTTON_H

#include <string>

#include "vega_draw/markup.h"
#include "vega_draw/widget.h"

namespace vega_draw {

struct ButtonStyle {
    Color face{96, 96, 108, 255, true};
    Color down{64, 64, 76, 255, true};
    Color highlight{128, 128, 144, 255, true};
    Color border{160, 160, 176, 255, true};
    Color text{255, 255, 255, 255, true};
    Color down_text{255, 255, 255, 255, true};
    Color highlight_text{255, 255, 255, 255, true};
    Color shadow_light{255, 255, 255, 120, true};
    Color shadow_dark{0, 0, 0, 160, true};
    float font_grid = 20.0f; // per-1000
    float weight = kWeightNormal;
    float border_px = 1.0f;
    float shadow_px = 1.0f;
};

// A push button: chrome (fill/border/beam shadows) with a centred mark-up label.
// On release over the button it fires its command to its command target, or
// bubbles the command to its parent.
class PushButton : public Widget {
public:
    void setLabel(const std::string &label) {
        m_label = label;
    }
    const std::string &label() const {
        return m_label;
    }

    ButtonStyle &style() {
        return m_style;
    }
    void setStyle(const ButtonStyle &style) {
        m_style = style;
    }

    void setCommand(const EventCommand &command) {
        m_command = command;
    }
    void setCommandTarget(Widget *target) {
        m_target = target;
    }
    void setEnabled(bool enabled) {
        m_enabled = enabled;
    }
    bool enabled() const {
        return m_enabled;
    }
    bool pressed() const {
        return m_down;
    }

    void draw(ImDrawList *draw_list, const Viewport &viewport) override;
    bool onMouseDown(const InputEvent &event) override;
    bool onMouseUp(const InputEvent &event) override;
    bool onMouseMove(const InputEvent &event) override;

private:
    void fireCommand();

    std::string m_label;
    ButtonStyle m_style;
    EventCommand m_command;
    Widget *m_target = nullptr;
    bool m_down = false;
    bool m_highlight = false;
    bool m_enabled = true;
};

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_BUTTON_H
