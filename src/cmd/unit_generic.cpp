
#include <set>
#include "configxml.h"
#include "audiolib.h"
#include "unit_generic.h"
#include "beam.h"
#include "lin_time.h"
#include "xml_serializer.h"
#include "vsfilesystem.h"
#include "file_main.h"
#include "universe_util.h"
#include "unit_util.h"
#include "script/mission.h"
#include "script/flightgroup.h"
#include "cmd/ai/fire.h"
#include "cmd/ai/turretai.h"
#include "cmd/ai/communication.h"
#include "cmd/ai/navigation.h"
#include "cmd/ai/script.h"
#include "cmd/ai/missionscript.h"
#include "cmd/ai/flybywire.h"
#include "cmd/ai/aggressive.h"
#include "python/python_class.h"
#include "cmd/unit_factory.h"
#include "gfx/cockpit_generic.h"
#include "gfx/vsbox.h"
#include <algorithm>
#include "cmd/ai/ikarus.h"
#include "role_bitmask.h"
#include "unit_const_cache.h"
#include "gfx/warptrail.h"
#include "networking/netserver.h"
#include "networking/netclient.h"
#include "gfx/cockpit_generic.h"
#include "universe_generic.h"
#include "unit_bsp.h"
#include "gfx/bounding_box.h"
#include "csv.h"
#ifdef _WIN32
#define strcasecmp stricmp
#endif
#include "config.h"
using namespace Orders;
void	Unit::BackupState()
{
	this->old_state.setPosition( this->curr_physical_state.position);
	this->old_state.setOrientation( this->curr_physical_state.orientation);
	this->old_state.setVelocity( this->Velocity);
	this->old_state.setAcceleration( this->net_accel);
}

void Unit::BuildBSPTree(const char *filename, bool vplane, Mesh * hull) {
  bsp_tree * bsp=NULL;
  bsp_tree temp_node;
  vector <bsp_polygon> tri;
  vector <bsp_tree> triplane;
  if (hull!=NULL) {
    hull->GetPolys (tri);
  } else {
    for (int j=0;j<nummesh();j++) {
      meshdata[j]->GetPolys(tri);
    }
  }	
  for (unsigned int i=0;i<tri.size();i++) {
    if (!Cross (tri[i],temp_node)) {
      vector <bsp_polygon>::iterator ee = tri.begin();
      ee+=i;
      tri.erase(ee);
      i--;
      continue;
    }	
    // Calculate 'd'
    temp_node.d = (double) ((temp_node.a*tri[i].v[0].i)+(temp_node.b*tri[i].v[0].j)+(temp_node.c*tri[i].v[0].k));
    temp_node.d*=-1.0;
    triplane.push_back(temp_node);
    //                bsp=put_plane_in_tree3(bsp,&temp_node,&temp_poly3); 
 }
 
 bsp = buildbsp (bsp,tri,triplane, vplane?VPLANE_ALL:0);
 if (bsp) {
   VSError err = fo.OpenCreateWrite( filename, BSPFile);
   if (err<=Ok) {
     write_bsp_tree(bsp,0);
     fo.Close();
     bsp_stats (bsp);
     FreeBSP (&bsp);
   }
 }	

}

bool flickerDamage (Unit * un, float hullpercent) {
	#define damagelevel hullpercent
		  static double counter=getNewTime();
		  
		  static float flickertime = XMLSupport::parse_float (vs_config->getVariable ("graphics","glowflicker","time","30"));
		  static float flickerofftime = XMLSupport::parse_float (vs_config->getVariable ("graphics","glowflicker","off-time","2"));
		  static float minflickercycle = XMLSupport::parse_float (vs_config->getVariable ("graphics","glowflicker","min-cycle","2"));		  
		  static float flickeronprob= XMLSupport::parse_float (vs_config->getVariable ("graphics","glowflicker","num-times-per-second-on",".66"));
		  static float hullfornoflicker= XMLSupport::parse_float (vs_config->getVariable ("graphics","glowflicker","hull-for-total-dark",".04"));
		  float diff = getNewTime()-counter;
		  if (diff>flickertime) {
			  counter=getNewTime();
			  diff=0;
		  }
		  float tmpflicker=flickertime*damagelevel;
		  if (tmpflicker<minflickercycle) {
			  tmpflicker=minflickercycle;
		  }
		  diff = fmod (diff,tmpflicker);
		  //we know counter is somewhere between 0 and damage level
		  unsigned int thus = ((int)un)>>2;//cast this to an int for fun!
		  thus = thus % ((unsigned int)tmpflicker);
		  diff = fmod (diff+thus,tmpflicker);
		  if (flickerofftime>diff) {
			  if (damagelevel>hullfornoflicker)
				  return rand()>RAND_MAX*GetElapsedTime()*flickeronprob;
			  else
				  return true;
		  }
	  return false;
	  #undef damagelevel
}

//SERIOUSLY BROKEN
Vector ReflectNormal (const Vector &vel, const Vector & norm ) {
//THIS ONE WORKS...but no...we don't want works	return norm * (2*vel.Dot(norm)) - vel;
	return norm*vel.Magnitude();
}


#define INVERSEFORCEDISTANCE 5400

