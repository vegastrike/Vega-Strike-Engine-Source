/*
 * gl_light_pick.cpp
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


#include "gl_light.h"
#include "root_generic/options.h"
#include <queue>
#include <list>
#include "src/vs_logging.h"
#include "gfx/occlusion.h"

#include <vector>
#include <algorithm>
using std::priority_queue;
#include "hashtable_3d.h"
//using std::list;
using std::vector;
//optimization globals
float intensity_cutoff = 0.06; //something that would normally round down
float optintense = 0.2;
float optsat = 0.95;

struct light_key {
    int number;
    float intensity_key;

    light_key() {
        intensity_key = number = 0;
    }

    light_key(int num, float inte) {
        number = num;
        intensity_key = inte;
    }
};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"  // Disable the unused warning for this function only

static bool operator<(light_key tmp1, light_key tmp2) {
    return tmp1.intensity_key < tmp2.intensity_key;
}

#pragma GCC diagnostic pop

static priority_queue<light_key> lightQ;

//pickedlights was a list, but lists imply heavy reallocation, which is bad in critical sections
//( and pickedlights is used in the most critical section: just before GFXVertexList::Draw() )
static vector<int> pickedlights[2];
static vector<int> *newpicked = &pickedlights[0];
static vector<int> *oldpicked = &pickedlights[1];

void removelightfromnewpick(int index) {
    std::vector<int>::iterator where;
    for (int i = 0; i < 2; ++i) {
        while ((where = std::find(pickedlights[i].begin(), pickedlights[i].end(), index)) != pickedlights[i].end()) {
            pickedlights[i].erase(where);
        }
    }
}

inline int getIndex(const LineCollide &t) {
    return t.object.i;
}

static void swappicked() {
    if (newpicked == &pickedlights[0]) {
        newpicked = &pickedlights[1];
        oldpicked = &pickedlights[0];
    } else {
        newpicked = &pickedlights[0];
        oldpicked = &pickedlights[1];
    }
    newpicked->clear();
}

void unpicklights() {
    for (std::vector<int>::iterator i = newpicked->begin(); i != newpicked->end(); i++) {
        if (*i >= (int) _llights->size()) {
            VS_LOG(error,
                    (boost::format("GFXLIGHT FAILURE %1% is beyond array of size %2%") % ((int) *i)
                            % ((int) _llights->size())));
        }
        if (GLLights[(*_llights)[*i].Target()].index != *i) {
            VS_LOG(error, "GFXLIGHT uh oh");
            (*_llights)[*i].Target() = -1;
            continue;             //a lengthy operation... Since picked lights may have been smashed
        }
        int targ = (*_llights)[*i].Target();
        if (GLLights[targ].options & OpenGLL::GL_ENABLED) {
            glDisable(GL_LIGHT0 + targ);
            GLLights[targ].options = OpenGLL::GLL_LOCAL;
            GLLights[targ].index = -1;
            (*_llights)[*i].Target() = -1;               //unref
        }
    }
    newpicked->clear();
    oldpicked->clear();
}

static float occludedIntensity(const gfx_light &light, const Vector &center, const float rad) {
    return Occlusion::testOcclusion(light.getPosition().Cast(), light.getSize(), center.Cast(), rad);
}

static float attenuatedIntensity(const gfx_light &light, const Vector &center, const float rad) {
    float intensity = (1.0 / 3.0) * (
            light.diffuse[0] + light.specular[0]
                    + light.diffuse[1] + light.specular[1]
                    + light.diffuse[2] + light.specular[2]);
    float distance = float((Vector(light.vect[0], light.vect[1], light.vect[2]) - center).Magnitude()) - rad;
    float cf = light.attenuate[0];     //constant factor
    float lf = light.attenuate[1];     //linear factor
    float qf = light.attenuate[2];     //quadratic factor
    float att = (cf + lf * distance + qf * distance * distance);
    if ((distance <= 0) || (att <= 0)) {
        return 1.f;
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
    const gfx_light &light = (*_llights)[lightindex];
    return (
            !light.attenuated()
                    || (attenuated = attenuatedIntensity(light, center, rad) >= light.cutoff)
    ) && ((occlusion = occludedIntensity(light, center, rad)) * attenuated >= light.cutoff);
}

struct lightsort {
    Vector center;
    float rad;

    lightsort(const Vector &_center, const float _rad) : center(_center), rad(_rad) {
    }

    bool operator()(const int a, const int b) const {
        const gfx_light &lighta = (*_llights)[a];
        const gfx_light &lightb = (*_llights)[b];
        return attenuatedIntensity(lighta, center, rad) > attenuatedIntensity(lightb, center, rad);
    }
};

typedef vector<LineCollideStar> veclinecol;

void GFXGlobalLights(vector<int> &lights, const Vector &center, const float radius) {
    for (int i = 0; i < GFX_MAX_LIGHTS; ++i) {
        if ((GLLights[i].options & (OpenGLL::GL_ENABLED | OpenGLL::GLL_LOCAL)) == OpenGLL::GL_ENABLED) {
            // It's global and enabled
            gfx_light &light = (*_llights)[GLLights[i].index];
            light.occlusion = occludedIntensity(light, center, radius);
            lights.push_back(GLLights[i].index);
        }
    }
}

void GFXGlobalLights(vector<int> &lights) {
    for (int i = 0; i < GFX_MAX_LIGHTS; ++i) {
        if ((GLLights[i].options & (OpenGLL::GL_ENABLED | OpenGLL::GLL_LOCAL)) == OpenGLL::GL_ENABLED) {
            // It's global and enabled
            lights.push_back(GLLights[i].index);
        }
    }
}

void GFXPickLights(const Vector &center,
        const float radius,
        vector<int> &lights,
        const int maxlights,
        const bool pickglobals) {
    QVector tmp;
    //Beware if re-using rndvar !! Because rand returns an int and on 64 bits archs sizeof( void*) != sizeof( int) !!!
    //void * rndvar = (void *)rand();
    int lightsenabled = _GLLightsEnabled;
    tmp = QVector(radius, radius, radius);

    if (lightsenabled && pickglobals) {
        GFXGlobalLights(lights, center, radius);
    }

    veclinecol *tmppickt[2];
    lighttable.Get(center.Cast(), tmppickt);

    for (int j = 0; j < 2; j++) {
        veclinecol::iterator i;
        float attenuated = 0, occlusion = 0;

        for (i = tmppickt[j]->begin(); i != tmppickt[j]->end(); i++) {
            if (picklight(*i->lc, center, radius, lightsenabled, i->GetIndex(), attenuated, occlusion)) {
                int ix = i->GetIndex();
                gfx_light &l = (*_llights)[ix];
                l.occlusion = occlusion;
                lights.push_back(ix);
                lightsenabled++;
            }
        }
    }
    std::sort(lights.begin(), lights.end(), lightsort(center, radius));
}

void GFXPickLights(const Vector &center, const float radius) {
    swappicked();
    GFXPickLights(center, radius, *newpicked, 8, false);
    gfx_light::dopickenables();
}

void GFXPickLights(vector<int>::const_iterator begin, vector<int>::const_iterator end) {
    swappicked();
    newpicked->insert(newpicked->end(), begin, end);
    gfx_light::dopickenables();
}

void gfx_light::dopickenables() {
    //sort it to find minimum num lights changed from last time.
    sort(newpicked->begin(), newpicked->end());
    //newpicked->sort();
    std::vector<int>::iterator traverse;
    std::vector<int>::iterator oldtrav;
    for (traverse = newpicked->begin(); traverse != newpicked->end() && (!oldpicked->empty()); ++traverse) {
        for (oldtrav = oldpicked->begin(); oldtrav != oldpicked->end() && *oldtrav < *traverse;) {
            oldtrav++;
        }
        if (((*traverse) == (*oldtrav)) && ((*_llights)[*oldtrav].target >= 0)) {
            //BOGUS ASSERT... just like this light wasn't on if it was somehow clobberedassert (GLLights[(*_llights)[oldpicked->front()].target].index == oldpicked->front());
            oldpicked->erase(oldtrav);              //already taken care of. main screen turn on ;-)
        }
    }
    for (oldtrav = oldpicked->begin(); oldtrav != oldpicked->end(); ++oldtrav) {
        if (GLLights[(*_llights)[(*oldtrav)].target].index != (*oldtrav)) {
            continue;
        }             //don't clobber what's not yours
        GLLights[(*_llights)[(*oldtrav)].target].index = -1;
        GLLights[(*_llights)[(*oldtrav)].target].options &=
                (OpenGLL::GL_ENABLED & OpenGLL::GLL_LOCAL);               //set it to be desirable to kill
    }
    for (traverse = newpicked->begin(); traverse != newpicked->end(); ++traverse) {
        if (*traverse >= (int) _llights->size()) {
            VS_LOG(error,
                    (boost::format("GFXLIGHT FAILURE %1% is beyond array of size %2%") % ((int) *traverse)
                            % ((int) _llights->size())));
            continue;
        }
        if ((*_llights)[*traverse].target == -1) {
            int gltarg = findLocalClobberable();
            if (gltarg == -1) {
                newpicked->erase(traverse,
                        newpicked->end());                 //erase everything on the picked list. Nothing can fit;
                break;
            }
            (*_llights)[(*traverse)].ClobberGLLight(gltarg);
        } else {
            (*_llights)[(*traverse)].UpdateGLLight();
        }
    }
    /*while (!oldpicked->empty()) {
     *  int glind=(*_llights)[oldpicked->front()].target;
     *  if ((GLLights[glind].options&OpenGLL::GL_ENABLED)&&GLLights[glind].index==-1) {//if hasn't been duly clobbered
     *   glDisable (GL_LIGHT0+glind);
     *   GLLights[glind].options &= (~OpenGLL::GL_ENABLED);
     *  }
     *  (*_llights)[oldpicked->front()].target=-1;//make sure it doesn't think it owns any gl lights!
     *  oldpicked->pop_front();
     *  }*/
    for (oldtrav = oldpicked->begin(); oldtrav != oldpicked->end(); oldtrav++) {
        if (*oldtrav >= (int) _llights->size()) {
            VS_LOG(error,
                    (boost::format("GFXLIGHT FAILURE %1% is beyond array of size %2%") % ((int) *oldtrav)
                            % ((int) _llights->size())));
            continue;
        }

        int glind = (*_llights)[*oldtrav].target;
        if ((GLLights[glind].options & OpenGLL::GL_ENABLED) && GLLights[glind].index == -1) {
            //if hasn't been duly clobbered
            glDisable(GL_LIGHT0 + glind);
            GLLights[glind].options &= (~OpenGLL::GL_ENABLED);
        }
        (*_llights)[*oldtrav].target = -1;           //make sure it doesn't think it owns any gl lights!
    }
    oldpicked->clear();
}

