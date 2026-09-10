/*
 * sprite.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_SPRITE_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_SPRITE_H

#include <cstdint>

#include "vega_draw/markup.h"
#include "vega_draw/widget.h"

namespace vega_draw {

// An opaque backend texture handle (0 = none). For the ImGui/OpenGL backend this
// is the GL texture name; the adapter wraps it into an ImTextureRef.
using TextureId = std::uintptr_t;

// A texture plus the sub-rectangle (UV) to draw. Default UV is the whole texture
// with the origin at the top-left (ImGui convention); callers flip V for
// bottom-left textures.
struct TextureRegion {
    TextureId texture = 0;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
};

// A container that draws a texture behind its children (the base-computer window
// backgrounds). Falls back to a solid colour when no texture is set.
class TexturePanel : public Container {
public:
    void setTexture(const TextureRegion &texture) {
        m_texture = texture;
    }
    TextureRegion &texture() {
        return m_texture;
    }

    void setTint(const Color &tint) {
        m_tint = tint;
    }

    // Solid background used when no texture is set (and drawn under the texture).
    Color &background() {
        return m_background;
    }

    void draw(ImDrawList *draw_list, const Viewport &viewport) override;

private:
    TextureRegion m_texture;
    Color m_tint{255, 255, 255, 255, true};
    Color m_background{0, 0, 0, 0, true};
};

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_SPRITE_H
