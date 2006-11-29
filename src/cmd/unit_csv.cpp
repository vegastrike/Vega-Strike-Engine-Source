#include "unit_generic.h"
#include "csv.h"
#include "savegame.h"
#include "xml_serializer.h"
#include "gfx/sphere.h"
#include "unit_collide.h"
#include "collide/cs_compat.h"
#include "collide/rapcol.h"
#include "gfx/bsp.h"
#include "unit_factory.h"
#include "audiolib.h"
#include "unit_xml.h"
#include "gfx/quaternion.h"
#include "role_bitmask.h"
#include "unit_csv.h"
#include <algorithm>
#include "lin_time.h"
#include "unit_const_cache.h"
#define VS_PI 3.1415926535897931
CSVRow LookupUnitRow(const string &unitname, const string &faction) {
  string hashname=unitname+"__"+faction;
  unsigned int where; //gets munged
  for (vector<CSVTable*>::reverse_iterator i=unitTables.rbegin();i!=unitTables.rend();++i) {
    unsigned int where;
    if ((*i)->RowExists(hashname,where)) {
      return CSVRow((*i),where); 
    }else if ((*i)->RowExists(unitname,where)) {
      return CSVRow((*i),where);
    }
  } 
  return CSVRow();
}
using namespace std;
extern int GetModeFromName (const char * input_buffer);
extern void pushMesh( std::vector<Mesh*> &mesh, float &randomstartframe, float &randomstartseconds,  const char *filename, const float scale,int faction,class Flightgroup * fg, int startframe, double texturestarttime);
void addShieldMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);
void addRapidMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);
void addBSPMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);

static vector<string> parseSemicolon(string inp) {
  vector<string> ret;
  string::size_type where;
  string::size_type ofs=0;
  while((where=inp.find(';',ofs))!=string::npos) {
    ret.push_back(inp.substr(ofs,where-ofs));
    ofs = where+1;
  }
  ret.push_back(inp.substr(ofs));
  return ret;
}
static vector<vector<string> > parseBracketSemicolon(string inp) {
  vector<vector<string> > ret;  
  string::size_type where,when;
  string::size_type ofs=0;
  while ((where=inp.find('{',ofs))!=string::npos) {
    if ((when=inp.find('}',where+1))!=string::npos) {
      ret.push_back(parseSemicolon(inp.substr(where+1,when-where-1)));
      ofs = when+1;
    } else ofs=string::npos;
  }
  return ret;
}

static void UpgradeUnit (Unit * un, const std::string &upgrades) {
  string::size_type when;
  string::size_type ofs=0;
  while ((when=upgrades.find('{',ofs))!=string::npos) {
    string::size_type where = upgrades.find('}',when+1);
    string upgrade = upgrades.substr(when+1,((where==string::npos) ? string::npos : where-when-1));
    ofs = ((where==string::npos) ? string::npos : where+1);

    unsigned int mountoffset=0;
    unsigned int subunitoffset=0;
    string::size_type where1 = upgrade.find(';');
    string::size_type where2 = upgrade.rfind(';');
    if (where1!=string::npos) {
      mountoffset=XMLSupport::parse_int(upgrade.substr(where1+1,where2!=where1?where2:upgrade.length()));      
      if (where2!=where1&&where2!=string::npos) {
        subunitoffset=XMLSupport::parse_int(upgrade.substr(where2+1));
      }
    }
    upgrade = upgrade.substr(0,where1);
    if (upgrade.length()==0) 
      continue;
    const Unit * upgradee = UnitConstCache::getCachedConst (StringIntKey(upgrade,FactionUtil::GetUpgradeFaction()));
	if (!upgradee) {
		upgradee=UnitConstCache::setCachedConst(StringIntKey(upgrade,FactionUtil::GetUpgradeFaction()),
			UnitFactory::createUnit(upgrade.c_str(),
                                            true,
                                            FactionUtil::GetUpgradeFaction()));
	}
    double percent=1.0;
    un->Unit::Upgrade (upgradee,
                       mountoffset,
                       subunitoffset,
                       GetModeFromName (upgrade.c_str()),true,percent,NULL); 
  }  
}


void AddMeshes(std::vector<Mesh*>&xmeshes, float&randomstartframe, float&randomstartseconds, float unitscale, const std::string &meshes,int faction,Flightgroup *fg,vector<unsigned int> *counts){
  string::size_type where,when,wheresf,wherest,ofs=0;
  if (counts) counts->clear();
  {   
      int nelem=0;
      while ((ofs=meshes.find('{',ofs))!=string::npos) nelem++,ofs++;
      if (counts) counts->reserve(nelem);
      xmeshes.reserve(nelem);
      ofs=0;
  }
  while ((where=meshes.find('{',ofs))!=string::npos) {
    when=meshes.find('}',where+1);//matching closing brace
    string mesh = meshes.substr(where+1,((when==string::npos) ? string::npos : when-where-1));
    ofs = ((when==string::npos) ? string::npos : when+1);
    
    wheresf = mesh.find(';');
    string startf = "0";
    string startt="0";
    if (wheresf!=string::npos) {
      startf=mesh.substr(wheresf+1);
      mesh = mesh.substr(0,wheresf); 
      wherest = startf.find(';');
      if (wherest!=string::npos) {
        startt=startf.substr(wherest+1);
        startf=startf.substr(0,wherest);
      }
    }    
    int startframe = startf=="RANDOM"?-1:(startf=="ASYNC"?-1:atoi(startf.c_str()));
    float starttime = startt=="RANDOM"?-1.0f:atof(startt.c_str());
    unsigned int s=xmeshes.size();
    pushMesh(xmeshes,randomstartframe,randomstartseconds,mesh.c_str(),unitscale,faction,fg,startframe,starttime);
	if (counts) counts->push_back(xmeshes.size()-s);
  }
}
static pair<string::size_type,string::size_type> nextElementRange (const string &inp, string::size_type &start, string::size_type end) 
{
  string::size_type ostart = start;
  start = inp.find(';',start);
  if (start != string::npos && (start != end && (end == string::npos || start < end))) {
    ++start;
	return pair<string::size_type,string::size_type>(ostart,start-1);
  } else {
    start = end;
	return pair<string::size_type,string::size_type>(ostart,end);
  }
}

static string nextElementString (const string &inp, string::size_type &start, string::size_type end) 
{
  pair<string::size_type,string::size_type> rng = nextElementRange(inp,start,end);
  if (rng.second == string::npos)
	  return inp.substr(rng.first); else
	  return inp.substr(rng.first,rng.second-rng.first);
}

static int nextElementInt(const string &inp, string::size_type &start, string::size_type end, int def=0) 
{
  pair<string::size_type,string::size_type> rng = nextElementRange(inp,start,end);
  return (rng.first==rng.second)?def:atoi(inp.c_str()+rng.first);
}

static double nextElementFloat(const string &inp, string::size_type &start, string::size_type end, double def=0) 
{
  pair<string::size_type,string::size_type> rng = nextElementRange(inp,start,end);
  return (rng.first==rng.second)?def:atof(inp.c_str()+rng.first);
}

static double nextElementBool(const string &inp, string::size_type &start, string::size_type end, bool def=false) 
{
  pair<string::size_type,string::size_type> rng = nextElementRange(inp,start,end);
  return (rng.first==rng.second)?def:XMLSupport::parse_bool(inp.substr(rng.first,((rng.second==string::npos)?string::npos:(rng.second-rng.first))));
}

static string nextElement (string &inp) 
{
  string::size_type start=0;
  pair<string::size_type,string::size_type> rng = nextElementRange(inp,start,string::npos);
  string ret = inp.substr(rng.first,((rng.second==string::npos)?string::npos:(rng.second-rng.first)));
  inp.erase(0,((rng.second==string::npos)?string::npos:(rng.second+1)));
  return ret;
}


static bool stob(const string &inp, bool defaul) {
  if (inp.length()!=0) 
    return XMLSupport::parse_bool(inp);
  return defaul;
}
static double stof(const string &inp, double def=0) {
  if (inp.length()!=0)
    return XMLSupport::parse_float(inp);
  return def;
}
static int stoi(const string &inp, int def=0) {
  if (inp.length()!=0)
    return XMLSupport::parse_int(inp);
  return def;
}
extern bool CheckAccessory(Unit *);
extern int parseMountSizes (const char * str);
static Mount * createMount(const std::string& name, int ammo, int volume, float xyscale, float zscale, float func, float maxfunc, bool banked) //short fix
{
	return new Mount (name.c_str(), ammo,volume,xyscale, zscale,func,maxfunc,banked);

}

