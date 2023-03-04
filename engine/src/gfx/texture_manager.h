/*
 * texture_manager.h
 *
 * Copyright (C) 2001-2022 Daniel Horn, Roy Falk. Stephen G. Tuggy,
 * and other Vega Strike contributors
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


#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <vector>

#include "gfx/aux_texture.h"

class TextureManager {
    vega_types::SequenceContainer<vega_types::SharedPtr<Texture>> textures;
public:
    TextureManager();

    static TextureManager &GetInstance();
    vega_types::SharedPtr<Texture> GetTexture(std::string const &name, enum FILTER mipmap);
};

#endif // TEXTUREMANAGER_H
