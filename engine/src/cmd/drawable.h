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
};

/*
class MeshAnimation
{
protected:
    std::vector< std::vector<Mesh *> *> vecAnimations;
    std::vector< string > vecAnimationNames;

    bool animatedMesh;
    unsigned int activeAnimation;
    double timeperframe;
    bool   done;
        unsigned int activeMesh;
        unsigned int nextactiveMesh;
        bool infiniteLoop;
        unsigned int    loopCount;

        string uniqueUnitName;
    Unit *unitDst;

public:
    double curtime;

    static unsigned int unitCount;

    static std::map< string, Unit * > Units;

    MeshAnimation(Unit* _unitDst);

    bool Init(const char *filename, int faction, Flightgroup *flightgrp = NULL, const char *animationExt = NULL);

    static void UpdateFrames();

    void AnimationStep();

    void clear();

    ~MeshAnimation() { clear(); }

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
};*/

#endif // DRAWABLE_H
