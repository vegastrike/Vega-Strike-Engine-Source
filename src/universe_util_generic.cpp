#include <math.h>
#include "lin_time.h"
#include "cmd/script/mission.h"
#include "universe_util.h"
#include "universe_generic.h"
#include "cmd/unit_generic.h"
//#include "cmd/unit_interface.h"
#include "cmd/unit_factory.h" //for UnitFactory::getMasterPartList()
#include "cmd/collection.h"
#include "star_system_generic.h"
#include <string>
#include "savegame.h"
#include "cmd/unit_csv.h"
//#include "audiolib.h"
//#include "gfx/animation.h"
#include "gfx/cockpit_generic.h"
#include "lin_time.h"
#include "load_mission.h"
#include "configxml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "cmd/unit_util.h"
#include "networking/netserver.h"
#include "cmd/csv.h"
#include "linecollide.h"
#include "cmd/unit_collide.h"
//extern class Music *muzak;
//extern unsigned int AddAnimation (const QVector & pos, const float size, bool mvolatile, const std::string &name, float percentgrow );
extern Unit&GetUnitMasterPartList();
extern std::string universe_path;
extern int num_delayed_missions();
using std::string;

#define activeSys _Universe->activeStarSystem() //less to write
using namespace VSFileSystem;

namespace UniverseUtil {
	Unit * PythonUnitIter::current(){
		Unit * ret=NULL;
		while ((ret= UnitIterator::current())) {
			if (ret->hull>0)
				return ret;
			advance();
		}
		return ret;
	}
	
	Unit * GetUnitFromSerial( ObjSerial serial)
	{
		Unit* un = NULL;
		un_iter it = UniverseUtil::getUnitList();
		// Find the unit
		do
		{
			if( it.current()->GetSerial()==serial)
				un = it.current();
			it.advance();
		}
		while(un==NULL && it.current()!=NULL);
		if( un==NULL)
			cout<<"ERROR --> Received a fire order for non-existing UNIT"<<endl;
		return un;
	}
	std::string vsConfig(std::string category,std::string option,std::string def){
		return vs_config->getVariable(category,option,def);
	}

	Unit *launchJumppoint(string name_string,
			string faction_string,
			string type_string,
			string unittype_string,
			string ai_string,
			int nr_of_ships,
			int nr_of_waves, 
			QVector pos, 
			string squadlogo, 
			string destinations){

		int clstype=UNITPTR;
		if (unittype_string=="planet") {
			clstype =PLANETPTR;			
		}else if (unittype_string=="asteroid") {
			clstype = ASTEROIDPTR;
		}else if (unittype_string=="nebula") {
			clstype = NEBULAPTR;
		}
		CreateFlightgroup cf;
		cf.fg = Flightgroup::newFlightgroup (name_string,type_string,faction_string,ai_string,nr_of_ships,nr_of_waves,squadlogo,"",mission);
		cf.unittype=CreateFlightgroup::UNIT;
		cf.terrain_nr=-1;
		cf.waves=nr_of_waves;
		cf.nr_ships=nr_of_ships;
		cf.fg->pos=pos;
        cf.rot[0]=cf.rot[1]=cf.rot[2]=0.0f;
		Unit *tmp= mission->call_unit_launch(&cf,clstype,destinations);
		mission->number_of_ships+=nr_of_ships;

		return tmp;
	}
	Cargo getRandCargo(int quantity, string category) {
	  Cargo *ret=NULL;
	  Unit *mpl = &GetUnitMasterPartList();
	  unsigned int max=mpl->numCargo();
	  if (!category.empty()) {
            size_t Begin,End;
	    mpl->GetSortedCargoCat (category,Begin,End);
	    if (Begin<End) {
	      unsigned int i=Begin+(rand()%(End-Begin));              
	      ret = &mpl->GetCargo(i);
	    }
	  }else {
	    if (mpl->numCargo()) {
	      for (unsigned int i=0;i<500;i++) {
		ret = &mpl->GetCargo(rand()%max);  
		if (ret->content.find("mission")==string::npos) {
		  break;
		}
	      }
	    }		  
	  }
	  if (ret) {
		Cargo tempret = *ret;
		tempret.quantity=quantity;
	    return tempret;//uses copy
	  }else {
	    Cargo newret;
	    newret.quantity=0;
	    return newret;
	  }
	}
	//NOTEXPORTEDYET
	float GetGameTime () {
		return mission->gametime;
	}
	void SetTimeCompression () {
		setTimeCompression(1.0);
	}
	static UnitContainer scratch_unit;
	static QVector scratch_vector;

