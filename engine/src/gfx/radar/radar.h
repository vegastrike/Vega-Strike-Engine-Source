// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGASTRIKE_GFX_RADAR_RADAR_H
#define VEGASTRIKE_GFX_RADAR_RADAR_H

#include <string>
#include <vector>
#include <memory>

class Unit;
class VSSprite;

namespace Radar
{

class Sensor;

// Draws radar on display
class Display
{
public:
    virtual ~Display() {}

    virtual void Draw(const Sensor& sensor, VSSprite*, VSSprite *) = 0;

    virtual void OnDockEnd() {}
    virtual void OnJumpBegin() {}
    virtual void OnJumpEnd() {}
    virtual void OnPauseBegin() {}
    virtual void OnPauseEnd() {}
};

struct Type
{
    enum Value
    {
        NullDisplay,
        SphereDisplay,
        BubbleDisplay,
        PlaneDisplay
    };
};

std::auto_ptr<Display> Factory(Type::Value);

} // namespace Radar

#endif
