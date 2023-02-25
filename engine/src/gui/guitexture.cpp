/*
 * guitexture.cpp
 *
 * Copyright (C) 2001-2023 David Ranger, Mike Byron, Daniel Horn,
 * pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "vegastrike.h"

#include "guitexture.h"

#include <gnuhash.h>

#include <string>
#include <png.h>
#include "vsfilesystem.h"
#include "gfx/vsimage.h"
#include "gldrv/gl_globals.h"
#include "gfx/aux_texture.h"

using namespace VSFileSystem;

//Read a texture from a file and bind it.
bool GuiTexture::read(const std::string &fileName) {
    vega_types::SharedPtr<Texture> oldTexture(m_texture);
    m_texture = Texture::createTexture(fileName.c_str(), 0, BILINEAR);
    if (m_texture && !m_texture->LoadSuccess() && oldTexture) {
        m_texture.reset();
        m_texture = oldTexture;
    } else {
        oldTexture.reset();
    }
    return m_texture->LoadSuccess();
}

//Draw this texture, stretching to fit the rect.
void GuiTexture::draw(const Rect &rect) const {
    //Don't draw unless there is something usable.
    if (!m_texture || !m_texture->LoadSuccess()) {
        return;
    }
    m_texture->MakeActive();
    GFXColor4f(1, 1, 1, 1);
    const float verts[4 * (3 + 2)] = {
            rect.left(), rect.top(), 0, 0, 1,
            rect.left(), rect.bottom(), 0, 0, 0,
            rect.right(), rect.bottom(), 0, 1, 0,
            rect.right(), rect.top(), 0, 1, 1,
    };
    GFXDraw(GFXQUAD, verts, 4, 3, 0, 2);
}

//CONSTRUCTION
GuiTexture::GuiTexture() :
        m_texture(nullptr) {
}

GuiTexture::~GuiTexture() = default;

