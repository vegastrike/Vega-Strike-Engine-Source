/*
 * cockpit_gfx.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */
// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
#ifndef VEGA_STRIKE_ENGINE_GFX_COCKPIT_GFX_H
#define VEGA_STRIKE_ENGINE_GFX_COCKPIT_GFX_H

#include "gfx/vec.h"
#include "gfx/radar/sensor.h"
#include "gfx/radar/radar.h"

#include <vector>
#include <memory>

class Camera;
class GameCockpit;
class Gauge;
struct GFXColor;
class NavigationSystem;
class TextPlane;
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

//Draw the arrow pointing to the target.
void DrawArrowToTarget(const Radar::Sensor&, Unit*,
                       float  projection_limit_x, float projection_limit_y,
                       float inv_screen_aspect_ratio);
void DrawArrowToTarget(const Radar::Sensor&, Vector LocalCoordinates,
                       float  projection_limit_x, float projection_limit_y,
                       float inv_screen_aspect_ratio);

void DrawCommunicatingBoxes(std::vector< VDU* >vdu);

///Draws unit gauges
void DrawGauges( GameCockpit *cockpit, Unit *un, Gauge *gauges[],
                 float gauge_time[], float cockpit_time, TextPlane *text,
                 GFXColor textcol);

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

#endif //VEGA_STRIKE_ENGINE_GFX_COCKPIT_GFX_H
