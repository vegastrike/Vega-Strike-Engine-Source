#include <assert.h>
#include "star_system_generic.h"
#include "gfx/vec.h"
#include "cmd/planet_generic.h"
#include "cmd/unit_generic.h"
#include "cmd/unit_collide.h"
#include "cmd/collection.h"
#include "gfx/cockpit_generic.h"
#include "audiolib.h"
//#include "cmd/click_list.h"
//#include "cmd/ai/input_dfa.h"
#include "lin_time.h"
#include "cmd/beam.h"
//#include "gfx/sphere.h"
//#include "cmd/unit_collide.h"
//#include "gfx/star.h"
#include "cmd/bolt.h"
#include <expat.h>
//#include "cmd/music.h"
#include "configxml.h"
#include "vs_globals.h"
//#include "cmd/cont_terrain.h"
#include "vegastrike.h"
#include "universe_generic.h"
//#include "cmd/atmosphere.h"
#include "hashtable.h"
#include "cmd/nebula_generic.h"
#include "galaxy_gen.h"
#include "cmd/script/mission.h"
#include "in_kb.h"
#include "cmd/script/flightgroup.h"
#include "load_mission.h"
#include "lin_time.h"
//#include "gfx/particle.h"
//extern Music *muzak;
//extern Vector mouseline;
#include "cmd/unit_collide.h"
#include "savegame.h"
#include "networking/netclient.h"
#ifndef USE_BOOST_128
#include <boost/python.hpp>
#else
#include <boost/python/detail/extension_class.hpp>
#endif
vector<Vector> perplines;
//static SphereMesh *foo;
//static Unit *earth;

//extern void reset_time_compression (int,KBSTATE);

//Atmosphere *theAtmosphere;
extern std::vector <unorigdest *>pendingjump;

void TentativeJumpTo (StarSystem * ss, Unit * un, Unit * jumppoint, const std::string &system) {
  for (unsigned int i=0;i<pendingjump.size();i++) {
    if (pendingjump[i]->un.GetUnit()==un) {
      return;
    }
  }
  ss->JumpTo (un,jumppoint,system);
}
float ScaleJumpRadius (float radius) {
	    static float jump_radius_scale=parse_float (vs_config->getVariable("physics","jump_radius_scale","2"));
	    static float game_speed = parse_float (vs_config->getVariable ("physics","game_speed","1"));
	    radius *= jump_radius_scale*game_speed;//need this because sys scale doesn't affect j-point size
	    return radius;
}

