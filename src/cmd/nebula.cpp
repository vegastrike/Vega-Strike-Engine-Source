#include "nebula.h"
#include "vegastrike.h"
#include "vs_path.h"
#include <assert.h>
#include "config_xml.h"
#include "vs_globals.h"

#include "xml_support.h"
#include "gfx/mesh.h"
namespace NebulaXML {
	FOGMODE parse_fogmode (string val) {
		if (val=="exp") {
			return FOG_EXP;
		} else if (val=="exp2") {
			return FOG_EXP2;
		} else if (val=="linear") {
			return FOG_LINEAR;
		} else {
			return FOG_OFF;
		}
	}
	enum Names {
		UNKNOWN,
		NEBULA,
		RED,
		GREEN,
		BLUE,
		COLOR,
		MODE,
		NEBNEAR,
		NEBFAR,
		DENSITY,
		LIMITS,
		INDEX,
		EXPLOSIONTIME,
		FOGTHIS
	};
	const unsigned short int MAXENAMES=4;
	const unsigned short int MAXANAMES=11;

	const EnumMap::Pair element_names[MAXENAMES] = {
		EnumMap::Pair ("UNKNOWN", UNKNOWN),
		EnumMap::Pair ("Nebula", NEBULA),
		EnumMap::Pair ("Color", COLOR),
		EnumMap::Pair ("Limits", LIMITS),

	};
	const EnumMap::Pair attribute_names[MAXANAMES] = {
		EnumMap::Pair ("UNKNOWN", UNKNOWN),
		EnumMap::Pair ("Red", RED),
		EnumMap::Pair ("Green", GREEN),
		EnumMap::Pair ("Blue", BLUE),
		EnumMap::Pair ("Near", NEBNEAR),
		EnumMap::Pair ("Far", NEBFAR),
		EnumMap::Pair ("Density", DENSITY),
		EnumMap::Pair ("Mode", MODE),
		EnumMap::Pair ("Index", INDEX),
		EnumMap::Pair ("ExplosionTime",EXPLOSIONTIME),
		EnumMap::Pair ("FogThis",FOGTHIS)
	};

	const EnumMap element_map(element_names, MAXENAMES);
	const EnumMap attribute_map(attribute_names, MAXANAMES);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace NebulaXML;

void Nebula::beginElement (void * Userdata,const XML_Char * name, const XML_Char ** atts) {
	((Nebula *)Userdata)->beginElem(std::string(name),AttributeList(atts));
}

static void Nebula_endElement (void * Userdata,const XML_Char *) {}


void Nebula::beginElem(const std::string& name, const AttributeList& atts) {
	Names elem = (Names)element_map.lookup(name);
	AttributeList::const_iterator iter;
	switch(elem) {
	case UNKNOWN:
		break;
	case NEBULA:
	  fogme=true;
	  explosiontime=0;
	  index=0;
	  fogmode=FOG_LINEAR;
		for(iter = atts.begin(); iter!=atts.end(); iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case DENSITY:
				Density = parse_float ((*iter).value);
				break;
			case INDEX: 
				index = parse_int ((*iter).value);
				break;
			case MODE: 
				fogmode = parse_fogmode ((*iter).value);
				break;
			case EXPLOSIONTIME:
			        explosiontime= parse_float ((*iter).value);
			        break;
			case FOGTHIS:
			  fogme = parse_bool ((*iter).value);
			  break;
			}
		}
		break;
	case COLOR:
		for(iter = atts.begin(); iter!=atts.end(); iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case RED:
				color.i = parse_float ((*iter).value);
				break;
			case GREEN: 
				color.j = parse_float ((*iter).value);
				break;
			case BLUE: 
				color.k = parse_float ((*iter).value);
				break;
			}
		}
		break;
	case LIMITS:
		for(iter = atts.begin(); iter!=atts.end(); iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case NEBNEAR:
				fognear = parse_float ((*iter).value);
				break;
			case NEBFAR: 
				fogfar = parse_float ((*iter).value);
				break;
			}
		}
		break;
	}
}

