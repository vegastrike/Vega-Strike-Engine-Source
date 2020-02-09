// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGASTRIKE_GFX_RADAR_SPHERE_DISPLAY_H
#define VEGASTRIKE_GFX_RADAR_SPHERE_DISPLAY_H

#include "sensor.h"
#include "radar.h"
#include "dual_display.h"

#include <memory>

struct GFXColor;

namespace Radar
{

struct ViewArea;

class SphereDisplay : public DualDisplayBase
{
public:
    SphereDisplay();
    virtual ~SphereDisplay();

    void Draw(const Sensor& sensor, VSSprite *, VSSprite *);

private:
    struct Impl;
    std::auto_ptr< Impl > impl;
    
protected:
    void DrawBackground(const Sensor&, const ViewArea&);
    void DrawTrack(const Sensor&, const ViewArea&, const Track&, bool negate_z=false);
    void DrawTargetMarker(const Vector&, const GFXColor&, float);

protected:
    const float innerSphere;
    float radarTime;
};

} // namespace Radar

#endif