StarSystem::StarSystem() {
  stars = NULL;
  bolts = NULL;
  collidetable = NULL;
  no_collision_time=0;//(int)(1+2.000/SIMULATION_ATOM);
  ///adds to jumping table;
  name = NULL;
  //_Universe->pushActiveStarSystem (this);
  current_stage=PHY_AI;
  //systemInputDFA = new InputDFA (this);
  /*
  LoadXML(filename,centr,timeofyear);
  if (!name)
    name =strdup (filename);
  */
  //AddStarsystemToUniverse(filename);
  time = 0;
  //_Universe->popActiveStarSystem ();
  zone = 0;
}
StarSystem::StarSystem(const char * filename, const Vector & centr,const float timeofyear) {
  no_collision_time=0;//(int)(1+2.000/SIMULATION_ATOM);
  ///adds to jumping table;
  name = NULL;
  zone = 0;
  _Universe->pushActiveStarSystem (this);
  //GFXCreateLightContext (lightcontext);
  //bolts = new bolt_draw;
  collidetable = new CollideTable(this);
  //  cout << "origin: " << centr.i << " " << centr.j << " " << centr.k << " " << planetname << endl;
  current_stage=PHY_AI;
  //systemInputDFA = new InputDFA (this);
  this->filename = filename;
  LoadXML(filename,centr,timeofyear);
  if (!name)
    name =strdup (filename);
  AddStarsystemToUniverse(filename);
//  primaries[0]->SetPosition(0,0,0);
  //iter = primaries->createIterator();
  //iter->advance();
  //earth=iter->current();
  //delete iter;

  // Calculate movement arcs; set behavior of primaries to follow these arcs
  //Iterator *primary_iterator = primaries->createIterator(); 
  //primaries->SetPosition(0,0,5);
  //foo = new SphereMesh(1,5,5,"moon.bmp");
  //cam[1].SetProjectionType(Camera::PARALLEL);
  //cam[1].SetZoom(1);
  //cam[1].SetPosition(Vector(0,0,0));
  //cam[1].LookAt(Vector(0,0,0), Vector(0,0,1));
  //cam[1].SetPosition(Vector(0,5,-2.5));
  //cam[1].SetSubwindow(0,0,1,1);
  //cam[2].SetProjectionType(Camera::PARALLEL);
  //cam[2].SetZoom(10.0);
  //cam[2].SetPosition(Vector(5,0,0));
  //cam[2].LookAt(Vector(0,0,0), Vector(0,-1,0));
  //cam[2].SetPosition(Vector(5,0,-2.5));
  //cam[2].SetSubwindow(0.10,0,0.10,0.10);
  //UpdateTime();
  time = 0;

/*
  Atmosphere::Parameters params;
  params.radius = 40000;
  params.low_color[0] = GFXColor(0,0.5,0.0);
  params.low_color[1] = GFXColor(0,1.0,0.0);
  params.low_ambient_color[0] = GFXColor(0.0/255.0,0.0/255.0,0.0/255.0);
  params.low_ambient_color[1] = GFXColor(0.0/255.0,0.0/255.0,0.0/255.0);
  params.high_color[0] = GFXColor(0.5,0.0,0.0);
  params.high_color[1] = GFXColor(1.0,0.0,0.0);
  params.high_ambient_color[0] = GFXColor(0,0,0);
  params.high_ambient_color[1] = GFXColor(0,0,0);
  params.low_color[0] = GFXColor(241.0/255.0,123.0/255.0,67.0/255.0);
  params.low_color[1] = GFXColor(253.0/255.0,65.0/255.0,55.0/255.0);
  params.low_ambient_color[0] = GFXColor(0.0/255.0,0.0/255.0,0.0/255.0);
  params.low_ambient_color[1] = GFXColor(0.0/255.0,0.0/255.0,0.0/255.0);
  params.high_color[0] = GFXColor(60.0/255.0,102.0/255.0,249.0/255.0);
  params.high_color[1] = GFXColor(57.0/255.0,188.0/255.0,251.0/255.0);
  params.high_ambient_color[0] = GFXColor(0,0,0);
  params.high_ambient_color[1] = GFXColor(0,0,0);
  */
  //params.scattering = 5;
  //theAtmosphere = new Atmosphere(params);
  _Universe->popActiveStarSystem ();
}
extern void ClientServerSetLightContext (int lightcontext);
StarSystem::~StarSystem() {
  _Universe->activeStarSystem()->SwapOut();
  _Universe->pushActiveStarSystem(this);
  //  _Universe->activeStarSystem()->SwapIn();  
  ClientServerSetLightContext (lightcontext);
  //delete stars;
  delete [] name;
  //delete systemInputDFA;
  /* //FIXME  after doign so much debugging I think you shouldn't delete this
  for (int i=0;i<numprimaries;i++) {
	delete primaries[i];
	}
	delete [] primaries;

  */

  UnitCollection::UnitIterator iter = drawList.createIterator();
  Unit *unit;
  //  VSFileSystem::vs_fprintf (stderr,"|t%f i%lf|",GetElapsedTime(),interpolation_blend_factor);
  while((unit = iter.current())!=NULL) {
    unit->Kill(false);
    iter.advance();
  }
  //if the next line goes ANYWHERE else Vega Strike will CRASH!!!!!
  if (collidetable) delete collidetable;//DO NOT MOVE THIS LINE! IT MUST STAY
  //_Universe->activeStarSystem()->SwapOut();  
  _Universe->popActiveStarSystem();
  _Universe->activeStarSystem()->SwapIn();  
  RemoveStarsystemFromUniverse();
  
}


