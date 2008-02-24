#ifndef _GENERICSYSTEM_H_
#define _GENERICSYSTEM_H_

#include <expat.h>
#include <string>
#include <vector>
#include "xml_support.h"
#include "gfxlib_struct.h"
#include "gfx/vec.h"
#include "cmd/collection.h"
#include "cmd/container.h"
#include <map>
class Stars;
class Planet;
class ClickList;
class Unit;
class Background;
class Terrain;
class ContinuousTerrain;
class Universe;
class CollideMap;
//class TextPlane;
struct AtmosphericFogMesh
{
	std::string meshname;
	double scale;
	float er;float eg; float eb; float ea;
	float dr;float dg; float db; float da;
	double focus;
	double concavity;
	int tail_mode_start;
	int tail_mode_end;
	int min_alpha;
	int max_alpha;
	AtmosphericFogMesh ();
};

using XMLSupport::AttributeList;
//class Atmosphere;
/**
 * Star System
 * Scene management for a star system
 * Per-Frame Drawing & Physics simulation
 **/
const int SIM_QUEUE_SIZE=128;
class StarSystem
{
	protected:
		///Starsystem XML Struct For use with XML loading
		struct StarXML
		{
			Terrain * parentterrain;
			ContinuousTerrain * ct;
			int unitlevel;
			std::vector <GFXLight> lights;
			std::vector <Planet *> moons;
			std::string backgroundname;
			Vector systemcentroid;
			Vector cursun;
			float timeofyear;
			float reflectivity;
			int numnearstars;
			int numstars;
			bool fade;
			float starsp;
			float scale;
			std::vector <AtmosphericFogMesh> fog;
			int fogopticalillusion;
		} *xml;
		void LoadXML(const char*, const Vector & centroid, const float timeofyear);
		void beginElement(const std::string &name, const AttributeList &attributes);
		void endElement(const std::string &name);

	public:
		struct Statistics
		{
								 //neutral, friendly, enemy
			std::vector<UnitContainer> navs[3];
			stdext::hash_map <std::string, UnitContainer> jumpPoints;
			int system_faction;
			int newfriendlycount;
			int newenemycount;
			int newcitizencount;
			int newneutralcount;
			int friendlycount;
			int enemycount;
			int neutralcount;
			int citizencount;
			size_t checkIter;
			size_t navCheckIter;
			Statistics();
			void AddUnit(Unit * un);
			void RemoveUnit(Unit *un);
			void CheckVitals(StarSystem*ss);
		}stats;
		StarSystem();
		StarSystem(const char * filename, const Vector & centroid=Vector (0,0,0), const float timeofyear=0);
		virtual ~StarSystem();
	protected:
		///Physics is divided into 3 stages spread over 3 frames
		enum PHYSICS_STAGE {MISSION_SIMULATION,PROCESS_UNIT,PHY_NUM}
		current_stage;

		///Stars, planets, etc. Orbital mechanics precalculated

	protected:
		std::vector <Terrain *> terrains;
		std::vector <ContinuousTerrain *>contterrains;
		/// Everything to be drawn. Folded missiles in here oneday
		UnitCollection drawList;
		UnitCollection GravitationalUnits;
		UnitCollection physics_buffer[SIM_QUEUE_SIZE+1];
		unsigned int current_sim_location;
		/// Objects subject to global gravitron physics (disabled)
		unsigned char no_collision_time;
		///system name
		char * name;
		std::string filename;
		un_iter sigIter;
		///Process global clicks for input/output
		//InputDFA * systemInputDFA;
		///to track the next given physics frame
		double time;
		///The background associated with this system
		//Background * bg;
		///The moving, fading stars
		Stars *stars;
		///The Light Map corresponding for the BP for spheremapping
		//Texture *LightMap[6];
		void AddStarsystemToUniverse(const std::string &filename);
		void RemoveStarsystemFromUniverse();
		friend class Universe;
		int lightcontext;
		std::vector <class MissileEffect *> dischargedMissiles;
		unsigned int zone;		 //short fix
	public:
		std::multimap<Unit*,Unit*>last_collisions;
								 //short fix
		void  SetZone( unsigned int zonenum) {
			this->zone = zonenum;
		}
		unsigned int GetZone() { //short fix
			return this->zone;
		}
		virtual void AddMissileToQueue(class MissileEffect *);
		virtual void UpdateMissiles();
		void UpdateUnitPhysics(bool firstframe);
		///Requeues the unit so that it is simulated ASAP.
		void RequestPhysics(Unit *un, unsigned int queue);
		virtual void activateLightMap(int stage=1) {}
		///activates the light map texture
		virtual unsigned int numTerrain () {return 0;}
		virtual unsigned int numContTerrain () {return 0;}
		///Draws a frame of action, interpolating between physics frames
		virtual void Draw(bool DrawCockpit=true) {}
		/// update a simulation atom ExecuteDirector must be false if star system is just loaded before mission is loaded
		void Update(float priority, bool executeDirector);
		// This one is temporarly used on server side
		void Update(float priority);
		///re-enables the included lights and terrains
		virtual void SwapIn () {}
		///Disables included lights and terrains
		virtual void SwapOut () {}
		virtual Terrain * getTerrain (unsigned int which) {return NULL;}
		virtual ContinuousTerrain * getContTerrain (unsigned int which) {return NULL;}
		virtual Background* getBackground() {return NULL;}
		virtual ClickList *getClickList() { return NULL;}
		///Gets the current simulation frame
		unsigned int getCurrentSimFrame() const { return current_sim_location; };

		void ExecuteUnitAI();
		class CollideTable *collidetable;
								 //0 = UNIT_ONLY 1 = UNIT_BOLT...use enum from Unit::
		CollideMap *collidemap[2];
		class bolt_draw *bolts;
		static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
		static void endElement(void *userData, const XML_Char *name);
		std::string getFileName()const ;
		std::string getName();
		///Loads the star system from an XML file
		UnitCollection& getUnitList() {return drawList;}
		UnitCollection& gravitationalUnits() {return GravitationalUnits;}
		Unit *nextSignificantUnit();
		/// returns xy sorted bounding spheres of all units in current view
		///Adds to draw list
		void AddUnit(Unit *unit);
		///Removes from draw list
		bool RemoveUnit(Unit *unit);
		bool JumpTo (Unit * unit, Unit * jumppoint, const std::string &system, bool force=false, bool save_coordinates=false/*for intersystem transit the long way*/);
		static void ProcessPendingJumps ();
		virtual void VolitalizeJumpAnimation(const int ani){}
		virtual void DoJumpingComeSightAndSound(Unit * un){}
		virtual int DoJumpingLeaveSightAndSound (Unit * un){return -1;}
		//friend class Atmosphere;
		virtual void  createBackground( StarSystem::StarXML * xml) {}
};
bool PendingJumpsEmpty();
double calc_blend_factor(double frac, int priority, int when_it_will_be_simulated, int cur_simulation_frame);
#endif
