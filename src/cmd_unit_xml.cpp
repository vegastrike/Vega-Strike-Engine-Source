#include "cmd_unit.h"
#include "xml_support.h"
#include "gfx_halo.h"
//#include <iostream.h>
#include <fstream>
#include <expat.h>
//#include <values.h>
#include <float.h>
#include "gfx_mesh.h"
#include "gfx_sphere.h"
#include "gfx_bsp.h"
#define VS_PI 3.1415926536
void Unit::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  ((Unit*)userData)->beginElement(name, AttributeList(atts));
}

void Unit::endElement(void *userData, const XML_Char *name) {
  ((Unit*)userData)->endElement(name);
}


namespace UnitXML {
    enum Names {
      UNKNOWN,
      UNIT,
      SUBUNIT,
      MESHFILE,
      SHIELDMESH,
      MOUNT,
      MESHLIGHT,
      XFILE,
      X,
      Y,
      Z,
      RI,
      RJ,
      RK,
      QI,
      QJ,
      QK,
      RED,
      GREEN,
      BLUE,
      ALPHA,
      MOUNTSIZE,
      WEAPON,
      DEFENSE,
      ARMOR,
      FORWARD,
      RETRO,
      FRONT,
      BACK,
      LEFT,
      RIGHT,
      TOP,
      BOTTOM,
      SHIELDS,
      RECHARGE,
      HULL,
      STRENGTH,
      STATS,
      MASS,
      MOMENTOFINERTIA,
      FUEL,
      THRUST,
      MANEUVER,
      YAW,
      ROLL,
      PITCH,
      ENGINE,
      COMPUTER,
      AACCEL,
      ENERGY,
      REACTOR,
      LIMIT,
      RESTRICTED,
      MAX,
      MIN,
      MAXSPEED,
      AFTERBURNER,
      BSPTREE,
      SHIELDTIGHT 
    };

  const EnumMap::Pair element_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("Unit", UNIT),
    EnumMap::Pair ("SubUnit", SUBUNIT),
    EnumMap::Pair ("MeshFile", MESHFILE),
    EnumMap::Pair ("ShieldMesh",SHIELDMESH),
    EnumMap::Pair ("Light",MESHLIGHT),
    EnumMap::Pair ("Defense", DEFENSE),
    EnumMap::Pair ("Armor", ARMOR),
    EnumMap::Pair ("Shields", SHIELDS),
    EnumMap::Pair ("Hull", HULL),
    EnumMap::Pair ("Stats", STATS),
    EnumMap::Pair ("Thrust", THRUST),
    EnumMap::Pair ("Maneuver", MANEUVER),
    EnumMap::Pair ("Engine", ENGINE),
    EnumMap::Pair ("Computer",COMPUTER),
    EnumMap::Pair ("Energy", ENERGY),
    EnumMap::Pair ("Reactor", REACTOR),
    EnumMap::Pair ("Restricted", RESTRICTED),
    EnumMap::Pair ("Yaw", YAW),
    EnumMap::Pair ("Pitch", PITCH),
    EnumMap::Pair ("Roll", ROLL),
    EnumMap::Pair ("Mount", MOUNT)

  };
  const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("file", XFILE), 
    EnumMap::Pair ("x", X), 
    EnumMap::Pair ("y", Y), 
    EnumMap::Pair ("z", Z), 
    EnumMap::Pair ("ri", RI), 
    EnumMap::Pair ("rj", RJ), 
    EnumMap::Pair ("rk", RK), 
    EnumMap::Pair ("qi", QI),     
    EnumMap::Pair ("qj", QJ),     
    EnumMap::Pair ("qk", QK),
    EnumMap::Pair ("red",RED),
    EnumMap::Pair ("green",GREEN),
    EnumMap::Pair ("blue",BLUE),    
    EnumMap::Pair ("alpha",ALPHA),
    EnumMap::Pair ("size", MOUNTSIZE),
    EnumMap::Pair ("forward",FORWARD),
    EnumMap::Pair ("retro", RETRO),    
    EnumMap::Pair ("front", FRONT),
    EnumMap::Pair ("back", BACK),
    EnumMap::Pair ("left", LEFT),
    EnumMap::Pair ("right", RIGHT),
    EnumMap::Pair ("top", TOP),
    EnumMap::Pair ("bottom", BOTTOM),
    EnumMap::Pair ("recharge", RECHARGE),
    EnumMap::Pair ("strength", STRENGTH),
    EnumMap::Pair ("mass", MASS),
    EnumMap::Pair ("momentofinertia", MOMENTOFINERTIA),
    EnumMap::Pair ("fuel", FUEL),
    EnumMap::Pair ("yaw", YAW),
    EnumMap::Pair ("pitch", PITCH),
    EnumMap::Pair ("roll", ROLL),
    EnumMap::Pair ("accel", AACCEL),
    EnumMap::Pair ("recharge", RECHARGE),
    EnumMap::Pair ("limit", LIMIT),
    EnumMap::Pair ("max", MAX),
    EnumMap::Pair ("min", MIN),
    EnumMap::Pair ("weapon", WEAPON),
    EnumMap::Pair ("maxspeed", MAXSPEED),
    EnumMap::Pair ("afterburner", AFTERBURNER),
    EnumMap::Pair ("bsptree",BSPTREE),
    EnumMap::Pair ("tightness",SHIELDTIGHT)
};

  const EnumMap element_map(element_names, 22);
  const EnumMap attribute_map(attribute_names, 42);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace UnitXML;