	Unit *GetMasterPartList () {
		return UnitFactory::getMasterPartList();
	}
	Unit *getScratchUnit () {
		return scratch_unit.GetUnit();
	}
        void setScratchUnit (Unit * un) {
		scratch_unit.SetUnit(un);
	}

	QVector getScratchVector () {
		return scratch_vector;
	}
        void setScratchVector (QVector un) {
		scratch_vector=un;
	}

	void pushSystem (string name) {
		StarSystem * ss = _Universe->GenerateStarSystem (name.c_str(),"",Vector(0,0,0));
		_Universe->pushActiveStarSystem(ss);
	}
	void popSystem () {
		_Universe->popActiveStarSystem();
	}
	string getSystemFile() {
		return activeSys->getFileName();
	}

	string getSystemName() {
		return activeSys->getName();
	}
	///tells the respective flightgroups in this system to start shooting at each other
	void TargetEachOther (string fgname, string faction, string enfgname, string enfaction){
		int fac = FactionUtil::GetFaction(faction.c_str());
		int enfac = FactionUtil::GetFaction(enfaction.c_str());		
		un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();
		Unit * un=i.current();
		Unit * en=NULL;
		Unit * al=NULL;
		while (un && ((NULL== en) || (NULL==al))){
			if (un->faction==enfac && UnitUtil::getFlightgroupName(un)==enfgname) {
				if ((NULL== en) || (rand()%3==0)){
					en=un;
				}
			}
			if (un->faction==fac && UnitUtil::getFlightgroupName(un)==fgname){
				al=un;
			}
			un=i.next();
		}
		if (en && al) {
			UnitUtil::setFlightgroupLeader(al,al);
			al->Target(en);
			UnitUtil::setFgDirective (al,"A."); //attack target, darent change target!
			UnitUtil::setFlightgroupLeader(en,en);
			en->Target(al);
			UnitUtil::setFgDirective (en,"h");//help me out here!
		}
	}