static void AddMounts(Unit * thus, Unit::XML &xml, const std::string &mounts) {
  string::size_type where,when,ofs=0;
  unsigned int first_new_mount = thus->mounts.size();
  {   
      int nmountz=0;
      while ((ofs=mounts.find('{',ofs))!=string::npos) nmountz++,ofs++;
	  thus->mounts.reserve(nmountz+thus->mounts.size());
      ofs=0;
  }
  while ((where=mounts.find('{',ofs))!=string::npos) {
    if ((when=mounts.find('}',where+1))!=string::npos) {
      string::size_type elemstart = where+1, elemend = when;
      ofs = when+1;

      QVector P;
      QVector Q = QVector (0,1,0);
      QVector R = QVector (0,0,1);
      QVector pos = QVector (0,0,0);
      
      string filename = nextElementString(mounts,elemstart,elemend);
      int ammo = nextElementInt(mounts,elemstart,elemend,-1);
      int volume = nextElementInt(mounts,elemstart,elemend);
      string mountsize=nextElementString(mounts,elemstart,elemend);
      pos.i = nextElementFloat(mounts,elemstart,elemend);
      pos.j = nextElementFloat(mounts,elemstart,elemend);
      pos.k = nextElementFloat(mounts,elemstart,elemend);
      double xyscale = nextElementFloat(mounts,elemstart,elemend);
      double zscale = nextElementFloat(mounts,elemstart,elemend);
      R.i = nextElementFloat(mounts,elemstart,elemend);
      R.j = nextElementFloat(mounts,elemstart,elemend);
      R.k = nextElementFloat(mounts,elemstart,elemend,1);
      Q.i = nextElementFloat(mounts,elemstart,elemend);
      Q.j = nextElementFloat(mounts,elemstart,elemend,1);
      Q.k = nextElementFloat(mounts,elemstart,elemend);
      float func = nextElementFloat(mounts,elemstart,elemend,1);
      float maxfunc = nextElementFloat(mounts,elemstart,elemend,1);
      bool banked = nextElementBool(mounts,elemstart,elemend,false);
      Q.Normalize();
      if (fabs(Q.i)==fabs(R.i)&&fabs(Q.j)==fabs(R.j)&&fabs(Q.k)==fabs(R.k)){
        Q.i=-1;
        Q.j=0;
        Q.k=0;
      }
      R.Normalize();
      
      CrossProduct (Q,R,P);
      CrossProduct (R,P,Q);
      Q.Normalize();
      //Transformation(Quaternion (from_vectors (P,Q,R),pos);
	  Mount mnt(filename,ammo,volume,xml.unitscale*xyscale,xml.unitscale*zscale,func,maxfunc,banked);
      mnt.SetMountOrientation(Quaternion::from_vectors(P.Cast(),Q.Cast(),R.Cast()));
      mnt.SetMountPosition(xml.unitscale*pos.Cast());
      int mntsiz=weapon_info::NOWEAP;
      if (mountsize.length()) {
        mntsiz=parseMountSizes(mountsize.c_str());
        mnt.size=mntsiz;
      } else {
        mnt.size = mnt.type->size;
      }
      thus->mounts.push_back(mnt);
    } else ofs=string::npos;
  }
  unsigned char parity=0;
  for (unsigned int a=first_new_mount;a<thus->mounts.size();++a) {
    static bool half_sounds = XMLSupport::parse_bool(vs_config->getVariable ("audio","every_other_mount","false"));
    if ((a&1)==parity) {
      int b=a;
      if((a&3) == 2 && (int) a < (thus->GetNumMounts()-1)) 
        if (thus->mounts[a].type->type != weapon_info::PROJECTILE&&thus->mounts[a+1].type->type != weapon_info::PROJECTILE)
          b=a+1;
      thus->mounts[b].sound = AUDCreateSound (thus->mounts[b].type->sound,thus->mounts[b].type->type!=weapon_info::PROJECTILE);
    } else if ((!half_sounds)||thus->mounts[a].type->type == weapon_info::PROJECTILE) {
      thus->mounts[a].sound = AUDCreateSound (thus->mounts[a].type->sound,thus->mounts[a].type->type!=weapon_info::PROJECTILE);    //lloping also flase in unit_customize  
    }
    if (a>0) {
      if (thus->mounts[a].sound==thus->mounts[a-1].sound&&thus->mounts[a].sound!=-1) {
        printf ("Sound error\n");
      }
    }
  }

}
struct SubUnitStruct{
  string filename;
  QVector pos;
  QVector Q;
  QVector R;
  double restricted;
  SubUnitStruct(string fn, QVector POS, QVector q, QVector r, double res) {
    filename=fn;
    pos=POS;
    Q=q;
    R=r;
    restricted=res;
  }
};
static vector<SubUnitStruct> GetSubUnits(const std::string &subunits) {
  string::size_type where,when,ofs=0;
  vector<SubUnitStruct> ret;
  {   
      int nelem=0;
      while ((ofs=subunits.find('{',ofs))!=string::npos) nelem++,ofs++;
      ret.reserve(nelem);
      ofs=0;
  }
  while ((where=subunits.find('{',ofs))!=string::npos) {
    if ((when=subunits.find('}',ofs))!=string::npos) {
      string::size_type elemstart = where+1, elemend = when;
      ofs = when+1;

      QVector pos,Q,R;
      
      string filename = nextElementString(subunits,elemstart,elemend);
      pos.i = nextElementFloat(subunits,elemstart,elemend);
      pos.j = nextElementFloat(subunits,elemstart,elemend);
      pos.k = nextElementFloat(subunits,elemstart,elemend);
      R.i = nextElementFloat(subunits,elemstart,elemend);
      R.j = nextElementFloat(subunits,elemstart,elemend);
      R.k = nextElementFloat(subunits,elemstart,elemend);
      Q.i = nextElementFloat(subunits,elemstart,elemend);
      Q.j = nextElementFloat(subunits,elemstart,elemend);
      Q.k = nextElementFloat(subunits,elemstart,elemend);
      double restricted=cos(nextElementFloat(subunits,elemstart,elemend,180)*VS_PI/180.0);

      ret.push_back(SubUnitStruct(filename,pos,Q,R,restricted));
    } else ofs = string::npos;
  }
  return ret;
}
static void AddSubUnits (Unit *thus, Unit::XML &xml, const std::string &subunits, int faction, const std::string &modification) {
  vector<SubUnitStruct> su=GetSubUnits(subunits);
  xml.units.reserve(subunits.size()+xml.units.size());
  for (vector<SubUnitStruct>::iterator i=su.begin();i!=su.end();++i) {
    string filename=(*i).filename;
    QVector pos=(*i).pos;
    QVector Q= (*i).Q;
    QVector R= (*i).R;
    double restricted=(*i).restricted;
    xml.units.push_back(UnitFactory::createUnit (filename.c_str(),true,faction,modification,NULL)); // I set here the fg arg to NULL
    if (xml.units.back()->name=="LOAD_FAILED") {
      xml.units.back()->limits.yaw=0;
      xml.units.back()->limits.pitch=0;
      xml.units.back()->limits.roll=0;
      xml.units.back()->limits.lateral = xml.units.back()->limits.retro = xml.units.back()->limits.forward = xml.units.back()->limits.afterburn=0.0;        
    }
    if (!thus->isSubUnit()) //Useless to set recursive owner in subunits - as parent will do the same
        xml.units.back()->SetRecursiveOwner (thus);
    xml.units.back()->SetOrientation (Q,R);
    R.Normalize();
    xml.units.back()->prev_physical_state = xml.units.back()->curr_physical_state;
    xml.units.back()->SetPosition(pos*xml.unitscale);
    //    xml.units.back()->prev_physical_state= Transformation(Quaternion::from_vectors(P,Q,R),pos);
    //    xml.units.back()->curr_physical_state=xml.units.back()->prev_physical_state;
    xml.units.back()->limits.structurelimits=R.Cast();
    xml.units.back()->limits.limitmin=restricted;
    xml.units.back()->name = filename;
    if (xml.units.back()->image->unitwriter!=NULL) {
      xml.units.back()->image->unitwriter->setName (filename);
    }
    CheckAccessory(xml.units.back());//turns on the ceerazy rotation for the turr          
  }
  for(int a=xml.units.size()-1; a>=0; a--) {
      bool randomspawn = xml.units[a]->name.get().find("randomspawn")!=string::npos;
      if (randomspawn) {
          int chancetospawn = xml.units[a]->WarpCapData();
          if (chancetospawn > rand()%100)
              thus->SubUnits.prepend(xml.units[a]); else
              xml.units[a]->Kill();
      } else thus->SubUnits.prepend(xml.units[a]);
  }
}

void AddDocks (Unit* thus, Unit::XML &xml, const string &docks) {
  string::size_type where,when;
  string::size_type ofs=0;
  {   
      int nelem=0;
      while ((ofs=docks.find('{',ofs))!=string::npos) nelem++,ofs++;
      thus->image->dockingports.reserve(nelem+thus->image->dockingports.size());
      ofs=0;
  }
  while ((where=docks.find('{',ofs))!=string::npos) {
    if ((when=docks.find('}',where+1))!=string::npos) {
      string::size_type elemstart = where+1, elemend = when;
      ofs = when+1;

      QVector pos=QVector(0,0,0);
      bool internal=nextElementBool(docks,elemstart,elemend);
      pos.i=nextElementFloat(docks,elemstart,elemend);
      pos.j=nextElementFloat(docks,elemstart,elemend);
      pos.k=nextElementFloat(docks,elemstart,elemend);
      double size=nextElementFloat(docks,elemstart,elemend);
      double minsize=nextElementFloat(docks,elemstart,elemend);

      thus->image->dockingports.push_back (DockingPorts(pos.Cast()*xml.unitscale,size*xml.unitscale,minsize*xml.unitscale,internal));
    } else ofs=string::npos;
  }
}
void AddLights (Unit * thus, Unit::XML &xml, const string &lights) 
{
  string::size_type where,when;
  string::size_type ofs=0;
  while ((where=lights.find('{',ofs))!=string::npos) {
    if ((when=lights.find('}',where+1))!=string::npos) {
      string::size_type elemstart = where+1, elemend = when;
      ofs = when+1;

      string filename = nextElementString(lights,elemstart,elemend);
      QVector pos,scale;
      GFXColor halocolor;
      pos.i=nextElementFloat(lights,elemstart,elemend);
      pos.j=nextElementFloat(lights,elemstart,elemend);
      pos.k=nextElementFloat(lights,elemstart,elemend);
      scale.i=xml.unitscale*nextElementFloat(lights,elemstart,elemend,1);
      scale.j=scale.k=scale.i;
      halocolor.r=nextElementFloat(lights,elemstart,elemend,1);
      halocolor.g=nextElementFloat(lights,elemstart,elemend,1);
      halocolor.b=nextElementFloat(lights,elemstart,elemend,1);
      halocolor.a=nextElementFloat(lights,elemstart,elemend,1);
      double act_speed=nextElementFloat(lights,elemstart,elemend);

      thus->addHalo(filename.c_str(),pos*xml.unitscale,scale.Cast(),halocolor,"",act_speed);
    } else ofs=string::npos;
  }
}