/********* FROM STAR SYSTEM XML *********/
void setStaticFlightgroup (vector<Flightgroup *> &fg, const std::string &nam,int faction) {
  while (faction>=(int)fg.size()) {
    fg.push_back(new Flightgroup());
    fg.back()->nr_ships=0;
  }
  if (fg[faction]->nr_ships==0) {
    fg[faction]->flightgroup_nr=faction;
    fg[faction]->pos.i=fg[faction]->pos.j=fg[faction]->pos.k=0;
    //    fg[faction]->rot[0]=fg[faction]->rot[1]=fg[faction]->rot[2]=0;
    fg[faction]->nr_ships=0;
    fg[faction]->ainame="default";
     fg[faction]->faction=FactionUtil::GetFaction(faction);
    fg[faction]->type="Base";
    fg[faction]->nr_waves_left=0;
    fg[faction]->nr_ships_left=0;
    fg[faction]->name=nam;
  }
  fg[faction]->nr_ships++;
  fg[faction]->nr_ships_left++;
}
Flightgroup *getStaticBaseFlightgroup (int faction) {
  static vector <Flightgroup *> fg;//warning mem leak...not big O(num factions)
  setStaticFlightgroup (fg,"Base",faction);
  return fg[faction];
}
Flightgroup *getStaticStarFlightgroup (int faction) {
  static vector <Flightgroup *> fg;//warning mem leak...not big O(num factions)
  setStaticFlightgroup (fg,"Base",faction);
  return fg[faction];
}

Flightgroup *getStaticNebulaFlightgroup (int faction) {
  static vector <Flightgroup *> fg;
  setStaticFlightgroup (fg,"Nebula",faction);
  return fg[faction];
}
Flightgroup *getStaticAsteroidFlightgroup (int faction) {
  static vector <Flightgroup *> fg;
  setStaticFlightgroup (fg,"Asteroid",faction);
  return fg[faction];
}
Flightgroup *getStaticUnknownFlightgroup (int faction) {
  static vector <Flightgroup *> fg;
  setStaticFlightgroup (fg,"Unknown",faction);
  return fg[faction];
}

void StarSystem::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  ((StarSystem*)userData)->beginElement(name, AttributeList(atts));
}

void StarSystem::endElement(void *userData, const XML_Char *name) {
  ((StarSystem*)userData)->endElement(name);
}

extern string RemoveDotSystem (const char *input);
string StarSystem::getFileName() {
  return getStarSystemSector (filename)+string("/")+RemoveDotSystem(getStarSystemName (filename).c_str());
}
string StarSystem::getName () {
  return string(name);
}



void StarSystem::AddUnit(Unit *unit) {
  units.prepend(unit);
  drawList.prepend(unit);
}

bool StarSystem::RemoveUnit(Unit *un) {
  bool removed2=false;
  UnitCollection::UnitIterator iter = units.createIterator();
  Unit *unit;
  while((unit = iter.current())!=NULL) {
    if (unit==un) {
      iter.remove();
      removed2 =true;
      break;
    } else {
      iter.advance();
    }
  }
  bool removed =false;
  if (removed2) {
    UnitCollection::UnitIterator iter = drawList.createIterator();
    Unit *unit;
    while((unit = iter.current())!=NULL) {
      if (unit==un) {
	iter.remove();
	removed =true;
	break;
      }else {
	iter.advance();
      }
    }
  }
  return removed;
}