void Unit::reactToCollision(Unit * smalle, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal,  float dist) {
  clsptr smltyp = smalle->isUnit();
  if (smltyp==ENHANCEMENTPTR||smltyp==MISSILEPTR) {
    if (isUnit()!=ENHANCEMENTPTR&&isUnit()!=MISSILEPTR) {
      smalle->reactToCollision (this,smalllocation,smallnormal,biglocation,bignormal,dist);
      return;
    }
  }	       
  //don't bounce if you can Juuuuuuuuuuuuuump
  if (!jumpReactToCollision(smalle)) {
#ifdef NOBOUNCECOLLISION
#else
    static float bouncepercent = XMLSupport::parse_float (vs_config->getVariable ("physics","BouncePercent",".1"));
	
	float m1=smalle->GetMass(),m2=GetMass();
    
	//Vector Elastic_dvl = (m1-m2)/(m1+m2)*smalle->GetVelocity() + smalle->GetVelocity()*2*m2/(m1+m2);
    //Vector Elastic_dvs = (m2-m1)/(m1+m2)*smalle->GetVelocity() + smalle->GetVelocity()*2*m1/(m1+m2);
    Vector Inelastic_vf = (m1/(m1+m2))*smalle->GetVelocity() + (m2/(m1+m2))*GetVelocity();
//	Vector SmallerElastic_vf = ReflectNormal(smalle->GetVelocity()-Inelastic_vf,bignormal)+Inelastic_vf;
//	Vector ThisElastic_vf = ReflectNormal(smalle->GetVelocity()-Inelastic_vf,smallnormal)+Inelastic_vf;
	Vector SmallerElastic_vf = (smalle->GetVelocity()*(m1-m2)/(m1+m2)+(2*m2/(m1+m2))*GetVelocity());
	Vector ThisElastic_vf = (GetVelocity()*(m2-m1)/(m1+m2)+(2*m1/(m1+m2))*smalle->GetVelocity());
	// Make bounce along opposite normals
	// HACK ALERT: FOLLOWING LINES are the victims of A HACK
	Cockpit * thcp = _Universe->isPlayerStarship (this);
	Cockpit * smcp = _Universe->isPlayerStarship (smalle);
	static float mintime = XMLSupport::parse_float (vs_config->getVariable ("physics","minimum_time_between_recorded_player_collisions","0.1"));
	bool isnotplayerorhasbeenmintime=true;
	static float minvel = XMLSupport::parse_float (vs_config->getVariable ("physics","minimum_collision_velocity","5"));
	ThisElastic_vf=((ThisElastic_vf.Magnitude()>minvel||!thcp)?ThisElastic_vf.Magnitude():minvel)*smallnormal;
	SmallerElastic_vf=((SmallerElastic_vf.Magnitude()>minvel||!smcp)?SmallerElastic_vf.Magnitude():minvel)*bignormal;

    float LargeKE = (0.5)*m2*GetVelocity().MagnitudeSquared();
    float SmallKE = (0.5)*m1*smalle->GetVelocity().MagnitudeSquared();
    float FinalInelasticKE = Inelastic_vf.MagnitudeSquared()*(0.5)*(m1+m2);
	float InelasticDeltaKE = LargeKE +SmallKE - FinalInelasticKE;
    static float kilojoules_per_damage = XMLSupport::parse_float (vs_config->getVariable ("physics","kilojoules_per_unit_damage","5400"));
	
    static float inelastic_scale = XMLSupport::parse_float (vs_config->getVariable ("physics","inelastic_scale",".5"));
	float large_damage=inelastic_scale*(InelasticDeltaKE *(1.0/4.0 + (0.5*m2/(m1+m2))) )/kilojoules_per_damage;
    float small_damage=inelastic_scale*(InelasticDeltaKE *(1.0/4.0 + (0.5*m1/(m1+m2))) )/kilojoules_per_damage;
	

#if 0	
	Vector smforce =(bignormal*.4*smalle->GetMass()*fabs(bignormal.Dot (((smalle->GetVelocity()-this->GetVelocity())/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
	Vector thisforce=(smallnormal*.4*GetMass()*fabs(smallnormal.Dot ((smalle->GetVelocity()-this->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
	float smag = smforce.Magnitude();
	float tmag= thisforce.Magnitude();
	if (smag>.000001)
		smforce = (smforce/smag)*(large_damage+small_damage)*INVERSEFORCEDISTANCE*bouncepercent;
	if (tmag>.000001)
		thisforce = (thisforce/tmag)*(large_damage+small_damage)*INVERSEFORCEDISTANCE*bouncepercent;
#endif
	Vector ThisDesiredVelocity = ThisElastic_vf*(1-inelastic_scale/2)+Inelastic_vf*inelastic_scale/2;
	Vector SmallerDesiredVelocity = SmallerElastic_vf*(1-inelastic_scale)+Inelastic_vf*inelastic_scale;
	Vector smforce = (SmallerDesiredVelocity-smalle->GetVelocity())*smalle->GetMass()/SIMULATION_ATOM;
	Vector thisforce = (ThisDesiredVelocity-GetVelocity())*GetMass()/SIMULATION_ATOM;
	
	
	//UniverseUtil::IOmessage(0,"game","all",string("damaging collision ")+XMLSupport::tostring(smforce.i)+string(",")+XMLSupport::tostring(smforce.j)+string(",")+XMLSupport::tostring(smforce.k)+string(" resultantkinetic ")+XMLSupport::tostring(FinalInelasticKE)+string(" resultant damages ")+XMLSupport::tostring(small_damage)+string(" ")+XMLSupport::tostring(large_damage)+string(" bouncepercent ")+XMLSupport::tostring(bouncepercent)); 

	if(thcp){
		if((getNewTime()-thcp->TimeOfLastCollision)>mintime){
			if(ThisDesiredVelocity.Magnitude()>minvel){
				thcp->TimeOfLastCollision=getNewTime();
			}
		}else{
			isnotplayerorhasbeenmintime=false;
		}
	}
    
	if(smcp){
		if((getNewTime()-smcp->TimeOfLastCollision)>mintime){
			if(SmallerDesiredVelocity.Magnitude()>minvel){
				smcp->TimeOfLastCollision=getNewTime();
			}
		}else{
			isnotplayerorhasbeenmintime=false;
		}
	}

	if((smalle->isUnit()!=MISSILEPTR)&&isnotplayerorhasbeenmintime){ 
	  smalle->ApplyForce (smforce);
	}
    if((this->isUnit()!=MISSILEPTR)&&isnotplayerorhasbeenmintime) {
	  this->ApplyForce (thisforce);
	}
/*    smalle->curr_physical_state = smalle->prev_physical_state;
	  this->curr_physical_state = this->prev_physical_state;*/
    smalle->ApplyDamage (biglocation.Cast(),bignormal,small_damage,smalle,GFXColor(1,1,1,2),NULL);
    this->ApplyDamage (smalllocation.Cast(),smallnormal,large_damage,this,GFXColor(1,1,1,2),NULL);

    //OLDE METHODE
    //    smalle->ApplyDamage (biglocation.Cast(),bignormal,.33*g_game.difficulty*(  .5*fabs((smalle->GetVelocity()-this->GetVelocity()).MagnitudeSquared())*this->mass*SIMULATION_ATOM),smalle,GFXColor(1,1,1,2),NULL);
    //    this->ApplyDamage (smalllocation.Cast(),smallnormal, .33*g_game.difficulty*(.5*fabs((smalle->GetVelocity()-this->GetVelocity()).MagnitudeSquared())*smalle->mass*SIMULATION_ATOM),this,GFXColor(1,1,1,2),NULL);

#endif
  //each mesh with each mesh? naw that should be in one way collide
  }
}

static Unit * getFuelUpgrade () {
  return UnitFactory::createUnit("add_fuel",true,FactionUtil::GetFaction("upgrades"));
}
static float getFuelAmt () {
  Unit * un = getFuelUpgrade();
  float ret = un->FuelData();
  un->Kill();
  return ret;
}
static float GetJumpFuelQuantity() {
  static float f= getFuelAmt();
  return f;
}

void Unit::ActivateJumpDrive (int destination) {
  //const int jumpfuelratio=1;
  if (((docked&(DOCKED|DOCKED_INSIDE))==0)&&jump.drive!=-2) {
	  /*if (1warpenergy>=jump.energy&&(jump.energy>=0)) {*/
	  jump.drive = destination;
	  //float fuel_used=0;
//    warpenergy-=jump.energy; //don't spend energy until later
	  /*}*/
  }
}

void Unit::DeactivateJumpDrive () {
  if (jump.drive>=0) {
    jump.drive=-1;
  }
}

float copysign (float x, float y) {
	if (y>0)
			return x;
	else
			return -x;
}

float rand01 () {
	return ((float)rand()/(float)RAND_MAX);
}
float capship_size=500;

/* UGLYNESS short fix */
unsigned int apply_float_to_unsigned_int (float tmp) {
  static unsigned long int seed = 2531011;
  seed +=214013;
  seed %=4294967295u;
  unsigned  int ans = (unsigned int) tmp;
  tmp -=ans;//now we have decimal;
  if (seed<(unsigned long int)(4294967295u*tmp))
    ans +=1;
  return ans;
}

std::string accelStarHandler (const XMLType &input,void *mythis) {
  static float game_speed = XMLSupport::parse_float (vs_config->getVariable ("physics","game_speed","1"));
  static float game_accel = XMLSupport::parse_float (vs_config->getVariable ("physics","game_accel","1"));
  return XMLSupport::tostring(*input.w.f/(game_speed*game_accel));
}
std::string speedStarHandler (const XMLType &input,void *mythis) {
  static float game_speed = XMLSupport::parse_float (vs_config->getVariable ("physics","game_speed","1"));
  return XMLSupport::tostring((*input.w.f)/game_speed);
}

static list<Unit*> Unitdeletequeue;
static Hashtable <long, Unit, 2095> deletedUn;
int deathofvs=1;
void CheckUnit(Unit * un) {
  if (deletedUn.Get ((long)un)!=NULL) {
    while (deathofvs) {
      printf ("%ld died",(long)un);
    }
  }
}
void UncheckUnit (Unit * un) {
  if (deletedUn.Get ((long)un)!=NULL) {
    deletedUn.Delete ((long)un);
  }  
}
string GetUnitDir(string filename) {
	return filename.substr(0,filename.find("."));
}
char * GetUnitDir (const char * filename) {
  char * retval=strdup (filename);
  if (retval[0]=='\0')
    return retval;
  if (retval[1]=='\0')
    return retval;
  for (int i=0;retval[i]!=0;i++) {
    if (retval[i]=='.') {
      retval[i]='\0';
      break;
    }
  }
  return retval;
}

// From weapon_xml.cpp
std::string lookupMountSize (int s) {
  std::string result;
  if (s&weapon_info::LIGHT) {
    result+="LIGHT ";
  }
  if (s&weapon_info::MEDIUM) {
    result+="MEDIUM ";
  }
  if (s&weapon_info::HEAVY) {
    result+="HEAVY ";
  }
  if (s&weapon_info::CAPSHIPLIGHT) {
    result+="CAPSHIP-LIGHT ";
  }
  if (s&weapon_info::CAPSHIPHEAVY) {
    result+="CAPSHIP-HEAVY ";
  }
  if (s&weapon_info::SPECIAL) {
    result+="SPECIAL ";
  }
  if (s&weapon_info::LIGHTMISSILE) {
    result+="LIGHT-MISSILE ";
  }
  if (s&weapon_info::MEDIUMMISSILE) {
    result+="MEDIUM-MISSILE ";
  }
  if (s&weapon_info::HEAVYMISSILE) {
    result+="HEAVY-MISSILE ";
  }
  if (s&weapon_info::CAPSHIPLIGHTMISSILE) {
    result+="LIGHT-CAPSHIP-MISSILE ";
  }
  if (s&weapon_info::CAPSHIPHEAVYMISSILE) {
    result+="HEAVY-CAPSHIP-MISSILE ";
  }
  if (s&weapon_info::SPECIALMISSILE) {
    result+="SPECIAL-MISSILE ";
  }
  if (s&weapon_info::AUTOTRACKING) {
    result+="AUTOTRACKING ";
  }
  return result;
}
/***********************************************************************************/
/**** UNIT STUFF                                                            */
/***********************************************************************************/
Unit::Unit( int /*dummy*/ ) {
  ZeroAll();
  image = new UnitImages;
  sound = new UnitSounds;
  aistate=NULL;
  image->cockpit_damage=NULL;
  //SetAI (new Order());
  Init();
}
Unit::Unit() {
  ZeroAll();
  image = new UnitImages;
  sound = new UnitSounds;
  aistate=NULL;
  image->cockpit_damage=NULL;
  //SetAI (new Order());
  Init();
}

Unit::Unit (std::vector <Mesh *> & meshes, bool SubU, int fact) {
  ZeroAll();
  image = new UnitImages;
  sound = new UnitSounds;
  aistate=NULL;
  image->cockpit_damage=NULL;
  //SetAI (new Order());
  Init();
  hull=1000;
  maxhull=100000;
  this->faction = fact;
  graphicOptions.SubUnit = SubU;
  meshdata = meshes;
  meshes.clear();
  meshdata.push_back(NULL);
  calculate_extent(false);
}

extern void update_ani_cache();
Unit::Unit(const char *filename, bool SubU, int faction,std::string unitModifications, Flightgroup *flightgrp,int fg_subnumber, string * netxml) {
  ZeroAll();
  image = new UnitImages;
  sound = new UnitSounds;
  aistate=NULL;
  image->cockpit_damage=NULL;
  //SetAI (new Order());
  Init( filename, SubU, faction, unitModifications, flightgrp, fg_subnumber, netxml); 
}

Unit::~Unit()
{
  free(image->cockpit_damage);
  if ((!killed)) {
    VSFileSystem::vs_fprintf (stderr,"Assumed exit on unit %s(if not quitting, report error)\n",name.c_str());
  }
  if (ucref) {
    VSFileSystem::vs_fprintf (stderr,"DISASTER AREA!!!!");
  }
#ifdef DESTRUCTDEBUG
  VSFileSystem::vs_fprintf (stderr,"stage %d %x %d\n", 0,this,ucref);
  fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
  VSFileSystem::vs_fprintf (stderr,"%d %x ", 1,planet);
  fflush (stderr);
  VSFileSystem::vs_fprintf (stderr,"%d %x\n", 2,image->hudImage);
  fflush (stderr);
#endif
  if (image->unitwriter)
    delete image->unitwriter;
  unsigned int i;
  for (i=0;i<image->destination.size();i++) {
    free (image->destination[i]);
  }

#ifdef DESTRUCTDEBUG
  VSFileSystem::vs_fprintf (stderr,"%d %x", 3,image);
  fflush (stderr);
#endif
  delete image;
  delete sound;
#ifdef DESTRUCTDEBUG
  VSFileSystem::vs_fprintf (stderr,"%d %x %x", 4,bspTree, bspShield);
  fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
  VSFileSystem::vs_fprintf (stderr,"%d", 5);
  fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
  VSFileSystem::vs_fprintf (stderr,"%d %x", 6,&mounts);
  fflush (stderr);
#endif

#ifdef DESTRUCTDEBUG
  VSFileSystem::vs_fprintf (stderr,"%d %x ", 9, halos);
  fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
  VSFileSystem::vs_fprintf (stderr,"%d %x ", 1,&mounts);
  fflush (stderr);
#endif
#ifndef NO_MOUNT_STAR
	for( vector<Mount *>::iterator jj=mounts.begin(); jj!=mounts.end(); jj++)
	{
		// Free all mounts elements
		if( (*jj)!=NULL)
			delete (*jj);
	}
#endif
	mounts.clear();
#ifdef DESTRUCTDEBUG
  VSFileSystem::vs_fprintf (stderr,"%d", 0);
  fflush (stderr);
#endif
  for(unsigned int meshcount = 0; meshcount < meshdata.size(); meshcount++)
    if (meshdata[meshcount])
      delete meshdata[meshcount];
  meshdata.clear();
}

void Unit::ZeroAll( )
{
    sound            = NULL;
    ucref            = 0;
    networked        = false;
    serial           = 0;
    net_accel.i      = 0;
    net_accel.j      = 0;
    net_accel.k      = 0;
    // old_state has a constructor
    damages          = NO_DAMAGE;
    // SubUnits has a constructor
    combat_role      = 0;
    nebula           = NULL;
    planet           = NULL;
    activeStarSystem = NULL;
    // computer has a constructor
    // jump needs fixing
    selected         = false;
    // scanner needs fixing
    xml              = NULL;
    owner            = NULL;
    // prev_physical_state has a constructor
    // curr_physical_state has a constructor
    // cumulative_transformation_matrix has a constructor
    // cumulative_transformation has a constructor
    cumulative_velocity.i = 0;
    cumulative_velocity.j = 0;
    cumulative_velocity.k = 0;
    NetForce.i            = 0;
    NetForce.j            = 0;
    NetForce.k            = 0;
    NetLocalForce.i       = 0;
    NetLocalForce.j       = 0;
    NetLocalForce.k       = 0;
    NetTorque.i           = 0;
    NetTorque.j           = 0;
    NetTorque.k           = 0;
    NetLocalTorque.i      = 0;
    NetLocalTorque.j      = 0;
    NetLocalTorque.k      = 0;
    AngularVelocity.i     = 0;
    AngularVelocity.j     = 0;
    AngularVelocity.k     = 0;
    Velocity.i            = 0;
    Velocity.j            = 0;
    Velocity.k            = 0;
    image                 = NULL;
    Mass                  = 0;
    shieldtight           = 0;
    fuel                  = 0;
    afterburnenergy       = 0;
    Momentofinertia       = 0;
    // limits has a constructor
    cloaking              = 0;
    cloakmin              = 0;
    radial_size           = 0;
    killed                = false;
    invisible             = 0;
    corner_min.i          = 0;
    corner_min.j          = 0;
    corner_min.k          = 0;
    corner_max.i          = 0;
    corner_max.j          = 0;
    corner_max.k          = 0;
    resolveforces         = false;
    // armor has a constructor
    // shield has a constructor
    hull                  = 0;
    maxhull               = 0;
    recharge              = 0;
    maxenergy             = 0;
    energy                = 0;
    maxwarpenergy         = 0;
    warpenergy            = 0;
    // target_fgid has a constructor
    aistate               = NULL;
    // CollideInfo has a constructor
    colTrees              = NULL;
    docked                = NOT_DOCKED;
    faction               = 0;
    flightgroup           = NULL;
    flightgroup_subnumber = 0;
}

void Unit::Init()
{
  /*
  static std::map <Unit *, bool> m;
  if (m[this]) {
    VSFileSystem::vs_fprintf (stderr,"already called this");
  }else {
    m[this]=1;
    }*/
	if( Network==NULL)
		this->networked=0;
	else
		this->networked=1;

	damages = NO_DAMAGE;

	graphicOptions.RecurseIntoSubUnitsOnCollision=false;
	this->combat_role=ROLES::getRole("INERT");
	this->computer.combat_mode=true;
#ifdef CONTAINER_DEBUG
  UncheckUnit (this);
#endif
  static float capsize = XMLSupport::parse_float(vs_config->getVariable("physics","capship_size","500"));

  capship_size=capsize;
  activeStarSystem=NULL;
  xml=NULL;
  docked=NOT_DOCKED;
  graphicOptions.SubUnit =0;
  jump.energy = 100;
  static float insys_jump_cost = XMLSupport::parse_float (vs_config->getVariable ("physics","insystem_jump_cost",".1"));
  jump.insysenergy=insys_jump_cost*jump.energy;
  jump.delay=5;
  jump.damage=0;
  jump.warpDriveRating=0;
  graphicOptions.FaceCamera=false;
  jump.drive=-2;// disabled
  afterburnenergy=0;
  planet=NULL;
  nebula=NULL;
  limits.structurelimits=Vector(0,0,1);
  limits.limitmin=-1;
  cloaking=-1;
  image->repair_droid=0;
  image->ecm=0;
  image->cloakglass=false;
  image->cargo_volume=0;
  image->unitwriter=NULL;
  cloakmin=image->cloakglass?1:0;
  image->equipment_volume=0;
  image->cloakrate=100;
  image->cloakenergy=0;
  image->forcejump=false;
  sound->engine=-1;  sound->armor=-1;  sound->shield=-1;  sound->hull=-1; sound->explode=-1;
  image->fireControlFunctionality=1.0f;
  image->fireControlFunctionalityMax=1.0f;
  image->SPECDriveFunctionality=1.0f;
  image->SPECDriveFunctionalityMax=1.0f;
  image->CommFunctionality=1.0f;
  image->CommFunctionalityMax=1.0f;
  image->LifeSupportFunctionality=1.0f;
  image->LifeSupportFunctionalityMax=1.0f;

  sound->cloak=-1;
  sound->jump=-1;
  image->hudImage=NULL;
  owner = NULL;
  faction =0;
  resolveforces=true;
  colTrees=NULL;
  invisible=DEFAULTVIS;
  //origin.Set(0,0,0);
  corner_min.Set (FLT_MAX,FLT_MAX,FLT_MAX);
  corner_max.Set (-FLT_MAX,-FLT_MAX,-FLT_MAX);
  
  shieldtight=0;//sphere mesh by default
  energy=maxenergy=1;
  warpenergy=0;
  maxwarpenergy=0;
  recharge = 1;
  shield.recharge=shield.leak=0;
  shield.shield2fb.front=shield.shield2fb.back=shield.shield2fb.frontmax=shield.shield2fb.backmax=armor.frontrighttop=armor.backrighttop=armor.frontlefttop=armor.backlefttop=armor.frontrightbottom=armor.backrightbottom=armor.frontleftbottom=armor.backleftbottom=0;
  hull=10;
  maxhull=10;
  shield.number=2;
  
  image->explosion=NULL;
  image->timeexplode=0;
  killed=false;
  ucref=0;
  aistate = NULL;
  Identity(cumulative_transformation_matrix);
  cumulative_transformation = identity_transformation;
  curr_physical_state = prev_physical_state = identity_transformation;
  Mass = .01;
  fuel = 000;

  static Vector myang(XMLSupport::parse_float (vs_config->getVariable ("general","pitch","0")),XMLSupport::parse_float (vs_config->getVariable ("general","yaw","0")),XMLSupport::parse_float (vs_config->getVariable ("general","roll","0")));
  static float rr = XMLSupport::parse_float (vs_config->getVariable ("graphics","hud","radarRange","20000"));
  static float minTrackingNum = XMLSupport::parse_float (vs_config->getVariable("physics",
										  "autotracking",
										".93"));// DO NOT CHANGE see unit_customize.cpp
    
  static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));// DO NOT CHANGE see unit_customize.cpp

  Momentofinertia = .01;
  AngularVelocity = myang;
  cumulative_velocity=Velocity = Vector(0,0,0);
  
  NetTorque =NetLocalTorque = Vector(0,0,0);
  NetForce = Vector(0,0,0);
  NetLocalForce=Vector(0,0,0);

  selected = false;
  //  image->selectionBox = NULL;

  limits.yaw = 2.55;
  limits.pitch = 2.55;
  limits.roll = 2.55;
	
  limits.lateral = 2;
  limits.vertical = 8;
  limits.forward = 2;
  limits.afterburn=5;
  limits.retro=2;
  VelocityReference(NULL);
  computer.threat.SetUnit (NULL);
  computer.threatlevel=0;
  computer.slide_start=computer.slide_end=0;
  computer.set_speed=0;
  computer.max_combat_speed=1;
  computer.max_combat_ab_speed=1;
  computer.max_yaw=1;
  computer.max_pitch=1;
  computer.max_roll=1;
  computer.NavPoint=Vector(0,0,0);
  computer.itts = false;
  computer.radar.maxrange=rr;
  computer.radar.locked=false;
  computer.radar.maxcone=-1;
  computer.radar.trackingcone = minTrackingNum;
  computer.radar.lockcone=lc;
  computer.radar.mintargetsize=0;
  computer.radar.color=true;

  flightgroup=NULL;
  flightgroup_subnumber=0;

  scanner.last_scantime=0.0;
  // No cockpit reference here
  if (!image->cockpit_damage) {
    int numg= (1+MAXVDUS+UnitImages::NUMGAUGES)*2;
    image->cockpit_damage=(float*)malloc((numg)*sizeof(float));
    for (unsigned int damageiterator=0;damageiterator<numg;damageiterator++) {
      image->cockpit_damage[damageiterator]=1;
    }
  }
  CollideInfo.object.u = NULL;
  CollideInfo.type = LineCollide::UNIT;
  CollideInfo.Mini.Set (0,0,0);
  CollideInfo.Maxi.Set (0,0,0);

  /*
  yprrestricted=0;
  ymin = pmin = rmin = -PI;
  ymax = pmax = rmax = PI;
  ycur = pcur = rcur = 0;
  */
  // Not needed here
  //static Vector myang(XMLSupport::parse_float (vs_config->getVariable ("general","pitch","0")),XMLSupport::parse_float (vs_config->getVariable ("general","yaw","0")),XMLSupport::parse_float (vs_config->getVariable ("general","roll","0")));
  // Not needed here
  //static float rr = XMLSupport::parse_float (vs_config->getVariable ("graphics","hud","radarRange","20000"));
  // Not needed here
  /*
  static float minTrackingNum = XMLSupport::parse_float (vs_config->getVariable("physics",
										  "autotracking",
										".93"));// DO NOT CHANGE see unit_customize.cpp
  */
  // Not needed here
  //static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));// DO NOT CHANGE see unit_customize.cpp
  //  Fire();

}

std::string getMasterPartListUnitName();
using namespace VSFileSystem;
extern std::string GetReadPlayerSaveGame (int);
CSVRow GetUnitRow(string filename, bool subu, int faction, bool readLast, bool &read);
#if 0
static std::string csvUnit(std::string un) {
  string::size_type i=un.find_last_of(".");
  string::size_type del=un.find_last_of("/\\:");
  if (i==std::string::npos) {
    return un+".csv";
  }
  if (del==std::string::npos||del<i) {
    return un.substr(0,i)+".csv";
  }
  return un+".csv";
}
#endif
void Unit::Init(const char *filename, bool SubU, int faction,std::string unitModifications, Flightgroup *flightgrp,int fg_subnumber, string * netxml)
{
        static bool UNITTAB = XMLSupport::parse_bool(vs_config->getVariable("physics","UnitTable","false"));

	this->Unit::Init();
	//if (!SubU)
	//  _Universe->AccessCockpit()->savegame->AddUnitToSave(filename,UNITPTR,FactionUtil::GetFaction(faction),(long)this);
	graphicOptions.SubUnit = SubU?1:0;
	graphicOptions.Animating=1;
	graphicOptions.RecurseIntoSubUnitsOnCollision=!isSubUnit();
	this->faction = faction;
	SetFg (flightgrp,fg_subnumber);
	VSFile f;
        VSFile f2;
	VSError err = Unspecified;
        VSFile unitTab;
        VSError taberr= Unspecified;;
        bool foundFile=false;
  if( netxml==NULL)
  {
		if (unitModifications.length()!=0) {
		  string nonautosave=GetReadPlayerSaveGame(_Universe->CurrentCockpit());
		  string filepath("");
		  // In network mode we only look in the save subdir in HOME
		  if( Network==NULL && !SERVER)
		  {
			  if (nonautosave.empty()) {
				  VSFileSystem::CreateDirectoryHome (VSFileSystem::savedunitpath+"/"+unitModifications);
				  filepath = unitModifications+"/"+string(filename);
			  }else {
				  VSFileSystem::CreateDirectoryHome (VSFileSystem::savedunitpath+"/"+nonautosave);
				  filepath = nonautosave+"/"+string(filename);
			  }
		  }
		  // This is not necessary as I think... to watch
		  //VSFileSystem::vs_chdir( "save");

		  // Try to open save
		  if (filename[0]) {
                    taberr=unitTab.OpenReadOnly( filepath+".csv", UnitSaveFile);
                    if (taberr<=Ok) {
                      unitTables.push_back(new CSVTable(unitTab));
                      unitTab.Close();
                    }
                    if (!UNITTAB) 
                      err = f.OpenReadOnly( filepath, UnitSaveFile);
                    
                  }
		}
	}
  // If save was not succesfull we try to open the unit file itself
  if( netxml==NULL)
  {
      if (filename[0]) {
	    string subdir = "factions/"+FactionUtil::GetFactionName(faction);
           // begin deprecated code (5/11)            
            if (UNITTAB) {

            }else {
		if( err>Ok) {
	  	    f.SetSubDirectory(subdir);
			// No save found loading default unit
			err = f.OpenReadOnly (filename, UnitFile);
			if (err>Ok) {
				f.SetSubDirectory("");
				err = f.OpenReadOnly (filename, UnitFile);
			}
		}else{
			f2.SetSubDirectory(subdir);
			// Save found so just opening default unit to get its directory for further loading
			err = f2.OpenReadOnly (filename, UnitFile);
			if (err>Ok) {
				f2.SetSubDirectory("");
				err = f2.OpenReadOnly (filename, UnitFile);
			}
		}
                
            }
                //end deprecated code
	  }
  }
  CSVRow unitRow;
  if (UNITTAB) {
    unitRow = GetUnitRow(filename,SubU,faction,true, foundFile);
  }else {
    foundFile = (err<=Ok);
  }
  if(!foundFile) {
	cout << "Unit file " << filename << " not found" << endl;
	fprintf (stderr,"Assertion failed unit_generic.cpp:711 Unit %s not found\n",filename);

    VSFileSystem::vs_fprintf (stderr,"Warning: Cannot locate %s\n",filename);	  
    meshdata.clear();
    meshdata.push_back(NULL);
	this->fullname=filename;
    this->name=string("LOAD_FAILED");
	calculate_extent(false);		
	radial_size=1;
    //	    assert ("Unit Not Found"==NULL);
	//assert(0);
        if (taberr<=Ok&&taberr!=Unspecified) {
          delete unitTables.back();
          unitTables.pop_back();
        }
	return;
  }

	name = filename;
        bool tmpbool;
        if (UNITTAB) {
              // load from table?

          // we have to set the root directory to where the saved unit would have come from.
          // saved only exists if taberr<=Ok && taberr!=Unspecified...that's why we pass in said boolean
              VSFileSystem::current_path.push_back(taberr<=Ok&&taberr!=Unspecified?GetUnitRow(filename,SubU,faction,false,tmpbool).getRoot():unitRow.getRoot());
              VSFileSystem::current_subdirectory.push_back("/"+unitRow["Directory"]);
              VSFileSystem::current_type.push_back(UnitFile);
              LoadRow(unitRow,unitModifications,netxml);
              VSFileSystem::current_type.pop_back();
              VSFileSystem::current_subdirectory.pop_back();
              VSFileSystem::current_path.pop_back();
              if (taberr<=Ok&&taberr!=Unspecified) {
                delete unitTables.back();
                unitTables.pop_back();
              }

        }else {
          if( netxml==NULL)
            Unit::LoadXML(f,unitModifications.c_str());
          else
            Unit::LoadXML( f, "", netxml);
          if( err<=Ok)
            f.Close();
          if( f2.Valid())
            f2.Close();
        }
        calculate_extent(false);
///	  ToggleWeapon(true);//change missiles to only fire 1
}

vector <Mesh *> Unit::StealMeshes() {
  vector <Mesh *>ret;
  
  Mesh * shield = meshdata.empty()?NULL:meshdata.back();
  for (int i=0;i<nummesh();i++) {
    ret.push_back (meshdata[i]);
  }
  meshdata.clear();
  meshdata.push_back(shield);
  
  return ret;
}
static float tmpmax (float a , float b) {
	return a>b?a:b;
}
void Unit::calculate_extent(bool update_collide_queue) {  
  int a;
  corner_min=Vector (FLT_MAX,FLT_MAX,FLT_MAX);
  corner_max=Vector (-FLT_MAX,-FLT_MAX,-FLT_MAX);

  for(a=0; a<nummesh(); a++) {
    corner_min = corner_min.Min(meshdata[a]->corner_min());
    corner_max = corner_max.Max(meshdata[a]->corner_max());
  }/* have subunits now in table*/
  un_kiter iter =SubUnits.constIterator();
  const Unit * un;
  while ((un = iter.current())) {
    corner_min = corner_min.Min(un->LocalPosition().Cast()+un->corner_min);
    corner_max = corner_max.Max(un->LocalPosition().Cast()+un->corner_max);
    iter.advance();
  }

  if (corner_min.i==FLT_MAX||corner_max.i==-FLT_MAX||!FINITE (corner_min.i)||!FINITE(corner_max.i)) {
    radial_size=0;
    corner_min.Set (0,0,0);
    corner_max.Set (0,0,0);
  }else {
    float tmp1 = corner_min.Magnitude();
    float tmp2 = corner_max.Magnitude();
    radial_size = tmp1>tmp2?tmp1:tmp2;
    //    if (!SubUnit)
    //      image->selectionBox = new Box(corner_min, corner_max);
  }
  if (!isSubUnit()&&update_collide_queue) {
    UpdateCollideQueue();
  }
  if (isUnit()==PLANETPTR) {
    radial_size = tmpmax(tmpmax(corner_max.i,corner_max.j),corner_max.k) ;
  }
}

void Unit::scanSystem(){

  double nowtime=mission->getGametime();

  if(scanner.last_scantime==nowtime){
    return;
  }

    StarSystem *ssystem=_Universe->activeStarSystem();
    un_iter uiter(ssystem->getUnitList().createIterator());
    
    float min_enemy_dist=9999999.0;
    float min_friend_dist=9999999.0;
    float min_ship_dist=9999999.0;
    Unit * min_enemy=NULL;
    Unit * min_friend=NULL;
    Unit * min_ship=NULL;
    
    int leader_num=getFgSubnumber(); //my own subnumber
    Unit *my_leader=this; // say I'm the leader
    Flightgroup *my_fg=getFlightgroup();
    
    Unit *unit=uiter.current();
    while(unit!=NULL){
      
      if(this!=unit){
	// won;t scan ourselves
	
	QVector unit_pos=unit->Position();
	double dist=getMinDis(unit_pos);
	float relation=getRelation(unit);
	
	if(relation<0.0){
	  //we are enmies
	  if(dist<min_enemy_dist){
	    min_enemy_dist=dist;
	    min_enemy=unit;
	  }
	}
	if(relation>0.0){
	//we are friends
	  if(dist<min_friend_dist){
	    min_friend_dist=dist;
	  min_friend=unit;
	  }
	  // check for flightgroup leader
	  if(my_fg!=NULL && my_fg==unit->getFlightgroup()){
	    // it's a ship from our flightgroup
	    int fgnum=unit->getFgSubnumber();
	    if(fgnum<leader_num){
	      //set this to be our leader
	      my_leader=unit;
	      leader_num=fgnum;
	    }
	  }
	}
	// for all ships
	if(dist<min_ship_dist){
	  min_ship_dist=dist;
	  min_ship=unit;
	}
      }
      
      unit=++(uiter);
  }
    
    scanner.nearest_enemy_dist=min_enemy_dist;
    scanner.nearest_enemy=min_enemy;
    
    scanner.nearest_friend_dist=min_friend_dist;
    scanner.nearest_friend=min_friend;

    scanner.nearest_ship_dist=min_ship_dist;
    scanner.nearest_ship=min_ship;
    
    scanner.leader=my_leader;
}

StarSystem * Unit::getStarSystem () {

  if (activeStarSystem) {
    return activeStarSystem;
  }else {
    Cockpit * cp=_Universe->isPlayerStarship(this);
    if (cp) {
      if (cp->activeStarSystem)
	return cp->activeStarSystem;
    }
  }
  return _Universe->activeStarSystem();
}

void Unit::Fire (unsigned int weapon_type_bitmask, bool listen_to_owner) {
    if (cloaking>=0||graphicOptions.InWarp)
        return;
	int nm = 0;
	vector <Mount>
		::iterator i = mounts.begin();//note to self: if vector<Mount *> is ever changed to vector<Mount> remove the const_ from the const_iterator
    for (;i!=mounts.end();++i, nm++) {
			if ((*i).type->type==weapon_info::BEAM) {
				if ((*i).type->EnergyRate*SIMULATION_ATOM>energy) {
					// On server side send a PACKET TO ALL CLIENT TO NOTIFY UNFIRE
					// Including the one who fires to make sure it stops
					if( SERVER)
						VSServer->BroadcastUnfire( this->serial, nm, this->activeStarSystem->GetZone());
					// NOT ONLY IN non-networking mode : anyway, the server will tell everyone including us to stop if not already done
					// if( !SERVER && Network==NULL)
						(*i).UnFire();
					continue;
				}
			}else{
				// Only in non-networking mode
				if ( Network==NULL && (*i).type->EnergyRate>energy)
					continue;
			}

			const bool mis = (*i).type->type==weapon_info::PROJECTILE;
			const bool locked_on = (*i).time_to_lock<=0;
			const bool lockable_weapon = (*i).type->LockTime>0;
			const bool autotracking_gun =(!mis)&&0!=((*i).size&weapon_info::AUTOTRACKING)&&locked_on;
			const bool fire_non_autotrackers = (0==(weapon_type_bitmask&ROLES::FIRE_ONLY_AUTOTRACKERS));
			const bool locked_missile = (mis&&locked_on&&lockable_weapon);
			const bool missile_and_want_to_fire_missiles = (mis&&(weapon_type_bitmask&ROLES::FIRE_MISSILES));
			const bool gun_and_want_to_fire_guns =((!mis)&&(weapon_type_bitmask&ROLES::FIRE_GUNS));
			if(missile_and_want_to_fire_missiles&&locked_missile){
				VSFileSystem::vs_fprintf (stderr,"\n about to fire locked missile \n");
			}
			if (fire_non_autotrackers||autotracking_gun||locked_missile) {
				if ((ROLES::EVERYTHING_ELSE&weapon_type_bitmask&(*i).type->role_bits)
					||(*i).type->role_bits==0) {
					if ((locked_on&&missile_and_want_to_fire_missiles)
						||gun_and_want_to_fire_guns) {
						// If in non-networking mode and mount fire has been accepted or if on server side
						if( Network==NULL || SERVER || (*i).processed==Mount::ACCEPTED)
						{
							// If we are on server or if the weapon has been accepted for fire we fire
							if ((*i).Fire(owner==NULL?this:owner,mis,listen_to_owner)) {
								ObjSerial serid;
								if( missile_and_want_to_fire_missiles)
								{
									serid = getUniqueSerial();
									(*i).serial = serid;
								}
								else
									serid = 0;
								if( SERVER)
									VSServer->BroadcastFire( this->serial, nm, serid, this->activeStarSystem->GetZone());
								// We could only refresh energy on server side or in non-networking mode, on client side it is done with
								// info the server sends with ack for fire
								// FOR NOW WE TRUST THE CLIENT SINCE THE SERVER CAN REFUSE A FIRE
								// if( Network==NULL || SERVER)
								if ((*i).type->type==weapon_info::BEAM) {
									if ((*i).ref.gun)
										if ((!(*i).ref.gun->Dissolved())||(*i).ref.gun->Ready()) {
											energy -=(*i).type->EnergyRate*SIMULATION_ATOM;
										}
									
								}else{
									energy-=(*i).type->EnergyRate;
								}
								// IF WE REFRESH ENERGY FROM SERVER : Think to send the energy update to the firing client with ACK TO fireRequest
								if (mis) weapon_type_bitmask &= (~ROLES::FIRE_MISSILES);//fire only 1 missile at a time
							}
						}
						else
						{
							// Request a fire order to the server telling him the serial of the unit and the mount index (nm)
							char mis2 = mis;
							int playernum = _Universe->whichPlayerStarship( this);
							if( playernum>=0)
								Network[playernum].fireRequest( this->serial, nm, mis2);
							// Mark the mount as fire requested
							//(*i).processed = Mount::REQUESTED;
						}
					}
				}
			}
    }

}
bool CheckAccessory (Unit * tur) {
  bool accessory = tur->name.find ("accessory")!=string::npos;
  if (accessory) {
    tur->SetAngularVelocity(tur->DownCoordinateLevel(Vector (tur->GetComputerData().max_pitch,
							   tur->GetComputerData().max_yaw,
							   tur->GetComputerData().max_roll)));
  }
  return accessory;
}

const string Unit::getFgID()  {
    if(flightgroup!=NULL){
      char buffer[200];
      sprintf(buffer,"-%d",flightgroup_subnumber);
      return flightgroup->name+buffer;
    }
    else{
      return fullname;
    }
};

void Unit::SetFaction (int faction) {
  this->faction=faction;
  for (un_iter ui=getSubUnits();(*ui)!=NULL;++ui) {
    (*ui)->SetFaction(faction);
  }
}

//FIXME Daughter units should be able to be turrets (have y/p/r)
void Unit::SetResolveForces (bool ys) {
  resolveforces = ys;
  /*
  for (int i=0;i<numsubunit;i++) {
    subunits[i]->SetResolveForces (ys);
  }
  */
}

void Unit::SetFg(Flightgroup * fg, int fg_subnumber) {
  flightgroup=fg;
  flightgroup_subnumber=fg_subnumber;
}

void Unit::AddDestination (const char * dest) {
  image->destination.push_back (strdup (dest));
}

const std::vector <char *>& Unit::GetDestinations () const{
  return image->destination;
}

float Unit::TrackingGuns(bool &missilelock) {
  float trackingcone = 0;
  missilelock=false;
  for (int i=0;i<GetNumMounts();i++) {
	  if (mounts[i].status==Mount::ACTIVE&&(mounts[i].size&weapon_info::AUTOTRACKING)) {
      trackingcone= computer.radar.trackingcone;
    }
    if (mounts[i].status==Mount::ACTIVE&&mounts[i].type->LockTime>0&&mounts[i].time_to_lock<=0) {
      missilelock=true;
    }
  }
  return trackingcone;
}

void Unit::getAverageGunSpeed(float & speed, float &grange, float &mrange) const {
  mrange=-1;
  grange=-1;
  speed=-1;
  bool beam=true;
  if (GetNumMounts()) {
    grange=0;
    speed=0;
	mrange=0;
	int nummt = 0;
	// this breaks the name, but... it _is_ more useful.
    for (int i=0;i<GetNumMounts();i++) {
	 if (mounts[i].status==Mount::ACTIVE||mounts[i].status==Mount::INACTIVE) {
      if (mounts[i].type->type!=weapon_info::PROJECTILE) {
	    if (mounts[i].type->Range > grange) {
	      grange=mounts[i].type->Range;
		}
	  
		if (mounts[i].status==Mount::ACTIVE) {
			speed+=mounts[i].type->Speed;
			nummt++;
			beam&= (mounts[i].type->type==weapon_info::BEAM);		
		}
	  }
	  else if(mounts[i].type->type==weapon_info::PROJECTILE){
		  if(mounts[i].type->Range > mrange) {
			  mrange=mounts[i].type->Range;
		  }
	  }
	 }
	}
	if(nummt){
		if (beam)
			speed=FLT_MAX;
		else
			speed = speed/nummt;
	  
	}
  }
}

QVector Unit::PositionITTS (const QVector& absposit,Vector velocity, float speed, bool steady_itts) const{
	if (speed==FLT_MAX)
		return this->Position();
	float difficultyscale=1;
	if (g_game.difficulty<.99)
		GetVelocityDifficultyMult(difficultyscale);
	velocity = (cumulative_velocity.Scale(difficultyscale)-velocity);
	QVector posit (this->Position()-absposit);
	QVector curguess(posit);
	for (unsigned int i=0;i<3;++i) {
		float time = 0;
		if(speed>0.001){
			time = curguess.Magnitude()/speed;
		}	 
		if (steady_itts) {
			curguess = posit+GetVelocity().Cast().Scale(time); // ** jay
		}
		else {
			curguess = posit+velocity.Scale(time).Cast();
		}
	}
	return curguess+absposit;
}
static float tmpsqr (float x) {
	return x*x;
}
float CloseEnoughCone (Unit * me) {
	static float close_autotrack_cone = XMLSupport::parse_float (vs_config->getVariable ("physics","near_autotrack_cone",".9"));
	return close_autotrack_cone;
}
bool CloseEnoughToAutotrack (Unit * me, Unit * targ, float &cone) {
	if (targ) {
		static float close_enough_to_autotrack = tmpsqr(XMLSupport::parse_float (vs_config->getVariable ("physics","close_enough_to_autotrack","4")));
		float dissqr = (me->curr_physical_state.position.Cast()-targ->curr_physical_state.position.Cast()).MagnitudeSquared();
		float movesqr=close_enough_to_autotrack*(me->prev_physical_state.position.Cast()-me->curr_physical_state.position.Cast()).MagnitudeSquared();
		if (dissqr<movesqr&&movesqr>0) {
			cone = CloseEnoughCone(me)*(movesqr-dissqr)/movesqr + 1*dissqr/movesqr;
			return true;
		}
	}
	return false;
}

float Unit::cosAngleTo (Unit * targ, float &dist, float speed, float range) const{
  Vector Normal (cumulative_transformation_matrix.getR());
   //   if (range!=FLT_MAX) {
   //     getAverageGunSpeed(speed,range);
   //   }
   QVector totarget (targ->PositionITTS(cumulative_transformation.position,cumulative_velocity, speed,false));
   totarget = totarget-cumulative_transformation.position;
   double tmpcos = Normal.Cast().Dot (totarget);
   dist = totarget.Magnitude();
   if (tmpcos>0) {
      tmpcos = dist*dist - tmpcos*tmpcos;
	  if( tmpcos >0)
    	  tmpcos = targ->rSize()/sqrt( tmpcos);//one over distance perpendicular away from straight ahead times the size...high is good WARNING POTENTIAL DIV/0
	  else
		  tmpcos = 1;
   } else {
     tmpcos /= dist;
   }
   float rsize = targ->rSize()+rSize();
   if ((!targ->GetDestinations().empty()&&jump.drive>=0)||(targ->faction==faction)) {
     rsize=0;//HACK so missions work well
   }
   dist = (dist-rsize)/range;//WARNING POTENTIAL DIV/0
   if (!FINITE(dist)||dist<0) {
     dist=0;
   }
   return tmpcos;
}

float Unit::cosAngleFromMountTo (Unit * targ, float & dist) const{
  float retval = -1;
  dist = FLT_MAX;
  float tmpcos;
  Matrix mat;
  for (int i=0;i<GetNumMounts();i++) {
    float tmpdist = .001;
    Transformation finaltrans (mounts[i].GetMountOrientation(),mounts[i].GetMountLocation().Cast());
    finaltrans.Compose (cumulative_transformation, cumulative_transformation_matrix);
    finaltrans.to_matrix (mat);
    Vector Normal (mat.getR());
    
    QVector totarget (targ->PositionITTS(finaltrans.position,cumulative_velocity, mounts[i].type->Speed,false));
    
    tmpcos = Normal.Dot (totarget.Cast());
    tmpdist = totarget.Magnitude();
    if (tmpcos>0) {
      tmpcos = tmpdist*tmpdist - tmpcos*tmpcos;
      tmpcos = targ->rSize()/tmpcos;//one over distance perpendicular away from straight ahead times the size...high is good WARNING POTENTIAL DIV/0
    } else {
      tmpcos /= tmpdist;
    }
    tmpdist /= mounts[i].type->Range;//UNLIKELY DIV/0
    if (tmpdist < 1||tmpdist<dist) {
      if (tmpcos-tmpdist/2 > retval-dist/2) {
	dist = tmpdist;
	retval = tmpcos;
      }      
    }
  }
  return retval;
}

#define PARANOIA .4
void Unit::Threaten (Unit * targ, float danger) {
  if (!targ) {
    computer.threatlevel=danger;
    computer.threat.SetUnit (NULL);
  }else {
    if (targ->owner!=this&&this->owner!=targ&&danger>PARANOIA&&danger>computer.threatlevel) {
      computer.threat.SetUnit(targ);
      computer.threatlevel = danger;
    }
  }
}

std::string Unit::getCockpit () const{
	return image->cockpitImage;
}

void Unit::Select() {
  selected = true;
}
void Unit::Deselect() {
  selected = false;
}
void disableSubUnits (Unit * uhn) {
	Unit * un;
	for (un_iter i = uhn->getSubUnits();(un=*i)!=NULL;++i) {
		disableSubUnits(un);
		for (unsigned int j=0;j<uhn->mounts.size();++j) {
			uhn->mounts[j].status=Mount::DESTROYED;
		}
		uhn->ClearMounts();
		
	}
}
un_iter Unit::getSubUnits () {
  return SubUnits.createIterator();
}
un_kiter Unit::viewSubUnits() const{
  return SubUnits.constIterator();
}

void Unit::SetVisible(bool vis) {
	if (vis) {
		invisible&=(~INVISCAMERA);
	}else {
		invisible|=INVISCAMERA;
	}
}
void Unit::SetAllVisible(bool vis) {
	if (vis) {
		invisible&=(~INVISUNIT);
	}else {
		invisible|=INVISUNIT;
	}
}
void Unit::SetGlowVisible(bool vis) {
	if (vis) {
		invisible&=(~INVISGLOW);
	}else {
		invisible|=INVISGLOW;
	}
}

float Unit::GetElasticity() {return .5;}

/***********************************************************************************/
/**** UNIT AI STUFF                                                                */
/***********************************************************************************/
void Unit::LoadAIScript(const std::string & s) {
  //static bool init=false;
  //  static bool initsuccess= initPythonAI();
  if (s.find (".py")!=string::npos) {
    Order * ai = PythonClass <FireAt>::Factory (s);
    PrimeOrders (ai);
    return;
  }else {
    if (s.length()>0) {
      if (*s.begin()=='_') {
	mission->addModule (s.substr (1));
	PrimeOrders (new AImissionScript (s.substr(1)));
      }else {
	if (s=="ikarus") {
	  PrimeOrders( new Orders::Ikarus ());
	}else {
	  string ai_agg=s+".agg.xml";
	  string ai_int=s+".int.xml";
	  PrimeOrders( new Orders::AggressiveAI (ai_agg.c_str(), ai_int.c_str()));
	}
      }
    }else {
      PrimeOrders();
    }
  }
}
void Unit::eraseOrderType (unsigned int type) {
	if (aistate) {
		aistate->eraseType(type);
	}
}
bool Unit::LoadLastPythonAIScript() {
  Order * pyai = PythonClass <Orders::FireAt>::LastPythonClass();
  if (pyai) {
    PrimeOrders (pyai);
  }else if (!aistate) {
    PrimeOrders();
    return false;
  }
  return true;
}
bool Unit::EnqueueLastPythonAIScript() {
  Order * pyai = PythonClass <Orders::FireAt>::LastPythonClass();
  if (pyai) {
    EnqueueAI (pyai);
  }else if (!aistate) {
    return false;
  }
  return true;
}

void Unit::PrimeOrders (Order * newAI) {
  if (newAI) {
    if (aistate) {
      aistate->Destroy();
    }
    aistate = newAI;
    newAI->SetParent (this);
  }else {
    PrimeOrders();
  }
}
void Unit::PrimeOrders () {
  if (aistate) {
    aistate->Destroy();
    aistate=NULL;
  }
  aistate = new Order; //get 'er ready for enqueueing
  aistate->SetParent (this);
}

void Unit::SetAI(Order *newAI)
{
  newAI->SetParent(this);
  if (aistate) {
    aistate->ReplaceOrder (newAI);
  }else {
    aistate = newAI;
  }
}
void Unit::EnqueueAI(Order *newAI) {
  newAI->SetParent(this);
  if (aistate) {
    aistate->EnqueueOrder (newAI);
  }else {
    aistate = newAI;
  }
}
void Unit::EnqueueAIFirst(Order *newAI) {
  newAI->SetParent(this);
  if (aistate) {
    aistate->EnqueueOrderFirst (newAI);
  }else {
    aistate = newAI;
  }
}
void Unit::ExecuteAI() {
  if (flightgroup) {
      Unit * leader = flightgroup->leader.GetUnit();
      if (leader?(flightgroup->leader_decision>-1)&&(leader->getFgSubnumber()>=getFgSubnumber()):true) {//no heirarchy in flight group
	if (!leader) {
	  flightgroup->leader_decision = flightgroup->nr_ships;
	}
	flightgroup->leader.SetUnit(this);
      }
      flightgroup->leader_decision--;
  
  }
  if(aistate) aistate->Execute();
  if (!SubUnits.empty()) {
    un_iter iter =getSubUnits();
    Unit * un;
    while ((un = iter.current())) {
      un->ExecuteAI();//like dubya
      iter.advance();
    }
  }
}

string Unit::getFullAIDescription(){
  if (getAIState()) {
    return getFgID()+":"+getAIState()->createFullOrderDescription(0).c_str();
  }else {
    return "no order";
  }
}

float Unit::getRelation (Unit * targ) {
  if (aistate) {
    return aistate->GetEffectiveRelationship (targ);
  }else {
    return FactionUtil::GetIntRelation (faction,targ->faction);
  }
}

void Unit::setTargetFg(string primary,string secondary,string tertiary){
  target_fgid[0]=primary;
  target_fgid[1]=secondary;
  target_fgid[2]=tertiary;

  ReTargetFg(0);
}

void Unit::ReTargetFg(int which_target){
#if 0
      StarSystem *ssystem=_Universe->activeStarSystem();
      UnitCollection *unitlist=ssystem->getUnitList();
      Iterator uiter=unitlist->createIterator();

      GameUnit *other_unit=uiter.current();
      GameUnit *found_target=NULL;
      int found_attackers=1000;

      while(other_unit!=NULL){
	string other_fgid=other_unit->getFgID();
	if(other_unit->matchesFg(target_fgid[which_target])){
	  // the other unit matches our primary target

	  int num_attackers=other_unit->getNumAttackers();
	  if(num_attackers<found_attackers){
	    // there's less ships attacking this target than the previous one
	    found_target=other_unit;
	    found_attackers=num_attackers;
	    setTarget(found_target);
	  }
	}

	other_unit=uiter.advance();
      }

      if(found_target==NULL){
	// we haven't found a target yet, search again
	if(which_target<=1){
	  ReTargetFg(which_target+1);
	}
	else{
	  // we can't find any target
	  setTarget(NULL);
	}
      }
#endif
}

void Unit::SetTurretAI () {
  static bool talkinturrets = XMLSupport::parse_bool(vs_config->getVariable("AI","independent_turrets","false"));
  if (talkinturrets) {
    UnitCollection::UnitIterator iter = getSubUnits();
    Unit * un;
    while (NULL!=(un=iter.current())) {
      if (!CheckAccessory(un)) {
	un->EnqueueAIFirst (new Orders::FireAt(.2,15));
	un->EnqueueAIFirst (new Orders::FaceTarget (false,3));
      }
      un->SetTurretAI ();
      iter.advance();
    }
  }else {
    UnitCollection::UnitIterator iter = getSubUnits();
    Unit * un;
    while (NULL!=(un=iter.current())) {
      if (!CheckAccessory(un)) {
	if (un->aistate) {
	  un->aistate->Destroy();
	}
	un->aistate = (new Orders::TurretAI());
	un->aistate->SetParent (un);
      }
      un->SetTurretAI ();
      iter.advance();
    }    
  }
}
void Unit::DisableTurretAI () {
  UnitCollection::UnitIterator iter = getSubUnits();
  Unit * un;
  while (NULL!=(un=iter.current())) {
    if (un->aistate) {
      un->aistate->Destroy();
    }
    un->aistate = new Order; //get 'er ready for enqueueing
    un->aistate->SetParent (un);
    un->UnFire();
    un->DisableTurretAI ();
    iter.advance();
  }
}

/***********************************************************************************/
/**** UNIT_PHYSICS STUFF                                                           */
/***********************************************************************************/

extern signed char  ComputeAutoGuarantee ( Unit * un);
extern float getAutoRSize (Unit * orig,Unit * un, bool ignore_friend=false);
extern void SetShieldZero(Unit*);
void Unit::UpdatePhysics (const Transformation &trans, const Matrix &transmat, const Vector & cum_vel,  bool lastframe, UnitCollection *uc) {
  static float VELOCITY_MAX=XMLSupport::parse_float(vs_config->getVariable ("physics","velocity_max","10000"));

	Transformation old_physical_state = curr_physical_state;
  if (docked&DOCKING_UNITS) {
    PerformDockingOperations();
  }
  Repair();
  if (fuel<0)
    fuel=0;
  if (cloaking>=cloakmin) {
    if (image->cloakenergy*SIMULATION_ATOM>warpenergy) {
      Cloak(false);//Decloak
    } else {
      SetShieldZero(this);
      if (image->cloakrate>0||cloaking==cloakmin) {
		warpenergy-=(SIMULATION_ATOM*image->cloakenergy);
      }
      if (cloaking>cloakmin) {
		AUDAdjustSound (sound->cloak, cumulative_transformation.position,cumulative_velocity);
		if ((cloaking==(2147483647)&&image->cloakrate>0)||(cloaking==cloakmin+1&&image->cloakrate<0)) {//short fix
		  AUDStartPlaying (sound->cloak);
	    }
		cloaking-= (int)(image->cloakrate*SIMULATION_ATOM); //short fix
		if (cloaking<=cloakmin&&image->cloakrate>0) {
		  //AUDStopPlaying (sound->cloak);
		  cloaking=cloakmin;
	    }
	    if (cloaking<0&&image->cloakrate<0) {
	      //AUDStopPlaying (sound->cloak);
	      cloaking=-2147483647-1;//wraps short fix
	    }
      }
    }
  }

  // Only on server or non-networking
  if( SERVER || Network==NULL)
  	RegenShields();
  if (lastframe) {
    if (!(docked&(DOCKED|DOCKED_INSIDE))) 
      prev_physical_state = curr_physical_state;//the AIscript should take care
#ifdef FIX_TERRAIN
    if (planet) {
      if (!planet->dirty) {
	SetPlanetOrbitData (NULL);
      }else {
	planet->pps = planet->cps;
      }
    }
#endif
  }

  if (isUnit()==PLANETPTR) {
    ((Planet *)this)->gravitate (uc);
  } else {
    if (resolveforces) {
      net_accel = ResolveForces (trans,transmat);//clamp velocity
      if (fabs (Velocity.i)>VELOCITY_MAX) {
	Velocity.i = copysign (VELOCITY_MAX,Velocity.i);
      }
      if (fabs (Velocity.j)>VELOCITY_MAX) {
	Velocity.j = copysign (VELOCITY_MAX,Velocity.j);
      }
      if (fabs (Velocity.k)>VELOCITY_MAX) {
	Velocity.k = copysign (VELOCITY_MAX,Velocity.k);
      }
    }
  } 
  float difficulty;
  Cockpit * player_cockpit=GetVelocityDifficultyMult (difficulty);

  this->UpdatePhysics2( trans, old_physical_state, net_accel, difficulty, transmat, cum_vel, lastframe, uc);

  float dist_sqr_to_target=FLT_MAX;
  Unit * target = Unit::Target();
  bool increase_locking=false;
  if (target&&cloaking<0/*-1 or -32768*/) {
    if (target->isUnit()!=PLANETPTR) {
      Vector TargetPos (InvTransform (cumulative_transformation_matrix,(target->Position()).Cast())); 
      dist_sqr_to_target = TargetPos.MagnitudeSquared(); 
      TargetPos.Normalize(); 
      if (TargetPos.Dot(Vector(0,0,1))>computer.radar.lockcone) {
	increase_locking=true;
      }
    }
  }
  static string LockingSoundName = vs_config->getVariable ("unitaudio","locking","locking.wav");
  static int LockingSound = AUDCreateSoundWAV (LockingSoundName,true);

  bool locking=false;
  bool touched=false;

  for (int i=0;(int)i<GetNumMounts();i++) {
//    if (increase_locking&&cloaking<0) {
//      mounts[i].time_to_lock-=SIMULATION_ATOM;
//    }
    if (mounts[i].status==Mount::ACTIVE&&cloaking<0&&mounts[i].ammo!=0) {
      if (player_cockpit) {
	  touched=true;
      }
      if (increase_locking&&(dist_sqr_to_target<mounts[i].type->Range*mounts[i].type->Range)) {
		mounts[i].time_to_lock-=SIMULATION_ATOM;
		static bool ai_lock_cheat=XMLSupport::parse_bool(vs_config->getVariable ("physics","ai_lock_cheat","true"));	
		if (!player_cockpit) {
		  if (ai_lock_cheat) {
			mounts[i].time_to_lock=-1;
		  }
		}else {

		  if (mounts[i].type->LockTime>0) {
			static string LockedSoundName= vs_config->getVariable ("unitaudio","locked","locked.wav");
			static int LockedSound = AUDCreateSoundWAV (LockedSoundName,false);

			if (mounts[i].time_to_lock>-SIMULATION_ATOM&&mounts[i].time_to_lock<=0) {
			  if (!AUDIsPlaying(LockedSound)) {
			AUDStartPlaying(LockedSound);
			AUDStopPlaying(LockingSound);	      
			  }
			  AUDAdjustSound (LockedSound,Position(),GetVelocity()); 
			}else if (mounts[i].time_to_lock>0)  {
			  locking=true;
			  if (!AUDIsPlaying(LockingSound)) {
			AUDStartPlaying(LockingSound);	      
			  }
			  AUDAdjustSound (LockingSound,Position(),GetVelocity());
			}
		  }
		}
      }else {
        if (mounts[i].ammo!=0) {
	  mounts[i].time_to_lock=mounts[i].type->LockTime;
        }
      }
    } else {
      if (mounts[i].ammo!=0) {
        mounts[i].time_to_lock=mounts[i].type->LockTime;
      }
    }
    if (mounts[i].type->type==weapon_info::BEAM) {
      if (mounts[i].ref.gun) {
		  Unit * autotarg = ((mounts[i].size&weapon_info::AUTOTRACKING)&&(mounts[i].time_to_lock<=0))?target:NULL;
		  float trackingcone = computer.radar.trackingcone;
		  if (CloseEnoughToAutotrack(this,target,trackingcone)) {
			  if (autotarg) {
				  if (computer.radar.trackingcone<trackingcone)
					  trackingcone = computer.radar.trackingcone;
			  }
			  autotarg = target;
		  }
		  mounts[i].ref.gun->UpdatePhysics (cumulative_transformation, cumulative_transformation_matrix,autotarg,trackingcone, target);
      }
    } else {
      mounts[i].ref.refire+=SIMULATION_ATOM;
    }
    if (mounts[i].processed==Mount::FIRED) {
      Transformation t1;
      Matrix m1;
      t1=prev_physical_state;//a hack that will not work on turrets
      t1.Compose (trans,transmat);
      t1.to_matrix (m1);
      int autotrack=0;
	  if ((0!=(mounts[i].size&weapon_info::AUTOTRACKING))) {
		  autotrack = computer.itts?2:1;
      }
	  float trackingcone = computer.radar.trackingcone;	  
	  if (CloseEnoughToAutotrack(this,target,trackingcone)) {
		  if (autotrack) {
			  if (trackingcone>computer.radar.trackingcone) {
				  trackingcone = computer.radar.trackingcone;
			  }
		  }
		  autotrack=2;
	  }
      if (!mounts[i].PhysicsAlignedFire (t1,m1,cumulative_velocity,(!isSubUnit()||owner==NULL)?this:owner,target,autotrack, trackingcone)) {
		  const weapon_info * typ = mounts[i].type;
		  energy+=typ->EnergyRate*(typ->type==weapon_info::BEAM?SIMULATION_ATOM:1);
		  if (mounts[i].ammo>=0)
			  mounts[i].ammo++;
	  }
      if (mounts[i].ammo==0&&mounts[i].type->type==weapon_info::PROJECTILE, i) {
//		  if (isPlayerStarship(this))
//			  ToggleWeapon (true);
      }
    }else if (mounts[i].processed==Mount::UNFIRED) {
      mounts[i].PhysicsAlignedUnfire();
    }
  }
  if (locking==false&&touched==true) {
    if (AUDIsPlaying(LockingSound)) {
      AUDStopPlaying(LockingSound);	
    }      
  }
  bool dead=true;

  if (!SubUnits.empty()) {
    Unit * su;
    UnitCollection::UnitIterator iter=getSubUnits();
    while ((su=iter.current())) {
      su->UpdatePhysics(cumulative_transformation,cumulative_transformation_matrix,cumulative_velocity,lastframe,uc); 
      su->cloaking = (unsigned int) cloaking; //short fix
      if (hull<0) {
	su->Target(NULL);
		UnFire();//don't want to go off shooting while your body's splitting everywhere
		//DEPRECATEDsu->hull-=SIMULATION_ATOM;
      }
      iter.advance();
      //    dead &=(subunits[i]->hull<0);
    }
  }
  // Really kill the unit only in non-networking or on server side
  if ((Network==NULL || SERVER) && hull<0) {
    dead&= (image->explosion==NULL);    
    if (dead)
      Kill();
  }
  if ((!isSubUnit())&&(!killed)&&(!(docked&DOCKED_INSIDE))) {
    UpdateCollideQueue();
  }
}
void Unit::AddVelocity(float difficulty) {
   static float warpramptime=XMLSupport::parse_float (vs_config->getVariable ("physics","warpramptime","0.5"));     
   Vector v=Velocity;
   if(graphicOptions.WarpRamping){ // Warp Turning off/on
	  graphicOptions.RampCounter=warpramptime;
	  graphicOptions.WarpRamping=0;
   }
   if(graphicOptions.InWarp==1||graphicOptions.RampCounter!=0){
	   static float fmultiplier=XMLSupport::parse_float(vs_config->getVariable("physics","hyperspace_multiplier","3141592"));
	   static float autopilot_term_distance = XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_termination_distance","6000"));     
	   static float smallwarphack = XMLSupport::parse_float (vs_config->getVariable ("physics","minwarpeffectsize","1000"));     
       //    float speed = v.Magnitude();
       //    if (speed>.01)
       //      if (speed>(computer.max_combat_speed))
       //      v*=computer.max_combat_speed/speed;
	   float minmultiplier=fmultiplier;
	   Unit * planet;
	   for (un_iter iter = _Universe->activeStarSystem()->gravitationalUnits().createIterator();(planet=*iter);++iter) {
		   if (planet==this) {
			   continue;
		   }
		 float multipliertemp=1;
		 float minsizeeffect = (planet->rSize()>smallwarphack)?planet->rSize():smallwarphack;
		 float effectiverad = autopilot_term_distance+minsizeeffect*(1.0f+UniverseUtil::getPlanetRadiusPercent())+getAutoRSize(this,this)+rSize();
		 double onethird=1.0/3.0;
		 double Nthoudist=9000000;
		 double Nthoudistalt=minsizeeffect*450;
		 Nthoudist=(Nthoudist<Nthoudistalt)?(Nthoudist):(Nthoudistalt);
		 double Nthouslow=9000/pow(Nthoudist,onethird);
		 double dist=(Position()-planet->Position()).Magnitude();
		 double cuberoot=pow((dist-(effectiverad)-Nthoudist),onethird);
		 if(dist>(effectiverad+Nthoudist)) {
			 multipliertemp=9000+(314*cuberoot);
		 } else if (dist>effectiverad){
			multipliertemp=9000-(Nthouslow*pow(-(dist-(effectiverad)-Nthoudist),onethird));
		 }else{
			minmultiplier=1;
		 }
		 minmultiplier=(multipliertemp<minmultiplier)?multipliertemp:minmultiplier;
	   }
	   float rampmult=1;
	   if(graphicOptions.RampCounter!=0){
	     graphicOptions.RampCounter-=SIMULATION_ATOM;
	     if(graphicOptions.RampCounter<=0){
	       graphicOptions.RampCounter=0;
		 }
	     rampmult=(graphicOptions.InWarp)?1.0-((graphicOptions.RampCounter/warpramptime)*(graphicOptions.RampCounter/warpramptime)):(graphicOptions.RampCounter/warpramptime)*(graphicOptions.RampCounter/warpramptime);
	   }
           static float warpMultiplier=XMLSupport::parse_float(vs_config->getVariable("physics","warpMultiplier","0"));
           static float warpMultiplierMax=XMLSupport::parse_float(vs_config->getVariable("physics","warpMultiplierMax","300000000"));
           if (warpMultiplier==0) {
             warpMultiplier=PI*PI*PI;
           }
	   minmultiplier*=rampmult;
	   if(minmultiplier<warpMultiplier) {
		 if(graphicOptions.InWarp==1) {
		   minmultiplier=warpMultiplier*(1-graphicOptions.RampCounter);
//		   printf("RAMP: %f\n",graphicOptions.RampCounter);
		 } else if (graphicOptions.RampCounter!=0) {
		   minmultiplier=warpMultiplier*graphicOptions.RampCounter;
		 } else {
		   minmultiplier=1;
		 }
	   }
	   if(minmultiplier>fmultiplier) {
		   minmultiplier=fmultiplier; //SOFT LIMIT
	   }
	   v*=minmultiplier;
	   float vmag=sqrt(v.i*v.i+v.j*v.j+v.k*v.k);
	   if(vmag>warpMultiplier*warpMultiplierMax){
		   v*=warpMultiplier*warpMultiplierMax/vmag; // HARD LIMIT
	   }
	   graphicOptions.WarpFieldStrength=minmultiplier;
   } else {
	   graphicOptions.WarpFieldStrength=1;
   }
   curr_physical_state.position = curr_physical_state.position +  (v*SIMULATION_ATOM*difficulty).Cast();
}
void Unit::UpdatePhysics2 (const Transformation &trans, const Transformation & old_physical_state, const Vector & accel, float difficulty, const Matrix &transmat, const Vector & cum_vel,  bool lastframe, UnitCollection *uc)
{
	Cockpit * cp = _Universe->isPlayerStarship( this);
  // Only in non-networking OR networking && is a player OR SERVER && not a player
  if( (Network==NULL && !SERVER) || (Network!=NULL && cp && !SERVER) || (SERVER && !cp))
  {
	  if(AngularVelocity.i||AngularVelocity.j||AngularVelocity.k) {
	    Rotate (SIMULATION_ATOM*(AngularVelocity));
	  }
  }

	// SERVERSIDE ONLY : If it is not a player, it is a unit controlled by server so compute changes
	if( SERVER && Network!=NULL && !cp)
	{
		AddVelocity(difficulty);
                
		cumulative_transformation = curr_physical_state;
		cumulative_transformation.Compose (trans,transmat);
		cumulative_transformation.to_matrix (cumulative_transformation_matrix);
		cumulative_velocity = TransformNormal (transmat,Velocity)+cum_vel;
	}
}
static float tempmin (float a, float b) {
  return a>b?b:a;
}
static QVector RealPosition (Unit * un) {
  if (un->isSubUnit())
    return un->Position();
  return un->LocalPosition();
}
static QVector AutoSafeEntrancePoint (const QVector start, float rsize,Unit * goal) {
  QVector def = UniverseUtil::SafeEntrancePoint(start,rsize);
  float bdis = (def-RealPosition(goal)).MagnitudeSquared();
  for (int i=-1;i<=1;i++) {
    for (int j=-1;j<=1;j++) {
      for (int k=-1;k<=1;k+=2) {
	QVector delta(i,j,k);delta.Normalize();
	QVector tmp =RealPosition(goal)+delta*(goal->rSize()+rsize);
	tmp=UniverseUtil::SafeEntrancePoint (tmp,rsize);
	float tmag = (tmp-RealPosition(goal)).MagnitudeSquared();
	if (tmag<bdis){
	  bdis = tmag;
	  def = tmp;
	}
      }
    }
  }
  return def;
}
bool Unit::AutoPilotTo (Unit * target, bool ignore_energy_requirements, int recursive_level) {
	static bool auto_valid = XMLSupport::parse_bool (vs_config->getVariable ("physics","insystem_jump_or_timeless_auto-pilot","false"));	
	if(!auto_valid) {
		return false;
	}
	
	if (target->isUnit()==PLANETPTR) {
		un_iter i = target->getSubUnits();
		Unit * targ =*i;
		if (targ&&0==targ->graphicOptions.FaceCamera)
			return AutoPilotTo(targ,ignore_energy_requirements,recursive_level);
	}
	//static float insys_jump_cost = XMLSupport::parse_float (vs_config->getVariable ("physics","insystem_jump_cost",".1"));
  if (warpenergy<jump.insysenergy) {
	  if (!ignore_energy_requirements)
		  return false;
  }
  signed char Guaranteed = ComputeAutoGuarantee (this);
  if (Guaranteed==Mission::AUTO_OFF) {
    return false;
  }
  static float autopilot_term_distance = XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_termination_distance","6000"));
//  static float autopilot_p_term_distance = XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_planet_termination_distance","60000"));
  if (isSubUnit()) {
    return false;//we can't auto here;
  }
  StarSystem * ss = activeStarSystem;
  if (ss==NULL) {
    ss = _Universe->activeStarSystem();
  }

  Unit * un=NULL;
  QVector start (Position());
  QVector end (RealPosition(target));
  float totallength = (start-end).Magnitude();
  bool nanspace=false;
  if (!FINITE(totallength)) {
    nanspace=true;
    start=QVector(100000000.0,100000000.0,10000000000000.0);
    totallength = (start-end).Magnitude();
	if (!FINITE(totallength)) {
		end=QVector(200000000.0,100000000.0,10000000000000.0);;
		totallength=(start-end).Magnitude();
	}
  }
  float totpercent=1;
  if (totallength>1) {
    //    float apt = (target->isUnit()==PLANETPTR&&target->GetDestinations().empty())?autopilot_p_term_distance:autopilot_term_distance;
	  float apt = (target->isUnit()==PLANETPTR)?(autopilot_term_distance+target->rSize()*UniverseUtil::getPlanetRadiusPercent()):autopilot_term_distance;
    float percent = (getAutoRSize(this,this)+rSize()+target->rSize()+apt)/totallength;
    if (percent>1) {
      end=start;
      totpercent=0;
    }else {
      totpercent*=(1-percent);
      end = start*percent+end*(1-percent);
    }
  }
  bool ok=true;
  static bool teleport_autopilot= XMLSupport::parse_bool(vs_config->getVariable("physics","teleport_autopilot","true"));
  if ((!teleport_autopilot)&&(!nanspace)) {
  if (Guaranteed==Mission::AUTO_NORMAL&&CloakVisible()>.5) {
	  bool ignore_friendlies=true;
    for (un_iter i=ss->getUnitList().createIterator(); (un=*i)!=NULL; ++i) {
      static bool canflythruplanets= XMLSupport::parse_bool(vs_config->getVariable("physics","can_auto_through_planets","true"));
      if ((!(un->isUnit()==PLANETPTR&&canflythruplanets))&&un->isUnit()!=NEBULAPTR && (!UnitUtil::isSun(un))) {
		if (un!=this&&un!=target) {
    	  if ((start-un->Position()).Magnitude()-getAutoRSize (this,this,ignore_friendlies)-rSize()-un->rSize()-getAutoRSize(this,un,ignore_friendlies)<=0) {
	    return false;
	  }
	  float intersection = un->querySphere (start,end,getAutoRSize (this,un,ignore_friendlies));
	  if (intersection>0) {
	    end = start+ (end-start)*intersection;
	    totpercent*=intersection;
	    ok=false;
	  }
	 }
    }
   }
  }
  }else if (!nanspace) {
	  //just make sure we aren't in an asteroid field
	  Unit * un;
	  for (un_iter i=ss->getUnitList().createIterator(); (un=*i)!=NULL; ++i) {
		  if (un->isUnit()==ASTEROIDPTR) {
			  if (UnitUtil::getDistance(this,un)<0) {
				  return false;//no auto in roid field
			  }
		  }
	  }
	  
  }
  bool nowhere=false;
	
  if (this!=target) {
    warpenergy-=totpercent*jump.insysenergy;
    QVector sep (UniverseUtil::SafeEntrancePoint(end,rSize()));
    if ((sep-end).MagnitudeSquared()>16*rSize()*rSize()) {
      sep = AutoSafeEntrancePoint (end,(RealPosition(target)-end).Magnitude()-target->rSize(),target);
    }
    if ((sep-RealPosition(target)).MagnitudeSquared()>(RealPosition(this)-RealPosition(target)).MagnitudeSquared()) {
		sep= RealPosition(this);
		nowhere=true;
	}

    static bool auto_turn_towards =XMLSupport::parse_bool(vs_config->getVariable ("physics","auto_turn_towards","true"));
    if (auto_turn_towards) {
      Vector methem(RealPosition(target).Cast()-sep.Cast());
      methem.Normalize();
      Vector p,q,r;
      GetOrientation(p,q,r);
      p=methem.Cross(r);
      float theta = p.Magnitude();
      if (theta*theta>.00001){
	p*= (asin (theta)/theta);
	Rotate(p);
	GetOrientation (p,q,r);
	if (r.Dot(methem)<0) {
	  Rotate (p*(PI/theta));
	}
      }
    }
    static string insys_jump_ani = vs_config->getVariable ("graphics","insys_jump_animation","warp.ani");
    if (

		//nowhere==false&&

		insys_jump_ani.length()) {
		static bool docache=true;
		if (docache){
			UniverseUtil::cacheAnimation (insys_jump_ani);
			docache=false;
		}
                static float insys_jump_ani_size = XMLSupport::parse_float(vs_config->getVariable("graphics","insys_jump_animation_size","4"));
                static float insys_jump_ani_growth = XMLSupport::parse_float(vs_config->getVariable("graphics","insys_jump_animation_growth",".99"));
		UniverseUtil::playAnimationGrow (insys_jump_ani,RealPosition(this),rSize()*insys_jump_ani_size,insys_jump_ani_growth);


      Vector v(GetVelocity());
      v.Normalize();
      Vector p,q,r;GetOrientation(p,q,r);
	  static float sec = XMLSupport::parse_float(vs_config->getVariable("graphics","insys_jump_ani_second_ahead","1.5"));
      UniverseUtil::playAnimationGrow (insys_jump_ani,sep+GetVelocity()*sec+v*rSize(),rSize()*8,.97);
      UniverseUtil::playAnimationGrow (insys_jump_ani,sep+GetVelocity()*sec+2*v*rSize()+r*4*rSize(),rSize()*16,.97);
    }
    static bool warptrail = XMLSupport::parse_bool (vs_config->getVariable ("graphics","warp_trail","true"));
    if (warptrail&&(!nowhere)) {
      static float warptrailtime = XMLSupport::parse_float (vs_config->getVariable ("graphics","warp_trail_time","20"));
      AddWarp(this,RealPosition(this),warptrailtime);
    }
    //    sep =UniverseUtil::SafeEntrancePoint (sep);
	if (!nowhere)
		SetCurPosition(sep);
    if (_Universe->isPlayerStarship (this)&&getFlightgroup()!=NULL) {
      Unit * other=NULL;
      if (recursive_level>0)
      for (un_iter ui=ss->getUnitList().createIterator(); NULL!=(other = *ui); ++ui) {
    	Flightgroup * ff = other->getFlightgroup();
	bool leadah=(ff==getFlightgroup());
	if (ff) {
	  if (ff->leader.GetUnit()==this) {
	    leadah=true;
	  }
	}
	Order * otherord = other->getAIState();
	if (otherord)
	  if (otherord->PursueTarget (this,leadah)) {
	    other->AutoPilotTo(this,ignore_energy_requirements,recursive_level-1);
	    if (leadah) {
	      if (NULL==_Universe->isPlayerStarship (other)) {
		other->SetPosition(AutoSafeEntrancePoint (LocalPosition(),other->rSize()*1.5,other));
	      }
	    }
	  }
      
      }
    }
  }
  return ok;
}

extern void ActivateAnimation(Unit * jp);
void TurnJumpOKLightOn(Unit * un, Cockpit * cp) {
	if (cp) {
		if (un->GetWarpEnergy()>=un->GetJumpStatus().energy) {
			if (un->GetJumpStatus().drive>-2) {
				cp->jumpok=1;
			}
		}
	}
}
bool Unit::jumpReactToCollision (Unit * smalle) {
	static bool ai_jump_cheat=XMLSupport::parse_bool(vs_config->getVariable("AI","jump_without_energy","false"));
  if (!GetDestinations().empty()) {//only allow big with small
	  Cockpit * cp = _Universe->isPlayerStarship(smalle);
	  TurnJumpOKLightOn(smalle,cp);
	  //ActivateAnimation(this);
	  if ((smalle->GetJumpStatus().drive>=0&&
		   (smalle->warpenergy>=smalle->GetJumpStatus().energy
			||(ai_jump_cheat&&cp==NULL)
			   ))
		  ||image->forcejump){
		smalle->warpenergy-=smalle->GetJumpStatus().energy;
		int dest = smalle->GetJumpStatus().drive;
		if (dest<0)
			dest=0;
		smalle->DeactivateJumpDrive();
		Unit * jumppoint = this;
		_Universe->activeStarSystem()->JumpTo (smalle, jumppoint, std::string(GetDestinations()[dest%GetDestinations().size()]));
		if( SERVER)
			VSServer->sendJump( smalle->GetSerial(), this->serial, true);
		return true;
    }
	/* NOT NECESSARY ANYMORE SINCE THE CLIENT ONLY ASK FOR AUTH WITHOUT EXPECTING AN ANSWER
	else
	{
		if( SERVER)
			VSServer->sendJump( this->serial, false);
	}
	*/
    return true;
  }
  if (!smalle->GetDestinations().empty()) {
	  Cockpit * cp = _Universe->isPlayerStarship(this);	  
	  TurnJumpOKLightOn(this,cp);	  
	  //  ActivateAnimation(smalle);	  
    if ((GetJumpStatus().drive>=0&&
		   (warpenergy>=GetJumpStatus().energy
			||(ai_jump_cheat&&cp==NULL)
				))
		||smalle->image->forcejump){
      DeactivateJumpDrive();
      Unit * jumppoint = smalle;
      _Universe->activeStarSystem()->JumpTo (this, jumppoint, std::string(smalle->GetDestinations()[GetJumpStatus().drive%smalle->GetDestinations().size()]));
		if( SERVER)
			VSServer->sendJump( smalle->GetSerial(), this->serial, true);
      return true;
    }
	else
	{
		if( SERVER)
			VSServer->sendJump( this->serial, smalle->GetSerial(), false);
	}
    return true;
  }
  else
  {
	if( SERVER)
		VSServer->sendJump( this->serial, smalle->GetSerial(), false);
  }
  return false;
}

Cockpit * Unit::GetVelocityDifficultyMult(float &difficulty) const{
  difficulty=1;
  Cockpit * player_cockpit=_Universe->isPlayerStarship(this);
  if ((player_cockpit)==NULL) {
    static float exp = XMLSupport::parse_float (vs_config->getVariable ("physics","difficulty_speed_exponent",".2"));
    difficulty = pow(g_game.difficulty,exp);
  }
  return player_cockpit;
}

void Unit::Rotate (const Vector &axis)
{
	double theta = axis.Magnitude();
	double ootheta=0;
	if( theta==0) return;
	ootheta = 1/theta;
	float s = cos (theta * .5);
	Quaternion rot = Quaternion(s, axis * (sinf (theta*.5)*ootheta));
	
	if(theta < 0.0001) {
	  rot = identity_quaternion;
	}
	curr_physical_state.orientation *= rot;
	
	if (limits.limitmin>-1) {
	  Matrix mat;
	  curr_physical_state.orientation.to_matrix (mat);
	  if (limits.structurelimits.Dot (mat.getR())<limits.limitmin) {
	    curr_physical_state.orientation=prev_physical_state.orientation;
		//VSFileSystem::vs_fprintf (stderr,"wierd case... with an i before the e\n", mat.getR().i,mat.getR().j,mat.getR().k);
		
	  }
	}
	
}

void Unit::FireEngines (const Vector &Direction/*unit vector... might default to "r"*/,
					float FuelSpeed,
					float FMass)
{
  //mass -= FMass; //fuel is sent out Now we separated mass and fuel
	static float fuelpct=XMLSupport::parse_float(vs_config->getVariable("physics","FuelUsage","1"));
	fuel -= fuelpct*FMass;
	if (fuel <0)
	{
		
		FMass +=fuel;
		//mass -= fuel;
		fuel = 0; //ha ha!
	}
	NetForce += Direction *(FuelSpeed *FMass/GetElapsedTime());
}
void Unit::ApplyForce(const Vector &Vforce) //applies a force for the whole gameturn upon the center of mass
{
	if (FINITE (Vforce.i)&&FINITE(Vforce.j)&&FINITE(Vforce.k)) {
		NetForce += Vforce;
	}else {
		VSFileSystem::vs_fprintf (stderr,"fatal force");
	}
}
void Unit::ApplyLocalForce(const Vector &Vforce) //applies a force for the whole gameturn upon the center of mass
{
	if (FINITE (Vforce.i)&&FINITE(Vforce.j)&&FINITE(Vforce.k)) {
		NetLocalForce += Vforce;
	}else {
		VSFileSystem::vs_fprintf (stderr,"fatal local force");
	}
}
void Unit::Accelerate(const Vector &Vforce)
{
	if (FINITE (Vforce.i)&&FINITE(Vforce.j)&&FINITE(Vforce.k)) {	
		NetForce += Vforce * GetMass();
	}else {
		VSFileSystem::vs_fprintf (stderr,"fatal force");
	}
}

void Unit::ApplyTorque (const Vector &Vforce, const QVector &Location)
{
  //Not completely correct
	NetForce += Vforce;
	NetTorque += Vforce.Cross ((Location-curr_physical_state.position).Cast());
}
void Unit::ApplyLocalTorque (const Vector &Vforce, const Vector &Location)
{
	NetForce += Vforce;
	NetTorque += Vforce.Cross (Location);
}
void Unit::ApplyBalancedLocalTorque (const Vector &Vforce, const Vector &Location) //usually from thrusters remember if I have 2 balanced thrusters I should multiply their effect by 2 :)
{
	NetTorque += Vforce.Cross (Location);
	
}

void Unit::ApplyLocalTorque(const Vector &torque) {
  /*  Vector p,q,r;
  Vector tmp(ClampTorque(torque));
  GetOrientation (p,q,r);
  VSFileSystem::vs_fprintf (stderr,"P: %f,%f,%f Q: %f,%f,%f",p.i,p.j,p.k,q.i,q.j,q.k);
  NetTorque+=tmp.i*p+tmp.j*q+tmp.k*r; 
  */
  NetLocalTorque+= ClampTorque(torque); 
}

Vector Unit::MaxTorque(const Vector &torque) {
  // torque is a normal
  return torque * (Vector(copysign(limits.pitch, torque.i), 
			  copysign(limits.yaw, torque.j),
			  copysign(limits.roll, torque.k)) * torque);
}

float GetFuelUsage (bool afterburner) {
  static float total_accel=XMLSupport::parse_float (vs_config->getVariable ("physics","game_speed",".9"))*XMLSupport::parse_float (vs_config->getVariable("physics","game_accel","1"));
  static float normalfuelusage = XMLSupport::parse_float (vs_config->getVariable ("physics","FuelUsage","1"));
  static float abfuelusage = XMLSupport::parse_float (vs_config->getVariable ("physics","AfterburnerFuelUsage","4"));
  if (afterburner) 
    return abfuelusage/total_accel;
  return normalfuelusage/total_accel;
}
Vector Unit::ClampTorque (const Vector &amt1) {
  Vector Res=amt1;
 
  static float staticfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelThrust",".9"));
  float fuelclamp=(fuel<=0)?staticfuelclamp:1;
  if (fabs(amt1.i)>fuelclamp*limits.pitch)
    Res.i=copysign(fuelclamp*limits.pitch,amt1.i);
  if (fabs(amt1.j)>fuelclamp*limits.yaw)
    Res.j=copysign(fuelclamp*limits.yaw,amt1.j);
  if (fabs(amt1.k)>fuelclamp*limits.roll)
    Res.k=copysign(fuelclamp*limits.roll,amt1.k);
  static float fuelenergytomassconversionconstant = XMLSupport::parse_float(vs_config->getVariable ("physics","FuelEnergyDensity","343596000000000.0")); // note that we have KiloJoules, so it's to the 14th
  fuel-=GetFuelUsage(false)*SIMULATION_ATOM*Res.Magnitude()*.00000004;//HACK this forces the reaction to be deut-deut fusion with efficiency governed by the getFuelUsage function
  return Res;
}
float Unit::Computer::max_speed() const {
  static float combat_mode_mult = XMLSupport::parse_float (vs_config->getVariable ("physics","combat_speed_boost","100"));
  return (!combat_mode)?combat_mode_mult*max_combat_speed:max_combat_speed;
}
float Unit::Computer::max_ab_speed() const {
  static float combat_mode_mult = XMLSupport::parse_float (vs_config->getVariable ("physics","combat_speed_boost","100"));
  return (!combat_mode)?combat_mode_mult*max_combat_speed:max_combat_ab_speed;//same capped big speed as combat...else different
}
void Unit::SwitchCombatFlightMode() {
  if (computer.combat_mode)
    computer.combat_mode=false;
  else
    computer.combat_mode=true;
}
bool Unit::CombatMode() {
  return computer.combat_mode;
}
Vector Unit::ClampVelocity (const Vector & velocity, const bool afterburn) {
  static float staticfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelThrust",".9"));
  static float staticabfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelAfterburn",".1"));
  float fuelclamp=(fuel<=0)?staticfuelclamp:1;
  float abfuelclamp= (fuel<=0||(energy<afterburnenergy))?staticabfuelclamp:1;
  float limit = afterburn?(abfuelclamp*(computer.max_ab_speed()-computer.max_speed())+(fuelclamp*computer.max_speed())):fuelclamp*computer.max_speed();
  float tmp = velocity.Magnitude();
  if (tmp>fabs(limit)) {
    return velocity * (limit/tmp);
  }
  return velocity;
}
void Unit::ClearMounts() {
	mounts.clear();
	Unit * su;
	for (un_iter i = getSubUnits(); (su= *i)!=NULL;++i) {
		su->ClearMounts();
	}
}

Vector Unit::ClampAngVel (const Vector & velocity) {
  Vector res (velocity);
  if (fabs (res.i)>computer.max_pitch) {
    res.i = copysign (computer.max_pitch,res.i);
  }
  if (fabs (res.j)>computer.max_yaw) {
    res.j = copysign (computer.max_yaw,res.j);
  }
  if (fabs (res.k)>computer.max_roll) {
    res.k = copysign (computer.max_roll,res.k);
  }
  return res;
}


Vector Unit::MaxThrust(const Vector &amt1) {
  // amt1 is a normal
  return amt1 * (Vector(copysign(limits.lateral, amt1.i), 
	       copysign(limits.vertical, amt1.j),
	       amt1.k>0?limits.forward:-limits.retro) * amt1);
}
/* misnomer..this doesn't get the max value of each axis
Vector Unit::ClampThrust(const Vector &amt1){ 
  // Yes, this can be a lot faster with LUT
  Vector norm = amt1;
  norm.Normalize();
  Vector max = MaxThrust(norm);

  if(max.Magnitude() > amt1.Magnitude())
    return amt1;
  else 
    return max;
}
*/
//CMD_FLYBYWIRE depends on new version of Clampthrust... don't change without resolving it

Vector Unit::ClampThrust (const Vector &amt1, bool afterburn) {
  Vector Res=amt1;
  if (energy<afterburnenergy) {
    afterburn=false;
  }
  if (afterburn) {
    energy -= afterburnenergy*SIMULATION_ATOM;
  }

  static float staticfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelThrust",".4"));
  static float staticabfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelAfterburn","0"));
  float fuelclamp=(fuel<=0)?staticfuelclamp:1;
  float abfuelclamp= (fuel<=0)?staticabfuelclamp:1;
  if (fabs(amt1.i)>fabs(fuelclamp*limits.lateral))
    Res.i=copysign(fuelclamp*limits.lateral,amt1.i);
  if (fabs(amt1.j)>fabs(fuelclamp*limits.vertical))
    Res.j=copysign(fuelclamp*limits.vertical,amt1.j);
  float ablimit =       
    afterburn
    ?((limits.afterburn-limits.forward)*abfuelclamp+limits.forward*fuelclamp)
    :limits.forward;

  if (amt1.k>ablimit)
    Res.k=ablimit;
  if (amt1.k<-limits.retro)
    Res.k =-limits.retro;
  //energy = 1/2t^2*Force^2/mass
  static float fuelenergytomassconversionconstant = XMLSupport::parse_float(vs_config->getVariable ("physics","FuelEnergyDensity","343596000000000.0")); // note that we have KiloJoules, so it's to the 14th
  fuel-=GetFuelUsage(afterburn)*SIMULATION_ATOM*Res.Magnitude()*.00000004;//HACK this forces the reaction to be deut-deut fusion with efficiency governed by the getFuelUsage function
  return Res;
}

void Unit::Thrust(const Vector &amt1,bool afterburn){
  Vector amt = ClampThrust(amt1,afterburn);
  ApplyLocalForce(amt);  
  }

void Unit::LateralThrust(float amt) {
  if(amt>1.0) amt = 1.0;
  if(amt<-1.0) amt = -1.0;
  ApplyLocalForce(amt*limits.lateral * Vector(1,0,0));
}

void Unit::VerticalThrust(float amt) {
  if(amt>1.0) amt = 1.0;
  if(amt<-1.0) amt = -1.0;
  ApplyLocalForce(amt*limits.vertical * Vector(0,1,0));
}

void Unit::LongitudinalThrust(float amt) {
  if(amt>1.0) amt = 1.0;
  if(amt<-1.0) amt = -1.0;
  ApplyLocalForce(amt*limits.forward * Vector(0,0,1));
}

void Unit::YawTorque(float amt) {
  if(amt>limits.yaw) amt = limits.yaw;
  else if(amt<-limits.yaw) amt = -limits.yaw;
  ApplyLocalTorque(amt * Vector(0,1,0));
}

void Unit::PitchTorque(float amt) {
  if(amt>limits.pitch) amt = limits.pitch;
  else if(amt<-limits.pitch) amt = -limits.pitch;
  ApplyLocalTorque(amt * Vector(1,0,0));
}

void Unit::RollTorque(float amt) {
  if(amt>limits.roll) amt = limits.roll;
  else if(amt<-limits.roll) amt = -limits.roll;
  ApplyLocalTorque(amt * Vector(0,0,1));
}
float WARPENERGYMULTIPLIER(Unit * un) {
  static float warpenergymultiplier = XMLSupport::parse_float (vs_config->getVariable ("physics","warp_energy_multiplier",".04"));
  static float playerwarpenergymultiplier = XMLSupport::parse_float (vs_config->getVariable ("physics","warp_energy_player_multiplier",".3"));
  bool player=_Universe->isPlayerStarship(un)!=NULL;
  Flightgroup * fg =un->getFlightgroup();
  if (fg&&!player) {
	  player = _Universe->isPlayerStarship(fg->leader.GetUnit())!=NULL;
  }
  return player?playerwarpenergymultiplier:warpenergymultiplier;
}
static bool applyto (float &shield, const float max, const float amt) { //short fix
  shield+=amt; //short fix
  if (shield>max)
    shield=max;
  return (shield>=max)?1:0;
}

float totalShieldVal (const Shield & shield) {
	float maxshield=0;
	switch (shield.number)  {
  case 2:
    maxshield = shield.shield2fb.frontmax+shield.shield2fb.backmax;
    break;
  case 4:
    maxshield = shield.shield4fbrl.frontmax+shield.shield4fbrl.backmax+shield.shield4fbrl.leftmax+shield.shield4fbrl.rightmax;
    break;
  case 8:
    maxshield = shield.shield8.frontrighttopmax+shield.shield8.backrighttopmax+shield.shield8.frontlefttopmax+shield.shield8.backlefttopmax+shield.shield8.frontrightbottommax+shield.shield8.backrightbottommax+shield.shield8.frontleftbottommax+shield.shield8.backleftbottommax;
    break;
	}
	return maxshield;
}

float currentTotalShieldVal (const Shield & shield) {
  float maxshield=0;
  switch (shield.number)  {
  case 2:
    maxshield = shield.shield2fb.front+shield.shield2fb.back;
    break;
  case 4:
    maxshield = shield.shield4fbrl.front+shield.shield4fbrl.back+shield.shield4fbrl.left+shield.shield4fbrl.right;
    break;
  case 8:
    maxshield = shield.shield8.frontrighttop+shield.shield8.backrighttop+shield.shield8.frontlefttop+shield.shield8.backlefttop+shield.shield8.frontrightbottom+shield.shield8.backrightbottom+shield.shield8.frontleftbottom+shield.shield8.backleftbottom;
    break;
	}
  return maxshield;
}

float totalShieldEnergyCapacitance (const Shield & shield) {
	static float shieldenergycap = XMLSupport::parse_float(vs_config->getVariable ("physics","shield_energy_capacitance",".2"));
        static bool use_max_shield_value = XMLSupport::parse_bool(vs_config->getVariable("physics","use_max_shield_energy_usage","true"));
        
	return shieldenergycap * use_max_shield_value?totalShieldVal(shield):currentTotalShieldVal(shield);
}
float Unit::MaxShieldVal() const{
  float maxshield=0;
  switch (shield.number) {
  case 2:
    maxshield = .5*(shield.shield2fb.frontmax+shield.shield2fb.backmax);
    break;
  case 4:
    maxshield = .25*(shield.shield4fbrl.frontmax+shield.shield4fbrl.backmax+shield.shield4fbrl.leftmax+shield.shield4fbrl.rightmax);
    break;
  case 6:
    maxshield = .125*(shield.shield8.frontrighttopmax+shield.shield8.backrighttopmax+shield.shield8.frontlefttopmax+shield.shield8.backlefttopmax+shield.shield8.frontrightbottommax+shield.shield8.backrightbottommax+shield.shield8.frontleftbottommax+shield.shield8.backleftbottommax);
    break;
  }
  return maxshield;
}
void Unit::RechargeEnergy() {
#if 1
	energy+=recharge*SIMULATION_ATOM;
#else
	/* short fix
    unsigned short newenergy=apply_float_to_short (recharge *SIMULATION_ATOM);
    if (((int)energy)+((int)newenergy)>65535) {
      newenergy= 65535 - energy;
      energy=65535;
      if (newenergy>0) {
	newenergy=apply_float_to_short (newenergy*WARPENERGYMULTIPLIER(this));
	if (((int)warpenergy)+((int)newenergy)>65535) {	
	  warpenergy+=newenergy;
	}
      }
    }else {
      energy+=recharge*SIMULATION_ATOM;
	}
	*/
#endif
}
void Unit::RegenShields () {
  int rechargesh=1; // used ... oddly
  float maxshield=totalShieldEnergyCapacitance(shield);
  static bool energy_before_shield=XMLSupport::parse_bool(vs_config->getVariable ("physics","engine_energy_priority","true"));
  static bool apply_difficulty_shields = XMLSupport::parse_bool (vs_config->getVariable("physics","difficulty_based_shield_recharge","true"));
  if (!energy_before_shield) {
    RechargeEnergy();
  }
  float rec = shield.recharge*SIMULATION_ATOM>energy?energy:shield.recharge*SIMULATION_ATOM;
  if (apply_difficulty_shields) {
    if (!_Universe->isPlayerStarship(this)) {
      rec*=g_game.difficulty;
    }else {
      rec*=g_game.difficulty;//sqrtf(g_game.difficulty);
    }
  }
  bool velocity_discharge=false;
  static float speed_leniency = XMLSupport::parse_float (vs_config->getVariable("physics","speed_shield_drain_leniency","1.18"));
  /*
  if ((computer.max_combat_ab_speed>4)&&(GetVelocity().MagnitudeSquared()>(computer.max_combat_ab_speed*speed_leniency*computer.max_combat_ab_speed*speed_leniency))) {
      rec=0;
      velocity_discharge=true;
  }
  */
  if (graphicOptions.InWarp) {
      rec=0;
      velocity_discharge=true;
  }
  if ((image->ecm>0)) {
    static float ecmadj = XMLSupport::parse_float(vs_config->getVariable ("physics","ecm_energy_cost",".05"));
    float sim_atom_ecm = ecmadj * image->ecm*SIMULATION_ATOM;
    if (energy-10>sim_atom_ecm) {
      energy-=sim_atom_ecm;
    }else {
      energy=energy<10?energy:10;
    }
  }
  if (GetNebula()!=NULL) {
    static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
    rec *=nebshields;
  }
  static float discharge_per_second=XMLSupport::parse_float (vs_config->getVariable("physics","speeding_discharge",".95"));
  const float dischargerate = (1-(1-discharge_per_second)*SIMULATION_ATOM);//approx
  static float min_shield_discharge=XMLSupport::parse_float (vs_config->getVariable("physics","min_shield_speeding_discharge",".1"));

  switch (shield.number) {
  case 2:

    shield.shield2fb.front+=rec;
    shield.shield2fb.back+=rec;
    if (shield.shield2fb.front>shield.shield2fb.frontmax) {
      shield.shield2fb.front=shield.shield2fb.frontmax;
    } else {
      rechargesh=0;
    }
    if (shield.shield2fb.back>shield.shield2fb.backmax) {
      shield.shield2fb.back=shield.shield2fb.backmax;

    } else {
      rechargesh=0;
    }
    if (velocity_discharge) {
      if (shield.shield2fb.back>min_shield_discharge*shield.shield2fb.backmax)
	shield.shield2fb.back*=dischargerate;
      if (shield.shield2fb.front>min_shield_discharge*shield.shield2fb.frontmax)
	shield.shield2fb.front*=dischargerate;
    }
    break;
  case 4:
    rechargesh = applyto (shield.shield4fbrl.front,shield.shield4fbrl.frontmax,rec)*(applyto (shield.shield4fbrl.back,shield.shield4fbrl.backmax,rec))*applyto (shield.shield4fbrl.right,shield.shield4fbrl.rightmax,rec)*applyto (shield.shield4fbrl.left,shield.shield4fbrl.leftmax,rec);
    if (velocity_discharge) {
      if (shield.shield4fbrl.front>min_shield_discharge*shield.shield4fbrl.frontmax)
	shield.shield4fbrl.front*=dischargerate;
      if (shield.shield4fbrl.left>min_shield_discharge*shield.shield4fbrl.leftmax)
	shield.shield4fbrl.left*=dischargerate;
      if (shield.shield4fbrl.back>min_shield_discharge*shield.shield4fbrl.backmax)
	shield.shield4fbrl.back*=dischargerate;
      if (shield.shield4fbrl.right>min_shield_discharge*shield.shield4fbrl.rightmax)
	shield.shield4fbrl.right*=dischargerate;
    }
    break;
  case 8:
    rechargesh = applyto (shield.shield8.frontrighttop,shield.shield8.frontrighttopmax,rec)*(applyto (shield.shield8.backrighttop,shield.shield8.backrighttopmax,rec))*applyto (shield.shield8.frontlefttop,shield.shield8.frontlefttopmax,rec)*applyto (shield.shield8.backlefttop,shield.shield8.backlefttopmax,rec)*applyto (shield.shield8.frontrightbottom,shield.shield8.frontrightbottommax,rec)*(applyto (shield.shield8.backrightbottom,shield.shield8.backrightbottommax,rec))*applyto (shield.shield8.frontleftbottom,shield.shield8.frontleftbottommax,rec)*applyto (shield.shield8.backleftbottom,shield.shield8.backleftbottommax,rec);
    if (velocity_discharge) {
      if (shield.shield8.frontrighttop>min_shield_discharge*shield.shield8.frontrighttopmax)
		shield.shield8.frontrighttop*=dischargerate;
	  if (shield.shield8.frontlefttop>min_shield_discharge*shield.shield8.frontlefttopmax)
		shield.shield8.frontlefttop*=dischargerate;
	  if (shield.shield8.backrighttop>min_shield_discharge*shield.shield8.backrighttopmax)
		shield.shield8.backrighttop*=dischargerate;
	  if (shield.shield8.backlefttop>min_shield_discharge*shield.shield8.backlefttopmax)
		shield.shield8.backlefttop*=dischargerate;
	  if (shield.shield8.frontrightbottom>min_shield_discharge*shield.shield8.frontrightbottommax)
		shield.shield8.frontrightbottom*=dischargerate;
	  if (shield.shield8.frontleftbottom>min_shield_discharge*shield.shield8.frontleftbottommax)
		shield.shield8.frontleftbottom*=dischargerate;
	  if (shield.shield8.backrightbottom>min_shield_discharge*shield.shield8.backrightbottommax)
		shield.shield8.backrightbottom*=dischargerate;
	  if (shield.shield8.backleftbottom>min_shield_discharge*shield.shield8.backleftbottommax)
		shield.shield8.backleftbottom*=dischargerate;
    }
    break;
  }
  if (rechargesh==0)
    energy-=rec;
  static float max_shield_lowers_recharge=XMLSupport::parse_float(vs_config->getVariable("physics","max_shield_recharge_drain","0"));
  static bool max_shield_lowers_capacitance=XMLSupport::parse_bool(vs_config->getVariable("physics","max_shield_lowers_capacitance","true"));
  if (max_shield_lowers_recharge) {
    energy-=max_shield_lowers_recharge*SIMULATION_ATOM*maxshield;
  }
  if (energy_before_shield) {
    RechargeEnergy();
  }
  if (!max_shield_lowers_capacitance) {
    maxshield=0;
  }
  static float low_power_mode = XMLSupport::parse_float(vs_config->getVariable("physics","low_power_mode_energy","10"));
  float menergy = maxenergy;
  if (menergy-maxshield<low_power_mode) {
	  menergy=maxshield+low_power_mode;
	  if (_Universe->isPlayerStarship(this))
		  if (rand()<.00005*RAND_MAX)
			  UniverseUtil::IOmessage(0,"	game","all","**Warning** Power Supply Overdrawn: downgrade shield or purchase reactor capacitance!");
  }
  static int modcounter=0;
  
  if(graphicOptions.InWarp){ //FIXME FIXME FIXME
	  static float bleedfactor = XMLSupport::parse_float(vs_config->getVariable("physics","warpbleed","20"));
	  float bleed=jump.insysenergy/bleedfactor*SIMULATION_ATOM;
	  if(warpenergy>bleed){
		    warpenergy-=bleed;
	  } else {
		  graphicOptions.InWarp=0;
	  }
  }

  if (menergy>maxshield) {
    if (energy>menergy-maxshield) {//allow shields to absorb xtra power
      float excessenergy = energy - (menergy-maxshield);
      energy=menergy-maxshield;  
      if (excessenergy >0) {
		  warpenergy=warpenergy+WARPENERGYMULTIPLIER(this)*excessenergy;
		  float mwe = maxwarpenergy;
		  if (mwe<jump.energy&&mwe==0)
			  mwe = jump.energy;
		  if (warpenergy>mwe)
			  warpenergy=mwe;
      }
    }
  }else {
    energy=0;
  }
}

Vector Unit::ResolveForces (const Transformation &trans, const Matrix &transmat) {
  Vector p, q, r;
  GetOrientation(p,q,r);
  Vector temp1 (NetLocalTorque.i*p+NetLocalTorque.j*q+NetLocalTorque.k *r);
  if (NetTorque.i||NetTorque.j||NetTorque.k) {
    temp1 += InvTransformNormal(transmat,NetTorque);
  }
  if (GetMoment())
	  temp1=temp1/GetMoment();
  else
	  VSFileSystem::vs_fprintf (stderr,"zero moment of inertia %s\n",name.c_str());
  Vector temp (temp1*SIMULATION_ATOM);
  /*  //FIXME  does this shit happen!
      if (FINITE(temp.i)&&FINITE (temp.j)&&FINITE(temp.k)) */
  {
	  if (!FINITE(temp.i)||FINITE (temp.j)||FINITE(temp.k)) {

	  }
    AngularVelocity += temp;
  }
  Vector temp2 = (NetLocalForce.i*p + NetLocalForce.j*q + NetLocalForce.k*r ); //acceleration
  if (!(FINITE(NetForce.i)&&FINITE(NetForce.j)&&FINITE(NetForce.k))) {
	  cout << "NetForce skrewed";
  }

  if (NetForce.i||NetForce.j||NetForce.k) {
    temp2+=InvTransformNormal(transmat,NetForce);
  }
  
  temp2=temp2/GetMass();
  temp = temp2*SIMULATION_ATOM;
  if (!(FINITE(temp2.i)&&FINITE(temp2.j)&&FINITE(temp2.k))) {
	  cout << "NetForce transform skrewed";
  }
  float oldmagsquared = Velocity.MagnitudeSquared();
  /*if (FINITE(temp.i)&&FINITE (temp.j)&&FINITE(temp.k)) */{	//FIXME
    Velocity += temp;
  }
  float newmagsquared = Velocity.MagnitudeSquared();
  static float warpstretchcutoff= XMLSupport::parse_float (vs_config->getVariable( "graphics","warp_stretch_cutoff","500000"))*XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"));
  static float warpstretchoutcutoff= XMLSupport::parse_float (vs_config->getVariable( "graphics","warp_stretch_decel_cutoff","500000"))*XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"));
  static float cutsqr = warpstretchcutoff*warpstretchcutoff;
  static float outcutsqr = warpstretchoutcutoff*warpstretchoutcutoff;
  bool oldbig = oldmagsquared>cutsqr;
  bool newbig = newmagsquared>cutsqr;
  bool oldoutbig = oldmagsquared>outcutsqr;
  bool newoutbig = newmagsquared>outcutsqr;
  if ((newbig&&!oldbig)||(oldoutbig&&!newoutbig)) {

	  static string insys_jump_ani = vs_config->getVariable ("graphics","insys_jump_animation","warp.ani");
	  static bool docache=true;
	  if (docache){
		  UniverseUtil::cacheAnimation (insys_jump_ani);
		  docache=false;
	  }

      Vector v(GetVelocity());
      v.Normalize();
      Vector p,q,r;GetOrientation(p,q,r);
	  static float sec = XMLSupport::parse_float(vs_config->getVariable("graphics","insys_jump_ani_second_ahead","4"))/(XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"))*XMLSupport::parse_float(vs_config->getVariable("physics","game_accel","1")));
	  static float endsec = XMLSupport::parse_float(vs_config->getVariable("graphics","insys_jump_ani_second_ahead_end",".03"))/(XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"))*XMLSupport::parse_float(vs_config->getVariable("physics","game_accel","1")));
	  float tmpsec = oldbig?endsec:sec;
      UniverseUtil::playAnimationGrow (insys_jump_ani,RealPosition(this).Cast()+Velocity*tmpsec+v*rSize(),rSize()*8,1);
//      UniverseUtil::playAnimationGrow (insys_jump_ani,RealPosition(this).Cast()+GetVelocity()*sec+2*v*rSize()+r*4*rSize(),rSize()*16,.97);
	  
	  
  }
  
  static float air_res_coef =XMLSupport::parse_float (active_missions[0]->getVariable ("air_resistance","0"));
  static float lateral_air_res_coef =XMLSupport::parse_float (active_missions[0]->getVariable ("lateral_air_resistance","0"));
    
    if (air_res_coef||lateral_air_res_coef) {
      float velmag = Velocity.Magnitude();
      Vector AirResistance = Velocity*(air_res_coef*velmag/GetMass())*(corner_max.i-corner_min.i)*(corner_max.j-corner_min.j);
      if (AirResistance.Magnitude()>velmag) {
	Velocity.Set(0,0,0);
      }else {
	Velocity = Velocity-AirResistance;
	if (lateral_air_res_coef) {
	  Vector p,q,r;
	  GetOrientation (p,q,r);
	  Vector lateralVel= p*Velocity.Dot (p)+q*Velocity.Dot (q);
	  AirResistance = lateralVel*(lateral_air_res_coef*velmag/GetMass())*(corner_max.i-corner_min.i)*(corner_max.j-corner_min.j);	  
	  if (AirResistance.Magnitude ()> lateralVel.Magnitude()){
	    Velocity = r*Velocity.Dot(r);
	  }else {
	    Velocity = Velocity - AirResistance;
	  }
	}

      }
    }
   
  NetForce = NetLocalForce = NetTorque = NetLocalTorque = Vector(0,0,0);

  /*
    if (fabs (Velocity.i)+fabs(Velocity.j)+fabs(Velocity.k)> co10) {
    float magvel = Velocity.Magnitude(); float y = (1-magvel*magvel*oocc);
    temp = temp * powf (y,1.5);
    }*/

	return temp2;
}
void Unit::SetOrientation (QVector q, QVector r) {
  q.Normalize();
  r.Normalize();
  QVector p;
  CrossProduct (q,r,p);
  CrossProduct (r,p,q);
  curr_physical_state = Transformation (Quaternion::from_vectors (p.Cast(),q.Cast(),r.Cast()),Position());
}
void Unit::SetOrientation(Quaternion Q) {
	curr_physical_state = Transformation ( Q, Position());
}

Vector Unit::UpCoordinateLevel (const Vector &v) const {
  Matrix m;
  curr_physical_state.to_matrix(m);
#define M(A,B) m.r[B*3+A]
  return Vector(v.i*M(0,0)+v.j*M(1,0)+v.k*M(2,0),
		v.i*M(0,1)+v.j*M(1,1)+v.k*M(2,1),
		v.i*M(0,2)+v.j*M(1,2)+v.k*M(2,2));
#undef M
}
Vector Unit::DownCoordinateLevel (const Vector &v) const {
  Matrix m;
  curr_physical_state.to_matrix(m);
  return TransformNormal(m,v);
}

Vector Unit::ToLocalCoordinates(const Vector &v) const {
  //Matrix m;
  //062201: not a cumulative transformation...in prev unit space  curr_physical_state.to_matrix(m);
  
#define M(A,B) cumulative_transformation_matrix.r[B*3+A]
  return Vector(v.i*M(0,0)+v.j*M(1,0)+v.k*M(2,0),
		v.i*M(0,1)+v.j*M(1,1)+v.k*M(2,1),
		v.i*M(0,2)+v.j*M(1,2)+v.k*M(2,2));
#undef M
}

Vector Unit::ToWorldCoordinates(const Vector &v) const {
  return TransformNormal(cumulative_transformation_matrix,v); 
#undef M

}

/***********************************************************************************/
/**** UNIT_DAMAGE STUFF                                                            */
/***********************************************************************************/

// NEW TESTING MODIFICATIONS
// We do it also on client side to display hits on shields/armor -> not to compute damage
// Damage are computed on server side and shield/armor data are sent with the DAMAGE SNAPSHOT
float Unit::ApplyLocalDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedUnit,const GFXColor &color, float phasedamage) {
  static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
  //  #ifdef REALLY_EASY
  Cockpit * cpt;
  if ( (cpt=_Universe->isPlayerStarship(this))!=NULL) {
    if (color.a!=2) {
      //    ApplyDamage (amt);
      static bool apply_difficulty_enemy_damage=XMLSupport::parse_bool(vs_config->getVariable("physics","difficulty_based_enemy_damage","true"));
      if (apply_difficulty_enemy_damage) {
        phasedamage*= g_game.difficulty;
        amt*=g_game.difficulty;
      }
      cpt->Shake (amt);
    }
  }
  //  #endif
  float absamt= amt>=0?amt:-amt;  
  float ppercentage=0;
  // We also do the following lock on client side in order not to display shield hits 
    static bool nodockdamage = XMLSupport::parse_float (vs_config->getVariable("physics","no_damage_to_docked_ships","false"));
    if (nodockdamage) {
      if (DockedOrDocking()&(DOCKED_INSIDE|DOCKED)) {
	    return -1;
	  }
	}
    if (affectedUnit!=this) {
	  affectedUnit->ApplyLocalDamage (pnt,normal,amt,affectedUnit,color,phasedamage);
	  return -1;
	}

  if (aistate)
   aistate->ChooseTarget();

  float leakamt = phasedamage+amt*.01*shield.leak;
  amt *= 1-.01*shield.leak;
  // Percentage returned by DealDamageToShield
  float spercentage = 0;
  // If not a nebula or if shields recharge in nebula => WE COMPUTE SHIELDS DAMAGE AND APPLY
  if (GetNebula()==NULL||(nebshields>0)) {
	// Compute spercentage even in networking because doesn't apply damage on client side
	//if( Network==NULL || SERVER)
    float origabsamt=absamt;
    spercentage = DealDamageToShield (pnt,absamt);
    
    //percentage = DealDamageToShield (pnt,absamt);
    amt = amt>=0?absamt:-absamt;
    if (meshdata.back()&&spercentage>0&&(origabsamt-absamt>shield.recharge||amt==0)){//shields are up
      /*      meshdata[nummesh]->LocalFX.push_back (GFXLight (true,
	      GFXColor(pnt.i+normal.i,pnt.j+normal.j,pnt.k+normal.k),
	      GFXColor (.3,.3,.3), GFXColor (0,0,0,1), 
	      GFXColor (.5,.5,.5),GFXColor (1,0,.01)));*/
      //calculate percentage
      if (GetNebula()==NULL) 
		meshdata.back()->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector(0,0,0),spercentage>1?1:spercentage,color);
    }
  }
  // If shields failing or... => WE COMPUTE DAMAGE TO HULL
  if (shield.leak>0||!meshdata.back()||spercentage==0||absamt>0||phasedamage) {
	// ONLY in server or in non-networking
	// Compute spercentage even in networking because doesn't apply damage on client side
	//if( Network==NULL || SERVER)
		ppercentage = DealDamageToHull (pnt, leakamt+amt);
	if (ppercentage!=-1) {//returns -1 on death--could delete
	  for (int i=0;i<nummesh();i++) {
		if (ppercentage)
			meshdata[i]->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector (0,0,0),ppercentage,color);
	  }
	}
  }
  // If server and there is damage to shields or if unit is not killed (ppercentage>0)
  if( SERVER && (ppercentage>0 || spercentage>0))
  {
#ifdef NET_SHIELD_SYSTEM_1
	  // FIRST METHOD : send each time a unit is hit all the damage info to all clients in the current zone
	  // If server side, we send the unit serial + serialized shields + shield recharge + shield leak + ...
	  Vector netpnt = pnt;
	  Vector netnormal = normal;
	  GFXColor col( color.r, color.g, color.b, color.a);
	  VSServer->sendDamages( this->serial, this->activeStarSystem->GetZone(), shield, armor, ppercentage, spercentage, amt, netpnt, netnormal, col);
	  // This way the client computes damages based on what we send to him => less reliable
	  //VSServer->sendDamages( this->serial, pnt, normal, amt, col, phasedamage);
#else
		// SECOND METHOD : we just put a flag on the unit telling its shield/armor data has changed
		if( spercentage)
			this->damages &= SHIELD_DAMAGED;
		if( ppercentage)
			this->damages &= ARMOR_DAMAGED;
#endif
  }

  return 1;
}

void	Unit::ApplyNetDamage( Vector & pnt, Vector & normal, float amt, float ppercentage, float spercentage, GFXColor & color)
{
  static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
  Cockpit * cpt;
  if ( (cpt=_Universe->isPlayerStarship(this))!=NULL) {
    if (color.a!=2)
      cpt->Shake (amt);
  }
  if( GetNebula()==NULL||nebshields>0)
  {
    if (meshdata.back()&&spercentage>0&&amt==0) {//shields are up
      if (GetNebula()==NULL) 
		meshdata.back()->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector(0,0,0),spercentage,color);
	}
  }
  if (shield.leak>0||!meshdata.back()||spercentage==0||amt>0) {
	if (ppercentage!=-1) {//returns -1 on death--could delete
	  for (int i=0;i<nummesh();i++) {
		if (ppercentage)
			meshdata[i]->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector (0,0,0),ppercentage,color);
	  }
	}
  }
}

extern void ScoreKill (Cockpit * cp, Unit * un, int faction);
// Changed order of things -> Vectors and ApplyLocalDamage are computed before Cockpit thing now
void Unit::ApplyDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedUnit, const GFXColor & color, Unit * ownerDoNotDereference, float phasedamage) {
  Cockpit * cp = _Universe->isPlayerStarship (ownerDoNotDereference);

  // Only on client side
  if (!SERVER && cp) {
      //now we can dereference it because we checked it against the parent
      CommunicationMessage c(ownerDoNotDereference,this,NULL,0);
      c.SetCurrentState(c.fsm->GetHitNode(),NULL,0);
      if (this->getAIState()) this->getAIState()->Communicate (c);      
      Threaten (ownerDoNotDereference,10);//the dark danger is real!
  }
  bool mykilled = hull<0;
  Vector localpnt (InvTransform(cumulative_transformation_matrix,pnt));
  Vector localnorm (ToLocalCoordinates (normal));
  // Only call when on servre side or non-networking
  // If networking damages are applied as they are received
  if( SERVER || Network==NULL)
	ApplyLocalDamage(localpnt, localnorm, amt,affectedUnit,color,phasedamage);
  if (hull<0) {
	  ClearMounts();
	  if (!mykilled) {
		  if (cp) {
			  ScoreKill (cp,ownerDoNotDereference,faction);
		  }
	  }
  }
}

// NUMGAUGES has been moved to images.h in UnitImages
void Unit::DamageRandSys(float dam, const Vector &vec, float randnum, float degrees) {
	float deg = fabs(180*atan2 (vec.i,vec.k)/M_PI);
	//float randnum=rand01();
	//float degrees=deg;
	randnum=rand01();
	degrees=deg;
	if (degrees>180) {
		degrees=360-degrees;
	}
	if (degrees>=0&&degrees<20) {
		//DAMAGE COCKPIT
		if (randnum>=.85) {
			computer.set_speed=(rand01()*computer.max_speed()*(5/3))-(computer.max_speed()*(2/3)); //Set the speed to a random speed
		} else if (randnum>=.775) {
			computer.itts=false; //Set the computer to not have an itts
		} else if (randnum>=.7) {
			computer.radar.color=false; //set the radar to not have color
		} else if (randnum>=.5) {
			// THIS IS NOT YET SUPPORTED IN NETWORKING
			computer.target=NULL; //set the target to NULL
		} else if (randnum>=.4) {
			limits.retro*=dam;
		} else if (randnum>=.3275) {
			computer.radar.maxcone+=(1-dam);
			if (computer.radar.maxcone>.9)
				computer.radar.maxcone=.9;
		}else if (randnum>=.325) {
			computer.radar.lockcone+=(1-dam);
			if (computer.radar.lockcone>.95)
				computer.radar.lockcone=.95;
		} else if (randnum>=.25) {
			computer.radar.trackingcone+=(1-dam);
			if (computer.radar.trackingcone>.98)
				computer.radar.trackingcone=.98;
		} else if (randnum>=.175) {
			computer.radar.maxrange*=dam;
		} else {
		  int which= rand()%(1+UnitImages::NUMGAUGES+MAXVDUS);
		  image->cockpit_damage[which]*=dam;
		  if (image->cockpit_damage[which]<.1) {
		    image->cockpit_damage[which]=0;
		  }
		}
		damages &= COMPUTER_DAMAGED;
		return;
	}
	if (degrees>=20&&degrees<35) {
		//DAMAGE MOUNT
		if (randnum>=.65&&randnum<.9) {
			image->ecm*=dam;
		} else if (GetNumMounts()) {
			unsigned int whichmount=rand()%GetNumMounts();
			if (randnum>=.9) {
				mounts[whichmount].status=Mount::DESTROYED;
			}else if (mounts[whichmount].ammo>0&&randnum>=.4) {
			  mounts[whichmount].ammo*=dam;
			} else if (randnum>=.1) {
				mounts[whichmount].time_to_lock+=(100-(100*dam));
			} else {
				mounts[whichmount].size&=(~weapon_info::AUTOTRACKING);
			}
		}
		damages &= MOUNT_DAMAGED;
		return;
	}
	if (degrees>=35&&degrees<60) {
		//DAMAGE FUEL
		if (randnum>=.75) {
			fuel*=dam;
		} else if (randnum>=.5) {
			this->afterburnenergy+=((1-dam)*recharge);
		} else if (randnum>=.25) {
			image->cargo_volume*=dam;
		} else {  //Do something NASTY to the cargo
			if (image->cargo.size()>0) {
				int i=0;
				unsigned int cargorand;
				do {
					cargorand=rand()%image->cargo.size();
				} while (image->cargo[cargorand].quantity!=0&&++i<image->cargo.size());
				image->cargo[cargorand].quantity*=dam;
			}
		}
		damages &= CARGOFUEL_DAMAGED;
		return;
	}
	if (degrees>=60&&degrees<90) {
		//DAMAGE ROLL/YAW/PITCH/THRUST
		if (randnum>=.8) {
			computer.max_pitch*=dam;
		} else if (randnum>=.6) {
			computer.max_yaw*=dam;
		} else if (randnum>=.55) {
			computer.max_roll*=dam;
		} else if (randnum>=.5) {
			limits.roll*=dam;
		} else if (randnum>=.3) {
			limits.yaw*=dam;
		} else if (randnum>=.1) {
			limits.pitch*=dam;
		} else {
			limits.lateral*=dam;
		}
		damages &= LIMITS_DAMAGED;
		return;
	}
	if (degrees>=90&&degrees<120) {
		//DAMAGE Shield
		//DAMAGE cloak
		if (randnum>=.95) {
			this->cloaking=-1;
		} else if (randnum>=.78) {
			image->cloakenergy+=((1-dam)*recharge);
		} else if (randnum>=.7) {
			cloakmin+=(rand()%(32000-cloakmin));
		}
		switch (shield.number) {
		case 2:
			if (randnum>=.25&&randnum<.75) {
				shield.shield2fb.frontmax*=dam;
			} else {
				shield.shield2fb.backmax*=dam;
			}
			break;
		case 4:
			if (randnum>=.5&&randnum<.75) {
				shield.shield4fbrl.frontmax*=dam;
			} else if (randnum>=.75) {
				shield.shield4fbrl.backmax*=dam;
			} else if (randnum>=.25) {
				shield.shield4fbrl.leftmax*=dam;
			} else {
				shield.shield4fbrl.rightmax*=dam;
			}
			break;
		case 8:
			if (randnum<.125) {
				shield.shield8.frontrighttopmax*=dam;
			} else if (randnum<.25) {
				shield.shield8.backrighttopmax*=dam;
			} else if (randnum<.375) {
				shield.shield8.frontlefttopmax*=dam;
			} else if (randnum<.5) {
				shield.shield8.backrighttopmax*=dam;
			} else if (randnum<.625) {
				shield.shield8.frontrightbottommax*=dam;
			} else if (randnum<.75) {
				shield.shield8.backrightbottommax*=dam;
			} else if (randnum<.875) {
				shield.shield8.frontleftbottommax*=dam;
			} else {
				shield.shield8.backrightbottommax*=dam;
			}
			break;
		}
		damages &= CLOAK_DAMAGED;
		return;
	}
	if (degrees>=120&&degrees<150) {
		//DAMAGE Reactor
		//DAMAGE JUMP
		if (randnum>=.9) {
			shield.leak=(char)((randnum-.9)*900);
			if (shield.leak<0)
				shield.leak=0;
			if (shield.leak>100)
				shield.leak=100;
		} else if (randnum>=.7) {
			shield.recharge*=dam;
		} else if (randnum>=.5) {
			this->recharge*=dam;
		} else if (randnum>=.3) {
			this->maxenergy*=dam;
		} else if (randnum>=.2) {
			this->jump.energy*=(2-dam);
		} else if (randnum>=.03){
			this->jump.damage+=100*(1-dam);
		} else {
		  if (image->repair_droid>0) {
		    image->repair_droid--;
		  }
		}
		damages &= JUMP_DAMAGED;
		return;
	}
	if (degrees>=150&&degrees<=180) {
		//DAMAGE ENGINES
		if (randnum>=.8) {
			computer.max_combat_ab_speed*=dam;
		} else if (randnum>=.6) {
			computer.max_combat_speed*=dam;
		} else if (randnum>=.4) {
			limits.afterburn*=dam;
		} else if (randnum>=.2) {
			limits.vertical*=dam;
		} else {
			limits.forward*=dam;
		}
		damages &= LIMITS_DAMAGED;
		return;
	}
}

void Unit::Kill(bool erasefromsave, bool quitting) {

  //if (erasefromsave)
  //  _Universe->AccessCockpit()->savegame->RemoveUnitFromSave((long)this);

  if (this->colTrees)
    this->colTrees->Dec();//might delete
  this->colTrees=NULL;
  for (int beamcount=0;beamcount<GetNumMounts();beamcount++) {
    AUDStopPlaying(mounts[beamcount].sound);
    AUDDeleteSound(mounts[beamcount].sound);
    if (mounts[beamcount].ref.gun&&mounts[beamcount].type->type==weapon_info::BEAM)
      delete mounts[beamcount].ref.gun;//hope we're not killin' em twice...they don't go in gunqueue
  }

  if (docked&(DOCKING_UNITS)) {
    vector <Unit *> dockedun;
    unsigned int i;
    for (i=0;i<image->dockedunits.size();i++) {
      Unit * un;
      if (NULL!=(un=image->dockedunits[i]->uc.GetUnit())) 
	dockedun.push_back (un);
    }
    while (!dockedun.empty()) {
      dockedun.back()->UnDock(this);
      dockedun.pop_back();
    }
  }

  #ifndef NO_MOUNT_STAR
	for( vector<Mount *>::iterator jj=mounts.begin(); jj!=mounts.end(); jj++)
	{
		// Free all mounts elements
		if( (*jj)!=NULL)
			delete (*jj);
	}
	#endif
    mounts.clear();
  //eraticate everything. naturally (see previous line) we won't erraticate beams erraticated above
  if (!isSubUnit()) 
    RemoveFromSystem();
  killed = true;
  computer.target.SetUnit (NULL);

  //God I can't believe this next line cost me 1 GIG of memory until I added it
  computer.threat.SetUnit (NULL);
  computer.velocity_ref.SetUnit(NULL);

	  if(aistate) {
	    aistate->ClearMessages();
	    aistate->Destroy();
	  }
  aistate=NULL;
  UnitCollection::UnitIterator iter = getSubUnits();
  Unit *un;
  while ((un=iter.current())) {
    un->Kill();
    iter.advance();
  }
  if (ucref==0) {
    Unitdeletequeue.push_back(this);
  if (flightgroup) {
    if (flightgroup->leader.GetUnit()==this) {
      flightgroup->leader.SetUnit(NULL);
    }
  }

#ifdef DESTRUCTDEBUG
    VSFileSystem::vs_fprintf (stderr,"%s 0x%x - %d\n",name.c_str(),this,Unitdeletequeue.size());
#endif
  }
}

void Unit::leach (float damShield, float damShieldRecharge, float damEnRecharge) {
  recharge*=damEnRecharge;
  shield.recharge*=damShieldRecharge;
  switch (shield.number) {
  case 2:
    shield.shield2fb.frontmax*=damShield;
    shield.shield2fb.backmax*=damShield;
    break;
  case 4:
    shield.shield4fbrl.frontmax*=damShield;
    shield.shield4fbrl.backmax*=damShield;
    shield.shield4fbrl.leftmax*=damShield;
    shield.shield4fbrl.rightmax*=damShield;
    break;
  case 8:
    shield.shield8.frontrighttopmax*=damShield;
    shield.shield8.backrighttopmax*=damShield;
    shield.shield8.frontlefttopmax*=damShield;
    shield.shield8.backlefttopmax*=damShield;
	shield.shield8.frontrightbottommax*=damShield;
    shield.shield8.backrightbottommax*=damShield;
    shield.shield8.frontleftbottommax*=damShield;
    shield.shield8.backleftbottommax*=damShield;
    break;
  }
}

void Unit::UnRef() {
#ifdef CONTAINER_DEBUG
  CheckUnit(this);
#endif
  ucref--;
  if (killed&&ucref==0) {
    deletedUn.Put ((long)this,this);
    Unitdeletequeue.push_back(this);//delete
#ifdef DESTRUCTDEBUG
    VSFileSystem::vs_fprintf (stderr,"%s 0x%x - %d\n",name.c_str(),this,Unitdeletequeue.size());
#endif
  }
}

float Unit::ExplosionRadius() {
  static float expsize=XMLSupport::parse_float(vs_config->getVariable ("graphics","explosion_size","3"));
  return expsize*rSize();
}

void Unit::ArmorData (float armor[8]) const{  //short fix
  armor[0]=this->armor.frontrighttop;
  armor[1]=this->armor.backrighttop;
  armor[2]=this->armor.frontlefttop;
  armor[3]=this->armor.backlefttop;
  armor[4]=this->armor.frontrightbottom;
  armor[5]=this->armor.backrightbottom;
  armor[6]=this->armor.frontleftbottom;
  armor[7]=this->armor.backleftbottom;
}

float Unit::FuelData () const{
  return fuel;
}
float Unit::WarpEnergyData() const {
  if (maxwarpenergy>0)
    return ((float)warpenergy)/((float)maxwarpenergy);
  else
    return ((float)warpenergy)/((float)jump.energy);
}
float Unit::EnergyData() const{
  static bool max_shield_lowers_capacitance=XMLSupport::parse_bool(vs_config->getVariable("physics","max_shield_lowers_capacitance","true"));
  if (max_shield_lowers_capacitance) {
    if (maxenergy<=totalShieldEnergyCapacitance(shield)) {
      return 0;
    }
    return ((float)energy)/(maxenergy-totalShieldEnergyCapacitance(shield));
  }else {
    return ((float)energy)/maxenergy;
  }
}

float Unit::FShieldData() const{
  switch (shield.number) {
  case 2: { if( shield.shield2fb.frontmax!=0) return shield.shield2fb.front/shield.shield2fb.frontmax;}
  case 4: { if( shield.shield4fbrl.frontmax!=0) return (shield.shield4fbrl.front)/shield.shield4fbrl.frontmax;}
  case 8: { if( shield.shield8.frontrighttopmax!=0||shield.shield8.frontrightbottommax!=0||shield.shield8.frontlefttopmax!=0||shield.shield8.frontleftbottommax!=0) return (shield.shield8.frontrighttop+shield.shield8.frontrightbottom+shield.shield8.frontlefttop+shield.shield8.frontleftbottom)/(shield.shield8.frontrighttopmax+shield.shield8.frontrightbottommax+shield.shield8.frontlefttopmax+shield.shield8.frontleftbottommax);}
  }
  return 0;
}
float Unit::BShieldData() const{
  switch (shield.number) {
  case 2: { if( shield.shield2fb.backmax!=0) return shield.shield2fb.back/shield.shield2fb.backmax;}
  case 4: { if( shield.shield4fbrl.backmax!=0) return (shield.shield4fbrl.back)/shield.shield4fbrl.backmax;}
  case 8: { if( shield.shield8.backrighttopmax!=0||shield.shield8.backrightbottommax!=0||shield.shield8.backlefttopmax!=0||shield.shield8.backleftbottommax!=0) return (shield.shield8.backrighttop+shield.shield8.backrightbottom+shield.shield8.backlefttop+shield.shield8.backleftbottom)/(shield.shield8.backrighttopmax+shield.shield8.backrightbottommax+shield.shield8.backlefttopmax+shield.shield8.backleftbottommax);}
  }
  return 0;
}
float Unit::LShieldData() const{
  switch (shield.number) {
  case 2: return 0;//no data, captain
  case 4: { if( shield.shield4fbrl.leftmax!=0) return (shield.shield4fbrl.left)/shield.shield4fbrl.leftmax;}
  case 8: { if( shield.shield8.backlefttopmax!=0||shield.shield8.backleftbottommax!=0||shield.shield8.frontlefttopmax!=0||shield.shield8.frontleftbottommax!=0) return (shield.shield8.backlefttop+shield.shield8.backleftbottom+shield.shield8.frontlefttop+shield.shield8.frontleftbottom)/(shield.shield8.backlefttopmax+shield.shield8.backleftbottommax+shield.shield8.frontlefttopmax+shield.shield8.frontleftbottommax);}
  }
  return 0;
}
float Unit::RShieldData() const{
  switch (shield.number) {
  case 2: return 0;//don't react to stuff we have no data on
  case 4: { if( shield.shield4fbrl.rightmax!=0) return (shield.shield4fbrl.right)/shield.shield4fbrl.rightmax;}
  case 8: { if( shield.shield8.backrighttopmax!=0||shield.shield8.backrightbottommax!=0||shield.shield8.frontrighttopmax!=0||shield.shield8.frontrightbottommax!=0) return (shield.shield8.backrighttop+shield.shield8.backrightbottom+shield.shield8.frontrighttop+shield.shield8.frontrightbottom)/(shield.shield8.backrighttopmax+shield.shield8.backrightbottommax+shield.shield8.frontrighttopmax+shield.shield8.frontrightbottommax);}
  }
  return 0;
}

void Unit::ProcessDeleteQueue() {
  while (!Unitdeletequeue.empty()) {
#ifdef DESTRUCTDEBUG
    VSFileSystem::vs_fprintf (stderr,"Eliminatin' 0x%x - %d",Unitdeletequeue.back(),Unitdeletequeue.size());
    fflush (stderr);
    VSFileSystem::vs_fprintf (stderr,"Eliminatin' %s\n",Unitdeletequeue.back()->name.c_str());
#endif
#ifdef DESTRUCTDEBUG
    if (Unitdeletequeue.back()->isSubUnit()){

      VSFileSystem::vs_fprintf (stderr,"Subunit Deleting (related to double dipping)");

    }
#endif
    Unit * mydeleter = Unitdeletequeue.back();
    Unitdeletequeue.pop_back();
    delete mydeleter;///might modify unitdeletequeue
    
#ifdef DESTRUCTDEBUG
    VSFileSystem::vs_fprintf (stderr,"Completed %d\n",Unitdeletequeue.size());
    fflush (stderr);
#endif

  }
}
bool DestroySystem (float hull, float maxhull, float numhits) {
	static float damage_chance=XMLSupport::parse_float(vs_config->getVariable ("physics","damage_chance",".005"));
	float chance = 1-(damage_chance*(maxhull-hull)/maxhull);
	if (numhits>1)
		chance=pow (chance,numhits);
	return (rand01()>chance);
}
bool DestroyPlayerSystem (float hull, float maxhull, float numhits) {
	static float damage_chance=XMLSupport::parse_float(vs_config->getVariable ("physics","damage_player_chance",".5"));
	float chance = 1-(damage_chance*(maxhull-hull)/maxhull);
	if (numhits>1)
		chance=pow (chance,numhits);
	bool ret = (rand01()>chance);
	if (ret) {
//		printf("DAAAAAAMAGED!!!!\n");
	}
	return ret;
}
static const char * DamagedCategory="upgrades/Damaged/";
float Unit::DealDamageToHullReturnArmor (const Vector & pnt, float damage, float * &targ) { //short fix
  float percent;
#ifndef ISUCK
  if (hull<0) {
    return -1;
  }
#endif
  if (pnt.i>0) {
    if (pnt.j>0) {
		if(pnt.k>0){
			targ=&armor.frontlefttop;
		} else {
			targ=&armor.backlefttop;
		}
	} else {
		if(pnt.k>0){
			targ=&armor.frontleftbottom;
		} else {
			targ=&armor.backleftbottom;
		}
	}
  }else {
    if (pnt.j>0) {
		if(pnt.k>0){
			targ=&armor.frontrighttop;
		} else {
			targ=&armor.backrighttop;
		}
	} else {
		if(pnt.k>0){
			targ=&armor.frontrightbottom;
		} else {
			targ=&armor.backrightbottom;
		}
	}
  }
  unsigned int biggerthan=*targ; //short fix
  float absdamage = damage>=0?damage:-damage;
  percent = absdamage/(*targ+hull);

  // ONLY APLY DAMAGE ON SERVER SIDE
  if( Network==NULL || SERVER)
  {
		  if( percent == -1)
			  return -1;
		  if (absdamage<*targ) {
			ArmorDamageSound( pnt);
			*targ -= apply_float_to_unsigned_int(absdamage);  //short fix
		  }else {
			HullDamageSound( pnt);
			absdamage -= *targ;
			damage= damage>=0?absdamage:-absdamage;
			*targ= 0;
			if (numCargo()>0) {
				if (DestroySystem(hull,maxhull,numCargo())) {
					int which = rand()%numCargo();
					if (GetCargo(which).category.find("upgrades/")==0&& GetCargo(which).category.find(DamagedCategory)!=0) {
						int lenupgrades = strlen("upgrades/");
						GetCargo(which).category = string(DamagedCategory)+GetCargo(which).category.substr(lenupgrades);
					}
				}
			}
			bool isplayer = _Universe->isPlayerStarship(this);
			if ((!isplayer)||_Universe->AccessCockpit()->godliness<=0||hull>damage) {//hull > damage is similar to hull>absdamage|| damage<0	
				static float system_failure=XMLSupport::parse_float(vs_config->getVariable ("physics","indiscriminate_system_destruction",".25"));
				if ((!isplayer)&&DestroySystem(hull,maxhull,1)) {
					
					DamageRandSys(system_failure*rand01()+(1-system_failure)*(1-(absdamage/hull)),pnt);
				}else if (isplayer&&DestroyPlayerSystem(hull,maxhull,1)) {
					DamageRandSys(system_failure*rand01()+(1-system_failure)*(1-(absdamage/hull)),pnt);
				}
			  if (damage>0) {
				  hull -=damage;//FIXME
			  }else {
				  recharge+=damage;
				  shield.recharge+=damage;
				  if (recharge<0)
					  recharge=0;
				  if (shield.recharge<0)
					  shield.recharge=0;
			  }
			}else {
			  _Universe->AccessCockpit()->godliness-=absdamage;
			  if (DestroyPlayerSystem(hull,maxhull,1)) {
				  DamageRandSys(rand01()*.5+.2,pnt);//get system damage...but live!
			  }
			}
		  }
		  if (*targ>biggerthan)
			  VSFileSystem::vs_fprintf (stderr,"errore fatale mit den armorn");
		  if (hull <0) {
			  static float hulldamtoeject = XMLSupport::parse_float(vs_config->getVariable ("physics","hull_damage_to_eject","100"));
			if (!isSubUnit()&&hull>-hulldamtoeject) {
			  static float autoejectpercent = XMLSupport::parse_float(vs_config->getVariable ("physics","autoeject_percent",".5"));

			  static float cargoejectpercent = XMLSupport::parse_float(vs_config->getVariable ("physics","eject_cargo_percent",".25"));
			  if (rand()<(RAND_MAX*autoejectpercent)&&isUnit()==UNITPTR) {
			EjectCargo ((unsigned int)-1);
			  }
			  for (unsigned int i=0;i<numCargo();i++) {
			if (rand()<(RAND_MAX*cargoejectpercent)) {
			  EjectCargo(i);
			}
			  }
			}
		#ifdef ISUCK
			Destroy();
		#endif
			PrimeOrders();
			maxenergy=energy=0;

			Split (rand()%3+1);
		#ifndef ISUCK
			Destroy();
			return -1;
		#endif
  		  }
  }
  /////////////////////////////
  if (!FINITE (percent))
    percent = 0;
  return percent;
}

float Unit::DealDamageToShield (const Vector &pnt, float &damage) {
  int index;
  float percent=0;
  float * targ=NULL; //short fix
  
  // ONLY APPLY DAMAGES IN NON-NETWORKING OR ON SERVER SIDE
  switch (shield.number){
  case 2:
    index=(pnt.k>0)?0:1;
	if(index){
		if( shield.shield2fb.backmax!=0)
			percent = damage/shield.shield2fb.backmax;//comparing with max
		else
			percent = 0;
		if( Network==NULL || SERVER)
		{
   			shield.shield2fb.back-=damage;
			damage =0;
			if (shield.shield2fb.back<0) {
				damage = -shield.shield2fb.back;
				shield.shield2fb.back=0;
			}
		}
	} else {
		if( shield.shield2fb.frontmax!=0)
			percent = damage/shield.shield2fb.frontmax;//comparing with max
		else
			percent = 0;
				if( Network==NULL || SERVER)
		{
   			shield.shield2fb.front-=damage;
			damage =0;
			if (shield.shield2fb.front<0) {
				damage = -shield.shield2fb.front;
				shield.shield2fb.front=0;
			}
		}

	}

	
    break;
  case 8:
      if (pnt.i>0) {
		if(pnt.j>0){
		  if(pnt.k>0){
			  percent = damage/shield.shield8.frontlefttopmax;
			  targ=&shield.shield8.frontlefttop;
		  } else {
			  percent = damage/shield.shield8.backlefttopmax;
			  targ=&shield.shield8.backlefttop;
		  }
		} else {
		  if(pnt.k>0){
			  percent = damage/shield.shield8.frontleftbottommax;
			  targ=&shield.shield8.frontleftbottom;
		  } else {
			  percent = damage/shield.shield8.backleftbottommax;
			  targ=&shield.shield8.backleftbottom;
		  }
		}
	  } else {
		if(pnt.j>0){
		  if(pnt.k>0){
			  percent = damage/shield.shield8.frontrighttopmax;
			  targ=&shield.shield8.frontrighttop;
		  } else {
			  percent = damage/shield.shield8.backrighttopmax;
			  targ=&shield.shield8.backrighttop;
		  }
		} else {
		  if(pnt.k>0){
			  percent = damage/shield.shield8.frontrightbottommax;
			  targ=&shield.shield8.frontrightbottom;
		  } else {
			  percent = damage/shield.shield8.backrightbottommax;
			  targ=&shield.shield8.backrightbottom;
		  }
		}
	  }
    
		


	if( Network==NULL || SERVER)
	{
      if (damage>*targ) {
        damage -= *targ;
          *targ=0;
      } else {
          *targ-=damage; //short fix
        damage = 0;
      }
	}
    break;
  case 4:
  default:
    if (fabs(pnt.k)>fabs (pnt.i)) {
      if (pnt.k>0) {
	targ = &shield.shield4fbrl.front;
	percent = damage/shield.shield4fbrl.frontmax;
      } else {
	targ = &shield.shield4fbrl.back;
	percent = damage/shield.shield4fbrl.backmax;
      }
    } else {
      if (pnt.i>0) {
	percent = damage/shield.shield4fbrl.leftmax;
	targ = &shield.shield4fbrl.left;
      } else {
	targ = &shield.shield4fbrl.right;
	percent = damage/shield.shield4fbrl.rightmax;
      }
    }
	if( Network==NULL || SERVER)
	{
      if (damage>*targ) {
        damage-=*targ;
          *targ=0;
      } else {
          *targ -= (damage); //short fix	
        damage=0;
      }
	}
    break;
  }
  if (!FINITE (percent))
    percent = 0;
  return percent;
}

bool Unit::ShieldUp (const Vector &pnt) const{
  const int shieldmin=5;
  int index;
  static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
  if (nebula!=NULL||nebshields>0)
    return false;
  switch (shield.number){
  case 2:
	  return ((pnt.k>0)?(shield.shield2fb.front):(shield.shield2fb.back))>shieldmin;
    break;
  case 8:
if (pnt.i>0) {
		if(pnt.j>0){
		  if(pnt.k>0){
			  return shield.shield8.frontlefttop>shieldmin;
		  } else {
			  return shield.shield8.backlefttop>shieldmin;
		  }
		} else {
		  if(pnt.k>0){
			  return shield.shield8.frontleftbottom>shieldmin;
		  } else {
			  return shield.shield8.backleftbottom>shieldmin;
		  }
		}
	  } else {
		if(pnt.j>0){
		  if(pnt.k>0){
			  return shield.shield8.frontrighttop>shieldmin;
		  } else {
			  return shield.shield8.backrighttop>shieldmin;
		  }
		} else {
		  if(pnt.k>0){
			  return shield.shield8.frontrightbottom>shieldmin;
		  } else {
			  return shield.shield8.backrightbottom>shieldmin;
		  }
		}
	  }   

    break;
  case 4:
  default:
    if (fabs(pnt.k)>fabs (pnt.i)) {
      if (pnt.k>0) {
	return shield.shield4fbrl.front>shieldmin;
      } else {
	return shield.shield4fbrl.back>shieldmin;
      }
    } else {
      if (pnt.i>0) {
	return shield.shield4fbrl.left>shieldmin;
      } else {
	return shield.shield4fbrl.right>shieldmin;
      }
    }
    return false;
  }
}


/***********************************************************************************/
/**** UNIT_WEAPON STUFF                                                            */
/***********************************************************************************/

void Unit::TargetTurret (Unit * targ) {
	if (!SubUnits.empty()) {
		un_iter iter = getSubUnits();
		Unit * su;
		bool inrange = (targ!=NULL)?InRange(targ):true;
        if (inrange) {
          while ((su=iter.current())) {
			su->Target (targ);
			su->TargetTurret(targ);
			iter.advance();
		  }
        }
	}
}

// WARNING : WHEN TURRETS WE MAY NOT WANT TO ASK THE SERVER FOR INFOS ! ONLY FOR LOCAL PLAYERS (_Universe-isStarship())
void Unit::Target (Unit *targ) {
  if (targ==this) {
    return;
  }
  if (!(activeStarSystem==NULL||activeStarSystem==_Universe->activeStarSystem())) {
    computer.target.SetUnit(NULL);
    return;
    VSFileSystem::vs_fprintf (stderr,"bad target system");
    const int BADTARGETSYSTEM=0;
    assert (BADTARGETSYSTEM);
  }
  if (targ) {
    if (targ->activeStarSystem==_Universe->activeStarSystem()||targ->activeStarSystem==NULL) {
		if (targ!=Unit::Target()) {
        for (int i=0;i<GetNumMounts();i++){ 
  	  mounts[i].time_to_lock = mounts[i].type->LockTime;
        }
        computer.target.SetUnit(targ);
	LockTarget(false);
      }
    }else {
      if (jump.drive!=-1) {
	un_iter i= _Universe->activeStarSystem()->getUnitList().createIterator();
	Unit * u;
	for (;(u=*i)!=NULL;i++) {
	  if (!u->GetDestinations().empty()) {
	    if (std::find (u->GetDestinations().begin(),u->GetDestinations().end(),targ->activeStarSystem->getFileName())!=u->GetDestinations().end()) {
	      Target (u);
	      ActivateJumpDrive(0);
	    }
	  }
	}
      }else {
	computer.target.SetUnit(NULL);
      }
    }
  }else {
    computer.target.SetUnit(NULL);
  }
}
void Unit::VelocityReference (Unit *targ) {
  computer.velocity_ref.SetUnit(targ);
}
void Unit::SetOwner(Unit *target) {
  owner=target;
}

void Unit::Cloak (bool loak) {
  if (loak) {
    if (image->cloakenergy<warpenergy) {
      image->cloakrate =(image->cloakrate>=0)?image->cloakrate:-image->cloakrate; 
      if (cloaking<-1) { //short fix
	cloaking=2147483647; //short fix
      } else {
       
      }
    }
  }else {
    image->cloakrate= (image->cloakrate>=0)?-image->cloakrate:image->cloakrate;
    if (cloaking==cloakmin)
      cloaking++;
  }
}

void Unit::SelectAllWeapon (bool Missile) {
  for (int i=0;i<GetNumMounts();i++) {
	  if (mounts[i].status<Mount::DESTROYED) {
		  if (mounts[i].type->size!=weapon_info::SPECIAL) {
			  mounts[i].Activate (Missile);
		  }
	  }
  }
}

void Unit::UnFire () {
  for (int i=0;i<GetNumMounts();i++) {
    mounts[i].UnFire();//turns off beams;
  }
}

///cycles through the loop twice turning on all matching to ms weapons of size or after size
void Unit::ActivateGuns (const weapon_info * sz, bool ms) {
  for (int j=0;j<2;j++) {
    for (int i=0;i<GetNumMounts();i++) {
      if (mounts[i].type==sz) {
	if (mounts[i].status<Mount::DESTROYED&&mounts[i].ammo!=0&&(mounts[i].type->type==weapon_info::PROJECTILE)==ms) {
	  mounts[i].Activate(ms);
	}else {
	  sz = mounts[(i+1)%GetNumMounts()].type;
	}
      }
    }
  }
}

typedef std::set<int> WeaponGroup;

template<bool FORWARD> class WeaponComparator {
public:
	bool operator() (const WeaponGroup &a, const WeaponGroup &b) const {
		if (a.size()==b.size()) {
			for (WeaponGroup::const_iterator iterA=a.begin(), iterB=b.begin();
					iterA!=a.end()&&iterB!=b.end();
					++iterA, ++iterB) {
				if ((*iterA)<(*iterB)) {
					return FORWARD;
				} else if ((*iterB)<(*iterA)) {
					return (!FORWARD);
				}
			}
			return false;
		} else if (a.size()<b.size()) {
			return FORWARD;
		} else {
			return (!FORWARD);
		}
	}

	typedef std::set<WeaponGroup, WeaponComparator<FORWARD> > WeaponGroupSet;

	static bool checkmount(Unit *un, int i, bool missile) {
		return (un->mounts[i].status<Mount::DESTROYED&&((un->mounts[i].type->type==weapon_info::PROJECTILE)==missile)&&un->mounts[i].ammo!=0);
	}

	static bool isSpecial(const Mount &mount) {
		return mount.type->size==weapon_info::SPECIAL||mount.type->size==weapon_info::SPECIALMISSILE;
	}
	
	static bool notSpecial(const Mount &mount) {
		return !isSpecial(mount);
	}
	
	static void ToggleWeaponSet(Unit *un, bool missile) {
		if (un->mounts.size()==0) {
			return;
		}
		WeaponGroup allWeapons;
		WeaponGroup allWeaponsNoSpecial;
		WeaponGroupSet myset;
		unsigned int i;
		typename WeaponGroupSet::const_iterator iter;
		printf("ToggleWeaponSet: %s\n", FORWARD?"true":"false");
		for (i=0;i<un->mounts.size();i++) {
			if (checkmount(un,i,missile)) {
				WeaponGroup mygroup;
				for (unsigned int j=0;j<un->mounts.size();j++) {
					if (un->mounts[j].type==un->mounts[i].type) {
						if (checkmount(un,j,missile)) {
							mygroup.insert(j);
						}
					}
				}
				myset.insert(mygroup); // WIll ignore if already there.
				allWeapons.insert(i);
				if (notSpecial(un->mounts[i])) {
					allWeaponsNoSpecial.insert(i);
				}
			}
		}
		const WeaponGroupSet mypairset (myset);
		for (iter=mypairset.begin();iter!=mypairset.end();++iter) {
			if ((*iter).size()&&notSpecial(un->mounts[(*((*iter).begin()))])) {
				typename WeaponGroupSet::const_iterator iter2;
				for (iter2=mypairset.begin();iter2!=mypairset.end();++iter2) {
					if ((*iter2).size()&&notSpecial(un->mounts[(*((*iter2).begin()))])) {
						WeaponGroup myGroup;
						set_union((*iter).begin(), (*iter).end(), (*iter2).begin(), (*iter2).end(),
							inserter(myGroup, myGroup.begin()));
						myset.insert(myGroup);
					}
				}
			}
		}
		myset.insert(allWeapons);
		myset.insert(allWeaponsNoSpecial);
		for (iter=myset.begin();iter!=myset.end();++iter) {
			for (WeaponGroup::const_iterator iter2=(*iter).begin();iter2!=(*iter).end();++iter2) {
				printf("%d:%s ", *iter2, un->mounts[*iter2].type->weapon_name.c_str());
			}
			printf("\n");
		}
		WeaponGroup activeWeapons;
		printf("CURRENT: ");
		for (i=0;i<un->mounts.size();++i) {
			if (un->mounts[i].status==Mount::ACTIVE&&checkmount(un,i,missile)) {
				activeWeapons.insert(i);
				printf("%d:%s ", i, un->mounts[i].type->weapon_name.c_str());
			}
		}
		printf("\n");
		iter=myset.upper_bound(activeWeapons);
		if (iter==myset.end()) {
			iter=myset.begin();
		}
		if (iter==myset.end()) {
			return;
		}
		for (i=0;i<un->mounts.size();++i) {
			un->mounts[i].DeActive(missile);
		}
		printf("ACTIVE: ");
		for (WeaponGroup::const_iterator iter2=(*iter).begin();iter2!=(*iter).end();++iter2) {
			printf("%d:%s ", *iter2, un->mounts[*iter2].type->weapon_name.c_str());
			un->mounts[*iter2].Activate(missile);
		}
		printf("\n");
		printf("ToggleWeapon end...\n");
	}
};

void Unit::ToggleWeapon (bool missile, bool forward) {
	if (forward) {
		WeaponComparator<true>::ToggleWeaponSet(this, missile);
	} else {
		WeaponComparator<false>::ToggleWeaponSet(this, missile);
	}
}

/*
///In short I have NO CLUE how this works! It just...grudgingly does
void Unit::ToggleWeapon (bool Missile) {
  int activecount=0;
  int totalcount=0;
  bool lasttotal=true;
//  weapon_info::MOUNT_SIZE sz = weapon_info::NOWEAP;
  const weapon_info * sz=NULL;
  if (GetNumMounts()<1)
    return;
  sz = mounts[0].type;
  if (Missile) {
	  int whichmissile=-2;//-2 means not choosen -1 means all
	  int lastmissile=-2;
	  int count=0;
	  for (int i=0;i<GetNumMounts();++i) {
		  if (mounts[i].type->type==weapon_info::PROJECTILE&&mounts[i].status<Mount::DESTROYED) {
			  if( mounts[i].status==Mount::ACTIVE) {
				  if (whichmissile==-2) {
					  whichmissile=count;
				  }else {
					  whichmissile=-1;
				  }
			  }
			  lastmissile=count;
			  count++;
		  }
	  }
	  if (lastmissile!=-2) {
		  bool found=false;
		  
		  if (whichmissile==-1||whichmissile!=lastmissile){
			  whichmissile++;
			  //activate whichmissile
			  int count=0;
			  for (unsigned int i=0;i<GetNumMounts();++i) {
				  if (mounts[i].type->type==weapon_info::PROJECTILE&&mounts[i].status<Mount::DESTROYED) {
					  if (count==whichmissile) {
						  mounts[i].status = Mount::ACTIVE;
						  found=true;
					  }else {
						  mounts[i].status = Mount::INACTIVE;
					  }
					  count++;
				  }
			  }
		  }
		  
		  if (!found||whichmissile==lastmissile||whichmissile==-2) {
			  //activate all
			  SelectAllWeapon(true);
		  }
					   
	  }	  
  }else {
  for (int i=0;i<GetNumMounts();i++) {
	  if ((mounts[i].type->type==weapon_info::PROJECTILE)==Missile&&!Missile&&mounts[i].status<Mount::DESTROYED) {
		  if (mounts[i].type->size!=weapon_info::SPECIAL)			  
			  totalcount++;
      lasttotal=false;
      if (mounts[i].status==Mount::ACTIVE) {
	activecount++;
	lasttotal=true;
	mounts[i].DeActive (Missile);
	if (i==GetNumMounts()-1) {
	  sz=mounts[0].type;
	}else {
	  sz =mounts[i+1].type;
	}
      }
    }
    if ((mounts[i].type->type==weapon_info::PROJECTILE)==Missile&&Missile&&mounts[i].status<Mount::DESTROYED) {
      if (mounts[i].status==Mount::ACTIVE) {
	activecount++;//totalcount=0;
	mounts[i].DeActive (Missile);
	if (lasttotal) {
	  totalcount=(i+1)%GetNumMounts();
	  if (i==GetNumMounts()-1) {
	    sz = mounts[0].type;
	  }else {
	    sz =mounts[i+1].type;
	  }
	}
	lasttotal=false;
      } 
    }
  }
  if (Missile) {
    int i=totalcount;
    for (int j=0;j<2;j++) {
      for (;i<GetNumMounts();i++) {
	if (mounts[i].type==sz) {
	  if ((mounts[i].type->type==weapon_info::PROJECTILE)) {
	    mounts[i].Activate(true);
	    return;
	  }else {
	    sz = mounts[(i+1)%GetNumMounts()].type;
	  }
	}
      }
      i=0;
    }
  }
  if (totalcount==activecount) {
	  ActivateGuns (mounts[0].type,Missile);	
  } else {
    if (lasttotal) {
      SelectAllWeapon(Missile);
    }else {
      ActivateGuns (sz,Missile);
    }
  }
  }
}
*/

void Unit::SetRecursiveOwner(Unit *target) {
  owner=target;
  if (!SubUnits.empty()) {
    UnitCollection::UnitIterator iter = getSubUnits();
    Unit * su;
    while ((su=iter.current())) {
      su->SetRecursiveOwner (target);
      iter.advance();
    }
  }
}

int Unit::LockMissile() const{
  bool missilelock=false;
  bool dumblock=false;
  for (int i=0;i<GetNumMounts();i++) {
    if (mounts[i].status==Mount::ACTIVE&&mounts[i].type->LockTime>0&&mounts[i].time_to_lock<=0&&mounts[i].type->type==weapon_info::PROJECTILE) {
      missilelock=true;
    }else {
      if (mounts[i].status==Mount::ACTIVE&&mounts[i].type->LockTime==0&&mounts[i].type->type==weapon_info::PROJECTILE&&mounts[i].time_to_lock<=0) {
	dumblock=true;
      }
    }    
  }
  return (missilelock?1:(dumblock?-1:0));
}


/***********************************************************************************/
/**** UNIT_COLLIDE STUFF                                                            */
/***********************************************************************************/

void Unit::Destroy() {
  if (!killed)
    if (!Explode(false,SIMULATION_ATOM))
	{
  		// The server send a kill notification to all concerned clients but not if it is an upgrade
  		if( SERVER)
  			VSServer->sendKill( this->serial, this->activeStarSystem->GetZone());
  
        Kill();
 	}
}

void Unit::SetCollisionParent (Unit * name) {
  assert (0); //deprecated... many less collisions with subunits out of the table
#if 0
    for (int i=0;i<numsubunit;i++) {
      subunits[i]->CollideInfo.object.u = name;
      subunits[i]->SetCollisionParent (name);
    }
#endif
}

double Unit::getMinDis (const QVector &pnt) {
  float minsofar=1e+10;
  float tmpvar;
  int i;
  Vector TargetPoint (cumulative_transformation_matrix.getP());

#ifdef VARIABLE_LENGTH_PQR
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint)); //the scale factor of the current UNIT
#endif
  for (i=0;i<nummesh();i++) {

    TargetPoint = (Transform(cumulative_transformation_matrix,meshdata[i]->Position()).Cast()-pnt).Cast();
    tmpvar = sqrtf (TargetPoint.Dot (TargetPoint))-meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
	*SizeScaleFactor
#endif
      ;
    if (tmpvar<minsofar) {
      minsofar = tmpvar;
    }
  }
  un_fiter ui = SubUnits.fastIterator();
  Unit * su;
  while ((su=ui.current())) {
    tmpvar = su->getMinDis (pnt);
    if (tmpvar<minsofar) {
      minsofar=tmpvar;
    }			
    ui.advance();
  }
  return minsofar;
}

