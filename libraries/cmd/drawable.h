/*
 * drawable.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_DRAWABLE_H
#define VEGA_STRIKE_ENGINE_CMD_DRAWABLE_H

#include "gfx_generic/quaternion.h"
#include "gfx/halo_system.h"
#include <vector>
#include <map>
#include <string>
#include <memory>

#include "components/cloak.h"

class Mesh;
class Flightgroup;
class Unit;
struct GFXColor;
class VSSprite;

using std::vector;
using std::string;
using std::map;

class Drawable {
public:
    //number of meshes (each with separate texture) this unit has
    std::vector<Mesh *> meshdata;
    std::unique_ptr<HaloSystem> halos;

protected:
    vector<vector<Mesh *> *> vecAnimations;
    vector<string> vecAnimationNames;

    bool animatedMesh;
    unsigned int activeAnimation;
    double timeperframe;
    bool done;
    unsigned int activeMesh;
    unsigned int nextactiveMesh;
    bool infiniteLoop;
    unsigned int loopCount;

    string uniqueUnitName;

    unsigned int num_chunks{};
public:
    static std::string root;

    double curtime;

    static unsigned int unitCount;

    static std::map<string, Unit *> Units;

    Drawable();
    virtual ~Drawable();

    bool DrawableInit(const char *filename,
            int faction,
            Flightgroup *flightgrp = nullptr,
            const char *animationExt = nullptr);

    static void UpdateFrames();

    void AnimationStep();

    void clear();

    // forbidden
    Drawable(const Drawable &) = delete;
    // forbidden
    Drawable &operator=(const Drawable &) = delete;

    string getAnimationName(unsigned int animationNumber) const;

    unsigned int getAnimationNumber(const char *name) const;

    void ChangeAnimation(const char *name);

    void ChangeAnimation(unsigned int AnimNumber);

    //set how_many_times to 0 for continuous loop animation
    void StartAnimation(unsigned int how_many_times = 0, int numAnimation = 0);

    void StopAnimation();

    bool isAnimatedMesh() const;

    bool animationRuns() const;

    unsigned int numAnimations() const;

    bool isContinuousLoop() const;

    void addAnimation(std::vector<Mesh *> *meshes, const char *name);

    double framesPerSecond() const;

    double timePerFrame() const;

    void ToggleAnimatedMesh(bool on);

    void SetAniSpeed(float speed);

    //Uses GFX so only in Unit class
    virtual void Draw(const Transformation &quat = identity_transformation,
            const Matrix &m = identity_matrix);

    ///Draws this unit with the transformation and matrix (should be equiv) separately
    virtual void DrawNow(const Matrix &m = identity_matrix, float lod = 1000000000);
    virtual std::string drawableGetName() = 0;

    void Sparkle(bool on_screen, const Matrix *ctm);
    void DrawHalo(bool on_screen, float apparent_size, Matrix wmat, Cloak cloak);
    void DrawSubunits(bool on_screen, Matrix wmat, Cloak cloak, float average_scale, unsigned char char_damage);

    //Split this mesh with into 2^level submeshes at arbitrary planes
    virtual void Split(int level);

    unsigned int nummesh() const {
        // Return number of meshes except shield
        return (meshdata.size() - 1);
    }

    //Lights the shields, without applying damage or making the AI mad - useful for special effects
    void LightShields(const Vector &pnt, const Vector &normal, float amt, const GFXColor &color);

    ///Holds temporary values for inter-function XML communication Saves deprecated restr info
    Matrix WarpMatrix(const Matrix &ctm) const;

    ///Sets the camera to be within this unit.
    void UpdateHudMatrix(int whichcam);

    VSSprite *getHudImage() const;
};

Matrix *GetCumulativeTransformationMatrix(Unit *unit, const Matrix &parentMatrix, Matrix invview);

#endif //VEGA_STRIKE_ENGINE_CMD_DRAWABLE_H
