#include "mesh.h"
#include "mesh_xml.h"
#include "aux_texture.h"
#include "aux_logo.h"
#include "vegastrike.h"
#include <iostream>
#include <fstream>
#include <expat.h>
#include <float.h>
#include <assert.h>
#include "ani_texture.h"
#if !defined(_WIN32) && !(defined(__APPLE__) || defined(MACOSX)) && !defined(BSD)
#include <values.h>
#endif
#include "xml_support.h"
#include "vec.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "cmd/script/mission.h"
#include "cmd/script/flightgroup.h"
#include "hashtable.h"

#ifdef max
#undef max
#endif
#include "gldrv/winsys.h"
static inline float max(float x, float y) {

  if(x>y) return x;
  else return y;

}



#ifdef min
#undef min
#endif

static inline float min(float x, float y) {
  if(x<y) return x;
  else return y;
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;
struct GFXMaterial;

const EnumMap::Pair MeshXML::element_names[] = {
  EnumMap::Pair("UNKNOWN", MeshXML::UNKNOWN),
  EnumMap::Pair("Material", MeshXML::MATERIAL),
  EnumMap::Pair("LOD", MeshXML::LOD),
  EnumMap::Pair("Ambient", MeshXML::AMBIENT),
  EnumMap::Pair("Diffuse", MeshXML::DIFFUSE),
  EnumMap::Pair("Specular", MeshXML::SPECULAR),
  EnumMap::Pair("Emissive", MeshXML::EMISSIVE),
  EnumMap::Pair("Mesh", MeshXML::MESH),
  EnumMap::Pair("Points", MeshXML::POINTS),
  EnumMap::Pair("Point", MeshXML::POINT),
  EnumMap::Pair("Location", MeshXML::LOCATION),
  EnumMap::Pair("Normal", MeshXML::NORMAL),
  EnumMap::Pair("Polygons", MeshXML::POLYGONS),
  EnumMap::Pair("Line", MeshXML::LINE),
  EnumMap::Pair("Tri", MeshXML::TRI),
  EnumMap::Pair("Quad", MeshXML::QUAD),
  EnumMap::Pair("Linestrip",MeshXML::LINESTRIP),
  EnumMap::Pair("Tristrip", MeshXML::TRISTRIP),
  EnumMap::Pair("Trifan", MeshXML::TRIFAN),
  EnumMap::Pair("Quadstrip", MeshXML::QUADSTRIP),
  EnumMap::Pair("Vertex", MeshXML::VERTEX),
  EnumMap::Pair("Logo", MeshXML::LOGO),
  EnumMap::Pair("Ref",MeshXML::REF),
  EnumMap::Pair("DetailPlane",MeshXML::DETAILPLANE)
};

const EnumMap::Pair MeshXML::attribute_names[] = {
  EnumMap::Pair("UNKNOWN", MeshXML::UNKNOWN),
  EnumMap::Pair("Scale",MeshXML::SCALE),
  EnumMap::Pair("Blend",MeshXML::BLENDMODE),
  EnumMap::Pair("texture", MeshXML::TEXTURE),
  EnumMap::Pair("alphamap", MeshXML::ALPHAMAP),
  EnumMap::Pair("sharevertex", MeshXML::SHAREVERT),
  EnumMap::Pair("red", MeshXML::RED),
  EnumMap::Pair("green", MeshXML::GREEN),
  EnumMap::Pair("blue", MeshXML::BLUE),
  EnumMap::Pair("alpha", MeshXML::ALPHA),
  EnumMap::Pair("power", MeshXML::POWER),
  EnumMap::Pair("reflect", MeshXML::REFLECT),
  EnumMap::Pair("x", MeshXML::X),
  EnumMap::Pair("y", MeshXML::Y),
  EnumMap::Pair("z", MeshXML::Z),
  EnumMap::Pair("i", MeshXML::I),
  EnumMap::Pair("j", MeshXML::J),
  EnumMap::Pair("k", MeshXML::K),
  EnumMap::Pair("Shade", MeshXML::FLATSHADE),
  EnumMap::Pair("point", MeshXML::POINT),
  EnumMap::Pair("s", MeshXML::S),
  EnumMap::Pair("t", MeshXML::T),
  //Logo stuffs
  EnumMap::Pair("Type",MeshXML::TYPE),
  EnumMap::Pair("Rotate", MeshXML::ROTATE),
  EnumMap::Pair("Weight", MeshXML::WEIGHT),
  EnumMap::Pair("Size", MeshXML::SIZE),
  EnumMap::Pair("Offset",MeshXML::OFFSET),
  EnumMap::Pair("meshfile",MeshXML::LODFILE),
  EnumMap::Pair ("Animation",MeshXML::ANIMATEDTEXTURE),
  EnumMap::Pair ("Reverse",MeshXML::REVERSE),
  EnumMap::Pair ("LightingOn",MeshXML::LIGHTINGON),
  EnumMap::Pair ("CullFace",MeshXML::CULLFACE),
  EnumMap::Pair ("ForceTexture",MeshXML::FORCETEXTURE),
  EnumMap::Pair ("UseNormals",MeshXML::USENORMALS),
  EnumMap::Pair ("PolygonOffset",MeshXML::POLYGONOFFSET),
  EnumMap::Pair ("DetailTexture",MeshXML::DETAILTEXTURE),
  EnumMap::Pair ("FramesPerSecond",MeshXML::FRAMESPERSECOND)
};



const EnumMap MeshXML::element_map(MeshXML::element_names, 24);
const EnumMap MeshXML::attribute_map(MeshXML::attribute_names, 37);



void Mesh::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  MeshXML * xml = (MeshXML*)userData;
  xml->mesh->beginElement(xml,name, AttributeList(atts));
}



void Mesh::endElement(void *userData, const XML_Char *name) {
  MeshXML * xml = (MeshXML*)userData;
  xml->mesh->endElement(xml,std::string(name));
}



enum BLENDFUNC parse_alpha (const char * tmp ) {
  if (strcmp (tmp,"ZERO")==0) {
    return ZERO;
  }

  if (strcmp (tmp,"ONE")==0) {
    return ONE;
  }

  if (strcmp (tmp,"SRCCOLOR")==0) {
    return SRCCOLOR;
  }

  if (strcmp (tmp,"INVSRCCOLOR")==0) {
    return INVSRCCOLOR;
  }

  if (strcmp (tmp,"SRCALPHA")==0) {
    return SRCALPHA;
  }

  if (strcmp (tmp,"INVSRCALPHA")==0) {
    return INVSRCALPHA;
  }

  if (strcmp (tmp,"DESTALPHA")==0) {
    return DESTALPHA;
  }

  if (strcmp (tmp,"INVDESTALPHA")==0) {
    return INVDESTALPHA;
  }

  if (strcmp (tmp,"DESTCOLOR")==0) {
    return DESTCOLOR;
  }

  if (strcmp (tmp,"INVDESTCOLOR")==0) {
    return INVDESTCOLOR;
  }

  if (strcmp (tmp,"SRCALPHASAT")==0) {
    return SRCALPHASAT;
  }

  if (strcmp (tmp,"CONSTALPHA")==0) {
    return CONSTALPHA;
  }

  if (strcmp (tmp,"INVCONSTALPHA")==0) {
    return INVCONSTALPHA;
  }

  if (strcmp (tmp,"CONSTCOLOR")==0) {
    return CONSTCOLOR;
  }

  if (strcmp (tmp,"INVCONSTCOLOR")==0) {
    return INVCONSTCOLOR;
  }

  return ZERO;
}





#if 0
std::string parse_alpha (enum BLENDMODE tmp ) {
  switch (tmp) {
  case ZERO: return "ZERO";
  case ONE: return "ONE";
  case SRCCOLOR: return "SRCCOLOR";
  case INVSRCCOLOR: return "INVSRCCOLOR";
  case SRCALPHA: return "SRCALPHA";
  case INVSRCALPHA: return "INVSRCALPHA";
  case DESTALPHA: return "DESTALPHA";
  case INVDESTALPHA: return "INVDESTALPHA";
  case DESTCOLOR: return "DESTCOLOR";
  case INVDESTCOLOR: return "INVDESTCOLOR";
  case SRCALPHASAT: return "SRCALPHASAT";
  case CONSTALPHA: return "CONSTALPHA";
  case INVCONSTALPHA: return "INVCONSTALPHA";
  case CONSTCOLOR: return "CONSTCOLOR";
  case INVCONSTCOLOR: return "INVCONSTCOLOR";
  }
  return ZERO;
}
#endif




