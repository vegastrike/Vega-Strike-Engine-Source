#include <expat.h>
#include "xml_support.h"
#include "star_system.h"
#include "gfx/background.h"
#include "gfx/env_map_gent.h"
#include "gfx/aux_texture.h"
#include "cmd/planet.h"
#include "gfx/star.h"
#include "vs_globals.h"
#include "vs_path.h"
#include "config_xml.h"
#include "vegastrike.h"
#include "cmd/cont_terrain.h"
#include <assert.h>	/// needed for assert() calls.
#include "cmd/building.h"
#include "cmd/ai/aggressive.h"
#include "cmd/atmosphere.h"
void StarSystem::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  ((StarSystem*)userData)->beginElement(name, AttributeList(atts));
}

void StarSystem::endElement(void *userData, const XML_Char *name) {
  ((StarSystem*)userData)->endElement(name);
}

vector <char *> ParseDestinations (const string &value) {
  vector <char *> tmp;
  int i;
  int j;
  int k;
  for (j=0;value[j]!=0;){
    for (i=0;value[j]!=' '&&value[j]!='\0';i++,j++) {
    }
    tmp.push_back(new char [i+1]);
    for (k=0;k<i;k++) {
      tmp[tmp.size()-1][k]=value[k+j-i];
    }
    tmp[tmp.size()-1][i]='\0';
    if (value[j]!=0)
      j++;
  }
  return tmp;
}

namespace StarXML {
  enum Names {
    UNKNOWN,
    XFILE,
    X,
    Y,
    Z,
    RI,
    RJ,
    RK,
    SI,
    SJ,
    SK,
    QI,
    QJ,
    QK,
    NAME,
    RADIUS,
    GRAVITY,
    VELOCITY,
    PPOSITION,
    SYSTEM,
    PLANET,
    UNIT,
    EMRED,
    EMGREEN,
    EMBLUE,
    EMALPHA,
    BACKGROUND,
    STARS,
    STARSPREAD,
    NEARSTARS,
    FADESTARS,
    REFLECTIVITY,
    ALPHA,
    DESTINATION,
    JUMP,
    FACTION,
    LIGHT,
    COLL,
    ATTEN,
    DIFF,
    SPEC,
    AMB,
    TERRAIN,
    CONTTERRAIN,
    MASS,
    BUILDING,
    VEHICLE,
    ATMOSPHERE
  };

  const EnumMap::Pair element_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("Planet", PLANET),
    EnumMap::Pair ("System", SYSTEM),
    EnumMap::Pair ("Unit", UNIT),
    EnumMap::Pair ("Jump", JUMP),
    EnumMap::Pair ("Light", LIGHT),
    EnumMap::Pair ("Attenuated",ATTEN),
    EnumMap::Pair ("Diffuse",DIFF),
    EnumMap::Pair ("Specular",SPEC),
    EnumMap::Pair ("Ambient",AMB),
    EnumMap::Pair ("Terrain",TERRAIN),
    EnumMap::Pair ("ContinuousTerrain",CONTTERRAIN),
    EnumMap::Pair ("Building",BUILDING),
    EnumMap::Pair ("Vehicle",VEHICLE),
    EnumMap::Pair ("Atmosphere",ATMOSPHERE)
  };
  const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("background", BACKGROUND), 
    EnumMap::Pair ("stars", STARS),
    EnumMap::Pair ("nearstars", NEARSTARS),
    EnumMap::Pair ("fadestars", FADESTARS),
    EnumMap::Pair ("starspread", STARSPREAD), 
    EnumMap::Pair ("reflectivity", REFLECTIVITY), 
    EnumMap::Pair ("file", XFILE),
    EnumMap::Pair ("alpha", ALPHA),
    EnumMap::Pair ("destination", DESTINATION), 
    EnumMap::Pair ("x", X), 
    EnumMap::Pair ("y", Y), 
    EnumMap::Pair ("z", Z), 
    EnumMap::Pair ("ri", RI), 
    EnumMap::Pair ("rj", RJ), 
    EnumMap::Pair ("rk", RK), 
    EnumMap::Pair ("si", SI),     
    EnumMap::Pair ("sj", SJ),     
    EnumMap::Pair ("sk", SK),
    EnumMap::Pair ("qi", QI), 
    EnumMap::Pair ("qj", QJ), 
    EnumMap::Pair ("qk", QK), 
    EnumMap::Pair ("name", NAME),
    EnumMap::Pair ("radius", RADIUS),
    EnumMap::Pair ("gravity", GRAVITY),
    EnumMap::Pair ("velocity", VELOCITY),
    EnumMap::Pair ("position", PPOSITION),
    EnumMap::Pair ("Red", EMRED),
    EnumMap::Pair ("Green", EMGREEN),
    EnumMap::Pair ("Blue", EMBLUE),
    EnumMap::Pair ("Alpha", EMALPHA),
    EnumMap::Pair ("faction", FACTION),
    EnumMap::Pair ("Light", LIGHT),
    EnumMap::Pair ("Mass", MASS)
  };

  const EnumMap element_map(element_names, 15);
  const EnumMap attribute_map(attribute_names, 34);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace StarXML;