static void ImportCargo(Unit * thus, const string &imports) 
{
  string::size_type where,when,ofs=0;
  {   
      int nelem=0;
      while ((ofs=imports.find('{',ofs))!=string::npos) nelem++,ofs++;
      thus->image->cargo.reserve(nelem+thus->image->cargo.size());
      ofs=0;
  }
  while ((where=imports.find('{',ofs))!=string::npos) {
    if ((when=imports.find('}',where+1))!=string::npos) {
      string::size_type elemstart = where+1, elemend = when;
      ofs = when+1;

      string filename = nextElementString(imports,elemstart,elemend);
      double price = nextElementFloat(imports,elemstart,elemend,1);
      double pricestddev = nextElementFloat(imports,elemstart,elemend);
      double quant = nextElementFloat(imports,elemstart,elemend,1);
      double quantstddev = nextElementFloat(imports,elemstart,elemend);

      thus->ImportPartList(filename,price,pricestddev,quant,quantstddev);
    } else ofs=string::npos;
  }
}
static void AddCarg (Unit *thus, const string &cargos) 
{
  string::size_type where,when,ofs=0;
  {   
      int nelem=0;
      while ((ofs=cargos.find('{',ofs))!=string::npos) nelem++,ofs++;
      thus->image->cargo.reserve(nelem+thus->image->cargo.size());
      ofs=0;
  }
  while ((where=cargos.find('{',ofs))!=string::npos) {
    if ((when=cargos.find('}',where+1))!=string::npos) {
      Cargo carg;
      string::size_type elemstart = where+1, elemend = when;
      ofs = when+1;

      carg.content          = nextElementString(cargos,elemstart,elemend);      
      carg.category         = nextElementString(cargos,elemstart,elemend);
      carg.price            = nextElementFloat(cargos,elemstart,elemend);
      carg.quantity         = nextElementInt(cargos,elemstart,elemend);
      carg.mass             = nextElementFloat(cargos,elemstart,elemend);
      carg.volume           = nextElementFloat(cargos,elemstart,elemend);
      carg.functionality    = nextElementFloat(cargos,elemstart,elemend);
      carg.maxfunctionality = nextElementFloat(cargos,elemstart,elemend);
      carg.description      = nextElementString(cargos,elemstart,elemend);
      carg.mission          = nextElementBool(cargos,elemstart,elemend,false);

      thus->AddCargo(carg,false);
    } else ofs = string::npos;
  }
}
void HudDamage(float * dam, const string &damages) 
{
	if (dam) {
		string::size_type elemstart = 0, elemend = string::npos;
		for (int i=0;i<1+MAXVDUS+UnitImages::NUMGAUGES;++i)
			dam[i] = nextElementFloat(damages,elemstart,elemend,1);
	}
}
string WriteHudDamage (Unit * un) {
  string ret;
  const string semi=";";
  if (un->image->cockpit_damage) {
    for (int i=0;i<1+MAXVDUS+UnitImages::NUMGAUGES;++i) {
      ret+=XMLSupport::tostring(un->image->cockpit_damage[i]);
      ret+=semi;
    }
  }
  return ret;
}
string WriteHudDamageFunc (Unit * un) {
  string ret;
  const string semi=";";
  if (un->image->cockpit_damage) {
    int numg=1+MAXVDUS+UnitImages::NUMGAUGES;
    for (int i=numg;i<2*numg;++i) {
      ret+=XMLSupport::tostring(un->image->cockpit_damage[i]);
      ret+=semi;
    }
  }
  return ret;
}
void AddSounds(Unit * thus, string sounds) {
  if (sounds.length()!=0) {
    string tmp=nextElement(sounds);

    if (tmp.length())
      thus->sound->shield=AUDCreateSoundWAV(tmp,false);
    tmp=nextElement(sounds);
    if (tmp.length())
      thus->sound->armor=AUDCreateSoundWAV(tmp,false);
    tmp=nextElement(sounds);
    if (tmp.length())
      thus->sound->hull=AUDCreateSoundWAV(tmp,false);
    tmp=nextElement(sounds);
    if (tmp.length())
      thus->sound->jump=AUDCreateSoundWAV(tmp,false);
    tmp=nextElement(sounds);
    if (tmp.length())
      thus->sound->explode=AUDCreateSoundWAV(tmp,false);
    tmp=nextElement(sounds);
    if (tmp.length())
      thus->sound->cloak=AUDCreateSoundWAV(tmp,false);
    tmp=nextElement(sounds);
    if (tmp.length())
      thus->sound->engine=AUDCreateSoundWAV(tmp,true);
  }
    if (thus->sound->cloak==-1) {
        static std::string ssound=vs_config->getVariable ("unitaudio","cloak", "sfx43.wav");
        thus->sound->cloak=AUDCreateSound(ssound,false);
    }
    if (thus->sound->engine==-1) {
        static std::string ssound=vs_config->getVariable ("unitaudio","afterburner","sfx10.wav");
        thus->sound->engine=AUDCreateSound(ssound,false);
    }
    if (thus->sound->shield==-1) {
        static std::string ssound=vs_config->getVariable ("unitaudio","shield","sfx09.wav");
        thus->sound->shield=AUDCreateSound (ssound,false);
    }
    if (thus->sound->armor==-1) {
        static std::string ssound=vs_config->getVariable ("unitaudio","armor","sfx08.wav");
        thus->sound->armor=AUDCreateSound (ssound,false);
    }
    if (thus->sound->hull==-1) {
        static std::string ssound=vs_config->getVariable ("unitaudio","armor","sfx08.wav");
        thus->sound->hull=AUDCreateSound (ssound,false);
    }
    if (thus->sound->explode==-1) {
        static std::string ssound=vs_config->getVariable ("unitaudio","explode","explosion.wav");
        thus->sound->explode=AUDCreateSound (ssound,false);
    }
    if (thus->sound->jump==-1) {
        static std::string ssound=vs_config->getVariable ("unitaudio","explode","sfx43.wav");
        thus->sound->jump=AUDCreateSound (ssound,false);
    }
}
void LoadCockpit(Unit * thus, const string &cockpit) 
{
	string::size_type elemstart=0, elemend=string::npos;
	thus->image->cockpitImage   =nextElementString(cockpit,elemstart,elemend);
	thus->image->CockpitCenter.i=nextElementFloat(cockpit,elemstart,elemend);
	thus->image->CockpitCenter.j=nextElementFloat(cockpit,elemstart,elemend);
	thus->image->CockpitCenter.k=nextElementFloat(cockpit,elemstart,elemend);
}
static string str(string inp, string def) {
  if (inp.length()==0) return def;
  return inp;
}
static int AssignIf(const string &inp,float &val,float&val1, float&val2) {
  if (inp.length()) {
    val=stof(inp);
    val1=stof(inp);
    val2=stof(inp);
    return 1;
  }
  return 0;
}

static int AssignIfDeg(const string &inp,float &val) {
  if (inp.length()) {
    val=stof(inp)*VS_PI/180;
    return 1;
  }
  return 0;
}

float getFuelConversion(){
  static float fuel_conversion = XMLSupport::parse_float(vs_config->getVariable("physics","FuelConversion",".00144"));
  return fuel_conversion;
}

const std::string EMPTY_STRING("");
#define LOADROW_OPTIMIZER 0x348299ab
#define FORCE_OPTIMIZER 1 /*After all, it's always used in the end*/
#define OPTIMIZER_INDEX(Variable) OPTIDX_##Variable
#define INIT_OPTIMIZER(keys,Variable) OPTIMIZER_INDEX(Variable) = (keys.push_back(#Variable),(keys.size()-1))
#define DEF_OPTIMIZER(Variable) static unsigned int OPTIMIZER_INDEX(Variable) = CSVTable::optimizer_undefined;
#if FORCE_OPTIMIZER
#define OPTIM_GET(row,table,variable) ((table->optimizer_indexes[OPTIMIZER_INDEX(variable)]==CSVTable::optimizer_undefined)?EMPTY_STRING:row[table->optimizer_indexes[OPTIMIZER_INDEX(variable)]])
#else
#define OPTIM_GET(row,table,variable) (use_optimizer?(((OPTIMIZER_INDEX(variable)==CSVTable::optimizer_undefined) || (table->optimizer_indexes[OPTIMIZER_INDEX(variable)]==CSVTable::optimizer_undefined))?EMPTY_STRING:row[table->optimizer_indexes[OPTIMIZER_INDEX(variable)]]):row[#variable])
#endif