void StarSystem::ExecuteUnitAI () {
   try {
      un_iter iter = this->getUnitList().createIterator();
      Unit * unit=NULL;
      while((unit = iter.current())!=NULL) {
	  unit->ExecuteAI(); 
	  unit->ResetThreatLevel();
	  iter.advance();
      }
   }catch (const boost::python::error_already_set) {
      if (PyErr_Occurred()) {
         PyErr_Print();
         PyErr_Clear();
         fflush(stderr);         
         fflush(stdout);
      }
      throw;
   }

}
//sorry boyz...I'm just a tourist with a frag nav console--could you tell me where I am?
Unit * getTopLevelOwner( ) {//returns terrible memory--don't dereference...ever...not even aligned
  return (Unit *)0x31337;
}

void CarSimUpdate (Unit *un, float height) {
	un->SetVelocity(Vector(un->GetVelocity().i,0,un->GetVelocity().k));
	un->curr_physical_state.position= QVector(un->curr_physical_state.position.i
											  ,height,
											  un->curr_physical_state.position.k);
	
	
	

}


void StarSystem::UpdateUnitPhysics (bool firstframe) {
  un_iter iter = this->getUnitList().createIterator();
#ifdef CAR_SIM
  float unitheight=0;  
  if (_Universe->AccessCockpit(0)->savegame->getMissionData("unitheight").empty())
	  _Universe->AccessCockpit(0)->savegame->getMissionData("unitheight").push_back(0);
																						   unitheight=_Universe->AccessCockpit(0)->savegame->getMissionData("unitheight")[0];
#endif
  Unit * unit=NULL;
	while((unit = iter.current())!=NULL) {
#ifdef OLD_AUTOPILOT
	if (owner) 
	  if (owner->getRelation(unit)<0) {
	    static float neardist =XMLSupport::parse_float(vs_config->getVariable("physics","autodist","4000"));
	    Vector diff (owner->Position()-unit->Position());
	    if (diff.Dot(diff)<neardist*neardist) {
	      if (getTimeCompression!=.0000001) {//if not paused
		reset_time_compression(0,PRESS);
	      }
	    }
	  }
#endif
	  unit->UpdatePhysics(identity_transformation,identity_matrix,Vector (0,0,0),firstframe,&this->gravitationalUnits());
#ifdef CAR_SIM
	  Cockpit * cp = _Universe->isPlayerStarship(unit);
	  float tmp=0;
	  if (cp) {
		  if (cp->savegame->getMissionData("unitheight").empty())
			  cp->savegame->getMissionData("unitheight").push_back(0);
		  tmp= cp->savegame->getMissionData("unitheight")[0];
	  }
	  static string s = vs_config->getVariable ("physics","seeker","skart");
	  CarSimUpdate(unit,unit->name==s?unitheight:tmp);
#endif
	  iter.advance();
	}
}

extern void	UpdateTerrain();
extern void	TerrainCollide();
extern void UpdateAnimatedTexture();
extern void UpdateCameraSnds();
extern void NebulaUpdate( StarSystem * ss);
extern void TestMusic();

extern void reset_time_compression (int,KBSTATE);

extern float getTimeCompression();
void StarSystem::Update( float priority)
{
	Unit * unit;
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"begin Update");
  fflush (stderr);
#endif
  bool firstframe = true;