#ifdef MIRACLESAVESDAY

void /*GFXDRVAPI*/ GFXPickLights( const float *transform )
{
    //picks 1-5 lights to use
    //glMatrixMode(GL_MODELVIEW);
    //glPushMatrix();
    //float tm [16]={1,0,0,1000,0,1,0,1000,0,0,1,1000,0,0,0,1};
    //glLoadIdentity();
    //GFXLoadIdentity(MODEL);
    if (!GFXLIGHTING) return;
    Vector loc( transform[12], transform[13], transform[14] );
    SetLocalCompare( loc );
    newQsize = 0;
    unsigned int i;
    light_key    tmpvar;
    unsigned int lightQsize = lightQ.size();
    for (i = 0; i < lightQsize; i++) {
        tmpvar = lightQ.top();
        if (i < GFX_MAX_LIGHTS) {
            //do more stuff with intensity before hacking it to hell
            if (i > GFX_OPTIMAL_LIGHTS) {
                if ( (tmpvar.intensity_key-optintense)*(GFX_MAX_LIGHTS-GFX_OPTIMAL_LIGHTS)
                    < (i-GFX_OPTIMAL_LIGHTS)*(optsat-optintense) )
                    break;                      //no new lights
            } else {
                if (i == GFX_OPTIMAL_LIGHTS-2 && tmpvar.intensity_key < .25*optintense)
                    break;                      //no new lights
                if (i == GFX_OPTIMAL_LIGHTS-1 && tmpvar.intensity_key < .5*optintense)
                    break;                      //no new lights
                if (i == GFX_OPTIMAL_LIGHTS && tmpvar.intensity_key < optintense)
                    break;
            }
            newQsize++;
#ifdef PER_MESH_ATTENUATE
            if ( (*_llights)[i].vect[3] ) {
                //intensity now has become the attenuation factor
                newQ[i] = tmpvar.number;
                AttenuateQ[i] = tmpvar.intensity_key/(*_llights)[i].intensity;
            } else {
#endif
            newQ[i] = tmpvar.number;
#ifdef PER_MESH_ATTENUATE
            AttenuateQ[i] = 0;
        }
#endif
        } else
            break;
        lightQ.pop();
    }
    unsigned int light;
    for (i = 0; i < newQsize; i++) {
        light = newQ[i];
        if ( (*_llights)[light].target >= 0 ) {
#ifdef PER_MESH_ATTENUATE
            if (AttenuateQ[i])
                EnableExistingAttenuated( light, AttenuateQ[i] );
            else
#endif
            EnableExisting( newQ[i] );
            /***daniel 081901
             *  AttTmp[0]=(*_llights)[light].vect[0]-loc.i;
             *  AttTmp[1]=(*_llights)[light].vect[1]-loc.j;
             *  AttTmp[2]=(*_llights)[light].vect[2]-loc.k;
             *  VecT[3]=0;
             *  VecT[0]=AttTmp[0]*transform[0]+AttTmp[1]*transform[1]+AttTmp[2]*transform[2];
             *  VecT[1]=AttTmp[0]*transform[4]+AttTmp[1]*transform[5]+AttTmp[2]*transform[6];
             *  VecT[2]=AttTmp[0]*transform[8]+AttTmp[1]*transform[9]+AttTmp[2]*transform[10];
             *
             *  glLightfv (GL_LIGHT0+(*_llights)[light].target, GL_POSITION, VecT);
             ****/
        }
    }
    for (i = 0; i < GFX_MAX_LIGHTS; i++)
        DisableExisting( i );
    //go through newQ and tag all existing lights for enablement
    //disable rest of lights
    //ForceActivate the rest.
    unsigned int tmp     = 0;     //where to start looking for disabled lights
    unsigned int newtarg = 0;
    for (i = 0; i < newQsize; i++) {
        light = newQ[i];
        if ( (*_llights)[light].target == -1 ) {
            if (AttenuateQ[i])
                ForceEnableAttenuated( light, AttenuateQ[i], tmp, newtarg );
            else
                ForceEnable( light, tmp, newtarg );
            AttTmp[0] = (*_llights)[light].vect[0]-loc.i;
            AttTmp[1] = (*_llights)[light].vect[1]-loc.j;
            AttTmp[2] = (*_llights)[light].vect[2]-loc.k;
            AttTmp[3] = 0;
            VecT[0]   = AttTmp[0]*transform[0]+AttTmp[1]*transform[1]+AttTmp[2]*transform[2];
            VecT[1]   = AttTmp[0]*transform[4]+AttTmp[1]*transform[5]+AttTmp[2]*transform[6];
            VecT[2]   = AttTmp[0]*transform[8]+AttTmp[1]*transform[9]+AttTmp[2]*transform[10];
            glLightfv( GL_LIGHT0+(*_llights)[light].target, GL_POSITION, VecT );

            //glLightfv (GL_LIGHT0+(*llight_dat)[newQ[i]].target, GL_POSITION, /*wrong*/(*_llights)[light].vect);
        }
    }
    //glPopMatrix();
}

/*
 *     curlightloc.intensity=(curlight.specular[0]+curlight.specular[1]+curlight.specular[2]+curlight.diffuse[0]+curlight.diffuse[1]+curlight.diffuse[2]+curlight.ambient[0]+curlight.ambient[1]+curlight.ambient[2])*.333333333333333333333333;//calculate new cuttof val for light 'caching'
 */
#endif

