// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGASTRIKE_GFX_RADAR_BUBBLE_DISPLAY_H
#define VEGASTRIKE_GFX_RADAR_BUBBLE_DISPLAY_H

#include <queue>
#include "sensor.h"
#include "radar.h"
#include "dual_display.h"

namespace Radar
{

class ViewArea;

class BubbleDisplay : public DualDisplayBase
{
public:
    BubbleDisplay();

    void Draw(const Sensor&, VSSprite *, VSSprite *);

    void OnDockEnd();
    void OnJumpBegin();
    void OnJumpEnd();

protected:
    typedef std::vector<float> ZoomSequence;

    void DrawBackground(const ViewArea&, float);
    void DrawTrack(const Sensor&, const ViewArea&, const Track&);
    void DrawTargetMarker(const Vector&, float);

    void Animate();
    void PrepareAnimation(const ZoomSequence&);

protected:
    const float innerSphere;
    const float outerSphere;
    float sphereZoom;
    float radarTime;
    float currentTargetMarkerSize;

    struct AnimationItem
    {
        float sphereZoom;
        float duration;
    };
    typedef std::queue<AnimationItem> AnimationCollection;
    AnimationCollection animation;
    float lastAnimationTime;
    ZoomSequence explodeSequence;
    ZoomSequence implodeSequence;
};

} // namespace Radar

#endif