// No time compression here
  float normal_simulation_atom = SIMULATION_ATOM;
  time += GetElapsedTime();
  _Universe->pushActiveStarSystem(this);

  if(time/SIMULATION_ATOM>=(1./PHY_NUM))	  
  {
    while(time/SIMULATION_ATOM >= (1./PHY_NUM))
	{ // Chew up all SIMULATION_ATOMs that have elapsed since last update
      UnitCollection::UnitIterator iter;
      if (current_stage==PHY_AI)
	  {
		#ifdef UPDATEDEBUG
		  VSFileSystem::vs_fprintf (stderr,"AI");
		  fflush (stderr);
		#endif
		  ExecuteUnitAI();
		current_stage=TERRAIN_BOLT_COLLIDE;
      }
	  else if (current_stage==TERRAIN_BOLT_COLLIDE)
	  {
		#ifdef UPDATEDEBUG
		  VSFileSystem::vs_fprintf (stderr,"TerCol");
		  fflush (stderr);
		#endif
			TerrainCollide();
		#ifdef UPDATEDEBUG
		  VSFileSystem::vs_fprintf (stderr,"DelQ");
		  fflush (stderr);
		#endif
		  Unit::ProcessDeleteQueue();
		  current_stage=MISSION_SIMULATION;
      }
	  else if (current_stage==MISSION_SIMULATION)
	  {
		// What to do here on server side for missions ????????
		current_stage=PHY_COLLIDE;
      }
	  else if (current_stage==PHY_COLLIDE)
	  {
		#ifdef NO_COLLISION_TIME
			if (no_collision_time) {
			  no_collision_time--;//don't resolve physics until 2 seconds
			}else 
		#endif
			{
		#ifdef UPDATEDEBUG
		  VSFileSystem::vs_fprintf (stderr,"neb");
		  fflush (stderr);
		#endif
			  iter = drawList.createIterator();
			  while((unit = iter.current())!=NULL) {
				unit->SetNebula(NULL); 
				iter.advance();
			  }
			  iter = drawList.createIterator();
		#ifdef UPDATEDEBUG
		  VSFileSystem::vs_fprintf (stderr,"Coll");
		  fflush (stderr);
		#endif
			  while((unit = iter.current())!=NULL) {
				  unit->CollideAll();
				  iter.advance();
			  }
	        }
			UpdateMissiles();//do explosions
			current_stage=PHY_TERRAIN;
      }
	  else if (current_stage==PHY_TERRAIN)
	  {
		    // What to do on server side ????
			current_stage=PHY_RESOLV;
      }
	  else if (current_stage==PHY_RESOLV)
	  {
		#ifdef UPDATEDEBUG
		  VSFileSystem::vs_fprintf (stderr,"muzak");
		  fflush (stderr);
		#endif
		  UpdateUnitPhysics(firstframe);
		#ifdef UPDATEDEBUG
		  VSFileSystem::vs_fprintf (stderr,"boltphi");
		  fflush (stderr);
		#endif
		  bolts->UpdatePhysics();
		/*
		for (unsigned int i=0;i<active_missions.size();i++) {
		  active_missions[i]->BriefingUpdate();//waste of farkin time
		}
		*/
		  current_stage=PHY_AI;
		  firstframe = false;
      }
      time -= (1./PHY_NUM)*SIMULATION_ATOM;
    }
  }
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"endupd\n");
  fflush (stderr);
#endif
  UnitCollection::FreeUnusedNodes();
  collidetable->Update();
  SIMULATION_ATOM =  normal_simulation_atom;
  _Universe->popActiveStarSystem();
  //  VSFileSystem::vs_fprintf (stderr,"bf:%lf",interpolation_blend_factor);
}
void ExecuteDirector () {
	    unsigned int curcockpit= _Universe->CurrentCockpit();
	    for (unsigned int i=0;i<active_missions.size();i++) {
			if (active_missions[i]) {
				_Universe->SetActiveCockpit(active_missions[i]->player_num);
				StarSystem * ss=_Universe->AccessCockpit()->activeStarSystem;
				if (ss) _Universe->pushActiveStarSystem(ss);
				mission=active_missions[i];
				active_missions[i]->DirectorLoop();
				active_missions[i]->DirectorBenchmark();
				if (ss)_Universe->popActiveStarSystem();
			}
	    }
		_Universe->SetActiveCockpit(curcockpit);
		mission=active_missions[0];
		processDelayedMissions();
}
void StarSystem::Update(float priority , bool executeDirector) {

  Unit *unit;
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"begin Update");
  fflush (stderr);
