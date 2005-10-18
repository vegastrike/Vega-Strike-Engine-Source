/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _WrapGFX_H
#define _WrapGFX_H

#include "universe_generic.h"
#include "gfx/camera.h"
#include "star_system.h"
#include "gfx/cockpit.h"
#include "faction_generic.h"

/**
 * Class GameUniverse Deals with universal constants. It is a global, 
 * accessed from anywhere as _Universe-> Universe may be queried for 
 * Relationships, the current star system rendering is taking place in
 * etc.  It acts as a wrapper to the active Star System.
 * Additionally it handles beginning and ending the main loop.
 * And starting and ending graphics. (incl the task of wiping temp lights)
 * Deprecated: loaded dynamic gldrv module
 */
class GameUniverse: public Universe {
protected:
  ///The users cockpit
  //unsigned int current_cockpit;
  //std::vector <Cockpit *> cockpit;
  ///a generic camera facing the HUD
  Camera hud_camera;
  ///init proc
  void StartGL();

private:
  friend void bootstrap_main_loop();//so it can get all cockpits
  ///A list of all factions 
  
 public:
  StarSystem * GenerateStarSystem (const char * file, const char * jumpback, Vector origin);
  void WriteSaveGame(bool auto_save);
  void SetupCockpits (std::vector <std::string> players);
  void activateLightMap(int stage=1);
  ///inits graphics with args
  GameUniverse(int argc, char **argv, const char * galaxy);
  GameUniverse();
  void Init(int argc, char** argv, const char * galaxy);
  ~GameUniverse();
  ///Loads Defaults in Graphics Drivers
  void StartGFX();
  ///Should load the Universe data file. Now just inits system with test.xml
  class StarSystem * Init(string systemfile, const Vector & centroid=Vector (0,0,0), const string planetname=string());
  ///Begins a scene
  void StartDraw();
  ///Runs the main loop
  void Loop(void main_loop());
  ///Wrapper function for Star System
  void SelectCamera(int cam) {
    AccessCockpit()->SelectCamera(cam);
  }
  ///Accessor to cockpit
  //unsigned int CurrentCockpit(){return current_cockpit;}
  //Cockpit *AccessCockpit() {return cockpit[current_cockpit];}
  //Cockpit *AccessCockpit (int i) {return cockpit[i];}
  ///Wrapper function for Star System
  Camera *AccessCamera(int num) {
    return AccessCockpit()->AccessCamera(num);
  }
  ///Wrapper function for star system
  Camera *AccessCamera() {
    return AccessCockpit()->AccessCamera();
  }
  ///Returns the current hud cam
  Camera *AccessHudCamera() { return &hud_camera; }
  ///Wrapper function for star system
  void SetViewport() {
    AccessCockpit()->SetViewport();
  }
};

#endif

