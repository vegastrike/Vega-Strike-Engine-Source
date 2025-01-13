/*
 * stream_texture.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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


#include "stream_texture.h"

StreamTexture *StreamTexture::Clone() {
    unsigned char *x = Map();
    StreamTexture *ret = new StreamTexture(sizeX, sizeY, filtertype, x);
    UnMap(false);
    return ret;
}

StreamTexture::StreamTexture(int width, int height, enum FILTER filtertype, unsigned char *origdata) {
    /*
     *  img_type=Unrecognized;
     *  img_depth=8;
     *  img_color_type=(PNG_HAS_COLOR&PNG_HAS_ALPHA);
     *  img_alpha=true;
     *  strip_16=true;
     */
    mode = _24BITRGBA;
    this->stage = 0;
    this->sizeX = width;
    this->sizeY = height;
    this->original = NULL;
    this->palette = NULL;
    this->refcount = 0;
    texture_target = TEXTURE2D;
    image_target = TEXTURE_2D;
    ismipmapped = filtertype;
    GFXCreateTexture(width, height, RGBA32, &name, 0, 0, filtertype);
    this->mutabledata = (unsigned char *) malloc(sizeof(unsigned char) * width * height * 4);
    if (origdata) {
        memcpy(this->mutabledata, origdata, sizeof(unsigned char) * width * height * 4);
        GFXTransferTexture(mutabledata, name, sizeX, sizeY, RGBA32);
    }
}

unsigned char *StreamTexture::Map() {
    return mutabledata;
}

void StreamTexture::UnMap(bool changed) {
    if (changed) {
        MakeActive(0);
        GFXTransferTexture(mutabledata, name, sizeX, sizeY, RGBA32);
    }
}

StreamTexture::~StreamTexture() {
    GFXDeleteTexture(name);
    name = -1;
    if (this->mutabledata != nullptr) {
        free(this->mutabledata);
        this->mutabledata = nullptr;
    }
}

void StreamTexture::MakeActive(int stage) {
    GFXSelectTexture(name, stage);
}

