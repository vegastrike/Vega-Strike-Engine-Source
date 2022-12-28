/*
 * gl_light.h
 *
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

#ifndef _GL_LIGHT_H_
#define _GL_LIGHT_H_

#include <options.h>
#include "gfxlib.h"
#include "hashtable_3d.h"
#include "gl_globals.h"

extern GLint GFX_MAX_LIGHTS;
extern GLint GFX_OPTIMAL_LIGHTS;
extern GFXBOOL GFXLIGHTING;

#define GFX_ATTENUATED 1
//#define GFX_DIFFUSE 2
//#define GFX_SPECULAR 4
//#define GFX_AMBIENT 8
//#define GFX_LIGHT_POS 16
#define GFX_LIGHT_ENABLED 32
#define GFX_LOCAL_LIGHT 64
const unsigned int lighthuge = 20 * 20 * 20;
constexpr size_t kGfxMaxContexts = 64;

/**
 * This stores the state of a given GL Light in its fullness
 * It inherits all values a light may have, and gains a number of functions
 * that assist it in maintaining the virtual OpenGL state amidst the limited
 * Number of lights, etc
 */
class gfx_light : public GFXLight {
public:
    gfx_light() : GFXLight() {
    }

    ///assigns a GFXLight to a gfx_light
    gfx_light & operator=(const GFXLight &tmp);

    ///Returns the number this light is in the _llights list
    int lightNum();

    ///Returns if this light was saved as a local light
    bool LocalLight() const {
        return (options & GFX_LOCAL_LIGHT) != 0;
    }

    ///Retursn if this light is enabled
    bool enabled() const {
        return (options & GFX_LIGHT_ENABLED) != 0;
    }

    ///Returns the target OpenGL light of this light. -1 is unassigned to a "real" light
    int &Target() {
        return target;
    }

    /**
     * if global, puts it into GLlights (if space ||enabled) <clobber?>
     * for local lights, if enabled, call Enable().
     */
    bool Create(const GFXLight &, bool global);

    /// Disables it (may remove from table), trashes it from GLlights. sets target to -2 (dead)
    void Kill();

    ///properly utilizes union to send to OGL
    void SendGLPosition(const GLenum target) const;

    ///replaces target GL light in the implementation. Sets this->target! Checks for -1 and calls ContextSwitch to clobber completely
    void ClobberGLLight(const int target);

    ///resends target GL light data (assumes valid target)
    void UpdateGLLight() const;

    ///replaces target GL light, copying all state sets this->target!
    inline void FinesseClobberLight(const GLenum target, const int original);

    ///replaces target GL light, copying all state sets this->target!
    inline void ContextSwitchClobberLight(const GLenum target, const int original) const;

    /**
     * for global lights, clobbers SOMETHING for sure, calls GLenable
     * for local lights, puts it into the light table
     */
    void Enable();

    /**
     * for global lights, GLdisables it.
     * for local lights, removes it from the table. and trashes it form GLlights.
     */
    void Disable();

    /** sets properties, making minimum GL state changes for global,
     *  for local lights, removes it from table, trashes it from GLlights,
     *  if enabled, puts it bakc in table.
     */
    void ResetProperties(const enum LIGHT_TARGET, const GFXColor &color);

    ///Adds this light to table (assume local)
    void AddToTable();

    ///Removes this light from light table
    bool RemoveFromTable(bool shouldremove = true, const GFXLight &t = GFXLight());

    ///Trash this light from active GLLights
    void TrashFromGLLights();

    ///Do all enables from picking
    static void dopickenables();

    ///calculates bounds for the table given cutoffs!
    LineCollide CalculateBounds(bool &err);
};

namespace OpenGLL {
///If a light is off
const char GLL_OFF = 0;
///If a light is on
const char GLL_ON = 1;
///If a light is local or not
const char GLL_LOCAL = 2;
///If a light is enabled for OpenGL
const char GL_ENABLED = 4;
}

struct OpenGLLights {
    /// Index into _gllights.  -1 == unassigned
    int index;
    ///Options GLL_OFF, on GLL_ENABLED
    char options;
};
///Rekeys a frame, remembering trashing old lights activated last frame
void light_rekey_frame();
///picks doubtless changed position
void unpickLights();
void removeLightFromNewPick(int whichlight);

class ManagerOfStaticLightsData {
public:
    ///The current light context
    int current_context{};
    ///The light data _llights points to one of these
    vega_types::SharedPtr<vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<gfx_light>>>>> local_lights_dat;
    ///The ambient lights that are around
    vega_types::SharedPtr<vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<GFXColor>>> ambient_light;
    ///The lights existing in a certain context. Points to local_lights_dat
    vega_types::SharedPtr<vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<gfx_light>>> l_lights;
    ///How many lights are enabled (for fast picking)
    int gl_lights_enabled{};
    ///currently stored GL lights!
    vega_types::SharedPtr<vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<OpenGLLights>>> gl_lights;

