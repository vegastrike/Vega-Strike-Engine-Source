/*
 * texture_manager.cpp
 *
 * Copyright (C) 2021 Roy Falk
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#ifndef VEGA_STRIKE_ENGINE_GFX_TEXTURE_MANAGER_H
#define VEGA_STRIKE_ENGINE_GFX_TEXTURE_MANAGER_H

#include <vector>

#include "gfx/aux_texture.h"

class TextureManager {
    std::vector<Texture *> textures;
public:
    TextureManager();

    static TextureManager &GetInstance();
    Texture *GetTexture(std::string const &name, enum FILTER mipmap);
};

#endif //VEGA_STRIKE_ENGINE_GFX_TEXTURE_MANAGER_H