/* Load stages:

0 - no tags seen
1 - waiting for points
2 - processing points 
3 - done processing points, waiting for face data

Note that this is only a debugging aid. Once DTD is written, there
will be no need for this sort of checking
 */


bool shouldreflect (string r) {
    if (strtoupper(r)=="FALSE")
		return false;
	int i;
	for (i=0;i<(int)r.length();++i) {
		if (r[i]!='0'&&r[i]!='.'&&r[i]!='+'&&r[i]!='e')
			return true;
	}
	return false;
}
void Mesh::beginElement(MeshXML * xml, const string &name, const AttributeList &attributes) {
	static bool use_detail_texture = XMLSupport::parse_bool(vs_config->getVariable("graphics","use_detail_texture","true"));
  //static bool flatshadeit=false;
  AttributeList::const_iterator iter;
  float flotsize=1;
  MeshXML::Names elem = (MeshXML::Names)MeshXML::element_map.lookup(name);
  MeshXML::Names top;
  if(xml->state_stack.size()>0) top = *xml->state_stack.rbegin();
  xml->state_stack.push_back(elem);
  bool texture_found = false;
  switch(elem) {
      case MeshXML::DETAILPLANE:
	  if (use_detail_texture) {
		  Vector vec (detailPlanes.size()>=2?1:0,detailPlanes.size()==1?1:0,detailPlanes.size()==0?1:0);
		  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		    switch(MeshXML::attribute_map.lookup((*iter).name)) {
			case MeshXML::X:
				vec.i=XMLSupport::parse_float(iter->value);
				break;
			case MeshXML::Y:
				vec.j=XMLSupport::parse_float(iter->value);
				break;
				
			case MeshXML::Z:
				vec.k=XMLSupport::parse_float(iter->value);
				break;
			}
		  }
		  static float detailscale = XMLSupport::parse_float(vs_config->getVariable("graphics","detail_texture_scale","1"));
		  if (detailPlanes.size()<6) {
			  detailPlanes.push_back(vec*detailscale);
		  }

	  }
	  break;
	  case MeshXML::MATERIAL:
	    //		  assert(xml->load_stage==4);
		  xml->load_stage=7;
		  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		    switch(MeshXML::attribute_map.lookup((*iter).name)) {
			case MeshXML::USENORMALS:
			  xml->usenormals = XMLSupport::parse_bool (iter->value);
			  break;
		    case MeshXML::POWER:
		      xml->material.power=XMLSupport::parse_float((*iter).value);
		      break;
		    case MeshXML::REFLECT:
		      setEnvMap ( shouldreflect((*iter).value));
		      break;
		    case MeshXML::LIGHTINGON:
		      setLighting (XMLSupport::parse_bool (vs_config->getVariable ("graphics","ForceLighting","true"))||XMLSupport::parse_bool((*iter).value)); 
		      break;
		    case MeshXML::CULLFACE:
		      forceCullFace (XMLSupport::parse_bool((*iter).value)); 
		      break;
		    }
		  }
		  break;
  case MeshXML::DIFFUSE:
	  //assert(xml->load_stage==7);
	  xml->load_stage=8;
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		  switch(MeshXML::attribute_map.lookup((*iter).name)) {
		  case MeshXML::RED:
			  xml->material.dr=XMLSupport::parse_float((*iter).value);
			  break;
		  case MeshXML::BLUE:
			  xml->material.db=XMLSupport::parse_float((*iter).value);
			  break;
		  case MeshXML::ALPHA:
			  xml->material.da=XMLSupport::parse_float((*iter).value);
			  break;
		  case MeshXML::GREEN:
			  xml->material.dg=XMLSupport::parse_float((*iter).value);
			  break;
		  }
	  }
	  break;
  case MeshXML::EMISSIVE:
	  //assert(xml->load_stage==7);
	  xml->load_stage=8;
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		  switch(MeshXML::attribute_map.lookup((*iter).name)) {
		  case MeshXML::RED:
			  xml->material.er=XMLSupport::parse_float((*iter).value);
			  break;
		  case MeshXML::BLUE:
			  xml->material.eb=XMLSupport::parse_float((*iter).value);
			  break;
		  case MeshXML::ALPHA:
			  xml->material.ea=XMLSupport::parse_float((*iter).value);
			  break;
		  case MeshXML::GREEN:
			  xml->material.eg=XMLSupport::parse_float((*iter).value);
			  break;
		  }
	  }
	  break;
  case MeshXML::SPECULAR:
	  //assert(xml->load_stage==7);
	  xml->load_stage=8;
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		  switch(MeshXML::attribute_map.lookup((*iter).name)) {
		  case MeshXML::RED:
			  xml->material.sr=XMLSupport::parse_float((*iter).value);
			  break;
		  case MeshXML::BLUE:
			  xml->material.sb=XMLSupport::parse_float((*iter).value);
			  break;
		  case MeshXML::ALPHA:
			  xml->material.sa=XMLSupport::parse_float((*iter).value);
			  break;
		  case MeshXML::GREEN:
			  xml->material.sg=XMLSupport::parse_float((*iter).value);
			  break;
		  }
	  }
	  break;
  case MeshXML::AMBIENT:
	  //assert(xml->load_stage==7);
	  xml->load_stage=8;
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		  switch(MeshXML::attribute_map.lookup((*iter).name)) {
		  case MeshXML::RED:
			  xml->material.ar=XMLSupport::parse_float((*iter).value);
			  break;
		  case MeshXML::BLUE:
			  xml->material.ab=XMLSupport::parse_float((*iter).value);
			  break;
		  case MeshXML::ALPHA:
			  xml->material.aa=XMLSupport::parse_float((*iter).value);
			  break;
		  case MeshXML::GREEN:
			  xml->material.ag=XMLSupport::parse_float((*iter).value);
			  break;
		  }
	  }
	  break;
  case MeshXML::UNKNOWN:
   VSFileSystem::vs_fprintf (stderr, "Unknown element start tag '%s' detected\n",name.c_str());
    break;
  case MeshXML::MESH:
    assert(xml->load_stage == 0);
    assert(xml->state_stack.size()==1);
    xml->load_stage = 1;
    // Read in texture attribute
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::REVERSE:
	xml->reverse = XMLSupport::parse_bool((*iter).value);
	break;
      case MeshXML::FORCETEXTURE:
	xml->force_texture=XMLSupport::parse_bool ((*iter).value);
	break;
      case MeshXML::SCALE:
	xml->scale *=  XMLSupport::parse_float ((*iter).value);
	break;
      case MeshXML::SHAREVERT:
	xml->sharevert = (XMLSupport::parse_bool ((*iter).value)&&XMLSupport::parse_bool (vs_config->getVariable ("graphics","SharedVertexArrays","true")));
	break;
	  case MeshXML::POLYGONOFFSET:
		  this->polygon_offset = XMLSupport::parse_float ((*iter).value);
		  break;
      case MeshXML::BLENDMODE:
	{
	  char *csrc=strdup ((*iter).value.c_str());
	  char *cdst=strdup((*iter).value.c_str());
	  sscanf (((*iter).value).c_str(),"%s %s",csrc,cdst);
	  SetBlendMode (parse_alpha (csrc),parse_alpha (cdst));
	  free (csrc);
	  free (cdst);
	}
	break;
	  case MeshXML::DETAILTEXTURE:
		  if (use_detail_texture)
			  detailTexture = TempGetTexture(xml, iter->value,FactionUtil::GetFaction(xml->faction),GFXTRUE);
		  break;
      case MeshXML::TEXTURE:
          //NO BREAK..goes to next statement
      case MeshXML::ALPHAMAP:
      case MeshXML::ANIMATEDTEXTURE:
      case MeshXML::UNKNOWN:
        {
          MeshXML::Names whichtype = MeshXML::UNKNOWN;
          int strsize=0;
          if (strtoupper(iter->name).find("ANIMATION")==0) {
              whichtype = MeshXML::ANIMATEDTEXTURE;
              strsize = strlen ("ANIMATION");
          }
          if (strtoupper(iter->name).find("TEXTURE")==0){
              whichtype= MeshXML::TEXTURE;
              strsize = strlen ("TEXTURE");
          }
          if (strtoupper(iter->name).find("ALPHAMAP")==0){
              whichtype=MeshXML::ALPHAMAP;
              strsize= strlen ("ALPHAMAP");
          }
          if (whichtype!=MeshXML::UNKNOWN) {
              unsigned int texindex =0;
              string ind(iter->name.substr (strsize));
              if (!ind.empty())
                  texindex=XMLSupport::parse_int(ind);
			  static bool per_pixel_lighting = XMLSupport::parse_bool (vs_config->getVariable ("graphics","per_pixel_lighting","true"));
			  if (texindex==0||per_pixel_lighting) {
              while (xml->decals.size()<=texindex)
                  xml->decals.push_back(MeshXML::ZeTexture());
              switch (whichtype) {
                  case MeshXML::ANIMATEDTEXTURE:
                      xml->decals[texindex].animated_name=iter->value;
                      break;
                  case MeshXML::ALPHAMAP:
                      xml->decals[texindex].alpha_name=iter->value;
                      break;
                  default:
                      xml->decals[texindex].decal_name=iter->value;
              }
			  }
              if (texindex==0) {
                  texture_found = true;                  
              }
          }
        }
        break;
      }
    }
    assert(texture_found);
    break;
  case MeshXML::POINTS:
    assert(top==MeshXML::MESH);
    //assert(xml->load_stage == 1);
    xml->load_stage = 2;
    break;
  case MeshXML::POINT:
    assert(top==MeshXML::POINTS);
    
    memset(&xml->vertex, 0, sizeof(xml->vertex));
    xml->point_state = 0; // Point state is used to check that all necessary attributes are recorded
    break;
  case MeshXML::LOCATION:
    assert(top==MeshXML::POINT);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	VSFileSystem::vs_fprintf (stderr, "Unknown attribute '%s' encountered in Location tag\n",(*iter).name.c_str());
	break;
      case MeshXML::X:
	assert(!(xml->point_state & MeshXML::P_X));
	xml->vertex.x = XMLSupport::parse_float((*iter).value);
	xml->vertex.i = 0;
	xml->point_state |= MeshXML::P_X;
	break;
      case MeshXML::Y:
	assert(!(xml->point_state & MeshXML::P_Y));
	xml->vertex.y = XMLSupport::parse_float((*iter).value);
	xml->vertex.j = 0;
	xml->point_state |= MeshXML::P_Y;
	break;
     case MeshXML::Z:
	assert(!(xml->point_state & MeshXML::P_Z));
	xml->vertex.z = XMLSupport::parse_float((*iter).value);
	xml->vertex.k = 0;
	xml->point_state |= MeshXML::P_Z;
	break;
      case MeshXML::S:
	xml->vertex.s = XMLSupport::parse_float ((*iter).value);
	break;
      case MeshXML::T:
	xml->vertex.t = XMLSupport::parse_float ((*iter).value);
	break;
      default:
	assert(0);
      }
    }
    assert(xml->point_state & (MeshXML::P_X |
			       MeshXML::P_Y |
			       MeshXML::P_Z) == 
	   (MeshXML::P_X |
	    MeshXML::P_Y |
	    MeshXML::P_Z) );
    break;
  case MeshXML::NORMAL:
    assert(top==MeshXML::POINT);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	VSFileSystem::vs_fprintf (stderr, "Unknown attribute '%s' encountered in Normal tag\n",(*iter).name.c_str());
	break;
      case MeshXML::I:
	assert(!(xml->point_state & MeshXML::P_I));
	xml->vertex.i = XMLSupport::parse_float((*iter).value);
	xml->point_state |= MeshXML::P_I;
	break;
      case MeshXML::J:
	assert(!(xml->point_state & MeshXML::P_J));
	xml->vertex.j = XMLSupport::parse_float((*iter).value);
	xml->point_state |= MeshXML::P_J;
	break;
      case MeshXML::K:
	assert(!(xml->point_state & MeshXML::P_K));
	xml->vertex.k = XMLSupport::parse_float((*iter).value);
	xml->point_state |= MeshXML::P_K;
	break;
      default:
	assert(0);
      }
    }
    if (xml->point_state & (MeshXML::P_I |
			       MeshXML::P_J |
			       MeshXML::P_K) != 
	   (MeshXML::P_I |
	    MeshXML::P_J |
	    MeshXML::P_K) ) {
      if (!xml->recalc_norm) {
	xml->vertex.i=xml->vertex.j=xml->vertex.k=0;
	xml->recalc_norm=true;
      }
    }
    break;
  case MeshXML::POLYGONS:
    assert(top==MeshXML::MESH);
    //assert(xml->load_stage==3);
    xml->load_stage = 4;
    break;
  case MeshXML::LINE:
    assert(top==MeshXML::POLYGONS);
    //assert(xml->load_stage==4);
    xml->num_vertices=2;
    xml->active_list = &xml->lines;
    xml->active_ind = &xml->lineind;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	VSFileSystem::vs_fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
	break;
      case MeshXML::FLATSHADE:
	if ((*iter).value=="Flat") {
	  VSFileSystem::vs_fprintf (stderr,"Cannot Flatshade Lines\n");
	}else {
	  if ((*iter).value=="Smooth") {
	    //ignored -- already done
	  }
	}
	break;
      default:
	assert (0);
      }
    }
    break;
  case MeshXML::TRI:
    assert(top==MeshXML::POLYGONS);
    //assert(xml->load_stage==4);
    xml->num_vertices=3;
    xml->active_list = &xml->tris;
    xml->active_ind = &xml->triind;
    xml->trishade.push_back (0);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	VSFileSystem::vs_fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
	break;
      case MeshXML::FLATSHADE:
	if ((*iter).value=="Flat") {
	  xml->trishade[xml->trishade.size()-1]=1;
	}else {
	  if ((*iter).value=="Smooth") {
	    xml->trishade[xml->trishade.size()-1]=0;
	  }
	}
	break;
      default:
	assert (0);
      }
    }
    break;
  case MeshXML::LINESTRIP:
    assert(top==MeshXML::POLYGONS);
    //assert(xml->load_stage==4);
    xml->num_vertices=2;
    xml->linestrips.push_back (vector<GFXVertex>());
    xml->active_list = &(xml->linestrips[xml->linestrips.size()-1]);
    xml->lstrcnt = xml->linestripind.size();
    xml->active_ind = &xml->linestripind;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	VSFileSystem::vs_fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
	break;
      case MeshXML::FLATSHADE:
	if ((*iter).value=="Flat") {
	  VSFileSystem::vs_fprintf(stderr,"Cannot Flatshade Linestrips\n");
	}else {
	  if ((*iter).value=="Smooth") {
	    //ignored -- already done
	  }
	}
	break;
      default:
	assert (0);
      }
    }
    break;

  case MeshXML::TRISTRIP:
    assert(top==MeshXML::POLYGONS);
    //assert(xml->load_stage==4);
    xml->num_vertices=3;//minimum number vertices
    xml->tristrips.push_back (vector<GFXVertex>());
    xml->active_list = &(xml->tristrips[xml->tristrips.size()-1]);
    xml->tstrcnt = xml->tristripind.size();
    xml->active_ind = &xml->tristripind;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	VSFileSystem::vs_fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
	break;
      case MeshXML::FLATSHADE:
	if ((*iter).value=="Flat") {
	  VSFileSystem::vs_fprintf(stderr,"Cannot Flatshade Tristrips\n");
	}else {
	  if ((*iter).value=="Smooth") {
	    //ignored -- already done
	  }
	}
	break;
      default:
	assert (0);
      }
    }
    break;

  case MeshXML::TRIFAN:
    assert(top==MeshXML::POLYGONS);
    //assert(xml->load_stage==4);
    xml->num_vertices=3;//minimum number vertices
    xml->trifans.push_back (vector<GFXVertex>());
    xml->active_list = &(xml->trifans[xml->trifans.size()-1]);
    xml->tfancnt = xml->trifanind.size();
    xml->active_ind = &xml->trifanind;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	VSFileSystem::vs_fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
	break;
      case MeshXML::FLATSHADE:
	if ((*iter).value=="Flat") {
	  VSFileSystem::vs_fprintf (stderr,"Cannot Flatshade Trifans\n");
	}else {
	  if ((*iter).value=="Smooth") {
	    //ignored -- already done
	  }
	}
	break;
      default:
	assert (0);
      }
    }
    break;

  case MeshXML::QUADSTRIP:
    assert(top==MeshXML::POLYGONS);
    //assert(xml->load_stage==4);
    xml->num_vertices=4;//minimum number vertices
    xml->quadstrips.push_back (vector<GFXVertex>());
    xml->active_list = &(xml->quadstrips[xml->quadstrips.size()-1]);
    xml->qstrcnt = xml->quadstripind.size();
    xml->active_ind = &xml->quadstripind;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	VSFileSystem::vs_fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
	break;
      case MeshXML::FLATSHADE:
	if ((*iter).value=="Flat") {
	  VSFileSystem::vs_fprintf (stderr, "Cannot Flatshade Quadstrips\n");
	}else {
	  if ((*iter).value=="Smooth") {
	    //ignored -- already done
	  }
	}
	break;
      default:
	assert (0);
      }
    }
    break;
   
  case MeshXML::QUAD:
    assert(top==MeshXML::POLYGONS);
    //assert(xml->load_stage==4);
    xml->num_vertices=4;
    xml->active_list = &xml->quads;
    xml->active_ind = &xml->quadind;
    xml->quadshade.push_back (0);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	VSFileSystem::vs_fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
	break;
      case MeshXML::FLATSHADE:
	if ((*iter).value=="Flat") {
	  xml->quadshade[xml->quadshade.size()-1]=1;
	}else {
	  if ((*iter).value=="Smooth") {
	    xml->quadshade[xml->quadshade.size()-1]=0;
	  }
	}
	break;
      default:
	assert(0);
      }
    }
    break;
  case MeshXML::LOD: 
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	break;
	  case MeshXML::FRAMESPERSECOND:
		  framespersecond=XMLSupport::parse_float((*iter).value);
		  break;
      case MeshXML::LODFILE:
	xml->lod.push_back(new Mesh ((*iter).value.c_str(),xml->lodscale,xml->faction,xml->fg,true));//make orig mesh
	break;
      case MeshXML::SIZE:
	flotsize = XMLSupport::parse_float ((*iter).value);
	break;
      }
    }
    if (xml->lodsize.size()!=xml->lod.size()) {
      xml->lodsize.push_back (flotsize);
    }
    break;
  case MeshXML::VERTEX:
    assert(top==MeshXML::TRI || top==MeshXML::QUAD || top==MeshXML::LINE ||top ==MeshXML::TRISTRIP || top ==MeshXML::TRIFAN||top ==MeshXML::QUADSTRIP || top==MeshXML::LINESTRIP);
    //assert(xml->load_stage==4);

    xml->vertex_state = 0;
    unsigned int index;
    float s, t;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	VSFileSystem::vs_fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
	break;
      case MeshXML::POINT:
	assert(!(xml->vertex_state & MeshXML::V_POINT));
	xml->vertex_state |= MeshXML::V_POINT;
	index = XMLSupport::parse_int((*iter).value);
	break;
      case MeshXML::S:
	assert(!(xml->vertex_state & MeshXML::V_S));
	xml->vertex_state |= MeshXML::V_S;
	s = XMLSupport::parse_float((*iter).value);
	break;
      case MeshXML::T:
	assert(!(xml->vertex_state & MeshXML::V_T));
	xml->vertex_state |= MeshXML::V_T;
	t = XMLSupport::parse_float((*iter).value);
	break;
      default:
	assert(0);
     }
    }
    assert(xml->vertex_state & (MeshXML::V_POINT|
				MeshXML::V_S|
				MeshXML::V_T) == 
	   (MeshXML::V_POINT|
	    MeshXML::V_S|
	    MeshXML::V_T) );
    assert(index < xml->vertices.size());

    memset(&xml->vertex, 0, sizeof(xml->vertex));
    xml->vertex = xml->vertices[index];
    xml->vertexcount[index]+=1;
    if ((!xml->vertex.i)&&(!xml->vertex.j)&&(!xml->vertex.k)) {
      if (!xml->recalc_norm) {
	
	xml->recalc_norm=true;
      }
    }
    //        xml->vertex.x*=scale;
    //        xml->vertex.y*=scale;
    //        xml->vertex.z*=scale;//FIXME
    xml->vertex.s = s;
    xml->vertex.t = t;
    xml->active_list->push_back(xml->vertex);
    xml->active_ind->push_back(index);
    if (xml->reverse) {
	  unsigned int i;
      for (i=xml->active_ind->size()-1;i>0;i--) {
	(*xml->active_ind)[i]=(*xml->active_ind)[i-1];
      }
      (*xml->active_ind)[0]=index;
      for ( i=xml->active_list->size()-1;i>0;i--) {
	(*xml->active_list)[i]=(*xml->active_list)[i-1];
      }
      (*xml->active_list)[0]=xml->vertex;
    }
    xml->num_vertices--;
    break;
  case MeshXML::LOGO: 
    assert (top==MeshXML::MESH);
    //assert (xml->load_stage==4);
    xml->load_stage=5;
    xml->vertex_state=0;
    unsigned int typ;
    float rot, siz,offset;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	VSFileSystem::vs_fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
	break;
      case MeshXML::TYPE:
	assert (!(xml->vertex_state&MeshXML::V_TYPE));
	xml->vertex_state|=MeshXML::V_TYPE;
	typ = XMLSupport::parse_int((*iter).value);
	
	break;
      case MeshXML::ROTATE:
	assert (!(xml->vertex_state&MeshXML::V_ROTATE));
	xml->vertex_state|=MeshXML::V_ROTATE;
	rot = XMLSupport::parse_float((*iter).value);

	break;
      case MeshXML::SIZE:
	assert (!(xml->vertex_state&MeshXML::V_SIZE));
	xml->vertex_state|=MeshXML::V_SIZE;
	siz = XMLSupport::parse_float((*iter).value);
	break;
      case MeshXML::OFFSET:
	assert (!(xml->vertex_state&MeshXML::V_OFFSET));
	xml->vertex_state|=MeshXML::V_OFFSET;
	offset = XMLSupport::parse_float ((*iter).value);
	break;
      default:
	assert(0);
     }
    }

    assert(xml->vertex_state & (MeshXML::V_TYPE|
				MeshXML::V_ROTATE|
				MeshXML::V_SIZE|
				MeshXML::V_OFFSET) == 
	   (MeshXML::V_TYPE|
	    MeshXML::V_ROTATE|
	    MeshXML::V_SIZE|
	    MeshXML::V_OFFSET) );
    xml->logos.push_back(MeshXML::ZeLogo());
    xml->logos[xml->logos.size()-1].type = typ;
    xml->logos[xml->logos.size()-1].rotate = rot;
    xml->logos[xml->logos.size()-1].size = siz;
    xml->logos[xml->logos.size()-1].offset = offset;
    break;
  case MeshXML::REF:
    {
    assert (top==MeshXML::LOGO);
    //assert (xml->load_stage==5);
    xml->load_stage=6;
    unsigned int ind=0;
    float indweight=1;
    bool foundindex=false;
    int ttttttt;
    ttttttt=0;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(MeshXML::attribute_map.lookup((*iter).name)) {
      case MeshXML::UNKNOWN:
	VSFileSystem::vs_fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
	break;
      case MeshXML::POINT:
	assert (ttttttt<2);
	xml->vertex_state |= MeshXML::V_POINT;
	ind = XMLSupport::parse_int((*iter).value);
	foundindex=true;
	ttttttt+=2;
	break;
      case MeshXML::WEIGHT:
	assert ((ttttttt&1)==0);
	ttttttt+=1;
	xml->vertex_state |= MeshXML::V_S;
	indweight = XMLSupport::parse_float((*iter).value);
	break;
      default:
	assert(0);
     }
    }
    assert (ttttttt==3);
    if (!foundindex) {
      VSFileSystem::vs_fprintf (stderr,"mesh with uninitalized logo");
    }
    xml->logos[xml->logos.size()-1].refpnt.push_back(ind);
    xml->logos[xml->logos.size()-1].refweight.push_back(indweight);
    xml->vertex_state+=MeshXML::V_REF;
    }
    break;
  default:
    assert(0);
  }
}

