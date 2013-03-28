// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGASTRIKE_GFX_RADAR_BUBBLE_DISPLAY_H
#define VEGASTRIKE_GFX_RADAR_BUBBLE_DISPLAY_H

#include <queue>
#include "sensor.h"
#include "radar.h"
#include "dual_display.h"

#include <memory>

struct GFXColor;

namespace Radar
{

struct ViewArea;

class BubbleDisplay : public DualDisplayBase
{
public:
    BubbleDisplay();
    virtual ~BubbleDisplay();

    void Draw(const Sensor&, VSSprite *, VSSprite *);

    void OnDockEnd();
    void OnJumpBegin();
    void OnJumpEnd();

private:
    struct Impl;
    std::auto_ptr<Impl> impl;

protected:
    typedef std::vector<float> ZoomSequence;

    void DrawBackground(const ViewArea&, float);
    void DrawTrack(const Sensor&, const ViewArea&, const Track&);
    void DrawTargetMarker(const Vector&, const GFXColor&, float);

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
