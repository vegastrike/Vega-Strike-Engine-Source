#include "configxml.h"
#include "unit_generic.h"
#include "lin_time.h"
#include "xml_serializer.h"
#include "vs_path.h"
#include "file_main.h"
#include "unit_factory.h"
#include <algorithm>
#ifdef _WIN32
#define strcasecmp stricmp
#endif

float copysign (float x, float y) {
	if (y>0)
			return x;
	else
			return -x;
}

float capship_size=500;
unsigned short apply_float_to_short (float tmp) {
  unsigned  short ans = (unsigned short) tmp;
  tmp -=ans;//now we have decimal;
  if (((float)rand())/((float)RAND_MAX)<tmp)
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
static Hashtable <long, Unit, char[2095]> deletedUn;
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
	Init();
}
Unit::Unit() {
	Init();
}

Unit::Unit (std::vector <string> & meshes, bool SubU, int fact) {
  Init();
  this->faction = fact;
  SubUnit = SubU;
  meshdata = meshes;
  meshes.clear();
  meshdata.push_back(NULL);
  calculate_extent(false);
}

Unit::Unit(const char *filename, bool SubU, int faction,std::string unitModifications, Flightgroup *flightgrp,int fg_subnumber) {
	Init();
	//update_ani_cache();
	//if (!SubU)
	//  _Universe.AccessCockpit()->savegame->AddUnitToSave(filename,UNITPTR,GetFaction(faction),(long)this);
	this->player = false;
	SubUnit = SubU;
	this->faction = faction;
	SetFg (flightgrp,fg_subnumber);
	bool doubleup=false;
	char * my_directory=GetUnitDir(filename);
	vssetdir (GetSharedUnitPath().c_str());
	FILE * fp=NULL;
	if (!fp) {
	  const char *c;
	  if ((c=FactionUtil::GetFaction(faction)))
	    vschdir (c);
	  else
	    vschdir ("unknown");
	  doubleup=true;
	  vschdir (my_directory);
	} else {
	  fclose (fp);
	}
	fp = fopen (filename,"r");
	if (!fp) {
	  vscdup();
	  vscdup();
	  doubleup=false;
	  vschdir (my_directory);
	  fp = fopen (filename,"r");
	}

	if (!fp) {
	  if (doubleup) {
	    vscdup();
	  }
	  vscdup();
	  vschdir ("neutral");
	  faction=FactionUtil::GetFaction("neutral");//set it to neutral
	  doubleup=true;
	  vschdir (my_directory);
	  fp = fopen (filename,"r");
	  if (fp) fclose (fp); 
	  else {
	    fprintf (stderr,"Warning: Cannot locate %s",filename);	  
	    meshdata.clear();
	    meshdata.push_back(NULL);
	    this->name=string("LOAD_FAILED");
	    //	    assert ("Unit Not Found"==NULL);
	  }
	}else {
	  fclose (fp);
	}
	free(my_directory);
	/*Insert file loading stuff here*/
	if(1&&fp) {
	  name = filename;

	  LoadXML(filename,unitModifications.c_str());
	}
	if (1) {
	  calculate_extent(false);
	  ToggleWeapon(true);//change missiles to only fire 1
       	  vscdup();
	  if (doubleup) 
	    vscdup();
	  vsresetdir();
	  return;
	}
	LoadFile(filename);
	int nummesh;
	ReadInt(nummesh);
	meshdata.clear();
	for(int meshcount = 0; meshcount < nummesh; meshcount++)
	{
		int meshtype;
		ReadInt(meshtype);
		char meshfilename[64];
		float x,y,z;
		//ReadMesh(meshfilename, x,y,z);
		meshdata.push_back(meshfilename);

		//		meshdata[meshcount]->SetPosition(Vector (x,y,z));
	}
	meshdata.push_back(NULL);
	int numsubunit;
	ReadInt(numsubunit);
	for(int unitcount = 0; unitcount < numsubunit; unitcount++)
	{
		char unitfilename[64];
		float x,y,z;
		int type;
		ReadUnit(unitfilename, type, x,y,z);
		Unit *un;
		switch(type)
		{
		default:
		  SubUnits.prepend (un=UnitFactory::createGenericUnit (unitfilename,true,faction,unitModifications,flightgroup,flightgroup_subnumber));

		}
		un->SetPosition(QVector(x,y,z));
	}

	int restricted;
	float min, max;
	ReadInt(restricted); //turrets and weapons
	//ReadInt(restricted); // What's going on here? i hsould have 2, but that screws things up

	ReadRestriction(restricted, min, max);
	if(restricted) {
	  //RestrictYaw(min,max);
	}
	ReadRestriction(restricted, min, max);
	if(restricted) {
	  //RestrictPitch(min,max);
	}
	ReadRestriction(restricted, min, max);
	if(restricted) {
	  //RestrictRoll(min,max);
	}
	float maxspeed, maxaccel, mass;
	ReadFloat(maxspeed);
	ReadFloat(maxaccel);
	ReadFloat(mass);


	CloseFile();
	calculate_extent(false);
	ToggleWeapon(true);//change missiles to only fire 1
	vscdup();
	if (fp) 
	  vscdup();
	vsresetdir();
}

Unit::~Unit()
{
  free(image->cockpit_damage);
  if ((!killed)) {
    fprintf (stderr,"Assumed exit on unit %s(if not quitting, report error)\n",name.c_str());
  }
  if (ucref) {
    fprintf (stderr,"DISASTER AREA!!!!");
  }
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"stage %d %x %d\n", 0,this,ucref);
  fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x ", 1,planet);
  fflush (stderr);
  fprintf (stderr,"%d %x\n", 2,image->hudImage);
  fflush (stderr);
#endif
  if (image->hudImage )
    delete image->hudImage;
  if (image->unitwriter)
    delete image->unitwriter;
  unsigned int i;
  for (i=0;i<image->destination.size();i++) {
    delete [] image->destination[i];
  }

#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x", 3,image);
  fflush (stderr);
#endif
  delete image;
  delete sound;
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x %x", 4,bspTree, bspShield);
  fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d", 5);
  fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x", 6,mounts);
  fflush (stderr);
#endif

#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x ", 9, halos);
  fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x ", 1,mounts);
  fflush (stderr);
#endif
	for( vector<Mount *>::iterator jj=mounts.begin(); jj!=mounts.end(); jj++)
	{
		// Free all mounts elements
		if( (*jj)!=NULL)
			delete (*jj);
	}
	mounts.clear();
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d", 0);
  fflush (stderr);