void Mesh::endElement(MeshXML* xml, const string &name) {
  //cerr << "End tag: " << name << endl;

  MeshXML::Names elem = (MeshXML::Names)MeshXML::element_map.lookup(name);
  assert(*xml->state_stack.rbegin() == elem);
  xml->state_stack.pop_back();
  unsigned int i;
  switch(elem) {
  case MeshXML::UNKNOWN:
    VSFileSystem::vs_fprintf (stderr,"Unknown element end tag '%s' detected\n",name.c_str());
    break;
  case MeshXML::POINT:
    assert(xml->point_state & (MeshXML::P_X | 
			       MeshXML::P_Y | 
			       MeshXML::P_Z |
			       MeshXML::P_I |
			       MeshXML::P_J |
			       MeshXML::P_K) == 
	   (MeshXML::P_X | 
	    MeshXML::P_Y | 
	    MeshXML::P_Z |
	    MeshXML::P_I |
	    MeshXML::P_J |
	    MeshXML::P_K) );
    xml->vertices.push_back(xml->vertex);
    xml->vertexcount.push_back(0);
    break;
  case MeshXML::POINTS:
    xml->load_stage = 3;

    /*
    cerr << xml->vertices.size() << " vertices\n";
    for(int a=0; a<xml->vertices.size(); a++) {
      clog << "Point: (" << xml->vertices[a].x << ", " << xml->vertices[a].y << ", " << xml->vertices[a].z << ") (" << xml->vertices[a].i << ", " << xml->vertices[a].j << ", " << xml->vertices[a].k << ")\n";
    }
    clog << endl;
    */
    break;
  case MeshXML::LINE:
    assert (xml->num_vertices==0);
    break;
  case MeshXML::TRI:
    assert(xml->num_vertices==0);
    break;
  case MeshXML::QUAD:
    assert(xml->num_vertices==0);
    break;
  case MeshXML::LINESTRIP:
    assert (xml->num_vertices<=0);
    for (i=xml->lstrcnt+1;i<xml->linestripind.size();i++) {
      xml->nrmllinstrip.push_back (xml->linestripind[i-1]);
      xml->nrmllinstrip.push_back (xml->linestripind[i]);
    }
    break;
  case MeshXML::TRISTRIP:
    assert(xml->num_vertices<=0);   
    for (i=xml->tstrcnt+2;i<xml->tristripind.size();i++) {
      if ((i-xml->tstrcnt)%2) {
	//normal order
	xml->nrmltristrip.push_back (xml->tristripind[i-2]);
	xml->nrmltristrip.push_back (xml->tristripind[i-1]);
	xml->nrmltristrip.push_back (xml->tristripind[i]);
      } else {
	//reverse order
	xml->nrmltristrip.push_back (xml->tristripind[i-1]);
	xml->nrmltristrip.push_back (xml->tristripind[i-2]);
	xml->nrmltristrip.push_back (xml->tristripind[i]);
      }
    }
    break;
  case MeshXML::TRIFAN:
    assert (xml->num_vertices<=0);
    for (i=xml->tfancnt+2;i<xml->trifanind.size();i++) {
      xml->nrmltrifan.push_back (xml->trifanind[xml->tfancnt]);
      xml->nrmltrifan.push_back (xml->trifanind[i-1]);
      xml->nrmltrifan.push_back (xml->trifanind[i]);
    }
    break;
  case MeshXML::QUADSTRIP://have to fix up nrmlquadstrip so that it 'looks' like a quad list for smooth shading
    assert(xml->num_vertices<=0);
    for (i=xml->qstrcnt+3;i<xml->quadstripind.size();i+=2) {
      xml->nrmlquadstrip.push_back (xml->quadstripind[i-3]);
      xml->nrmlquadstrip.push_back (xml->quadstripind[i-2]);
      xml->nrmlquadstrip.push_back (xml->quadstripind[i]);
      xml->nrmlquadstrip.push_back (xml->quadstripind[i-1]);
    }
    break;
  case MeshXML::POLYGONS:
    assert(xml->tris.size()%3==0);
    assert(xml->quads.size()%4==0);
    break;
  case MeshXML::REF:
    //assert (xml->load_stage==6);
    xml->load_stage=5;
    break;
  case MeshXML::LOGO:
    //assert (xml->load_stage==5);
    assert (xml->vertex_state>=MeshXML::V_REF*3);//make sure there are at least 3 reference points
    xml->load_stage=4;
    break;
  case MeshXML::MATERIAL:
	  //assert(xml->load_stage==7);
	  xml->load_stage=4;
	  break;
  case MeshXML::DETAILPLANE:
	  
	  break;
  case MeshXML::DIFFUSE:
	  //assert(xml->load_stage==8);
	  xml->load_stage=7;
	  break;
  case MeshXML::EMISSIVE:
	  //assert(xml->load_stage==8);
	  xml->load_stage=7;
	  break;
  case MeshXML::SPECULAR:
	  //assert(xml->load_stage==8);
	  xml->load_stage=7;
	  break;
  case MeshXML::AMBIENT:
	  //assert(xml->load_stage==8);
	  xml->load_stage=7;
	  break;
  case MeshXML::MESH:
    //assert(xml->load_stage==4);//4 is done with poly, 5 is done with Logos

    xml->load_stage=5;
    break;
  default:
    ;
  }
}



