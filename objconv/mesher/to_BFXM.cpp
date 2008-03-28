#include "mesh_io.h"
#include "to_BFXM.h"
#include <cstring>

//#define fprintf aprintf
int32bit aprintf(...) {
   return 0;
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;

const EnumMap::Pair XML::element_names[] = {
  EnumMap::Pair("UNKNOWN", XML::UNKNOWN),
  EnumMap::Pair("Material", XML::MATERIAL),
  EnumMap::Pair("LOD", XML::LOD),
  EnumMap::Pair("Ambient", XML::AMBIENT),
  EnumMap::Pair("Diffuse", XML::DIFFUSE),
  EnumMap::Pair("Specular", XML::SPECULAR),
  EnumMap::Pair("Emissive", XML::EMISSIVE),
  EnumMap::Pair("Mesh", XML::MESH),
  EnumMap::Pair("Points", XML::POINTS),
  EnumMap::Pair("Point", XML::POINT),
  EnumMap::Pair("Location", XML::LOCATION),
  EnumMap::Pair("Normal", XML::NORMAL),
  EnumMap::Pair("Polygons", XML::POLYGONS),
  EnumMap::Pair("Line", XML::LINE),
  EnumMap::Pair("Tri", XML::TRI),
  EnumMap::Pair("Quad", XML::QUAD),
  EnumMap::Pair("Linestrip",XML::LINESTRIP),
  EnumMap::Pair("Tristrip", XML::TRISTRIP),
  EnumMap::Pair("Trifan", XML::TRIFAN),
  EnumMap::Pair("Quadstrip", XML::QUADSTRIP),
  EnumMap::Pair("Vertex", XML::VERTEX),
  EnumMap::Pair("Logo", XML::LOGO),
  EnumMap::Pair("Ref",XML::REF),
  EnumMap::Pair("DetailPlane",XML::DETAILPLANE),
  EnumMap::Pair("AnimationDefinition",XML::ANIMDEF),
  EnumMap::Pair("Frame",XML::ANIMFRAME),
  EnumMap::Pair ("AnimationFrameIndex",XML::ANIMATIONFRAMEINDEX)
};

const EnumMap::Pair XML::attribute_names[] = {
  EnumMap::Pair("UNKNOWN", XML::UNKNOWN),
  EnumMap::Pair("Scale",XML::SCALE),
  EnumMap::Pair("Blend",XML::BLENDMODE),
  EnumMap::Pair("texture", XML::TEXTURE),
  EnumMap::Pair("alphamap", XML::ALPHAMAP),
  EnumMap::Pair("sharevertex", XML::SHAREVERT),
  EnumMap::Pair("red", XML::RED),
  EnumMap::Pair("green", XML::GREEN),
  EnumMap::Pair("blue", XML::BLUE),
  EnumMap::Pair("alpha", XML::ALPHA),
  EnumMap::Pair("power", XML::POWER),
  EnumMap::Pair("reflect", XML::REFLECT),
  EnumMap::Pair("x", XML::X),
  EnumMap::Pair("y", XML::Y),
  EnumMap::Pair("z", XML::Z),
  EnumMap::Pair("i", XML::I),
  EnumMap::Pair("j", XML::J),
  EnumMap::Pair("k", XML::K),
  EnumMap::Pair("Shade", XML::FLATSHADE),
  EnumMap::Pair("point", XML::POINT),
  EnumMap::Pair("s", XML::S),
  EnumMap::Pair("t", XML::T),
  //Logo stuffs
  EnumMap::Pair("Type",XML::TYPE),
  EnumMap::Pair("Rotate", XML::ROTATE),
  EnumMap::Pair("Weight", XML::WEIGHT),
  EnumMap::Pair("Size", XML::SIZE),
  EnumMap::Pair("Offset",XML::OFFSET),
  EnumMap::Pair("meshfile",XML::LODFILE),
  EnumMap::Pair ("Animation",XML::ANIMATEDTEXTURE),
  EnumMap::Pair ("Reverse",XML::REVERSE),
  EnumMap::Pair ("LightingOn",XML::LIGHTINGON),
  EnumMap::Pair ("CullFace",XML::CULLFACE),
  EnumMap::Pair ("ForceTexture",XML::FORCETEXTURE),
  EnumMap::Pair ("UseNormals",XML::USENORMALS),
  EnumMap::Pair ("PolygonOffset",XML::POLYGONOFFSET),
  EnumMap::Pair ("DetailTexture",XML::DETAILTEXTURE),
  EnumMap::Pair ("FramesPerSecond",XML::FRAMESPERSECOND),
  EnumMap::Pair ("FrameMeshName",XML::FRAMEMESHNAME),
  EnumMap::Pair ("AnimationName",XML::ANIMATIONNAME),
  EnumMap::Pair ("AnimationMeshIndex",XML::ANIMATIONMESHINDEX),
  EnumMap::Pair("alphatest",XML::ALPHATEST)
};


const EnumMap XML::element_map(XML::element_names, 27);
const EnumMap XML::attribute_map(XML::attribute_names, 41);


void CopyNormal (GFXVertex &outp,
                const GFXVertex &inp) {
  outp.i=inp.i;
  outp.j=inp.j;
  outp.k=inp.k;
}
void AddNormal (GFXVertex &outp,
                const GFXVertex &inp) {
  outp.i+=inp.i;
  outp.j+=inp.j;
  outp.k+=inp.k;
}


void SetNormal (GFXVertex &outp,
                const GFXVertex &a,
                const GFXVertex &b,
                const GFXVertex &c) {
  GFXVertex left,right;
  left.i=b.x-a.x;
  left.j=b.y-a.y;
  left.k=b.z-a.z;
  right.i=c.x-a.x;
  right.j=c.y-a.y;
  right.k=c.z-a.z;
  outp.i = left.j*right.k-left.k*right.j;//xpd
  outp.j = left.k*right.i-left.i*right.k;
  outp.k = left.i*right.j-left.j*right.i;
  float len = (float)sqrt (outp.i*outp.i+outp.j*outp.j+outp.k*outp.k);
  if (len>.00001) {
    outp.i/=len;
    outp.j/=len;
    outp.k/=len;
  }
}
                

void boundscheck(XML xml){
float maxa=1.0;
float maxd=1.0;
float maxe=1.0;
float maxs=1.0;
maxa=(maxa<xml.material.aa)?xml.material.aa:maxa;
maxa=(maxa<xml.material.ar)?xml.material.ar:maxa;
maxa=(maxa<xml.material.ag)?xml.material.ag:maxa;
maxa=(maxa<xml.material.ab)?xml.material.ab:maxa;
maxd=(maxd<xml.material.da)?xml.material.da:maxd;
maxd=(maxd<xml.material.dr)?xml.material.dr:maxd;
maxd=(maxd<xml.material.dg)?xml.material.dg:maxd;
maxd=(maxd<xml.material.db)?xml.material.db:maxd;
maxe=(maxe<xml.material.ea)?xml.material.ea:maxe;
maxe=(maxe<xml.material.er)?xml.material.er:maxe;
maxe=(maxe<xml.material.eg)?xml.material.eg:maxe;
maxe=(maxe<xml.material.eb)?xml.material.eb:maxe;
maxs=(maxs<xml.material.sa)?xml.material.sa:maxs;
maxs=(maxs<xml.material.sr)?xml.material.sr:maxs;
maxs=(maxs<xml.material.sg)?xml.material.sg:maxs;
maxs=(maxs<xml.material.sb)?xml.material.sb:maxs;
xml.material.aa/=maxa;
xml.material.ar/=maxa;
xml.material.ag/=maxa;
xml.material.ab/=maxa;
xml.material.da/=maxd;
xml.material.dr/=maxd;
xml.material.dg/=maxd;
xml.material.db/=maxd;
xml.material.ea/=maxe;
xml.material.er/=maxe;
xml.material.eg/=maxe;
xml.material.eb/=maxe;
xml.material.sa/=maxs;
xml.material.sr/=maxs;
xml.material.sg/=maxs;
xml.material.sb/=maxs;
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

bool shouldreflect (string r) {
    if (strtoupper(r)=="FALSE")
		return false;
	int32bit i;
	for (i=0;i<(int32bit)r.length();++i) {
		if (r[i]!='0'&&r[i]!='.'&&r[i]!='+'&&r[i]!='e')
			return true; // Just about anything other than "FALSE" or 0.00 etc.
	}
	return false;
}


extern bool flips,flipt,flipn;

void beginElement(const string &name, const AttributeList &attributes, XML * xml) {
  
  AttributeList::const_iterator iter;
  XML::Names elem = (XML::Names)XML::element_map.lookup(name);
  XML::Names top;
  if(xml->state_stack.size()>0) top = *xml->state_stack.rbegin();
  xml->state_stack.push_back(elem);
  switch(elem) {
  case XML::DETAILPLANE:
	 memset(&xml->detailplane, 0, sizeof(xml->detailplane));
	 for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
	    switch(XML::attribute_map.lookup((*iter).name)) {
		case XML::X:
			xml->detailplane.x=XMLSupport::parse_float(iter->value);
			break;
		case XML::Y:
			xml->detailplane.y=XMLSupport::parse_float(iter->value);
			break;
		case XML::Z:
			xml->detailplane.z=XMLSupport::parse_float(iter->value);
			break;
			}
	  }
	  xml->detailplanes.push_back(xml->detailplane);
	  break;
  case XML::MATERIAL:
  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		    switch(XML::attribute_map.lookup((*iter).name)) {
			case XML::USENORMALS:
			  xml->usenormals = XMLSupport::parse_bool (iter->value);
			  break;
		    case XML::POWER:
		      xml->material.power=XMLSupport::parse_float((*iter).value);
		      break;
		    case XML::REFLECT:
		      xml->reflect= ( shouldreflect((*iter).value));
		      break;
		    case XML::LIGHTINGON:
		      xml->lighting= (XMLSupport::parse_bool((*iter).value)); 
		      break;
		    case XML::CULLFACE:
		      xml->cullface =(XMLSupport::parse_bool((*iter).value)); 
		      break;
		    }
		  }
		  break;
  case XML::DIFFUSE:
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		  switch(XML::attribute_map.lookup((*iter).name)) {
		  case XML::RED:
			  xml->material.dr=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::BLUE:
			  xml->material.db=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::ALPHA:
			  xml->material.da=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::GREEN:
			  xml->material.dg=XMLSupport::parse_float((*iter).value);
			  break;
		  }
	  }
	  break;
  case XML::EMISSIVE:
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		  switch(XML::attribute_map.lookup((*iter).name)) {
		  case XML::RED:
			  xml->material.er=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::BLUE:
			  xml->material.eb=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::ALPHA:
			  xml->material.ea=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::GREEN:
			  xml->material.eg=XMLSupport::parse_float((*iter).value);
			  break;
		  }
	  }
	  break;
  case XML::SPECULAR:
	   for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		  switch(XML::attribute_map.lookup((*iter).name)) {
		  case XML::RED:
			  xml->material.sr=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::BLUE:
			  xml->material.sb=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::ALPHA:
			  xml->material.sa=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::GREEN:
			  xml->material.sg=XMLSupport::parse_float((*iter).value);
			  break;
		  }
	  }
	  break;
  case XML::AMBIENT:
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		  switch(XML::attribute_map.lookup((*iter).name)) {
		  case XML::RED:
			  xml->material.ar=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::BLUE:
			  xml->material.ab=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::ALPHA:
			  xml->material.aa=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::GREEN:
			  xml->material.ag=XMLSupport::parse_float((*iter).value);
			  break;
		  }
	  }
	  break;
  case XML::UNKNOWN:
   fprintf (stderr, "Unknown element start tag '%s' detected\n",name.c_str());
    break;
  case XML::MESH:
	//memset(&xml->material, 0, sizeof(xml->material));
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
	  case XML::REVERSE:
		xml->reverse = XMLSupport::parse_bool((*iter).value);
		break;
      case XML::FORCETEXTURE:
		xml->force_texture=XMLSupport::parse_bool ((*iter).value);
		break;
	  case XML::SCALE:
		xml->scale =  XMLSupport::parse_float ((*iter).value);
		break;
      case XML::ALPHATEST:
        xml->alphatest=XMLSupport::parse_float((*iter).value);
        break;
	  case XML::SHAREVERT:
		xml->sharevert = XMLSupport::parse_bool ((*iter).value);
		break;
	  case XML::POLYGONOFFSET:
	    xml->polygon_offset = XMLSupport::parse_float ((*iter).value);
	    break;
	  case XML::BLENDMODE:
		{
		char *csrc=strdup ((*iter).value.c_str());
		char *cdst=strdup((*iter).value.c_str());
		sscanf (((*iter).value).c_str(),"%s %s",csrc,cdst);
		xml->blend_src=parse_alpha (csrc);
		xml->blend_dst=parse_alpha (cdst);
		free (csrc);
		free (cdst);
		}
		break;
	  case XML::DETAILTEXTURE:
		  {
		string detnametmp=(*iter).value.c_str();
		xml->detailtexture.type=TEXTURE;
		xml->detailtexture.index=0;
		xml->detailtexture.name=vector<char8bit>();
		for(int32bit detnamelen=0;detnamelen<detnametmp.size();detnamelen++){
			xml->detailtexture.name.push_back(detnametmp[detnamelen]);
		}
		  }
		break;
      case XML::TEXTURE: 
	  case XML::ALPHAMAP: 
	  case XML::ANIMATEDTEXTURE:
	  case XML::UNKNOWN: //FIXME?
		 {
          XML::Names whichtype = XML::UNKNOWN;
          int32bit strsize=0;
          if (strtoupper(iter->name).find("ANIMATION")==0) {
              xml->texturetemp.type=ANIMATION;
			  whichtype = XML::ANIMATEDTEXTURE;
              strsize = strlen ("ANIMATION");
          }
          if (strtoupper(iter->name).find("TEXTURE")==0){
              xml->texturetemp.type=TEXTURE;
			  whichtype= XML::TEXTURE;
              strsize = strlen ("TEXTURE");
          }
          if (strtoupper(iter->name).find("ALPHAMAP")==0){
              xml->texturetemp.type=ALPHAMAP;
			  whichtype=XML::ALPHAMAP;
              strsize= strlen ("ALPHAMAP");
          }
          if (whichtype!=XML::UNKNOWN) {
              unsigned int32bit texindex =0;
              string ind(iter->name.substr (strsize));
              if (!ind.empty()){
				texindex=atoi(ind.c_str());
			  }
			  xml->texturetemp.index=texindex;
			  xml->texturetemp.name=vector<char8bit>();
			  string nomdujour=iter->value.c_str();
			  for(int32bit tni=0;tni<nomdujour.size();tni++){
				xml->texturetemp.name.push_back(nomdujour[tni]);
			  }
			xml->textures.push_back(xml->texturetemp);
		  }
		 }
		break;
	  }
	}
  case XML::POINTS:
    break;
  case XML::POINT:
	memset(&xml->vertex, 0, sizeof(xml->vertex));
    break;
  case XML::LOCATION:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::X:
	    xml->vertex.x = XMLSupport::parse_float((*iter).value);
	    break;
      case XML::Y:
		xml->vertex.y = XMLSupport::parse_float((*iter).value);
		break;
     case XML::Z:
		xml->vertex.z = XMLSupport::parse_float((*iter).value);
		break;
      case XML::S:
        xml->vertex.s = XMLSupport::parse_float ((*iter).value) * (flips ? -1:+1);
		break;
      case XML::T:
		xml->vertex.t = XMLSupport::parse_float ((*iter).value) * (flipt ? -1:+1);
		break;
      }
    }
    break;
  case XML::NORMAL:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::I:
        xml->vertex.i = XMLSupport::parse_float((*iter).value) * (flipn?-1:+1);
		break;
      case XML::J:
		xml->vertex.j = XMLSupport::parse_float((*iter).value) * (flipn?-1:+1);
		break;
      case XML::K:
		xml->vertex.k = XMLSupport::parse_float((*iter).value) * (flipn?-1:+1);
		break;
      }

    }
    break;
  case XML::POLYGONS:
    break;
  case XML::LINE:
	memset(&xml->linetemp, 0, sizeof(xml->linetemp));
	xml->curpolytype=LINE;
	xml->curpolyindex=0;
	xml->linetemp.flatshade=0;
    break;
  case XML::TRI:
	memset(&xml->triangletemp, 0, sizeof(xml->triangletemp));
	xml->curpolytype=TRIANGLE;
	xml->curpolyindex=0;
	for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::FLATSHADE:
		if ((*iter).value=="Flat") {
			xml->triangletemp.flatshade=1;
		}else {
			xml->triangletemp.flatshade=0;
		}break;
	  }
	}
    break;
  case XML::QUAD:
	memset(&xml->quadtemp, 0, sizeof(xml->quadtemp));
	xml->curpolytype=QUAD;
	xml->curpolyindex=0;
	for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::FLATSHADE:
		if ((*iter).value=="Flat") {
			xml->quadtemp.flatshade=1;
		}else {
			xml->quadtemp.flatshade=0;
		}break;
	  }
	}
    break;
  case XML::LINESTRIP: //FIXME?
	xml->striptemp=strip();
	xml->curpolytype=LINESTRIP;
	xml->striptemp.flatshade=0;
	break;
  case XML::TRISTRIP: //FIXME?
    xml->striptemp=strip();
	xml->curpolytype=TRISTRIP;
	xml->striptemp.flatshade=0;
	break;
  case XML::TRIFAN: //FIXME?
    xml->striptemp=strip();
	xml->curpolytype=TRIFAN;
	xml->striptemp.flatshade=0;
	break;
  case XML::QUADSTRIP: //FIXME?
    xml->striptemp=strip();
	xml->curpolytype=QUADSTRIP;
	xml->striptemp.flatshade=0;
	break;
  case XML::VERTEX:
	unsigned int32bit index;
    float32bit s,t;
	s=0;
	t=0;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::POINT:
		index = XMLSupport::parse_int((*iter).value);
		break;
      case XML::S:
		s = XMLSupport::parse_float((*iter).value);
		break;
      case XML::T:
		t = XMLSupport::parse_float((*iter).value);
		break;
     }
    }
    if (index<xml->num_vertex_references.size()){
        if (xml->num_vertex_references[index]==0) {
            xml->vertices[index].i=xml->vertices[index].j=xml->vertices[index].k=0;

            if (xml->vertices[index].s==0) xml->vertices[index].s=s;
            if (xml->vertices[index].t==0) xml->vertices[index].t=t;
        }
        xml->num_vertex_references[index]++;
    }
	switch(xml->curpolytype){
	case LINE:
		xml->linetemp.indexref[xml->curpolyindex]=index;
		xml->linetemp.s[xml->curpolyindex]=s;
		xml->linetemp.t[xml->curpolyindex]=t;
                if (xml->curpolyindex==1) {
                  float x = 
                    xml->vertices[xml->linetemp.indexref[1]].x-
                    xml->vertices[xml->linetemp.indexref[0]].x;
                  float y = 
                    xml->vertices[xml->linetemp.indexref[1]].y-
                    xml->vertices[xml->linetemp.indexref[0]].y;
                  float z = 
                    xml->vertices[xml->linetemp.indexref[1]].z-
                    xml->vertices[xml->linetemp.indexref[0]].z;
                  float len = sqrt(x*x+y*y+z*z);
                  if (len>.0001) {x/=len;y/=len;z/=len;}                    
                  xml->vertices[xml->linetemp.indexref[0]].i=-x;
                  xml->vertices[xml->linetemp.indexref[0]].j=-y;
                  xml->vertices[xml->linetemp.indexref[0]].k=-z;
                  xml->vertices[xml->linetemp.indexref[1]].i=x;
                  xml->vertices[xml->linetemp.indexref[1]].j=y;
                  xml->vertices[xml->linetemp.indexref[1]].k=z                  ;
                    }
                
		break;
	case TRIANGLE:
		xml->triangletemp.indexref[xml->curpolyindex]=index;
		xml->triangletemp.s[xml->curpolyindex]=s;
		xml->triangletemp.t[xml->curpolyindex]=t;
                if (xml->curpolyindex==2) {
                  GFXVertex temp;
                  SetNormal(temp,
                            xml->vertices[xml->triangletemp.indexref[2]],
                            xml->vertices[xml->triangletemp.indexref[1]],
                            xml->vertices[xml->triangletemp.indexref[0]]);
                  AddNormal(xml->vertices[xml->triangletemp.indexref[0]],
                            temp);
                  AddNormal(xml->vertices[xml->triangletemp.indexref[1]],
                            temp);
                  AddNormal(xml->vertices[xml->triangletemp.indexref[2]],
                            temp);
                }
		break;
	case QUAD:
		xml->quadtemp.indexref[xml->curpolyindex]=index;
		xml->quadtemp.s[xml->curpolyindex]=s;
		xml->quadtemp.t[xml->curpolyindex]=t;
                if (xml->curpolyindex==3) {
                  GFXVertex temp;
                  SetNormal(temp,
                            xml->vertices[xml->quadtemp.indexref[2]],
                            xml->vertices[xml->quadtemp.indexref[1]],
                            xml->vertices[xml->quadtemp.indexref[0]]);
                  AddNormal(xml->vertices[xml->quadtemp.indexref[0]],
                            temp);
                  AddNormal(xml->vertices[xml->quadtemp.indexref[1]],
                            temp);
                  AddNormal(xml->vertices[xml->quadtemp.indexref[2]],
                            temp);
                  AddNormal(xml->vertices[xml->quadtemp.indexref[3]],
                            temp);                            
                }
		break;
	case LINESTRIP:
	case TRISTRIP:
	case TRIFAN:
	case QUADSTRIP:
		xml->stripelementtemp=stripelement();
		xml->stripelementtemp.indexref=index;
		xml->stripelementtemp.s=s;
		xml->stripelementtemp.t=t;
		xml->striptemp.points.push_back(xml->stripelementtemp);
                if (xml->striptemp.points.size()>2
                    &&(xml->curpolytype!=QUADSTRIP||
                       xml->striptemp.points.size()%2==0)) {
                  GFXVertex temp;
                  bool rev = ((xml->striptemp.points.size()%2==0)&&
                              xml->curpolytype==TRISTRIP)
                    || xml->curpolytype==QUADSTRIP;
                  SetNormal(temp,
                            xml->vertices[xml->striptemp.points[xml->striptemp.points.size()-(rev?3:2)].indexref],
                            xml->vertices[xml->striptemp.points[xml->striptemp.points.size()-(rev?2:3)].indexref],
                            xml->vertices[xml->striptemp.points.back().indexref]);
                  AddNormal(xml->vertices[xml->striptemp.points.back().indexref],
                            temp);
                  AddNormal(xml->vertices[xml->striptemp.points[xml->striptemp.points.size()-2].indexref],
                            temp);
                  AddNormal(xml->vertices[xml->striptemp.points[xml->striptemp.points.size()-3].indexref],
                            temp);
                  if (xml->curpolytype==QUADSTRIP) 
                    AddNormal(xml->vertices[xml->striptemp.points[xml->striptemp.points.size()-4].indexref],
                              temp);
                }
		break;
	}
	xml->curpolyindex+=1;	
    break;
  case XML::LOD: //FIXME?
	  xml->lodtemp=LODholder();
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		switch(XML::attribute_map.lookup((*iter).name)) {
		case XML::UNKNOWN:
		break;
		case XML::FRAMESPERSECOND:
		  continue;
		  break;
		case XML::SIZE:
		  xml->lodtemp.size = XMLSupport::parse_float ((*iter).value);
		  break;
		case XML::LODFILE:
		  string lodname = (*iter).value.c_str();
		  xml->lodtemp.name=vector<char8bit>();
		  for(int32bit index=0;index<lodname.size();index++){
			xml->lodtemp.name.push_back(lodname[index]);
		  }
		  break;
		}
	  }
	  xml->LODs.push_back(xml->lodtemp);
	  break;
  case XML::LOGO: 
	unsigned int32bit typ;
    float32bit rot, siz,offset;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::UNKNOWN:
		fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
		break;
      case XML::TYPE:
		typ = XMLSupport::parse_int((*iter).value);
		break;
      case XML::ROTATE:
		rot = XMLSupport::parse_float((*iter).value);
		break;
      case XML::SIZE:
		siz = XMLSupport::parse_float((*iter).value);
		break;
      case XML::OFFSET:
		offset = XMLSupport::parse_float ((*iter).value);
		break;
      }
    }
    xml->logos.push_back(XML::ZeLogo());
    xml->logos[xml->logos.size()-1].type = typ;
    xml->logos[xml->logos.size()-1].rotate = rot;
    xml->logos[xml->logos.size()-1].size = siz;
    xml->logos[xml->logos.size()-1].offset = offset;
    break;
  case XML::REF: //FIXME
	{
	unsigned int32bit ind=0;
    float32bit indweight=1;
    bool foundindex=false;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::UNKNOWN:
		fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
		break;
      case XML::POINT:
		ind = XMLSupport::parse_int((*iter).value);
		foundindex=true;
		break;
      case XML::WEIGHT:
		indweight = XMLSupport::parse_float((*iter).value);
		break;
      }
    }
    
    xml->logos[xml->logos.size()-1].refpnt.push_back(ind);
    xml->logos[xml->logos.size()-1].refweight.push_back(indweight);
	}
    break;
  case XML::ANIMDEF:
	xml->animdeftemp=animdef();
	for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
	    case XML::ANIMATIONNAME:
		{
		  string animname = (*iter).value.c_str();
		  xml->animdeftemp.name=vector<char8bit>();
		  for(int32bit index=0;index<animname.size();index++){
			xml->animdeftemp.name.push_back(animname[index]);
		  }
		}
		  break;
		case XML::FRAMESPERSECOND:
		  xml->animdeftemp.FPS=XMLSupport::parse_float((*iter).value);
		  break;
	  }
	}
	break;
  case XML::ANIMATIONFRAMEINDEX:
	for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
	  case XML::ANIMATIONMESHINDEX:
        xml->animdeftemp.meshoffsets.push_back(XMLSupport::parse_int((*iter).value));
		break;
	  }
	}
    break;
  case XML::ANIMFRAME:
    xml->animframetemp=animframe();
	for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
	    case XML::FRAMEMESHNAME:
		  string framename = (*iter).value.c_str();
		  xml->animframetemp.name=vector<char8bit>();
		  for(int32bit index=0;index<framename.size();index++){
			xml->animframetemp.name.push_back(framename[index]);
		  }
		  break;
	  }
	}
	xml->animframes.push_back(xml->animframetemp);
	break;
  }
    
}