// This function should not be used on server side
extern vector<Vector> perplines;
float Unit::querySphereClickList (const QVector &st, const QVector &dir, float err) const{
  int i;
  float retval=0;
  float adjretval=0;
  const Matrix * tmpo = &cumulative_transformation_matrix;

  Vector TargetPoint (tmpo->getP());
  for (i=0;i<nummesh();i++) {
    TargetPoint = Transform (*tmpo,meshdata[i]->Position());
    Vector origPoint = TargetPoint;

    perplines.push_back(TargetPoint);
    //find distance away from the line now :-)
    //find scale factor of end on start to get line.
    QVector tst = TargetPoint.Cast()-st;
    //Vector tst = TargetPoint;
    float k = tst.Dot (dir);
    TargetPoint = (tst - k*(dir)).Cast();
    /*
    cerr << origPoint << "-" << st << " = " << tst << " projected length " << k << " along direction " << dir << endl;
    cerr << "projected line " << st << " - " << st + k*dir << endl;
    cerr << "length of orthogonal projection " << TargetPoint.Magnitude() << ", " << "radius " << meshdata[i]->rSize() << endl;
    */
    perplines.push_back(origPoint-TargetPoint);
    
    ///      VSFileSystem::vs_fprintf (stderr, "i%f,j%f,k%f end %f,%f,%f>, k %f distance %f, rSize %f\n", st.i,st.j,st.k,end.i,end.j,end.k,k,TargetPoint.Dot(TargetPoint), meshdata[i]->rSize());    
    
    if (TargetPoint.Dot (TargetPoint)< 
	err*err+
	meshdata[i]->rSize()*meshdata[i]->rSize()+2*err*meshdata[i]->rSize()
	)
      {
	if (retval==0) {
	  retval = k;
	  adjretval=k;
	  if (adjretval<0) {
		adjretval+=meshdata[i]->rSize();
		if (adjretval>0)
				adjretval=.001;
		}
	}else {
		if (retval>0&&k<retval&&k>-meshdata[i]->rSize()){
			retval = k;
			adjretval=k;
			if (adjretval<0) {
				adjretval+=meshdata[i]->rSize();
				if (adjretval>0)
					adjretval=.001;
			}
		}
		if (retval<0&&k+meshdata[i]->rSize()>retval) {
			retval = k;
			adjretval=k+meshdata[i]->rSize();
			if (adjretval>0)
				adjretval=.001;//THRESHOLD;
		}
	}
    }
  }
  un_kiter ui = viewSubUnits();
  const Unit * su;
  while ((su=ui.current())) {
    float tmp=su->querySphereClickList (st,dir,err);
    if (tmp==0) {
      ui.advance();
      continue;
    }
    if (retval==0) {
      retval = tmp;
    }else{
		if (adjretval>0&&tmp<adjretval) {
			retval = tmp;
			adjretval=tmp;
		}
		if (adjretval<0&&tmp>adjretval) {
		    retval = tmp;
			adjretval=tmp;
		}
    }
    ui.advance();
  }

  return adjretval;
}


