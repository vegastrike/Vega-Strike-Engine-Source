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
#ifndef _GENERICUNIVERSE_H
#define _GENERICUNIVERSE_H


#define NUM_LIGHT	8
#define NUM_FORCES	64

#include "star_system_generic.h"
#include "faction_generic.h"
class Cockpit;
class Camera;
class Unit;

/**
 * Class Universe Deals with universal constants. It is a global, 
 * accessed from anywhere as _Universe-> Universe may be queried for 
 * Relationships, the current star system rendering is taking place in
 * etc.  It acts as a wrapper to the active Star System.
 * Additionally it handles beginning and ending the main loop.
 * And starting and ending graphics. (incl the task of wiping temp lights)
 * Deprecated: loaded dynamic gldrv module
 */
namespace GalaxyXML {
  class Galaxy;

}
class Universe {
protected:
  class GalaxyXML::Galaxy * galaxy;
  ///The users cockpit
  unsigned int current_cockpit;
  std::vector <Cockpit *> cockpit;
  ///a generic camera facing the HUD
  //Camera hud_camera;
  ///init proc
  //void StartGL();
  ///currently only 1 star system is stored
  std::vector <StarSystem *> active_star_system;

  // the system currently used by the scripting
  StarSystem *script_system;
  bool is_server;

private:
  ///Many C++ implementations count classes within as friends. (not all)
  friend class Faction;
  //friend void bootstrap_main_loop();//so it can get all cockpits
  ///A list of all factions 
  
 public:
  bool	isServer() { return is_server;}
  Cockpit * isPlayerStarship (const Unit* fighter);
  int whichPlayerStarship (const Unit* fighter);
  Cockpit *AccessCockpit() {return cockpit[current_cockpit];}
  Cockpit *AccessCockpit (int i) {return cockpit[i];}
  unsigned int CurrentCockpit(){return current_cockpit;}
  ///Wrapper function for Star System
  virtual Camera *AccessCamera(int num) { return NULL;}
  virtual Camera *AccessCamera() {return NULL;}
  ///Returns the current hud cam
  virtual Camera *AccessHudCamera() { return NULL; }
  ///Wrapper function for star system
  virtual void SetViewport() {}
  void SetActiveCockpit (int whichcockpit);
  void SetActiveCockpit (Cockpit * which);
  virtual void WriteSaveGame(bool auto_save) {}
  virtual void SetupCockpits (std::vector <std::string> players);
  virtual void activateLightMap() {}
  virtual void SelectCamera(int cam) {}
  //virtual unsigned int CurrentCockpit(){return 0;}
  Cockpit * createCockpit( std::string player);

  std::vector <std::string> getAdjacentStarSystems(const std::string &ss);
  std::string getGalaxyProperty (const std::string &sys, const std::string & prop);
  std::string getGalaxyPropertyDefault (const std::string & sys, const std::string & prop, const std::string def="");
  bool StillExists(StarSystem * ss);
  void setActiveStarSystem(StarSystem * ss) {active_star_system.back()=ss;}
  void pushActiveStarSystem(StarSystem * ss) {active_star_system.push_back (ss);}
  void popActiveStarSystem() {active_star_system.pop_back();}
  //void SetActiveCockpit (int whichcockpit);
  //void SetActiveCockpit (Cockpit * which);
  StarSystem * getActiveStarSystem (unsigned int size) {return active_star_system[size];}
  unsigned int getNumActiveStarSystem() {return active_star_system.size();}
  void LoadStarSystem(StarSystem * ss);
  void UnloadStarSystem(StarSystem * ss);
  void Generate1( const char * file, const char * jumpback);
  void Generate2( StarSystem * ss);
  virtual StarSystem * GenerateStarSystem (const char * file, const char * jumpback, Vector origin);
  ///Loads and parses factions
  void LoadFactionXML (const char * factfile) {
    Faction::LoadXML (factfile);
  }
  //void SetupCockpits (std::vector <std::string> players);
  //void WriteSaveGame(bool auto_save);
  //void activateLightMap();
  ///inits graphics with args
  Universe();
  Universe(int argc, char **argv, const char * galaxy, bool server=false);
  void Init( const char * gal);
  virtual ~Universe();
  ///Loads Defaults in Graphics Drivers
  //void StartGFX();
  ///Should load the Universe data file. Now just inits system with test.xml
  virtual class StarSystem * Init(string systemfile, const Vector & centroid=Vector (0,0,0), const string planetname=string());
  ///Begins a scene
  virtual void StartDraw() {}
  // Update starsystems (for server side)
  void Update();
  ///Runs the main loop
  virtual void Loop(void f()) {}
  ///returns active star system
  StarSystem* activeStarSystem() {return active_star_system.back();}
  ///Wrapper function for Star System
  /*
  void SelectCamera(int cam) {
    AccessCockpit()->SelectCamera(cam);
  }
  */
  ///Accessor to cockpit
  //unsigned int CurrentCockpit(){return current_cockpit;}
  //Cockpit *AccessCockpit() {return cockpit[current_cockpit];}
  int numPlayers () {return cockpit.size();}
  //Cockpit *AccessCockpit (int i) {return cockpit[i];}
  ///Wrapper function for Star System
  /*
  Camera *AccessCamera(int num) {
    return AccessCockpit()->AccessCamera(num);
  }
  */
  ///Wrapper function for star system
  /*
  Camera *AccessCamera() {
    return AccessCockpit()->AccessCamera();
  }
  */
  ///Returns the current hud cam
  //Camera *AccessHudCamera() { return &hud_camera; }
  ///Wrapper function for star system
  /*
  void SetViewport() {
    AccessCockpit()->SetViewport();
  }
  */

  StarSystem *getStarSystem(string name);
  StarSystem *scriptStarSystem() {
    if(script_system!=NULL){ return script_system; }
    else { return activeStarSystem(); };
  }
  bool setScriptSystem(string name){
    if(name=="-active-"){
      script_system=NULL;
      return true;
    }
    StarSystem *ss=getStarSystem(name);
    if(script_system!=NULL){
      script_system=ss;
      return true;
    }
    return false;
  }
 public:
  std::vector <StarSystem *> star_system;
  int	StarSystemIndex( StarSystem *ss);
};

#endif
