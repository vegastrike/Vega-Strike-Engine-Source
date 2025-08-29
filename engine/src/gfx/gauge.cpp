/*
 * gauge.cpp
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


#include "gfx/gauge.h"

Gauge::Gauge(const char *file, DIRECTION up) : VSSprite(file) {
    dir = up;
    float sx, sy;
    VSSprite::GetSize(sx, sy);
    SetSize(-sx, -sy);
}

void Gauge::SetSize(float x, float y) {
    if (dir == GAUGE_UP) {
        y = -y;
    } else if (dir == GAUGE_LEFT) {
        x = -x;
    }
    VSSprite::SetSize(x, y);
}

void Gauge::GetSize(float &x, float &y) {
    VSSprite::GetSize(x, y);
    if (dir == GAUGE_UP) {
        y = -y;
    } else if (dir == GAUGE_LEFT) {
        x = -x;
    }
}

Gauge::~Gauge() = default;

void Gauge::Draw(float percentage) {
    if (percentage < 0) {
        percentage = 0;
    } else if (percentage > 1) {
        percentage = 1;
    }
    float sx, sy, px, py;
    VSSprite::GetSize(sx, sy);
    VSSprite::GetPosition(px, py);
    switch (dir) {
        case GAUGE_RIGHT:
        case GAUGE_LEFT:
            SetST((1 - percentage), 0);
            VSSprite::SetSize(sx * percentage, sy);
            VSSprite::SetPosition(px + sx * (1 - percentage) * .5, py);
            break;
        case GAUGE_DOWN:
        case GAUGE_UP:
            SetST(0, (1 - percentage));
            VSSprite::SetSize(sx, sy * percentage);
            VSSprite::SetPosition(px, py + sy * (1 - percentage) * .5);
            break;
        case GAUGE_TIME:
            VSSprite::SetTime(percentage);
            break;
    }
    VSSprite::Draw();
    VSSprite::SetSize(sx, sy);
    VSSprite::SetPosition(px, py);
}