int parseMountSizes (const char * str) {
  char tmp[13][50];
  int ans = weapon_info::NOWEAP;
  int num= sscanf (str,"%s %s %s %s %s %s %s %s %s %s %s %s %s",tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8],tmp[9],tmp[10],tmp[11],tmp[12]);
  for (int i=0;i<num;i++) {
    ans |= lookupMountSize (tmp[i]);
  }
  return ans;
}

void Unit::beginElement(const string &name, const AttributeList &attributes) {
    string filename;
    Vector P;
    int indx;
    Vector Q;
    Vector R;
    Vector pos;
    bool tempbool;
    Names elem = (Names)element_map.lookup(name);
    int mntsiz=weapon_info::NOWEAP;
    AttributeList::const_iterator iter;
  switch(elem) {
  case UNKNOWN:
	xml->unitlevel++;

//    cerr << "Unknown element start tag '" << name << "' detected " << endl;
    break;
  case SHIELDMESH:
	assert (xml->unitlevel==1);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	xml->shieldmesh =(new Mesh((*iter).value.c_str(), true));
	break;
      case SHIELDTIGHT: 
	shieldtight = parse_float ((*iter).value);
	break;
      }
    }
    break;
  case MESHFILE:
	assert (xml->unitlevel==1);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	xml->meshes.push_back(new Mesh((*iter).value.c_str(), true));
	break;
      }
    }
    break;
  case MESHLIGHT:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    P=Vector (1,1,1);
    Q=Vector (1,1,1);
    pos=Vector(0,0,0);
    for (iter = attributes.begin();iter!=attributes.end();iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case X:
	pos.i=parse_float((*iter).value);
	break;
      case Y:
	pos.j=parse_float((*iter).value);
	break;
      case Z:
	pos.k=parse_float((*iter).value);
	break;
      case RED:
	Q.i=parse_float((*iter).value);
	break;
      case GREEN:
       	Q.j=parse_float((*iter).value);
	break;
      case BLUE:
	Q.k=parse_float((*iter).value);
	break;
      case ALPHA:
	P.k=parse_float((*iter).value);
	break;
      case XFILE:
	filename = (*iter).value;
	break;
      case MOUNTSIZE:
	P.i=parse_float((*iter).value);
	P.j=parse_float((*iter).value);
	break;
      }
    }
    xml->halos.push_back(new Halo(filename.c_str(),GFXColor(Q.i,Q.j,Q.k,P.k),pos,P.i,P.j));
    break;
  case MOUNT:
	assert (xml->unitlevel==1);
	xml->unitlevel++;
    Q = Vector (0,1,0);
    R = Vector (0,0,1);
    pos = Vector (0,0,0);
    tempbool=false;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case WEAPON:
	filename = (*iter).value;
	break;
      case MOUNTSIZE:
	tempbool=true;
	mntsiz=parseMountSizes((*iter).value.c_str());
	break;
      case X:
	pos.i=parse_float((*iter).value);
	break;
      case Y:
	pos.j=parse_float((*iter).value);
	break;
      case Z:
	pos.k=parse_float((*iter).value);
	break;
      case RI:
	R.i=parse_float((*iter).value);
	break;
      case RJ:
	R.j=parse_float((*iter).value);
	break;
      case RK:
	R.k=parse_float((*iter).value);
	break;
      case QI:
	Q.i=parse_float((*iter).value);
	break;
      case QJ:
	Q.j=parse_float((*iter).value);
	break;
      case QK:
	Q.k=parse_float((*iter).value);
	break;
      }

    }

     
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
    indx = xml->mountz.size();
    xml->mountz.push_back(new Mount (filename.c_str()));
    xml->mountz[indx]->SetMountPosition(Transformation(Quaternion::from_vectors(P,Q,R),pos));
    //xml->mountz[indx]->Activate();
    if (tempbool)
      xml->mountz[indx]->size=mntsiz;
    else
      xml->mountz[indx]->size = xml->mountz[indx]->type.size;
    //->curr_physical_state=xml->units[indx]->prev_physical_state;

    break;

  case SUBUNIT:
	assert (xml->unitlevel==1);
	xml->unitlevel++;
    Q = Vector (0,1,0);
    R = Vector (0,0,1);
    pos = Vector (0,0,0);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	filename = (*iter).value;
	break;
      case X:
	pos.i=parse_float((*iter).value);
	break;
      case Y:
	pos.j=parse_float((*iter).value);
	break;
      case Z:
	pos.k=parse_float((*iter).value);
	break;
      case RI:
	R.i=parse_float((*iter).value);
	break;
      case RJ:
	R.j=parse_float((*iter).value);
	break;
      case RK:
	R.k=parse_float((*iter).value);
	break;
      case QI:
	Q.i=parse_float((*iter).value);
	break;
      case QJ:
	Q.j=parse_float((*iter).value);
	break;
      case QK:
	Q.k=parse_float((*iter).value);
	break;
      }

    }
    Q.Normalize();
    R.Normalize();
    
    CrossProduct (Q,R,P);
    indx = xml->units.size();
    xml->units.push_back(new Unit (filename.c_str(), true));
    xml->units[indx]->prev_physical_state= Transformation(Quaternion::from_vectors(P,Q,R),pos);
    xml->units[indx]->curr_physical_state=xml->units[indx]->prev_physical_state;
    
    break;

  case ARMOR:
	assert (xml->unitlevel==2);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case FRONT:
	armor.front = parse_float((*iter).value);
	break;
      case BACK:
	armor.back=parse_float((*iter).value);
	break;
      case LEFT:
	armor.left=parse_float((*iter).value);
	break;
      case RIGHT:
	armor.right=parse_float((*iter).value);
	break;
      }
    }

 
    break;
  case SHIELDS:
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case FRONT:
	shield.front = parse_float((*iter).value);
	shield.number++;
	break;
      case BACK:
	shield.back=parse_float((*iter).value);
	shield.number++;
	break;
      case LEFT:
	shield.left=parse_float((*iter).value);
	shield.number++;
	break;
      case RIGHT:
	shield.right=parse_float((*iter).value);
	shield.number++;
	break;
      case TOP:
	shield.top=parse_float((*iter).value);
	shield.number++;
	break;
      case BOTTOM:
	shield.bottom=parse_float((*iter).value);
	shield.number++;
	break;
      case RECHARGE:
	shield.recharge=parse_float((*iter).value);
	break;
      }
    }
    break;
  case HULL:
	assert (xml->unitlevel==2);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case STRENGTH:
	hull = parse_float((*iter).value);
	break;
      }
    }
   
    break;
  case STATS:
	assert (xml->unitlevel==1);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MASS:
	mass = parse_float((*iter).value);
	break;
      case MOMENTOFINERTIA:
	MomentOfInertia=parse_float((*iter).value);
	break;
      case FUEL:
	fuel=parse_float((*iter).value);
	break;
      }
    }
	break;
  case MANEUVER:
	assert (xml->unitlevel==2);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case YAW:
	limits.yaw = parse_float((*iter).value)*(VS_PI/180);
	break;
      case PITCH:
	limits.pitch=parse_float((*iter).value)*(VS_PI/180);
	break;
      case ROLL:
	limits.roll=parse_float((*iter).value)*(VS_PI/180);
	break;
      }
    }

    
    break;

  case ENGINE:
	  
	assert (xml->unitlevel==2);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case AACCEL:
	accel=parse_float((*iter).value);
	break;
      case FORWARD:
	limits.forward=parse_float((*iter).value);
	break;
      case RETRO:
	limits.retro=parse_float((*iter).value);
	break;
      case AFTERBURNER:
	limits.afterburn=parse_float ((*iter).value);
	break;
      case LEFT:
	limits.lateral=parse_float((*iter).value);
	break;
      case RIGHT:
	limits.lateral=parse_float((*iter).value);
	break;
      case TOP:
	limits.vertical=parse_float((*iter).value);
	break;
      case BOTTOM:
	limits.vertical=parse_float((*iter).value);
	break;
    }
    }

    break;

  case COMPUTER:  
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MAXSPEED:
	computer.max_speed=parse_float((*iter).value);
	break;
      case AFTERBURNER:
	computer.max_ab_speed+=parse_float((*iter).value);
	break;
      case YAW:
	computer.max_yaw=parse_float((*iter).value)*(VS_PI/180);
	break;
      case PITCH:
	computer.max_pitch=parse_float((*iter).value)*(VS_PI/180);
	break;
      case ROLL:
	computer.max_roll=parse_float((*iter).value)*(VS_PI/180);
	break;
      }
    }
    break;


  case REACTOR:
	assert (xml->unitlevel==2);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case RECHARGE:
	recharge=parse_float((*iter).value);
	break;
      case LIMIT:
	energy=parse_float((*iter).value);
	break;
    }
    }
    break;

  case YAW:
	  yprrestricted+=YRESTR;
	assert (xml->unitlevel==2);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MAX:
	ymax=parse_float((*iter).value)*(VS_PI/180);
	break;
      case MIN:
	ymin=parse_float((*iter).value)*(VS_PI/180);
	break;
    }
    }
    break;

  case PITCH:
    yprrestricted+=PRESTR;
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MAX:
	pmax=parse_float((*iter).value)*(VS_PI/180);
	break;
      case MIN:
	pmin=parse_float((*iter).value)*(VS_PI/180);
	break;
      }
    }
    break;

  case ROLL:
    yprrestricted+=RRESTR;
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MAX:
	rmax=parse_float((*iter).value)*(VS_PI/180);
	break;
      case MIN:
	rmin=parse_float((*iter).value)*(VS_PI/180);
	break;
      }
    }
    break;

  case UNIT:
	assert (xml->unitlevel==0);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case BSPTREE:
	if (!bspTree)
	  bspTree = new BSPTree ((*iter).value.c_str());
	break;
      default:
	break;
      }
    }
    break;

  case DEFENSE:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    break;

  case THRUST:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    break;

  case ENERGY:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    break;

  case RESTRICTED:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    break;
	
  default:
	
    break;
  }
}

