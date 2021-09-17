/**
 * drawable.h
 *
 * Copyright (C) 2020 Roy Falk, Stephen G. Tuggy and other Vega Strike
 * contributors
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


#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "gfx/quaternion.h"
#include <vector>
#include <map>
#include <string>

class Mesh;
class Flightgroup;
class Unit;
//class Transformation;

using std::vector;
using std::string;
using std::map;

class Drawable
{
public:
  //number of meshes (each with separate texture) this unit has
  std::vector< Mesh* >meshdata;

protected:
    vector< vector<Mesh *> *> vecAnimations;
    vector< string > vecAnimationNames;

    bool animatedMesh;
    unsigned int activeAnimation;
    double timeperframe;
    bool   done;
    unsigned int activeMesh;
    unsigned int nextactiveMesh;
    bool infiniteLoop;
    unsigned int    loopCount;

    string uniqueUnitName;

public:
    double curtime;

    static unsigned int unitCount;

    static std::map< string, Unit * > Units;

    Drawable();

    bool DrawableInit(const char *filename, int faction, Flightgroup *flightgrp = NULL, const char *animationExt = NULL);

    static void UpdateFrames();

    void AnimationStep();

    void clear();

protected:
    virtual ~Drawable() { clear(); }
    // forbidden
    Drawable( const Drawable& ) = delete;
    // forbidden
    Drawable& operator=( const Drawable& ) = delete;

public:
    string getAnimationName(unsigned int animationNumber) const;

    unsigned int getAnimationNumber(const char *name) const;

    void ChangeAnimation( const char *name );

    void ChangeAnimation( unsigned int AnimNumber );

    //set how_many_times to 0 for continuous loop animation
    void StartAnimation( unsigned int how_many_times = 0, int numAnimation = 0 );

    void StopAnimation();

    bool isAnimatedMesh() const;

    bool animationRuns() const;

    unsigned int numAnimations();

    bool isContinuousLoop() const;

    void addAnimation( std::vector<Mesh *> *meshes, const char* name );

    double framesPerSecond() const;

    double timePerFrame() const;

    void ToggleAnimatedMesh( bool on );

    void SetAniSpeed( float speed );

    //Uses GFX so only in Unit class
    virtual void Draw( const Transformation &quat = identity_transformation, const Matrix &m = identity_matrix ) = 0;
    virtual void DrawNow( const Matrix &m = identity_matrix, float lod = 1000000000 ) = 0;
    virtual std::string drawableGetName() = 0;

    void Sparkle(bool on_screen, Matrix *ctm);
    void DrawHalo(bool on_screen, float apparent_size, Matrix wmat, int cloak);
    void DrawSubunits(bool on_screen, Matrix wmat, int cloak, float average_scale, unsigned char char_damage);
};

Matrix* GetCumulativeTransformationMatrix(Unit *unit, const Matrix &parentMatrix, Matrix invview);

#endif // DRAWABLE_H
