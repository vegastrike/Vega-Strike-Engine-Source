#ifndef COCKPITGFX_H
#define COCKPITGFX_H

#include "gfx/vec.h"
#include "gfx/radar/sensor.h"

// TODO: remove
inline void DrawDockingBoxes( Unit *un, const Unit *target, const Vector &CamP, const Vector &CamQ, const Vector &CamR );
inline void DrawOneTargetBox( const QVector &Loc,
                              float rSize,
                              const Vector &CamP,
                              const Vector &CamQ,
                              const Vector &CamR,
                              float lock_percent,
                              bool ComputerLockon,
                              bool Diamond = false);

///draws the navigation symbol around targetted location
void DrawNavigationSymbol( const Vector &loc, const Vector &p, const Vector &q, float size );
///draws the target box around targetted unit
void DrawTargetBox(const Radar::Sensor&, bool draw_line_to_target, bool draw_line_to_targets_target,
                   bool always_itts, float locking_percent, bool draw_line_to_itts, bool steady_itts);

float computeLockingSymbol( Unit *par );
#endif // COCKPITGFX_H