static void GetLights (const vector <GFXLight> &origlights, vector <GFXLightLocal> &curlights, const char *str) {
  int tint;
  char isloc;
  char * tmp=strdup (str);
  GFXLightLocal  lloc;
  char * st =tmp;
  int numel;
  while ((numel=sscanf (st,"%d%c",&tint,&isloc))>0) {
    assert (tint<(int)origlights.size());
    lloc.ligh = origlights[tint];
    lloc.islocal = (numel>1&&isloc=='l');
    curlights.push_back (lloc);
    while (isspace(*st) )
      st++;
    while (isalnum (*st)) {
      st++;
    }
  }
  free (tmp);
} 

void StarSystem::beginElement(const string &name, const AttributeList &attributes) {
  std::string myfile;
  vector <GFXLightLocal> curlights;
  xml->cursun.i=0;
  GFXColor tmpcol(0,0,0,1);
  LIGHT_TARGET tmptarg= POSITION;
  xml->cursun.j=0;
  int faction=0;
  xml->cursun.k=0;	
  GFXMaterial ourmat;
  GFXGetMaterial (0,ourmat);
  vs_config->getColor ("planet_mat_ambient",&ourmat.ar);
  vs_config->getColor ("planet_mat_diffuse",&ourmat.dr);
  vs_config->getColor ("planet_mat_specular",&ourmat.sr);
  vs_config->getColor ("planet_mat_emmissive",&ourmat.er);

  vector <char *>dest;
  char * filename =NULL;
  char * alpha = NULL;
  float gravity=0;
  float velocity=0;
  float position=0;
  Vector S(0,0,0), pos(0,0,0), R(0,0,0);
  Names elem = (Names)element_map.lookup(name);
  float radius=1;
  AttributeList::const_iterator iter;
  switch(elem) {
  case UNKNOWN:
    xml->unitlevel++;

    //    cerr << "Unknown element start tag '" << name << "' detected " << endl;
    return;

  case SYSTEM:
    assert (xml->unitlevel==0);
    xml->unitlevel++;
    pos = Vector (0,0,0);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case REFLECTIVITY:
	xml->reflectivity=parse_float ((*iter).value);
	break;
      case BACKGROUND:
	xml->backgroundname=(*iter).value;
	break;
      case NEARSTARS:
	xml->numnearstars = parse_int((*iter).value);
	break;
      case STARS:
	xml->numstars = parse_int ((*iter).value);
	break;
      case STARSPREAD:
	xml->starsp = parse_float ((*iter).value);
	break;
      case FADESTARS:
	xml->fade = parse_bool ((*iter).value);
	break;
      case NAME:
	this->name = new char [strlen((*iter).value.c_str())+1];
	strcpy(this->name,(*iter).value.c_str());
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
      }

    }

    break;
  case ATMOSPHERE:
    {
      xml->unitlevel++;
      for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
	switch(attribute_map.lookup((*iter).name)) {
	default:
	  break;
	}
      }  
      Atmosphere * a = new Atmosphere();
      if (xml->unitlevel>2) {
	assert(xml->moons.size()!=0);
	Planet * p =xml->moons.back()->GetTopPlanet(xml->unitlevel-1);
	if (p)
	  p->setAtmosphere (a);
	else
	  fprintf (stderr,"atmosphere loose. no planet for it");
      } 
      
    }
    break;
  case TERRAIN:
  case CONTTERRAIN:
    xml->unitlevel++;
    S = Vector (0,1,0);
    R = Vector (0,0,1);
    pos = Vector (0,0,0);
    radius=-1;
    position=parse_float (vs_config->getVariable ("terrain","mass","100"));
    gravity=0;

    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	myfile = (*iter).value;
	break;
      case GRAVITY:
	gravity=parse_float((*iter).value);
	break;
      case MASS:
	position = parse_float ((*iter).value);
	break;
      case RADIUS:
	radius = parse_float ((*iter).value);
	break;
      case X:
	pos.i = parse_float ((*iter).value);
	break;
      case Y:
	pos.j = parse_float ((*iter).value);
	break;
      case Z:
	pos.k = parse_float ((*iter).value);
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
	S.i=parse_float((*iter).value);
	break;
      case QJ:
	S.j=parse_float((*iter).value);
	break;
      case QK:
	S.k=parse_float((*iter).value);
	break;
      }	
    }
    {
      Vector TerrainScale (XMLSupport::parse_float (vs_config->getVariable ("terrain","xscale","1")),XMLSupport::parse_float (vs_config->getVariable ("terrain","yscale","1")),XMLSupport::parse_float (vs_config->getVariable ("terrain","zscale","1")));
      Matrix t;
      Identity(t);
      float y =S.Magnitude();
      Normalize(S);
      float z =R.Magnitude();
      Normalize(R);
      TerrainScale.i*=z;
      TerrainScale.k*=z;
      TerrainScale.j*=y;      
      t[4]=S.i*TerrainScale.j;
      t[5]=S.j*TerrainScale.j;
      t[6]=S.k*TerrainScale.j;
      t[8]=R.i*TerrainScale.k;
      t[9]=R.j*TerrainScale.k;
      t[10]=R.k*TerrainScale.k;
      S = S.Cross (R);
      t[0]=S.i*TerrainScale.i;
      t[1]=S.j*TerrainScale.i;
      t[2]=S.k*TerrainScale.i;
      t[12]=pos.i+xml->systemcentroid.i;
      t[13]=pos.i+xml->systemcentroid.j;
      t[14]=pos.i+xml->systemcentroid.k;
      if (myfile.length()) {
	if (elem==TERRAIN) {
	  terrains.push_back (new Terrain (myfile.c_str(),TerrainScale,position,radius));
	  xml->parentterrain = terrains.back();
	  terrains.back()->SetTransformation(t);
	}else {
	  contterrains.push_back (new ContinuousTerrain (myfile.c_str(),TerrainScale,position));
	  xml->ct =contterrains.back();;
	  contterrains.back()->SetTransformation (t);
	  if (xml->unitlevel>2) {
	    assert(xml->moons.size()!=0);
	    Planet * p =xml->moons.back()->GetTopPlanet(xml->unitlevel-1);
	    if (p)
	      p->setTerrain (xml->ct);
	  } 
	}
      }
    }
    break;
  case LIGHT: 
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    xml->lights.push_back (GFXLight (true,Vector(0,0,0)));
    break;
  case ATTEN:
    tmptarg=ATTENUATE;
    goto addlightprop;
  case AMB:
    tmptarg=AMBIENT;
    goto addlightprop;
  case SPEC:
    tmptarg=SPECULAR;
    goto addlightprop;
  case DIFF:
    tmptarg=DIFFUSE;
    goto addlightprop;
  addlightprop:
    assert (xml->unitlevel==2);
    assert (xml->lights.size());
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case EMRED:
	tmpcol.r=parse_float ((*iter).value);
	break;
      case EMGREEN:
	tmpcol.g=parse_float ((*iter).value);
	break;
      case EMBLUE:
	tmpcol.b=parse_float ((*iter).value);
	break;
      case EMALPHA:
	tmpcol.a=parse_float ((*iter).value);
	break;
      }
    }
    xml->lights.back().SetProperties (tmptarg,tmpcol);
    break;
  case JUMP:
  case PLANET:
    assert (xml->unitlevel>0);
    xml->unitlevel++;
    S = Vector (0,1,0);
    R = Vector (0,0,1);
    filename = new char [1];
    filename[0]='\0';
    alpha=new char[1];
    alpha[0]='\0';
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	delete []filename;
	filename = new char [strlen((*iter).value.c_str())+1];
	strcpy(filename,(*iter).value.c_str());
	break;
      case DESTINATION:
	dest=ParseDestinations((*iter).value);
	break;
      case ALPHA:
	delete []alpha;
	alpha = new char [strlen((*iter).value.c_str())+1];
	strcpy(alpha,(*iter).value.c_str());
	break;
      case LIGHT:
	GetLights (xml->lights,curlights,(*iter).value.c_str());
		//assert (parse_int ((*iter).value)<xml->lights.size());
	//curlights.push_back (xml->lights[parse_int ((*iter).value)]);
	break;
      case FACTION:
	faction = _Universe->GetFaction ((*iter).value.c_str());
	break;
      case EMRED:
	ourmat.er = parse_float((*iter).value);
	break;
      case EMGREEN:
	ourmat.eg = parse_float((*iter).value);
	break;
      case EMBLUE:
	ourmat.eb = parse_float((*iter).value);
	break;
      case EMALPHA:
	ourmat.ea = parse_float((*iter).value);
      case RI:
	R.i=parse_float((*iter).value);
	break;
      case RJ:
	R.j=parse_float((*iter).value);
	break;
      case RK:
	R.k=parse_float((*iter).value);
	break;
      case SI:
	S.i=parse_float((*iter).value);
	break;
      case SJ:
	S.j=parse_float((*iter).value);
	break;
      case SK:
	S.k=parse_float((*iter).value);
	break;
      case X:
 	xml->cursun.i=parse_float((*iter).value);
 	break;

      case Y:
 	xml->cursun.j=parse_float((*iter).value);
 	break;

      case Z:
 	xml->cursun.k=parse_float((*iter).value);
 	break;

      case RADIUS:
	radius=parse_float((*iter).value);
	break;
      case PPOSITION:
	position=parse_float((*iter).value);
	break;
      case VELOCITY:
	velocity=parse_float((*iter).value);
	break;
      case GRAVITY:
	gravity=parse_float((*iter).value);
	break;
      }

    }
    if (alpha[0]==0) {
      delete [] alpha;
      alpha=NULL;
    }
    if (xml->unitlevel>2) {
      assert(xml->moons.size()!=0);
      xml->moons[xml->moons.size()-1]->beginElement(R,S,velocity,position,gravity,radius,filename,alpha,dest,xml->unitlevel-1, ourmat,curlights,false,faction);
    } else {
      xml->moons.push_back(new Planet(R,S,velocity,position,gravity,radius,filename,alpha,dest, xml->cursun+xml->systemcentroid, NULL, ourmat,curlights,faction));
      xml->moons[xml->moons.size()-1]->SetPosAndCumPos(R+S+xml->cursun+xml->systemcentroid);
    }
    delete []filename;
    break;
  case UNIT:
  case BUILDING:
  case VEHICLE:
    assert (xml->unitlevel>0);
    xml->unitlevel++;
    S = Vector (0,1,0);
    R = Vector (0,0,1);
    filename = new char [1];
    filename[0]='\0';
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	delete []filename;
	filename = new char [strlen((*iter).value.c_str())+1];
	strcpy(filename,(*iter).value.c_str());
	break;
      case FACTION:
	faction = _Universe->GetFaction ((*iter).value.c_str());
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
      case SI:
	S.i=parse_float((*iter).value);
	break;
      case SJ:
	S.j=parse_float((*iter).value);
	break;
      case SK:
	S.k=parse_float((*iter).value);
	break;
      case X:
 	xml->cursun.i=parse_float((*iter).value);

 	break;
      case Y:
 	xml->cursun.j=parse_float((*iter).value);

 	break;
      case Z:
 	xml->cursun.k=parse_float((*iter).value);

 	break;

      case PPOSITION:
	position=parse_float((*iter).value);
	break;
      case VELOCITY:
	velocity=parse_float((*iter).value);
	break;
      }

    }  
    if ((xml->parentterrain==NULL&&xml->ct==NULL)&&xml->unitlevel>((xml->parentterrain==NULL&&xml->ct==NULL)?2:3)) {
      assert(xml->moons.size()!=0);
      xml->moons[xml->moons.size()-1]->Planet::beginElement(R,S,velocity,position,gravity,radius,filename,NULL,vector <char *>(),xml->unitlevel-((xml->parentterrain==NULL&&xml->ct==NULL)?1:2),ourmat,curlights,true,faction);
    } else {
      if (xml->parentterrain!=NULL) {
	Unit * b = new Building (xml->parentterrain,elem==VEHICLE,filename,true,false,faction);
	b->SetPosAndCumPos (xml->cursun+xml->systemcentroid);
	b->EnqueueAI( new Orders::AggressiveAI ("default.agg.xml", "default.int.xml"));
	AddUnit (b);
      }else if (xml->ct!=NULL) {
	Unit * b=new Building (xml->ct,elem==VEHICLE,filename,true,false,faction);
	b->SetPosAndCumPos (xml->cursun+xml->systemcentroid);
	b->EnqueueAI( new Orders::AggressiveAI ("default.agg.xml", "default.int.xml"));
	AddUnit (b);
      }else {
	xml->moons.push_back((Planet *)new Unit(filename,true ,false,faction));
	xml->moons[xml->moons.size()-1]->SetAI(new PlanetaryOrbit(xml->moons[xml->moons.size()-1],velocity,position,R,S,xml->cursun+xml->systemcentroid, NULL));
	xml->moons[xml->moons.size()-1]->SetPosAndCumPos(R+S+xml->cursun+xml->systemcentroid);
      }
    }
    delete []filename;
    break;
	
  default:
	
    break;
  }
}

