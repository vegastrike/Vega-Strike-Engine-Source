/*
 * gl_light.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Alan Shieh
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


#include <stack>

#include "configuration/configuration.h"
using std::stack;
#include <assert.h>
#include "root_generic/vs_globals.h"
#include "src/gfxlib.h"
#include "gl_light.h"
#include "src/config_xml.h"
#include "root_generic/options.h"

GLint GFX_MAX_LIGHTS = 8;
GLint GFX_OPTIMAL_LIGHTS = 4;
GFXBOOL GFXLIGHTING = GFXFALSE;

int _currentContext = 0;
vector<vector<gfx_light> > _local_lights_dat;
vector<GFXColor> _ambient_light;
vector<gfx_light> *_llights = nullptr;

//currently stored GL lights!
OpenGLLights *GLLights = nullptr; //{-1,-1,-1,-1,-1,-1,-1,-1};
static stack<bool *> GlobalEffects;
static stack<bool *> GlobalEffectsFreelist;
static stack<GFXColor> GlobalEffectsAmbient;

void /*GFXDRVAPI*/ GFXPushGlobalEffects() {
    bool *tmp;
    if (GlobalEffectsFreelist.empty()) {
        tmp = new bool[GFX_MAX_LIGHTS];
    } else {
        tmp = GlobalEffectsFreelist.top();
        GlobalEffectsFreelist.pop();
    }

    unpicklights();     //costly but necessary to get rid of pesky local enables that shouldn't be tagged to get re-enabled
    for (int i = 0; i < GFX_MAX_LIGHTS; i++) {
        tmp[i] = (0 != (GLLights[i].options & OpenGLL::GL_ENABLED));
        if (GLLights[i].options & OpenGLL::GL_ENABLED) {
            glDisable(GL_LIGHT0 + i);
        }
    }
    GlobalEffects.push(tmp);
    GlobalEffectsAmbient.push(_ambient_light.at(_currentContext));
    GFXLightContextAmbient(GFXColor(0, 0, 0, 1));
}

GFXBOOL /*GFXDRVAPI*/ GFXPopGlobalEffects() {
    if (GlobalEffects.empty()) {
        return false;
    }
    for (int i = 0; i < GFX_MAX_LIGHTS; i++) {
        if (GlobalEffects.top()[i]) {
            glEnable(GL_LIGHT0 + i);
        }
    }

    if (GlobalEffectsFreelist.size() >= 10) {
        delete[] GlobalEffects.top();
    } else {
        GlobalEffectsFreelist.push(GlobalEffects.top());
    }
    GlobalEffects.pop();

    GFXLightContextAmbient(GlobalEffectsAmbient.top());
    GlobalEffectsAmbient.pop();
    return true;
}

GFXLight::GFXLight(const bool enabled,
        const GFXColor &vect,
        const GFXColor &diffuse,
        const GFXColor &specular,
        const GFXColor &ambient,
        const GFXColor &attenuate,
        const GFXColor &direction,
        float exp,
        float cutoff,
        float size) {
    target = -1;
    options = 0;
    memcpy(this->vect, &vect, sizeof(float) * 3);
    memcpy(this->diffuse, &diffuse, sizeof(float) * 4);
    memcpy(this->specular, &specular, sizeof(float) * 4);
    memcpy(this->ambient, &ambient, sizeof(float) * 4);
    memcpy(this->attenuate, &attenuate, sizeof(float) * 3);
    memcpy(this->direction, &direction, sizeof(this->direction));
    this->exp = exp;
    this->cutoff = cutoff;
    this->size = size;
    apply_attenuate(attenuated());
    if (enabled) {
        this->enable();
    } else {
        this->disable();
    }
}

void GFXLight::disable() {
    options &= (~GFX_LIGHT_ENABLED);
}

void GFXLight::enable() {
    options |= GFX_LIGHT_ENABLED;
}

bool GFXLight::attenuated() const {
    return (attenuate[0] != 1) || (attenuate[1] != 0) || (attenuate[2] != 0);
}

void GFXLight::apply_attenuate(bool attenuated) {
    options = attenuated
            ? (options | GFX_ATTENUATED)
            : (options & (~GFX_ATTENUATED));
}

