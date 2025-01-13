/*
 * animation.cpp
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

#include "cmd/unit_generic.h"
#include "animation.h"
#include "aux_texture.h"
#include "camera.h"
#include "lin_time.h"
#include <stack>
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "point_to_cam.h"
#include "config_xml.h"
#include "xml_support.h"
#include "sprite.h"
#include <algorithm>
#include "../gldrv/gl_globals.h"
#include "universe.h"

using std::vector;
using std::stack;

static vector<Animation *> far_animationdrawqueue;

bool AnimationsLeftInFarQueue() {
    return !far_animationdrawqueue.empty();
}

static vector<Animation *> animationdrawqueue;

bool AnimationsLeftInQueue() {
    return !animationdrawqueue.empty();
}

static const unsigned char ani_up = 0x01;
static const unsigned char ani_close = 0x02;
static const unsigned char ani_alpha = 0x04;
static const unsigned char ani_repeat = 0x08;

Animation::Animation() {
    VSCONSTRUCT2('a')
    height = 0.001F;
    width = 0.001F;
}

void Animation::SetFaceCam(bool face) {
    if (face) {
        options |= ani_up;
    } else {
        options &= (~ani_up);
    }
}

using namespace VSFileSystem;

Animation::Animation(VSFileSystem::VSFile *f,
        bool Rep,
        float priority,
        enum FILTER ismipmapped,
        bool camorient,
        bool appear_near_by_radius,
        const GFXColor &c) : mycolor(c) {
}

Animation::Animation(const char *FileName,
        bool Rep,
        float priority,
        enum FILTER ismipmapped,
        bool camorient,
        bool appear_near_by_radius,
        const GFXColor &c) : mycolor(c) {
    Identity(local_transformation);
    VSCONSTRUCT2('a')
    //repeat = Rep;
    options = 0;
    if (Rep) {
        options |= ani_repeat;
    }
    if (camorient) {
        options |= ani_up;
    }
    if (appear_near_by_radius) {
        options |= ani_close;
    }
    SetLoop(Rep);     //setup AnimatedTexture's loop flag - NOTE: Load() will leave it like this unless a force(No)Loop option is present
    SetLoopInterp(Rep);     //Default interpolation method == looping method
    VSFile f;
    VSError err = f.OpenReadOnly(FileName, AnimFile);
    if (err > Ok) {
        //load success already set false
    } else {
        f.Fscanf("%f %f", &width, &height);
        if (width > 0) {
            options |= ani_alpha;
        }
        width = fabs(width * 0.5F);
        height = height * 0.5F;
        Load(f, 0, ismipmapped);
        f.Close();
    }
    //VSFileSystem::ResetCurrentPath();
}

Animation::~Animation() {
    vector<Animation *>::iterator i;
    while ((i =
            std::find(far_animationdrawqueue.begin(), far_animationdrawqueue.end(),
                    this)) != far_animationdrawqueue.end()) {
        far_animationdrawqueue.erase(i);
    }
    while ((i = std::find(animationdrawqueue.begin(), animationdrawqueue.end(), this)) != animationdrawqueue.end()) {
        animationdrawqueue.erase(i);
    }
    VSDESTRUCT2
}

void Animation::SetPosition(const QVector &p) {
    local_transformation.p = p;
}

void Animation::SetOrientation(const Vector &p, const Vector &q, const Vector &r) {
    VectorAndPositionToMatrix(local_transformation, p, q, r, local_transformation.p);
}

QVector Animation::Position() {
    return local_transformation.p;
}

void Animation::SetDimensions(float wid, float hei) {
    width = wid;
    height = hei;
}

void Animation::GetDimensions(float &wid, float &hei) {
    wid = width;
    hei = height;
}

void Animation::ProcessDrawQueue() {
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXDisable(LIGHTING);
    GFXEnable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXDisable(DEPTHWRITE);
    ProcessDrawQueue(animationdrawqueue, -FLT_MAX);
}

bool Animation::NeedsProcessDrawQueue() {
    return AnimationsLeftInQueue();
}

void Animation::ProcessFarDrawQueue(float farval) {
    //set farshit
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXDisable(LIGHTING);
    GFXEnable(TEXTURE0);
    GFXDisable(TEXTURE1);

    ProcessDrawQueue(far_animationdrawqueue, farval);
}

bool Animation::NeedsProcessFarDrawQueue() {
    return AnimationsLeftInFarQueue();
}

void Animation::ProcessDrawQueue(std::vector<Animation *> &animationdrawqueue, float limit) {
    if (g_game.use_animations == 0 && g_game.use_textures == 0) {
        return;
    }
    unsigned char alphamaps = ani_alpha;
    int i, j;     //NOT UNSIGNED
    for (i = animationdrawqueue.size() - 1; i >= 0; i--) {
        GFXColorf(animationdrawqueue[i]->mycolor);         //fixme, should we need this? we get som egreenie explosions
        Matrix result;
        if (alphamaps != (animationdrawqueue[i]->options & ani_alpha)) {
            alphamaps = (animationdrawqueue[i]->options & ani_alpha);
            GFXBlendMode((alphamaps != 0) ? SRCALPHA : ONE, (alphamaps != 0) ? INVSRCALPHA : ONE);
        }
        QVector campos = _Universe->AccessCamera()->GetPosition();
        animationdrawqueue[i]->CalculateOrientation(result);
        if ((limit
                <= -FLT_MAX) ||
                (animationdrawqueue[i]->Position() - campos).Magnitude() - animationdrawqueue[i]->height > limit) {
            //other way was inconsistent about what was far and what was not--need to use the same test for putting to far queueu and drawing it--otherwise graphical glitches
            GFXFogMode(FOG_OFF);
            animationdrawqueue[i]->DrawNow(result);
            animationdrawqueue[i] =
                    0;             //Flag for deletion: gets called multiple times with decreasing values, and eventually is called with limit=-FLT_MAX.
        }
    }
    //Delete flagged ones
    i = 0;
    while (i < static_cast<int>(animationdrawqueue.size()) && animationdrawqueue[i]) {
        ++i;
    }
    j = i;
    while (i < static_cast<int>(animationdrawqueue.size())) {
        while (i < static_cast<int>(animationdrawqueue.size()) && !animationdrawqueue[i]) {
            ++i;
        }
        while (i < static_cast<int>(animationdrawqueue.size()) && animationdrawqueue[i]) {
            animationdrawqueue[j++] = animationdrawqueue[i++];
        }
    }
    if (j >= 0) {
        animationdrawqueue.resize(j);
    }
}

bool Animation::CalculateOrientation(Matrix &result) {
    Vector camp, camq, camr;
    QVector pos(Position());
    float hei = height;
    float wid = width;
    static float HaloOffset = XMLSupport::parse_float(vs_config->getVariable("graphics", "HaloOffset", ".1"));
    bool retval =
            ::CalculateOrientation(pos,
                    camp,
                    camq,
                    camr,
                    wid,
                    hei,
                    (options & ani_close) ? HaloOffset : 0,
                    false,
                    (options & ani_up) ? NULL : &local_transformation);

    /*
     *  Camera* TempCam = _Universe->AccessCamera();
     *  TempCam->GetPQR(camp,camq,camr);
     *  if (!camup){
     *  Vector q1 (local_transformation[1],local_transformation[5],local_transformation[9]);
     *  Vector p1 ((q1.Dot(camq))*camq);
     *  camq = (q1.Dot(camp))*camp+p1;
     *  Vector posit;
     *  TempCam->GetPosition (posit);
     *  camr.i = -local_transformation[12]+posit.i;
     *  camr.j = -local_transformation[13]+posit.j;
     *  camr.k = -local_transformation[14]+posit.k;
     *  Normalize (camr);
     *  ScaledCrossProduct (camq,camr,camp);
     *  ScaledCrossProduct (camr,camp,camq);
     *  //if the vectors are linearly dependant we're phucked :) fun fun fun
     *  }
     */
    VectorAndPositionToMatrix(result, camp, camq, camr, pos);
    return retval;
}

