// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include <cassert>
#include <stdexcept>
#include "sensor.h"
#include "radar.h"
#include "null_display.h"
#include "sphere_display.h"
#include "bubble_display.h"
#include "plane_display.h"

namespace Radar
{

std::auto_ptr<Display> Factory(Type::Value type)
{
    switch (type)
    {
    case Type::NullDisplay:
        return std::auto_ptr<Display>(new NullDisplay);

    case Type::SphereDisplay:
        return std::auto_ptr<Display>(new SphereDisplay);

    case Type::BubbleDisplay:
        return std::auto_ptr<Display>(new BubbleDisplay);

    case Type::PlaneDisplay:
        return std::auto_ptr<Display>(new PlaneDisplay);

    default:
        assert(false);
        throw std::invalid_argument("Unknown radar type");
    }
}

} // namespace Radar
