#ifndef DRAWABLE_H
#define DRAWABLE_H

//#include "gfx/mesh.h"
#include <vector>
#include <map>
#include <string>

class Mesh;
class Flightgroup;
class Unit;

using std::vector;
using std::string;
using std::map;

class Drawable
{

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
    Unit *unitDst;

public:
    double curtime;

    static unsigned int unitCount;

    static std::map< string, Unit * > Units;

    Drawable(Unit* _unitDst);

    bool Init(const char *filename, int faction, Flightgroup *flightgrp = NULL, const char *animationExt = NULL);

    static void UpdateFrames();

    void AnimationStep();

    void clear();

    ~Drawable() { clear(); }

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