#endif
}
void Unit::Init()
{
	this->player=0;
	this->networked=0;
#ifdef CONTAINER_DEBUG
  UncheckUnit (this);
#endif
  static float capsize = XMLSupport::parse_float(vs_config->getVariable("physics","capship_size","500"));
  capship_size=capsize;
  activeStarSystem=NULL;
  xml_str=NULL;
  docked=NOT_DOCKED;
  SubUnit =0;

  jump.energy = 100;
  jump.delay=5;
  jump.damage=0;
  jump.drive=-2;// disabled
  afterburnenergy=0;
  planet=NULL;
  nebula=NULL;
  image = new UnitImages;
  sound = new UnitSounds;
  limits.structurelimits=Vector(0,0,1);
  limits.limitmin=-1;
  cloaking=-1;
  image->repair_droid=0;
  image->ecm=0;
  image->cloakglass=false;
  image->cargo_volume=0;
  image->unitwriter=NULL;
  cloakmin=image->cloakglass?1:0;
  image->cloakrate=100;
  image->cloakenergy=0;
  image->forcejump=false;
  sound->engine=-1;  sound->armor=-1;  sound->shield=-1;  sound->hull=-1; sound->explode=-1;
  sound->cloak=-1;
  image->hudImage=NULL;
  owner = NULL;
  faction =0;
  resolveforces=true;
  colTrees=NULL;
  invisible=false;
  //origin.Set(0,0,0);
  corner_min.Set (FLT_MAX,FLT_MAX,FLT_MAX);
  corner_max.Set (-FLT_MAX,-FLT_MAX,-FLT_MAX);
  
  shieldtight=0;//sphere mesh by default
  energy=maxenergy=1;
  recharge = 1;
  shield.recharge=shield.leak=0;
  shield.fb[0]=shield.fb[1]=shield.fb[2]=shield.fb[3]=armor.front=armor.back=armor.right=armor.left=0;
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
  mass = .01;
  fuel = 000;

  static Vector myang(XMLSupport::parse_float (vs_config->getVariable ("general","pitch","0")),XMLSupport::parse_float (vs_config->getVariable ("general","yaw","0")),XMLSupport::parse_float (vs_config->getVariable ("general","roll","0")));
  static float rr = XMLSupport::parse_float (vs_config->getVariable ("graphics","hud","radarRange","20000"));
  static float minTrackingNum = XMLSupport::parse_float (vs_config->getVariable("physics",
										  "autotracking",
										".93"));// DO NOT CHANGE see unit_customize.cpp
    
  static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));// DO NOT CHANGE see unit_customize.cpp

  MomentOfInertia = .01;
  AngularVelocity = myang;
  cumulative_velocity=Velocity = Vector(0,0,0);
  
  NetTorque =NetLocalTorque = Vector(0,0,0);
  NetForce = Vector(0,0,0);
  NetLocalForce=Vector(0,0,0);

  selected = false;
  image->selectionBox = NULL;

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
  computer.max_speed=1;
  computer.max_ab_speed=1;
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
 //int numg= 1+MAXVDUS+Cockpit::NUMGAUGES;
  //image->cockpit_damage=(float*)malloc((numg)*sizeof(float));
  //for (unsigned int damageiterator=0;damageiterator<numg;damageiterator++) {
//	image->cockpit_damage[damageiterator]=1;
//  }
  // No CollideInfo yet
  /*
  CollideInfo.object.u = NULL;
  CollideInfo.type = LineCollide::UNIT;
  CollideInfo.Mini.Set (0,0,0);
  CollideInfo.Maxi.Set (0,0,0);
  */
  // No AI here
  //SetAI (new Order());
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
	  if (mounts[i]->status==Mount::ACTIVE&&(mounts[i]->size&weapon_info::AUTOTRACKING)) {
      trackingcone= computer.radar.trackingcone;
    }
    if (mounts[i]->status==Mount::ACTIVE&&mounts[i]->type->LockTime>0&&mounts[i]->time_to_lock<=0) {
      missilelock=true;
    }
  }
  return trackingcone;
}

void Unit::getAverageGunSpeed(float & speed, float &range) const {
   if (GetNumMounts()) {
     range=0;
     speed=0;
     int nummt = GetNumMounts();
     for (int i=0;i<GetNumMounts();i++) {
       if (mounts[i]->type->type!=weapon_info::PROJECTILE) {
	 if (mounts[i]->type->Range > range) {
	   range=mounts[i]->type->Range;
	 }
	 speed+=mounts[i]->type->Speed;
       } else {
	 nummt--;
       }
     }
     if (nummt) {
       speed/=nummt;
     }
   }
  
}

QVector Unit::PositionITTS (const QVector & posit, float speed) const{
  QVector retval = Position()-posit;
  speed = retval.Magnitude()/speed;//FIXME DIV/0 POSSIBLE
  retval = Position()+GetVelocity().Cast().Scale(speed);
  return retval;
}

float Unit::cosAngleTo (Unit * targ, float &dist, float speed, float range) const{
  Vector Normal (cumulative_transformation_matrix.getR());
   //   if (range!=FLT_MAX) {
   //     getAverageGunSpeed(speed,range);
   //   }
   QVector totarget (targ->PositionITTS(cumulative_transformation.position, speed+((targ->Position()-Position()).Normalize().Dot (GetVelocity().Cast()))));
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
    Transformation finaltrans (mounts[i]->GetMountLocation());
    finaltrans.Compose (cumulative_transformation, cumulative_transformation_matrix);
    finaltrans.to_matrix (mat);
    Vector Normal (mat.getR());
    
    QVector totarget (targ->PositionITTS(finaltrans.position, mounts[i]->type->Speed));
    
    tmpcos = Normal.Dot (totarget.Cast());
    tmpdist = totarget.Magnitude();
    if (tmpcos>0) {
      tmpcos = tmpdist*tmpdist - tmpcos*tmpcos;
      tmpcos = targ->rSize()/tmpcos;//one over distance perpendicular away from straight ahead times the size...high is good WARNING POTENTIAL DIV/0
    } else {
      tmpcos /= tmpdist;
    }
    tmpdist /= mounts[i]->type->Range;//UNLIKELY DIV/0
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

un_iter Unit::getSubUnits () {
  return SubUnits.createIterator();
}
un_kiter Unit::viewSubUnits() const{
  return SubUnits.constIterator();
}

void Unit::SetVisible(bool invis) {
  invisible=!invis;
}

float Unit::GetElasticity() {return .5;}


/***********************************************************************************/
/**** UNIT_FACTORY STUFF                                                           */
/***********************************************************************************/

Unit* UnitFactory::createGenericUnit( const char *filename,
		               bool        SubUnit,
		               int         faction,
		               std::string customizedUnit,
		               Flightgroup *flightgroup,
		               int         fg_subnumber )
{
    return new Unit( filename,
                     SubUnit,
                     faction,
                     customizedUnit,
                     flightgroup,
                     fg_subnumber );
}


/***********************************************************************************/
/**** UNIT_PHYSICS STUFF                                                           */
/***********************************************************************************/

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
	  }
	}
}

void Unit:: FireEngines (const Vector &Direction/*unit vector... might default to "r"*/,
					float FuelSpeed,
					float FMass)
{
	mass -= FMass; //fuel is sent out
	fuel -= FMass;
	if (fuel <0)
	{
		
		FMass +=fuel;
		mass -= fuel;
		fuel = 0; //ha ha!
	}
	NetForce += Direction *(FuelSpeed *FMass/GetElapsedTime());
}
void Unit::ApplyForce(const Vector &Vforce) //applies a force for the whole gameturn upon the center of mass
{
	NetForce += Vforce;
}
void Unit::ApplyLocalForce(const Vector &Vforce) //applies a force for the whole gameturn upon the center of mass
{
	NetLocalForce += Vforce;
}
void Unit::Accelerate(const Vector &Vforce)
{
  NetForce += Vforce * mass;
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
  fprintf (stderr,"P: %f,%f,%f Q: %f,%f,%f",p.i,p.j,p.k,q.i,q.j,q.k);
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
  fuel-=Res.Magnitude()*SIMULATION_ATOM;
  return Res;
}
Vector Unit::ClampVelocity (const Vector & velocity, const bool afterburn) {
  static float staticfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelThrust",".9"));
  static float staticabfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelAfterburn",".1"));
  float fuelclamp=(fuel<=0)?staticfuelclamp:1;
  float abfuelclamp= (fuel<=0||(energy<afterburnenergy))?staticabfuelclamp:1;
  float limit = afterburn?(abfuelclamp*(computer.max_ab_speed-computer.max_speed)+(fuelclamp*computer.max_speed)):fuelclamp*computer.max_speed;
  float tmp = velocity.Magnitude();
  if (tmp>fabs(limit)) {
    return velocity * (limit/tmp);
  }
  return velocity;
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
    energy -=apply_float_to_short( afterburnenergy*SIMULATION_ATOM);
  }

  static float staticfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelThrust",".4"));
  static float staticabfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelAfterburn","0"));
  static float abfuelusage = XMLSupport::parse_float (vs_config->getVariable ("physics","AfterburnerFuelUsage","4"));
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
  fuel-=(afterburn?abfuelusage:1)*Res.Magnitude();
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