bool Unit::queryBoundingBox (const QVector &pnt, float err) {
  int i;
  BoundingBox * bbox=NULL;
  for (i=0;i<nummesh();i++) {
    bbox = meshdata[i]->getBoundingBox();
    bbox->Transform (cumulative_transformation_matrix);
    if (bbox->Within(pnt,err)) {
      delete bbox;
      return true;
    }
    delete bbox;
  }
  Unit * su;
  UnitCollection::UnitIterator ui=getSubUnits();
  while ((su=ui.current())) {
    if ((su)->queryBoundingBox (pnt,err)) {
      return true;
    }
    ui.advance();
  }
  return false;
}

int Unit::queryBoundingBox (const QVector &origin, const Vector &direction, float err) {
  int i;
  int retval=0;
  BoundingBox * bbox=NULL;
  for (i=0;i<nummesh();i++) {
    bbox = meshdata[i]->getBoundingBox();
    bbox->Transform (cumulative_transformation_matrix);
    switch (bbox->Intersect(origin,direction.Cast(),err)) {
    case 1:delete bbox;
      return 1;
    case -1:delete bbox;
      retval =-1;
      break;
    case 0: delete bbox;
      break;
    }
  }
  UnitCollection::UnitIterator ui = getSubUnits();
  Unit  * su;
  while ((su=ui.current())) {
    switch (su->queryBoundingBox (origin,direction,err)) {
    case 1: 
      return 1;
    case -1: 
      retval= -1;
      break;
    case 0: 
      break;
    }
    ui.advance();
  }
  return retval;
}

