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


#define NUM_LIGHT	8
#define NUM_FORCES	64

#include "gfx/camera.h"
#include "star_system.h"

class Cockpit;

/**
 * Class Universe Deals with universal constants. It is a global, 
 * accessed from anywhere as _Universe. Universe may be queried for 
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
  Cockpit * cockpit;
  ///a generic camera facing the HUD
  Camera hud_camera;
  ///init proc
  void StartGL();
  ///currently only 1 star system is stored
  std::vector <StarSystem *> active_star_system;
  std::vector <StarSystem *> star_system;
  // the system currently used by the scripting
  StarSystem *script_system;

private:
  /**
   * Class Faction holds the relationship between one faction and another
   * All factions are indexed in a vector. Each has a name and a logo.
   * In addition each has a list of other factions and the relationship to them
   */
  class Faction {

  public:
    /**
     * Faction_stuff holds the index and relationship of each and every
     * Other faction.  After it has been properly processed when calling
     * LoadXML, it will hold an ordered list containing all factions.
     * myfaction.faction[theirfaction].relationship will contain my 
     * attitude to theirfaction
     */
    struct faction_stuff {
      ///for internal purposes only.
      union faction_name {
	int index;
	char * name;
      } stats;
      ///A value between 0 and 1 indicating my attitude towards index
      float relationship;
      class FSM * conversation;//a conversation any two factions can have
      faction_stuff () {conversation=NULL;relationship=0;}
    };		
  public:
    /**
     * holds the relationships to all _other_ factions loaded 
     * hold misguided info for self FIXME
     */
    vector <faction_stuff> faction;
    ///Logos used by the ships of that faction
    Texture * logo;
    ///char * of the name
    char * factionname; 
    std::vector <std::vector <class Animation *> > comm_faces;
    std::vector <unsigned char> comm_face_sex;
    Unit * contraband;
    ///Figures out the relationships of each faction with each other
    static void ParseAllAllies(Universe * thisuni);
    void ParseAllies(Universe * thisuni);
    
    static void LoadXML(const char * factionfile, Universe * thisuni);  
    static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
    static void endElement(void *userData, const XML_Char *name);
    Faction(); //constructor
    ~Faction(); //destructor
  };
  ///Many C++ implementations count classes within as friends. (not all)
  friend class Faction;
  ///A list of all factions 
  vector <Faction *> factions; //the factions
  
 public:
  bool StillExists(StarSystem * ss);
  void setActiveStarSystem(StarSystem * ss) {active_star_system.back()=ss;}
  void pushActiveStarSystem(StarSystem * ss) {active_star_system.push_back (ss);}
  void popActiveStarSystem() {active_star_system.pop_back();}
  StarSystem * getActiveStarSystem (unsigned int size) {return active_star_system[size];}
  unsigned int getNumActiveStarSystem() {return active_star_system.size();}
  void LoadStarSystem(StarSystem * ss);
  void UnloadStarSystem(StarSystem * ss);
  StarSystem * GenerateStarSystem (const char * file, const char * jumpback, Vector origin);
  ///Loads and parses factions
  void LoadFactionXML (const char * factfile) {
    Faction::LoadXML (factfile,this);
  }
  ///returns the index of the faction with that name
  int GetFaction (const char *factionname);
  int GetNumAnimation(int faction);
  void SerializeFaction (FILE * file);
  void LoadSerializedFaction (FILE * file);
  std::vector <class Animation *>* GetRandAnimation(int faction, unsigned char &sex);
  std::vector <class Animation *> * GetAnimation(int faction, int n, unsigned char &sex);
  const char * GetFaction (int faction);
  class Unit * GetContraband(int faction);
  void LoadContrabandLists();
  /**
   * Returns the relationship between myfaction and theirfaction
   * 1 is happy. 0 is neutral (btw 1 and 0 will not attack)
   * -1 is mad. <0 will attack
   */
  float GetRelation (const int myfaction, const int theirfaction);
  unsigned int GetNumFactions () {
    return factions.size();
  }
  void AdjustRelation (const int myfaction, const int theirfaction, const float factor, const float rank);
  //Returns a conversation that a myfaction might have with a theirfaction
  FSM* GetConversation (const int myfaction, const int theirfaction);
  void activateLightMap();
  ///Returns force logo
  Texture * getForceLogo (int faction);
  ///Returns force logo FIXME should return squad logo!
  Texture * getSquadLogo (int faction);
  ///inits graphics with args
  Universe(int argc, char **argv, const char * galaxy);
  ~Universe();
  ///Loads Defaults in Graphics Drivers
  void StartGFX();
  ///Should load the Universe data file. Now just inits system with test.xml
  void Init(string systemfile, const Vector & centroid=Vector (0,0,0), const string planetname=string());
  ///Begins a scene
  void StartDraw();
  ///Runs the main loop
  void Loop(void main_loop());
  ///returns active star system
  StarSystem* activeStarSystem() {return active_star_system.back();}
  ///Wrapper function for Star System
  void SelectCamera(int cam) {
    activeStarSystem()->SelectCamera(cam);
    
  }
  ///Accessor to cockpit
  Cockpit *AccessCockpit() {return cockpit;}
  ///Wrapper function for Star System
  Camera *AccessCamera(int num) {
    return activeStarSystem()->AccessCamera(num);
  }
  ///Wrapper function for star system
  Camera *AccessCamera() {
    return activeStarSystem()->AccessCamera();
  }
  ///Returns the current hud cam
  Camera *AccessHudCamera() { return &hud_camera; }
  ///Wrapper function for star system
  void SetViewport() {
    activeStarSystem()->SetViewport();
  }

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

};

#endif