#endif
  bool firstframe = true;

  ///this makes it so systems without players may be simulated less accurately
  for (int k=0;k<_Universe->numPlayers();k++) {
    if (_Universe->AccessCockpit(k)->activeStarSystem==this) {
      priority=1;
    }
  }
  float normal_simulation_atom = SIMULATION_ATOM;
  SIMULATION_ATOM/=(priority/getTimeCompression());
  ///just be sure to restore this at the end

  time += GetElapsedTime();
  _Universe->pushActiveStarSystem(this);
  //WARNING PERFORMANCE HACK!!!!!
    if (time>2*SIMULATION_ATOM) {
      time = 2*SIMULATION_ATOM;
    }
  if(time/SIMULATION_ATOM>=(1./PHY_NUM)) {
    while(time/SIMULATION_ATOM >= (1./PHY_NUM)) { // Chew up all SIMULATION_ATOMs that have elapsed since last update
      UnitCollection::UnitIterator iter;
      if (current_stage==PHY_AI) {
	if (firstframe&&rand()%2) {
	  if (this==_Universe->getActiveStarSystem(0)) {
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"Snd");
  fflush (stderr);
#endif
	    AUDRefreshSounds();
	  }
	}
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"AI");
  fflush (stderr);
#endif
  ExecuteUnitAI();
	current_stage=TERRAIN_BOLT_COLLIDE;
      } else if (current_stage==TERRAIN_BOLT_COLLIDE) {
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"TerCol");
  fflush (stderr);
#endif
	TerrainCollide();
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"Ani");
  fflush (stderr);
#endif
	UpdateAnimatedTexture();
	///Do gravitation!
	iter = units.createIterator();
	  while((unit = iter.current())!=NULL) {
	    //gravitate //FIXME
	    iter.advance();
	  }
	  //FIXME somehow only works if called once per frame
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"DelQ");
  fflush (stderr);
#endif
	  Unit::ProcessDeleteQueue();
	  current_stage=MISSION_SIMULATION;
      }else if (current_stage==MISSION_SIMULATION) {
		  // If we run only player starsystem and this is first (only) starsystem or if we run all starsystems
		if( (run_only_player_starsystem && _Universe->getActiveStarSystem(0)==this) || !run_only_player_starsystem)
		{
			if (executeDirector) {
				ExecuteDirector();
			}
		}

		current_stage=PHY_COLLIDE;
      }else if (current_stage==PHY_COLLIDE) {
#ifdef NO_COLLISION_TIME
	if (no_collision_time) {
	  no_collision_time--;//don't resolve physics until 2 seconds
	}else 
#endif
	  {
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"neb");
  fflush (stderr);
#endif
	  iter = drawList.createIterator();
	  while((unit = iter.current())!=NULL) {
		unit->SetNebula(NULL); 
		iter.advance();
	  }
	  iter = drawList.createIterator();
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"Coll");
  fflush (stderr);
#endif
	  while((unit = iter.current())!=NULL) {
	    unit->CollideAll();
	    iter.advance();
	  }

	}
	UpdateMissiles();//do explosions
	current_stage=PHY_TERRAIN;
      } else if (current_stage==PHY_TERRAIN) {
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"TerU");
  fflush (stderr);
#endif
	UpdateTerrain();
	unsigned int i=_Universe->CurrentCockpit();
	for (int j=0;j<_Universe->numPlayers();j++) {
	  if (_Universe->AccessCockpit(j)->activeStarSystem==this) {
	    _Universe->SetActiveCockpit(j);
	    _Universe->AccessCockpit(j)->Update();
	  }
	}
	_Universe->SetActiveCockpit(i);
	current_stage=PHY_RESOLV;
      } else if (current_stage==PHY_RESOLV) {
	iter = drawList.createIterator();
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"muzak");
  fflush (stderr);
#endif
	if (this==_Universe->getActiveStarSystem(0))
		UpdateCameraSnds();
	TestMusic();
	NebulaUpdate( this);
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"unphi");
  fflush (stderr);