/***********************************************************************************/
/**** UNIT_DOCK STUFF                                                            */
/***********************************************************************************/

bool Unit::EndRequestClearance(Unit *targ) {
	std::vector <Unit *>::iterator lookcleared;
	if ((lookcleared = std::find (targ->image->clearedunits.begin(),targ->image->clearedunits.end(),this))!=targ->image->clearedunits.end()) {
		int whichdockport;
		targ->image->clearedunits.erase (lookcleared);
		return true;
	} else {
		return false;
	}
}

bool Unit::RequestClearance (Unit * dockingunit) {
    static float clearencetime=(XMLSupport::parse_float (vs_config->getVariable ("general","dockingtime","20")));
    EnqueueAIFirst (new ExecuteFor (new Orders::MatchVelocity (Vector(0,0,0),
							       Vector(0,0,0),
							       true,
							       false,
							       true),clearencetime));
    if (std::find (image->clearedunits.begin(),image->clearedunits.end(),dockingunit)==image->clearedunits.end())
      image->clearedunits.push_back (dockingunit);
    return true;
}

void Unit::FreeDockingPort (unsigned int i) {
      if (image->dockedunits.size()==1) {
	docked&= (~DOCKING_UNITS);
      }
      unsigned int whichdock =image->dockedunits[i]->whichdock;
      image->dockingports[whichdock].used=false;      
      image->dockedunits[i]->uc.SetUnit(NULL);
      delete image->dockedunits[i];
      image->dockedunits.erase (image->dockedunits.begin()+i);

}
static Transformation HoldPositionWithRespectTo (Transformation holder, const Transformation &changeold, const Transformation &changenew) {
  Quaternion bak = holder.orientation;
  holder.position=holder.position-changeold.position;

  Quaternion invandrest =changeold.orientation.Conjugate();
  invandrest*=  changenew.orientation;
  holder.orientation*=invandrest;
  Matrix m;

  invandrest.to_matrix(m);
  holder.position = TransformNormal (m,holder.position);

  holder.position=holder.position+changenew.position;
  static bool changeddockedorient=(XMLSupport::parse_bool (vs_config->getVariable ("physics","change_docking_orientation","false")));
  if (!changeddockedorient) {
    holder.orientation = bak;
  }
  return holder;
}
extern void ExecuteDirector();
std::set <Unit *> arrested_list_do_not_dereference;
void Unit::PerformDockingOperations () {
  for (unsigned int i=0;i<image->dockedunits.size();i++) {
    Unit * un;
    if ((un=image->dockedunits[i]->uc.GetUnit())==NULL) {
      FreeDockingPort (i);
      i--;
      continue;
    }
    //Transformation t = un->prev_physical_state;
    float tmp; //short fix
    tmp=un->maxwarpenergy;
    if (tmp<un->jump.energy)
      tmp=un->jump.energy;
    if (tmp>un->warpenergy){
      un->warpenergy=tmp;
      int cockpit=UnitUtil::isPlayerStarship(un);
      if (cockpit>=0&&cockpit<_Universe->numPlayers()) {
	static float docking_fee = XMLSupport::parse_float (vs_config->getVariable("general","docking_fee","0"));
	_Universe->AccessCockpit(cockpit)->credits-=docking_fee;
      }
    }
	std::set<Unit *>::iterator arrested=arrested_list_do_not_dereference.find(this);
	if (arrested!=arrested_list_do_not_dereference.end()) {
		arrested_list_do_not_dereference.erase (arrested);
		//do this for jail time
		for (unsigned int j=0;j<100000;++j) {
			for (unsigned int i=0;i<active_missions.size();++i) {
				
				ExecuteDirector();
			}
		}
	}
    un->prev_physical_state=un->curr_physical_state;
    un->curr_physical_state =HoldPositionWithRespectTo (un->curr_physical_state,prev_physical_state,curr_physical_state);
    un->NetForce=Vector(0,0,0);
    un->NetLocalForce=Vector(0,0,0);
    un->NetTorque=Vector(0,0,0);
    un->NetLocalTorque=Vector (0,0,0);
    un->AngularVelocity=Vector (0,0,0);
    un->Velocity=Vector (0,0,0);
    if (un==_Universe->AccessCockpit()->GetParent()) {
      ///CHOOSE NEW MISSION
      for (unsigned int i=0;i<image->clearedunits.size();i++) {
	if (image->clearedunits[i]==un) {//this is a hack because we don't have an interface to say "I want to buy a ship"  this does it if you press shift-c in the base
	  image->clearedunits.erase(image->clearedunits.begin()+i);
	  un->UpgradeInterface(this);
	}
      }
    }
    //now we know the unit's still alive... what do we do to him *G*
    ///force him in a box...err where he is
  }
}