void SumNormals (int trimax, int t3vert, 
		 vector <GFXVertex> &vertices,
		 vector <int> &triind,
		 vector <int> &vertexcount,
		 bool * vertrw ) {
  int a=0;
  int i=0;
  int j=0;
  if (t3vert==2) {//oh man we have a line--what to do, what to do!
    for (i=0;i<trimax;i++,a+=t3vert) {
      Vector Cur (vertices[triind[a]].x-vertices[triind[a+1]].x,
		  vertices[triind[a]].y-vertices[triind[a+1]].y,
		  vertices[triind[a]].z-vertices[triind[a+1]].z);
      Normalize (Cur);
      vertices[triind[a+1]].i += Cur.i/vertexcount[triind[a+1]];
      vertices[triind[a+1]].j += Cur.j/vertexcount[triind[a+1]];
      vertices[triind[a+1]].k += Cur.k/vertexcount[triind[a+1]];

      vertices[triind[a]].i -= Cur.i/vertexcount[triind[a]];
      vertices[triind[a]].j -= Cur.j/vertexcount[triind[a]];
      vertices[triind[a]].k -= Cur.k/vertexcount[triind[a]];
    }
    return; //bye bye mrs american pie 
  }
  for (i=0;i<trimax;i++,a+=t3vert) {
    for (j=0;j<t3vert;j++) {
      if (vertrw[triind[a+j]]) {
	Vector Cur (vertices[triind[a+j]].x,
		    vertices[triind[a+j]].y,
		    vertices[triind[a+j]].z);
	Cur = (Vector (vertices[triind[a+((j+2)%t3vert)]].x,
		       vertices[triind[a+((j+2)%t3vert)]].y,
		       vertices[triind[a+((j+2)%t3vert)]].z)-Cur)
	  .Cross(Vector (vertices[triind[a+((j+1)%t3vert)]].x,
			 vertices[triind[a+((j+1)%t3vert)]].y,
			 vertices[triind[a+((j+1)%t3vert)]].z)-Cur);
	const float eps = .00001;
	if (fabs(Cur.i)>eps||fabs(Cur.j)>eps||fabs(Cur.k)<eps) {
	  Normalize(Cur);	 
	  //Cur = Cur*(1.00F/xml->vertexcount[a+j]);
	  vertices[triind[a+j]].i+=Cur.i;
	  vertices[triind[a+j]].j+=Cur.j;
	  vertices[triind[a+j]].k+=Cur.k;
	}else {
	  if (vertexcount[triind[a+j]]>1) {
	    vertexcount[triind[a+j]]--;
	  }
	}
      }
    }
  }
}
void updateMax (Vector &mn, Vector & mx, const GFXVertex &ver) {
    mn.i = min(ver.x, mn.i);
    mx.i = max(ver.x, mx.i);
    mn.j = min(ver.y, mn.j);
    mx.j = max(ver.y, mx.j);
    mn.k = min(ver.z, mn.k);
    mx.k = max(ver.z, mx.k);
}

