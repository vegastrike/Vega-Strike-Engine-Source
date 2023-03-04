/*
 * pipelined_texture.h
 *
 * Copyright (c) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike Contributors
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


#ifndef __PIPELINED_TEXTURE_H__INCLUDED_
#define __PIPELINED_TEXTURE_H__INCLUDED_

#include "aux_texture.h"

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

    virtual vega_types::SharedPtr<Texture> Clone();
    const vega_types::SharedPtr<const Texture> OriginalConst() const override;
    vega_types::SharedPtr<Texture> Original() override;
    PipelinedTexture();
    PipelinedTexture(unsigned int width, unsigned int height, unsigned char *current, unsigned char *last);
    ~PipelinedTexture() override;
    unsigned char *beginMutate();
    void Swap();
    void endMutate(int xoffset, int yoffset, int width, int height);
    void MakeBothActive();
    void MakeActive();
    void MakeLastActive();
};

#endif