void endElement(const string &name, XML * xml) {
  xml->state_stack.pop_back();
  XML::Names elem = (XML::Names)XML::element_map.lookup(name);
 
  switch(elem) {
  case XML::UNKNOWN:
    fprintf (stderr,"Unknown element end tag '%s' detected\n",name.c_str());
    break;
  case XML::POINT:
    xml->vertices.push_back (xml->vertex);
    xml->num_vertex_references.push_back (0);
    break;
  case XML::VERTEX:
	break;
  case XML::POINTS:
    break;
  case XML::LINE:
	xml->lines.push_back(xml->linetemp);        
    break;
  case XML::TRI:
	xml->tris.push_back (xml->triangletemp);
    break;
  case XML::QUAD:
	xml->quads.push_back (xml->quadtemp);
    break;
  case XML::LINESTRIP://FIXE?
	xml->linestrips.push_back(xml->striptemp);
    break;
  case XML::TRISTRIP://FIXME?
	xml->tristrips.push_back(xml->striptemp);
    break;
  case XML::TRIFAN://FIXME?
	xml->trifans.push_back(xml->striptemp);
    break;
  case XML::QUADSTRIP://FIXME?
	xml->quadstrips.push_back(xml->striptemp);
    break;
  case XML::POLYGONS:
    { 
        for (int i=0; (i<xml->vertices.size()) && (i<xml->num_vertex_references.size()); i++) {
            float f=((xml->num_vertex_references[i]>0)?1.f/xml->num_vertex_references[i]:1.f) * (flipn?-1:+1);
            xml->vertices[i].i *= f;
            xml->vertices[i].j *= f;
            xml->vertices[i].k *= f;
        }
    }
    break;
  case XML::REF:
    break;
  case XML::LOGO: 
    break;
  case XML::MATERIAL:
	  break;
  case XML::DIFFUSE:
	  break;
  case XML::EMISSIVE:
	  break;
  case XML::SPECULAR:
	  break;
  case XML::AMBIENT:
	  break;
  case XML::MESH:
    break;
  case XML::ANIMDEF:
	xml->animdefs.push_back(xml->animdeftemp);
	break;
  default:
    ;
  }
}

void beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  beginElement(name, AttributeList(atts),(XML *)userData);
}



void endElement(void *userData, const XML_Char *name) {
  endElement(name, (XML *) userData);
}


XML LoadXML(const char *filename, float32bit unitscale) {
  const int32bit chunk_size = 16384;
  FILE* inFile = fopen (filename, "r");
  if(!inFile) {
    fprintf (stderr,"Cannot Open Mesh File %s\n",filename);
    exit(0);
    return XML();
  }

  XML xml;
  xml.scale = unitscale;
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, &xml);
  XML_SetElementHandler(parser, &beginElement, &endElement);
  
	
  do {
    char buf[chunk_size];
    int32bit length;
    
    length = fread(buf,1, chunk_size,inFile);
    XML_Parse (parser,buf,length,feof(inFile));
  } while(!feof(inFile));
  fclose (inFile);
  XML_ParserFree (parser);
  // Now, copy everything into the mesh data structures
//  boundscheck(xml);
  return xml;
}

void xmeshToBFXM(XML memfile,FILE* Outputfile,char mode,bool forcenormals, bool force_shared_vertex){//converts input file to BFXM creates new, or appends record based on mode
  unsigned int32bit intbuf;
  
  bool append=(mode=='a');
  
  int32bit runningbytenum=0;
  if(!append){
      runningbytenum+=writesuperheader(memfile,Outputfile,force_shared_vertex); // Write superheader
  }
  fseek(Outputfile,0,SEEK_END);
  runningbytenum+=appendrecordfromxml(memfile,Outputfile,forcenormals,force_shared_vertex); //Append one record

  rewind(Outputfile);
  fseek(Outputfile,4+7*sizeof(int32bit),SEEK_SET);
  fread(&intbuf,sizeof(int32bit),1,Outputfile);//Current number of records
  intbuf=VSSwapHostIntToLittle(intbuf);
  ++intbuf;
  intbuf=VSSwapHostIntToLittle(intbuf);
  fseek(Outputfile,4+7*sizeof(int32bit),SEEK_SET);
  fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//number of records++

  fseek(Outputfile,4+sizeof(int32bit),SEEK_SET);
  fread(&intbuf,sizeof(int32bit),1,Outputfile);//Current length of file
  intbuf=VSSwapHostIntToLittle(intbuf);
  intbuf+=runningbytenum;
  intbuf=VSSwapHostIntToLittle(intbuf);
  fseek(Outputfile,4+sizeof(int32bit),SEEK_SET);
  fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Correct number of bytes for total file
}
extern float transx,transy,transz;
extern float scalex,scaley,scalez;
int32bit writesuperheader(XML memfile, FILE* Outputfile, bool force_shared){
  unsigned int32bit intbuf;
  int32bit versionnumber=VSSwapHostIntToLittle(20);
  char8bit bytebuf;
  int32bit runningbytenum=0;
  //SUPER HEADER

  bytebuf='B'; // "Magic Word"
  runningbytenum+=fwrite(&bytebuf,1,1,Outputfile);
  bytebuf='F';
  runningbytenum+=fwrite(&bytebuf,1,1,Outputfile);
  bytebuf='X';
  runningbytenum+=fwrite(&bytebuf,1,1,Outputfile);
  bytebuf='M';
  runningbytenum+=fwrite(&bytebuf,1,1,Outputfile);
  runningbytenum+=sizeof(int32bit)*fwrite(&versionnumber,sizeof(int32bit),1,Outputfile);// VERSION number for BinaryFormattedXMesh
  intbuf=VSSwapHostIntToLittle(0);//Length of File Placeholder
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);// Number of bytes in file
  //Super-Header Meaty part
  intbuf=VSSwapHostIntToLittle(4+(9*sizeof(int32bit)));//Super-Header length in Bytes for version 0.10
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);// Number of bytes in Superheader
  intbuf=VSSwapHostIntToLittle(8);//Number of fields per vertex
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//
  intbuf=VSSwapHostIntToLittle(force_shared?0:1);//Number of fields per polygon structure
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);// 
  intbuf=VSSwapHostIntToLittle(force_shared?1:3);//Number of fields per referenced vertex
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);// 
  intbuf=VSSwapHostIntToLittle(1);//Number of fields per referenced animation
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);// 
  intbuf=VSSwapHostIntToLittle(0);//Number of records - initially 0
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);// Number of records
  intbuf=VSSwapHostIntToLittle(1);//Number of fields per animation def
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);// 
  return runningbytenum;
}

