#include "gfx_mesh.h"

#include <iostream.h>
#include <fstream.h>
#include <expat.h>
#include <values.h>
#include "xml_support.h"

#ifdef max
#undefine max
#endif

static inline float max(float x, float y) {
  if(x>y) return x;
  else return y;
}

#ifdef min
#undefine min
#endif

static inline float min(float x, float y) {
  if(x<y) return x;
  else return y;
}

const float scale=0.06;

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_int;

const EnumMap::Pair Mesh::XML::element_names[] = {
  {"UNKNOWN", XML::UNKNOWN},
  {"Mesh", XML::MESH},
  {"Points", XML::POINTS},
  {"Point", XML::POINT},
  {"Location", XML::LOCATION},
  {"Normal", XML::NORMAL},
  {"Polygons", XML::POLYGONS},
  {"Tri", XML::TRI},
  {"Quad", XML::QUAD},
  {"Vertex", XML::VERTEX}};

const EnumMap::Pair Mesh::XML::attribute_names[] = {
  {"UNKNOWN", XML::UNKNOWN},
  {"texture", XML::TEXTURE},
  {"x", XML::X},
  {"y", XML::Y},
  {"z", XML::Z},
  {"i", XML::I},
  {"j", XML::J},
  {"k", XML::K},
  {"point", XML::POINT},
  {"s", XML::S},
  {"t", XML::T}};

const EnumMap Mesh::XML::element_map(XML::element_names, 10);
const EnumMap Mesh::XML::attribute_map(XML::attribute_names, 11);

void Mesh::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  ((Mesh*)userData)->beginElement(name, AttributeList(atts));
}

void Mesh::endElement(void *userData, const XML_Char *name) {
  ((Mesh*)userData)->endElement(name);
}

/* Load stages:
0 - no tags seen
1 - waiting for points
2 - processing points 
3 - done processing points, waiting for face data

Note that this is only a debugging aid. Once DTD is written, there
will be no need for this sort of checking
 */

void Mesh::beginElement(const string &name, const AttributeList &attributes) {
  //cerr << "Start tag: " << name << endl;

  XML::Names elem = (XML::Names)XML::element_map.lookup(name);
  XML::Names top;
  if(xml->state_stack.size()>0) top = *xml->state_stack.rbegin();
  xml->state_stack.push_back(elem);

  bool texture_found = false;
  switch(elem) {
  case XML::UNKNOWN:
    cerr << "Unknown element start tag '" << name << "' detected " << endl;
    break;
  case XML::MESH:
    assert(xml->load_stage == 0);
    assert(xml->state_stack.size()==1);

    xml->load_stage = 1;
    // Read in texture attribute
    
    for(AttributeList::const_iterator iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::TEXTURE:
	xml->decal_name = (*iter).value;
	texture_found = true;
	goto texture_done;
      }
    }
  texture_done:
    assert(texture_found);
    break;
  case XML::POINTS:
    assert(top==XML::MESH);
    assert(xml->load_stage == 1);

    xml->load_stage = 2;
    break;
  case XML::POINT:
    assert(top==XML::POINTS);
    
    memset(&xml->vertex, 0, sizeof(xml->vertex));
    xml->point_state = 0; // Point state is used to check that all necessary attributes are recorded
    break;
  case XML::LOCATION:
    assert(top==XML::POINT);

    for(AttributeList::const_iterator iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::UNKNOWN:
	cerr << "Unknown attribute '" << (*iter).name << "' encountered in Location tag" << endl;
	break;
      case XML::X:
	assert(!(xml->point_state & XML::P_X));
	xml->vertex.x = parse_float((*iter).value);
	xml->point_state |= XML::P_X;
	break;
      case XML::Y:
	assert(!(xml->point_state & XML::P_Y));
	xml->vertex.y = parse_float((*iter).value);
	xml->point_state |= XML::P_Y;
	break;
      case XML::Z:
	assert(!(xml->point_state & XML::P_Z));
	xml->vertex.z = parse_float((*iter).value);
	xml->point_state |= XML::P_Z;
	break;
      default:
	assert(0);
      }
    }
    assert(xml->point_state & (XML::P_X |
			       XML::P_Y |
			       XML::P_Z) == 
	   (XML::P_X |
	    XML::P_Y |
	    XML::P_Z) );
    break;
  case XML::NORMAL:
    assert(top==XML::POINT);

    for(AttributeList::const_iterator iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::UNKNOWN:
	cerr << "Unknown attribute '" << (*iter).name << "' encountered in Normal tag" << endl;
	break;
      case XML::I:
	assert(!(xml->point_state & XML::P_I));
	xml->vertex.i = parse_float((*iter).value);
	xml->point_state |= XML::P_I;
	break;
      case XML::J:
	assert(!(xml->point_state & XML::P_J));
	xml->vertex.j = parse_float((*iter).value);
	xml->point_state |= XML::P_J;
	break;
      case XML::K:
	assert(!(xml->point_state & XML::P_K));
	xml->vertex.k = parse_float((*iter).value);
	xml->point_state |= XML::P_K;
	break;
      default:
	assert(0);
      }
    }
    assert(xml->point_state & (XML::P_I |
			       XML::P_J |
			       XML::P_K) == 
	   (XML::P_I |
	    XML::P_J |
	    XML::P_K) );
    break;
  case XML::POLYGONS:
    assert(top==XML::MESH);
    assert(xml->load_stage==3);

    xml->load_stage = 4;
    break;
  case XML::TRI:
    assert(top==XML::POLYGONS);
    assert(xml->load_stage==4);

    xml->num_vertices=3;
    xml->active_list = &xml->tris;
    break;
  case XML::QUAD:
    assert(top==XML::POLYGONS);
    assert(xml->load_stage==4);

    xml->num_vertices=4;
    xml->active_list = &xml->quads;
    break;
  case XML::VERTEX:
    assert(top==XML::TRI || top==XML::QUAD);
    assert(xml->load_stage==4);

    xml->vertex_state = 0;
    unsigned int index;
    float s, t;
    for(AttributeList::const_iterator iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::UNKNOWN:
	cerr << "Unknown attribute '" << (*iter).name << "' encountered in Vertex tag" << endl;
	break;
      case XML::POINT:
	assert(!(xml->vertex_state & XML::V_POINT));
	xml->vertex_state |= XML::V_POINT;
	index = parse_int((*iter).value);
	break;
      case XML::S:
	assert(!(xml->vertex_state & XML::V_S));
	xml->vertex_state |= XML::V_S;
	s = parse_float((*iter).value);
	break;
      case XML::T:
	assert(!(xml->vertex_state & XML::V_T));
	xml->vertex_state |= XML::V_T;
	t = parse_float((*iter).value);
	break;
      default:
	assert(0);
      }
    }
    assert(xml->vertex_state & (XML::V_POINT|
				XML::V_S|
				XML::V_T) == 
	   (XML::V_POINT|
	    XML::V_S|
	    XML::V_T) );
    assert(index < xml->vertices.size());

    memset(&xml->vertex, 0, sizeof(xml->vertex));
    xml->vertex = xml->vertices[index];
    xml->vertex.x *= scale;
    xml->vertex.y *= scale;
    xml->vertex.z *= scale;
    xml->vertex.s = s;
    xml->vertex.t = t;
    xml->active_list->push_back(xml->vertex);
    xml->num_vertices--;
    break;
  default:
    assert(0);
  }
}