	///tells the respective flightgroups in this system to stop killing each other urgently...they may still attack--just not warping and stuff
	void StopTargettingEachOther(string fgname, string faction, string enfgname, string enfaction){
		int fac = FactionUtil::GetFaction(faction.c_str());
		int enfac = FactionUtil::GetFaction(enfaction.c_str());	 		
		un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();
		Unit * un=i.current();
		int clear = 0;
		while (un&&clear!=3) {
			if ((un->faction==enfac && UnitUtil::getFlightgroupName(un)==enfgname)) {
				clear|=1;
				UnitUtil::setFgDirective (un,"b");				
			}else
				if (un->faction==fac && UnitUtil::getFlightgroupName(un)==fgname) {
					clear|=2;
					UnitUtil::setFgDirective (un,"b");
					//check to see that its' in this flightgroup or something :-)
				}
			un=i.next();
		}
	}
	
	
	bool systemInMemory(string nam) {
	  unsigned int nass = _Universe->star_system.size();;
	  for (unsigned int i=0;i<nass;i++) {
	    if (_Universe->star_system[i]->getFileName()==nam)
	      return true;
	  }
	  return false;
	}
	void setMissionOwner(int whichplayer) {
		mission->player_num=whichplayer;
	}
	int getMissionOwner() {
		return mission->player_num;
	}
	UniverseUtil::PythonUnitIter getUnitList() {
		return activeSys->getUnitList().createIterator();
	}
	Unit *getUnit(int index) {
		un_iter iter=activeSys->getUnitList().createIterator();
		Unit * un=NULL;
		for(int i=-1;(un=iter.current())&&i<index;iter.advance()) {
			if (un->GetHull()>0)
				i++;
			if (i==index)
				break;
		}
		return un;
	}
    Unit *getUnitByName(std::string name) {
		un_iter iter=activeSys->getUnitList().createIterator();
        while (iter.notDone() && UnitUtil::getName(iter.current()) != name)
            iter.advance();
        return iter.notDone()?iter.current():NULL;
	}
        int getNumUnits() {
	  int count=0;
	  un_iter iter=activeSys->getUnitList().createIterator();
	  while (iter.current()){
	    iter.advance();
	    count++;
	  }
	  return count;
	}
	//NOTEXPORTEDYET
	/*
	float GetGameTime () {
		return mission->gametime;
	}
	void SetTimeCompression () {
		setTimeCompression(1.0);
	}
	*/
	string GetAdjacentSystem (string str, int which) {
		return _Universe->getAdjacentStarSystems(str)[which];
	}
	string GetGalaxyProperty (string sys, string prop) {
		return _Universe->getGalaxyProperty(sys,prop);
	}
	string GetGalaxyPropertyDefault (string sys, string prop, string def) {
		return _Universe->getGalaxyPropertyDefault(sys,prop,def);
	}
#define DEFAULT_FACTION_SAVENAME "FactionTookOver_"