void /*GFXDRVAPI*/ GFXLight::SetProperties(enum LIGHT_TARGET lighttarg, const GFXColor &color) {
    switch (lighttarg) {
        case DIFFUSE:
            diffuse[0] = color.r;
            diffuse[1] = color.g;
            diffuse[2] = color.b;
            diffuse[3] = color.a;
            break;
        case SPECULAR:
            specular[0] = color.r;
            specular[1] = color.g;
            specular[2] = color.b;
            specular[3] = color.a;
            break;
        case AMBIENT:
            ambient[0] = color.r;
            ambient[1] = color.g;
            ambient[2] = color.b;
            ambient[3] = color.a;
            break;
        case POSITION:
            vect[0] = color.r;
            vect[1] = color.g;
            vect[2] = color.b;
            break;
        case ATTENUATE:
            attenuate[0] = color.r;
            attenuate[1] = color.g;
            attenuate[2] = color.b;
            break;
        case EMISSION:
        default:
            break;
    }
    apply_attenuate(attenuated());
}

void /*GFXDRVAPI*/ GFXLight::SetProperties(const enum LIGHT_TARGET light_target, const Vector& vector) {
    switch (light_target) {
        case DIFFUSE:
        case SPECULAR:
        case AMBIENT:
            VS_LOG(error, (boost::format("%1%: Called wrong overload for this property, with Vector& instead of GFXColor&")
                    % __FUNCTION__));
            break;
        case POSITION:
            vect[0] = vector.i;
            vect[1] = vector.j;
            vect[2] = vector.k;
            break;
        case ATTENUATE:
            attenuate[0] = vector.i;
            attenuate[1] = vector.j;
            attenuate[2] = vector.k;
            break;
        case EMISSION:
        default:
            break;
    }
    apply_attenuate(attenuated());
}

GFXColor /*GFXDRVAPI*/ GFXLight::GetProperties(enum LIGHT_TARGET lighttarg) const {
    switch (lighttarg) {
        case SPECULAR:
            return GFXColor(specular[0],
                    specular[1],
                    specular[2],
                    specular[3]);

        case AMBIENT:
            return GFXColor(ambient[0],
                    ambient[1],
                    ambient[2],
                    ambient[3]);

        case POSITION:
            return GFXColor(vect[0],
                    vect[1],
                    vect[2]);

            break;
        case ATTENUATE:
            return GFXColor(
                    attenuate[0],
                    attenuate[1],
                    attenuate[2]);

        case DIFFUSE:
        default:     //just for kicks
            return GFXColor(diffuse[0],
                    diffuse[1],
                    diffuse[2],
                    diffuse[3]);
    }
}

GFXBOOL /*GFXDRVAPI*/ GFXSetCutoff(const float ttcutoff) {
    if (ttcutoff < 0) {
        return GFXFALSE;
    }
    intensity_cutoff = ttcutoff;
    return GFXTRUE;
}

void /*GFXDRVAPI*/ GFXSetOptimalIntensity(const float intensity, const float saturate) {
    optintense = intensity;
    optsat = saturate;
}

GFXBOOL /*GFXDRVAPI*/ GFXSetOptimalNumLights(const int numLights) {
    if (numLights > GFX_MAX_LIGHTS || numLights < 0) {
        return GFXFALSE;
    }
    GFX_OPTIMAL_LIGHTS = numLights;
    return GFXTRUE;
}

GFXBOOL /*GFXDRVAPI*/ GFXSetSeparateSpecularColor(const GFXBOOL spec) {
#if !defined (_WIN32) && !defined (__CYGWIN__)
    if (spec) {
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    } else {
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
        return GFXFALSE;
    }
#else
    return GFXFALSE;
#endif
    return GFXTRUE;
}

GFXBOOL /*GFXDRVAPI*/ GFXLightContextAmbient(const GFXColor &amb) {
    if (_currentContext >= static_cast<int>(_ambient_light.size())) {
        return GFXFALSE;
    }
    (_ambient_light.at(_currentContext)) = amb;
    //(_ambient_light.at(_currentContext))[1]=amb.g;
    //(_ambient_light.at(_currentContext))[2]=amb.b;
    //(_ambient_light.at(_currentContext))[3]=amb.a;
    float tmp[4] = {amb.r, amb.g, amb.b, amb.a};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, tmp);
    return GFXTRUE;
}

