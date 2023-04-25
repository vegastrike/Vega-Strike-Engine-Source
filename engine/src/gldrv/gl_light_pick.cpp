/*
 * gl_light_pick.cpp
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


#include "gl_light.h"
#include "options.h"
#include <queue>
#include <list>
#include "vs_logging.h"
#include "gfx/occlusion.h"

#include <vector>
#include <algorithm>
#include "preferred_types.h"
using std::priority_queue;
#include "hashtable_3d.h"
//using std::list;
using std::vector;
using namespace vega_types;

//optimization globals
static const constexpr float kDefaultIntensityCutoff = 0.06F;
float intensity_cutoff = kDefaultIntensityCutoff; //something that would normally round down
static const constexpr float kDefaultOptIntense = 0.2F;
float optintense = kDefaultOptIntense;
static const constexpr float kDefaultOptSat = 0.95F;
float optsat = kDefaultOptSat;

struct light_key {
    int number;
    float intensity_key;

    light_key() : number(0), intensity_key(0.0F) {
    }

    light_key(int num, float inte) : number(num), intensity_key(inte) {
    }
};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"  // Disable the unused warning for this function only

static bool operator<(light_key tmp1, light_key tmp2) {
    return tmp1.intensity_key < tmp2.intensity_key;
}

#pragma GCC diagnostic pop

static SharedPtr<priority_queue<light_key>> lightQ() {
    static const SharedPtr<priority_queue<light_key>> kLightQ = MakeShared<priority_queue<light_key>>();
    return kLightQ;
}

void removeLightFromNewPick(int index) {
    for (int i = 0; i < 2; ++i) {
        SharedPtr<SequenceContainer<int>> const &p_collection = staticLightsDataManager()->picked_lights->at(i);
        auto first_to_remove = std::stable_partition(p_collection->begin(), p_collection->end(), [index](int elem) { return elem != index; });
        p_collection->erase(first_to_remove, p_collection->end());
    }
}

inline int getIndex(const LineCollide &t) {
    return t.object.i;
}

void unpickLights() {
    for (int & i : *staticLightsDataManager()->new_picked) {
        if (i >= staticLightsDataManager()->l_lights->size()) {
            VS_LOG(error,
                    (boost::format("GFXLIGHT FAILURE %1% is beyond array of size %2%") % i % staticLightsDataManager()->l_lights->size()));
        }
        if (staticLightsDataManager()->gl_lights->at(staticLightsDataManager()->localLightAtIndex(i)->Target())->index != i) {
            VS_LOG(error, "GFXLIGHT uh oh");
            staticLightsDataManager()->localLightAtIndex(i)->Target() = -1;
            continue;             //a lengthy operation... Since picked lights may have been smashed
        }
        int const targ = staticLightsDataManager()->localLightAtIndex(i)->Target();
        if (staticLightsDataManager()->gl_lights->at(targ)->options & OpenGLL::GL_ENABLED) {
            glDisable(GL_LIGHT0 + targ);
            staticLightsDataManager()->gl_lights->at(targ)->options = OpenGLL::GLL_LOCAL;
            staticLightsDataManager()->gl_lights->at(targ)->index = -1;
            staticLightsDataManager()->localLightAtIndex(i)->Target() = -1;               //unref
        }
    }
    staticLightsDataManager()->new_picked->clear();
    staticLightsDataManager()->old_picked->clear();
}

vega_types::SharedPtr<ManagerOfStaticLightsData> staticLightsDataManager() {
    static const SharedPtr<ManagerOfStaticLightsData> kStaticLightsDataManager = MakeShared<ManagerOfStaticLightsData>();
    return kStaticLightsDataManager;
}

static float occludedIntensity(const gfx_light &light, const Vector &center, const float rad) {
    return Occlusion::testOcclusion(light.getPosition().Cast(), light.getSize(), center.Cast(), rad);
}

static float attenuatedIntensity(const gfx_light &light, const Vector &center, const float rad) {
    float const intensity = (1.0F / 3.0F) * (
            light.diffuse[0] + light.specular[0]
                    + light.diffuse[1] + light.specular[1]
                    + light.diffuse[2] + light.specular[2]);
    float const distance = float((Vector(light.vect[0], light.vect[1], light.vect[2]) - center).Magnitude()) - rad;
    float const cf = light.attenuate[0];     //constant factor
    float const lf = light.attenuate[1];     //linear factor
    float const qf = light.attenuate[2];     //quadratic factor
    float const att = (cf + lf * distance + qf * distance * distance);
    if ((distance <= 0.0F) || (att <= 0.0F)) {
        return 1.0F;
    } else {
        return (intensity / att) >= light.cutoff;
    }
}

static bool picklight(const LineCollide &lightcollide,
        const Vector &center,
        const float rad,
        const int lightsenabled,
        const int lightindex,
        float &attenuated,
        float &occlusion) {
    const gfx_light &light = *(staticLightsDataManager()->localLightAtIndex(lightindex));
    return (
            !light.attenuated()
                    || ((attenuated = attenuatedIntensity(light, center, rad)) >= light.cutoff)
    ) && ((occlusion = occludedIntensity(light, center, rad)) * attenuated >= light.cutoff);
}

struct lightsort {
    Vector center;
    float rad;

    lightsort(const Vector &_center, const float _rad) : center(_center), rad(_rad) {
    }

    bool operator()(const int a, const int b) const {
        const gfx_light &lighta = *(staticLightsDataManager()->localLightAtIndex(a));
        const gfx_light &lightb = *(staticLightsDataManager()->localLightAtIndex(b));
        return attenuatedIntensity(lighta, center, rad) > attenuatedIntensity(lightb, center, rad);
    }
};

typedef vector<LineCollideStar> veclinecol;

void GFXGlobalLights(SequenceContainer<int> &lights, const Vector &center, const float radius) {
    for (int i = 0; i < GFX_MAX_LIGHTS; ++i) {
        if ((staticLightsDataManager()->gl_lights->at(i)->options & (OpenGLL::GL_ENABLED | OpenGLL::GLL_LOCAL)) == OpenGLL::GL_ENABLED) {
            // It's global and enabled
            gfx_light &light = *(staticLightsDataManager()->localLightAtIndex(staticLightsDataManager()->gl_lights->at(i)->index));
            light.occlusion = occludedIntensity(light, center, radius);
            lights.push_back(staticLightsDataManager()->gl_lights->at(i)->index);
        }
    }
}

void GFXGlobalLights(SequenceContainer<int> &lights) {
    for (int i = 0; i < GFX_MAX_LIGHTS; ++i) {
        if ((staticLightsDataManager()->gl_lights->at(i)->options & (OpenGLL::GL_ENABLED | OpenGLL::GLL_LOCAL)) == OpenGLL::GL_ENABLED) {
            // It's global and enabled
            lights.push_back(staticLightsDataManager()->gl_lights->at(i)->index);
        }
    }
}

void GFXPickLights(const Vector &center,
                   const float radius,
                   SequenceContainer<int> &lights,
                   const int maxlights,
                   const bool pickglobals) {
    QVector tmp;
    //Beware if re-using rndvar !! Because rand returns an int and on 64 bits archs sizeof( void*) != sizeof( int) !!!
    //void * rndvar = (void *)rand();
    int lights_enabled = staticLightsDataManager()->gl_lights_enabled;
    tmp = QVector(radius, radius, radius);

    if (lights_enabled && pickglobals) {
        GFXGlobalLights(lights, center, radius);
    }

    veclinecol *tmppickt[2];
    lighttable.Get(center.Cast(), static_cast<veclinecol **>(tmppickt));

    for (auto & j : tmppickt) {
        veclinecol::iterator i;
        float attenuated = 0, occlusion = 0;

        for (i = j->begin(); i != j->end(); i++) {
            if (picklight(*i->lc, center, radius, lights_enabled, i->GetIndex(), attenuated, occlusion)) {
                int const ix = i->GetIndex();
                gfx_light &l = *(staticLightsDataManager()->localLightAtIndex(ix));
                l.occlusion = occlusion;
                lights.push_back(ix);
                lights_enabled++;
            }
        }
    }
    std::sort(lights.begin(), lights.end(), lightsort(center, radius));
}

void GFXPickLights(const Vector &center, const float radius) {
    staticLightsDataManager()->swapPicked();
    GFXPickLights(center, radius, *staticLightsDataManager()->new_picked, 8, false);
    gfx_light::dopickenables();
}

void GFXPickLights(vector<int>::const_iterator begin, vector<int>::const_iterator end) {
    staticLightsDataManager()->swapPicked();
    staticLightsDataManager()->new_picked->insert(staticLightsDataManager()->new_picked->end(), begin, end);
    gfx_light::dopickenables();
}

void gfx_light::dopickenables() {
    //sort it to find minimum num lights changed from last time.
    std::sort(staticLightsDataManager()->new_picked->begin(), staticLightsDataManager()->new_picked->end());
    //newpicked->sort();
    SequenceContainer<int>::iterator traverse;
    SequenceContainer<int>::iterator oldtrav;
    for (traverse = staticLightsDataManager()->new_picked->begin();
            traverse != staticLightsDataManager()->new_picked->end() && (!staticLightsDataManager()->old_picked->empty());
            ++traverse) {
        for (oldtrav = staticLightsDataManager()->old_picked->begin();
                oldtrav != staticLightsDataManager()->old_picked->end() && *oldtrav < *traverse;
                ) {
            oldtrav++;
        }
        if (((*traverse) == (*oldtrav)) && (staticLightsDataManager()->localLightAtIndex(*oldtrav)->Target() >= 0)) {
            //BOGUS ASSERT [according to a previous coder] ... just like this light wasn't on if it was somehow clobbered
            //assert (GLLights[(*_llights)[oldpicked->front()].target].index == oldpicked->front());
            staticLightsDataManager()->old_picked->erase(oldtrav);              //already taken care of. main screen turn on ;-)
        }
    }
    for (oldtrav = staticLightsDataManager()->old_picked->begin(); oldtrav != staticLightsDataManager()->old_picked->end(); ++oldtrav) {
        int const old_local_light_target = staticLightsDataManager()->localLightAtIndex(*oldtrav)->target;
        SharedPtr<OpenGLLights> const & old_light = staticLightsDataManager()->gl_lights->at(old_local_light_target);
        if (old_light->index != (*oldtrav)) {
            continue;
        }             //don't clobber what's not yours
        old_light->index = -1;
        old_light->options &= (OpenGLL::GL_ENABLED & OpenGLL::GLL_LOCAL);               //set it to be desirable to kill
    }
    for (traverse = staticLightsDataManager()->new_picked->begin(); traverse != staticLightsDataManager()->new_picked->end(); ++traverse) {
        if (*traverse >= staticLightsDataManager()->l_lights->size()) {
            VS_LOG(error,
                    (boost::format("GFXLIGHT FAILURE %1% is beyond array of size %2%") % (*traverse)
                            % (staticLightsDataManager()->l_lights->size())));
            continue;
        }
        if (staticLightsDataManager()->localLightAtIndex(*traverse)->target == -1) {
            int const gltarg = findLocalClobberable();
            if (gltarg == -1) {
                staticLightsDataManager()->new_picked->erase(traverse,
                        staticLightsDataManager()->new_picked->end());                 //erase everything on the picked list. Nothing can fit;
                break;
            }
            staticLightsDataManager()->localLightAtIndex(*traverse)->ClobberGLLight(gltarg);
        } else {
            staticLightsDataManager()->localLightAtIndex(*traverse)->UpdateGLLight();
        }
    }
    for (oldtrav = staticLightsDataManager()->old_picked->begin(); oldtrav != staticLightsDataManager()->old_picked->end(); oldtrav++) {
        if (*oldtrav >= staticLightsDataManager()->l_lights->size()) {
            VS_LOG(error,
                    (boost::format("GFXLIGHT FAILURE %1% is beyond array of size %2%") % (*oldtrav)
                            % (staticLightsDataManager()->l_lights->size())));
            continue;
        }

        int const glind = staticLightsDataManager()->localLightAtIndex(*oldtrav)->target;
        if ((staticLightsDataManager()->gl_lights->at(glind)->options & OpenGLL::GL_ENABLED) && staticLightsDataManager()->gl_lights->at(glind)->index == -1) {
            //if hasn't been duly clobbered
            glDisable(GL_LIGHT0 + glind);
            staticLightsDataManager()->gl_lights->at(glind)->options &= (~OpenGLL::GL_ENABLED);
        }
        staticLightsDataManager()->localLightAtIndex(*oldtrav)->target = -1;           //make sure it doesn't think it owns any gl lights!
    }
    staticLightsDataManager()->old_picked->clear();
}