				string GetGalaxyFaction (string sys) {
						string fac = _Universe->getGalaxyProperty (sys,"faction");
						vector <string> * ans = &(_Universe->AccessCockpit(0)->savegame->getMissionStringData(string(DEFAULT_FACTION_SAVENAME)+sys));
						if (ans->size()) {
								fac = (*ans)[0];
						}
						return fac;
				}
				void SetGalaxyFaction (string sys, string fac) {
						vector <string> * ans = &(_Universe->AccessCockpit(0)->savegame->getMissionStringData(string(DEFAULT_FACTION_SAVENAME)+sys));
						if (ans->size()) {
								(*ans)[0]=fac;
						}else {
								ans->push_back(fac);
						}
				}
				int GetNumAdjacentSystems (string sysname) {
						return _Universe->getAdjacentStarSystems(sysname).size();
				}
				/*
				   int musicAddList(string str) {
				   return muzak->Addlist(str.c_str());
				   }
				   void musicPlaySong(string str) {
				   muzak->GotoSong(str);
				   }
				   void musicPlayList(int which) {
				   muzak->SkipRandSong(which);
				   }
				   void musicLoopList (int numloops) {
				   muzak->loopsleft=numloops;
				   }
				 */
				float GetDifficulty () {
						return g_game.difficulty;
				}
				void SetDifficulty (float diff) {
						g_game.difficulty=diff;
				}
				/*
				   void playSound(string soundName, QVector loc, Vector speed) {
				   int sound = AUDCreateSoundWAV (soundName,false);
				   AUDAdjustSound (sound,loc,speed);
				   AUDStartPlaying (sound);
				   AUDDeleteSound(sound);
				   }
				   void cacheAnimation(string aniName) {
				   static vector <Animation *> anis;
				   anis.push_back (new Animation(aniName.c_str()));
				   }
				   void playAnimation(string aniName, QVector loc, float size) {
				   AddAnimation(loc,size,true,aniName,1);
				   }
				   void playAnimationGrow(string aniName, QVector loc, float size, float growpercent) {
				   AddAnimation(loc,size,true,aniName,growpercent);
				   }
				   unsigned int getCurrentPlayer() {
				   return _Universe->CurrentCockpit();
				   }
				   Unit *getPlayer(){
				   return _Universe->AccessCockpit()->GetParent();;
				   }
				   int getNumPlayers () {
				   return _Universe->numPlayers();
				   }
				   Unit *getPlayerX(int which){
				   if (which>=getNumPlayers()) {
				   return NULL;
				   }
				   return _Universe->AccessCockpit(which)->GetParent();
				   }
				 */
				extern void playVictoryTune();
				void terminateMission(bool Win){
						if (Win)
								playVictoryTune();
						mission->terminateMission();
				}
  static string dontBlankOut(string objective) {
    while(1) {
      std::string::size_type where=objective.find(".blank");
      if (where!=string::npos) {
        objective = objective.substr(0,where)+objective.substr(where+strlen(".blank"));
      }else return objective;
    }
    return objective;
  }
				int addObjective(string objective) {
						mission->objectives.push_back(Mission::Objective(0,dontBlankOut(objective)));
						return mission->objectives.size()-1;
				}
				void setObjective(int which, string newobjective) {
						if (which<(int)mission->objectives.size()) {
								mission->objectives[which].objective=dontBlankOut(newobjective);
						}
				}
				void setCompleteness(int which, float completeNess) {
						if (which<(int)mission->objectives.size()) {
								mission->objectives[which].completeness=completeNess;
						}
				}
				float getCompleteness(int which) {
						if (which<(int)mission->objectives.size()) {
								return mission->objectives[which].completeness;
						} else {
								return 0;
						}
				}
				void setOwnerII(int which,Unit *owner) {
						if (which<(int)mission->objectives.size()) {
								mission->objectives[which].setOwner(owner);
						}
				}
				Unit* getOwner(int which) {
						if (which<(int)mission->objectives.size()) {
								return mission->objectives[which].getOwner();
						} else {
								return 0;
						}
				}
				int numActiveMissions() {
						return active_missions.size()+::num_delayed_missions();
				}
				void IOmessage(int delay,string from,string to,string message){
						static bool news_from_cargolist=XMLSupport::parse_bool(vs_config->getVariable("cargo","news_from_cargolist","false"));
						if (to=="news"&&(!news_from_cargolist))
								_Universe->AccessCockpit(0)->savegame->getMissionStringData("dynamic_news").push_back(string("#")+message);
						else
								mission->msgcenter->add(from,to,message,delay);
				}
				Unit *GetContrabandList (string faction) {
						return FactionUtil::GetContraband(FactionUtil::GetFaction(faction.c_str()));
				}
				void LoadMission (string missionname) {
					::LoadMission (missionname.c_str(),"",false);
				}
				void LoadMissionScript (string missionscript) {
					::LoadMission ("nothing.mission",missionscript,false);
				}