GFXBOOL /*GFXDRVAPI*/ GFXGetLightContextAmbient(GFXColor &amb) {
    if (_currentContext >= static_cast<int>(_ambient_light.size())) {
        return GFXFALSE;
    }
    amb = (_ambient_light.at(_currentContext));
    return GFXTRUE;
}

GFXBOOL /*GFXDRVAPI*/ GFXCreateLight(int &light, const GFXLight &templatecopy, const bool global) {
    for (light = 0; light < static_cast<int>(_llights->size()); light++) {
        if (_llights->at(light).Target() == -2) {
            break;
        }
    }
    if (light == static_cast<int>(_llights->size())) {
        VS_LOG_AND_FLUSH(debug, (boost::format("%1%: Creating light number %2% in context %3%") % __FUNCTION__ % light % _currentContext));
        _llights->emplace_back();
    }
    return _llights->at(light).Create(templatecopy, global);
}

void /*GFXDRVAPI*/ GFXDeleteLight(const int light) {
    VS_LOG_AND_FLUSH(debug, (boost::format("%1%: Deleting light number %2% from context %3%") % __FUNCTION__ % light % _currentContext));
    _llights->at(light).Kill();
}

GFXBOOL /*GFXDRVAPI*/ GFXSetLight(const int light, const enum LIGHT_TARGET lt, const GFXColor &color) {
    if (_llights->at(light).Target() == -2) {
        return GFXFALSE;
    }
    _llights->at(light).ResetProperties(lt, color);

    return GFXTRUE;
}

GFXBOOL /*GFXDRVAPI*/  GFXSetLight(const int light, const enum LIGHT_TARGET lt, const Vector& vector) {
    if (_llights->size() <= light || _llights->at(light).Target() == -2) {
        return GFXFALSE;
    }
    _llights->at(light).ResetProperties(lt, vector);

    return GFXTRUE;
}

const GFXLight & /*GFXDRVAPI*/ GFXGetLight(const int light) {
    if (light < 0) {
        VS_LOG_AND_FLUSH(error, (boost::format("%1%: light # %2% < zero") % __FUNCTION__ % light));
        throw std::out_of_range("GFXGetLight()");
    } else if (light >= static_cast<int>(_llights->size())) {
        VS_LOG_AND_FLUSH(error, (boost::format("%1%: light # %2% is past the upper bound of the collection %3%") % __FUNCTION__ % light % _llights->size()));
        throw std::out_of_range("GFXGetLight()");
    }
    return _llights->at(light);
}

GFXBOOL /*GFXDRVAPI*/ GFXEnableLight(int light) {
    if (light < 0) {
        VS_LOG_AND_FLUSH(error, (boost::format("%1%: light # %2% < zero") % __FUNCTION__ % light));
        return GFXFALSE;
    } else if (light >= static_cast<int>(_llights->size())) {
        VS_LOG_AND_FLUSH(error, (boost::format("%1%: light # %2% is past the upper bound of the collection %3%") % __FUNCTION__ % light % _llights->size()));
        return GFXFALSE;
    }
    // assert(light >= 0 && light < static_cast<int>(_llights->size()));
    //return FALSE;
    if (_llights->at(light).Target() == -2) {
        return GFXFALSE;
    }
    _llights->at(light).Enable();
    return GFXTRUE;
}

GFXBOOL /*GFXDRVAPI*/ GFXDisableLight(int light) {
    if (light < 0) {
        VS_LOG_AND_FLUSH(error, (boost::format("%1%: light # %2% < zero") % __FUNCTION__ % light));
        return GFXFALSE;
    } else if (light >= static_cast<int>(_llights->size())) {
        VS_LOG_AND_FLUSH(error, (boost::format("%1%: light # %2% is past the upper bound of the collection %3%") % __FUNCTION__ % light % _llights->size()));
        return GFXFALSE;
    }
    // assert(light >= 0 && light < static_cast<int>(_llights->size()));
    if (_llights->at(light).Target() == -2) {
        return GFXFALSE;
    }
    _llights->at(light).Disable();
    return GFXTRUE;
}