void Mesh::endElement(const string &name) {
  //cerr << "End tag: " << name << endl;

  XML::Names elem = (XML::Names)XML::element_map.lookup(name);
  assert(*xml->state_stack.rbegin() == elem);
  xml->state_stack.pop_back();

  switch(elem) {
  case XML::UNKNOWN:
    cerr << "Unknown element end tag '" << name << "' detected " << endl;
    break;
  case XML::POINT:
    assert(xml->point_state & (XML::P_X | 
			       XML::P_Y | 
			       XML::P_Z |
			       XML::P_I |
			       XML::P_J |
			       XML::P_K) == 
	   (XML::P_X | 
	    XML::P_Y | 
	    XML::P_Z |
	    XML::P_I |
	    XML::P_J |
	    XML::P_K) );
    xml->vertices.push_back(xml->vertex);
    break;
  case XML::POINTS:
    xml->load_stage = 3;

    /*
    cerr << xml->vertices.size() << " vertices\n";
    for(int a=0; a<xml->vertices.size(); a++) {
      clog << "Point: (" << xml->vertices[a].x << ", " << xml->vertices[a].y << ", " << xml->vertices[a].z << ") (" << xml->vertices[a].i << ", " << xml->vertices[a].j << ", " << xml->vertices[a].k << ")\n";
    }
    clog << endl;
    */
    break;
  case XML::TRI:
    assert(xml->num_vertices==0);
    break;
  case XML::QUAD:
    assert(xml->num_vertices==0);
    break;
  case XML::POLYGONS:
    assert(xml->tris.size()%3==0);
    assert(xml->quads.size()%4==0);
    
    /*
    cerr << xml->tris.size()/3 << " triangles\n";
    cerr << xml->quads.size()/4 << " quadrilaterals\n";
    for(int a=0; a<xml->tris.size(); a++) {
      if(a%3==0) {
	clog << "Triangle\n";
      }
      clog << "    (" << xml->tris[a].x << ", " << xml->tris[a].y << ", " << xml->tris[a].z << ") (" << xml->tris[a].i << ", " << xml->tris[a].j << ", " << xml->tris[a].k << ") (" << xml->tris[a].s << ", " << xml->tris[a].t << ")\n";
    }
    clog << "** ** ** Quadrilaterals ** ** **\n";
    for(int a=0; a<xml->quads.size(); a++) {
      if(a%4==0) {
	clog << "Quadrilateral\n";
      }
      clog << "    (" << xml->quads[a].x << ", " << xml->quads[a].y << ", " << xml->quads[a].z << ") (" << xml->quads[a].i << ", " << xml->quads[a].j << ", " << xml->quads[a].k << ") (" << xml->quads[a].s << ", " << xml->quads[a].t << ")\n";
    }
    clog << endl;
*/
    break;
  case XML::MESH:
    assert(xml->load_stage==4);

    xml->load_stage=5;
    break;
  default:
    ;
  }
}