void Animation::DrawNow(const Matrix &final_orientation) {
    if ((g_game.use_animations || g_game.use_textures) && (!Done() || (options & ani_repeat))) {
        GFXLoadMatrixModel(final_orientation);
        size_t lyr;
        size_t numlayers = numLayers();
        bool multitex = (numlayers > 1);
        size_t numpasses = numPasses();
        float ms = mintcoord.i, Ms = maxtcoord.i;
        float mt = mintcoord.j, Mt = maxtcoord.j;
        BLENDFUNC src, dst;
        GFXGetBlendMode(src, dst);
        for (lyr = 0; (lyr < gl_options.Multitexture) || (lyr < numlayers); lyr++) {
            GFXToggleTexture((lyr < numlayers), lyr);
            if (lyr < numlayers) {
                GFXTextureCoordGenMode(lyr, NO_GEN, NULL, NULL);
            }
        }
        for (size_t pass = 0; pass < numpasses; pass++) {
            if (SetupPass(pass, 0, src, dst)) {
                MakeActive(0, pass);
                GFXTextureEnv(0, GFXMODULATETEXTURE);
                if (!multitex) {
                    const float verts[4 * (3 + 2)] = {
                            -width, -height, 0.0f, ms, Mt,    //lower left
                            width, -height, 0.0f, Ms, Mt,    //upper left
                            width, height, 0.0f, Ms, mt,    //upper right
                            -width, height, 0.0f, ms, mt,    //lower right
                    };
                    GFXDraw(GFXQUAD, verts, 4, 3, 0, 2);
                } else {
                    const float verts[4 * (3 + 4)] = {
                            -width, -height, 0.0f, ms, Mt, ms, Mt,
                            width, -height, 0.0f, Ms, Mt, Ms, Mt,
                            width, height, 0.0f, Ms, mt, Ms, mt,
                            -width, height, 0.0f, ms, mt, ms, mt,
                    };
                    GFXDraw(GFXQUAD, verts, 4, 3, 0, 2, 2);
                }
            }
        }
        for (lyr = 0; lyr < numlayers; lyr++) {
            GFXToggleTexture(false, lyr);
        }
        SetupPass(-1, 0, src, dst);
    }
}