static void SetupGLLightGlobals();

void /*GFXDRVAPI*/ GFXCreateLightContext(int &con_number) {
    static GFXBOOL LightInit = GFXFALSE;
    if (!LightInit) {
        LightInit = GFXTRUE;
        SetupGLLightGlobals();
    }
    con_number = _local_lights_dat.size();
    _currentContext = con_number;
    _ambient_light.emplace_back(0, 0, 0, 1);
    _local_lights_dat.emplace_back();
    VS_LOG_AND_FLUSH(debug, (boost::format("%1%: setting light context to %2%, containing %3% lights") % __FUNCTION__ % con_number % _local_lights_dat.at(con_number).size()));
    GFXSetLightContext(con_number);
}

void /*GFXDRVAPI*/ GFXDeleteLightContext(int con_number) {
    VS_LOG_AND_FLUSH(debug, (boost::format("%1%: deleting light context %2%, containing %3% lights") % __FUNCTION__ % con_number % _local_lights_dat.at(con_number).size()));
    _local_lights_dat.at(con_number) = vector<gfx_light>();
}

void /*GFXDRVAPI*/ GFXSetLightContext(const int con_number) {
    unpicklights();
    int GLLindex = 0;
    unsigned int i;
    lighttable.Clear();
    _currentContext = con_number;
    _llights = &_local_lights_dat.at(con_number);
    VS_LOG_AND_FLUSH(debug, (boost::format("%1%: current light context: %2%, containing %3% lights") % __FUNCTION__ % con_number % _llights->size()));
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, reinterpret_cast<GLfloat *>(&(_ambient_light.at(con_number))));
    //reset all lights so they aren't in GLLights
    for (i = 0; i < _llights->size(); i++) {
        _llights->at(i).Target() = -1;
    }
    for (i = 0; i < _llights->size() && GLLindex < GFX_MAX_LIGHTS; i++) {
        if (_llights->at(i).enabled()) {
            if (_llights->at(i).LocalLight()) {
                _llights->at(i).AddToTable();
            } else {
                GLLights[GLLindex].index = -1;                 //make it clobber completely! no trace of old light.
                _llights->at(i).ClobberGLLight(GLLindex);
                GLLindex++;
            }
        }
    }
    for (; GLLindex < GFX_MAX_LIGHTS; GLLindex++) {
        GLLights[GLLindex].index = -1;
        GLLights[GLLindex].options = OpenGLL::GLL_OFF;
        glDisable(GL_LIGHT0 + GLLindex);
    }
}

void GFXDestroyAllLights() {
    VS_LOG_AND_FLUSH(debug, (boost::format("%1%: current light context: %2%, containing %3% lights") % __FUNCTION__ % _currentContext % _llights->size()));
    lighttable.Clear();
    if (GLLights != nullptr) {
        free(GLLights);
        GLLights = nullptr;
    }
}

static void SetupGLLightGlobals() {
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,
            1);     //don't want lighting coming from infinity....we have to take the hit due to sphere mapping matrix tweaking
    glGetIntegerv(GL_MAX_LIGHTS, &GFX_MAX_LIGHTS);
    if (!GLLights) {
        GLLights = static_cast<OpenGLLights *>(malloc(sizeof(OpenGLLights) * GFX_MAX_LIGHTS));
        for (int i = 0; i < GFX_MAX_LIGHTS; i++) {
            GLLights[i].index = -1;
        }
    }

    GFXSetCutoff(configuration()->graphics.light_cutoff_flt);
    GFXSetOptimalIntensity(configuration()->graphics.light_optimal_intensity_flt, configuration()->graphics.light_saturation_flt);
    VS_LOG_AND_FLUSH(debug, (boost::format("%1%: configuration()->graphics.num_lights == %2%") % __FUNCTION__ % configuration()->graphics.num_lights));
    GFXSetOptimalNumLights(configuration()->graphics.num_lights);
    GFXSetSeparateSpecularColor(configuration()->graphics.separate_specular_color ? GFXTRUE : GFXFALSE);
}