int32bit appendrecordfromxml(XML memfile, FILE* Outputfile,bool forcenormals, bool force_shared_vertex){
  unsigned int32bit intbuf;
  int32bit runningbytenum=0;
  //Record Header
  intbuf=VSSwapHostIntToLittle(12);// Size of Record Header in bytes
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);// Number of bytes in record header
  intbuf=VSSwapHostIntToLittle(0);//Size of Record in bytes
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);// Number of bytes in record
  intbuf=VSSwapHostIntToLittle(1+memfile.LODs.size()+memfile.animframes.size());//Number of meshes = 1 + numLODs + numAnims. 
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);// Number of meshes
  
  runningbytenum+=appendmeshfromxml(memfile,Outputfile,forcenormals,force_shared_vertex); // write top level mesh
  int32bit mesh;
  for(mesh=0;mesh<memfile.LODs.size();mesh++){ //write all LOD meshes
	string LODname="";
	for(int32bit i=0;i<memfile.LODs[mesh].name.size();i++){
         LODname+=memfile.LODs[mesh].name[i];
	}
	XML submesh=LoadXML(LODname.c_str(),1);
	runningbytenum+=appendmeshfromxml(submesh,Outputfile,forcenormals,force_shared_vertex);
  }
  for(mesh=0;mesh<memfile.animframes.size();mesh++){ //write all Animation Frames
	string animname="";
	for(int32bit i=0;i<memfile.animframes[mesh].name.size();i++){
         animname+=memfile.animframes[mesh].name[i];
	}
	XML submesh=LoadXML(animname.c_str(),1);
	runningbytenum+=appendmeshfromxml(submesh,Outputfile,forcenormals,force_shared_vertex);
  }
  
  fseek(Outputfile,(-1*(runningbytenum))+4,SEEK_CUR);
  intbuf=runningbytenum;
  intbuf= VSSwapHostIntToLittle(intbuf);
  fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Correct number of bytes for total record
  fseek(Outputfile,0,SEEK_END);
  return runningbytenum;

}
static float mymax(float a, float b) {
  return a>b?a:b;
}
void NormalizeProperty(float&r, float&g, float&b, float&a) {
  if (r>1||g>1||b>1||a>1) {
    float mx = mymax(mymax(mymax(r,g),b),a);
    r/=mx;g/=mx;b/=mx;
    if (a>1)//sometimes they get the alpha right
      a/=mx;
  }
}
void NormalizeMaterial(GFXMaterial &m) {
  NormalizeProperty(m.ar,m.ag,m.ab,m.aa);
  NormalizeProperty(m.dr,m.dg,m.db,m.da);
  NormalizeProperty(m.sr,m.sg,m.sb,m.sa);
  NormalizeProperty(m.er,m.eg,m.eb,m.ea);
}
int32bit appendmeshfromxml(XML memfile, FILE* Outputfile,bool forcenormals, bool force_shared_vertex){
  unsigned int32bit intbuf;
  float32bit floatbuf;
  char8bit bytebuf;
  int32bit runningbytenum=0;
  
  //Mesh Header
  intbuf= VSSwapHostIntToLittle(11*sizeof(int32bit)+20*sizeof(float32bit));
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Size of Mesh header in Bytes
  intbuf= VSSwapHostIntToLittle(0);// Temp - rewind and fix.
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Size of this Mesh in Bytes
  floatbuf = VSSwapHostFloatToLittle(memfile.scale);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);// Mesh Scale
  intbuf= VSSwapHostIntToLittle(memfile.reverse);
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//reverse flag
  intbuf= VSSwapHostIntToLittle(memfile.force_texture);
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Force texture flag
  intbuf= VSSwapHostIntToLittle(memfile.sharevert||force_shared_vertex);
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Share vertex flag
  floatbuf= VSSwapHostFloatToLittle(memfile.polygon_offset);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Polygon offset
  intbuf= VSSwapHostIntToLittle(memfile.blend_src);
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Blend Source
  intbuf= VSSwapHostIntToLittle(memfile.blend_dst);
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Blend Destination
  NormalizeMaterial(memfile.material);
  floatbuf= VSSwapHostFloatToLittle(memfile.material.power);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Specular:Power
  floatbuf= VSSwapHostFloatToLittle(memfile.material.ar);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Ambient:Red
  floatbuf= VSSwapHostFloatToLittle(memfile.material.ag);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Ambient:Green
  floatbuf= VSSwapHostFloatToLittle(memfile.material.ab);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Ambient:Blue
  floatbuf= VSSwapHostFloatToLittle(memfile.material.aa);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Ambient:Alpha
  floatbuf= VSSwapHostFloatToLittle(memfile.material.dr);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Diffuse:Red
  floatbuf= VSSwapHostFloatToLittle(memfile.material.dg);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Diffuse:Green
  floatbuf= VSSwapHostFloatToLittle(memfile.material.db);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Diffuse:Blue
  floatbuf= VSSwapHostFloatToLittle(memfile.material.da);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Diffuse:Alpha
  floatbuf= VSSwapHostFloatToLittle(memfile.material.er);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Emissive:Red
  floatbuf= VSSwapHostFloatToLittle(memfile.material.eg);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Emissive:Green
  floatbuf= VSSwapHostFloatToLittle(memfile.material.eb);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Emissive:Blue
  floatbuf= VSSwapHostFloatToLittle(memfile.material.ea);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Emissive:Alpha
  floatbuf= VSSwapHostFloatToLittle(memfile.material.sr);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Specular:Red
  floatbuf= VSSwapHostFloatToLittle(memfile.material.sg);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Specular:Green
  floatbuf= VSSwapHostFloatToLittle(memfile.material.sb);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Specular:Blue
  floatbuf= VSSwapHostFloatToLittle(memfile.material.sa);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Material:Specular:Alpha
  intbuf= VSSwapHostIntToLittle(memfile.cullface);
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Cullface
  intbuf= VSSwapHostIntToLittle(memfile.lighting);
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//lighting
  intbuf= VSSwapHostIntToLittle(memfile.reflect);
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//reflect


  //Usenormals default value fix.
  if(forcenormals){
	  memfile.usenormals=true;
  }
  intbuf= VSSwapHostIntToLittle(memfile.usenormals);
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//usenormals

  //added by hellcatv
  floatbuf= VSSwapHostFloatToLittle(memfile.alphatest);
  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//alpha test value

  //END HEADER
  //Begin Variable sized Attributes
  int32bit VSAstart=runningbytenum;
  intbuf= VSSwapHostIntToLittle(0); // Temp value will overwrite later
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Length of Variable sized attribute section in bytes
  //Detail texture
  { 
	int32bit namelen=memfile.detailtexture.name.size();
	intbuf= VSSwapHostIntToLittle(namelen);
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Length of name of detail texture
        int32bit nametmp;
	for(nametmp=0;nametmp<namelen;nametmp++){
		bytebuf= memfile.detailtexture.name[nametmp];
		runningbytenum+=fwrite(&bytebuf,sizeof(char8bit),1,Outputfile);//char by char name of detail texture
	}
	int32bit padlength=(sizeof(int32bit)-(namelen%sizeof(int32bit)))%sizeof(int32bit);
	for(nametmp=0;nametmp<padlength;nametmp++){
		bytebuf=0;
		runningbytenum+=fwrite(&bytebuf,sizeof(char8bit),1,Outputfile);//Padded so that next field is word aligned
	}
  }
  //Detail Planes
  intbuf= VSSwapHostIntToLittle(memfile.detailplanes.size());
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of detail planes
  for(int32bit plane=0;plane<memfile.detailplanes.size();plane++){
	floatbuf= VSSwapHostFloatToLittle(memfile.detailplanes[plane].x);
	runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Detail Plane:X
	floatbuf= VSSwapHostFloatToLittle(memfile.detailplanes[plane].y);
	runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Detail Plane:Y
	floatbuf= VSSwapHostFloatToLittle(memfile.detailplanes[plane].z);
	runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Detail Plane:Z
  }
  //Textures
  {
  int32bit texnum;
  intbuf= 0;
  for(texnum=0;texnum<memfile.textures.size();texnum++) 
      if(memfile.textures[texnum].type != UNKNOWN)
          intbuf++;
  intbuf= VSSwapHostIntToLittle(intbuf);
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of textures
  for(texnum=0;texnum<memfile.textures.size();texnum++) if(memfile.textures[texnum].type != UNKNOWN) {
	intbuf= VSSwapHostIntToLittle(memfile.textures[texnum].type);
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//texture # texnum: type
	intbuf= VSSwapHostIntToLittle(memfile.textures[texnum].index);
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//texture # texnum: index
	int32bit namelen=memfile.textures[texnum].name.size();
	intbuf= VSSwapHostIntToLittle(namelen);
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Length of name of texture # texnum
    int32bit nametmp;
	for(nametmp=0;nametmp<namelen;nametmp++){
		bytebuf= memfile.textures[texnum].name[nametmp];
		runningbytenum+=fwrite(&bytebuf,sizeof(char8bit),1,Outputfile);//Name of texture # texnum
	}
	int32bit padlength=(sizeof(int32bit)-(namelen%sizeof(int32bit)))%sizeof(int32bit);
	for(nametmp=0;nametmp<padlength;nametmp++){
		bytebuf=0;
		runningbytenum+=fwrite(&bytebuf,sizeof(char8bit),1,Outputfile);//Padded so that next field is word aligned
	}
  }
  }

  //Logos
  //FIXME?
  intbuf= VSSwapHostIntToLittle(memfile.logos.size());
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of logos
  for(int32bit logonum=0;logonum<memfile.logos.size();logonum++){
	floatbuf= VSSwapHostFloatToLittle(memfile.logos[logonum].size);
	runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//logo # logonum: size
	floatbuf= VSSwapHostFloatToLittle(memfile.logos[logonum].offset);
	runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//logo # logonum: offset
	floatbuf= VSSwapHostFloatToLittle(memfile.logos[logonum].rotate);
	runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//logo # logonum: rotation
	intbuf= VSSwapHostIntToLittle(memfile.logos[logonum].type);
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//logo # logonum: type
	int32bit numrefs=memfile.logos[logonum].refpnt.size();
	intbuf=VSSwapHostIntToLittle(numrefs);
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//logo # logonum: number of references
	for(int32bit ref=0;ref<numrefs;ref++){
	  intbuf=VSSwapHostIntToLittle(memfile.logos[logonum].refpnt[ref]);
	  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//logo # logonum: reference # ref
	  floatbuf= VSSwapHostFloatToLittle(memfile.logos[logonum].refweight[ref]);
	  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//logo # logonum: reference # ref weight
	}
  }

  //LODs + Animations
  //LODs
  int32bit submeshref=1;
  intbuf=VSSwapHostIntToLittle(memfile.LODs.size());
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of LODs
  for(int32bit lod=0;lod<memfile.LODs.size();lod++){
	floatbuf= VSSwapHostFloatToLittle(memfile.LODs[lod].size);
	runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//LOD # lod: size
	intbuf=submeshref;
	intbuf=VSSwapHostIntToLittle(intbuf);
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//LOD mesh offset
	submeshref++;
  }

  //Current VS File format is not compatible with new animation specification - can't test until I fix old files (only 1 at present uses animations)
  
  	intbuf=VSSwapHostIntToLittle(memfile.animdefs.size());
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of animdefs
	for(int32bit anim=0;anim<memfile.animdefs.size();anim++){
	  int32bit namelen=memfile.animdefs[anim].name.size();
	  intbuf= VSSwapHostIntToLittle(namelen);
	  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Length of name animation
          int32bit nametmp;
	  for(nametmp=0;nametmp<namelen;nametmp++){
		bytebuf= memfile.animdefs[anim].name[nametmp];
		runningbytenum+=fwrite(&bytebuf,sizeof(char8bit),1,Outputfile);//char by char of above
	  }
	  int32bit padlength=(sizeof(int32bit)-(namelen%sizeof(int32bit)))%sizeof(int32bit);
	  for(nametmp=0;nametmp<padlength;nametmp++){
		bytebuf=0;
		runningbytenum+=fwrite(&bytebuf,sizeof(char8bit),1,Outputfile);//Padded so that next field is word aligned
	  }
	  floatbuf=VSSwapHostFloatToLittle(memfile.animdefs[anim].FPS);
	  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//Animdef # anim: FPS
	  intbuf=VSSwapHostIntToLittle(memfile.animdefs[anim].meshoffsets.size());
	  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//number of animation mesh offsets
	  for(int32bit offset=0;offset<memfile.animdefs[anim].meshoffsets.size();offset++){
		intbuf=submeshref+memfile.animdefs[anim].meshoffsets[offset];
		intbuf=VSSwapHostIntToLittle(intbuf);
		runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//animation mesh offset
	  }
	}

  //End Variable sized Attributes
  int32bit VSAend=runningbytenum;
  //GEOMETRY
  intbuf= VSSwapHostIntToLittle(memfile.vertices.size());
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of vertices
  for(int32bit verts=0;verts<memfile.vertices.size();verts++){
    floatbuf=VSSwapHostFloatToLittle(scalex*(memfile.vertices[verts].x+transx));
          float normallen = sqrt(memfile.vertices[verts].i*memfile.vertices[verts].i+
                                 memfile.vertices[verts].j*memfile.vertices[verts].j+
                                 memfile.vertices[verts].k*memfile.vertices[verts].k);
          if (normallen>.0001) {
            memfile.vertices[verts].i/=normallen;
            memfile.vertices[verts].j/=normallen;
            memfile.vertices[verts].k/=normallen;
          }
	  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//vertex #vert:x
	  floatbuf=VSSwapHostFloatToLittle(scaley*(memfile.vertices[verts].y+transy));
	  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//vertex #vert:y
	  floatbuf=VSSwapHostFloatToLittle(scalez*(memfile.vertices[verts].z+transz));
	  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//vertex #vert:z
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].i);
	  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//vertex #vert:i
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].j);
	  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//vertex #vert:j
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].k);
	  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//vertex #vert:k
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].s);
	  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//vertex #vert:s
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].t);
	  runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//vertex #vert:t
  }
  intbuf= VSSwapHostIntToLittle(memfile.lines.size());
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of lines
  for(int32bit lines=0;lines<memfile.lines.size();lines++){
      if (!force_shared_vertex) {
	intbuf= VSSwapHostIntToLittle(memfile.lines[lines].flatshade);
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Flatshade flag
      }
      for(int32bit tmpcounter=0;tmpcounter<2;tmpcounter++){
	  intbuf= VSSwapHostIntToLittle(memfile.lines[lines].indexref[tmpcounter]);
	  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//point index
	  if (!force_shared_vertex) {
	      floatbuf= VSSwapHostFloatToLittle(memfile.lines[lines].s[tmpcounter]);
	      runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//s coord
	      floatbuf= VSSwapHostFloatToLittle(memfile.lines[lines].t[tmpcounter]);
	      runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//t coord
	  }
      }
  }
  intbuf= VSSwapHostIntToLittle(memfile.tris.size());
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of triangles
  for(int32bit tris=0;tris<memfile.tris.size();tris++){
      if (!force_shared_vertex) {
	intbuf= VSSwapHostIntToLittle(memfile.tris[tris].flatshade);
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Flatshade flag
      }
      for(int32bit tmpcounter=0;tmpcounter<3;tmpcounter++){
	  intbuf= VSSwapHostIntToLittle(memfile.tris[tris].indexref[tmpcounter]);
	  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//point index
	  if (!force_shared_vertex) {
	      floatbuf= VSSwapHostFloatToLittle(memfile.tris[tris].s[tmpcounter]);
	      runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//s coord
	      floatbuf= VSSwapHostFloatToLittle(memfile.tris[tris].t[tmpcounter]);
	      runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//t coord
	  }
      }
  }
  intbuf= VSSwapHostIntToLittle(memfile.quads.size());
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of Quads
  for(int32bit quads=0;quads<memfile.quads.size();quads++){
      if (!force_shared_vertex) {
	  intbuf= VSSwapHostIntToLittle(memfile.quads[quads].flatshade);
	  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Flatshade flag
      }
      for(int32bit tmpcounter=0;tmpcounter<4;tmpcounter++){
	  intbuf= VSSwapHostIntToLittle(memfile.quads[quads].indexref[tmpcounter]);
	  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//point index
	  if (!force_shared_vertex) {
		floatbuf= VSSwapHostFloatToLittle(memfile.quads[quads].s[tmpcounter]);
		runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.quads[quads].t[tmpcounter]);
		runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//t coord
	  }
      }
  }
  intbuf= VSSwapHostIntToLittle(memfile.linestrips.size());
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of linestrips
  for(int32bit ls=0;ls<memfile.linestrips.size();ls++){
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[ls].points.size());
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of elements in current linestrip
	if (!force_shared_vertex) {
	    intbuf= VSSwapHostIntToLittle(memfile.linestrips[ls].flatshade);
	    runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Flatshade flag
	}
	for(int32bit tmpcounter=0;tmpcounter<memfile.linestrips[ls].points.size();tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.linestrips[ls].points[tmpcounter].indexref);
		runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//point index
		if (!force_shared_vertex) {
		    floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[ls].points[tmpcounter].s);
		    runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//s coord
		    floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[ls].points[tmpcounter].t);
		    runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//t coord
		}
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.tristrips.size());
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of tristrips
  for(int32bit ts=0;ts<memfile.tristrips.size();ts++){
	intbuf= VSSwapHostIntToLittle(memfile.tristrips[ts].points.size());
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of elements in current tristrip
	if (!force_shared_vertex) {
	    intbuf= VSSwapHostIntToLittle(memfile.tristrips[ts].flatshade);
	    runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Flatshade flag
	}
	for(int32bit tmpcounter=0;tmpcounter<memfile.tristrips[ts].points.size();tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.tristrips[ts].points[tmpcounter].indexref);
		runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//point index
		if (!force_shared_vertex) {
		    floatbuf= VSSwapHostFloatToLittle(memfile.tristrips[ts].points[tmpcounter].s);
		    runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//s coord
		    floatbuf= VSSwapHostFloatToLittle(memfile.tristrips[ts].points[tmpcounter].t);
		    runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//t coord
		}
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.trifans.size());
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of trifans
  for(int32bit tf=0;tf<memfile.trifans.size();tf++){
	intbuf= VSSwapHostIntToLittle(memfile.trifans[tf].points.size());
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of elements in current trifan
	if (!force_shared_vertex) {
	    intbuf= VSSwapHostIntToLittle(memfile.trifans[tf].flatshade);
	    runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Flatshade flag
	}
	for(int32bit tmpcounter=0;tmpcounter<memfile.trifans[tf].points.size();tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.trifans[tf].points[tmpcounter].indexref);
		runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//point index
		if (!force_shared_vertex) {
		    floatbuf= VSSwapHostFloatToLittle(memfile.trifans[tf].points[tmpcounter].s);
		    runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//s coord
		    floatbuf= VSSwapHostFloatToLittle(memfile.trifans[tf].points[tmpcounter].t);
		    runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//t coord
		}
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.quadstrips.size());
  runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of quadstrips
  for(int32bit qs=0;qs<memfile.quadstrips.size();qs++){
	intbuf= VSSwapHostIntToLittle(memfile.quadstrips[qs].points.size());
	runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Number of elements in current quadstrip
	if (!force_shared_vertex) {
	    intbuf= VSSwapHostIntToLittle(memfile.quadstrips[qs].flatshade);
	    runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Flatshade flag
	}
	for(int32bit tmpcounter=0;tmpcounter<memfile.quadstrips[qs].points.size();tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.quadstrips[qs].points[tmpcounter].indexref);
		runningbytenum+=sizeof(int32bit)*fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//point index
		if (!force_shared_vertex) {
		    floatbuf= VSSwapHostFloatToLittle(memfile.quadstrips[qs].points[tmpcounter].s);
		    runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//s coord
		    floatbuf= VSSwapHostFloatToLittle(memfile.quadstrips[qs].points[tmpcounter].t);
		    runningbytenum+=sizeof(float32bit)*fwrite(&floatbuf,sizeof(float32bit),1,Outputfile);//t coord
		}
	}
  }
  //END GEOMETRY


  fseek(Outputfile,(-1*(runningbytenum))+4,SEEK_CUR);
  intbuf=runningbytenum;
  intbuf= VSSwapHostIntToLittle(intbuf);
  fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Correct number of bytes for total mesh
  fseek(Outputfile,(-1*(runningbytenum-VSAstart)),SEEK_END);
  intbuf=VSAend-VSAstart;
  intbuf= VSSwapHostIntToLittle(intbuf);
  fwrite(&intbuf,sizeof(int32bit),1,Outputfile);//Correct number of bytes for Variable Sized Attribute section
  fseek(Outputfile,0,SEEK_END);
  return runningbytenum;
}


