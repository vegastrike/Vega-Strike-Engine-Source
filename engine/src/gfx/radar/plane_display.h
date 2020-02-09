// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGASTRIKE_GFX_RADAR_PLANE_DISPLAY_H
#define VEGASTRIKE_GFX_RADAR_PLANE_DISPLAY_H

#include <vector>
#include <queue>
#include "gfx/vec.h"
#include "gfx/matrix.h"
#include "sensor.h"
#include "dual_display.h"

class VSSprite;
struct GFXColor;  // Edit from class to struct as defined in gfxlib_struct.

namespace Radar
{

struct ViewArea;

class PlaneDisplay : public DualDisplayBase
{
public:
    PlaneDisplay();

    void Draw(const Sensor& sensor, VSSprite *, VSSprite *);

    void OnDockEnd();
    void OnJumpEnd();

private:
    struct Impl;
    std::auto_ptr< Impl > impl;
    
protected: 
    typedef std::vector<float> AngleSequence;

    void DrawGround(const Sensor&, const ViewArea&);
    void DrawNear(const Sensor&, const Sensor::TrackCollection&);
    void DrawDistant(const Sensor&, const Sensor::TrackCollection&);
    void DrawTrack(const Sensor&, const ViewArea&, const Track&, float);
    void DrawTarget(Track::Type::Value, const Vector&, const Vector&, float, const GFXColor&);
    void DrawTargetMarker(const Vector&, const Vector&, const Vector&, float, const GFXColor&, bool);

    void Animate();
    void PrepareAnimation(const Vector&, const Vector&, const AngleSequence&, const AngleSequence&, const AngleSequence&);

    void CalculateRotation();
    Vector Projection(const ViewArea&, const Vector&);

protected:
    Vector finalCameraAngle;
    Vector currentCameraAngle;
    Vector xrotation;
    Vector yrotation;
    Vector zrotation;
    std::vector<Vector> groundPlane;
    float radarTime;

    // Primitive animation system
    struct AnimationItem
    {
        float duration;
        Vector position;
    };
    typedef std::queue<AnimationItem> AnimationCollection;
    AnimationCollection animation;
    float lastAnimationTime;
    AngleSequence nothingSequence;
    AngleSequence bounceSequence;
    AngleSequence cosineSequence;
};

} // namespace Radar

#endif