#endif
      Unit * owner = _Universe->AccessCockpit()->GetParent();
      if (owner) {
	if (owner->InCorrectStarSystem(this)) {
	  if (getTimeCompression()>1) {//if not paused
	    if (!owner->AutoPilotTo (owner,true)) {
	      
	      reset_time_compression(0,PRESS);
	    }
	  }
	}
      }
      UpdateUnitPhysics(firstframe);
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"boltphi");
  fflush (stderr);
#endif
	bolts->UpdatePhysics();
	for (unsigned int i=0;i<active_missions.size();i++) {
	  active_missions[i]->BriefingUpdate();//waste of farkin time
	}
	current_stage=PHY_AI;
	firstframe = false;
      }
      time -= (1./PHY_NUM)*SIMULATION_ATOM;
    }
  }
#ifdef UPDATEDEBUG
  VSFileSystem::vs_fprintf (stderr,"endupd\n");
  fflush (stderr);
#endif
  UnitCollection::FreeUnusedNodes();
  collidetable->Update();
  SIMULATION_ATOM =  normal_simulation_atom;
  _Universe->popActiveStarSystem();
  //  VSFileSystem::vs_fprintf (stderr,"bf:%lf",interpolation_blend_factor);
}

/***************************************************************************************/
/*** STAR SYSTEM JUMP STUFF                                                          ***/
/***************************************************************************************/

Hashtable<std::string, StarSystem, 127> star_system_table;

void StarSystem::AddStarsystemToUniverse(const string &mname) {
  star_system_table.Put (mname,this);
}
void StarSystem::RemoveStarsystemFromUniverse () {
  star_system_table.Delete (filename);
}

StarSystem *GetLoadedStarSystem(const char * system) {
  StarSystem *ss = star_system_table.Get(string(system));
  std::string ssys (string(system)+string(".system"));
  if (!ss) {
    ss = star_system_table.Get (ssys);
  }
  return ss;
}

std::vector <unorigdest *> pendingjump;
bool PendingJumpsEmpty() {
	return pendingjump.empty();
}
void StarSystem::ProcessPendingJumps() {
  for (unsigned int kk=0;kk<pendingjump.size();kk++) {
	Unit * un = pendingjump[kk]->un.GetUnit();
    if (pendingjump[kk]->delay>=0) {
		Unit * jp = pendingjump[kk]->jumppoint.GetUnit();
		if (un&&jp) {
			QVector delta = (jp->LocalPosition()-un->LocalPosition());
			float dist =delta.Magnitude();
			if (pendingjump[kk]->delay>0) {
				float speed = dist/pendingjump[kk]->delay;
				if (dist>10) {
					un->SetCurPosition(un->LocalPosition()+SIMULATION_ATOM*delta*(speed/dist));
				}
			}
		}
		double time = GetElapsedTime();
		if (time>1)
			time=1;
		pendingjump[kk]->delay-=time;
		continue;
    } else {
#ifdef JUMP_DEBUG
  VSFileSystem::vs_fprintf (stderr,"Volitalizing pending jump animation.\n");
#endif
      _Universe->activeStarSystem()->VolitalizeJumpAnimation (pendingjump[kk]->animation);
    }
 	int playernum = _Universe->whichPlayerStarship( un);
	// In non-networking mode or in networking mode or a netplayer wants to jump and is ready or a non-player jump
 	if( Network==NULL || playernum<0 || Network!=NULL && playernum>=0 &&  Network[playernum].readyToJump())
	{
	    Unit * un=pendingjump[kk]->un.GetUnit();

	    StarSystem * savedStarSystem = _Universe->activeStarSystem();
		// Download client descriptions of the new zone (has to be blocking)
 		if( Network!=NULL)
 			Network[playernum].downloadZoneInfo();
 
	    if (un==NULL||!_Universe->StillExists (pendingjump[kk]->dest)||!_Universe->StillExists(pendingjump[kk]->orig))
		{
#ifdef JUMP_DEBUG
	      VSFileSystem::vs_fprintf (stderr,"Adez Mon! Unit destroyed during jump!\n");
#endif
	      delete pendingjump[kk];
	      pendingjump.erase (pendingjump.begin()+kk);
	      kk--;
	      continue;
	    }
	    bool dosightandsound = ((pendingjump[kk]->dest==savedStarSystem)||_Universe->isPlayerStarship(un));
	    _Universe->setActiveStarSystem (pendingjump[kk]->orig);
	    un->TransferUnitToSystem (kk, savedStarSystem,dosightandsound);
	    if (dosightandsound) {
	      _Universe->activeStarSystem()->DoJumpingComeSightAndSound(un);
	    }
	    delete pendingjump[kk];
	    pendingjump.erase (pendingjump.begin()+kk);
	    kk--;
	    _Universe->setActiveStarSystem(savedStarSystem);
		// In networking mode we tell the server we want to go back in game
		if( Network != NULL)
		{
			// Find the corresponding networked player
			if( playernum >= 0)
			{
				Network[playernum].inGame();
				Network[playernum].unreadyToJump();
			}
		}
	}
  }

}
void ActivateAnimation(Unit * jumppoint) {
	jumppoint->graphicOptions.Animating=1;
	Unit * un;
	for (un_iter i = jumppoint->getSubUnits();NULL!=(un = *i);++i){
		ActivateAnimation(un);
	}
}