void StarSystem::endElement(const string &name) {
  Names elem = (Names)element_map.lookup(name);

  switch(elem) {
  case UNKNOWN:
    xml->unitlevel--;
    //    cerr << "Unknown element end tag '" << name << "' detected " << endl;
    break;
  case CONTTERRAIN:
    xml->unitlevel--;
    xml->ct = NULL;
    break;
  case TERRAIN:
    xml->parentterrain=NULL;
    xml->unitlevel--;
    break;
  default:
    xml->unitlevel--;
    break;
  }
  if (xml->unitlevel==0) {
    numprimaries = xml->moons.size();
    this->primaries=new Unit * [xml->moons.size()];
    for(unsigned int i=0;i<xml->moons.size();i++) {
			
      primaries[i]=xml->moons[i];
    }
  }
}


void StarSystem::LoadXML(const char *filename, const Vector & centroid) {
  //  shield.number=0;
  const int chunk_size = 16384;
  // rrestricted=yrestricted=prestricted=false;
  FILE * inFile = fopen (filename, "r");
  if(!inFile) {
    printf("StarSystem: file not found %s\n",filename);
    assert(0);
    return;
  }

  xml = new StarXML;
  xml->parentterrain=NULL;
  xml->ct=NULL;
  xml->systemcentroid=centroid;
  xml->fade = (vs_config->getVariable ("graphics","starblend","true")==string("true"));
  xml->starsp = 150;
  xml->numnearstars=400;
  xml->numstars=800;
  xml->backgroundname = string("cube");
  xml->reflectivity=.7;
  xml->unitlevel=0;
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &StarSystem::beginElement, &StarSystem::endElement);
  
  do {
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
    int length;
    
    length = fread (buf,1, chunk_size,inFile);
    //length = inFile.gcount();
    XML_ParseBuffer(parser, length, feof(inFile));
  } while(!feof(inFile));
  fclose (inFile);
  XML_ParserFree (parser);
