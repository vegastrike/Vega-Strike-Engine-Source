#ifndef __FACTIONGENERIC_H
#define __FACTIONGENERIC_H
#include "config.h"
#include <string>
//#include <gnuhash.h>

#include "xml_support.h"

class FSM;
class Animation;
class Unit;
class Texture;
typedef vsUMap<std::string, float> MapStringFloat;
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
    bool citizen;
    int playlist;
    float sparkcolor[4];
    std::vector <faction_stuff> faction;
	///If logo==0, load from this one
	std::string logoName;
	std::string logoAlphaName;
	///If secondaryLogo==0, load from this one
	std::string secondaryLogoName;
	std::string secondaryLogoAlphaName;
    ///Logos used by the ships of that faction
    Texture * logo;
    //if the squadron doens't; have its own particular logo
    Texture * secondaryLogo;
    ///char * of the name
    char * factionname; 
    struct comm_face_t{
      std::vector <class Animation *> animations;
      enum CHOICE{CNO,CYES,CEITHER} ;
      CHOICE dockable;
      CHOICE base;
      comm_face_t():dockable(CEITHER),base(CEITHER){}
    };
    std::vector <comm_face_t> comm_faces;
    std::vector <class Animation *> explosion;
    std::vector <std::string> explosion_name;
    std::vector <unsigned char> comm_face_sex;
    MapStringFloat ship_relation_modifier;
    Unit * contraband;
    ///Figures out the relationships of each faction with each other
    static void ParseAllAllies(/*Universe * thisuni*/);
    void ParseAllies(/*Universe * thisuni,*/ unsigned int whichfaction);
    
    static void LoadXML(const char * factionfile, char * xmlbuffer=NULL, int buflength=0);
    static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
	static void endElement(void *userData, const XML_Char *name);
	Faction() {
	playlist=-1;
        citizen=false;
		logo=secondaryLogo=NULL;
		contraband=NULL;
		factionname=NULL;
		sparkcolor[0]=.5;
		sparkcolor[1]=.5;
		sparkcolor[2]=1;
		sparkcolor[3]=1;
	}
    ~Faction(); //destructor
 };

extern  std::vector <Faction *> factions; //the factions

namespace FactionUtil {
    extern int upgradefac;
    extern int neutralfac;
    extern int planetfac;
		std::vector <class Animation *>* GetRandCommAnimation(int faction, Unit * unit, unsigned char &sex);
		void SerializeFaction (FILE * file);
		std::string SerializeFaction();
		void LoadSerializedFaction (FILE * file);
		void LoadSerializedFaction( char * &buf);
		int numnums (const char * str);
		///returns the index of the faction with that name
		//int GetFaction (const char *factionname);
		int GetNumAnimation(int faction);
		const char * GetFaction (int faction);
		class Unit * GetContraband(int faction);
		/**
		* Returns the relationship between myfaction and theirfaction
		* 1 is happy. 0 is neutral (btw 1 and 0 will not attack)
		* -1 is mad. <0 will attack
		*/
		int GetFactionIndex(std::string name);
		inline int GetUpgradeFaction(){return upgradefac;}
		inline int GetNeutralFaction(){return neutralfac;}
		inline int GetPlanetFaction(){return planetfac;}
                inline float GetIntRelation (const int myfaction, const int theirfaction){
                   return factions[myfaction]->faction[theirfaction].relationship;
                }

		//float GetRelation (std::string myfaction, std::string theirfaction);
		std::string GetFactionName(int index);
		bool isCitizenInt(int index);
		bool isCitizen(std::string name);
		void AdjustIntRelation (const int myfaction, const int theirfaction, const float factor, const float rank);
		//void AdjustRelation(std::string myfaction,std::string theirfaction, float factor, float rank);
		int GetPlaylist (const int myfaction);
		const float* GetSparkColor (const int myfaction);
		unsigned int GetNumFactions();
		//Returns a conversation that a myfaction might have with a theirfaction
		FSM* GetConversation (const int myfaction, const int theirfaction);
		Texture *getForceLogo (int faction);
		Texture *getSquadLogo (int faction);

		Animation * createAnimation( const char * anim);
		Texture * createTexture( const char * tex, const char * tmp, bool force=false);
		Texture * createTexture( const char * tex, bool force=false);
		std::vector <class Animation *> * GetAnimation(int faction, int n, unsigned char &sex);
		Animation * GetRandExplosionAnimation (int whichfaction,std::string&which);
		void LoadFactionPlaylists();
		/** Still in faction_xml.cpp because createUnit **/
		void LoadContrabandLists();
};

extern  std::vector <Faction *> factions; //the factions

#endif