void Mesh::LoadXML(const char *filename, Mesh *oldmesh) {
  const int chunk_size = 16384;
  
  ifstream inFile(filename, ios::in | ios::binary);
  if(!inFile) {
    assert(0);
    return;
  }

  xml = new XML;
  xml->load_stage = 0;

  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &Mesh::beginElement, &Mesh::endElement);
  
  do {
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
    int length;
    
    inFile.read(buf, chunk_size);
    length = inFile.gcount();
    XML_ParseBuffer(parser, length, inFile.eof());
  } while(!inFile.eof());

  // Now, copy everything into the mesh data structures
  assert(xml->load_stage==5);

  // TODO: add alpha handling
  Decal = new Texture(xml->decal_name.c_str(), 0);

  int index = 0;
  vertexlist = new GFXVertex[xml->tris.size()+xml->quads.size()];
  minSizeX = minSizeY = minSizeZ = FLT_MAX;
  maxSizeX = maxSizeY = maxSizeZ = -FLT_MAX;
  if (xml->tris.size()==0&&xml->quads.size()==0) {
    minSizeX = minSizeY = minSizeZ = 0;
    maxSizeX = maxSizeY = maxSizeZ = 0;
    fprintf (stderr, "uhoh");
  }
  radialSize = 0;
  for(int a=0; a<xml->tris.size(); a++, index++) {
    vertexlist[index] = xml->tris[a];
    minSizeX = min(vertexlist[index].x, minSizeX);
    maxSizeX = max(vertexlist[index].x, maxSizeX);
    minSizeY = min(vertexlist[index].y, minSizeY);
    maxSizeY = max(vertexlist[index].y, maxSizeY);
    minSizeZ = min(vertexlist[index].z, minSizeZ);
    maxSizeZ = max(vertexlist[index].z, maxSizeZ);
  }
  for(int a=0; a<xml->quads.size(); a++, index++) {
    vertexlist[index] = xml->quads[a];
    minSizeX = min(vertexlist[index].x, minSizeX);
    maxSizeX = max(vertexlist[index].x, maxSizeX);
    minSizeY = min(vertexlist[index].y, minSizeY);
    maxSizeY = max(vertexlist[index].y, maxSizeY);
    minSizeZ = min(vertexlist[index].z, minSizeZ);
    maxSizeZ = max(vertexlist[index].z, maxSizeZ);
  }
  float x_center = (minSizeX + maxSizeX)/2.0,
    y_center = (minSizeY + maxSizeY)/2.0,
    z_center = (minSizeZ + maxSizeZ)/2.0;
  SetPosition(x_center, y_center, z_center);
  for(int a=0; a<xml->tris.size()+xml->quads.size(); a++) {
    vertexlist[a].x -= x_center;
    vertexlist[a].y -= y_center;
    vertexlist[a].z -= z_center;
  }

  minSizeX -= x_center;
  maxSizeX -= x_center;
  minSizeY -= y_center;
  maxSizeY -= y_center;
  minSizeZ -= z_center;
  maxSizeZ -= z_center;
  
  radialSize = .5*sqrtf ((maxSizeX-minSizeX)*(maxSizeX-minSizeX)+(maxSizeY-minSizeY)*(maxSizeY-minSizeY)+(maxSizeX-minSizeZ)*(maxSizeX-minSizeZ));

  vlist = new GFXVertexList(xml->tris.size() + xml->quads.size(),
			    xml->tris.size()/3, xml->quads.size()/4,
			    vertexlist);
 
  //TODO: add force handling


  if (radialSize==0) {
    int i;
  }

  // Calculate bounding sphere

  this->orig = oldmesh;
  *oldmesh=*this;
  oldmesh->orig = NULL;
  oldmesh->refcount++;
  fprintf (stderr, "Minx %f maxx %f, miny %f maxy %fminz %fmaxz %f, radsiz %f\n",minSizeX, maxSizeX,  minSizeY, maxSizeY,  minSizeZ, maxSizeZ,radialSize);  
  delete [] vertexlist;
  delete xml;
}