void Unit::LoadRow(CSVRow &row, string modification, string * netxml) {
  CSVTable *table = row.getParent();

  Unit::XML xml;
  xml.unitModifications=modification.c_str();
  xml.randomstartframe=((float)rand())/RAND_MAX;
  xml.randomstartseconds=0;
  xml.calculated_role=false;
  xml.damageiterator=0;
  xml.shieldmesh = NULL;
  xml.bspmesh = NULL;
  xml.rapidmesh = NULL;
  xml.hasBSP = true;
  xml.hasColTree=true;
  xml.unitlevel=0;
  xml.unitscale=1;
  xml.data=xml.shieldmesh=xml.bspmesh=xml.rapidmesh=NULL;//was uninitialized memory
  
  string tmpstr;
  csvRow = row[0];
  DEF_OPTIMIZER(FaceCamera);
  DEF_OPTIMIZER(Name);
  DEF_OPTIMIZER(Hud_image);
  DEF_OPTIMIZER(Combat_Role);//legacy only
  DEF_OPTIMIZER(Unit_Role);
  DEF_OPTIMIZER(Attack_Preference);
  DEF_OPTIMIZER(Num_Animation_Stages);
  DEF_OPTIMIZER(Unit_Scale);
  DEF_OPTIMIZER(Mesh);
  DEF_OPTIMIZER(Dock);
  DEF_OPTIMIZER(Sub_Units);
  DEF_OPTIMIZER(Mounts);
  DEF_OPTIMIZER(Hold_Volume);
  DEF_OPTIMIZER(Hidden_Hold_Volume);
  DEF_OPTIMIZER(Upgrade_Storage_Volume);
  DEF_OPTIMIZER(Equipment_Space);
  DEF_OPTIMIZER(Cargo_Import);
  DEF_OPTIMIZER(Cargo);
  DEF_OPTIMIZER(Sounds);
  DEF_OPTIMIZER(Cockpit);
  DEF_OPTIMIZER(CockpitX);
  DEF_OPTIMIZER(CockpitY);
  DEF_OPTIMIZER(CockpitZ);
  DEF_OPTIMIZER(Mass);
  DEF_OPTIMIZER(Moment_Of_Inertia);
  DEF_OPTIMIZER(Fuel_Capacity);
  DEF_OPTIMIZER(Hull);
  DEF_OPTIMIZER(Armor_Front_Top_Left);
  DEF_OPTIMIZER(Armor_Front_Top_Right);
  DEF_OPTIMIZER(Armor_Back_Top_Left);
  DEF_OPTIMIZER(Armor_Back_Top_Right);
  DEF_OPTIMIZER(Armor_Front_Bottom_Left);
  DEF_OPTIMIZER(Armor_Front_Bottom_Right);
  DEF_OPTIMIZER(Armor_Back_Bottom_Left);
  DEF_OPTIMIZER(Armor_Back_Bottom_Right);
  DEF_OPTIMIZER(Description);
  DEF_OPTIMIZER(Shield_Front_Top_Left);
  DEF_OPTIMIZER(Shield_Front_Top_Right);
  DEF_OPTIMIZER(Shield_Back_Top_Left);
  DEF_OPTIMIZER(Shield_Back_Top_Right);
  DEF_OPTIMIZER(Shield_Front_Bottom_Left);
  DEF_OPTIMIZER(Shield_Front_Bottom_Right);
  DEF_OPTIMIZER(Shield_Back_Bottom_Left);
  DEF_OPTIMIZER(Shield_Back_Bottom_Right);
  DEF_OPTIMIZER(Shield_Leak);
  DEF_OPTIMIZER(Shield_Recharge);
  DEF_OPTIMIZER(Shield_Efficiency);
  DEF_OPTIMIZER(Warp_Capacitor);
  DEF_OPTIMIZER(Warp_Min_Multiplier);
  DEF_OPTIMIZER(Warp_Max_Multiplier);
  DEF_OPTIMIZER(Primary_Capacitor);
  DEF_OPTIMIZER(Reactor_Recharge);
  DEF_OPTIMIZER(Jump_Drive_Present);
  DEF_OPTIMIZER(Jump_Drive_Delay);
  DEF_OPTIMIZER(Wormhole);
  DEF_OPTIMIZER(Collide_Subunits);
  DEF_OPTIMIZER(Outsystem_Jump_Cost);
  DEF_OPTIMIZER(Warp_Usage_Cost);
  DEF_OPTIMIZER(Afterburner_Usage_Cost);
  DEF_OPTIMIZER(Afterburner_Type);
  DEF_OPTIMIZER(Maneuver_Yaw);
  DEF_OPTIMIZER(Maneuver_Pitch);
  DEF_OPTIMIZER(Maneuver_Roll);
  DEF_OPTIMIZER(Yaw_Governor);
  DEF_OPTIMIZER(Yaw_Governor_Right);
  DEF_OPTIMIZER(Yaw_Governor_Left);
  DEF_OPTIMIZER(Pitch_Governor);
  DEF_OPTIMIZER(Pitch_Governor_Up);
  DEF_OPTIMIZER(Pitch_Governor_Down);
  DEF_OPTIMIZER(Roll_Governor);
  DEF_OPTIMIZER(Roll_Governor_Right);
  DEF_OPTIMIZER(Roll_Governor_Left);
  DEF_OPTIMIZER(Afterburner_Accel);
  DEF_OPTIMIZER(Forward_Accel);
  DEF_OPTIMIZER(Retro_Accel);
  DEF_OPTIMIZER(Left_Accel);
  DEF_OPTIMIZER(Right_Accel);
  DEF_OPTIMIZER(Top_Accel);
  DEF_OPTIMIZER(Bottom_Accel);
  DEF_OPTIMIZER(Default_Speed_Governor);
  DEF_OPTIMIZER(Afterburner_Speed_Governor);
  DEF_OPTIMIZER(ITTS);
  DEF_OPTIMIZER(Can_Lock);
  DEF_OPTIMIZER(Radar_Color);
  DEF_OPTIMIZER(Radar_Range);
  DEF_OPTIMIZER(Max_Cone);
  DEF_OPTIMIZER(Tracking_Cone);
  DEF_OPTIMIZER(Lock_Cone);
  DEF_OPTIMIZER(Cloak_Min);
  DEF_OPTIMIZER(Cloak_Glass);
  DEF_OPTIMIZER(Can_Cloak);
  DEF_OPTIMIZER(Cloak_Rate);
  DEF_OPTIMIZER(Cloak_Energy);
  DEF_OPTIMIZER(Repair_Droid);
  DEF_OPTIMIZER(ECM_Rating);
  DEF_OPTIMIZER(Heat_Sink_Rating);
  DEF_OPTIMIZER(Hud_Functionality);
  DEF_OPTIMIZER(Max_Hud_Functionality);
  DEF_OPTIMIZER(Lifesupport_Functionality);
  DEF_OPTIMIZER(Max_Lifesupport_Functionality);
  DEF_OPTIMIZER(Comm_Functionality);
  DEF_OPTIMIZER(Max_Comm_Functionality);
  DEF_OPTIMIZER(FireControl_Functionality);
  DEF_OPTIMIZER(Max_FireControl_Functionality);
  DEF_OPTIMIZER(SPECDrive_Functionality);
  DEF_OPTIMIZER(Max_SPECDrive_Functionality);
  DEF_OPTIMIZER(Slide_Start);
  DEF_OPTIMIZER(Slide_End);
  DEF_OPTIMIZER(Upgrades);
  DEF_OPTIMIZER(Tractorability);
  DEF_OPTIMIZER(Explosion);
  DEF_OPTIMIZER(Light);
  DEF_OPTIMIZER(Shield_Mesh);
  DEF_OPTIMIZER(BSP_Mesh);
  DEF_OPTIMIZER(Rapid_Mesh);
  DEF_OPTIMIZER(Use_BSP);
  DEF_OPTIMIZER(Use_Rapid);
  DEF_OPTIMIZER(NoDamageParticles);
  DEF_OPTIMIZER(Spec_Interdiction);

  if (table&&!table->optimizer_setup) {
      static std::vector<std::string> keys;
      static bool optimizer_keys_init=false;
      if (!optimizer_keys_init) {
          optimizer_keys_init=true;
          printf("Initializing optimizer\n");
          INIT_OPTIMIZER(keys,Name);
          INIT_OPTIMIZER(keys,Hud_image);
          INIT_OPTIMIZER(keys,FaceCamera);
          INIT_OPTIMIZER(keys,Combat_Role);//legacy only
          INIT_OPTIMIZER(keys,Unit_Role);
          INIT_OPTIMIZER(keys,Attack_Preference);
          INIT_OPTIMIZER(keys,Num_Animation_Stages);
          INIT_OPTIMIZER(keys,Unit_Scale);
          INIT_OPTIMIZER(keys,Mesh);
          INIT_OPTIMIZER(keys,Dock);
          INIT_OPTIMIZER(keys,Sub_Units);
          INIT_OPTIMIZER(keys,Mounts);
          INIT_OPTIMIZER(keys,Hold_Volume);
          INIT_OPTIMIZER(keys,Hidden_Hold_Volume);
          INIT_OPTIMIZER(keys,Upgrade_Storage_Volume);
          INIT_OPTIMIZER(keys,Equipment_Space);
          INIT_OPTIMIZER(keys,Cargo_Import);
          INIT_OPTIMIZER(keys,Cargo);
          INIT_OPTIMIZER(keys,Sounds);
          INIT_OPTIMIZER(keys,Cockpit);
          INIT_OPTIMIZER(keys,CockpitX);
          INIT_OPTIMIZER(keys,CockpitY);
          INIT_OPTIMIZER(keys,CockpitZ);
          INIT_OPTIMIZER(keys,Mass);
          INIT_OPTIMIZER(keys,Moment_Of_Inertia);
          INIT_OPTIMIZER(keys,Fuel_Capacity);
          INIT_OPTIMIZER(keys,Hull);
          INIT_OPTIMIZER(keys,Armor_Front_Top_Left);
          INIT_OPTIMIZER(keys,Armor_Front_Top_Right);
          INIT_OPTIMIZER(keys,Armor_Back_Top_Left);
          INIT_OPTIMIZER(keys,Armor_Back_Top_Right);
          INIT_OPTIMIZER(keys,Armor_Front_Bottom_Left);
          INIT_OPTIMIZER(keys,Armor_Front_Bottom_Right);
          INIT_OPTIMIZER(keys,Armor_Back_Bottom_Left);
          INIT_OPTIMIZER(keys,Armor_Back_Bottom_Right);
          INIT_OPTIMIZER(keys,Description);
          INIT_OPTIMIZER(keys,Shield_Front_Top_Left);
          INIT_OPTIMIZER(keys,Shield_Front_Top_Right);
          INIT_OPTIMIZER(keys,Shield_Back_Top_Left);
          INIT_OPTIMIZER(keys,Shield_Back_Top_Right);
          INIT_OPTIMIZER(keys,Shield_Front_Bottom_Left);
          INIT_OPTIMIZER(keys,Shield_Front_Bottom_Right);
          INIT_OPTIMIZER(keys,Shield_Back_Bottom_Left);
          INIT_OPTIMIZER(keys,Shield_Back_Bottom_Right);
          INIT_OPTIMIZER(keys,Shield_Leak);
          INIT_OPTIMIZER(keys,Shield_Recharge);
          INIT_OPTIMIZER(keys,Shield_Efficiency);
          INIT_OPTIMIZER(keys,Warp_Capacitor);
          INIT_OPTIMIZER(keys,Warp_Min_Multiplier);
          INIT_OPTIMIZER(keys,Warp_Max_Multiplier);
          INIT_OPTIMIZER(keys,Primary_Capacitor);
          INIT_OPTIMIZER(keys,Reactor_Recharge);
          INIT_OPTIMIZER(keys,Jump_Drive_Present);
          INIT_OPTIMIZER(keys,Jump_Drive_Delay);
          INIT_OPTIMIZER(keys,Wormhole);
          INIT_OPTIMIZER(keys,Collide_Subunits);
          INIT_OPTIMIZER(keys,Outsystem_Jump_Cost);
          INIT_OPTIMIZER(keys,Warp_Usage_Cost);
          INIT_OPTIMIZER(keys,Afterburner_Usage_Cost);
          INIT_OPTIMIZER(keys,Afterburner_Type);
          INIT_OPTIMIZER(keys,Maneuver_Yaw);
          INIT_OPTIMIZER(keys,Maneuver_Pitch);
          INIT_OPTIMIZER(keys,Maneuver_Roll);
          INIT_OPTIMIZER(keys,Yaw_Governor);
          INIT_OPTIMIZER(keys,Yaw_Governor_Right);
          INIT_OPTIMIZER(keys,Yaw_Governor_Left);
          INIT_OPTIMIZER(keys,Pitch_Governor);
          INIT_OPTIMIZER(keys,Pitch_Governor_Up);
          INIT_OPTIMIZER(keys,Pitch_Governor_Down);
          INIT_OPTIMIZER(keys,Roll_Governor);
          INIT_OPTIMIZER(keys,Roll_Governor_Right);
          INIT_OPTIMIZER(keys,Roll_Governor_Left);
          INIT_OPTIMIZER(keys,Afterburner_Accel);
          INIT_OPTIMIZER(keys,Forward_Accel);
          INIT_OPTIMIZER(keys,Retro_Accel);
          INIT_OPTIMIZER(keys,Left_Accel);
          INIT_OPTIMIZER(keys,Right_Accel);
          INIT_OPTIMIZER(keys,Top_Accel);
          INIT_OPTIMIZER(keys,Bottom_Accel);
          INIT_OPTIMIZER(keys,Default_Speed_Governor);
          INIT_OPTIMIZER(keys,Afterburner_Speed_Governor);
          INIT_OPTIMIZER(keys,ITTS);
          INIT_OPTIMIZER(keys,Can_Lock);
          INIT_OPTIMIZER(keys,Radar_Color);
          INIT_OPTIMIZER(keys,Radar_Range);
          INIT_OPTIMIZER(keys,Max_Cone);
          INIT_OPTIMIZER(keys,Tracking_Cone);
          INIT_OPTIMIZER(keys,Lock_Cone);
          INIT_OPTIMIZER(keys,Cloak_Min);
          INIT_OPTIMIZER(keys,Cloak_Glass);
          INIT_OPTIMIZER(keys,Can_Cloak);
          INIT_OPTIMIZER(keys,Cloak_Rate);
          INIT_OPTIMIZER(keys,Cloak_Energy);
          INIT_OPTIMIZER(keys,Repair_Droid);
          INIT_OPTIMIZER(keys,ECM_Rating);
          INIT_OPTIMIZER(keys,Heat_Sink_Rating);
          INIT_OPTIMIZER(keys,Hud_Functionality);
          INIT_OPTIMIZER(keys,Max_Hud_Functionality);
          INIT_OPTIMIZER(keys,Lifesupport_Functionality);
          INIT_OPTIMIZER(keys,Max_Lifesupport_Functionality);
          INIT_OPTIMIZER(keys,Comm_Functionality);
          INIT_OPTIMIZER(keys,Max_Comm_Functionality);
          INIT_OPTIMIZER(keys,FireControl_Functionality);
          INIT_OPTIMIZER(keys,Max_FireControl_Functionality);
          INIT_OPTIMIZER(keys,SPECDrive_Functionality);
          INIT_OPTIMIZER(keys,Max_SPECDrive_Functionality);
          INIT_OPTIMIZER(keys,Slide_Start);
          INIT_OPTIMIZER(keys,Slide_End);
          INIT_OPTIMIZER(keys,Upgrades);
          INIT_OPTIMIZER(keys,Tractorability);
          INIT_OPTIMIZER(keys,Explosion);
          INIT_OPTIMIZER(keys,Light);
          INIT_OPTIMIZER(keys,Shield_Mesh);
          INIT_OPTIMIZER(keys,BSP_Mesh);
          INIT_OPTIMIZER(keys,Rapid_Mesh);
          INIT_OPTIMIZER(keys,Use_BSP);
          INIT_OPTIMIZER(keys,Use_Rapid);
          INIT_OPTIMIZER(keys,NoDamageParticles);
          INIT_OPTIMIZER(keys,Spec_Interdiction);
      }
      table->SetupOptimizer(keys,LOADROW_OPTIMIZER);
  }
  bool use_optimizer=(table&&table->optimizer_setup&&table->optimizer_type==LOADROW_OPTIMIZER);

  //begin the geometry (and things that depend on stats)
  double start=queryTime();
  fullname=OPTIM_GET(row,table,Name);
  //image->description=OPTIM_GET(row,table,Description);
  if ((tmpstr=OPTIM_GET(row,table,Hud_image)).length()!=0) {
    std::string fac =FactionUtil::GetFaction(faction);
    fac+="_";
    fac+=tmpstr;
    image->hudImage = createVSSprite(fac.c_str());
    if (!isVSSpriteLoaded(image->hudImage)) {
      deleteVSSprite(image->hudImage);
      image->hudImage = createVSSprite(tmpstr.c_str());
    }
  }  
  if ((tmpstr=OPTIM_GET(row,table,FaceCamera)).length()!=0) {
    graphicOptions.FaceCamera=XMLSupport::parse_bool(tmpstr)?1:0;
  }
  double spritet=queryTime();
  std::string llegacy_combat_role(OPTIM_GET(row,table,Combat_Role));
  std::string lunit_role(OPTIM_GET(row,table,Unit_Role));
  std::string lattack_preference(OPTIM_GET(row,table,Attack_Preference));
  if (lunit_role.length()==0){
      this->setUnitRole(llegacy_combat_role);
  }else {
      this->setUnitRole(lunit_role);
  }
  if (lattack_preference.length()==0){
      this->setAttackPreference(llegacy_combat_role);
  }else {
      this->setAttackPreference(lattack_preference);
  }
  graphicOptions.NumAnimationPoints=stoi(OPTIM_GET(row,table,Num_Animation_Stages),0);
  graphicOptions.NoDamageParticles=stoi(OPTIM_GET(row,table,NoDamageParticles),0);
  if (graphicOptions.NumAnimationPoints>0)
    graphicOptions.Animating=0;
  xml.unitscale = stof(OPTIM_GET(row,table,Unit_Scale),1);
  if (!xml.unitscale) xml.unitscale=1;
  image->unitscale=xml.unitscale;
  double meshest=queryTime();
  AddMeshes(xml.meshes,xml.randomstartframe,xml.randomstartseconds,xml.unitscale,OPTIM_GET(row,table,Mesh),faction,getFlightgroup());
  double subunt=queryTime();
  AddDocks(this,xml,OPTIM_GET(row,table,Dock));
  AddSubUnits(this,xml,OPTIM_GET(row,table,Sub_Units),faction,modification);

  meshdata= xml.meshes;
  meshdata.push_back(NULL);
  corner_min = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
  corner_max = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  calculate_extent(false);
  double mountst=queryTime();
  AddMounts(this,xml,OPTIM_GET(row,table,Mounts));
  double cargot=queryTime();
  this->image->CargoVolume=stof(OPTIM_GET(row,table,Hold_Volume));
  this->image->HiddenCargoVolume=stof(OPTIM_GET(row,table,Hidden_Hold_Volume));
  this->image->UpgradeVolume=stof(OPTIM_GET(row,table,Upgrade_Storage_Volume));
  this->image->equipment_volume=stof(OPTIM_GET(row,table,Equipment_Space));
  ImportCargo(this,OPTIM_GET(row,table,Cargo_Import));//if this changes change planet_generic.cpp
  AddCarg(this,OPTIM_GET(row,table,Cargo));
  double soundst=queryTime();
  AddSounds(this,OPTIM_GET(row,table,Sounds));
  LoadCockpit(this,OPTIM_GET(row,table,Cockpit));
  image->CockpitCenter.i=stof(OPTIM_GET(row,table,CockpitX))*xml.unitscale;
  image->CockpitCenter.j=stof(OPTIM_GET(row,table,CockpitY))*xml.unitscale;
  image->CockpitCenter.k=stof(OPTIM_GET(row,table,CockpitZ))*xml.unitscale;
  Mass=stof(OPTIM_GET(row,table,Mass),1.0);
  Momentofinertia=stof(OPTIM_GET(row,table,Moment_Of_Inertia),1.0);
  fuel=stof(OPTIM_GET(row,table,Fuel_Capacity));
  hull=maxhull = stof(OPTIM_GET(row,table,Hull));
  specInterdiction=stof(OPTIM_GET(row,table,Spec_Interdiction));
  armor.frontlefttop=stof(OPTIM_GET(row,table,Armor_Front_Top_Left));
  armor.frontrighttop=stof(OPTIM_GET(row,table,Armor_Front_Top_Right));
  armor.backlefttop=stof(OPTIM_GET(row,table,Armor_Back_Top_Left));
  armor.backrighttop=stof(OPTIM_GET(row,table,Armor_Back_Top_Right));
  armor.frontleftbottom=stof(OPTIM_GET(row,table,Armor_Front_Bottom_Left));
  armor.frontrightbottom=stof(OPTIM_GET(row,table,Armor_Front_Bottom_Right));
  armor.backleftbottom=stof(OPTIM_GET(row,table,Armor_Back_Bottom_Left));
  armor.backrightbottom=stof(OPTIM_GET(row,table,Armor_Back_Bottom_Right));

  int shieldcount=0;
  Shield two;
  Shield four;
  Shield eight;
  memset(&two,0,sizeof(Shield));
  memset(&four,0,sizeof(Shield));
  memset(&eight,0,sizeof(Shield));
  shieldcount+=AssignIf(OPTIM_GET(row,table,Shield_Front_Top_Right),
                        two.shield2fb.front,four.shield4fbrl.front,eight.shield8.frontrighttop);
  shieldcount+=AssignIf(OPTIM_GET(row,table,Shield_Front_Top_Left),
                        two.shield2fb.front,four.shield4fbrl.front,eight.shield8.frontlefttop);
  shieldcount+=AssignIf(OPTIM_GET(row,table,Shield_Back_Top_Left),
                        two.shield2fb.back,four.shield4fbrl.back,eight.shield8.backlefttop);
  shieldcount+=AssignIf(OPTIM_GET(row,table,Shield_Back_Top_Right),
                        two.shield2fb.back,four.shield4fbrl.back,eight.shield8.backrighttop);

  shieldcount+=AssignIf(OPTIM_GET(row,table,Shield_Front_Bottom_Left),
                        two.shield2fb.front,four.shield4fbrl.left,eight.shield8.frontleftbottom);
  shieldcount+=AssignIf(OPTIM_GET(row,table,Shield_Front_Bottom_Right),
                        two.shield2fb.front,four.shield4fbrl.right,eight.shield8.frontrightbottom);
  shieldcount+=AssignIf(OPTIM_GET(row,table,Shield_Back_Bottom_Left),
                        two.shield2fb.back,four.shield4fbrl.left,eight.shield8.backleftbottom);
  shieldcount+=AssignIf(OPTIM_GET(row,table,Shield_Back_Bottom_Right),
                        two.shield2fb.back,four.shield4fbrl.right,eight.shield8.backrightbottom);

  two.shield2fb.frontmax=two.shield2fb.front;
  two.shield2fb.backmax=two.shield2fb.back;
  four.shield4fbrl.frontmax=four.shield4fbrl.front;
  four.shield4fbrl.backmax=four.shield4fbrl.back;
  four.shield4fbrl.rightmax=four.shield4fbrl.right;
  four.shield4fbrl.leftmax=four.shield4fbrl.left;

  eight.shield8.frontlefttopmax=eight.shield8.frontlefttop;
  eight.shield8.frontrighttopmax=eight.shield8.frontrighttop;
  eight.shield8.backrighttopmax=eight.shield8.backrighttop;
  eight.shield8.backlefttopmax=eight.shield8.backlefttop;
  eight.shield8.frontleftbottommax=eight.shield8.frontleftbottom;
  eight.shield8.frontrightbottommax=eight.shield8.frontrightbottom;
  eight.shield8.backrightbottommax=eight.shield8.backrightbottom;
  eight.shield8.backleftbottommax=eight.shield8.backleftbottom;
  float r45=VS_PI/4;
  float r90=VS_PI/2;
  float r135=3*VS_PI/4;
  float r180=VS_PI;
  float r225=5*VS_PI/4;
  float r270=3*VS_PI/2;
  float r315=7*VS_PI/4;
  float r360=2*VS_PI;
  int iter;
  if (shieldcount>MAX_SHIELD_NUMBER) {
    shieldcount=MAX_SHIELD_NUMBER;
  }
  memset(shield.range,0,sizeof(shield.range));
  if (shieldcount==8) {

    shield.number=8;
    shield.shield.cur[0]=shield.shield.max[0]=eight.shield8.frontlefttopmax;
    shield.range[0].thetamin=0;
    shield.range[0].thetamax=r90;
shield.range[0].    rhomin=0;
shield.range[0].    rhomax=r90;

    shield.shield.cur[1]=shield.shield.max[1]=eight.shield8.backlefttopmax;
shield.range[1].    thetamin=r90;
shield.range[1].    thetamax=r180;
shield.range[1].    rhomin=0;
shield.range[1].    rhomax=r90;

    shield.shield.cur[2]=shield.shield.max[2]=eight.shield8.frontrighttopmax;
    shield.range[2].thetamin=r270;
shield.range[2].    thetamax=r360;
shield.range[2].    rhomin=0;
shield.range[2].    rhomax=r90;

    shield.shield.cur[3]=shield.shield.max[3]=eight.shield8.backrighttopmax;
shield.range[3].    thetamin=r180;
shield.range[3].    thetamax=r270;
shield.range[3].    rhomin=0;
shield.range[3].    rhomax=r90;
    
    shield.shield.cur[4]=shield.shield.max[4]=eight.shield8.frontleftbottommax;
shield.range[4].    thetamin=0;
shield.range[4].    thetamax=r90;
shield.range[4].    rhomin=-r90;
shield.range[4].    rhomax=0;

    shield.shield.cur[5]=shield.shield.max[5]=eight.shield8.backleftbottommax;
shield.range[5].    thetamin=r90;
shield.range[5].    thetamax=r180;
shield.range[5].    rhomin=-r90;
shield.range[5].    rhomax=0;

    shield.shield.cur[6]=shield.shield.max[6]=eight.shield8.frontrightbottommax;
shield.range[6].   thetamin=r270;
shield.range[6].   thetamax=r360;
shield.range[6].   rhomin=-r90;
shield.range[6].   rhomax=0;

    shield.shield.cur[7]=shield.shield.max[7]=eight.shield8.backrightbottommax;
shield.range[7].   thetamin=r180;
shield.range[7].   thetamax=r270;
shield.range[7].   rhomin=-r90;
shield.range[7].   rhomax=0;
    
  }else if (shieldcount==4){
    shield.number=4;

    shield.shield.cur[0]=shield.shield.max[0]=four.shield4fbrl.frontmax;
shield.range[0].   thetamin=r315;
shield.range[0].   thetamax=r360+r45;
shield.range[0].   rhomin=-r90;
shield.range[0].   rhomax=r90;

    shield.shield.cur[1]=shield.shield.max[1]=four.shield4fbrl.backmax;
shield.range[1].   thetamin=r135;
shield.range[1].   thetamax=r225;
shield.range[1].   rhomin=-r90;
shield.range[1].   rhomax=r90;

    shield.shield.cur[2]=shield.shield.max[2]=four.shield4fbrl.rightmax;
shield.range[2].   thetamin=r225;
shield.range[2].   thetamax=r315;
shield.range[2].   rhomin=-r90;
shield.range[2].   rhomax=r90;

    shield.shield.cur[3]=shield.shield.max[3]=four.shield4fbrl.leftmax;
shield.range[3].   thetamin=r45;
shield.range[3].   thetamax=r225;
shield.range[3].   rhomin=-r90;
shield.range[3].   rhomax=r90;

  }else if (shieldcount==2) {
    shield.number=2;

    shield.shield.cur[0]=shield.shield.max[0]=four.shield2fb.frontmax;
shield.range[0].   thetamin=r270;
shield.range[0].   thetamax=r360+r90;
shield.range[0].   rhomin=-r90;
shield.range[0].   rhomax=r90;

    shield.shield.cur[1]=shield.shield.max[1]=four.shield2fb.backmax;
shield.range[1].   thetamin=r90;
shield.range[1].   thetamax=r270;
shield.range[1].   rhomin=-r90;
shield.range[1].   rhomax=r90;

  } else {
    //No shields
    shield.number=0;
  }
  for (iter =0;iter<shieldcount;++iter) {
    std::string shieldname= "Shield_"+XMLSupport::tostring(iter);    
    AssignIfDeg(row[shieldname+"_Min_Theta"],shield.range[iter].thetamin);
    AssignIfDeg(row[shieldname+"_Max_Theta"],shield.range[iter].thetamax);
    AssignIfDeg(row[shieldname+"_Min_Rho"],shield.range[iter].rhomin);
    AssignIfDeg(row[shieldname+"_Max_Rho"],shield.range[iter].rhomax);
  }
  shield.leak = (char)(stof(OPTIM_GET(row,table,Shield_Leak))*100.0);
  shield.recharge=stof(OPTIM_GET(row,table,Shield_Recharge));
  shield.efficiency=stof(OPTIM_GET(row,table,Shield_Efficiency),1.0);

  static bool  WCfuelhack=XMLSupport::parse_bool (vs_config->getVariable("physics","fuel_equals_warp","false"));
  maxwarpenergy=warpenergy=stof(OPTIM_GET(row,table,Warp_Capacitor));

  graphicOptions.MinWarpMultiplier=stof(OPTIM_GET(row,table,Warp_Min_Multiplier),1.0);
  graphicOptions.MaxWarpMultiplier=stof(OPTIM_GET(row,table,Warp_Max_Multiplier),1.0);

  maxenergy=energy=stof(OPTIM_GET(row,table,Primary_Capacitor));
  recharge=stof(OPTIM_GET(row,table,Reactor_Recharge));
  jump.drive=XMLSupport::parse_bool(OPTIM_GET(row,table,Jump_Drive_Present))?-1:-2;
  jump.delay = stoi(OPTIM_GET(row,table,Jump_Drive_Delay));
  image->forcejump=XMLSupport::parse_bool(OPTIM_GET(row,table,Wormhole));
  graphicOptions.RecurseIntoSubUnitsOnCollision=stob(OPTIM_GET(row,table,Collide_Subunits),graphicOptions.RecurseIntoSubUnitsOnCollision?true:false)?1:0;
  jump.energy=stof(OPTIM_GET(row,table,Outsystem_Jump_Cost));
  jump.insysenergy=stof(OPTIM_GET(row,table,Warp_Usage_Cost));
  if (WCfuelhack) fuel=warpenergy=warpenergy+jump.energy*0.1f; //this is required to make sure we don't trigger the "globally out of fuel" if we use all warp charges -- save some afterburner for later!!!
  afterburnenergy=stof(OPTIM_GET(row,table,Afterburner_Usage_Cost),32767);
  afterburntype = stoi(OPTIM_GET(row,table,Afterburner_Type)); //type 1 == "use fuel", type 0 == "use reactor energy", type 2 ==(hopefully) "use jump fuel"
  limits.yaw=stof(OPTIM_GET(row,table,Maneuver_Yaw))*VS_PI/180.;
  limits.pitch=stof(OPTIM_GET(row,table,Maneuver_Pitch))*VS_PI/180.;
  limits.roll=stof(OPTIM_GET(row,table,Maneuver_Roll))*VS_PI/180.;
  {
    std::string t,tn,tp;
    t=OPTIM_GET(row,table,Yaw_Governor);
    tn=OPTIM_GET(row,table,Yaw_Governor_Right);
    tp=OPTIM_GET(row,table,Yaw_Governor_Left);
    computer.max_yaw_right=stof(tn.length()>0?tn:t)*VS_PI/180.;
    computer.max_yaw_left=stof(tp.length()>0?tp:t)*VS_PI/180.;
    t=OPTIM_GET(row,table,Pitch_Governor);
    tn=OPTIM_GET(row,table,Pitch_Governor_Up);
    tp=OPTIM_GET(row,table,Pitch_Governor_Down);
    computer.max_pitch_up=stof(tn.length()>0?tn:t)*VS_PI/180.;
    computer.max_pitch_down=stof(tp.length()>0?tp:t)*VS_PI/180.;
    t=OPTIM_GET(row,table,Roll_Governor);
    tn=OPTIM_GET(row,table,Roll_Governor_Right);
    tp=OPTIM_GET(row,table,Roll_Governor_Left);
    computer.max_roll_right=stof(tn.length()>0?tn:t)*VS_PI/180.;
    computer.max_roll_left=stof(tp.length()>0?tp:t)*VS_PI/180.;
  }
  static float game_accel=XMLSupport::parse_float(vs_config->getVariable("physics","game_accel","1"));
  static float game_speed=XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"));
  limits.afterburn = stof(OPTIM_GET(row,table,Afterburner_Accel))*game_accel*game_speed;
  limits.forward = stof(OPTIM_GET(row,table,Forward_Accel))*game_accel*game_speed;
  limits.retro = stof(OPTIM_GET(row,table,Retro_Accel))*game_accel*game_speed;
  limits.lateral = .5*(stof(OPTIM_GET(row,table,Left_Accel))+stof(OPTIM_GET(row,table,Right_Accel)))*game_accel*game_speed;
  limits.vertical = .5*(stof(OPTIM_GET(row,table,Top_Accel))+stof(OPTIM_GET(row,table,Bottom_Accel)))*game_accel*game_speed;
  computer.max_combat_speed=stof(OPTIM_GET(row,table,Default_Speed_Governor))*game_speed;
  computer.max_combat_ab_speed=stof(OPTIM_GET(row,table,Afterburner_Speed_Governor))*game_speed;
  computer.itts = stob(OPTIM_GET(row,table,ITTS),true);
  computer.radar.canlock = stob(OPTIM_GET(row,table,Can_Lock),true);
  {
    std::string iffval = OPTIM_GET(row,table,Radar_Color);
    int iff=stoi(iffval,0);
    computer.radar.iff=iff?iff:stob(iffval,false);
  }
  computer.radar.maxrange=stof(OPTIM_GET(row,table,Radar_Range),FLT_MAX);
  computer.radar.maxcone=cos(stof(OPTIM_GET(row,table,Max_Cone),180)*VS_PI/180);
  computer.radar.trackingcone=cos(stof(OPTIM_GET(row,table,Tracking_Cone),180)*VS_PI/180);
  computer.radar.lockcone=cos(stof(OPTIM_GET(row,table,Lock_Cone),180)*VS_PI/180);
  cloakmin=(int)(stof(OPTIM_GET(row,table,Cloak_Min))*2147483136);
  if (cloakmin<0) cloakmin=0;
  image->cloakglass=XMLSupport::parse_bool(OPTIM_GET(row,table,Cloak_Glass));
  if ((cloakmin&0x1)&&!image->cloakglass) {
    cloakmin-=1;
  }
  if ((cloakmin&0x1)==0&&image->cloakglass) {
    cloakmin+=1;
  }


  if (!XMLSupport::parse_bool(OPTIM_GET(row,table,Can_Cloak)))
    cloaking=-1;
  else
    cloaking = (int)(-2147483647)-1;
  image->cloakrate = (int)(2147483136.*stof(OPTIM_GET(row,table,Cloak_Rate))); //short fix  
  image->cloakenergy=stof(OPTIM_GET(row,table,Cloak_Energy));
  image->repair_droid=stoi(OPTIM_GET(row,table,Repair_Droid));
  image->ecm = stoi(OPTIM_GET(row,table,ECM_Rating));

  this->HeatSink = stof(OPTIM_GET(row,table,Heat_Sink_Rating));
  if (image->ecm<0) image->ecm*=-1;
  if (image->cockpit_damage){
    HudDamage(image->cockpit_damage,OPTIM_GET(row,table,Hud_Functionality));
    HudDamage(image->cockpit_damage+1+MAXVDUS+UnitImages::NUMGAUGES,OPTIM_GET(row,table,Max_Hud_Functionality));
  }
  image->LifeSupportFunctionality=stof(OPTIM_GET(row,table,Lifesupport_Functionality));
  image->LifeSupportFunctionalityMax=stof(OPTIM_GET(row,table,Max_Lifesupport_Functionality));
  image->CommFunctionality=stof(OPTIM_GET(row,table,Comm_Functionality));
  image->CommFunctionalityMax=stof(OPTIM_GET(row,table,Max_Comm_Functionality));
  image->fireControlFunctionality=stof(OPTIM_GET(row,table,FireControl_Functionality));
  image->fireControlFunctionalityMax=stof(OPTIM_GET(row,table,Max_FireControl_Functionality));
  image->SPECDriveFunctionality=stof(OPTIM_GET(row,table,SPECDrive_Functionality));
  image->SPECDriveFunctionalityMax=stof(OPTIM_GET(row,table,Max_SPECDrive_Functionality));
  computer.slide_start=stoi(OPTIM_GET(row,table,Slide_Start));
  computer.slide_end=stoi(OPTIM_GET(row,table,Slide_End));
  double upgradet=queryTime();
  UpgradeUnit(this,OPTIM_GET(row,table,Upgrades));

  {
    std::string tractorability = OPTIM_GET(row,table,Tractorability);
    unsigned char tflags;
    if (!tractorability.empty()) {
        tflags = tractorImmune;
        if (tractorability.find_first_of("pP")!=string::npos)
            tflags |= tractorPush;
        if (tractorability.find_first_of("iI")!=string::npos)
            tflags |= tractorIn;
    } else tflags = tractorPush;
    setTractorability((enum tractorHow)tflags);
  }
  double explodet=queryTime();
  this->image->explosion_type = OPTIM_GET(row,table,Explosion);
  if (image->explosion_type.get().length()) {
    cache_ani (image->explosion_type);
  } else {
    static std::string expani = vs_config->getVariable ("graphics","explosion_animation","explosion_orange.ani");
    cache_ani (expani);
  }
  double lightt=queryTime();
  AddLights(this,xml,OPTIM_GET(row,table,Light));

  xml.shieldmesh_str = OPTIM_GET(row,table,Shield_Mesh);
  if (xml.shieldmesh_str.length()){
    addShieldMesh(&xml,xml.shieldmesh_str.c_str(),xml.unitscale,faction,getFlightgroup());
    meshdata.back()=xml.shieldmesh;
  }else {
    static int shieldstacks = XMLSupport::parse_int (vs_config->getVariable ("graphics","shield_detail","16"));
    static std::string shieldtex = vs_config->getVariable("graphics","shield_texture","shield.bmp");
    meshdata.back()= new SphereMesh (rSize(),shieldstacks,shieldstacks,shieldtex.c_str(), NULL, false,ONE, ONE);
  }
  meshdata.back()->EnableSpecialFX();

  //Begin the Pow-w-w-war Zone Collide Tree Generation
    double treet=queryTime();
  {
    xml.bspmesh_str = OPTIM_GET(row,table,BSP_Mesh);
    xml.rapidmesh_str = OPTIM_GET(row,table,Rapid_Mesh);
    vector<bsp_polygon> polies;

    std::string collideTreeHash = VSFileSystem::GetHashName(modification+"#"+row[0]);
    this->colTrees = collideTrees::Get(collideTreeHash);
    if (this->colTrees) {
      this->colTrees->Inc();
    }
    BSPTree * bspTree=NULL;
    BSPTree * bspShield=NULL;
    csRapidCollider *colShield=NULL;
    csRapidCollider *colTree=NULL;
    string tmpname = row[0];//key
    if (!this->colTrees) {
      string val;
      xml.hasBSP=1;
      xml.hasColTree=1;
      if ((val=OPTIM_GET(row,table,Use_BSP)).length()) {
        xml.hasBSP = XMLSupport::parse_bool(val);
      }
      if ((val=OPTIM_GET(row,table,Use_Rapid)).length()) {
        xml.hasColTree= XMLSupport::parse_bool(val);
      }
      if (xml.shieldmesh) {
        if (!CheckBSP ((tmpname+"_shield.bsp").c_str())) {
          BuildBSPTree ((tmpname+"_shield.bsp").c_str(), false, meshdata.back());
        }
        if (CheckBSP ((tmpname+"_shield.bsp").c_str())) {
          bspShield = new BSPTree ((tmpname+"_shield.bsp").c_str());
        }
        if (meshdata.back()) {
          meshdata.back()->GetPolys(polies);
          colShield = new csRapidCollider (polies);
        }
      }
      if (xml.rapidmesh_str.length())
        addRapidMesh(&xml,xml.rapidmesh_str.c_str(),xml.unitscale,faction,getFlightgroup());
      else 
        xml.rapidmesh=NULL;
      if (xml.bspmesh_str.length())
        addBSPMesh(&xml,xml.bspmesh_str.c_str(),xml.unitscale,faction,getFlightgroup());
      else
        xml.bspmesh=NULL;
      if (xml.hasBSP) {
        tmpname += ".bsp";
        if (!CheckBSP (tmpname.c_str())) {
          BuildBSPTree (tmpname.c_str(), false, xml.bspmesh);
        }
        if (CheckBSP (tmpname.c_str())) {
          bspTree = new BSPTree (tmpname.c_str());
        }	
      } else {
        bspTree = NULL;
      }
      polies.clear();
      if (xml.rapidmesh) {
        xml.rapidmesh->GetPolys(polies);
      }
      csRapidCollider * csrc=NULL;
      if (xml.hasColTree) {
        csrc=getCollideTree(Vector(1,1,1),
                            xml.rapidmesh?
                            &polies:NULL);
      }
      this->colTrees = new collideTrees (collideTreeHash,
                                         bspTree,
                                         bspShield,
                                         csrc,
                                         colShield);
      if (xml.rapidmesh&&xml.hasColTree) {
        //if we have a special rapid mesh we need to generate things now
        for (int i=1;i<collideTreesMaxTrees;++i) {
          if (!this->colTrees->rapidColliders[i]) {
            unsigned int which = 1<<i;
            this->colTrees->rapidColliders[i]= 
              getCollideTree(Vector (1,1,which),
                             &polies);
          }
        }
      }
      if (xml.bspmesh) {
        delete xml.bspmesh;
        xml.bspmesh=NULL;
      }
      if (xml.rapidmesh) {
        delete xml.rapidmesh;
        xml.rapidmesh=NULL;
      }
    }
  }

  CheckAccessory(this);//turns on the ceerazy rotation for any accessories
  this->setAverageGunSpeed();
  double endt=queryTime();
  //printf ("spr %f mesh %f subun %f mount %f carg %f sound %f upg %f exp %f light %f tree %f\n",spritet-start,subunt-meshest,mountst-subunt,cargot-mountst,soundst-cargot,upgradet-soundst,explodet-upgradet, lightt-explodet, treet-lightt, endt-treet); 
}

