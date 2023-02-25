/*
 * texture_manager.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, Roy Falk. Stephen G. Tuggy,
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


#include "animation.h"
#include "texture_manager.h"
#include "preferred_types.h"
#include "vega_cast_utils.h"

TextureManager::TextureManager() {

}

TextureManager &TextureManager::GetInstance() {
    static TextureManager instance;     // Guaranteed to be destroyed.
    return instance;                    // Instantiated on first use.
}

vega_types::SharedPtr<Texture> TextureManager::GetTexture(std::string const &name, enum FILTER mipmap) {
    // This is weird. We already store textures in a hashmap but can't rely on it.
    // TODO: figure out how to rely on it.
    vega_types::SharedPtr<Texture> texture = Texture::Exists(name);

    // Texture already exists
    if (!texture) {
        // Need to create texture
        texture = Texture::createTexture(name.c_str(), 0, mipmap, TEXTURE2D, TEXTURE_2D, GFXTRUE);
        textures.emplace_back(texture);
    }

    return texture;
}
