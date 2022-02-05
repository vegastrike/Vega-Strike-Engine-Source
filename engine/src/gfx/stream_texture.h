/**
 * stream_texture.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#include "aux_texture.h"

class StreamTexture : public Texture {
    unsigned char *mutabledata;
    FILTER filtertype;
    int handle;
public:
    StreamTexture(int width, int height, enum FILTER filtertype, unsigned char *origdata);
    ~StreamTexture();
    unsigned char *Map();
    void UnMap(bool changed = true);
    void MakeActive(int stage);

    void MakeActive()
    {
        MakeActive(this->stage);
    }

    StreamTexture *Clone();
};

