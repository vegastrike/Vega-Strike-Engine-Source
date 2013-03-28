// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGASTRIKE_GFX_RADAR_VIEWAREA_H
#define VEGASTRIKE_GFX_RADAR_VIEWAREA_H

#include "gfx/vec.h"
#include "gfx/sprite.h"

struct GFXColor;  // Edit from class to struct as defined in gfxlib_struct.

namespace Radar
{

struct ViewArea
{
    ViewArea();

    void SetSprite(VSSprite *);

    Vector Scale(const Vector&) const;

    GFXColor GetColor() const;

    bool IsActive() const;

    VSSprite *sprite;
    Vector position;
    Vector size;
};

} // namespace Radar

#endif
