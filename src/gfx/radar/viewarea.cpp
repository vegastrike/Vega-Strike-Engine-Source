// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include <cmath>
#include "gfxlib_struct.h"
#include "viewarea.h"

namespace Radar
{

ViewArea::ViewArea()
    : sprite(0)
{
}

void ViewArea::SetSprite(VSSprite *sprite)
{
    this->sprite = sprite;
    sprite->GetPosition(position.x, position.y);
    position.z = 0;
    sprite->GetSize(size.x, size.y);
    size.x = std::fabs(size.x);
    size.y = std::fabs(size.y);
    size.z = 0;
}

Vector ViewArea::Scale(const Vector& v) const
{
    return Vector(position.x + size.x / 2 * v.x,
                  position.y + size.y / 2 * v.y,
                  position.z + size.y / 2 * v.z);
}

GFXColor ViewArea::GetColor() const
{
    return GFXColor(0.0, 0.5, 0.0, 1.0);
}

} // namespace Radar