void Nebula::LoadXML(const char * filename) {
	const int chunk_size = 16384;
	FILE * inFile = fopen (filename, "r");
	static bool usefog= XMLSupport::parse_bool (vs_config->getVariable ("graphics","fog","true"));
        if(!inFile||!usefog) {
		if (inFile) {
		  fclose (inFile);
		}else {
		  fprintf(stderr,"\nUnit file %s not found\n",filename);
		}
		fogmode=FOG_OFF;
		return;
	}
	XML_Parser parser = XML_ParserCreate(NULL);
	XML_SetUserData(parser, this);
	XML_SetElementHandler(parser, &Nebula::beginElement, &Nebula_endElement);

	do {
#ifdef BIDBG
		char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
#else
		char buf[chunk_size];
#endif
		int length;
		length = fread (buf,1, chunk_size,inFile);
		//length = inFile.gcount();
		
#ifdef BIDBG
		XML_ParseBuffer(parser, length, feof(inFile));
#else
		XML_Parse(parser, buf,length, feof(inFile));
#endif
	} while(!feof(inFile));
	fclose (inFile);
	XML_ParserFree (parser);
}
void Nebula::SetFogState () {
  GFXFogMode (fogmode);
  GFXFogDensity (Density);
  GFXFogLimits (fognear,fogfar);
  GFXFogColor (GFXColor (color.i,color.j,color.k,1));
  GFXFogIndex (index);
  
}
Nebula::Nebula(const char * unitfile, bool SubU, int faction, Flightgroup* fg, int fg_snumber):
  Unit (unitfile,true,SubU,faction,fg,fg_snumber) {

  vssetdir (GetSharedUnitPath().c_str());
  vschdir (unitfile);
  string nebulafile (string(unitfile)+string(".nebula"));
  explosiontime=0;
  FILE *fp = fopen (nebulafile.c_str(),"r");
  bool uptwice=false;
  if (!fp) {
    uptwice=true;
    vscdup();
    const char *c;
    if ((c=_Universe->GetFaction(faction)))
      vschdir (c);
    else
      vschdir ("unknown");
    vschdir (unitfile);
    fp = fopen (nebulafile.c_str(),"r");
    if (fp) {
      fclose (fp);
    } else {
      vscdup();
      vscdup();
      vschdir ("neutral");
      vschdir (unitfile);
      faction=_Universe->GetFaction("neutral");
    }
  } else {
    fclose (fp);
  }
  LoadXML(nebulafile.c_str());
  vscdup();
  if (uptwice) 
    vscdup();
  vsresetdir();
}
void Nebula::reactToCollision(Unit * smaller, const Vector & biglocation, const Vector & bignormal, const Vector & smalllocation, const Vector & smallnormal, float dist){
  if (fogme)
    SetNebula(this);
  smaller->SetNebula(this);
}

void Nebula::UpdatePhysics (const Transformation &trans, const Matrix transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc) {
  Unit::UpdatePhysics (trans,transmat,CumulativeVelocity,ResolveLast,uc);
  Vector t1;
  float dis;
  unsigned int i;
  for (i=0;i<NUM_CAM;i++) {
    if (Inside (_Universe->AccessCamera(i)->GetPosition(),0,t1,dis)) {
      _Universe->AccessCamera(i)->SetNebula (this);
    }
  }

  if (nummesh) {
    i = rand()%nummesh;
    Vector randexpl (rand()%2*rSize()-rSize(),rand()%2*rSize()-rSize(),rand()%2*rSize()-rSize());
    if (((int)(explosiontime/SIMULATION_ATOM))!=0) 
      if (!(rand()%((int)(explosiontime/SIMULATION_ATOM)))) 
	meshdata[i]->AddDamageFX(randexpl,Vector (0,0,0),.00001,color);
  }
}
