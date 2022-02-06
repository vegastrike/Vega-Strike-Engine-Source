/*
 * Copyright (C) 2001-2022 Daniel Horn, Alan Shieh, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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
#include <cstring>
//#include "gl_globals.h"
//#include "gfx_transform_vector.h"

static vector<GFXMaterial> materialinfo;
static int selectedmaterial = -1;

void /*GFXDRVAPI*/ GFXSetMaterial(unsigned int &number, const GFXMaterial &material)
{
    number = (unsigned int) -1;     //intended warning for unsigned....
    for (unsigned int i = 0; i < materialinfo.size(); i++) {
        if (memcmp(&materialinfo[i], &material, sizeof(GFXMaterial)) == 0) {
            number = i;
            break;
        }
    }
    if (number == (unsigned int) (-1)) {
        //another intended warning
        number = materialinfo.size();
        materialinfo.push_back(material);
    }
}

void /*GFXDRVAPI*/ GFXModifyMaterial(const unsigned int number, const GFXMaterial &material)
{
    materialinfo[number] = material;
}

const GFXMaterial & /*GFXDRVAPI*/ GFXGetMaterial(const unsigned int number)
{
    if (number >= materialinfo.size()) {
        static GFXMaterial tmp;
        return tmp;
    }
    return materialinfo[number];
}

GFXBOOL /*GFXDRVAPI*/ GFXGetMaterial(const unsigned int number, GFXMaterial &material)
{
    if ((number == (unsigned int) -1) || number >= materialinfo.size()) {
        return GFXFALSE;
    }
    material = materialinfo[number];
    return GFXTRUE;
}

void /*GFXDRVAPI*/ GFXSelectMaterialAlpha(const unsigned int number, float alpha)
{
    selectedmaterial = -1;
    float matvect[4];
    matvect[0] = materialinfo[number].ar;
    matvect[1] = materialinfo[number].ag;
    matvect[2] = materialinfo[number].ab;
    matvect[3] = materialinfo[number].aa * alpha;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matvect);

    matvect[0] = materialinfo[number].dr;
    matvect[1] = materialinfo[number].dg;
    matvect[2] = materialinfo[number].db;
    matvect[3] = materialinfo[number].da * alpha;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matvect);

    matvect[0] = materialinfo[number].sr;
    matvect[1] = materialinfo[number].sg;
    matvect[2] = materialinfo[number].sb;
    matvect[3] = materialinfo[number].sa * alpha;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matvect);

    matvect[0] = materialinfo[number].er;
    matvect[1] = materialinfo[number].eg;
    matvect[2] = materialinfo[number].eb;
    matvect[3] = materialinfo[number].ea * alpha;
    glMaterialfv(GL_FRONT, GL_EMISSION, matvect);

    glMaterialfv(GL_FRONT, GL_SHININESS, &materialinfo[number].power);
}

const float zerovect[4] = {0.0};

void /*GFXDRVAPI*/ GFXSelectMaterialHighlights(const unsigned int number,
                                               const GFXColor &ambient,
                                               const GFXColor &diffuse,
                                               const GFXColor &specular,
                                               const GFXColor &emissive)
{
    selectedmaterial = -1;
    float matvect[4];
    matvect[0] = materialinfo[number].ar * ambient.r * ambient.a;
    matvect[1] = materialinfo[number].ag * ambient.g * ambient.a;
    matvect[2] = materialinfo[number].ab * ambient.b * ambient.a;
    matvect[3] = materialinfo[number].aa * ambient.a;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matvect);

    matvect[0] = materialinfo[number].dr * diffuse.r * diffuse.a;
    matvect[1] = materialinfo[number].dg * diffuse.g * diffuse.a;
    matvect[2] = materialinfo[number].db * diffuse.b * diffuse.a;
    matvect[3] = materialinfo[number].da * diffuse.a;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matvect);

    matvect[0] = materialinfo[number].sr * specular.r * specular.a;
    matvect[1] = materialinfo[number].sg * specular.g * specular.a;
    matvect[2] = materialinfo[number].sb * specular.b * specular.a;
    matvect[3] = materialinfo[number].sa * specular.a;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matvect);

    matvect[0] = emissive.r * emissive.a;
    matvect[1] = emissive.g * emissive.a;
    matvect[2] = emissive.b * emissive.a;
    matvect[3] = emissive.a;
    glMaterialfv(GL_FRONT, GL_EMISSION, matvect);

    glMaterialfv(GL_FRONT, GL_SHININESS, &materialinfo[number].power);
}

void /*GFXDRVAPI*/ GFXSelectMaterial(const unsigned int number)
{
    if (1 || static_cast<int>(number) != selectedmaterial) { //FIXME Last time I checked true or anything was true
        float matvect[4];
        matvect[0] = materialinfo[number].ar;
        matvect[1] = materialinfo[number].ag;
        matvect[2] = materialinfo[number].ab;
        matvect[3] = materialinfo[number].aa;
        glMaterialfv(GL_FRONT, GL_AMBIENT, matvect);

        matvect[0] = materialinfo[number].dr;
        matvect[1] = materialinfo[number].dg;
        matvect[2] = materialinfo[number].db;
        matvect[3] = materialinfo[number].da;
        glMaterialfv(GL_FRONT, GL_DIFFUSE, matvect);

        matvect[0] = materialinfo[number].sr;
        matvect[1] = materialinfo[number].sg;
        matvect[2] = materialinfo[number].sb;
        matvect[3] = materialinfo[number].sa;
        glMaterialfv(GL_FRONT, GL_SPECULAR, matvect);

        matvect[0] = materialinfo[number].er;
        matvect[1] = materialinfo[number].eg;
        matvect[2] = materialinfo[number].eb;
        matvect[3] = materialinfo[number].ea;
        glMaterialfv(GL_FRONT, GL_EMISSION, matvect);

        glMaterialfv(GL_FRONT, GL_SHININESS, &materialinfo[number].power);
        selectedmaterial = number;
    }
}