				void SetAutoStatus (int global_auto, int player_auto) {
						if (global_auto==1) {
								mission->global_autopilot = Mission::AUTO_ON;
						}else if (global_auto==-1) {
								mission->global_autopilot = Mission::AUTO_OFF;
						}else {
								mission->global_autopilot = Mission::AUTO_NORMAL;
						}

						if (player_auto==1) {
								mission->player_autopilot = Mission::AUTO_ON;	    
						}else if (player_auto==-1) {
								mission->player_autopilot = Mission::AUTO_OFF;
						}else {
								mission->player_autopilot = Mission::AUTO_NORMAL;
						}
				}
  static bool isAsteroid(std::string name){
    if (name.length()<8) return false;
    return (name[0]=='A'||name[0]=='a')&&
      name[1]=='s'&&
      name[2]=='t'&&
      name[3]=='e'&&
      name[4]=='r'&&
      name[5]=='o'&&
      name[6]=='i'&&
      name[7]=='d'
      ;
  }
				QVector SafeStarSystemEntrancePoint (StarSystem* sts,QVector pos, float radial_size) {
                                  
						static double def_un_size = XMLSupport::parse_float (vs_config->getVariable ("physics","respawn_unit_size","400"));
						if (radial_size<0)
								radial_size = def_un_size;
                                                
                                                LineCollide obtain(NULL,LineCollide::UNIT,pos-QVector(radial_size,radial_size,radial_size),pos+QVector(radial_size,radial_size,radial_size));
                                                UnitCollection * colQ [tablehuge+1];   
                                                int sizecolq = sts->collidetable->c.Get (&obtain,colQ,true);
                                  
						for (unsigned int k=0;k<10;k++) {
								Unit * un;
								bool collision=false;
                                                                for (int jjj=0;jjj<sizecolq;jjj++) {
                                                                  for (un_iter i=colQ[jjj]->createIterator();(un=*i)!=NULL;++i) {
										if (un->isUnit()==ASTEROIDPTR||un->isUnit()==NEBULAPTR||isAsteroid(un->name)) {
												continue;
										}
										double dist = (pos-un->LocalPosition()).Magnitude()-un->rSize()-/*def_un_size-*/radial_size;
										if (dist<0) {
												QVector delta  = pos-un->LocalPosition();
												double mag = delta.Magnitude();
												if (mag>.01){
														delta=delta/mag;
												}else {
														delta.Set(0,0,1);
												}
												delta = delta.Scale ( dist+un->rSize()+radial_size);
												if (k<5) {
														pos = pos+delta;
														collision=true;
												}else {
														QVector r(.5,.5,.5);
														pos+=(radial_size+un->rSize())*r;
														collision=true;
												}

										}
                                                                  }
                                                                  if (collision==false)
                                                                    break;
                                                                }
                                                
						}
						return pos;
				}
				QVector SafeEntrancePoint (QVector pos, float radial_size) {
                                  return SafeStarSystemEntrancePoint(_Universe->activeStarSystem(),pos,radial_size);

				}
				Unit* launch (string name_string,string type_string,string faction_string,string unittype, string ai_string,int nr_of_ships,int nr_of_waves, QVector pos, string sqadlogo){
						return launchJumppoint(name_string,faction_string,type_string,unittype,ai_string,nr_of_ships,nr_of_waves,pos,sqadlogo,"");
				}


	string LookupUnitStat(string unitname, string faction, string statname){
          CSVRow tmp(LookupUnitRow(unitname,faction));
          if (tmp.success())
            return tmp[statname];
          return "";          
        }

	
                                static std::vector <Unit *> cachedUnits;
				void precacheUnit (string type_string,string faction_string){
                                  cachedUnits.push_back(UnitFactory::createUnit(type_string.c_str(),true,FactionUtil::GetFactionIndex(faction_string)));
                                  
                                }
				Unit *getPlayer(){
						return _Universe->AccessCockpit()->GetParent();;
				}
				int getNumPlayers () {
						return _Universe->numPlayers();
				}
				Unit *getPlayerX(int which){
						if (which>=getNumPlayers()) {
								return NULL;
						}
						return _Universe->AccessCockpit(which)->GetParent();
				}
				float getPlanetRadiusPercent () {
						static float planet_rad_percent =  XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_planet_radius_percent",".75"));
						return planet_rad_percent;
				}
				std::string getVariable(std::string section,std::string name,std::string def) {
						return vs_config->getVariable(section,name,def);
				}
				std::string getSubVariable(std::string section,std::string subsection,std::string name,std::string def) {
						return vs_config->getVariable(section,subsection,name,def);
				}
				double timeofday () {return getNewTime();}
				double sqrt (double x) {return ::sqrt (x);}
				double log (double x) {return ::log (x);}
				double exp (double x) {return ::exp (x);}
				double cos (double x) {return ::cos (x);}
				double sin (double x) {return ::sin (x);}
				double acos (double x) {return ::acos (x);}
				double asin (double x) {return ::asin (x);}
				double atan (double x) {return ::atan (x);}
				double tan (double x) {return ::tan (x);}
				void micro_sleep(int n) {
						::micro_sleep(n);
				}