CSVRow GetUnitRow(string filename, bool subu, int faction, bool readlast, bool &rread) {
  std::string hashname = filename+"__"+FactionUtil::GetFactionName(faction);
  for (int i=((int)unitTables.size())-(readlast?1:2);i>=0;--i) {
    unsigned int where;
    if (unitTables[i]->RowExists(hashname,where)) {
      rread=true;
      return CSVRow(unitTables[i],where);
    }else if (unitTables[i]->RowExists(filename,where)) {
      rread=true;
      return CSVRow(unitTables[i],where); 
    }
  }
  rread=false;
  return CSVRow();
}

void Unit::WriteUnit (const char * modifications) {
  static bool UNITTAB = XMLSupport::parse_bool(vs_config->getVariable("physics","UnitTable","false"));
  if (UNITTAB) {
    bool bad=false;
    if (!modifications)bad=true;
    if (!bad) {
      if(!strlen(modifications)) {
        bad=true;
      }
    }
    if (bad) {
      fprintf(stderr,"Cannot Write out unit file %s %s that has no filename\n",name.get().c_str(),csvRow.get().c_str());
      return;
    }
    std::string savedir = modifications;
    VSFileSystem::CreateDirectoryHome( VSFileSystem::savedunitpath+"/"+savedir);
    VSFile f;
    //string filepath( savedir+"/"+this->filename);
    //cerr<<"Saving Unit to : "<<filepath<<endl;
    VSError err = f.OpenCreateWrite( savedir+"/"+name+".csv", UnitFile);
    if (err>Ok) {
      fprintf( stderr, "!!! ERROR : Writing saved unit file : %s\n", f.GetFullPath().c_str() );
      return;
    }
    std::string towrite = WriteUnitString();
    f.Write(towrite.c_str(),towrite.length());
    f.Close();
  }else {
    if (image->unitwriter)
      image->unitwriter->Write(modifications);
    for (un_iter ui= getSubUnits();(*ui)!=NULL;++ui) {
      (*ui)->WriteUnit(modifications);
    } 
  }
}
using XMLSupport::tostring;

