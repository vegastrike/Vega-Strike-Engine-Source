// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGASTRIKE_GFX_RADAR_NULL_DISPLAY_H
#define VEGASTRIKE_GFX_RADAR_NULL_DISPLAY_H

#include "radar.h"

namespace Radar
{

class NullDisplay : public Display
{
public:
    // Shows nothing
    void Draw(const Sensor&, VSSprite *, VSSprite *) {}
};

} // namespace Radar

#endif