using namespace VSFileSystem;

const bool USE_RECALC_NORM=true;
const bool FLAT_SHADE=true;
Mesh * Mesh::LoadMesh (const char * filename, const Vector & scale, int faction, Flightgroup * fg){
  vector<Mesh *> m = LoadMeshes(filename,scale,faction,fg);
  if (m.empty()) {
    return 0;
  }
  if (m.size()>1) {
    fprintf (stderr,"Mesh %s has %d subcomponents. Only first used!\n",filename,m.size());
    for (unsigned int i=1;i<m.size();++i) {
      delete m[i];
    }
  }
  return m[0];
}

Hashtable<std::string, std::vector <Mesh*>, 127> bfxmHashTable;
vector <Mesh*> Mesh::LoadMeshes(const char * filename, const Vector &scale, int faction, Flightgroup * fg) {
  /*
  if (strstr(filename,".xmesh")) {
    Mesh * m = new Mesh (filename,scale,faction,fg);
    vector <Mesh*> ret;
    ret.push_back(m);
    return ret;
    }*/
  string hash_name = VSFileSystem::GetHashName (filename,scale,faction);
  vector <Mesh *>* oldmesh = bfxmHashTable.Get(hash_name);
  if (oldmesh==0) {
    hash_name =VSFileSystem::GetSharedMeshHashName(filename,scale,faction);
    oldmesh = bfxmHashTable.Get(hash_name);
  }
  if (0!=oldmesh) {
    vector <Mesh *> ret;
    for (unsigned int i=0;i<oldmesh->size();++i) {
      ret.push_back(new Mesh());      
      Mesh* m = (*oldmesh)[i];
      ret.back()->LoadExistant(m->orig?m->orig:m);
    }
    return ret;
  }
    
  VSFile f;
  VSError err = f.OpenReadOnly( filename, MeshFile);
  if( err>Ok)
  {
	VSFileSystem::vs_fprintf (stderr,"Cannot Open Mesh File %s\n",filename);
	return vector<Mesh*>();
  }  
  char bfxm[4];
  f.Read(&bfxm[0],1);
  f.Read(&bfxm[1],1);
  f.Read(&bfxm[2],1);
  f.Read(&bfxm[3],1);
  if (bfxm[0]=='B'&&bfxm[1]=='F'&&bfxm[2]=='X'&&bfxm[3]=='M'){
    f.GoTo(0);
    hash_name =(err==VSFileSystem::Shared)?VSFileSystem::GetSharedMeshHashName (filename,scale,faction):VSFileSystem::GetHashName(filename,scale,faction);
    vector <Mesh*> retval (LoadMeshes(f,scale,faction,fg,hash_name));
    vector <Mesh*>* newvec = new vector<Mesh*>(retval);
    for (unsigned int i=0;i<retval.size();++i) {
      retval[i]->hash_name=hash_name;
      if (retval[i]->orig)
        retval[i]->orig->hash_name=hash_name;
      (*newvec)[i]=retval[i]->orig?retval[i]->orig:retval[i];
    }
    bfxmHashTable.Put(hash_name, newvec);
    return retval;
  }else {
    f.Close();
    Mesh * m = new Mesh (filename,scale,faction,fg);
    vector <Mesh*> ret;
    ret.push_back(m);
    return ret;    
  }
}
void Mesh::LoadXML(const char *filename,const Vector& scale, int faction, Flightgroup * fg, bool origthis) {
  VSFile f;
  VSError err = f.OpenReadOnly( filename, MeshFile);
  if( err>Ok)
  {
	VSFileSystem::vs_fprintf (stderr,"Cannot Open Mesh File %s\n",filename);
	cleanexit=1;
	winsys_exit(1);
	return;
  }
  LoadXML( f, scale, faction, fg, origthis);
  f.Close();
}