bool StarSystem::JumpTo (Unit * un, Unit * jumppoint, const std::string &system, bool force) {
  if ((un->DockedOrDocking()&(~Unit::DOCKING_UNITS))!=0) {
    return false;
  }
 if( Network==NULL || force)
 {
  if (un->jump.drive>=0) {
    un->jump.drive=-1;
  }
#ifdef JUMP_DEBUG
  VSFileSystem::vs_fprintf (stderr,"jumping to %s.  ",system.c_str());
#endif
  StarSystem * ss = star_system_table.Get(system);
  if( SERVER && !ss)
  {
	  // On server side the system must have been generated before a JumpTo call !
	  cout<<"!!! ERROR = StarSystem not found"<<endl;
	  exit(1);
  }
  std::string ssys (system+".system");
  if (!ss) {
    ss = star_system_table.Get (ssys);
  }
  bool justloaded=false;
  if (!ss) {
    justloaded=true;
    ss = _Universe->GenerateStarSystem (ssys.c_str(),filename.c_str(),Vector (0,0,0));
  }
  if(ss) {
#ifdef JUMP_DEBUG
	VSFileSystem::vs_fprintf (stderr,"Pushing back to pending queue!\n");
#endif
    bool dosightandsound = ((this==_Universe->getActiveStarSystem (0))||_Universe->isPlayerStarship (un));
    int ani =-1;
      if (dosightandsound) {
        ani=_Universe->activeStarSystem()->DoJumpingLeaveSightAndSound (un);
      }
      pendingjump.push_back (new unorigdest (un,jumppoint, this,ss,un->GetJumpStatus().delay,ani,justloaded ));
#if 0
      UnitImages * im=  &un->GetImageInformation();
      for (unsigned int i=0;i<=im->dockedunits.size();i++) {
        Unit* unk =NULL;
        if (i<im->dockedunits.size()) {
  	      im->dockedunits[i]->uc.GetUnit();
        }else {
	      unk = im->DockedTo.GetUnit();
        }
        if (unk!=NULL) {
	      TentativeJumpTo (this,unk,jumppoint,system);
        }
      }
    
#endif
  } else {
#ifdef JUMP_DEBUG
	VSFileSystem::vs_fprintf (stderr,"Failed to retrieve!\n");
#endif
    return false;
  }
  if (jumppoint) 
	  ActivateAnimation(jumppoint);
 }
 else
 {
 	// Networking mode
	if( !SERVER)
		Network->jumpRequest( system, jumppoint->GetSerial());
 }

  return true;
}
