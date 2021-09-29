/**
* gl_light.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
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
#include "gfxlib.h"
#include "hashtable_3d.h"
#include "gl_globals.h"
extern GLint   GFX_MAX_LIGHTS;
extern GLint   GFX_OPTIMAL_LIGHTS;
extern GFXBOOL GFXLIGHTING;
#define GFX_ATTENUATED 1
//#define GFX_DIFFUSE 2
//#define GFX_SPECULAR 4
//#define GFX_AMBIENT 8
//#define GFX_LIGHT_POS 16
#define GFX_LIGHT_ENABLED 32
#define GFX_LOCAL_LIGHT 64
const unsigned int lighthuge = 20*20*20;
/**
 * This stores the state of a given GL Light in its fullness
 * It inherits all values a light may have, and gains a number of functions
 * that assist it in maintaining the virtual OpenGL state amidst the limited
 * Number of lights, etc
 */
class gfx_light : public GFXLight
{
public: gfx_light() : GFXLight() {}
    ///assigns a GFXLight to a gfx_light
    GFXLight operator=( const GFXLight &tmp );

    ///Returns the number this light is in the _llights list
    int lightNum();
    
    ///Returns if this light was saved as a local light
    bool LocalLight() const
    {
        return (options&GFX_LOCAL_LIGHT) != 0;
    }

    ///Retursn if this light is enabled
    bool enabled() const
    {
        return (options&GFX_LIGHT_ENABLED) != 0;
    }

    ///Returns the target OpenGL light of this light. -1 is unassigned to a "real" light
    int& Target()
    {
        return target;
    }

    /**
     * if global, puts it into GLlights (if space ||enabled) <clobber?>
     * for local lights, if enabled, call Enable().
     */
    bool Create( const GFXLight&, bool global );

    /// Disables it (may remove from table), trashes it from GLlights. sets target to -2 (dead)
    void Kill();

    ///properly utilizes union to send to OGL
    void SendGLPosition( const GLenum target ) const;

    ///replaces target GL light in the implementation. Sets this->target! Checks for -1 and calls ContextSwitch to clobber completely
    void ClobberGLLight( const int target );
    
    ///resends target GL light data (assumes valid target)
    void UpdateGLLight() const;

    ///replaces target GL light, copying all state sets this->target!
    inline void FinesseClobberLight( const GLenum target, const int original );

    ///replaces target GL light, copying all state sets this->target!
    inline void ContextSwitchClobberLight( const GLenum target, const int original ) const;

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
    void ResetProperties( const enum LIGHT_TARGET, const GFXColor &color );

    ///Adds this light to table (assume local)
    void AddToTable();

    ///Removes this light from light table
    bool RemoveFromTable( bool shouldremove = true, const GFXLight &t = GFXLight() );

    ///Trash this light from active GLLights
    void TrashFromGLLights();

    ///Do all enables from picking
    static void dopickenables();
    
    ///calculates bounds for the table given cutoffs!
    LineCollide CalculateBounds( bool &err );
};

namespace OpenGLL
{
///If a light is off
const char GLL_OFF    = 0;
///If a light is on
const char GLL_ON     = 1;
///If a light is local or not
const char GLL_LOCAL  = 2;
///If a light is enabled for OpenGL
const char GL_ENABLED = 4;
}

struct OpenGLLights
{
    /// Index into _gllights.  -1 == unassigned
    int  index;
    ///Options GLL_OFF, on GLL_ENABLED
    char options;
};
///Rekeys a frame, remembering trashing old lights activated last frame
void light_rekey_frame();
///picks doubtless changed position
void unpicklights();
void removelightfromnewpick(int whichlight);
///The curren tlight context
extern int _currentContext;
///The light data _llights points to one of these
extern vector< vector< gfx_light > >_local_lights_dat;
///The ambient lights that are around
extern vector< GFXColor >   _ambient_light;
///The lights existing in a certain context. Points to _local_lights_dat
extern vector< gfx_light > *_llights;
///How many lights are enabled (for fast picking)
extern int _GLLightsEnabled;

///currently stored GL lights!
extern OpenGLLights *GLLights;

///A sortable line collide object that will sort by object addr for dup elim
struct LineCollideStar
{
    LineCollide *lc;
    LineCollideStar()
    {
        lc = NULL;
    }
    bool operator==( const LineCollideStar &b ) const
    {
        return lc->object.i == b.lc->object.i;
    }
    bool operator<( const LineCollideStar &b ) const
    {
        return *( (int*) &(lc->object.i) ) < *( (int*) &(b.lc->object.i) );
    }
    inline int GetIndex()
    {
        return *( (int*) (&lc->object.i) );
    }
};
///Finds the local lights that are clobberable for new lights (permanent perhaps)
int findLocalClobberable();

#define CTACC 40000
///table to store local lights, numerical pointers to _llights (eg indices)
extern Hashtable3d< LineCollideStar, 20, CTACC, lighthuge >lighttable;

///something that would normally round down
extern float intensity_cutoff;
///optimization globals
extern float optintense;
extern float optsat;

#endif