static void mapToStringVec(stdext::hash_map<string,string> a, vector<string> &key, vector<string> &value) {
	for (stdext::hash_map<string,string>::iterator i = a.begin();i!=a.end();++i) {
    key.push_back(i->first);
    value.push_back(i->second);
  }
}
static string tos(float val) {
  return XMLSupport::tostring(val);
}
static string tos(double val) {
  return XMLSupport::tostring((float)val);
}
static string tos(unsigned int val) {
  return XMLSupport::tostring(val);
}
static string tos(bool val) {
  return XMLSupport::tostring((int)val);
}
static string tos(int val) {
  return XMLSupport::tostring(val);
}
string Unit::WriteUnitString () {
  static bool UNITTAB = XMLSupport::parse_bool(vs_config->getVariable("physics","UnitTable","false"));
  string ret="";
  if (UNITTAB) {
    //this is the fillin part
    //fixme

    for (int i=unitTables.size()-1;i>=0;--i) {
      unsigned int where;
      string val;
      if (unitTables[i]->RowExists(csvRow,where)) {
        CSVRow row(unitTables[i],where);        
		stdext::hash_map<string,string> unit;        
        for (int jj=0;
             jj<row.size();
             ++jj) {
          if (jj!=0)
            unit[row.getKey(jj)]=row[jj];
        }
        // mutable things
        unit["Equipment_Space"]=XMLSupport::tostring(image->equipment_volume);
        unit["Hold_Volume"]=XMLSupport::tostring(image->CargoVolume);
        unit["Hidden_Hold_Volume"]=XMLSupport::tostring(image->HiddenCargoVolume);
        unit["Upgrade_Storage_Volume"]=XMLSupport::tostring(image->UpgradeVolume);
        string mountstr;
        double unitScale=  stof(unit["Unit_Scale"],1);
        {//mounts
          for (int j=0;j<mounts.size();++j) {
            char mnt[1024];
            Matrix m;
            Transformation tr(mounts[j].GetMountOrientation(),
                              mounts[j].GetMountLocation().Cast());
            tr.to_matrix(m);
            string printedname=mounts[j].type->weapon_name;
            if (mounts[j].status==Mount::DESTROYED||mounts[j].status==Mount::UNCHOSEN) {
              printedname="";
            }
            mountstr+="{"+printedname+";"+XMLSupport::tostring(mounts[j].ammo)+";"+XMLSupport::tostring(mounts[j].volume)+";"+lookupMountSize(mounts[j].size);
            sprintf(mnt,";%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf}",
                    m.p.i/unitScale,
                    m.p.j/unitScale,
                    m.p.k/unitScale,
                    (double)mounts[j].xyscale/unitScale,
                    (double)mounts[j].zscale/unitScale,
                    (double)m.getR().i,
                    (double)m.getR().j,
                    (double)m.getR().k,
                    (double)m.getQ().i,
                    (double)m.getQ().j,
                    (double)m.getQ().k,
                    (double)mounts[j].functionality,
                    (double)mounts[j].maxfunctionality
                    );
            mountstr+=mnt;
          }
          unit["Mounts"]=mountstr;
        }
        {//subunits
          vector<SubUnitStruct> subunits=GetSubUnits(unit["Sub_Units"]);
          if (subunits.size()) {
            unsigned int k=0;
            Unit * subun;
            for (;k<subunits.size();++k) {
              subunits[k].filename="destroyed_blank";
            }
            k=0;
            for (un_iter su=this->getSubUnits();(subun=(*su))!=NULL;++su,++k) {
              unsigned int j=k;
              for (;j<subunits.size();++j) {
                if ((subun->Position()-subunits[j].pos).MagnitudeSquared()<.00000001) {
                  //we've got a hit
                  break;
                }
              }
              if (j>=subunits.size()) j=k;
              if (j<subunits.size())
                subunits[j].filename=subun->name;
            }
            string str;
            for (k=0;k<subunits.size();++k) {
              char tmp[1024];
              sprintf(tmp,";%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf}",
                      subunits[k].pos.i,
                      subunits[k].pos.j,
                      subunits[k].pos.k,
                      subunits[k].R.i,
                      subunits[k].R.j,
                      subunits[k].R.k,
                      subunits[k].Q.i,
                      subunits[k].Q.j,
                      subunits[k].Q.k,
                      ((double)acos(subunits[k].restricted)*180./VS_PI));
              str+="{"+subunits[k].filename+tmp;
            }
            unit["Sub_Units"]=str;
          }
        }
        {
          string carg;
          for (unsigned int i=0;i<numCargo();++i) {
            Cargo *c=&GetCargo(i);
            char tmp[2048];
            sprintf (tmp,";%f;%d;%f;%f;%f;%f;;%s}",
                     c->price,
                     c->quantity,
                     c->mass,
                     c->volume,
                     c->functionality,
                     c->maxfunctionality,
                     c->mission?"true":"false");
            carg+="{"+c->GetContent()+";"+c->GetCategory()+tmp;
          }
          unit["Cargo"]=carg;
        }
        unit["Mass"]=tos(Mass);
        unit["Moment_Of_Inertia"]=tos(Momentofinertia);
        unit["Fuel_Capacity"]=tos(fuel);
        unit["Hull"]=tos(hull);
        unit["Spec_Interdiction"]=tos(specInterdiction);
        unit["Armor_Front_Top_Left"]=tos(armor.frontlefttop);
        unit["Armor_Front_Top_Right"]=tos(armor.frontrighttop);
        unit["Armor_Back_Top_Left"]=tos(armor.backlefttop);
        unit["Armor_Back_Top_Right"]=tos(armor.backrighttop);
        unit["Armor_Front_Bottom_Left"]=tos(armor.frontleftbottom);
        unit["Armor_Front_Bottom_Right"]=tos(armor.frontrightbottom);
        unit["Armor_Back_Bottom_Left"]=tos(armor.backleftbottom);
        unit["Armor_Back_Bottom_Right"]=tos(armor.backrightbottom);        
        {
          unit["Shield_Front_Top_Right"]="";
          unit["Shield_Front_Top_Left"]="";
          unit["Shield_Back_Top_Right"]="";
          unit["Shield_Back_Top_Left"]="";
          unit["Shield_Front_Bottom_Right"]="";
          unit["Shield_Front_Bottom_Left"]="";
          unit["Shield_Back_Bottom_Right"]="";
          unit["Shield_Back_Bottom_Left"]="";

          switch (shield.number) {
          case 8:
            unit["Shield_Front_Top_Right"]=tos(shield.shield8.frontrighttopmax);
            unit["Shield_Front_Top_Left"]=tos(shield.shield8.frontlefttopmax);
            unit["Shield_Back_Top_Right"]=tos(shield.shield8.backrighttopmax);
            unit["Shield_Back_Top_Left"]=tos(shield.shield8.backlefttopmax);
            unit["Shield_Front_Bottom_Right"]=tos(shield.shield8.frontrightbottommax);
            unit["Shield_Front_Bottom_Left"]=tos(shield.shield8.frontleftbottommax);
            unit["Shield_Back_Bottom_Right"]=tos(shield.shield8.backrightbottommax);
            unit["Shield_Back_Bottom_Left"]=tos(shield.shield8.backleftbottommax);            
            break;
          case 4:
            unit["Shield_Front_Top_Right"]=tos(shield.shield4fbrl.frontmax);            
            unit["Shield_Back_Top_Right"]=tos(shield.shield4fbrl.backmax);
            unit["Shield_Front_Bottom_Right"]=tos(shield.shield4fbrl.rightmax);
            unit["Shield_Front_Bottom_Left"]=tos(shield.shield4fbrl.leftmax);            
            break;
          case 2:
            unit["Shield_Front_Top_Right"]=tos(shield.shield2fb.frontmax);            
            unit["Shield_Back_Top_Right"]=tos(shield.shield2fb.backmax);
            break;
          // NOTE: otherwise, no shields
          }
        }
        unit["Shield_Leak"]=tos(shield.leak/100.0);
        unit["Shield_Recharge"]=tos(shield.recharge);
        unit["Shield_Efficiency"]=tos(shield.efficiency);
        unit["Warp_Capacitor"]=tos(maxwarpenergy);
        unit["Warp_Min_Multiplier"]=tos(graphicOptions.MinWarpMultiplier);
        unit["Warp_Max_Multiplier"]=tos(graphicOptions.MaxWarpMultiplier);
        unit["Primary_Capacitor"]=tos(maxenergy);
        unit["Reactor_Recharge"]=tos(recharge);
        unit["Jump_Drive_Present"]=tos(jump.drive>=-1);
        unit["Jump_Drive_Delay"]=tos(jump.delay);
        unit["Wormhole"]=tos(image->forcejump!=0);
        unit["Outsystem_Jump_Cost"]=tos(jump.energy);
        unit["Warp_Usage_Cost"]=tos(jump.insysenergy);
        unit["Afterburner_Usage_Cost"]=tos(afterburnenergy);
        unit["Afterburner_Type"]=tos(afterburntype);
        unit["Maneuver_Yaw"]=tos(limits.yaw*180/(VS_PI));
        unit["Maneuver_Pitch"]=tos(limits.pitch*180/(VS_PI));
        unit["Maneuver_Roll"]=tos(limits.roll*180/(VS_PI));
        unit["Yaw_Governor_Right"]=tos(computer.max_yaw_right*180/VS_PI);
        unit["Yaw_Governor_Left"]=tos(computer.max_yaw_left*180/VS_PI);
        unit["Pitch_Governor_Up"]=tos(computer.max_pitch_up*180/VS_PI);
        unit["Pitch_Governor_Down"]=tos(computer.max_pitch_down*180/VS_PI);
        unit["Roll_Governor_Right"]=tos(computer.max_roll_right*180/VS_PI);
        unit["Roll_Governor_Left"]=tos(computer.max_roll_left*180/VS_PI);
        static float game_accel=XMLSupport::parse_float(vs_config->getVariable("physics","game_accel","1"));
        static float game_speed=XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1"));
        unit["Afterburner_Accel"]=tos(limits.afterburn/(game_accel*game_speed));
        unit["Forward_Accel"]=tos(limits.forward/(game_accel*game_speed));
        unit["Retro_Accel"]=tos(limits.retro/(game_accel*game_speed));
        unit["Left_Accel"]=unit["Right_Accel"]=tos(limits.lateral/(game_accel*game_speed));
        unit["Bottom_Accel"]=unit["Top_Accel"]=tos(limits.vertical/(game_accel*game_speed));
        unit["Default_Speed_Governor"]=tos(computer.max_combat_speed/game_speed);
        unit["Afterburner_Speed_Governor"]=tos(computer.max_combat_ab_speed/game_speed);
        unit["ITTS"]=tos(computer.itts);
        unit["Can_Lock"]=tos(computer.radar.canlock);
        unit["Radar_Color"]=tos((int)computer.radar.iff);
        unit["Radar_Range"]=tos(computer.radar.maxrange);
        unit["Tracking_Cone"]=tos(acos(computer.radar.trackingcone)*180./VS_PI);
        unit["Max_Cone"]=tos(acos(computer.radar.maxcone)*180./VS_PI);
        unit["Lock_Cone"]=tos(acos(computer.radar.lockcone)*180./VS_PI);
        unit["Cloak_Min"]=tos(cloakmin/2147483136.);
        unit["Can_Cloak"]=tos(cloaking!=-1);
        unit["Cloak_Rate"]=tos(fabs(image->cloakrate/2147483136.));
        unit["Cloak_Energy"]=tos(image->cloakenergy);
        unit["Cloak_Glass"]=tos(image->cloakglass);
        unit["Repair_Droid"]=tos(image->repair_droid);
        unit["ECM_Rating"]=tos(image->ecm>0?image->ecm:-image->ecm);
        unit["Hud_Functionality"]=WriteHudDamage(this);
        unit["Max_Hud_Functionality"]=WriteHudDamageFunc(this);
        unit["Heat_Sink_Rating"]=tos(this->HeatSink);
        unit["Lifesupport_Functionality"]=tos(image->LifeSupportFunctionality);       
        unit["Max_Lifesupport_Functionality"]=tos(image->LifeSupportFunctionalityMax);
        unit["Comm_Functionality"]=tos(image->CommFunctionality);
        unit["Max_Comm_Functionality"]=tos(image->CommFunctionalityMax);        
        unit["Comm_Functionality"]=tos(image->CommFunctionality);
        unit["Max_Comm_Functionality"]=tos(image->CommFunctionalityMax);
        unit["FireControl_Functionality"]=tos(image->fireControlFunctionality);
        unit["Max_FireControl_Functionality"]=tos(image->fireControlFunctionalityMax);
        unit["SPECDrive_Functionality"]=tos(image->SPECDriveFunctionality);
        unit["Max_SPECDrive_Functionality"]=tos(image->SPECDriveFunctionalityMax);
        unit["Slide_Start"]=tos(computer.slide_start);
        unit["Slide_End"]=tos(computer.slide_end);
        unit["Cargo_Import"]=unit["Upgrades"]="";//make sure those are empty        
        {
            std::string trac;
            if (isTractorable(tractorPush)) trac += "p";
            if (isTractorable(tractorIn)) trac += "i";
            if (trac.empty()) trac = "-";
            unit["Tractorability"] = trac;
        }
        vector<string>keys,values;
        keys.push_back("Key");
        values.push_back(csvRow);//key has to come first
        mapToStringVec(unit,keys,values);
        return writeCSV(keys,values);
      }
    }
    fprintf (stderr,"Failed to locate base mesh for %s %s %s\n",csvRow.get().c_str(),name.get().c_str(),fullname.c_str());
  }else {
    if (image->unitwriter)
      ret = image->unitwriter->WriteString();
    for (un_iter ui= getSubUnits();(*ui)!=NULL;++ui) {
      ret = ret + ((*ui)->WriteUnitString());
    }
  }
  return ret;
}
void UpdateMasterPartList(Unit * ret) {
  for (int i=0;i<_Universe->numPlayers();++i) {
    Cockpit* cp = _Universe->AccessCockpit(i);
    std::vector<StringPool::Reference>* addedcargoname= &cp->savegame->getMissionStringData("master_part_list_content");
    std::vector<StringPool::Reference>* addedcargocat= &cp->savegame->getMissionStringData("master_part_list_category");
    std::vector<StringPool::Reference>* addedcargovol= &cp->savegame->getMissionStringData("master_part_list_volume");
    std::vector<StringPool::Reference>* addedcargoprice= &cp->savegame->getMissionStringData("master_part_list_price");
    std::vector<StringPool::Reference>* addedcargomass= &cp->savegame->getMissionStringData("master_part_list_mass");
    std::vector<StringPool::Reference>* addedcargodesc= &cp->savegame->getMissionStringData("master_part_list_description");
    for (unsigned int j=0;j<addedcargoname->size();++j) {
      Cargo carg;
      carg.content=(*addedcargoname)[j];
      carg.category=(j<addedcargocat->size()?(*addedcargocat)[j]:std::string("Uncategorized"));
      carg.volume=(j<addedcargovol->size()?XMLSupport::parse_float((*addedcargovol)[j]):1.0);
      carg.price=(j<addedcargoprice->size()?XMLSupport::parse_float((*addedcargoprice)[j]):0.0);
      carg.mass=(j<addedcargomass->size()?XMLSupport::parse_float((*addedcargomass)[j]):.01);
      carg.description=(j<addedcargodesc->size()?(*addedcargodesc)[j]:std::string("No Description Added"));
      carg.quantity=1;
      ret->GetImageInformation().cargo.push_back(carg);
    }
  }
  std::sort(ret->GetImageInformation().cargo.begin(),  ret->GetImageInformation().cargo.end());
  {
    Cargo last_cargo;
    for (int i=ret->numCargo()-1;i>=0;--i) {
      if (ret->GetCargo(i).content==last_cargo.content&&
          ret->GetCargo(i).category==last_cargo.category) {
        ret->RemoveCargo(i,ret->GetCargo(i).quantity,true);
      }else {
        last_cargo=ret->GetCargo(i);
      }
    }
  }
}
Unit * Unit::makeMasterPartList() {
  static std::string mpl = vs_config->getVariable("data","master_part_list","master_part_list");
  Unit * ret = new Unit();
  ret->name="master_part_list";
  VSFileSystem::VSFile mplf;
  VSFileSystem::VSError err = mplf.OpenReadOnly(mpl,VSFileSystem::UnknownFile);
  unsigned int i;
  if (err<=VSFileSystem::Ok) {
         CSVTable table(mplf,mplf.GetRoot());
         mplf.Close();
		 stdext::hash_map<std::string,int>::const_iterator it;
		 for (it=table.rows.begin();it!=table.rows.end();++it) {
			 CSVRow row(&table,it->second);
			 Cargo carg;
			 carg.content=row["file"];
			 carg.category=row["categoryname"];
			 carg.volume=stof(row["volume"],1);
			 carg.mass=stof(row["mass"],1);
			 carg.quantity=1;
			 carg.price=stoi(row["price"],1);
			 carg.description=row["description"];    
			 ret->GetImageInformation().cargo.push_back(carg);
		 }
  }
  UpdateMasterPartList(ret);
  
  if (!ret->GetCargo("Pilot",i)) {//required items
    ret->AddCargo(Cargo("Pilot","Contraband",800,1,.01,1,1.0,1.0),true);
  }
  if (!ret->GetCargo("Hitchhiker",i)) {
    ret->AddCargo(Cargo("Hitchhiker","Passengers",42,1,.01,5.0,1.0,1.0),true);
  }
  if (!ret->GetCargo("Slaves",i)) {
    ret->AddCargo(Cargo("Slaves","Contraband",800,1,.01,1,1,1),true);
  }
  return ret;
}