void Unit::endElement(const string &name) {
  Names elem = (Names)element_map.lookup(name);

  switch(elem) {
  case UNKNOWN:
	  xml->unitlevel--;
//    cerr << "Unknown element end tag '" << name << "' detected " << endl;
    break;
  default:
	  xml->unitlevel--;
    break;
  }
}

void Unit::LoadXML(const char *filename) {
  shield.number=0;
  const int chunk_size = 16384;
 // rrestricted=yrestricted=prestricted=false;
  FILE * inFile = fopen (filename, "r+b");
  if(!inFile) {
    assert(0);
    return;
  }

  xml = new XML;
  xml->shieldmesh = NULL;
  xml->unitlevel=0;
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &Unit::beginElement, &Unit::endElement);
  
  do {
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
    int length;
    
    length = fread (buf,1, chunk_size,inFile);
    //length = inFile.gcount();
    XML_ParseBuffer(parser, length, feof(inFile));
  } while(!feof(inFile));
fclose (inFile);
  // Load meshes into subunit
  nummesh = xml->meshes.size();
  meshdata = new Mesh*[nummesh+1];
  corner_min = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
  corner_max = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  int a;
  for( a=0; a<nummesh; a++) {
    meshdata[a] = xml->meshes[a];
  }
  numhalos = xml->halos.size();
  if (numhalos)
    halos = new Halo*[numhalos];
  else
    halos=NULL;
  for (a=0;a<numhalos;a++) {
    halos[a]=xml->halos[a];
  }
  nummounts = xml->mountz.size();
  if (nummounts)
    mounts = new Mount [nummounts];
  else
    mounts = NULL;
  for (a=0;a<nummounts;a++) {
    mounts[a]=*xml->mountz[a];
    delete xml->mountz[a];//do it stealthily... no cons/destructor
  }
  numsubunit = xml->units.size();
  if (numsubunit)
    subunits = new Unit*[numsubunit];
  else
    subunits=NULL;
  for( a=0; a<numsubunit; a++) {
    subunits[a] = xml->units[a];
  }

  calculate_extent();
  if (xml->shieldmesh) {
    meshdata[nummesh] = xml->shieldmesh;
  }else {
    SphereMesh * tmp = new SphereMesh (rSize(),16,16,"shield.bmp");
    tmp->SetBlendMode (ONE,ONE);
    meshdata[nummesh] = tmp;
  }

  meshdata[nummesh]->EnableSpecialFX();
  if (!bspTree) {
    string tmpname (filename);
    tmpname += ".bsp";
    FILE * fp = fopen (tmpname.c_str(),"r+b");
    if (!fp) {
      BuildBSPTree (tmpname.c_str());
    }else {
      fclose (fp);
    }
    fp = fopen (tmpname.c_str(),"r+b");
    if (fp) {
      fclose (fp);
      bspTree = new BSPTree (tmpname.c_str());
    }	
  }
  delete xml;
}