				void	ComputeSystemSerials( std::string & systempath)
				{
						using namespace VSFileSystem;
						// Read the file
						VSFile f;
						VSError err = f.OpenReadOnly( systempath, SystemFile);
						if( err<=Ok)
						{
							cout<<"\t\tcomputing serials for "<<systempath<<"...";
							std::string system = f.ReadFull();

							// Now looking for "<planet ", "<Planet ", "<PLANET ", "<unit ", "<Unit ", "<UNIT ", same for nebulas
							std::vector<std::string> search_patterns;

							bool newserials = true;
							if( system.find( "serial=", 0) != std::string::npos)
							{
								newserials = false;
								cout<<"Found serial in system file : replacing serials..."<<endl;
							}
							else
								cout<<"Found no serial in system file : generating..."<<endl;
							search_patterns.push_back( "<planet ");
							search_patterns.push_back( "<Planet ");
							search_patterns.push_back( "<PLANET ");
							search_patterns.push_back( "<unit ");
							search_patterns.push_back( "<Unit ");
							search_patterns.push_back( "<UNIT ");
							search_patterns.push_back( "<nebula ");
							search_patterns.push_back( "<Nebula ");
							search_patterns.push_back( "<NEBULA ");

							for( std::vector<std::string>::iterator ti=search_patterns.begin(); ti!=search_patterns.end(); ti++)
							{
								std::string search( (*ti));
								//cerr<<"Looking for "<<search<<endl;
								std::string::size_type search_length = (*ti).length();
								std::string::size_type curpos = 0;
								int nboc = 0;
								//cerr<<"\tLooking for "<<search<<" length="<<search_length<<endl;
								while( (curpos = system.find( search, curpos))!=std::string::npos)
								{
									//cerr<<"\t\tSearch position = "<<curpos<<endl;
									ObjSerial new_serial = getUniqueSerial();
									std::string serial_str( (*ti)+"serial=\""+XMLSupport::tostring5( new_serial)+"\" ");
									// If there are already serial in the file we replace that kind of string : <planet serial="XXXXX"
									// of length search_length + 14 (length of serial="XXXXX")
									if( newserials)
										system.replace( curpos, search_length, serial_str);
									else
										system.replace( curpos, search_length+15, serial_str);
									nboc++;
									curpos += search_length;
								}
								cerr<<"\t\tFound "<<nboc<<" occurences of "<<search<<endl;
							}

							// Add the system xml string to the server
							VSServer->addSystem( systempath, system);

							// Overwrite the system files with the buffer containing serials
							f.Close();
							// Should generate the modified system file in homedir
							err = f.OpenCreateWrite( systempath, SystemFile);
							if( err>Ok)
							{
								cerr<<"!!! ERROR : opening "<<systempath<<" for writing"<<endl;
								VSExit(1);
							}
							if( f.Write( system) != system.length() )
							{
								cerr<<"!!! ERROR : writing system file"<<endl;
								VSExit(1);
							}
							f.Close();

							cout<<" OK !"<<endl;
						}
						else
						{
							cerr<<"ERROR cannot open system file : "<<systempath<<endl;
							exit(1);
						}
				  }

	void	ComputeGalaxySerials( std::vector<std::string> & stak)
	{
	cout<<"Going through "<<stak.size()<<" sectors"<<endl;
	cout<<"Generating random serial numbers :"<<endl;
	static string sysdir = vs_config->getVariable("data","sectors","sectors");
	for( ;!stak.empty();)
	{
		string sys( stak.back()+".system");
		stak.pop_back();
		/*
		string sysfilename( sys+".system");
		string relpath( universe_path+sysdir+"/"+sysfilename);
		string systempath( datadir+relpath);
		*/
		
		ComputeSystemSerials( sys);
	}
	cout<<"Computing done."<<endl;
	}
}

#undef activeSys