void Mesh::LoadXML( VSFileSystem::VSFile & f, const Vector & scale, int faction, Flightgroup *fg, bool origthis)
{
  const int chunk_size = 16384;
  std::vector <unsigned int> ind;

  MeshXML * xml = new MeshXML;
  xml->mesh = this;
  xml->fg = fg;
  xml->usenormals=false;
  xml->force_texture=false;
  xml->reverse=false;
  xml->sharevert=false;
  xml->faction = faction;
  GFXGetMaterial (0, xml->material);//by default it's the default material;
  xml->load_stage = 0;
  xml->recalc_norm=false;
  xml->scale=scale;
  xml->lodscale=scale;
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, xml);
  XML_SetElementHandler(parser, &Mesh::beginElement, &Mesh::endElement);

  XML_Parse (parser,(f.ReadFull()).c_str(),f.Size(),1);

  /*
  do {
#ifdef BIDBG
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
#else
    char buf[chunk_size];
#endif
    int length;
    
    length = VSFileSystem::vs_read(buf,1, chunk_size,inFile);
    //length = inFile.gcount();
#ifdef BIDBG
    XML_ParseBuffer(parser, length, VSFileSystem::vs_feof(inFile));
#else
    XML_Parse (parser,buf,length,VSFileSystem::vs_feof(inFile));
#endif
  } while(!VSFileSystem::vs_feof(inFile));
  */
  XML_ParserFree (parser);
  // Now, copy everything into the mesh data structures

  if(xml->load_stage!=5) {
    VSFileSystem::vs_fprintf (stderr,"Warning: mesh load possibly failed\n");
    exit(-1);
  }

  PostProcessLoading(xml);
  numlods=xml->lod.size()+1;
  if (origthis) {
    orig=NULL;
  }else {
    orig = new Mesh [numlods];
    unsigned int i;
    for (i=0;i<xml->lod.size();i++) {
      orig[i+1] = *xml->lod[i];
      orig[i+1].lodsize=xml->lodsize[i];
    } 
  }

  delete xml;
}
void Mesh::PostProcessLoading(MeshXML * xml) {
  unsigned int i; unsigned int a=0;
  unsigned int j;
  //begin vertex normal calculations if necessary
  if (!xml->usenormals/*USE_RECALC_NORM||xml->recalc_norm*/) {//fixme!


    bool *vertrw = new bool [xml->vertices.size()]; 
    for (i=0;i<xml->vertices.size();i++) {
      if (xml->vertices[i].i==0&&
	  xml->vertices[i].j==0&&
	  xml->vertices[i].k==0) {
	vertrw[i]=true;
      }else {
	vertrw[i]=USE_RECALC_NORM;
      }
    }
    SumNormals (xml->tris.size()/3,3,xml->vertices, xml->triind,xml->vertexcount, vertrw);
    SumNormals (xml->quads.size()/4,4,xml->vertices, xml->quadind,xml->vertexcount, vertrw);
    SumNormals (xml->lines.size()/2,2,xml->vertices,xml->lineind,xml->vertexcount,vertrw);
    SumNormals (xml->nrmltristrip.size()/3,3,xml->vertices, xml->nrmltristrip,xml->vertexcount, vertrw);
    SumNormals (xml->nrmltrifan.size()/3,3,xml->vertices, xml->nrmltrifan,xml->vertexcount, vertrw);
    SumNormals (xml->nrmlquadstrip.size()/4,4,xml->vertices, xml->nrmlquadstrip,xml->vertexcount, vertrw);
    SumNormals (xml->nrmllinstrip.size()/2,2,xml->vertices, xml->nrmllinstrip,xml->vertexcount, vertrw);
    delete []vertrw;
    for (i=0;i<xml->vertices.size();i++) {
      float dis = sqrtf (xml->vertices[i].i*xml->vertices[i].i +
			 xml->vertices[i].j*xml->vertices[i].j +
			 xml->vertices[i].k*xml->vertices[i].k);
      if (dis!=0) {
	xml->vertices[i].i/=dis;//renormalize
	xml->vertices[i].j/=dis;
	xml->vertices[i].k/=dis;
	/*	VSFileSystem::vs_fprintf (stderr, "Vertex %d, (%f,%f,%f) <%f,%f,%f>\n",i,
		 xml->vertices[i].x,
		 xml->vertices[i].y,
		 xml->vertices[i].z,
		 xml->vertices[i].i,
		 xml->vertices[i].j,
		 xml->vertices[i].k);*/
      }else {
	xml->vertices[i].i=xml->vertices[i].x;
	xml->vertices[i].j=xml->vertices[i].y;
	xml->vertices[i].k=xml->vertices[i].z;
	dis = sqrtf (xml->vertices[i].i*xml->vertices[i].i +
		     xml->vertices[i].j*xml->vertices[i].j +
		     xml->vertices[i].k*xml->vertices[i].k);
	if (dis!=0) {
	  xml->vertices[i].i/=dis;//renormalize
	  xml->vertices[i].j/=dis;
	  xml->vertices[i].k/=dis;	  
	}else {
	  xml->vertices[i].i=0;
	  xml->vertices[i].j=0;
	  xml->vertices[i].k=1;	  
	}
      } 
    }
  }
    a=0;
  std::vector <unsigned int> ind;
    for (a=0;a<xml->tris.size();a+=3) {
      for (j=0;j<3;j++) {
	ind.push_back (xml->triind[a+j]);
	xml->tris[a+j].i = xml->vertices[xml->triind[a+j]].i;
	xml->tris[a+j].j = xml->vertices[xml->triind[a+j]].j;
	xml->tris[a+j].k = xml->vertices[xml->triind[a+j]].k;
      }
    }
    a=0;
    for (a=0;a<xml->quads.size();a+=4) {
      for (j=0;j<4;j++) {
	ind.push_back (xml->quadind[a+j]);
	xml->quads[a+j].i=xml->vertices[xml->quadind[a+j]].i;
	xml->quads[a+j].j=xml->vertices[xml->quadind[a+j]].j;
	xml->quads[a+j].k=xml->vertices[xml->quadind[a+j]].k;
      }
    }
    a=0;
    for (a=0;a<xml->lines.size();a+=2) {
      for (j=0;j<2;j++) {
	ind.push_back (xml->lineind[a+j]);
	xml->lines[a+j].i=xml->vertices[xml->lineind[a+j]].i;
	xml->lines[a+j].j=xml->vertices[xml->lineind[a+j]].j;
	xml->lines[a+j].k=xml->vertices[xml->lineind[a+j]].k;
      }
    }


    a=0;
    unsigned int k=0;
    unsigned int l=0;
    for (l=a=0;a<xml->tristrips.size();a++) {
      for (k=0;k<xml->tristrips[a].size();k++,l++) {
	ind.push_back (xml->tristripind[l]);
	xml->tristrips[a][k].i = xml->vertices[xml->tristripind[l]].i;
	xml->tristrips[a][k].j = xml->vertices[xml->tristripind[l]].j;
	xml->tristrips[a][k].k = xml->vertices[xml->tristripind[l]].k;
      }
    }
    for (l=a=0;a<xml->trifans.size();a++) {
      for (k=0;k<xml->trifans[a].size();k++,l++) {
	ind.push_back (xml->trifanind[l]);
	xml->trifans[a][k].i = xml->vertices[xml->trifanind[l]].i;
	xml->trifans[a][k].j = xml->vertices[xml->trifanind[l]].j;
	xml->trifans[a][k].k = xml->vertices[xml->trifanind[l]].k;
      }
    }
    for (l=a=0;a<xml->quadstrips.size();a++) {
      for (k=0;k<xml->quadstrips[a].size();k++,l++) {
	ind.push_back (xml->quadstripind[l]);
	xml->quadstrips[a][k].i = xml->vertices[xml->quadstripind[l]].i;
	xml->quadstrips[a][k].j = xml->vertices[xml->quadstripind[l]].j;
	xml->quadstrips[a][k].k = xml->vertices[xml->quadstripind[l]].k;
      }
    }
    for (l=a=0;a<xml->linestrips.size();a++) {
      for (k=0;k<xml->linestrips[a].size();k++,l++) {
	ind.push_back (xml->linestripind[l]);
	xml->linestrips[a][k].i = xml->vertices[xml->linestripind[l]].i;
	xml->linestrips[a][k].j = xml->vertices[xml->linestripind[l]].j;
	xml->linestrips[a][k].k = xml->vertices[xml->linestripind[l]].k;
      }
    }

  
  
  // TODO: add alpha handling

   //check for per-polygon flat shading
  unsigned int trimax = xml->tris.size()/3;
  a=0;
  i=0;
  j=0;
  if (!xml->usenormals) {
      for (i=0;i<trimax;i++,a+=3) {
          if (FLAT_SHADE||xml->trishade[i]==1) {
              for (j=0;j<3;j++) {
                  Vector Cur (xml->vertices[xml->triind[a+j]].x,
                              xml->vertices[xml->triind[a+j]].y,
                              xml->vertices[xml->triind[a+j]].z);
                  Cur = (Vector (xml->vertices[xml->triind[a+((j+2)%3)]].x,
                                 xml->vertices[xml->triind[a+((j+2)%3)]].y,
                                 xml->vertices[xml->triind[a+((j+2)%3)]].z)-Cur)
                      .Cross(Vector (xml->vertices[xml->triind[a+((j+1)%3)]].x,
                                     xml->vertices[xml->triind[a+((j+1)%3)]].y,
                                     xml->vertices[xml->triind[a+((j+1)%3)]].z)-Cur);
                  Normalize(Cur);
                  //Cur = Cur*(1.00F/xml->vertexcount[a+j]);
                  xml->tris[a+j].i=Cur.i;
                  xml->tris[a+j].j=Cur.j;
                  xml->tris[a+j].k=Cur.k;
#if 0
                  xml->tris[a+j].i=Cur.i/xml->vertexcount[xml->triind[a+j]];
                  xml->tris[a+j].j=Cur.j/xml->vertexcount[xml->triind[a+j]];
                  xml->tris[a+j].k=Cur.k/xml->vertexcount[xml->triind[a+j]];
#endif
              }
          }
      }
      a=0;
      trimax = xml->quads.size()/4;
      for (i=0;i<trimax;i++,a+=4) {
          if (xml->quadshade[i]==1||(FLAT_SHADE)) {
              for (j=0;j<4;j++) {
                  Vector Cur (xml->vertices[xml->quadind[a+j]].x,
                              xml->vertices[xml->quadind[a+j]].y,
                              xml->vertices[xml->quadind[a+j]].z);
                  Cur = (Vector (xml->vertices[xml->quadind[a+((j+2)%4)]].x,
                                 xml->vertices[xml->quadind[a+((j+2)%4)]].y,
                                 xml->vertices[xml->quadind[a+((j+2)%4)]].z)-Cur)
                      .Cross(Vector (xml->vertices[xml->quadind[a+((j+1)%4)]].x,
                                     xml->vertices[xml->quadind[a+((j+1)%4)]].y,
                                     xml->vertices[xml->quadind[a+((j+1)%4)]].z)-Cur);
                  Normalize(Cur);
                  //Cur = Cur*(1.00F/xml->vertexcount[a+j]);
                  xml->quads[a+j].i=Cur.i;//xml->vertexcount[xml->quadind[a+j]];
                  xml->quads[a+j].j=Cur.j;//xml->vertexcount[xml->quadind[a+j]];
                  xml->quads[a+j].k=Cur.k;//xml->vertexcount[xml->quadind[a+j]];
                  
#if 0
                  xml->quads[a+j].i=Cur.i/xml->vertexcount[xml->quadind[a+j]];
                  xml->quads[a+j].j=Cur.j/xml->vertexcount[xml->quadind[a+j]];
                  xml->quads[a+j].k=Cur.k/xml->vertexcount[xml->quadind[a+j]];
#endif
              }
          }
      }
      
  }
  string factionname = FactionUtil::GetFaction(xml->faction);
  while (Decal.size()<xml->decals.size())
      Decal.push_back(NULL);
  Decal[0]=(TempGetTexture(xml, 0,factionname));
  {for (unsigned int i=1;i<xml->decals.size();i++) {
      Decal[i]=(TempGetTexture(xml, i,factionname));
  }}
  while (Decal.back()==NULL&&Decal.size()>1) {
      Decal.pop_back();
  }

  unsigned int index = 0;

  unsigned int totalvertexsize = xml->tris.size()+xml->quads.size()+xml->lines.size();
  for (index=0;index<xml->tristrips.size();index++) {
    totalvertexsize += xml->tristrips[index].size();
  }
  for (index=0;index<xml->trifans.size();index++) {
    totalvertexsize += xml->trifans[index].size();
  }
  for (index=0;index<xml->quadstrips.size();index++) {
    totalvertexsize += xml->quadstrips[index].size();
  }
  for (index=0;index<xml->linestrips.size();index++) {
    totalvertexsize += xml->linestrips[index].size();
  }

  index =0;
  GFXVertex *vertexlist = new GFXVertex[totalvertexsize];

  mn = Vector (FLT_MAX,FLT_MAX,FLT_MAX);
  mx = Vector (-FLT_MAX,-FLT_MAX,-FLT_MAX);
  radialSize = 0;
  vector <enum POLYTYPE> polytypes;
  polytypes.insert(polytypes.begin(),totalvertexsize,GFXTRI);
  //  enum POLYTYPE * polytypes= new enum POLYTYPE[totalvertexsize];//overkill but what the hell
  vector <int> poly_offsets;
  poly_offsets.insert (poly_offsets.begin(),totalvertexsize,0);
  int o_index=0;
  if (xml->tris.size()) {
    polytypes[o_index]= GFXTRI;
    poly_offsets[o_index]=xml->tris.size();
    o_index++;
  }
  if (xml->quads.size()) {
    polytypes[o_index]=GFXQUAD;
    poly_offsets[o_index]=xml->quads.size();
    o_index++;
  }
  if (xml->lines.size()) {
    polytypes[o_index]=GFXLINE;
    poly_offsets[o_index]=xml->lines.size();
    o_index++;
  }
  /*
  if (xml->lines.size())
    polytypes[o_index]=GFXLINE;
    poly_offsets[o_index]=xml->lines.size()*2;  
    o_index++;
  */

  for(a=0; a<xml->tris.size(); a++, index++) {
    vertexlist[index] = xml->tris[a];		
    updateMax (mn,mx,vertexlist[index]);
  }
  for(a=0; a<xml->quads.size(); a++, index++) {
    vertexlist[index] = xml->quads[a];
    updateMax (mn,mx,vertexlist[index]);
  }
  for(a=0; a<xml->lines.size(); a++, index++) {
    vertexlist[index] = xml->lines[a];		
    updateMax (mn,mx,vertexlist[index]);
  }

  for (a=0;a<xml->tristrips.size();a++) {

    for (unsigned int m=0;m<xml->tristrips[a].size();m++,index++) {
      vertexlist[index] = xml->tristrips[a][m];
    updateMax (mn,mx,vertexlist[index]);
    }
    polytypes[o_index]= GFXTRISTRIP;
    poly_offsets[o_index]=xml->tristrips[a].size();
    o_index++;
  }
  for (a=0;a<xml->trifans.size();a++) {
    for (unsigned int m=0;m<xml->trifans[a].size();m++,index++) {
      vertexlist[index] = xml->trifans[a][m];
    updateMax (mn,mx,vertexlist[index]);
    }
    polytypes[o_index]= GFXTRIFAN;
    poly_offsets[o_index]=xml->trifans[a].size();

    o_index++;
  }
  for (a=0;a<xml->quadstrips.size();a++) {
    for (unsigned int m=0;m<xml->quadstrips[a].size();m++,index++) {
      vertexlist[index] = xml->quadstrips[a][m];
    updateMax (mn,mx,vertexlist[index]);
    }
    polytypes[o_index]= GFXQUADSTRIP;
    poly_offsets[o_index]=xml->quadstrips[a].size();
    o_index++;
  }
  for (a=0;a<xml->linestrips.size();a++) {

    for (unsigned int m=0;m<xml->linestrips[a].size();m++,index++) {
      vertexlist[index] = xml->linestrips[a][m];
      updateMax (mn,mx,vertexlist[index]);
    }
    polytypes[o_index]= GFXLINESTRIP;
    poly_offsets[o_index]=xml->linestrips[a].size();
    o_index++;
  }
  if (mn.i==FLT_MAX&&mn.j==FLT_MAX&&mn.k==FLT_MAX) {
	  mx.i=mx.j=mx.k=mn.i=mn.j=mn.k=0;
  }
  mn.i *=xml->scale.i;
  mn.j *=xml->scale.j;
  mn.k *=xml->scale.k;
  mx.i *=xml->scale.i;
  mx.j *=xml->scale.j;
  mx.k *=xml->scale.k;  
  float x_center = (mn.i + mx.i)/2.0,
    y_center = (mn.j + mx.j)/2.0,
    z_center = (mn.k + mx.k)/2.0;
  local_pos = Vector (x_center, y_center, z_center);
  for(a=0; a<totalvertexsize; a++) {
    vertexlist[a].x*=xml->scale.i;//FIXME
    vertexlist[a].y*=xml->scale.j;
    vertexlist[a].z*=xml->scale.k;
    vertexlist[a].i*=-1;
    vertexlist[a].j*=-1;
    vertexlist[a].k*=-1;

    /*
    vertexlist[a].x -= x_center;
    vertexlist[a].y -= y_center;
    vertexlist[a].z -= z_center;
    */
  }
  for (a=0;a<xml->vertices.size();a++) {
    xml->vertices[a].x*=xml->scale.i;//FIXME
    xml->vertices[a].y*=xml->scale.j;
    xml->vertices[a].z*=xml->scale.k;
    xml->vertices[a].i*=-1;    xml->vertices[a].k*=-1;    xml->vertices[a].j*=-1;
    /*
    xml->vertices[a].x -= x_center;
    xml->vertices[a].y -= y_center;
    xml->vertices[a].z -= z_center; //BSP generation and logos require the vertices NOT be centered!
    */
  }
  /*** NOW MIN/MAX size should NOT be centered for fast bounding queries
  minSizeX -= x_center;
  maxSizeX -= x_center;
  minSizeY -= y_center;
  maxSizeY -= y_center;
  minSizeZ -= z_center;
  maxSizeZ -= z_center;
  ***/
  if( o_index || index)
 	 radialSize = .5*(mx-mn).Magnitude();

  if (xml->sharevert) {
    vlist = new GFXVertexList (&polytypes[0], xml->vertices.size(),&xml->vertices[0],o_index,&poly_offsets[0],false,&ind[0]);
  }else {
    static bool usopttmp=(XMLSupport::parse_bool (vs_config->getVariable ("graphics","OptimizeVertexArrays","false")));
    static float optvertexlimit= (XMLSupport::parse_float (vs_config->getVariable ("graphics", "OptimizeVertexCondition","1.0")));
    bool cachunk=false;
    if (usopttmp) {
      int numopt =totalvertexsize;      
      GFXVertex * newv;
      unsigned int * ind;
      GFXOptimizeList (vertexlist,totalvertexsize,&newv,&numopt,&ind);
      if (numopt < totalvertexsize*optvertexlimit) {
	vlist = new GFXVertexList (&polytypes[0], numopt,newv,o_index,&poly_offsets[0],false,ind);
	cachunk = true;
      }
      free (ind);
      free (newv);
    }
    if (!cachunk) {
      vlist= new GFXVertexList(&polytypes[0],totalvertexsize,vertexlist,o_index,&poly_offsets[0]); 
    }
  }
  /*
  vlist[GFXQUAD]= new GFXVertexList(GFXQUAD,xml->quads.size(),vertexlist+xml->tris.size());
  index = xml->tris.size()+xml->quads.size();
  numQuadstrips = xml->tristrips.size()+xml->trifans.size()+xml->quadstrips.size();
  quadstrips = new GFXVertexList* [numQuadstrips];
  unsigned int tmpind =0;
  for (a=0;a<xml->tristrips.size();a++,tmpind++) {
    quadstrips[tmpind]= new GFXVertexList (GFXTRISTRIP,xml->tristrips[a].size(),vertexlist+index);
    index+= xml->tristrips[a].size();
  }
  for (a=0;a<xml->trifans.size();a++,tmpind++) {
    quadstrips[tmpind]= new GFXVertexList (GFXTRIFAN,xml->trifans[a].size(),vertexlist+index);
    index+= xml->trifans[a].size();
  }
  for (a=0;a<xml->quadstrips.size();a++,tmpind++) {
    quadstrips[tmpind]= new GFXVertexList (GFXQUADSTRIP,xml->quadstrips[a].size(),vertexlist+index);
    index+= xml->quadstrips[a].size();
  }
  */
  CreateLogos(xml,xml->faction,xml->fg);
  // Calculate bounding sphere
  
  if (mn.i==FLT_MAX) {
    mn=Vector (0,0,0);
    mx=Vector (0,0,0);
  }

  GFXSetMaterial (myMatNum,xml->material);

  delete [] vertexlist;

}

