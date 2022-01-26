#ifndef COCKPITGFX_H
#define COCKPITGFX_H

#include "gfx/vec.h"
#include "gfx/radar/sensor.h"
#include "gfx/radar/radar.h"

#include <vector>
#include <memory>

class Camera;
class Gauge;
class NavigationSystem;
class VSSprite;
class VDU;


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

void DrawCommunicatingBoxes(std::vector< VDU* >vdu);

///draws the navigation symbol around targetted location
void DrawNavigationSymbol( const Vector &loc, const Vector &p, const Vector &q, float size );

bool DrawNavSystem(NavigationSystem* nav_system, Camera* camera, float cockpit_offset);

///Draws all the tracks on the radar.
void DrawRadar(const Radar::Sensor&, float  cockpit_time, float radar_time,
               VSSprite *radarSprites[2],
               Radar::Display* radarDisplay);

void DrawTacticalTargetBox(const Radar::Sensor&);

///draws the target box around all units
void DrawTargetBoxes(const Radar::Sensor&);

///draws the target box around targetted unit
void DrawTargetBox(const Radar::Sensor&, bool draw_line_to_target, bool draw_line_to_targets_target,
                   bool always_itts, float locking_percent, bool draw_line_to_itts, bool steady_itts);

///draws a target cross around all units targeted by your turrets // ** jay
void DrawTurretTargetBoxes(const Radar::Sensor&);

///Draws target gauges
void DrawTargetGauges( Unit *target, Gauge *gauges[] );


float computeLockingSymbol( Unit *par );
#endif // COCKPITGFX_H
