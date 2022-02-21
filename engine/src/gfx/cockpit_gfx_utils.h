#ifndef COCKPITGFXUTILS_H
#define COCKPITGFXUTILS_H

#include "vec.h"
#include "gfxlib_struct.h"

enum class ShapeType {
    Box,
    Cross,
    Diamond,
    Default
};

VertexBuilder<> GetCross(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const float& size);
VertexBuilder<> GetDiamond(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const float& r_size);
VertexBuilder<> GetRectangle(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const float& r_size);
VertexBuilder<> GetOpenRectangle(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const float& r_size);
VertexBuilder<> GetLockingIcon(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const float& r_size,
                            const float& lock_percent);
VertexBuilder<> GetAnimatedLockingIcon(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const Vector& cam_r,
                            const float& r_size,
                            const float& lock_percent);

void SetThickness(ShapeType type);

#endif // COCKPITGFXUTILS_H
