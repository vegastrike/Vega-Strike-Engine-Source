/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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


#include "gfxlib.h"
#include "gl_globals.h"
static struct _GFXFogState {
    bool enabled;
    FOGMODE curmode;
    float density;
    float nnear;
    float ffar;
    GFXColor c;
    int index;

    _GFXFogState() : c(0, 0, 0, 0)
    {
        index = 0;
        nnear = 0;
        ffar = 1;
        density = 1;
        curmode = FOG_EXP;
    }
}
        GFXFogState;

void GFXFogMode(const FOGMODE fog)
{
    if (GFXFogState.curmode != fog || !GFXFogState.enabled) {
        switch (fog) {
            case FOG_OFF:
                if (!GFXFogState.enabled) {
                    return;
                }
                glDisable(GL_FOG);
                GFXFogState.enabled = false;
                return;
            case FOG_EXP:
                glFogi(GL_FOG_MODE, GL_EXP);
                break;
            case FOG_EXP2:
                glFogi(GL_FOG_MODE, GL_EXP2);
                break;
            case FOG_LINEAR:
                glFogi(GL_FOG_MODE, GL_LINEAR);
                break;
        }
    }
    if (!GFXFogState.enabled) {
        glEnable(GL_FOG);
    }
    GFXFogState.curmode = fog;
    GFXFogState.enabled = true;
    //glHint (GL_FOG_HINT,GL_NICEST );
}

void GFXFogDensity(const float fogdensity)
{
    if (fogdensity != GFXFogState.density) {
        glFogf(GL_FOG_DENSITY, fogdensity);
        GFXFogState.density = fogdensity;
    }
}

void GFXFogLimits(const float fognear, const float fogfar)
{
    if (GFXFogState.nnear != fognear || GFXFogState.ffar != fogfar) {
        GFXFogState.nnear = fognear;
        GFXFogState.ffar = fogfar;
        glFogf(GL_FOG_START, fognear);
        glFogf(GL_FOG_END, fogfar);
    }
}

void GFXFogColor(GFXColor c)
{
    if (c.r != GFXFogState.c.r || c.g != GFXFogState.c.g || c.b != GFXFogState.c.b || c.a != GFXFogState.c.a) {
        glFogfv(GL_FOG_COLOR, &c.r);
        GFXFogState.c = c;
    }
}

void GFXFogIndex(const int index)
{
    if (GFXFogState.index != index) {
        glFogi(GL_FOG_INDEX, index);
        GFXFogState.index = index;
    }
}

