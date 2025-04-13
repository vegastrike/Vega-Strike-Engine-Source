/*
 * texture_manager.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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


#include "texture_manager.h"

TextureManager::TextureManager() {

}

TextureManager &TextureManager::GetInstance() {
    static TextureManager instance;     // Guaranteed to be destroyed.
    return instance;                    // Instantiated on first use.
}

Texture *TextureManager::GetTexture(std::string const &name, enum FILTER mipmap) {
    // This is weird. We already store textures in a hashmap but can't rely on it.
    // TODO: figure out how to rely on it.
    Texture *texture = Texture::Exists(name);

    // Texture already exists
    if (!texture) {
        // Need to create texture
        texture = new Texture(name.c_str(), 0, mipmap, TEXTURE2D, TEXTURE_2D, GFXTRUE);
        textures.push_back(texture);
    }

    return texture;
}
