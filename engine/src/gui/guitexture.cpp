/*
 * guitexture.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: David Ranger, Mike Byron
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


#include "src/vegastrike.h"

#include "guitexture.h"

#include <gnuhash.h>

#include <string>
#include <png.h>
#include "vegadisk/vsfilesystem.h"
#include "gfx/vsimage.h"
#include "gldrv/gl_globals.h"
#include "gfx/aux_texture.h"
#include "imguitext.h"

using namespace VSFileSystem;

//Read a texture from a file and bind it.
bool GuiTexture::read(const std::string &fileName) {
    Texture *oldTexture(m_texture);
    m_texture = new Texture(fileName.c_str(), 0, BILINEAR);
    if (m_texture && !m_texture->LoadSuccess() && oldTexture) {
        delete m_texture;
        m_texture = oldTexture;
    } else {
        delete oldTexture;
    }
    return m_texture->LoadSuccess();
}

//Draw this texture, stretching to fit the rect.
void GuiTexture::draw(const Rect &rect) const {
    // Don't draw unless there is something usable.
    if (m_texture == NULL || !m_texture->LoadSuccess()) {
        return;
    }
    m_texture->MakeActive();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (!draw_list) return;

    // Convert normalized Vega Strike coordinates to pixels
    float pMinX = NORM_TO_PIXEL_X(rect.left());
    float pMinY = NORM_TO_PIXEL_Y(rect.top());
    float pMaxX = NORM_TO_PIXEL_X(rect.right());
    float pMaxY = NORM_TO_PIXEL_Y(rect.bottom());

    // Extract the OpenGL texture ID
    GLint actual_gl_id = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &actual_gl_id);

    // Guard against invalid/unbound textures
    if (actual_gl_id <= 0) {
        return;
    }
    ImTextureID texID = (ImTextureID)(uintptr_t)actual_gl_id;

    // Map UV coordinates to match original vertex layout
    ImVec2 uv_min(0.0f, 1.0f);
    ImVec2 uv_max(1.0f, 0.0f);

    // Default white tint (equivalent to GFXColor4f(1, 1, 1, 1))
    ImU32 tintColor = IM_COL32(255, 255, 255, 255);

    // Submit to ImGui draw list instead of immediate-mode GFXDraw
    draw_list->AddImage(
        texID, 
        ImVec2(pMinX, pMinY), 
        ImVec2(pMaxX, pMaxY), 
        uv_min, 
        uv_max, 
        tintColor
    );
}

//CONSTRUCTION
GuiTexture::GuiTexture(void) :
        m_texture(nullptr) {
}

GuiTexture::~GuiTexture(void) {
    if (m_texture != nullptr) {
        delete m_texture;
        m_texture = nullptr;
    }
}