int Unit::ForceDock (Unit * utdw, int whichdockport) {
	if (utdw->image->dockingports.size()<=whichdockport)
		return 0;
		
      utdw->image->dockingports[whichdockport].used=true;
	
      utdw->docked|=DOCKING_UNITS;
      utdw->image->dockedunits.push_back (new DockedUnits (this,whichdockport));
      if (utdw->image->dockingports[whichdockport].internal) {
	RemoveFromSystem();	
	SetVisible(false);
	docked|=DOCKED_INSIDE;
      }else {
	docked|= DOCKED;
      }
      image->DockedTo.SetUnit (utdw);
      computer.set_speed=0;
      if (this==_Universe->AccessCockpit()->GetParent()) {
		  this->RestoreGodliness();
	//_Universe->AccessCockpit()->RestoreGodliness();
      }
	  return whichdockport+1;
}
int Unit::Dock (Unit * utdw) {
// Do only if non networking mode or if server (for both Network==NULL)
if( Network==NULL)
{
  if (docked&(DOCKED_INSIDE|DOCKED))
    return 0;
  std::vector <Unit *>::iterator lookcleared;
  if ((lookcleared = std::find (utdw->image->clearedunits.begin(),
				utdw->image->clearedunits.end(),this))!=utdw->image->clearedunits.end()) {
    int whichdockport;
    if ((whichdockport=utdw->CanDockWithMe(this))!=-1) {
	    utdw->image->clearedunits.erase (lookcleared);
		return ForceDock(utdw,whichdockport);      
    }
  }
  return 0;
}
else
{
	// Send a dock request
	int playernum = _Universe->whichPlayerStarship( this);
	if( playernum>=0)
		Network[playernum].dockRequest( utdw->serial);
}
return 0;
}

inline bool insideDock (const DockingPorts &dock, const Vector & pos, float radius) {
  if (dock.used)
    return false;
  double rad=dock.radius+radius;
  return (pos-dock.pos).MagnitudeSquared()<rad*rad;
  if (dock.internal) {
    if ((pos.i+radius<dock.max.i)&&
	(pos.j+radius<dock.max.j)&&
	(pos.k+radius<dock.max.k)&&
	(pos.i-radius>dock.min.i)&&
	(pos.j-radius>dock.min.j)&&
	(pos.k-radius>dock.min.k)) {
      return true;
    }    
  }else {
    if ((pos-dock.pos).Magnitude()<dock.radius+radius&&
	(pos.i-radius<dock.max.i)&&
	(pos.j-radius<dock.max.j)&&
	(pos.k-radius<dock.max.k)&&
	(pos.i+radius>dock.min.i)&&
	(pos.j+radius>dock.min.j)&&
	(pos.k+radius>dock.min.k)) {
      return true;
    }
  }
  return false;
}

int Unit::CanDockWithMe(Unit * un) {
  //  if (_Universe->GetRelation(faction,un->faction)>=0) {//already clearneed
    for (unsigned int i=0;i<image->dockingports.size();i++) {
      if (un->image->dockingports.size()) {
	for (unsigned int j=0;j<un->image->dockingports.size();j++) {
	  if (insideDock (image->dockingports[i],InvTransform (cumulative_transformation_matrix,Transform (un->cumulative_transformation_matrix,un->image->dockingports[j].pos)),un->image->dockingports[j].radius)) {
	    if (((un->docked&(DOCKED_INSIDE|DOCKED))==0)&&(!(docked&DOCKED_INSIDE))) {
	      return i;
	    }
	  }
	}  
      }else {
	if (insideDock (image->dockingports[i],InvTransform (cumulative_transformation_matrix,un->Position().Cast()),un->rSize())) {
	  return i;
	}
      }
    }
    //  }
  return -1;
}

bool Unit::IsCleared (Unit * DockingUnit) {
  return (std::find (image->clearedunits.begin(),image->clearedunits.end(),DockingUnit)!=image->clearedunits.end());
}

bool Unit::isDocked (Unit* d) {
  if (!d)
	  return false;
  if (!(d->docked&(DOCKED_INSIDE|DOCKED))) {
    return false;
  }
  for (unsigned int i=0;i<image->dockedunits.size();i++) {
    Unit * un;
    if ((un=image->dockedunits[i]->uc.GetUnit())!=NULL) {
      if (un==d) {
	return true;
      }
    }
  }
  return false;
}

bool Unit::UnDock (Unit * utdw) {
  unsigned int i=0;

  cerr<<"Asking to undock"<<endl;
  if( Network!=NULL && !SERVER)
  {
	cerr<<"Sending an undock notification"<<endl;
	int playernum = _Universe->whichPlayerStarship( this);
	if( playernum>=0)
		Network[playernum].undockRequest( utdw->serial);
  }
  for (i=0;i<utdw->image->dockedunits.size();i++) {
    if (utdw->image->dockedunits[i]->uc.GetUnit()==this) {
      utdw->FreeDockingPort (i);
      i--;
      SetVisible(true);;
      docked&=(~(DOCKED_INSIDE|DOCKED));
      image->DockedTo.SetUnit (NULL);
      Velocity=utdw->Velocity;
      return true;
    }
  }
  return false;
}


/***********************************************************************************/
/**** UNIT_CUSTOMIZE STUFF                                                            */
/***********************************************************************************/
#define UPGRADEOK 1
#define NOTTHERE 0
#define CAUSESDOWNGRADE -1
#define LIMITEDBYTEMPLATE -2

typedef double (*adder) (double a, double b);
typedef double (*percenter) (double a, double b, double c);
typedef bool (*comparer) (double a, double b);

bool GreaterZero (double a, double b) {
  return a>=0;
}
double AddUp (double a, double b) {
  return a+b;
}
double MultUp (double a, double b) {
  return a*b;
}
double GetsB (double a, double b) {
  return b;
}
bool AGreaterB (double a, double b) {
  return a>b;
}
double SubtractUp(double a, double b) {
  return a-b;
}
double SubtractClamp (double a, double b) {
  return (a-b<0)?0:a-b;
}
bool ALessB (double a, double b) {
  return a<b;
}
double computePercent (double old, double upgrade, double newb) {
  if (newb)
    return old/newb;
  else
    return 0;
}
double computeWorsePercent (double old,double upgrade, double isnew) {
  if (old)
    return isnew/old;
  else
    return 1;
}
double computeAdderPercent (double a,double b, double c) {return 0;}
double computeMultPercent (double a,double b, double c) {return 0;}
double computeDowngradePercent (double old, double upgrade, double isnew) {
  if (upgrade) {
    return (old-isnew)/upgrade;
  }else {
    return 0;
  }
}

static int UpgradeFloat (double &result,double tobeupgraded, double upgrador, double templatelimit, double (*myadd) (double,double), bool (*betterthan) (double a, double b), double nothing,  double completeminimum, double (*computepercentage) (double oldvar, double upgrador, double newvar), double & percentage, bool forcedowngrade, bool usetemplate, double at_least_this,bool (*atLeastthiscompare)( double a, double b)=AGreaterB, bool clamp=false, bool force_nothing=false) {
  if (upgrador!=nothing||force_nothing) {//if upgrador is better than nothing
    if (clamp) {
      if (tobeupgraded>upgrador)
	upgrador=tobeupgraded;
    }
    float newsum = (*myadd)(tobeupgraded,upgrador);
    if (!force_nothing&&newsum < tobeupgraded&&at_least_this>=upgrador&&at_least_this>newsum&&at_least_this>=tobeupgraded){//if we're downgrading
        return newsum==upgrador?CAUSESDOWNGRADE:NOTTHERE;
    }
    if (newsum!=tobeupgraded&&(((*betterthan)(newsum, tobeupgraded)||forcedowngrade))) {
      if (((*betterthan)(newsum,templatelimit)&&usetemplate)||newsum<completeminimum) {
	if (!forcedowngrade)
	  return LIMITEDBYTEMPLATE;
	if (newsum<completeminimum)
	  newsum=completeminimum;
	else
	  newsum = templatelimit;
      }
      ///we know we can replace result with newsum
      percentage = (*computepercentage)(tobeupgraded,upgrador,newsum);
      if ((*atLeastthiscompare)(at_least_this,newsum)&&(!force_nothing)) {
            if ((*atLeastthiscompare)(at_least_this,tobeupgraded)) {
                newsum = tobeupgraded;//no shift
            }else {
                newsum = at_least_this;//set it to its min
            }
      }        
      result=newsum;
      return UPGRADEOK;
    }else {
      return CAUSESDOWNGRADE;
    }
  } else {
    return NOTTHERE;
  }
}


int UpgradeBoolval (int a, int upga, bool touchme, bool downgrade, int &numave,double &percentage, bool force_nothing) {
  if (downgrade) {
    if (a&&upga) {
      if (touchme) (a=false);
      numave++;
      percentage++;
    }
  }else {
    if (!a&&upga) {
      if (touchme) a=true;
      numave++;
      percentage++;
    }else if (force_nothing && a  && !upga) {
      if (touchme) a=false;
      numave++;
      percentage++;
	}
  }
  return a;
}

void YoinkNewlines (char * input_buffer) {
    for (int i=0;input_buffer[i]!='\0';i++) {
      if (input_buffer[i]=='\n'||input_buffer[i]=='\r') {
	input_buffer[i]='\0';
      }
    }
}
bool Quit (const char *input_buffer) {
	if (strcasecmp (input_buffer,"exit")==0||
	    strcasecmp (input_buffer,"quit")==0) {
	  return true;
	}
	return false;
}

