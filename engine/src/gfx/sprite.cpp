/*
 * sprite.cpp
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


#include <stdlib.h>
#include <iostream>
#include "root_generic/vsfilesystem.h"
#include "src/vs_logging.h"
#include "cmd/unit_generic.h"
#include "gfx/aux_texture.h"
#include "gfx/ani_texture.h"
#include "gfx/sprite.h"
#include "gfx_generic/matrix.h"
#include "src/gfxlib.h"
#include "src/vegastrike.h"
#include "root_generic/vs_globals.h"
#include "gldrv/gl_globals.h"
#include <assert.h>
#include <math.h>
#include "src/gnuhash.h"

#ifdef _WIN32
#include <direct.h>
#endif

#ifndef M_PI_2
# define M_PI_2 (1.57079632679489661923)
#endif

#include "audio/Types.h"
#include "audio/Source.h"

using namespace VSFileSystem;

typedef vsUMap<std::string, VSSprite *> VSSpriteCache;
static VSSpriteCache sprite_cache;

static std::pair<bool, VSSprite *> cacheLookup(const char *file) {
    std::string hashName = VSFileSystem::GetHashName(std::string(file));
    VSSpriteCache::iterator it = sprite_cache.find(hashName);
    if (it != sprite_cache.end()) {
        return std::pair<bool, VSSprite *>(true, it->second);
    } else {
        return std::pair<bool, VSSprite *>(false, (VSSprite *) NULL);
    }
}

static void cacheInsert(const char *file, VSSprite *spr) {
    std::string hashName = VSFileSystem::GetHashName(std::string(file));
    sprite_cache.insert(std::pair<std::string, VSSprite *>(hashName, spr));
}

VSSprite::VSSprite(Texture *_surface,
        float _xcenter,
        float _ycenter,
        float _width,
        float _height,
        float _s,
        float _t,
        bool _isAnimation) :
        xcenter(_xcenter),
        ycenter(_ycenter),
        widtho2(_width / 2),
        heighto2(_height / 2),
        maxs(_s),
        maxt(_t),
        rotation(0),
        isAnimation(_isAnimation) {
    surface = _surface;
}

VSSprite::VSSprite(const VSSprite &source) {
    *this = source;
    if (surface != NULL) {
        surface = surface->Clone();
    }
}

VSSprite::VSSprite(const char *file, enum FILTER texturefilter, GFXBOOL force) {
    VSCONSTRUCT2('S')
    xcenter = ycenter = 0;
    widtho2 = heighto2 = 0;
    rotation = 0;
    surface = NULL;
    maxs = maxt = 0;
    isAnimation = false;

    //Check cache
    {
        std::pair<bool, VSSprite *> lkup = cacheLookup(file);
        if (lkup.first) {
            if (lkup.second) {
                *this = *lkup.second;
                if (surface != NULL) {
                    surface = surface->Clone();
                }
            } else {
                return;
            }
        }
    }

    VSFile f;
    VSError err = Unspecified;
    if (file[0] != '\0') {
        err = f.OpenReadOnly(file, VSSpriteFile);
    }
    if (err <= Ok) {
        char texture[127] = {0};
        char texturea[127] = {0};
        f.Fscanf("%126s %126s", texture, texturea);
        f.Fscanf("%f %f", &widtho2, &heighto2);
        f.Fscanf("%f %f", &xcenter, &ycenter);
        texture[sizeof(texture) - sizeof(*texture) - 1] = 0;
        texturea[sizeof(texturea) - sizeof(*texturea) - 1] = 0;

        widtho2 /= 2;
        heighto2 /= -2;
        surface = NULL;
        if (g_game.use_sprites || force == GFXTRUE) {
            int len = strlen(texture);
            if (len > 4 && texture[len - 1] == 'i' && texture[len - 2] == 'n' && texture[len - 3] == 'a'
                    && texture[len - 4] == '.') {
                surface = new AnimatedTexture(f, 0, texturefilter, GFXFALSE);
                isAnimation = true;
            } else if (texturea[0] == '0') {
                surface = new Texture(texture, 0, texturefilter, TEXTURE2D, TEXTURE_2D, GFXTRUE, 65536, GFXFALSE);
                isAnimation = false;
            } else {
                surface = new Texture(texture,
                        texturea,
                        0,
                        texturefilter,
                        TEXTURE2D,
                        TEXTURE_2D,
                        1,
                        0,
                        GFXTRUE,
                        65536,
                        GFXFALSE);
                isAnimation = false;
            }
            if (!surface->LoadSuccess()) {
                delete surface;
                surface = NULL;
                VSSprite *newspr = new VSSprite();
                *newspr = *this;
                newspr->surface = NULL;
                cacheInsert(file, newspr);
            } else {
                //Update cache
                VSSprite *newspr = new VSSprite();
                *newspr = *this;
                newspr->surface = this->surface->Clone();
                cacheInsert(file, newspr);
            }
        }
        //Finally close file
        f.Close();
    } else {
        cacheInsert(file, 0);         //Mark bad file
        widtho2 = heighto2 = 0;
        xcenter = ycenter = 0;
    }
}

void VSSprite::ReadTexture(VSFileSystem::VSFile *f) {
    if (!f->Valid()) {
        widtho2 = heighto2 = 0;
        xcenter = ycenter = 0;
        VS_LOG(error, "VSSprite::ReadTexture error : VSFile not valid");
        return;
    }
    surface = new Texture(f);
}

VSSprite::~VSSprite() {
    VSDESTRUCT2
    if (surface != nullptr) {
        delete surface;
        surface = nullptr;
    }
}

void VSSprite::SetST(const float s, const float t) {
    maxs = s;
    maxt = t;
}

void VSSprite::GetST(float &s, float &t) {
    s = maxs;
    t = maxt;
}

void VSSprite::SetTime(double newtime) {
    if (surface) {
        surface->setTime(newtime);
    }
}

void VSSprite::DrawHere(Vector &ll, Vector &lr, Vector &ur, Vector &ul) {
    if (rotation) {
        const float cw = widtho2 * cos(rotation);
        const float sw = widtho2 * sin(rotation);
        const float ch = heighto2 * cos(M_PI_2 + rotation);
        const float sh = heighto2 * sin(M_PI_2 + rotation);
        const float wnew = cw + ch;
        const float hnew = sw + sh;
        ll = Vector(xcenter - wnew, ycenter + hnew, 0.00f);
        lr = Vector(xcenter + wnew, ycenter + hnew, 0.00f);
        ur = Vector(xcenter + wnew, ycenter - hnew, 0.00f);
        ul = Vector(xcenter - wnew, ycenter - hnew, 0.00f);
    } else {
        ll = Vector(xcenter - widtho2, ycenter + heighto2, 0.00f);
        lr = Vector(xcenter + widtho2, ycenter + heighto2, 0.00f);
        ur = Vector(xcenter + widtho2, ycenter - heighto2, 0.00f);
        ul = Vector(xcenter - widtho2, ycenter - heighto2, 0.00f);
    }
}

void VSSprite::Draw() {
    if (surface) {
        //don't do anything if no surface
        size_t lyr;
        size_t numlayers = surface->numLayers();
        bool multitex = (numlayers > 1);
        int numpasses = surface->numPasses();
        GFXDisable(CULLFACE);
        Vector ll, lr, ur, ul;
        DrawHere(ll, lr, ur, ul);
        BLENDFUNC src, dst;
        GFXGetBlendMode(src, dst);
        for (lyr = 0; (lyr < gl_options.Multitexture) || (lyr < numlayers); lyr++) {
            GFXToggleTexture((lyr < numlayers), lyr, surface->texture_target);
            if (lyr < numlayers) {
                GFXTextureCoordGenMode(lyr, NO_GEN, NULL, NULL);
            }
        }
        for (int pass = 0; pass < numpasses; pass++) {
            if (surface->SetupPass(pass, 0, src, dst)) {
                surface->MakeActive(0, pass);

                // Keep below MakeActive, AnimatedTexture only sets
                // the final effective coordinates there.
                float ms = surface->mintcoord.i, Ms = surface->maxtcoord.i;
                float mt = surface->mintcoord.j, Mt = surface->maxtcoord.j;
                ms = (Ms - ms) * maxs + ms;
                mt = (Mt - mt) * maxt + mt;

                GFXTextureEnv(0, GFXMODULATETEXTURE);
                if (!multitex) {
                    const float vert[4 * (3 + 2)] = {
                            ll.i, ll.j, ll.k, ms, Mt,
                            lr.i, lr.j, lr.k, Ms, Mt,
                            ur.i, ur.j, ur.k, Ms, mt,
                            ul.i, ul.j, ul.k, ms, mt
                    };
                    GFXDraw(GFXQUAD, vert, 4, 3, 0, 2);
                } else {
                    const float vert[4 * (3 + 4)] = {
                            ll.i, ll.j, ll.k, ms, Mt, ms, Mt,
                            lr.i, lr.j, lr.k, Ms, Mt, Ms, Mt,
                            ur.i, ur.j, ur.k, Ms, mt, Ms, mt,
                            ul.i, ul.j, ul.k, ms, mt, ms, mt
                    };
                    GFXDraw(GFXQUAD, vert, 4, 3, 0, 2, 2);
                }
            }
        }
        surface->SetupPass(-1, 0, src, dst);
        for (lyr = 0; lyr < numlayers; lyr++) {
            GFXToggleTexture(false, lyr, surface->texture_target);
        }
        GFXEnable(CULLFACE);
    }
}

void VSSprite::SetPosition(const float &x1, const float &y1) {
    xcenter = x1;
    ycenter = y1;
}

void VSSprite::GetPosition(float &x1, float &y1) {
    x1 = xcenter;
    y1 = ycenter;
}

void VSSprite::SetSize(float x1, float y1) {
    widtho2 = x1 / 2;
    heighto2 = y1 / 2;
}

void VSSprite::GetSize(float &x1, float &y1) {
    x1 = widtho2 * 2;
    y1 = heighto2 * 2;
}

void VSSprite::SetRotation(const float &rot) {
    rotation = rot;
}

void VSSprite::GetRotation(float &rot) {
    rot = rotation;
}

void VSSprite::SetTimeSource(SharedPtr<Audio::Source> source) {
    if (isAnimation) {
        ((AnimatedTexture *) surface)->SetTimeSource(source);
    }
}

SharedPtr<Audio::Source> VSSprite::GetTimeSource() const {
    if (isAnimation) {
        return ((AnimatedTexture *) surface)->GetTimeSource();
    } else {
        return SharedPtr<Audio::Source>();
    }
}

void VSSprite::ClearTimeSource() {
    if (isAnimation) {
        ((AnimatedTexture *) surface)->ClearTimeSource();
    }
}

bool VSSprite::Done() const {
    if (isAnimation) {
        return ((AnimatedTexture *) surface)->Done();
    } else {
        return false;
    }
}

void VSSprite::Reset() {
    if (isAnimation) {
        ((AnimatedTexture *) surface)->Reset();
    }
}

bool VSSprite::LoadSuccess() const {
    return surface != NULL && surface->LoadSuccess();
}