void Animation::DrawAsVSSprite(VSSprite *spr) {
    if (!spr) {
        return;
    }
    if (g_game.use_animations != 0 || g_game.use_textures != 0) {
        //unsigned char alphamaps=ani_alpha;
        GFXPushBlendMode();
        if (options & ani_alpha) {
            GFXBlendMode(SRCALPHA, INVSRCALPHA);
        } else {
            GFXBlendMode(ONE, ZERO);
        }
        size_t lyr;
        size_t numlayers = numLayers();
        bool multitex = (numlayers > 1);
        size_t numpasses = numPasses();
        float ms = mintcoord.i, Ms = maxtcoord.i;
        float mt = mintcoord.j, Mt = maxtcoord.j;
        GFXDisable(CULLFACE);
        Vector ll, lr, ur, ul;
        spr->DrawHere(ll, lr, ur, ul);
        BLENDFUNC src, dst;
        GFXGetBlendMode(src, dst);
        for (lyr = 0; (lyr < gl_options.Multitexture) || (lyr < numlayers); lyr++) {
            GFXToggleTexture((lyr < numlayers), lyr);
            if (lyr < numlayers) {
                GFXTextureCoordGenMode(lyr, NO_GEN, NULL, NULL);
            }
        }
        for (size_t pass = 0; pass < numpasses; pass++) {
            if (SetupPass(pass, 0, src, dst)) {
                MakeActive(0, pass);
                GFXTextureEnv(0, GFXMODULATETEXTURE);
                if (!multitex) {
                    const float verts[4 * (3 + 2)] = {
                            ll.i, ll.j, ll.k, ms, Mt,    //lower left
                            lr.i, lr.j, lr.k, Ms, Mt,    //upper left
                            ur.i, ur.j, ur.k, Ms, mt,    //upper right
                            ul.i, ul.j, ul.k, ms, mt,    //lower right
                    };
                    GFXDraw(GFXQUAD, verts, 4, 3, 0, 2);
                } else {
                    const float verts[4 * (3 + 4)] = {
                            ll.i, ll.j, ll.k, ms, Mt, ms, Mt,
                            lr.i, lr.j, lr.k, Ms, Mt, Ms, Mt,
                            ur.i, ur.j, ur.k, Ms, mt, Ms, mt,
                            ul.i, ul.j, ul.k, ms, mt, ms, mt,
                    };
                    GFXDraw(GFXQUAD, verts, 4, 3, 0, 2, 2);
                }
            }
        }
        SetupPass(-1, 0, src, dst);
        for (lyr = 0; lyr < numlayers; lyr++) {
            GFXToggleTexture(false, lyr);
        }
        GFXEnable(CULLFACE);
        GFXPopBlendMode();
    }
}