using std::string;
void Tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos) {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
std::string CheckBasicSizes (const std::string tokens) {
  if (tokens.find ("small")!=string::npos) {
    return "small";
  }
  if (tokens.find ("medium")!=string::npos) {
    return "medium";
  }
  if (tokens.find ("large")!=string::npos) {
    return "large";
  }
  if (tokens.find ("cargo")!=string::npos) {
    return "cargo";
  }
  if (tokens.find ("LR")!=string::npos||tokens.find ("massive")!=string::npos) {
    return "massive";
  }
  return "";
}

class VCString : public std::string {
public:
	VCString(){}
	VCString(const string & s): string(s){}
};
std::map<VCString,VCString> parseTurretSizes () {
	using namespace VSFileSystem;
	std::map<VCString,VCString> t;
	VSFile f;
	VSError err = f.OpenReadOnly( "units/subunits/size.txt", UnknownFile);
	if (err<=Ok) {
		int siz = f.Size();
		char * filedata= (char *)malloc (siz+1);
		filedata[siz]=0;
		while (f.ReadLine(filedata,siz)==Ok) {

			std::string x(filedata);
			string::size_type len= x.find (",");
			if (len!=std::string::npos) {
				std::string y = x.substr (len+1);
				x = x.substr(0,len);				
				len = y.find(",");
				y = y.substr(0,len);
				sscanf (y.c_str(),"%s",filedata);
				y = filedata;
				VCString key (x);
				VCString value (y);
				t[key]=value;
			}
		}
		free(filedata);
		f.Close();
	}
	return t;
}

std::string getTurretSize (const std::string &size) {
  static std::map <VCString,VCString> turretmap = parseTurretSizes();
  std::map<VCString,VCString>::iterator h= turretmap.find(size);
  if (h!=turretmap.end()) {
	  return (*h).second;
  }
  vector <string> tokens;
  Tokenize (size,tokens,"_");
  for (unsigned int i=0;i<tokens.size();i++) {
    if (tokens[i].find ("turret")!=string::npos) {
      string temp = CheckBasicSizes (tokens[i]);
      if (!temp.empty()) {
	return temp;
      }
    } else {
      return tokens[i];
    }
  }
  return "capitol";
}

bool Unit::UpgradeMounts (const Unit *up, int mountoffset, bool touchme, bool downgrade, int &numave, const Unit * templ, double &percentage) {
  int j;
  int i;
  bool cancompletefully=true;
  for (i=0,j=mountoffset;i<up->GetNumMounts()&&i<GetNumMounts()/*i should be GetNumMounts(), s'ok*/;i++,j++) {
    if (up->mounts[i].status==Mount::ACTIVE||up->mounts[i].status==Mount::INACTIVE) {//only mess with this if the upgrador has active mounts
      int jmod=j%GetNumMounts();//make sure since we're offsetting the starting we don't overrun the mounts
      if (!downgrade) {//if we wish to add guns instead of remove
	if (up->mounts[i].type->weapon_name!="MOUNT_UPGRADE") {


	  if (up->mounts[i].type->size==(up->mounts[i].type->size&mounts[jmod].size)) {//only look at this mount if it can fit in the rack
	    if (up->mounts[i].type->weapon_name!=mounts[jmod].type->weapon_name || mounts[jmod].status==Mount::DESTROYED || mounts[jmod].status==Mount::UNCHOSEN) {
	      numave++;//ok now we can compute percentage of used parts
		  Mount upmount(up->mounts[i]);
		  
	      if (templ) {
		if (templ->GetNumMounts()>jmod) {

		/* Volume controls maxammo, not template ammo
		 
		   int maxammo = templ->mounts[jmod].ammo;

		  if ((upmount.ammo>maxammo||upmount.ammo==-1)&&maxammo!=-1) {
		    upmount.ammo = maxammo;
		  }

		*/
		  if (templ->mounts[jmod].volume!=-1) {
		    if (upmount.ammo*upmount.type->volume>templ->mounts[jmod].volume) {
		      upmount.ammo = (int)((templ->mounts[jmod].volume+1)/upmount.type->volume);
		    }
		  }
		}
	      }
	      percentage+=mounts[jmod].Percentage(&upmount);//compute here
	      if (touchme) {//if we wish to modify the mounts
		mounts[jmod].ReplaceMounts (&upmount);//switch this mount with the upgrador mount
	      }
	    }else {
	      int tmpammo = mounts[jmod].ammo;
	      if (mounts[jmod].ammo!=-1&&up->mounts[i].ammo!=-1) {
			tmpammo+=up->mounts[i].ammo;
			if (templ) {
			  if (templ->GetNumMounts()>jmod) {
				/* this shouldn't be here... we don't know why it's here, but we're leaving it commented out
				if (templ->mounts[jmod].ammo!=-1) {
				  if (templ->mounts[jmod].ammo>tmpammo) {
					tmpammo=templ->mounts[jmod].ammo;
				  }
				}
				*/	
				if (templ->mounts[jmod].volume!=-1) {
				  if (templ->mounts[jmod].volume<mounts[jmod].type->volume*tmpammo) {
					tmpammo=(int)((templ->mounts[jmod].volume+1)/mounts[jmod].type->volume);
				  }
				}
		    
			  }
			} 
			if (tmpammo*mounts[jmod].type->volume>mounts[jmod].volume) {
			  tmpammo = (int)((1+mounts[jmod].volume)/mounts[jmod].type->volume);
			}
			if (tmpammo>mounts[jmod].ammo) {
			  cancompletefully=true;
			  if (touchme)
				mounts[jmod].ammo = tmpammo;
			}else {
			  cancompletefully=false;
			}
	      }
	      
	    }
	  } else {
	    cancompletefully=false;//since we cannot fit the mount in the slot we cannot complete fully
	  }
	}else {
	  unsigned int siz=0;
	  siz = ~siz;
	  if (templ) {
	    if (templ->GetNumMounts()>jmod) {
	      siz = templ->mounts[jmod].size;
	    }
	  }
	  if (((siz&up->mounts[i].size)|mounts[jmod].size)!=mounts[jmod].size) {
	    if (touchme) {
	      mounts[jmod].size|=up->mounts[i].size;
	    }
	    numave++;
	    percentage++;

	  }else {
	    cancompletefully=false;
	  }
	  //we need to |= the mount type
	}
      } else {
	if (up->mounts[i].type->weapon_name!="MOUNT_UPGRADE") {
	  bool found=false;//we haven't found a matching gun to remove

		for (unsigned int k=0;k<(unsigned int)GetNumMounts();k++) {///go through all guns
	      int jkmod = (jmod+k)%GetNumMounts();//we want to start with bias
	      if (strcasecmp(mounts[jkmod].type->weapon_name.c_str(),up->mounts[i].type->weapon_name.c_str())==0) {///search for right mount to remove starting from j. this is the right name
		found=true;//we got one
		percentage+=mounts[jkmod].Percentage(&up->mounts[i]);///calculate scrap value (if damaged)
		if (touchme){ //if we modify
			if (up->mounts[i].ammo&&up->mounts[i].ammo!=-1&&mounts[jkmod].ammo!=-1){ //if downgrading ammo based upgrade, checks for infinite ammo
				mounts[jkmod].ammo-=(mounts[jkmod].ammo>=up->mounts[i].ammo)?up->mounts[i].ammo:mounts[jkmod].ammo; //remove upgrade-worth, else remove remaining
				if(!mounts[jkmod].ammo){ //if none left
					mounts[jkmod].status=Mount::UNCHOSEN;///deactivate weapon
				}
			}else{
				mounts[jkmod].status=Mount::UNCHOSEN;///deactivate weapon
			}
		}
		break; 
	      }
	    }
	  
	  if (!found)
	    cancompletefully=false;//we did not find a matching weapon to remove
	}else {
	  bool found=false;
	  static   bool downmount =XMLSupport::parse_bool (vs_config->getVariable ("physics","can_downgrade_mount_upgrades","false"));
	  if (downmount ) {

	  for (unsigned int k=0;k<(unsigned int)GetNumMounts();k++) {///go through all guns
	    int jkmod = (jmod+k)%GetNumMounts();//we want to start with bias
	    if ((up->mounts[i].size&mounts[jkmod].size)==(up->mounts[i].size)) {
	      if (touchme) {
		mounts[jkmod].size&=(~up->mounts[i].size);
	      }
	      percentage++;
	      numave++;
	      found=true;
	    }
	  }
	  }
	  if (!found)
	    cancompletefully=false;
	}
      }
    }
  }
  if (i<up->GetNumMounts()) {
    cancompletefully=false;//if we didn't reach the last mount that we wished to upgrade, we did not fully complete
  }
  return cancompletefully;
}
Unit * CreateGenericTurret (std::string tur,int faction) {
  return new Unit (tur.c_str(),true,faction,"",0,0);
}

bool Unit::UpgradeSubUnits (const Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage)  {
  return UpgradeSubUnitsWithFactory( up, subunitoffset, touchme, downgrade, numave, percentage,&CreateGenericTurret);
}
bool Unit::UpgradeSubUnitsWithFactory (const Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage, Unit * (*createupgradesubunit) (std::string s, int faction))  {
  bool cancompletefully=true;
  int j;
  std::string turSize;
  un_iter ui;
  bool found=false;
  for (j=0,ui=getSubUnits();(*ui)!=NULL&&j<subunitoffset;++ui,j++) {
  }///set the turrets to the offset
  un_kiter upturrets;
  Unit * giveAway;

  giveAway=*ui;
  if (giveAway==NULL) {
    return true;
  }
  bool hasAnyTurrets=false;
    turSize = getTurretSize (giveAway->name);
  for (upturrets=up->viewSubUnits();((*upturrets)!=NULL)&&((*ui)!=NULL); ++ui,++upturrets) {//begin goign through other unit's turrets
    hasAnyTurrets = true;
    const Unit *addtome;

    addtome=*upturrets;//set pointers

    

    bool foundthis=false;
    if (turSize == getTurretSize (addtome->name)&&addtome->rSize()&&(turSize+"_blank"!=addtome->name)) {//if the new turret has any size at all
      if (!downgrade||addtome->name==giveAway->name) {
	found=true;
	foundthis=true;
	numave++;//add it
	percentage+=(giveAway->rSize()/addtome->rSize());//add up percentage equal to ratio of sizes
      }
    }
    if (foundthis) {
      if (touchme) {//if we wish to modify,
	Transformation addToMeCur = giveAway->curr_physical_state;
	Transformation addToMePrev = giveAway->prev_physical_state;
       	//	upturrets.postinsert (giveAway);//add it to the second unit
	giveAway->Kill();//risky??
	ui.remove();//remove the turret from the first unit
	
	if (!downgrade) {//if we are upgrading swap them
	  Unit * addToMeNew = (*createupgradesubunit)(addtome->name,addtome->faction);
	  addToMeNew->curr_physical_state = addToMeCur;
	  addToMeNew->SetFaction(faction);
	  addToMeNew->prev_physical_state = addToMePrev;
	  ui.preinsert(addToMeNew);//add unit to your ship
	  //	  upturrets.remove();//remove unit from being a turret on other ship
	  addToMeNew->SetRecursiveOwner(this);//set recursive owner
	} else {
	  Unit * un;//make garbage unit
	  // NOT 100% SURE A GENERIC UNIT CAN FIT (WAS GAME UNIT CREATION)
	  ui.preinsert (un=UnitFactory::createUnit("blank",true,faction));//give a default do-nothing unit
	  //WHAT?!?!?!?! 102302	  ui.preinsert (un=new Unit(0));//give a default do-nothing unit
	  un->SetFaction(faction);
	  un->curr_physical_state = addToMeCur;
	  un->prev_physical_state = addToMePrev;
	  un->limits.yaw=0;
	  un->limits.pitch=0;
	  un->limits.roll=0;
	  un->limits.lateral = un->limits.retro = un->limits.forward = un->limits.afterburn=0.0;

	  un->name = turSize+"_blank";
	  if (un->image->unitwriter!=NULL) {
	    un->image->unitwriter->setName (un->name);
	  }
	  un->SetRecursiveOwner(this);
	}
      }
    }
  }
  
  if (!found) {
    return !hasAnyTurrets;
  }
  if ((*upturrets)!=NULL) 
    return false;
  return cancompletefully;
}

bool Unit::canUpgrade (const Unit * upgrador, int mountoffset,  int subunitoffset, int additive, bool force,  double & percentage, const Unit * templ, bool force_change_on_nothing){
  return UpAndDownGrade(upgrador,templ,mountoffset,subunitoffset,false,false,additive,force,percentage,this, force_change_on_nothing);
}
bool Unit::Upgrade (const Unit * upgrador, int mountoffset,  int subunitoffset, int additive, bool force,  double & percentage, const Unit * templ, bool force_change_on_nothing) {
  return UpAndDownGrade(upgrador,templ,mountoffset,subunitoffset,true,false,additive,force,percentage,this, force_change_on_nothing);
}
bool Unit::canDowngrade (const Unit *downgradeor, int mountoffset, int subunitoffset, double & percentage, const Unit * downgradelimit){
  return UpAndDownGrade(downgradeor,NULL,mountoffset,subunitoffset,false,true,false,true,percentage,downgradelimit,false);
}
bool Unit::Downgrade (const Unit * downgradeor, int mountoffset, int subunitoffset,  double & percentage,const Unit * downgradelimit){
  return UpAndDownGrade(downgradeor,NULL,mountoffset,subunitoffset,true,true,false,true,percentage,downgradelimit,false);
}
double ComputeMinDowngradePercent() {
    static float MyPercentMin = XMLSupport::parse_float (vs_config->getVariable("general","remove_downgrades_less_than_percent",".9"));
    return MyPercentMin;
}
class DoubleName {
public:
  string s;
  double d;
  DoubleName (string ss,double dd) {
    d =dd;s=ss;
  }
  DoubleName () {
    d = -FLT_MAX;
  }
};
std::map <int, DoubleName> downgrademap;
int curdowngrademapoffset = 5*sizeof (Unit);
bool AddToDowngradeMap (std::string name,double value, int unitoffset,std::map <int,DoubleName> &tempdowngrademap) {
  using std::map;
  map<int,DoubleName>::iterator i =downgrademap.find (unitoffset);
  if (i!=downgrademap.end()) {
    if ((*i).second.d<=value) {
        tempdowngrademap[unitoffset] = DoubleName (name,value);
        return true;
    }
  }else {
      tempdowngrademap[unitoffset] = DoubleName (name,value);      
      return true;
  }
  return false;
}
void ClearDowngradeMap () {
  downgrademap.clear();
}
std::set<std::string> GetListOfDowngrades () {
  using std::map;
  map<int,DoubleName>::iterator i =downgrademap.begin();
  std::set<std::string> retval;
  for (;i!=downgrademap.end();++i) {
    retval.insert ((*i).second.s);
  }
  
//  return std::vector<std::string> (retval.begin(),retval.end());
  return retval;
}
bool Unit::UpAndDownGrade (const Unit * up, const Unit * templ, int mountoffset, int subunitoffset, bool touchme, bool downgrade, int additive, bool forcetransaction, double &percentage, const Unit * downgradelimit,bool force_change_on_nothing) {
  percentage=0;
  if (up->name=="disruptor") {
    printf ("dis");
  }
  if (up->name=="franklinpd") {
    printf ("frank");
  }
  int numave=0;
  bool cancompletefully=true;
  if (mountoffset>=0)
	  cancompletefully=UpgradeMounts(up,mountoffset,touchme,downgrade,numave,templ,percentage);
  bool cancompletefully1=true;
  if (subunitoffset>=0)
	  cancompletefully1=UpgradeSubUnits(up,subunitoffset,touchme,downgrade,numave,percentage);
  cancompletefully=cancompletefully&&cancompletefully1;
  adder Adder;
  comparer Comparer;
  percenter Percenter;
  std::map <int, DoubleName> tempdownmap;
  if (cancompletefully&&cancompletefully1&&downgrade) {
	  if (percentage>0)
		  AddToDowngradeMap (up->name,1,curdowngrademapoffset++,tempdownmap);
  }
  
  float tmax_speed = up->computer.max_combat_speed;
  float tmax_ab_speed = up->computer.max_combat_ab_speed;
  float tmax_yaw = up->computer.max_yaw;
  float tmax_pitch = up->computer.max_pitch;
  float tmax_roll = up->computer.max_roll;
  float tlimits_yaw=up->limits.yaw;
  float tlimits_roll=up->limits.roll;
  float tlimits_pitch=up->limits.pitch;
  float tlimits_lateral = up->limits.lateral;
  float tlimits_vertical = up->limits.vertical;
  float tlimits_forward = up->limits.forward;
  float tlimits_retro = up->limits.retro;
  float tlimits_afterburn = up->limits.afterburn;
  if (downgrade) {
    Adder=&SubtractUp;
    Percenter=&computeDowngradePercent;
    Comparer = &GreaterZero;
  } else{
    if (additive==1) {
      Adder=&AddUp;
      Percenter=&computeAdderPercent;
    }else if (additive==2) {
      Adder=&MultUp;
      Percenter=&computeMultPercent;
      tmax_speed = XMLSupport::parse_float (speedStarHandler (XMLType (&tmax_speed),NULL));
      tmax_ab_speed = XMLSupport::parse_float (speedStarHandler (XMLType (&tmax_ab_speed),NULL));
      tmax_yaw = XMLSupport::parse_float (angleStarHandler (XMLType (&tmax_yaw),NULL));
      tmax_pitch = XMLSupport::parse_float (angleStarHandler (XMLType (&tmax_pitch),NULL));
      tmax_roll = XMLSupport::parse_float (angleStarHandler (XMLType (&tmax_roll),NULL));

      tlimits_yaw = XMLSupport::parse_float (angleStarHandler (XMLType (&tlimits_yaw),NULL));
      tlimits_pitch = XMLSupport::parse_float (angleStarHandler (XMLType (&tlimits_pitch),NULL));
      tlimits_roll = XMLSupport::parse_float (angleStarHandler (XMLType (&tlimits_roll),NULL));

      tlimits_forward = XMLSupport::parse_float (accelStarHandler (XMLType (&tlimits_forward),NULL));
      tlimits_retro = XMLSupport::parse_float (accelStarHandler (XMLType (&tlimits_retro),NULL));
      tlimits_lateral = XMLSupport::parse_float (accelStarHandler (XMLType (&tlimits_lateral),NULL));
      tlimits_vertical = XMLSupport::parse_float (accelStarHandler (XMLType (&tlimits_vertical),NULL));
      tlimits_afterburn = XMLSupport::parse_float (accelStarHandler (XMLType (&tlimits_afterburn),NULL));
      
    }else {
      Adder=&GetsB;
      Percenter=&computePercent;
    }
    Comparer=AGreaterB;
  }
  double resultdoub;
  int retval;
  double temppercent;
  static Unit * blankship = UnitFactory::createServerSideUnit ("blank",true,FactionUtil::GetFaction("upgrades"));
#define STDUPGRADE_SPECIFY_DEFAULTS(my,oth,temp,noth,dgradelimer,dgradelimerdefault,clamp,value_to_lookat) retval=(UpgradeFloat(resultdoub,my,oth,(templ!=NULL)?temp:0,Adder,Comparer,noth,noth,Percenter, temppercent,forcetransaction,templ!=NULL,(downgradelimit!=NULL)?dgradelimer:dgradelimerdefault,AGreaterB,clamp,force_change_on_nothing)); if (retval==UPGRADEOK) {if (touchme){my=resultdoub;} percentage+=temppercent; numave++;AddToDowngradeMap (up->name,oth,((char *)&value_to_lookat)-(char *)this,tempdownmap);} else {if (retval!=NOTTHERE) cancompletefully=false;}

  
#define STDUPGRADE(my,oth,temp,noth) STDUPGRADE_SPECIFY_DEFAULTS (my,oth,temp,noth,downgradelimit->my,blankship->my,false,this->my)

#define STDUPGRADECLAMP(my,oth,temp,noth) STDUPGRADE_SPECIFY_DEFAULTS (my,oth,temp,noth,downgradelimit->my,blankship->my,!force_change_on_nothing,this->my)

  STDUPGRADE(armor.frontrighttop,up->armor.frontrighttop,templ->armor.frontrighttop,0);
  STDUPGRADE(armor.backrighttop,up->armor.backrighttop,templ->armor.backrighttop,0);
  STDUPGRADE(armor.frontlefttop,up->armor.frontlefttop,templ->armor.frontlefttop,0);
  STDUPGRADE(armor.backlefttop,up->armor.backlefttop,templ->armor.backlefttop,0);
  STDUPGRADE(armor.frontrightbottom,up->armor.frontrightbottom,templ->armor.frontrightbottom,0);
  STDUPGRADE(armor.backrightbottom,up->armor.backrightbottom,templ->armor.backrightbottom,0);
  STDUPGRADE(armor.frontleftbottom,up->armor.frontleftbottom,templ->armor.frontleftbottom,0);
  STDUPGRADE(armor.backleftbottom,up->armor.backleftbottom,templ->armor.backleftbottom,0);

  STDUPGRADE(shield.recharge,up->shield.recharge,templ->shield.recharge,0);
  STDUPGRADE(hull,up->hull,templ->hull,0);
  if (maxhull<hull) {
    if (hull!=0) 
      maxhull=hull;
  }
  STDUPGRADE(recharge,up->recharge,templ->recharge,0);
  STDUPGRADE(image->repair_droid,up->image->repair_droid,templ->image->repair_droid,0);
  static bool unittable=XMLSupport::parse_bool(vs_config->getVariable("physics","UnitTable","false"));
  STDUPGRADE(image->cargo_volume,up->image->cargo_volume,templ->image->cargo_volume,0);
  STDUPGRADE(image->equipment_volume,up->image->equipment_volume,templ->image->equipment_volume,0);
  image->ecm = abs(image->ecm);
  STDUPGRADE(image->ecm,abs(up->image->ecm),abs(templ->image->ecm),0);
  STDUPGRADE(maxenergy,up->maxenergy,templ->maxenergy,0);
  STDUPGRADE(maxwarpenergy,up->maxwarpenergy,templ->maxwarpenergy,0);
  STDUPGRADE(jump.warpDriveRating,up->jump.warpDriveRating,templ->jump.warpDriveRating,0);
  STDUPGRADE(limits.yaw,tlimits_yaw,templ->limits.yaw,0);
  STDUPGRADE(limits.pitch,tlimits_pitch,templ->limits.pitch,0);
  STDUPGRADE(limits.roll,tlimits_roll,templ->limits.roll,0);
  STDUPGRADE(limits.lateral,tlimits_lateral,templ->limits.lateral,0);
  STDUPGRADE(limits.vertical,tlimits_vertical,templ->limits.vertical,0);
  STDUPGRADE(limits.forward,tlimits_forward,templ->limits.forward,0);
  STDUPGRADE(limits.retro,tlimits_retro,templ->limits.retro,0);
  STDUPGRADE(limits.afterburn,tlimits_afterburn,templ->limits.afterburn,0);
  static bool use_template_maxrange= XMLSupport::parse_bool (vs_config->getVariable("physics","use_upgrade_template_maxrange","true"));
  
  STDUPGRADECLAMP(computer.radar.maxrange,up->computer.radar.maxrange,use_template_maxrange?templ->computer.radar.maxrange:FLT_MAX,0);
  STDUPGRADE(computer.max_combat_speed,tmax_speed,templ->computer.max_combat_speed,0);
  STDUPGRADE(computer.max_combat_ab_speed,tmax_ab_speed,templ->computer.max_combat_ab_speed,0);
  STDUPGRADE(computer.max_yaw,tmax_yaw,templ->computer.max_yaw,0);
  STDUPGRADE(computer.max_pitch,tmax_pitch,templ->computer.max_pitch,0);
  STDUPGRADE(computer.max_roll,tmax_roll,templ->computer.max_roll,0);
  STDUPGRADE(fuel,up->fuel,templ->fuel,0);

  static bool UpgradeCockpitDamage = XMLSupport::parse_bool (vs_config->getVariable("physics","upgrade_cockpit_damage","false"));
  if (UpgradeCockpitDamage) {
    STDUPGRADE(image->fireControlFunctionality,up->image->fireControlFunctionality,templ->image->fireControlFunctionality,(unittable?0:1));
    STDUPGRADE(image->fireControlFunctionalityMax,up->image->fireControlFunctionalityMax,templ->image->fireControlFunctionalityMax,(unittable?0:1));
    STDUPGRADE(image->SPECDriveFunctionality,up->image->SPECDriveFunctionality,templ->image->SPECDriveFunctionality,(unittable?0:1));
    STDUPGRADE(image->SPECDriveFunctionalityMax,up->image->SPECDriveFunctionalityMax,templ->image->SPECDriveFunctionalityMax,(unittable?0:1));
    STDUPGRADE(image->CommFunctionality,up->image->CommFunctionality,templ->image->CommFunctionality,(unittable?0:1));
    STDUPGRADE(image->CommFunctionalityMax,up->image->CommFunctionalityMax,templ->image->CommFunctionalityMax,(unittable?0:1));
    STDUPGRADE(image->LifeSupportFunctionality,up->image->LifeSupportFunctionality,templ->image->LifeSupportFunctionality,(unittable?0:1));
    STDUPGRADE(image->LifeSupportFunctionalityMax,up->image->LifeSupportFunctionalityMax,templ->image->LifeSupportFunctionalityMax,(unittable?0:1));
    
    for (unsigned int upgr=0;upgr<(UnitImages::NUMGAUGES+1+MAXVDUS)*2;upgr++) {
      STDUPGRADE(image->cockpit_damage[upgr],up->image->cockpit_damage[upgr],templ->image->cockpit_damage[upgr],(unittable?0:1));
      if (image->cockpit_damage[upgr]>1) {
        image->cockpit_damage[upgr]=1;//keep it real
      }
    }
  }
  if (shield.number==up->shield.number) {
    switch (shield.number) {
    case 2:
      STDUPGRADE(shield.shield2fb.frontmax,up->shield.shield2fb.frontmax,templ->shield.shield2fb.frontmax,0);
      STDUPGRADE(shield.shield2fb.backmax,up->shield.shield2fb.backmax,templ->shield.shield2fb.backmax,0);
      break;
    case 4:
      STDUPGRADE(shield.shield4fbrl.frontmax,up->shield.shield4fbrl.frontmax,templ->shield.shield4fbrl.frontmax,0);
      STDUPGRADE(shield.shield4fbrl.backmax,up->shield.shield4fbrl.backmax,templ->shield.shield4fbrl.backmax,0);
      STDUPGRADE(shield.shield4fbrl.leftmax,up->shield.shield4fbrl.leftmax,templ->shield.shield4fbrl.leftmax,0);
      STDUPGRADE(shield.shield4fbrl.rightmax,up->shield.shield4fbrl.rightmax,templ->shield.shield4fbrl.rightmax,0);
      break;
    case 8:
      STDUPGRADE(shield.shield8.frontrighttopmax,up->shield.shield8.frontrighttopmax,templ->shield.shield8.frontrighttopmax,0);
      STDUPGRADE(shield.shield8.backrighttopmax,up->shield.shield8.backrighttopmax,templ->shield.shield8.backrighttopmax,0);
      STDUPGRADE(shield.shield8.frontlefttopmax,up->shield.shield8.frontlefttopmax,templ->shield.shield8.frontlefttopmax,0);
      STDUPGRADE(shield.shield8.backlefttopmax,up->shield.shield8.backlefttopmax,templ->shield.shield8.backlefttopmax,0);
	  STDUPGRADE(shield.shield8.frontrightbottommax,up->shield.shield8.frontrightbottommax,templ->shield.shield8.frontrightbottommax,0);
      STDUPGRADE(shield.shield8.backrightbottommax,up->shield.shield8.backrightbottommax,templ->shield.shield8.backrightbottommax,0);
      STDUPGRADE(shield.shield8.frontleftbottommax,up->shield.shield8.frontleftbottommax,templ->shield.shield8.frontleftbottommax,0);
      STDUPGRADE(shield.shield8.backleftbottommax,up->shield.shield8.backleftbottommax,templ->shield.shield8.backleftbottommax,0);
      break;     
    }
  }else {
    if (up->FShieldData()>0||up->RShieldData()>0|| up->LShieldData()>0||up->BShieldData()>0) {
      cancompletefully=false;
    }
  }
  

  computer.radar.color=UpgradeBoolval(computer.radar.color,up->computer.radar.color,touchme,downgrade,numave,percentage,force_change_on_nothing);
  computer.itts=UpgradeBoolval(computer.itts,up->computer.radar.color,touchme,downgrade,numave,percentage,force_change_on_nothing);
  ///do the two reversed ones below
  
  double myleak=100-shield.leak;
  double upleak=100-up->shield.leak;
  double templeak=100-(templ!=NULL?templ->shield.leak:0);
  bool ccf = cancompletefully;
  STDUPGRADE_SPECIFY_DEFAULTS(myleak,upleak,templeak,0,100,100,false,shield.leak);
  if (touchme&&myleak<=100&&myleak>=0)shield.leak=100-myleak;
  
  myleak = 1-computer.radar.maxcone;
  upleak=1-up->computer.radar.maxcone;
  templeak=1-(templ!=NULL?templ->computer.radar.maxcone:-1);
  STDUPGRADE_SPECIFY_DEFAULTS(myleak,upleak,templeak,0,0,0,false,computer.radar.maxcone);
  if (touchme)computer.radar.maxcone=1-myleak;
  static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));// DO NOT CHANGE see unit_customize.cpp
  if (up->computer.radar.lockcone!=lc) {
    myleak = 1-computer.radar.lockcone;
    upleak=1-up->computer.radar.lockcone;
    templeak=1-(templ!=NULL?templ->computer.radar.lockcone:-1);
    if (templeak == 1-lc) {
      templeak=2;
    }
    STDUPGRADE_SPECIFY_DEFAULTS(myleak,upleak,templeak,0,0,0,false,computer.radar.lockcone);
    if (touchme)computer.radar.lockcone=1-myleak;
  }
  static float tc =XMLSupport::parse_float (vs_config->getVariable ("physics","autotracking",".93"));//DO NOT CHANGE! see unit.cpp:258
  if (up->computer.radar.trackingcone!=tc) {
    myleak = 1-computer.radar.trackingcone;
    upleak=1-up->computer.radar.trackingcone;
    templeak=1-(templ!=NULL?templ->computer.radar.trackingcone:-1);
    if (templeak==1-tc) {
      templeak=2;
    }
    STDUPGRADE_SPECIFY_DEFAULTS(myleak,upleak,templeak,0,0,0,false,computer.radar.trackingcone);
    if (touchme)computer.radar.trackingcone=1-myleak;    
  }
  cancompletefully=ccf;
  //NO CLUE FOR BELOW
  if (downgrade) {
    //    STDUPGRADE(image->cargo_volume,up->image->cargo_volume,templ->image->cargo_volume,0);
    if (jump.drive>=-1&&up->jump.drive>=-1) {
      if (touchme) jump.drive=-2;
      numave++;
      percentage+=.5*((float)(100-jump.damage))/(101-up->jump.damage);
	  AddToDowngradeMap (up->name,up->jump.drive,((char *)&this->jump.drive)-((char *)this),tempdownmap);
    }
    if (cloaking!=-1&&up->cloaking!=-1) {
      if (touchme) cloaking=-1;
      numave++;
      percentage++;
	  AddToDowngradeMap (up->name,up->cloaking,((char *)&this->cloaking)-((char *)this),tempdownmap);	  
    }
  
    if (afterburnenergy<32767&&afterburnenergy<=up->afterburnenergy&&up->afterburnenergy!=0) {
      if (touchme) afterburnenergy=32767;
      numave++;
      percentage++;
	  AddToDowngradeMap (up->name,65536-((int)up->afterburnenergy),((char *)&this->afterburnenergy)-((char *)this),tempdownmap);	  	  
    }
  
  }else {
    if (touchme) {
      for (unsigned int i=0;i<up->image->cargo.size();i++) {
	if (CanAddCargo(up->image->cargo[i])) {
	  AddCargo(up->image->cargo[i],false);
	}

      }

    }
    /*    if (image->cargo_volume<up->image->cargo_volume) {
      
      if (templ!=NULL?up->image->cargo_volume+image->cargo_volume<templ->image->cargo_volume:true) {
	if (touchme)image->cargo_volume+=up->image->cargo_volume;
	numave++;
	percentage++;
      }
      }*/
    if (cloaking==-1&&up->cloaking!=-1) {
      if (touchme) {cloaking=up->cloaking;cloakmin=up->cloakmin;image->cloakrate=up->image->cloakrate; image->cloakglass=up->image->cloakglass;image->cloakenergy=up->image->cloakenergy;}
      numave++;

    }else if (cloaking!=-1&& up->cloaking!=-1) {
      cancompletefully=false;
    }
    
    if (afterburnenergy>up->afterburnenergy&&up->afterburnenergy>0||force_change_on_nothing) {
      numave++;
      if (touchme) afterburnenergy=up->afterburnenergy;
    }else if (afterburnenergy<=up->afterburnenergy&&afterburnenergy>=0&&up->afterburnenergy>0&&up->afterburnenergy<65535) {
      cancompletefully=false;
    }
    if (jump.drive==-2&&up->jump.drive>=-1||force_change_on_nothing) {
      if (touchme) {jump.drive = up->jump.drive;jump.damage=0;}
      numave++;
    }else if (jump.drive>=-1&&up->jump.drive>=-1) {
      cancompletefully=false;
    }
  }
  if (numave)
    percentage=percentage/numave;
  if (0&&touchme&&up->Mass&&numave) {
    float multiplyer =((downgrade)?-1:1);
    Mass +=multiplyer*percentage*up->Mass;
    if (Mass<(templ?templ->Mass:.000000001))
      Mass=(templ?templ->Mass:.000000001);
    Momentofinertia +=multiplyer*percentage*up->Momentofinertia;
    if (Momentofinertia<(templ?templ->Momentofinertia:0.00000001)) {
      Momentofinertia=(templ?templ->Momentofinertia:0.00000001);
    }
  }
  float MyPercentMin = ComputeMinDowngradePercent();
  if (downgrade && percentage > MyPercentMin) {
      for (std::map<int,DoubleName>::iterator i = tempdownmap.begin();i!=tempdownmap.end();++i) {
          downgrademap[(*i).first]=(*i).second;
      }
  }
  return cancompletefully;
}
Unit * makeBlankUpgrade (string templnam, int faction) {
  Unit * bl =  UnitFactory::createServerSideUnit (templnam.c_str(),true,faction);
  for (int i= bl->numCargo()-1;i>=0;i--) {
    int q =bl->GetCargo (i).quantity;
    bl->RemoveCargo (i,q);
  }
  bl->Mass=0;
  return bl;
}


const Unit * makeFinalBlankUpgrade (string name, int faction) {
  char * unitdir = GetUnitDir(name.c_str());
  string limiternam = name;
  if (unitdir!=name)
	  limiternam=string(unitdir)+string(".blank");
  free (unitdir);
  const Unit * lim= UnitConstCache::getCachedConst (StringIntKey(limiternam,faction));
  if (!lim)
    lim = UnitConstCache::setCachedConst(StringIntKey(limiternam,faction),makeBlankUpgrade(limiternam,faction));
  return lim;
}
const Unit * makeTemplateUpgrade (string name, int faction) {
  char * unitdir = GetUnitDir(name.c_str());
  string limiternam = string(unitdir)+string(".template");
  free (unitdir);
  const Unit * lim= UnitConstCache::getCachedConst (StringIntKey(limiternam,faction));
  if (!lim)
    lim = UnitConstCache::setCachedConst(StringIntKey(limiternam,faction),UnitFactory::createUnit(limiternam.c_str(),true,faction));
  return lim;
}
const Unit * loadUnitByCache(std::string name,int faction) {
      const Unit * temprate= UnitConstCache::getCachedConst (StringIntKey(name,faction));
      if (!temprate)
		  temprate = UnitConstCache::setCachedConst(StringIntKey(name,faction),UnitFactory::createUnit(name.c_str(),true,faction));
	  return temprate;
}
bool Unit::ReduceToTemplate() {
	vector <Cargo> savedCargo;
	savedCargo.swap(image->cargo);
	vector <Mount> savedWeap;
	savedWeap.swap(mounts);
	int upfac = FactionUtil::GetFaction("upgrades");
	const Unit * temprate = makeFinalBlankUpgrade (name,faction);
    bool success=false;
    double pct=0;
    if (temprate->name!=string("LOAD_FAILED")) {
        success = Upgrade(temprate,-1,-1,0,true,pct,NULL,true);
	if (pct>0)
	  success=true;
    }
	savedCargo.swap(image->cargo);
	savedWeap.swap(mounts);
    return success;
}