#ifdef NV_CUBE_MAP
  LightMap[0]=new Texture ((xml->backgroundname+"_right_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_POSITIVE_X);
  LightMap[1]=new Texture ((xml->backgroundname+"_left_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_NEGATIVE_X);
  LightMap[2]=new Texture ((xml->backgroundname+"_up_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_POSITIVE_Y);
  LightMap[3]=new Texture ((xml->backgroundname+"_down_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_NEGATIVE_Y);
  LightMap[4]=new Texture ((xml->backgroundname+"_front_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_POSITIVE_Z);
  LightMap[5]=new Texture ((xml->backgroundname+"_back_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_NEGATIVE_Z);
#else
  FILE * tempo = fopen ((xml->backgroundname+"_light.bmp").c_str(),"rb");
  if (!tempo) {
    tempo = fopen (GetSharedTexturePath (xml->backgroundname+"_light.bmp").c_str(),"rb");
    if (!tempo) {
      EnvironmentMapGeneratorMain (xml->backgroundname.c_str(),(xml->backgroundname+"_light").c_str(), 0,xml->reflectivity,1);
    } else {
      fclose (tempo);
    }
  }else {
    fclose (tempo);
  }
  LightMap[0] = new Texture((xml->backgroundname+"_light.bmp").c_str(), 1);
#endif
  bg = new Background(xml->backgroundname.c_str(),xml->numstars,g_game.zfar*.9);
  stars = new Stars (xml->numnearstars, xml->starsp);
  stars->SetBlend (xml->fade,xml->fade);
  delete xml;
}

