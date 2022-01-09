/**
* sphere_display.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

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
    std::unique_ptr< Impl > impl;
    
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