    // picked_lights was a list, but lists imply heavy reallocation, which is bad in critical sections
    // ( and picked_lights is used in the most critical section: just before GFXVertexList::Draw() )
    vega_types::SharedPtr<vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<vega_types::SequenceContainer<int>>>> picked_lights;
    vega_types::SharedPtr<vega_types::SequenceContainer<int>> new_picked;
    vega_types::SharedPtr<vega_types::SequenceContainer<int>> old_picked;

public:
    // Default constructor -- Only meant to be used from the staticLightsDataManager Meyers singleton function
    ManagerOfStaticLightsData() {
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,
                      1);     //don't want lighting coming from infinity....we have to take the hit due to sphere mapping matrix tweaking
        glGetIntegerv(GL_MAX_LIGHTS, &GFX_MAX_LIGHTS);
        local_lights_dat = vega_types::MakeShared<vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<gfx_light>>>>>();
        local_lights_dat->reserve(kGfxMaxContexts);
        while (local_lights_dat->size() < kGfxMaxContexts) {
            local_lights_dat->push_back(vega_types::MakeShared<vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<gfx_light>>>());
            local_lights_dat->back()->reserve(GFX_MAX_LIGHTS);
            while (local_lights_dat->back()->size() < GFX_MAX_LIGHTS) {
                local_lights_dat->back()->push_back(vega_types::MakeShared<gfx_light>());
            }
        }
        ambient_light = vega_types::MakeShared<vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<GFXColor>>>();
        ambient_light->reserve(GFX_MAX_LIGHTS);
        while (ambient_light->size() < GFX_MAX_LIGHTS) {
            ambient_light->push_back(vega_types::MakeShared<GFXColor>(0, 0, 0, 1));
        }
        gl_lights = vega_types::MakeShared<vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<OpenGLLights>>>();
        gl_lights->reserve(GFX_MAX_LIGHTS);
        for (GLint i = 0; i < GFX_MAX_LIGHTS; ++i) {
            vega_types::SharedPtr<OpenGLLights> const new_light = vega_types::MakeShared<OpenGLLights>();
            new_light->index = -1;
            gl_lights->push_back(new_light);
        }

        picked_lights = vega_types::MakeShared<vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<vega_types::SequenceContainer<int>>>>(2);
        picked_lights->at(0) = vega_types::MakeShared<vega_types::SequenceContainer<int>>();
        picked_lights->at(1) = vega_types::MakeShared<vega_types::SequenceContainer<int>>();
        new_picked = picked_lights->at(0);
        old_picked = picked_lights->at(1);

        GFXSetCutoff(game_options()->lightcutoff);
        GFXSetOptimalIntensity(game_options()->lightoptimalintensity, game_options()->lightsaturation);
        GFXSetOptimalNumLights(game_options()->numlights);
        GFXSetSeparateSpecularColor(game_options()->separatespecularcolor ? GFXTRUE : GFXFALSE);
    }

    ManagerOfStaticLightsData(ManagerOfStaticLightsData const &) = delete;
    ManagerOfStaticLightsData(ManagerOfStaticLightsData &&) = delete;
    ManagerOfStaticLightsData& operator=(ManagerOfStaticLightsData const &) = delete;
    ManagerOfStaticLightsData& operator=(ManagerOfStaticLightsData &&) = delete;

    ~ManagerOfStaticLightsData() = default;

    void swapPicked() {
        if (new_picked == picked_lights->at(0)) {
            new_picked = picked_lights->at(1);
            old_picked = picked_lights->at(0);
        } else {
            new_picked = picked_lights->at(0);
            old_picked = picked_lights->at(1);
        }
    }

    inline vega_types::SharedPtr<gfx_light> localLightAtIndex(const size_t i) {
        return l_lights->at(i);
    }
};

extern vega_types::SharedPtr<ManagerOfStaticLightsData> staticLightsDataManager();

///A sortable line collide object that will sort by object addr for dup elim
struct LineCollideStar {
    LineCollide *lc;

    LineCollideStar() : lc(nullptr) {
    }

    bool operator==(const LineCollideStar &b) const {
        return lc->object.i == b.lc->object.i;
    }

    bool operator<(const LineCollideStar &b) const {
        return *((int *) &(lc->object.i)) < *((int *) &(b.lc->object.i));
    }

    inline int GetIndex() {
        return *((int *) (&lc->object.i));
    }
};
///Finds the local lights that are clobberable for new lights (permanent perhaps)
int findLocalClobberable();

#define CTACC 40000
///table to store local lights, numerical pointers to _llights (eg indices)
extern Hashtable3d<LineCollideStar, 20, CTACC, lighthuge> lighttable;

///something that would normally round down
extern float intensity_cutoff;
///optimization globals
extern float optintense;
extern float optsat;

#endif