static int applyto (unsigned short &shield, const unsigned short max, const float amt) {
  shield+=apply_float_to_short(amt);
  if (shield>max)
    shield=max;
  return (shield>=max)?1:0;
}

float Unit::MaxShieldVal() const{
  float maxshield=0;
  switch (shield.number) {
  case 2:
    maxshield = .5*(shield.fb[2]+shield.fb[3]);
    break;
  case 4:
    maxshield = .25*(float(shield.fbrl.frontmax)+shield.fbrl.backmax+shield.fbrl.leftmax+shield.fbrl.rightmax);
    break;
  case 6:
    maxshield = .25*float(shield.fbrltb.fbmax)+.75*shield.fbrltb.rltbmax;
    break;
  }
  return maxshield;
}
void Unit::RechargeEnergy() {
    energy +=apply_float_to_short (recharge *SIMULATION_ATOM);
}
void Unit::RegenShields () {
  int rechargesh=1;
  float maxshield=MaxShieldVal();
  static bool energy_before_shield=XMLSupport::parse_bool(vs_config->getVariable ("physics","engine_energy_priority","true"));
  if (!energy_before_shield) {
    RechargeEnergy();
  }
  float rec = shield.recharge*SIMULATION_ATOM>energy?energy:shield.recharge*SIMULATION_ATOM;
  if (!this->isPlayer()) {
    rec*=g_game.difficulty;
  }else {
    rec*=g_game.difficulty;//sqrtf(g_game.difficulty);
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
  switch (shield.number) {
  case 2:

    shield.fb[0]+=rec;
    shield.fb[1]+=rec;
    if (shield.fb[0]>shield.fb[2]) {
      shield.fb[0]=shield.fb[2];
    } else {
      rechargesh=0;
    }
    if (shield.fb[1]>shield.fb[3]) {
      shield.fb[1]=shield.fb[3];

    } else {
      rechargesh=0;
    }
    break;
  case 4:

    rechargesh = applyto (shield.fbrl.front,shield.fbrl.frontmax,rec)*(applyto (shield.fbrl.back,shield.fbrl.backmax,rec))*applyto (shield.fbrl.right,shield.fbrl.rightmax,rec)*applyto (shield.fbrl.left,shield.fbrl.leftmax,rec);
    break;
  case 6:
    rechargesh = (applyto(shield.fbrltb.v[0],shield.fbrltb.fbmax,rec))*applyto(shield.fbrltb.v[1],shield.fbrltb.fbmax,rec)*applyto(shield.fbrltb.v[2],shield.fbrltb.rltbmax,rec)*applyto(shield.fbrltb.v[3],shield.fbrltb.rltbmax,rec)*applyto(shield.fbrltb.v[4],shield.fbrltb.rltbmax,rec)*applyto(shield.fbrltb.v[5],shield.fbrltb.rltbmax,rec);
    break;
  }
  if (rechargesh==0)
    energy-=(short unsigned int)rec;
  if (energy_before_shield) {
    RechargeEnergy();
  }
  if (maxenergy>maxshield) {
    if (energy>maxenergy-maxshield)//allow shields to absorb xtra power
      energy=maxenergy-maxshield;  
  }else {
    energy=0;
}
}

Vector Unit::ResolveForces (const Transformation &trans, const Matrix &transmat) {
  Vector p, q, r;
  GetOrientation(p,q,r);
  Vector temp1 = (InvTransformNormal(transmat,NetTorque)+NetLocalTorque.i*p+NetLocalTorque.j*q+NetLocalTorque.k *r)*(1.0/MomentOfInertia);
  Vector temp = temp1*SIMULATION_ATOM;
  if (FINITE(temp.i)&&FINITE (temp.j)&&FINITE(temp.k)) {
    AngularVelocity += temp;
  }
  Vector temp2 = ((InvTransformNormal(transmat,NetForce) + NetLocalForce.i*p + NetLocalForce.j*q + NetLocalForce.k*r ))/mass; //acceleration
  temp = temp2*SIMULATION_ATOM;
  if (FINITE(temp.i)&&FINITE (temp.j)&&FINITE(temp.k)) {	//FIXME
    Velocity += temp;
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
void Unit::GetOrientation(Vector &p, Vector &q, Vector &r) const {
  Matrix m;
  curr_physical_state.to_matrix(m);
  p=m.getP();
  q=m.getQ();
  r=m.getR();
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

void Unit::leach (float damShield, float damShieldRecharge, float damEnRecharge) {
  recharge*=damEnRecharge;
  shield.recharge*=damShieldRecharge;
  switch (shield.number) {
  case 2:
    shield.fb[2]*=damShield;
    shield.fb[3]*=damShield;
    break;
  case 4:
    shield.fbrl.frontmax*=damShield;
    shield.fbrl.backmax*=damShield;
    shield.fbrl.leftmax*=damShield;
    shield.fbrl.rightmax*=damShield;
    break;
  case 6:
    shield.fbrltb.fbmax*=damShield;
    shield.fbrltb.rltbmax*=damShield;
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
    fprintf (stderr,"%s 0x%x - %d\n",name.c_str(),this,Unitdeletequeue.size());
#endif
  }
}

float Unit::ExplosionRadius() {
  static float expsize=XMLSupport::parse_float(vs_config->getVariable ("graphics","explosion_size","3"));
  return expsize*rSize();
}

void Unit::ArmorData (unsigned short armor[4]) const{
  //  memcpy (&armor[0],&this->armor.front,sizeof (unsigned short)*4);
  armor[0]=this->armor.front;
  armor[1]=this->armor.back;
  armor[2]=this->armor.right;
  armor[3]=this->armor.left;
}

float Unit::FuelData () const{
  return fuel;
}
float Unit::EnergyData() const{
  if (maxenergy<=MaxShieldVal()) {
    return 0;
  }
  return ((float)energy)/(maxenergy-MaxShieldVal());
}

float Unit::FShieldData() const{
  switch (shield.number) {
  case 2: { if( shield.fb[2]!=0) return shield.fb[0]/shield.fb[2];}
  case 4: { if( shield.fbrl.frontmax!=0) return ((float)shield.fbrl.front)/shield.fbrl.frontmax;}
  case 6: { if( shield.fbrltb.fbmax!=0) return ((float)shield.fbrltb.v[0])/shield.fbrltb.fbmax;}
  }
  return 0;
}
float Unit::BShieldData() const{
  switch (shield.number) {
  case 2: { if( shield.fb[3]!=0) return shield.fb[1]/shield.fb[3];}
  case 4: { if( shield.fbrl.backmax!=0) return ((float)shield.fbrl.back)/shield.fbrl.backmax;}
  case 6: { if( shield.fbrltb.fbmax!=0) return ((float)shield.fbrltb.v[1])/shield.fbrltb.fbmax;}
  }
  return 0;
}
float Unit::LShieldData() const{
  switch (shield.number) {
  case 2: return 0;//no data, captain
  case 4: { if( shield.fbrl.leftmax!=0) return ((float)shield.fbrl.left)/shield.fbrl.leftmax;}
  case 6: { if( shield.fbrltb.rltbmax!=0) return ((float)shield.fbrltb.v[3])/shield.fbrltb.rltbmax;}
  }
  return 0;
}
float Unit::RShieldData() const{
  switch (shield.number) {
  case 2: return 0;//don't react to stuff we have no data on
  case 4: { if( shield.fbrl.rightmax!=0) return ((float)shield.fbrl.right)/shield.fbrl.rightmax;}
  case 6: { if( shield.fbrltb.rltbmax!=0) return ((float)shield.fbrltb.v[2])/shield.fbrltb.rltbmax;}
  }
  return 0;
}

void Unit::ProcessDeleteQueue() {
  while (!Unitdeletequeue.empty()) {
#ifdef DESTRUCTDEBUG
    fprintf (stderr,"Eliminatin' 0x%x - %d",Unitdeletequeue.back(),Unitdeletequeue.size());
    fflush (stderr);
    fprintf (stderr,"Eliminatin' %s\n",Unitdeletequeue.back()->name.c_str());
#endif
#ifdef DESTRUCTDEBUG
    if (Unitdeletequeue.back()->SubUnit) {

      fprintf (stderr,"Subunit Deleting (related to double dipping)");

    }
#endif
    Unit * mydeleter = Unitdeletequeue.back();
    Unitdeletequeue.pop_back();
    delete mydeleter;///might modify unitdeletequeue
    
#ifdef DESTRUCTDEBUG
    fprintf (stderr,"Completed %d\n",Unitdeletequeue.size());
    fflush (stderr);
#endif

  }
}

float Unit::DealDamageToShield (const Vector &pnt, float &damage) {
  int index;
  float percent=0;
  unsigned short * targ=NULL;
  switch (shield.number){
  case 2:
    index = (pnt.k>0)?0:1;
	if( shield.fb[index+2]!=0)
	    percent = damage/shield.fb[index+2];//comparing with max
	else
		percent = 0;
    shield.fb[index]-=damage;
    damage =0;
    if (shield.fb[index]<0) {
      damage = -shield.fb[index];
      shield.fb[index]=0;
    }
    break;
  case 6:
    percent = damage/shield.fbrltb.rltbmax;
    if (fabs(pnt.i)>fabs(pnt.j)&&fabs(pnt.i)>fabs(pnt.k)) {
      if (pnt.i>0) {
	index = 3;//left
      } else {
	index = 2;//right
      }
    }else if (fabs(pnt.j)>fabs (pnt.k)) {
      if (pnt.j>0) {
	index = 4;//top;
      } else {
	index = 5;//bot;
      }
    } else {
      percent = damage/shield.fbrltb.fbmax;
      if (pnt.k>0) {
	index = 0;
      } else {
	index = 1;
      }
    }
    if (damage>shield.fbrltb.v[index]) {
      damage -= shield.fbrltb.v[index];
      shield.fbrltb.v[index]=0;
    } else {
      shield.fbrltb.v[index]-=apply_float_to_short (damage);
      damage = 0;
    }
    break;
  case 4:
  default:
    if (fabs(pnt.k)>fabs (pnt.i)) {
      if (pnt.k>0) {
	targ = &shield.fbrl.front;
	percent = damage/shield.fbrl.frontmax;
      } else {
	targ = &shield.fbrl.back;
	percent = damage/shield.fbrl.backmax;
      }
    } else {
      if (pnt.i>0) {
	percent = damage/shield.fbrl.leftmax;
	targ = &shield.fbrl.left;
      } else {
	targ = &shield.fbrl.right;
	percent = damage/shield.fbrl.rightmax;
      }
    }
    if (damage>*targ) {
      damage-=*targ;
      *targ=0;
    } else {
      *targ -= apply_float_to_short (damage);	
      damage=0;
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
    index = (pnt.k>0)?0:1;
    return shield.fb[index]>shieldmin;
    break;
  case 6:
    if (fabs(pnt.i)>fabs(pnt.j)&&fabs(pnt.i)>fabs(pnt.k)) {
      if (pnt.i>0) {
	index = 3;//left
      } else {
	index = 2;//right
      }
    }else if (fabs(pnt.j)>fabs (pnt.k)) {
      if (pnt.j>0) {
	index = 4;//top;
      } else {
	index = 5;//bot;
      }
    } else {
      if (pnt.k>0) {
	index = 0;
      } else {
	index = 1;
      }
    }
    return shield.fbrltb.v[index]>shieldmin;
    break;
  case 4:
  default:
    if (fabs(pnt.k)>fabs (pnt.i)) {
      if (pnt.k>0) {
	return shield.fbrl.front>shieldmin;
      } else {
	return shield.fbrl.back>shieldmin;
      }
    } else {
      if (pnt.i>0) {
	return shield.fbrl.left>shieldmin;
      } else {
	return shield.fbrl.right>shieldmin;
      }
    }
    return false;
  }
}


/***********************************************************************************/
/**** UNIT_WEAPON STUFF                                                            */
/***********************************************************************************/

Unit::Mount::Mount (){static weapon_info wi(weapon_info::BEAM); type=&wi; size=weapon_info::NOWEAP; ammo=-1;status= UNCHOSEN; processed=Mount::PROCESSED;sound=-1;}
Unit::Mount::Mount(const string& filename, short am,short vol): size(weapon_info::NOWEAP),ammo(am),sound(-1){
  static weapon_info wi(weapon_info::BEAM);
  type = &wi;
  this->volume=vol;
  ref.gun = NULL;
  status=(UNCHOSEN);
  processed=Mount::PROCESSED;
  /*
  weapon_info * temp = getTemplate (filename);  
  if (temp==NULL) {
    status=UNCHOSEN;
    time_to_lock=0;
  }else {
    type = temp;
    status=ACTIVE;
    time_to_lock = temp->LockTime;
  }
  */
}

void Unit::VelocityReference (Unit *targ) {
  computer.velocity_ref.SetUnit(targ);
}
void Unit::SetOwner(Unit *target) {
  owner=target;
}

void Unit::Cloak (bool loak) {
  if (loak) {
    if (image->cloakenergy<energy) {
      image->cloakrate =(image->cloakrate>=0)?image->cloakrate:-image->cloakrate; 
      if (cloaking==(short)32768) {
	cloaking=32767;
      } else {
       
      }
    }
  }else {
    image->cloakrate= (image->cloakrate>=0)?-image->cloakrate:image->cloakrate;
    if (cloaking==cloakmin)
      cloaking++;
  }
}

void Unit::Mount::Activate (bool Missile) {
  if ((type->type==weapon_info::PROJECTILE)==Missile) {
    if (status==INACTIVE)
      status = ACTIVE;
  }
}
///Sets this gun to inactive, unless unchosen or destroyed
void Unit::Mount::DeActive (bool Missile) {
  if ((type->type==weapon_info::PROJECTILE)==Missile) {
    if (status==ACTIVE)
      status = INACTIVE;
  }
}
void Unit::SelectAllWeapon (bool Missile) {
  for (int i=0;i<GetNumMounts();i++) {
    mounts[i]->Activate (Missile);
  }
}

void Unit::UnFire () {
  for (int i=0;i<GetNumMounts();i++) {
    mounts[i]->UnFire();//turns off beams;
  }
}

///cycles through the loop twice turning on all matching to ms weapons of size or after size
void Unit::ActivateGuns (const weapon_info * sz, bool ms) {
  for (int j=0;j<2;j++) {
    for (int i=0;i<GetNumMounts();i++) {
      if (mounts[i]->type==sz) {
	if (mounts[i]->status<Mount::DESTROYED&&(mounts[i]->type->type==weapon_info::PROJECTILE)==ms) {
	  mounts[i]->Activate(ms);
	}else {
	  sz = mounts[(i+1)%GetNumMounts()]->type;
	}
      }
    }
  }
}

///In short I have NO CLUE how this works! It just...grudgingly does
void Unit::ToggleWeapon (bool Missile) {
  int activecount=0;
  int totalcount=0;
  bool lasttotal=true;
//  weapon_info::MOUNT_SIZE sz = weapon_info::NOWEAP;
  const weapon_info * sz=NULL;
  if (GetNumMounts()<1)
    return;
  sz = mounts[0]->type;
  for (int i=0;i<GetNumMounts();i++) {
    if ((mounts[i]->type->type==weapon_info::PROJECTILE)==Missile&&!Missile&&mounts[i]->status<Mount::DESTROYED) {
      totalcount++;
      lasttotal=false;
      if (mounts[i]->status==Mount::ACTIVE) {
	activecount++;
	lasttotal=true;
	mounts[i]->DeActive (Missile);
	if (i==GetNumMounts()-1) {
	  sz=mounts[0]->type;
	}else {
	  sz =mounts[i+1]->type;
	}
      }
    }
    if ((mounts[i]->type->type==weapon_info::PROJECTILE)==Missile&&Missile&&mounts[i]->status<Mount::DESTROYED) {
      if (mounts[i]->status==Mount::ACTIVE) {
	activecount++;//totalcount=0;
	mounts[i]->DeActive (Missile);
	if (lasttotal) {
	  totalcount=(i+1)%GetNumMounts();
	  if (i==GetNumMounts()-1) {
	    sz = mounts[0]->type;
	  }else {
	    sz =mounts[i+1]->type;
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
	if (mounts[i]->type==sz) {
	  if ((mounts[i]->type->type==weapon_info::PROJECTILE)) {
	    mounts[i]->Activate(true);
	    return;
	  }else {
	    sz = mounts[(i+1)%GetNumMounts()]->type;
	  }
	}
      }
      i=0;
    }
  }
  if (totalcount==activecount) {
    ActivateGuns (mounts[0]->type,Missile);
  } else {
    if (lasttotal) {
      SelectAllWeapon(Missile);
    }else {
      ActivateGuns (sz,Missile);
    }
  }
}


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
    if (mounts[i]->status==Mount::ACTIVE&&mounts[i]->type->LockTime>0&&mounts[i]->time_to_lock<=0&&mounts[i]->type->type==weapon_info::PROJECTILE) {
      missilelock=true;
    }else {
      if (mounts[i]->status==Mount::ACTIVE&&mounts[i]->type->LockTime==0&&mounts[i]->type->type==weapon_info::PROJECTILE&&mounts[i]->time_to_lock<=0) {
	dumblock=true;
      }
    }    
  }
  return (missilelock?1:(dumblock?-1:0));
}


/***********************************************************************************/
/**** UNIT_COLLIDE STUFF                                                            */
/***********************************************************************************/

void Unit::SetCollisionParent (Unit * name) {
  assert (0); //deprecated... many less collisions with subunits out of the table
#if 0
    for (int i=0;i<numsubunit;i++) {
      subunits[i]->CollideInfo.object.u = name;
      subunits[i]->SetCollisionParent (name);
    }
#endif
}

/***********************************************************************************/
/**** UNIT_DOCK STUFF                                                            */
/***********************************************************************************/

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
  //  if (_Universe.GetRelation(faction,un->faction)>=0) {//already clearneed
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
  if (!(d->docked&DOCKED_INSIDE|DOCKED)) {
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

  for (i=0;i<utdw->image->dockedunits.size();i++) {
    if (utdw->image->dockedunits[i]->uc.GetUnit()==this) {
      utdw->FreeDockingPort (i);
      i--;
      invisible=false;
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

static int UpgradeFloat (double &result,double tobeupgraded, double upgrador, double templatelimit, double (*myadd) (double,double), bool (*betterthan) (double a, double b), double nothing,  double completeminimum, double (*computepercentage) (double oldvar, double upgrador, double newvar), double & percentage, bool forcedowngrade, bool usetemplate) {
  if (upgrador!=nothing) {//if upgrador is better than nothing
    float newsum = (*myadd)(tobeupgraded,upgrador);
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
      result=newsum;
      return UPGRADEOK;
    }else {
      return CAUSESDOWNGRADE;
    }
  } else {
    return NOTTHERE;
  }
}

void Unit::Mount::ReplaceMounts (const Unit::Mount *other) {
  short thisvol = volume;
  short thissize = size;
  Transformation t =this->GetMountLocation();
  *this=*other;
  this->size=thissize;
  volume=thisvol;
  this->SetMountPosition(t);
  ref.gun=NULL;
}

void Unit::Mount::SwapMounts (Unit::Mount * other) {
  short thisvol = volume;
  short othervol = other->volume;
  short othersize = other->size;
  short thissize = size;
  Mount mnt = *this;
  this->size=thissize;
  *this=*other;
  *other=mnt;
  volume=thisvol;

  other->volume=othervol;//volumes stay the same even if you swap out
  Transformation t =this->GetMountLocation();
  this->SetMountPosition(other->GetMountLocation());
  other->SetMountPosition (t);  
}

int UpgradeBoolval (int a, int upga, bool touchme, bool downgrade, int &numave,double &percentage) {
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

double Unit::Mount::Percentage (const Unit::Mount *newammo) const{
  float percentage=0;
  int thingstocompare=0;
  if (status==UNCHOSEN||status==DESTROYED)
    return 0;
  if (newammo->ammo==-1) {
    if (ammo!=-1) {
      thingstocompare++;
    }
  } else {
    if (newammo->ammo>0) {
      percentage+=ammo/newammo->ammo;
      thingstocompare++;
    }
  }
  if (newammo->type->Range) {
    percentage+= type->Range/newammo->type->Range;
    thingstocompare++;
  }
  if (newammo->type->Damage+100*newammo->type->PhaseDamage) {
    percentage += (type->Damage+100*type->PhaseDamage)/(newammo->type->Damage+100*newammo->type->PhaseDamage);
    thingstocompare++;
  }
  if (thingstocompare) {
    return percentage/thingstocompare;
  }else {
    return 0;
  }
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

std::string getTurretSize (const std::string &size) {
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

bool Unit::UpgradeMounts (const Unit *up, int mountoffset, bool touchme, bool downgrade, int &numave, Unit * templ, double &percentage) {
  int j;
  int i;
  bool cancompletefully=true;
  for (i=0,j=mountoffset;i<up->GetNumMounts()&&i<GetNumMounts()/*i should be GetNumMounts(), s'ok*/;i++,j++) {
    if (up->mounts[i]->status==Mount::ACTIVE||up->mounts[i]->status==Mount::INACTIVE) {//only mess with this if the upgrador has active mounts
      int jmod=j%GetNumMounts();//make sure since we're offsetting the starting we don't overrun the mounts
      if (!downgrade) {//if we wish to add guns instead of remove
	if (up->mounts[i]->type->weapon_name!="MOUNT_UPGRADE") {


	  if (up->mounts[i]->type->size==(up->mounts[i]->type->size&mounts[jmod]->size)) {//only look at this mount if it can fit in the rack
	    if (up->mounts[i]->type->weapon_name!=mounts[jmod]->type->weapon_name) {
	      numave++;//ok now we can compute percentage of used parts
	      if (templ) {
		if (templ->GetNumMounts()>jmod) {
		  int maxammo = templ->mounts[jmod]->ammo;
		  if ((up->mounts[i]->ammo>maxammo||up->mounts[i]->ammo==-1)&&maxammo!=-1) {
		    up->mounts[i]->ammo = maxammo;
		  }
		  if (templ->mounts[jmod]->volume!=-1) {
		    if (up->mounts[i]->ammo*up->mounts[i]->type->volume>templ->mounts[jmod]->volume) {
		      up->mounts[i]->ammo = (templ->mounts[jmod]->volume+1)/up->mounts[i]->type->volume;
		    }
		  }
		}
	      }
	      percentage+=mounts[jmod]->Percentage(up->mounts[i]);//compute here
	      if (touchme) {//if we wish to modify the mounts
		mounts[jmod]->ReplaceMounts (up->mounts[i]);//switch this mount with the upgrador mount
	      }
	    }else {
	      int tmpammo = mounts[jmod]->ammo;
	      if (mounts[jmod]->ammo!=-1&&up->mounts[i]->ammo!=-1) {
		tmpammo+=up->mounts[i]->ammo;
		if (templ) {
		  if (templ->GetNumMounts()>jmod) {
		    if (templ->mounts[jmod]->ammo!=-1) {
		      if (templ->mounts[jmod]->ammo>tmpammo) {
			tmpammo=templ->mounts[jmod]->ammo;
		      }
		    }
		    if (templ->mounts[jmod]->volume!=-1) {
		      if (templ->mounts[jmod]->volume>mounts[jmod]->type->volume*tmpammo) {
			tmpammo=(templ->mounts[jmod]->volume+1)/mounts[jmod]->type->volume;
		      }
		    }
		    
		  }
		} 
		if (tmpammo*mounts[jmod]->type->volume>mounts[jmod]->volume) {
		  tmpammo = (1+mounts[jmod]->volume)/mounts[jmod]->type->volume;
		}
		if (tmpammo>mounts[jmod]->ammo) {
		  cancompletefully=true;
		  if (touchme)
		    mounts[jmod]->ammo = tmpammo;
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
	      siz = templ->mounts[jmod]->size;
	    }
	  }
	  if (((siz&up->mounts[i]->size)|mounts[jmod]->size)!=mounts[jmod]->size) {
	    if (touchme) {
	      mounts[jmod]->size|=up->mounts[i]->size;
	    }
	    numave++;
	    percentage++;

	  }else {
	    cancompletefully=false;
	  }
	  //we need to |= the mount type
	}
      } else {
	if (up->mounts[i]->type->weapon_name!="MOUNT_UPGRADE") {
	  bool found=false;//we haven't found a matching gun to remove

		for (unsigned int k=0;k<(unsigned int)GetNumMounts();k++) {///go through all guns
	      int jkmod = (jmod+k)%GetNumMounts();//we want to start with bias
	      if (strcasecmp(mounts[jkmod]->type->weapon_name.c_str(),up->mounts[i]->type->weapon_name.c_str())==0) {///search for right mount to remove starting from j. this is the right name
		found=true;//we got one
		percentage+=mounts[jkmod]->Percentage(up->mounts[i]);///calculate scrap value (if damaged)
		if (touchme) //if we modify
		  mounts[jkmod]->status=Mount::UNCHOSEN;///deactivate weapon
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
	    if ((up->mounts[i]->size&mounts[jkmod]->size)==(up->mounts[i]->size)) {
	      if (touchme) {
		mounts[jkmod]->size&=(~up->mounts[i]->size);
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

bool Unit::UpgradeSubUnits (Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage)  {
  bool cancompletefully=true;
  int j;
  std::string turSize;
  un_iter ui;
  bool found=false;
  for (j=0,ui=getSubUnits();(*ui)!=NULL&&j<subunitoffset;++ui,j++) {
  }///set the turrets to the offset
  un_iter upturrets;
  Unit * giveAway;

  giveAway=*ui;
  if (giveAway==NULL) {
    return true;
  }
  bool hasAnyTurrets=false;
    turSize = getTurretSize (giveAway->name);
  for (upturrets=up->getSubUnits();((*upturrets)!=NULL)&&((*ui)!=NULL); ++ui,++upturrets) {//begin goign through other unit's turrets
    hasAnyTurrets = true;
    Unit *addtome;

    addtome=*upturrets;//set pointers

    

    bool foundthis=false;
    if (turSize == getTurretSize (addtome->name)&&addtome->rSize()) {//if the new turret has any size at all
      if (!downgrade||addtome->name==giveAway->name) {
	found=true;
	foundthis=true;
	numave++;//add it
	percentage+=(giveAway->rSize()/addtome->rSize());//add up percentage equal to ratio of sizes
      }
    }
    if (foundthis) {
      if (touchme) {//if we wish to modify,
	Transformation t(addtome->curr_physical_state);//switch their current positions
	addtome->curr_physical_state=giveAway->curr_physical_state;
	giveAway->curr_physical_state=t;
	t=addtome->prev_physical_state;
	addtome->prev_physical_state=giveAway->prev_physical_state;
	giveAway->prev_physical_state=t;//switch their previous positions
	giveAway->SetRecursiveOwner(up);//set the owners of the respective turrets
	
	upturrets.postinsert (giveAway);//add it to the second unit
	ui.remove();//remove the turret from the first unit
	
	if (!downgrade) {//if we are upgrading swap them
	  ui.preinsert(addtome);//add unit to your ship
	  upturrets.remove();//remove unit from being a turret on other ship
	  addtome->SetRecursiveOwner(this);//set recursive owner
	} else {
	  Unit * un;//make garbage unit
	  // TO REPLACE BACK WHEN WORK DONE ON FACTION !!
	  // ui.preinsert (un=UnitFactory::createUnit("blank",true,faction));//give a default do-nothing unit
	  ui.preinsert (un=new Unit(0));//give a default do-nothing unit
	  un->limits.yaw=0;
	  un->limits.pitch=0;
	  un->limits.roll=0;
	  un->limits.lateral = un->limits.retro = un->limits.forward = un->limits.afterburn=0.0;

	  un->name = turSize+"_blank";
	  if (un->image->unitwriter!=NULL) {
	    un->image->unitwriter->setName (un->name);
	  }
	  un->curr_physical_state=addtome->curr_physical_state;

	  un->prev_physical_state=addtome->prev_physical_state;
	  
	  un->SetRecursiveOwner(this);
	  upturrets.remove();//remove unit from being a turret on other ship
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

bool Unit::canUpgrade (Unit * upgrador, int mountoffset,  int subunitoffset, int additive, bool force,  double & percentage, Unit * templ){
  return UpAndDownGrade(upgrador,templ,mountoffset,subunitoffset,false,false,additive,force,percentage);
}
bool Unit::Upgrade (Unit * upgrador, int mountoffset,  int subunitoffset, int additive, bool force,  double & percentage, Unit * templ) {
  return UpAndDownGrade(upgrador,templ,mountoffset,subunitoffset,true,false,additive,true,percentage);
}
bool Unit::canDowngrade (Unit *downgradeor, int mountoffset, int subunitoffset, double & percentage){
  return UpAndDownGrade(downgradeor,NULL,mountoffset,subunitoffset,false,true,false,true,percentage);
}
bool Unit::Downgrade (Unit * downgradeor, int mountoffset, int subunitoffset,  double & percentage){
  return UpAndDownGrade(downgradeor,NULL,mountoffset,subunitoffset,true,true,false,true,percentage);
}

bool Unit::UpAndDownGrade (Unit * up, Unit * templ, int mountoffset, int subunitoffset, bool touchme, bool downgrade, int additive, bool forcetransaction, double &percentage) {
  percentage=0;
  int numave=0;
  bool cancompletefully=UpgradeMounts(up,mountoffset,touchme,downgrade,numave,templ,percentage);
  bool cancompletefully1=UpgradeSubUnits(up,subunitoffset,touchme,downgrade,numave,percentage);
  cancompletefully=cancompletefully&&cancompletefully1;
  adder Adder;
  comparer Comparer;
  percenter Percenter;


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
      up->computer.max_speed = XMLSupport::parse_float (speedStarHandler (XMLType (&up->computer.max_speed),up));
      up->computer.max_ab_speed = XMLSupport::parse_float (speedStarHandler (XMLType (&up->computer.max_ab_speed),up));
      up->computer.max_yaw = XMLSupport::parse_float (angleStarHandler (XMLType (&up->computer.max_yaw),up));
      up->computer.max_pitch = XMLSupport::parse_float (angleStarHandler (XMLType (&up->computer.max_pitch),up));
      up->computer.max_roll = XMLSupport::parse_float (angleStarHandler (XMLType (&up->computer.max_roll),up));

      up->limits.yaw = XMLSupport::parse_float (angleStarHandler (XMLType (&up->limits.yaw),up));
      up->limits.pitch = XMLSupport::parse_float (angleStarHandler (XMLType (&up->limits.pitch),up));
      up->limits.roll = XMLSupport::parse_float (angleStarHandler (XMLType (&up->limits.roll),up));

      up->limits.forward = XMLSupport::parse_float (accelStarHandler (XMLType (&up->limits.forward),up));
      up->limits.retro = XMLSupport::parse_float (accelStarHandler (XMLType (&up->limits.retro),up));
      up->limits.lateral = XMLSupport::parse_float (accelStarHandler (XMLType (&up->limits.lateral),up));
      up->limits.vertical = XMLSupport::parse_float (accelStarHandler (XMLType (&up->limits.vertical),up));
      up->limits.afterburn = XMLSupport::parse_float (accelStarHandler (XMLType (&up->limits.afterburn),up));
      
    }else {
      Adder=&GetsB;
      Percenter=&computePercent;
    }
    Comparer=AGreaterB;
  }
  double resultdoub;
  int retval;
  double temppercent;
#define STDUPGRADE(my,oth,temp,noth) retval=(UpgradeFloat(resultdoub,my,oth,(templ!=NULL)?temp:0,Adder,Comparer,noth,noth,Percenter, temppercent,forcetransaction,templ!=NULL)); if (retval==UPGRADEOK) {if (touchme){my=resultdoub;} percentage+=temppercent; numave++;} else {if (retval!=NOTTHERE) cancompletefully=false;}

  STDUPGRADE(armor.front,up->armor.front,templ->armor.front,0);
  STDUPGRADE(armor.back,up->armor.back,templ->armor.back,0);
  STDUPGRADE(armor.right,up->armor.right,templ->armor.right,0);
  STDUPGRADE(armor.left,up->armor.left,templ->armor.left,0);
  STDUPGRADE(shield.recharge,up->shield.recharge,templ->shield.recharge,0);
  STDUPGRADE(hull,up->hull,templ->hull,0);
  if (maxhull<hull) {
    if (hull!=0) 
      maxhull=hull;
  }
  STDUPGRADE(recharge,up->recharge,templ->recharge,0);
  STDUPGRADE(image->repair_droid,up->image->repair_droid,templ->image->repair_droid,0);
  STDUPGRADE(image->cargo_volume,up->image->cargo_volume,templ->image->cargo_volume,0);
  image->ecm = abs(image->ecm);
  STDUPGRADE(image->ecm,abs(up->image->ecm),abs(templ->image->ecm),0);
  STDUPGRADE(maxenergy,up->maxenergy,templ->maxenergy,0);
  //  STDUPGRADE(afterburnenergy,up->afterburnenergy,templ->afterburnenergy,0);
  STDUPGRADE(limits.yaw,up->limits.yaw,templ->limits.yaw,0);
  STDUPGRADE(limits.pitch,up->limits.pitch,templ->limits.pitch,0);
  STDUPGRADE(limits.roll,up->limits.roll,templ->limits.roll,0);
  STDUPGRADE(limits.lateral,up->limits.lateral,templ->limits.lateral,0);
  STDUPGRADE(limits.vertical,up->limits.vertical,templ->limits.vertical,0);
  STDUPGRADE(limits.forward,up->limits.forward,templ->limits.forward,0);
  STDUPGRADE(limits.retro,up->limits.retro,templ->limits.retro,0);
  STDUPGRADE(limits.afterburn,up->limits.afterburn,templ->limits.afterburn,0);
  STDUPGRADE(computer.radar.maxrange,up->computer.radar.maxrange,templ->computer.radar.maxrange,0);
  STDUPGRADE(computer.max_speed,up->computer.max_speed,templ->computer.max_speed,0);
  STDUPGRADE(computer.max_ab_speed,up->computer.max_ab_speed,templ->computer.max_ab_speed,0);
  STDUPGRADE(computer.max_yaw,up->computer.max_yaw,templ->computer.max_yaw,0);
  STDUPGRADE(computer.max_pitch,up->computer.max_pitch,templ->computer.max_pitch,0);
  STDUPGRADE(computer.max_roll,up->computer.max_roll,templ->computer.max_roll,0);
  STDUPGRADE(fuel,up->fuel,templ->fuel,0);

  for (unsigned int upgr=0;upgr<Cockpit::NUMGAUGES+1+MAXVDUS;upgr++) {
	STDUPGRADE(image->cockpit_damage[upgr],up->image->cockpit_damage[upgr],templ->image->cockpit_damage[upgr],1);
	if (image->cockpit_damage[upgr]>1) {
	  image->cockpit_damage[upgr]=1;//keep it real
	}
  }
  if (shield.number==up->shield.number) {
    switch (shield.number) {
    case 2:
      STDUPGRADE(shield.fb[2],up->shield.fb[2],templ->shield.fb[2],0);
      STDUPGRADE(shield.fb[3],up->shield.fb[3],templ->shield.fb[3],0);
      break;
    case 4:
      STDUPGRADE(shield.fbrl.frontmax,up->shield.fbrl.frontmax,templ->shield.fbrl.frontmax,0);
      STDUPGRADE(shield.fbrl.backmax,up->shield.fbrl.backmax,templ->shield.fbrl.backmax,0);
      STDUPGRADE(shield.fbrl.leftmax,up->shield.fbrl.leftmax,templ->shield.fbrl.leftmax,0);
      STDUPGRADE(shield.fbrl.rightmax,up->shield.fbrl.rightmax,templ->shield.fbrl.rightmax,0);
      break;
    case 6:
      STDUPGRADE(shield.fbrltb.fbmax,up->shield.fbrltb.fbmax,templ->shield.fbrltb.fbmax,0);
      STDUPGRADE(shield.fbrltb.rltbmax,up->shield.fbrltb.rltbmax,templ->shield.fbrltb.rltbmax,0);
      break;     
    }
  }else {
    if (up->FShieldData()>0||up->RShieldData()>0|| up->LShieldData()>0||up->BShieldData()>0) {
      cancompletefully=false;
    }
  }
  

  computer.radar.color=UpgradeBoolval(computer.radar.color,up->computer.radar.color,touchme,downgrade,numave,percentage);
  computer.itts=UpgradeBoolval(computer.itts,up->computer.radar.color,touchme,downgrade,numave,percentage);
  ///do the two reversed ones below
  
  double myleak=100-shield.leak;
  double upleak=100-up->shield.leak;
  double templeak=100-(templ!=NULL?templ->shield.leak:0);
  bool ccf = cancompletefully;
  STDUPGRADE(myleak,upleak,templeak,0);
  if (touchme&&myleak<=100&&myleak>=0)shield.leak=100-myleak;
  
  myleak = 1-computer.radar.maxcone;
  upleak=1-up->computer.radar.maxcone;
  templeak=1-(templ!=NULL?templ->computer.radar.maxcone:-1);
  STDUPGRADE(myleak,upleak,templeak,0);
  if (touchme)computer.radar.maxcone=1-myleak;
  static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));// DO NOT CHANGE see unit_customize.cpp
  if (up->computer.radar.lockcone!=lc) {
    myleak = 1-computer.radar.lockcone;
    upleak=1-up->computer.radar.lockcone;
    templeak=1-(templ!=NULL?templ->computer.radar.lockcone:-1);
    if (templeak == 1-lc) {
      templeak=2;
    }
    STDUPGRADE(myleak,upleak,templeak,0);
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
    STDUPGRADE(myleak,upleak,templeak,0);
    if (touchme)computer.radar.trackingcone=1-myleak;    
  }
  cancompletefully=ccf;
  //NO CLUE FOR BELOW
  if (downgrade) {
    //    STDUPGRADE(image->cargo_volume,up->image->cargo_volume,templ->image->cargo_volume,0);
    if (jump.drive>=-1&&up->jump.drive>=-1) {
      if (touchme) jump.drive=-2;
      numave++;
      percentage+=(jump.energy&&jump.delay)?(.25*(up->jump.energy/jump.energy+up->jump.delay/jump.delay)):1;
      percentage+=.5*((float)(100-jump.damage))/(101-up->jump.damage);
    }
    if (cloaking!=-1&&up->cloaking!=-1) {
      if (touchme) cloaking=-1;
      numave++;
      percentage++;
    }
  
    if (afterburnenergy<32767&&afterburnenergy<=up->afterburnenergy&&up->afterburnenergy!=0) {
      if (touchme) afterburnenergy=32767;
      numave++;
      percentage++;
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

    }
    
    if (afterburnenergy>up->afterburnenergy&&up->afterburnenergy>0) {
      numave++;
      afterburnenergy=up->afterburnenergy;
    }
    
    if (jump.drive==-2&&up->jump.drive>=-1) {
      if (touchme) {jump.drive = up->jump.drive;jump.energy=up->jump.energy;jump.delay=up->jump.delay; jump.damage=0;}
      numave++;
    }
  }
  if (numave)
    percentage=percentage/numave;
  if (touchme&&up->mass&&numave) {
    float multiplyer =((downgrade)?-1:1);
    mass +=multiplyer*percentage*up->mass;
    if (mass<(templ?templ->mass:.000000001))
      mass=(templ?templ->mass:.000000001);
    MomentOfInertia +=multiplyer*percentage*up->MomentOfInertia;
    if (MomentOfInertia<(templ?templ->MomentOfInertia:0.00000001)) {
      MomentOfInertia=(templ?templ->MomentOfInertia:0.00000001);
    }
  }
  return cancompletefully;
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

int Unit::RemoveCargo (unsigned int i, int quantity,bool eraseZero) {
  assert (i<image->cargo.size());
  if (quantity>image->cargo[i].quantity)
    quantity=image->cargo[i].quantity;
  mass-=quantity*image->cargo[i].mass;
  image->cargo[i].quantity-=quantity;
  if (image->cargo[i].quantity<=0&&eraseZero)
    image->cargo.erase (image->cargo.begin()+i);
  return quantity;
}


void Unit::AddCargo (const Cargo &carg, bool sort) {
  mass+=carg.quantity*carg.mass;
  image->cargo.push_back (carg);   
  if (sort)
    SortCargo();
}

bool Unit::CanAddCargo (const Cargo &carg)const {
  float total_volume=carg.quantity*carg.volume;
  unsigned int j;
  for (j=0;j<image->cargo.size();j++) {
    total_volume+=image->cargo[j].quantity*image->cargo[j].volume;
  }
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
  if (i<0||i>=image->cargo.size()||!buyer->CanAddCargo(image->cargo[i])||mass<image->cargo[i].mass)
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
  mass+=carg.quantity*carg.mass;
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

void Unit::ImportPartList (const std::string& category, float price, float pricedev,  float quantity, float quantdev) {
/*
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
*/
}

std::string Unit::massSerializer (const XMLType &input, void *mythis) {
  Unit * un = (Unit *)mythis;
  float mass = un->mass;
  for (unsigned int i=0;i<un->image->cargo.size();i++) {
    mass-=un->image->cargo[i].mass*un->image->cargo[i].quantity;
  }
  return XMLSupport::tostring((float)mass);
}
std::string Unit::shieldSerializer (const XMLType &input, void * mythis) {
  Unit * un=(Unit *)mythis;
  switch (un->shield.number) {
  case 2:
    return tostring(un->shield.fb[2])+string("\" back=\"")+tostring(un->shield.fb[3]);
  case 6:
    return tostring(un->shield.fbrltb.fbmax)+string("\" back=\"")+tostring(un->shield.fbrltb.fbmax)+string("\" left=\"")+tostring(un->shield.fbrltb.rltbmax)+string("\" right=\"")+tostring(un->shield.fbrltb.rltbmax)+string("\" top=\"")+tostring(un->shield.fbrltb.rltbmax)+string("\" bottom=\"")+tostring(un->shield.fbrltb.rltbmax);
  case 4:
  default:
    return tostring(un->shield.fbrl.frontmax)+string("\" back=\"")+tostring(un->shield.fbrl.backmax)+string("\" left=\"")+tostring(un->shield.fbrl.leftmax)+string("\" right=\"")+tostring(un->shield.fbrl.rightmax);
  }
  return string("");
}

std::string Unit::mountSerializer (const XMLType &input, void * mythis) {
  Unit * un=(Unit *)mythis;
  int i=input.w.hardint;
  if (un->GetNumMounts()>i) {
    string result(lookupMountSize(un->mounts[i]->size));
    if (un->mounts[i]->status==Mount::INACTIVE||un->mounts[i]->status==Mount::ACTIVE)
      result+=string("\" weapon=\"")+(un->mounts[i]->type->weapon_name);
    if (un->mounts[i]->ammo!=-1)
      result+=string("\" ammo=\"")+XMLSupport::tostring(un->mounts[i]->ammo);
    if (un->mounts[i]->volume!=-1) {
      result+=string("\" volume=\"")+XMLSupport::tostring(un->mounts[i]->volume);
    }
    Matrix m;
    un->mounts[i]->GetMountLocation().to_matrix(m);
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

void Unit::SortCargo() {
  Unit *un=this;
  std::sort (un->image->cargo.begin(),un->image->cargo.end());

  for (unsigned int i=0;i+1<un->image->cargo.size();i++) {
    if (un->image->cargo[i].content==un->image->cargo[i+1].content) {
      float tmpmass = un->image->cargo[i].quantity*un->image->cargo[i].mass+un->image->cargo[i+1].quantity*un->image->cargo[i+1].mass;
      float tmpvolume = un->image->cargo[i].quantity*un->image->cargo[i].volume+un->image->cargo[i+1].quantity*un->image->cargo[i+1].volume;
      un->image->cargo[i].quantity+=un->image->cargo[i+1].quantity;
      tmpmass/=un->image->cargo[i].quantity;
      tmpvolume/=un->image->cargo[i].quantity;
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
  if (ViewComputerData().max_ab_speed>.001)
    return ((OriginalCourse-(FinalCourse)).Magnitude()/ViewComputerData().max_ab_speed);
  else
    return (FinalCourse-OriginalCourse).Magnitude();
}

/***************************************************************************************/
/*** STAR SYSTEM JUMP STUFF                                                          ***/
/***************************************************************************************/

bool GameUnit::TransferUnitToSystem (StarSystem * Current) {
  if (activeStarSystem->RemoveUnit (this)) {
    this->RemoveFromSystem();  
    this->Target(NULL);
    Current->AddUnit (this);    

    activeStarSystem = Current;
    return true;
  }else {
    fprintf (stderr,"Fatal Error: cannot remove starship from critical system");
  }
  return false;
}

