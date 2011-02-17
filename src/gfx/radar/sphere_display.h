// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGASTRIKE_GFX_RADAR_SPHERE_DISPLAY_H
#define VEGASTRIKE_GFX_RADAR_SPHERE_DISPLAY_H

#include "sensor.h"
#include "radar.h"
#include "dual_display.h"

namespace Radar
{

struct ViewArea;

class SphereDisplay : public DualDisplayBase
{
public:
    SphereDisplay();

    void Draw(const Sensor& sensor, VSSprite *, VSSprite *);

protected:
    void DrawBackground(const Sensor&, const ViewArea&);
    void DrawTrack(const Sensor&, const ViewArea&, const Track&, bool negate_z=false);
    void DrawTargetMarker(const Vector&, float);

protected:
    const float innerSphere;
    float radarTime;
};

} // namespace Radar

#endif
