/**
 * pipelined_texture.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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
#ifndef VEGA_STRIKE_ENGINE_GFX_PIPELINED_TEXTURE_H
#define VEGA_STRIKE_ENGINE_GFX_PIPELINED_TEXTURE_H

#include "gfx/aux_texture.h"

class PipelinedTexture : public Texture {
    int current;
    int last;
//clone==2 means both current and last are clones 1 means last is a clone
    char clone;
public:
    bool operator==(const Texture &b) {
        return false;
    }

    bool operator<(const Texture &b) {
        return true;
    }

    virtual Texture *Clone();
    virtual Texture *Original();
    PipelinedTexture();
    PipelinedTexture(unsigned int width, unsigned int height, unsigned char *current, unsigned char *last);
    ~PipelinedTexture();
    unsigned char *beginMutate();
    void Swap();
    void endMutate(int xoffset, int yoffset, int width, int height);
    void MakeBothActive();
    void MakeActive();
    void MakeLastActive();
};

#endif //VEGA_STRIKE_ENGINE_GFX_PIPELINED_TEXTURE_H