void Animation::DrawNoTransform(bool cross, bool blendoption) {
    if (g_game.use_animations == 0 && g_game.use_textures == 0) {
    } else if (!Done() || (options & ani_repeat)) {
        size_t lyr;
        size_t numlayers = numLayers();
        bool multitex = (numlayers > 1);
        size_t numpasses = numPasses();
        float ms = mintcoord.i, Ms = maxtcoord.i;
        float mt = mintcoord.j, Mt = maxtcoord.j;
        if (blendoption) {
            if (options & ani_alpha) {
                GFXEnable(DEPTHWRITE);
                GFXBlendMode(SRCALPHA, INVSRCALPHA);
            } else {
                GFXBlendMode(ONE, ONE);
            }
        }
        BLENDFUNC src, dst;
        GFXGetBlendMode(src, dst);
        for (lyr = 0; (lyr < gl_options.Multitexture) || (lyr < numlayers); lyr++) {
            GFXToggleTexture((lyr < numlayers), lyr);
            if (lyr < numlayers) {
                GFXTextureCoordGenMode(lyr, NO_GEN, NULL, NULL);
            }
        }
        for (size_t pass = 0; pass < numpasses; pass++) {
            if (SetupPass(pass, 0, src, dst)) {
                MakeActive(0, pass);
                GFXTextureEnv(0, GFXMODULATETEXTURE);
                int vnum = cross ? 12 : 4;
                if (!multitex) {
                    const float verts[12 * (3 + 2)] = {
                            -width, -height, 0.0f, ms, Mt,    //lower left
                            width, -height, 0.0f, Ms, Mt,    //upper left
                            width, height, 0.0f, Ms, mt,    //upper right
                            -width, height, 0.0f, ms, mt,    //lower right

                            -width, 0.0f, -height, ms, Mt,    //lower left
                            width, 0.0f, -height, Ms, Mt,    //upper left
                            width, 0.0f, height, Ms, mt,    //upper right
                            -width, 0.0f, height, ms, mt,    //lower right

                            0.0f, -height, -height, ms, Mt,    //lower left
                            0.0f, height, -height, Ms, Mt,    //upper left
                            0.0f, height, height, Ms, mt,    //upper right
                            0.0f, -height, height, ms, mt,    //lower right
                    };
                    GFXDraw(GFXQUAD, verts, vnum, 3, 0, 2);
                } else {
                    const float verts[12 * (3 + 4)] = {
                            -width, -height, 0.0f, ms, Mt, ms, Mt,
                            width, -height, 0.0f, Ms, Mt, Ms, Mt,
                            width, height, 0.0f, Ms, mt, Ms, mt,
                            -width, height, 0.0f, ms, mt, ms, mt,

                            -width, 0.0f, -height, ms, Mt, ms, Mt,
                            width, 0.0f, -height, Ms, Mt, Ms, Mt,
                            width, 0.0f, height, Ms, mt, Ms, mt,
                            -width, 0.0f, height, ms, mt, ms, mt,

                            0.0f, -height, -height, ms, Mt, ms, Mt,
                            0.0f, height, -height, Ms, Mt, Ms, Mt,
                            0.0f, height, height, Ms, mt, Ms, mt,
                            0.0f, -height, height, ms, mt, ms, mt,
                    };
                    GFXDraw(GFXQUAD, verts, vnum, 3, 0, 2, 2);
                }
            }
        }
        SetupPass(-1, 0, src, dst);
        for (lyr = 0; lyr < numlayers; lyr++) {
            GFXToggleTexture(false, lyr);
        }
        if (blendoption) {
            if (options & ani_alpha) {
                GFXDisable(DEPTHWRITE);
            }
        }
    }
}

void Animation::Draw() {
    if (g_game.use_animations != 0 || g_game.use_textures != 0) {
        Vector camp, camq, camr;
        QVector pos(Position());

        static float HaloOffset = XMLSupport::parse_float(vs_config->getVariable("graphics", "HaloOffset", ".1"));

        /**/
        //Why do all this if we can use ::CalculateOrientation?
        //-- well one reason is that the code change broke it :-/  Until suns display properly or we switch to ogre we should keep it as it was (problem was, flare wouldn't display--- or would display behind the sun)
        QVector R(_Universe->AccessCamera()->GetR().i, _Universe->AccessCamera()->GetR().j,
                _Universe->AccessCamera()->GetR().k);
        static float too_far_dist = XMLSupport::parse_float(
                vs_config->getVariable("graphics", "anim_far_percent", ".8"));
        if (( /*R.Dot*/ (Position()
                - _Universe->AccessCamera()->GetPosition()).Magnitude() + HaloOffset
                *
                        (height > width ? height : width)) <
                too_far_dist * g_game.zfar) {
            //if (::CalculateOrientation (pos,camp,camq,camr,wid,hei,(options&ani_close)?HaloOffset:0,false)) {ss
            animationdrawqueue.push_back(this);
        } else {
            far_animationdrawqueue.push_back(this);
        }
    }
}