int Unit::RepairCost () {
	int cost =1;
	unsigned int i;
	for (i=0;i < (1+MAXVDUS+UnitImages::NUMGAUGES)*2;i++) {
        if (image->cockpit_damage[i]<1) {
			cost++;
		}
	}
        if (image->fireControlFunctionality<1)
          cost++;
        if (image->fireControlFunctionalityMax<1)
          cost++;
        if (image->SPECDriveFunctionality<1)
          cost++;
        if (image->SPECDriveFunctionalityMax<1)
          cost++;
        if (image->CommFunctionality<1)
          cost++;
        if (image->CommFunctionalityMax<1)
          cost++;
        if (image->LifeSupportFunctionality<1)
          cost++;
        if (image->LifeSupportFunctionalityMax<1)
          cost++;
        
	for (i=0;i<numCargo();++i) {
		if (GetCargo(i).category.find(DamagedCategory)==0)
			cost++;
	}
	return cost;
}
int Unit::RepairUpgrade () {
	vector <Cargo> savedCargo;
	savedCargo.swap(image->cargo);
	vector <Mount> savedWeap;
	savedWeap.swap(mounts);
	int upfac = FactionUtil::GetFaction("upgrades");
	const Unit * temprate = makeFinalBlankUpgrade (name,faction);
    int success=0;
    double pct=0;
    if (temprate->name!=string("LOAD_FAILED")) {
        success = Upgrade(temprate,0,0,0,false,pct,NULL,false)?1:0;
	if (pct>0)
	  success=1;
    }
	savedCargo.swap(image->cargo);
	savedWeap.swap(mounts);
    UnitImages * im= &GetImageInformation();
    for (int i=0;i < (1+MAXVDUS+UnitImages::NUMGAUGES)*2;i++) {
        if (im->cockpit_damage[i]<1) {
            im->cockpit_damage[i]=1;
            success+=1;
            pct = 1;
        }
    }
    if (im->fireControlFunctionality<1) {
      im->fireControlFunctionality=1;
      pct=1;
      success+=1;
    }
    if (im->fireControlFunctionalityMax<1) {
      im->fireControlFunctionalityMax=1;
      pct=1;
      success+=1;
    }
    if (im->SPECDriveFunctionality<1) {
      im->SPECDriveFunctionality=1;
      pct=1;
      success+=1;
    }
    if (im->SPECDriveFunctionalityMax<1) {
      im->SPECDriveFunctionalityMax=1;
      pct=1;
      success+=1;
    } 
    if (im->CommFunctionality<1) {
      im->CommFunctionality=1;
      pct=1;
      success+=1;
    }
    if (im->CommFunctionalityMax<1) {
      im->CommFunctionalityMax=1;
      pct=1;
      success+=1;
    }
    if (im->LifeSupportFunctionality<1) {
      im->LifeSupportFunctionality=1;
      pct=1;
      success+=1;
    }
    if (im->LifeSupportFunctionalityMax<1) {
      im->LifeSupportFunctionalityMax=1;
      pct=1;
      success+=1;
    }
     
	bool ret = success && pct>0;
	static bool ComponentBasedUpgrades = XMLSupport::parse_bool (vs_config->getVariable("physics","component_based_upgrades","false"));
	if (ComponentBasedUpgrades){
	for (unsigned int i=0;i<numCargo();++i) {
		if (GetCargo(i).category.find(DamagedCategory)==0){
			success++;
			static int damlen = strlen(DamagedCategory);
			GetCargo(i).category="upgrades/"+GetCargo(i).category.substr(damlen);
		}
	}
	}else if (ret) {
		const Unit * maxrecharge= makeTemplateUpgrade(name+".template",faction);
		
		Unit * mpl = UnitFactory::getMasterPartList();
		for (unsigned int i=0;i<mpl->numCargo();++i) {
			if (mpl->GetCargo(i).category.find("upgrades")==0) {
				const Unit * up = loadUnitByCache(mpl->GetCargo(i).content,upfac);
				//now we analyzify up!
				if (up->MaxShieldVal()==MaxShieldVal()&&up->shield.recharge>shield.recharge) {
					shield.recharge = up->shield.recharge;
					if (maxrecharge)
						if (shield.recharge>maxrecharge->shield.recharge)
							shield.recharge=maxrecharge->shield.recharge;
				}
				if (up->maxenergy==maxenergy&&up->recharge>recharge){
					recharge = up->recharge;
					if (recharge>maxrecharge->recharge)
						recharge= maxrecharge->recharge;

				}
			}
		}
		
	}
    return success;
}
/***********************************************************************************/
/**** UNIT_CARGO STUFF                                                            */
/***********************************************************************************/

/***************** UNCOMMENT GETMASTERPARTLIST WHEN MODIFIED FACTION STUFF !!!!!! */

float Unit::PriceCargo (const std::string &s) {
  Cargo tmp;
  tmp.content=s;
  vector <Cargo>::iterator mycargo = std::find (image->cargo.begin(),image->cargo.end(),tmp);
  if (mycargo==image->cargo.end()) {
    static float spacejunk=parse_float (vs_config->getVariable ("cargo","space_junk_price","10"));
    return spacejunk;
  }
  float price;
 	/*
  if (mycargo==image->cargo.end()) {
   Cargo * masterlist;
    if ((masterlist=GetMasterPartList (s.c_str()))!=NULL) {
      price =masterlist->price;
    } else {
      static float spacejunk=parse_float (vs_config->getVariable ("cargo","space_junk_price","10"));
      price = spacejunk;
    }
  } else {
  */
    price = (*mycargo).price;
  //}
  return price;
}

static const GFXColor disable (1,0,0,1);
extern int GetModeFromName (const char *);
extern double ComputeMinDowngradePercent();

vector <CargoColor>& Unit::FilterDowngradeList (vector <CargoColor> & mylist, bool downgrade)
{
  const Unit * templ=NULL;
  const Unit * downgradelimit=NULL;
  static bool staticrem =XMLSupport::parse_bool (vs_config->getVariable ("general","remove_impossible_downgrades","true"));
  static float MyPercentMin= ComputeMinDowngradePercent();

  for (unsigned int i=0;i<mylist.size();i++) {
    bool removethis=staticrem;
    int mode=GetModeFromName(mylist[i].cargo.content.c_str());
    if (mode!=2 || (!downgrade)) {
      const Unit * NewPart =  UnitConstCache::getCachedConst (StringIntKey (mylist[i].cargo.content.c_str(),FactionUtil::GetFaction("upgrades")));
      if (!NewPart){
	NewPart= UnitConstCache::setCachedConst (StringIntKey (mylist[i].cargo.content,FactionUtil::GetFaction("upgrades")),UnitFactory::createUnit(mylist[i].cargo.content.c_str(),false,FactionUtil::GetFaction("upgrades")));
      }
      if (NewPart->name==string("LOAD_FAILED")) {
	const Unit * NewPart = UnitConstCache::getCachedConst (StringIntKey (mylist[i].cargo.content.c_str(),faction));
	if (!NewPart){
	  NewPart= UnitConstCache::setCachedConst (StringIntKey (mylist[i].cargo.content, faction),
				       UnitFactory::createUnit(mylist[i].cargo.content.c_str(),false,faction));
	}
      }
      if (NewPart->name!=string("LOAD_FAILED")) {
	int maxmountcheck = NewPart->GetNumMounts()?GetNumMounts():1;
	char * unitdir  = GetUnitDir(name.c_str());
	string templnam = string(unitdir)+".template";
        string limiternam = string(unitdir)+".blank";
        
	if (!downgrade) {
	  templ = UnitConstCache::getCachedConst (StringIntKey(templnam,faction));
	  if (templ==NULL) {
	    templ = UnitConstCache::setCachedConst (StringIntKey(templnam,faction),UnitFactory::createUnit (templnam.c_str(),true,this->faction));
	  }
          if (templ->name == std::string("LOAD_FAILED")) templ = NULL;

        }else {
            downgradelimit = UnitConstCache::getCachedConst (StringIntKey(limiternam,faction));
            if (downgradelimit==NULL) {
                downgradelimit = UnitConstCache::setCachedConst (StringIntKey (limiternam,faction),UnitFactory::createUnit(limiternam.c_str(),true,this->faction));
            }
            if (downgradelimit->name == std::string("LOAD_FAILED")) downgradelimit = NULL;
        }
	free (unitdir);
	for (int m=0;m<maxmountcheck;m++) {
	  int s =0;
	  for (un_iter ui=getSubUnits();s==0||((*ui)!=NULL);++ui,++s) {
	    double percent=1;
	    if (downgrade) {
	      if (canDowngrade (NewPart,m,s,percent,downgradelimit)) {
		if (percent>MyPercentMin) {
		  removethis=false;
		  break;
		}
	      }  
	    }else {

	      if (canUpgrade (NewPart,m,s,mode,false/*force*/, percent,templ)) {
		removethis=false;
		break;
	      }
	    }
	    
	    if (*ui==NULL) {
	    break;
	    }
	  }
	}
      }
  
    } else {
      removethis=true;
    }
    if (removethis) {
      if (downgrade) {
        mylist.erase (mylist.begin()+i);	  
        i--;
      } else {
        mylist[i].color=disable;
      }
    }
  }
  return mylist;
}

vector <CargoColor>& Unit::FilterUpgradeList (vector <CargoColor> & mylist) {
	static bool filtercargoprice = XMLSupport::parse_bool (vs_config->getVariable ("cargo","filter_expensive_cargo","false"));
	if (filtercargoprice) {
	Cockpit * cp = _Universe->isPlayerStarship (this);
	if (cp) {
	  for (unsigned int i=0;i<mylist.size();i++) {
	    if (mylist[i].cargo.price>cp->credits) {
//	      mylist.erase (mylist.begin()+i);
//	      i--;
          mylist[i].color=disable;
	    }
	  }
	}
	}
	return FilterDowngradeList(mylist,false);
}
inline float uniformrand (float min, float max) {
	return ((float)(rand ())/RAND_MAX)*(max-min)+min;
}
inline QVector randVector (float min, float max) {
	return QVector (uniformrand(min,max),
					uniformrand(min,max),
					uniformrand(min,max));
}
extern void SwitchUnits (Unit *,Unit*);
void Unit::EjectCargo (unsigned int index) {
  Cargo * tmp=NULL;
  Cargo ejectedPilot;
  string name;

  Cockpit * cp = NULL;
  if (index==((unsigned int)-1)) {
    int pilotnum = _Universe->CurrentCockpit();
    name = "Pilot";
    if (NULL!=(cp = _Universe->isPlayerStarship (this))) {
      string playernum =string("player")+((pilotnum==0)?string(""):XMLSupport::tostring(pilotnum));
      //name = vs_config->getVariable(playernum,"callsign","TigerShark");
    }
    ejectedPilot.content="eject";
    ejectedPilot.mass=.1;
    ejectedPilot.volume=1;
    tmp = &ejectedPilot;
  }
  if (index<numCargo()) {
    tmp = &GetCargo (index);
  }
  if (tmp) {
    string tmpcontent=tmp->content;
    if (tmp->mission)
      tmpcontent="Mission_Cargo";

    if (tmp->quantity>0) {
      const int sslen=strlen("starships");
      Unit * cargo = NULL;
      if (tmp->category.length()>=(unsigned int)sslen) {
	if ((!tmp->mission)&&memcmp (tmp->category.c_str(),"starships",sslen)==0) {
	  string ans = tmpcontent;
	  string::size_type blank = ans.find (".blank");
	  if (blank != string::npos) {
	    ans = ans.substr (0,blank);
	  }
	  Flightgroup * fg = this->getFlightgroup();
	  int fgsnumber=0;
	  if (fg!=NULL) {
	    fgsnumber=fg->nr_ships;
	    fg->nr_ships++;
	    fg->nr_ships_left++;
	  }
	  cargo = UnitFactory::createUnit (ans.c_str(),false,faction,"",fg,fgsnumber);
	  cargo->PrimeOrders();
	  cargo->SetAI (new Orders::AggressiveAI ("default.agg.xml","default.int.xml"));
	  cargo->SetTurretAI();	  
	  //he's alive!!!!!
	}
      }
      if (!cargo) {
		  if (tmpcontent=="eject") {
			  cargo = UnitFactory::createUnit ("eject",false,faction);
			  int fac = FactionUtil::GetFaction("neutral");
			  cargo->faction=fac;//set it back to neutral so that no one will bother with 'im
		  }else {
			  string tmpnam = tmpcontent+".cargo";
			  cargo = UnitFactory::createUnit (tmpnam.c_str(),false,FactionUtil::GetFaction("upgrades"));
		  }
      }
      if (cargo->name=="LOAD_FAILED") {
	cargo->Kill();
	cargo = UnitFactory::createUnit ("generic_cargo",false,FactionUtil::GetFaction("upgrades"));
      }
      if (cargo->rSize()>=rSize()) {
	cargo->Kill();
      }else {
	cargo->SetPosAndCumPos (Position()+randVector(-rSize(), rSize()));
	cargo->SetOwner (this);
	cargo->SetVelocity(Velocity+randVector(-.25,.25).Cast());
	cargo->Mass = tmp->mass;
	if (name.length()>0) {
	  cargo->name=name;
	} else {
	  if (tmp) {
	    cargo->name=tmpcontent;
	  }
	}
	if (cp&&_Universe->numPlayers()==1) {
	  cargo->SetOwner(NULL);
	  PrimeOrders();
	  cargo->SetTurretAI();
	  cargo->faction=faction;
	  cp->SetParent (cargo,"","",Position());
	  SwitchUnits (NULL,cargo);
	}
	_Universe->activeStarSystem()->AddUnit(cargo);
	if ((unsigned int) index!=((unsigned int)-1)) {
	  if (index<image->cargo.size()) {
	    RemoveCargo (index,1,true);
	  }
	}
      }
    }

  }
}

int Unit::RemoveCargo (unsigned int i, int quantity,bool eraseZero) {
  assert (i<image->cargo.size());
  if (quantity>image->cargo[i].quantity)
    quantity=image->cargo[i].quantity;
  static bool usemass = XMLSupport::parse_bool(vs_config->getVariable ("physics","use_cargo_mass","true"));
  if (usemass)
	  Mass-=quantity*image->cargo[i].mass;
  image->cargo[i].quantity-=quantity;
  if (image->cargo[i].quantity<=0&&eraseZero)
    image->cargo.erase (image->cargo.begin()+i);
  return quantity;
}


void Unit::AddCargo (const Cargo &carg, bool sort) {
  static bool usemass = XMLSupport::parse_bool(vs_config->getVariable ("physics","use_cargo_mass","true"));
  if (usemass)
	  Mass+=carg.quantity*carg.mass;
  image->cargo.push_back (carg);   
  if (sort)
    SortCargo();
}

bool Unit::CanAddCargo (const Cargo &carg)const {
  float total_volume=carg.quantity*carg.volume + CargoVolume();
  if  (total_volume<=image->cargo_volume)
    return true;
  const Unit * un;
  for (un_kiter i=viewSubUnits();(un = *i)!=NULL;i++) {
    if (un->CanAddCargo (carg)) {
      return true;
    }
  }
  return false;
}

// The cargo volume of this ship when empty.  Max cargo volume.
float Unit::EmptyCargoVolume(void) const {
	return image->cargo_volume;
}

float Unit::CargoVolume(void) const {
	float result = 0.0;
	for(int i=0; i<image->cargo.size(); i++) {
		result += image->cargo[i].quantity*image->cargo[i].volume;
	}

	return result;
}


UnitImages &Unit::GetImageInformation() {
  return *image;
}

Cargo& Unit::GetCargo (unsigned int i) {
  return image->cargo[i];
}

void Unit::GetCargoCat (const std::string &cat, vector <Cargo> &cats) {
  unsigned int max = numCargo();
  for (unsigned int i=0;i<max;i++) {
    if (GetCargo(i).category.find(cat)==0) {
      cats.push_back (GetCargo(i));
    }
  }
}

Cargo* Unit::GetCargo (const std::string &s, unsigned int &i) {
  Cargo searchfor;
  searchfor.content=s;
  vector<Cargo>::iterator tmp =(std::find(image->cargo.begin(),image->cargo.end(),searchfor));
  if (tmp==image->cargo.end())
    return NULL;
  i= (tmp-image->cargo.begin());
  return &(*tmp);
}

unsigned int Unit::numCargo ()const {
  return image->cargo.size();
}

std::string Unit::GetManifest (unsigned int i, Unit * scanningUnit, const Vector &oldspd) const{
///FIXME somehow mangle string
  string mangled = image->cargo[i].content;
  static float scramblingmanifest=XMLSupport::parse_float (vs_config->getVariable ("general","PercentageSpeedChangeToFaultSearch",".5"));
  if (CourseDeviation (oldspd,GetVelocity())>scramblingmanifest) {
    for (string::iterator i=mangled.begin();i!=mangled.end();i++) {
      (*i)+=(rand()%3-1);
    }
  }

  return mangled;
}

bool Unit::SellCargo (unsigned int i, int quantity, float &creds, Cargo & carg, Unit *buyer){
  if (i<0||i>=image->cargo.size()||!buyer->CanAddCargo(image->cargo[i])||Mass<image->cargo[i].mass)
    return false;
  if (quantity>image->cargo[i].quantity)
    quantity=image->cargo[i].quantity;
  carg = image->cargo[i];
  carg.price=buyer->PriceCargo (image->cargo[i].content);
  creds+=quantity*carg.price;
  carg.quantity=quantity;
  buyer->AddCargo (carg);
  
  RemoveCargo (i,quantity);
  return true;
}

bool Unit::SellCargo (const std::string &s, int quantity, float & creds, Cargo &carg, Unit *buyer){
  Cargo tmp;
  tmp.content=s;
  vector <Cargo>::iterator mycargo = std::find (image->cargo.begin(),image->cargo.end(),tmp);
  if (mycargo==image->cargo.end())
    return false;

  return SellCargo (mycargo-image->cargo.begin(),quantity,creds,carg,buyer);
}

bool Unit::BuyCargo (const Cargo &carg, float & creds){
  if (!CanAddCargo(carg)||creds<carg.quantity*carg.price) {
    return false;    
  }
  AddCargo (carg);
  creds-=carg.quantity*carg.price;
  Mass+=carg.quantity*carg.mass;
  return true;
}
bool Unit::BuyCargo (unsigned int i, unsigned int quantity, Unit * seller, float&creds) {
  Cargo soldcargo= seller->image->cargo[i];
  if (quantity>(unsigned int)soldcargo.quantity)
    quantity=soldcargo.quantity;
  if (quantity==0)
    return false;
  soldcargo.quantity=quantity;
  if (BuyCargo (soldcargo,creds)) {
    seller->RemoveCargo (i,quantity,false);
    return true;
  }
  return false;
}
bool Unit::BuyCargo (const std::string &cargo,unsigned int quantity, Unit * seller, float & creds) {
  unsigned int i;
  if (seller->GetCargo(cargo,i)) {
    return BuyCargo (i,quantity,seller,creds);
  }
  return false;
}

Unit& GetUnitMasterPartList () {
  return *UnitFactory::getMasterPartList( );
}

Cargo * GetMasterPartList(const char *input_buffer){
  unsigned int i;
  return GetUnitMasterPartList().GetCargo (input_buffer,i);
}

void Unit::ImportPartList (const std::string& category, float price, float pricedev,  float quantity, float quantdev) {
	unsigned int numcarg = GetUnitMasterPartList().numCargo();
  float minprice=FLT_MAX;
  float maxprice=0;
  for (unsigned int j=0;j<numcarg;++j) {
    if (GetUnitMasterPartList().GetCargo(j).category==category) {
      float price = GetUnitMasterPartList().GetCargo(j).price;
      if (price < minprice)
	minprice = price;
      else if (price > maxprice)
	maxprice = price;
    }
  }
  for (unsigned int i=0;i<numcarg;i++) {
    Cargo c= GetUnitMasterPartList().GetCargo (i);
    if (c.category==category) {

      static float aveweight = fabs(XMLSupport::parse_float (vs_config->getVariable ("cargo","price_recenter_factor","0")));
      c.quantity=quantity-quantdev;
      float baseprice=c.price;
      c.price*=price-pricedev;

      //stupid way
      c.quantity+=(quantdev*2+1)*((double)rand())/(((double)RAND_MAX)+1);
      c.price+=pricedev*2*((float)rand())/RAND_MAX;
      c.price=fabs(c.price);
      c.price=(c.price +(baseprice*aveweight))/ (aveweight+1);
      if (c.quantity<=0) {
	c.quantity=0;
      }else {
	//quantity more than zero
	if (maxprice>minprice+.01) {
	  float renormprice = (baseprice-minprice)/(maxprice-minprice);
	  static float maxpricequantadj = XMLSupport::parse_float (vs_config->getVariable ("cargo","max_price_quant_adj","5"));
	  static float minpricequantadj = XMLSupport::parse_float (vs_config->getVariable ("cargo","min_price_quant_adj","1"));
	  static float powah = XMLSupport::parse_float (vs_config->getVariable ("cargo","price_quant_adj_power","1"));
	  renormprice = pow(renormprice,powah);
	  renormprice *= (maxpricequantadj-minpricequantadj);
	  renormprice+=1;
	  if (renormprice>.001) {
	    c.quantity/=renormprice;
	    if (c.quantity<1)
	      c.quantity=1;
	  }
	}
      }
      if (c.price <.01)
	c.price=.01;
      c.quantity=abs (c.quantity);
      AddCargo(c,false);
    }
  }
}

std::string Unit::massSerializer (const XMLType &input, void *mythis) {
  Unit * un = (Unit *)mythis;
  float mass = un->Mass;
  static bool usemass = XMLSupport::parse_bool(vs_config->getVariable ("physics","use_cargo_mass","true"));
  for (unsigned int i=0;i<un->image->cargo.size();i++) {
    if (un->image->cargo[i].quantity>0){
      if (usemass)
	mass-=un->image->cargo[i].mass*un->image->cargo[i].quantity;
    }
  }
  return XMLSupport::tostring((float)mass);
}
std::string Unit::shieldSerializer (const XMLType &input, void * mythis) {
  Unit * un=(Unit *)mythis;
  switch (un->shield.number) {
  case 2:
    return tostring(un->shield.shield2fb.frontmax)+string("\" back=\"")+tostring(un->shield.shield2fb.backmax);
  case 8:
    return string("\" frontrighttop=\"")+tostring(un->shield.shield8.frontrighttop)+string("\" backrighttop=\"")+tostring(un->shield.shield8.backrighttop)+string("\" frontlefttop=\"")+tostring(un->shield.shield8.frontlefttop)+string("\" backlefttop=\"")+tostring(un->shield.shield8.backlefttop)+string("\" frontrightbottom=\"")+tostring(un->shield.shield8.frontrightbottom)+string("\" backrightbottom=\"")+tostring(un->shield.shield8.backrightbottom)+string("\" frontleftbottom=\"")+tostring(un->shield.shield8.frontleftbottom)+string("\" backleftbottom=\"")+tostring(un->shield.shield8.backleftbottom);
  case 4:
  default:
    return tostring(un->shield.shield4fbrl.frontmax)+string("\" back=\"")+tostring(un->shield.shield4fbrl.backmax)+string("\" left=\"")+tostring(un->shield.shield4fbrl.leftmax)+string("\" right=\"")+tostring(un->shield.shield4fbrl.rightmax);
  }
  return string("");
}

std::string Unit::mountSerializer (const XMLType &input, void * mythis) {
  Unit * un=(Unit *)mythis;
  int i=input.w.hardint;
  if (un->GetNumMounts()>i) {
    string result(lookupMountSize(un->mounts[i].size));
    if (un->mounts[i].status==Mount::INACTIVE||un->mounts[i].status==Mount::ACTIVE)
      result+=string("\" weapon=\"")+(un->mounts[i].type->weapon_name);
    if (un->mounts[i].ammo!=-1)
      result+=string("\" ammo=\"")+XMLSupport::tostring(un->mounts[i].ammo);
    if (un->mounts[i].volume!=-1) {
      result+=string("\" volume=\"")+XMLSupport::tostring(un->mounts[i].volume);
    }
	result+=string("\" xyscale=\"")+XMLSupport::tostring(un->mounts[i].xyscale)+string("\" zscale=\"")+XMLSupport::tostring(un->mounts[i].zscale);
    Matrix m;
    Transformation(un->mounts[i].GetMountOrientation(),un->mounts[i].GetMountLocation().Cast()).to_matrix(m);
    result+=string ("\" x=\"")+tostring((float)(m.p.i/parse_float(input.str)));
    result+=string ("\" y=\"")+tostring((float)(m.p.j/parse_float(input.str)));
    result+=string ("\" z=\"")+tostring((float)(m.p.k/parse_float(input.str)));

    result+=string ("\" qi=\"")+tostring(m.getQ().i);
    result+=string ("\" qj=\"")+tostring(m.getQ().j);
    result+=string ("\" qk=\"")+tostring(m.getQ().k);
     
    result+=string ("\" ri=\"")+tostring(m.getR().i);    
    result+=string ("\" rj=\"")+tostring(m.getR().j);    
    result+=string ("\" rk=\"")+tostring(m.getR().k);    
    return result;
  }else {
    return string("");
  }
}
std::string Unit::subunitSerializer (const XMLType &input, void * mythis) {
  Unit * un=(Unit *)mythis;
  int index=input.w.hardint;
  Unit *su;
  int i=0;
  for (un_iter ui=un->getSubUnits();NULL!= (su=ui.current());++ui,++i) {
    if (i==index) {
      if (su->image->unitwriter) {
	return su->image->unitwriter->getName();
      }
      return su->name;
    }
  }
  return string("destroyed_turret");
}
void Unit::setCombatRole(std::string s) {
   combatRole(ROLES::getRole(s));
}
std::string Unit::getCombatRole() const{
   return ROLES::getRole(combatRole());
}

void Unit::SortCargo() {
  Unit *un=this;
  std::sort (un->image->cargo.begin(),un->image->cargo.end());

  for (unsigned int i=0;i+1<un->image->cargo.size();i++) {
    if (un->image->cargo[i].content==un->image->cargo[i+1].content) {
      float tmpmass = un->image->cargo[i].quantity*un->image->cargo[i].mass+un->image->cargo[i+1].quantity*un->image->cargo[i+1].mass;
      float tmpvolume = un->image->cargo[i].quantity*un->image->cargo[i].volume+un->image->cargo[i+1].quantity*un->image->cargo[i+1].volume;
      un->image->cargo[i].quantity+=un->image->cargo[i+1].quantity;
      if (un->image->cargo[i].quantity) {
	tmpmass/=un->image->cargo[i].quantity;
	tmpvolume/=un->image->cargo[i].quantity;
      }
      un->image->cargo[i].volume=tmpvolume;
	  un->image->cargo[i].mission = (un->image->cargo[i].mission||un->image->cargo[i+1].mission);
      un->image->cargo[i].mass=tmpmass;
      un->image->cargo.erase(un->image->cargo.begin()+(i+1));//group up similar ones
      i--;
    }

  }
}
using XMLSupport::tostring;
using namespace std;
std::string CargoToString (const Cargo& cargo) {
  string missioncargo;
  if (cargo.mission) {
	  missioncargo = string("\" missioncargo=\"")+XMLSupport::tostring(cargo.mission);
  }
  return string ("\t\t\t<Cargo mass=\"")+XMLSupport::tostring((float)cargo.mass)+string("\" price=\"") +XMLSupport::tostring((float)cargo.price)+ string("\" volume=\"")+XMLSupport::tostring((float)cargo.volume)+string("\" quantity=\"")+XMLSupport::tostring((int)cargo.quantity)+string("\" file=\"")+cargo.content+missioncargo+ string("\"/>\n");
}

std::string Unit::cargoSerializer (const XMLType &input, void * mythis) {
  Unit * un= (Unit *)mythis;
  if (un->image->cargo.size()==0) {
    return string("0");
  }
  un->SortCargo();
  string retval("");
  if (!(un->image->cargo.empty())) {
    retval= un->image->cargo[0].category+string ("\">\n")+CargoToString(un->image->cargo[0]);
    
    for (unsigned int kk=1;kk<un->image->cargo.size();kk++) {
      if (un->image->cargo[kk].category!=un->image->cargo[kk-1].category) {
	retval+=string("\t\t</Category>\n\t\t<Category file=\"")+un->image->cargo[kk].category+string ("\">\n");
      }
      retval+=CargoToString(un->image->cargo[kk]); 
    }
    retval+=string("\t\t</Category>\n\t\t<Category file=\"nothing");
  }else {
    retval= string ("nothing");//nothing
  }
  return retval;
}

float Unit::CourseDeviation (const Vector &OriginalCourse, const Vector &FinalCourse) const{
  if (ViewComputerData().max_ab_speed()>.001)
    return ((OriginalCourse-(FinalCourse)).Magnitude()/ViewComputerData().max_ab_speed());
  else
    return (FinalCourse-OriginalCourse).Magnitude();
}

/***************************************************************************************/
/*** STAR SYSTEM JUMP STUFF                                                          ***/
/***************************************************************************************/

bool Unit::TransferUnitToSystem (StarSystem * Current) {
  if (activeStarSystem->RemoveUnit (this)) {
    this->RemoveFromSystem();  
    this->Target(NULL);
    Current->AddUnit (this);    

    activeStarSystem = Current;
    return true;
  }else {
    VSFileSystem::vs_fprintf (stderr,"Fatal Error: cannot remove starship from critical system");
  }
  return false;
}

/***************************************************************************************/
/*** UNIT_REPAIR STUFF                                                               ***/
/***************************************************************************************/

extern float rand01();
void Unit::Repair() {
  //note work slows down under time compression!
  static float repairtime =XMLSupport::parse_float(vs_config->getVariable ("physics","RepairDroidTime","1000"));
  float workunit = SIMULATION_ATOM/(repairtime*getTimeCompression());//a droid completes 1 work unit in repairtime
  switch (image->repair_droid) {
  case 6:
    //versatilize Weapons! (invent)
    if (GetNumMounts()) {
      if (rand01()<workunit) {
	int whichmount = rand()%GetNumMounts();
	mounts[whichmount].size |=(1>>(rand()%(8*sizeof(int))));
      }
      if (rand01()<workunit) {
	int whichmount= rand()%GetNumMounts();
	if (mounts[whichmount].ammo>0) {
	  mounts[whichmount].volume++;
	}
      }
    }
    if (computer.max_combat_speed<60) 
      computer.max_combat_speed+=workunit;
    if (computer.max_combat_ab_speed<160)
      computer.max_combat_ab_speed+=workunit;
  case 5:
    //increase maxrange
    computer.radar.maxrange+=workunit;
    if (computer.radar.maxcone>-1) {    //Repair MaxCone full
      computer.radar.maxcone-=workunit;
    }else {
      computer.radar.maxcone=-1;
    }
    if (computer.radar.lockcone>0) {    //Repair MaxCone full
      computer.radar.lockcone-=workunit;
    }else {
      //      computer.radar.lockcone=-1;
    }
    
    if (rand01()<workunit*.25) {
      computer.itts=true;
    }
    if (computer.radar.mintargetsize>0) {
      computer.radar.mintargetsize-=rSize()*workunit;
    }//no break...please continue, colonel
  case 4:
    if (GetNumMounts()) {    //    RepairWeapon();
      if (rand01()<workunit) {
	unsigned int i=rand()%GetNumMounts();
	if (mounts[i].status==Mount::DESTROYED) {
	  mounts[i].status=Mount::INACTIVE;
	}
      }
    }//nobreak
  case 3:
    if (computer.radar.mintargetsize>rSize()) {
      computer.radar.mintargetsize-=rSize()*workunit;
    }
    if (rand01()<.5*workunit) {
      computer.radar.color=true;
    }
    if (rand01()<workunit) {
      if (jump.damage>0)
	jump.damage--;
    }//nobreak
  case 2:
    {
      int whichgauge=rand()%(UnitImages::NUMGAUGES+1+MAXVDUS);
      if (image->cockpit_damage[whichgauge]<1) {
	image->cockpit_damage[whichgauge]+=workunit;
	if (image->cockpit_damage[whichgauge]>image->cockpit_damage[whichgauge+UnitImages::NUMGAUGES+1+MAXVDUS])
	  image->cockpit_damage[whichgauge]=image->cockpit_damage[whichgauge+UnitImages::NUMGAUGES+1+MAXVDUS];
      }
    }    
  case 1:
    if (computer.radar.maxcone>0) {    //Repair MaxCone half
      computer.radar.maxcone-=workunit;
    }
    if (computer.radar.lockcone>.7) {    //Repair MaxCone half
      computer.radar.lockcone-=workunit;
    }

    if (jump.drive!=-1) {    //    RepairJumpEnergy(jump.energy,maxenergy);
      if (jump.energy>maxenergy) {
	if (rand01()<workunit) {
	  jump.energy=maxenergy-1;
	}
      }
    }
    if (computer.radar.mintargetsize>1.5*rSize()) {//    RepairMinTargetSize
      computer.radar.mintargetsize-=rSize()*workunit;
    }

  default: 
    break;
  }
}

