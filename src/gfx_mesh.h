/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _MESH_H_
#define _MESH_H_

#include "gfx_primitive.h"

//#include "gfx_vertex.h"
//#include "gfx_bsp.h"
//#include "gfx_bounding_box.h"
//#include "gfxlib.h"
//#include "gfx_aux_logo.h"

#include <string>
#include <vector>
#include "xml_support.h"
using namespace std;

class Planet;
class BSPTree;
class GFXVertex;
class GFXVertexList;
class GFXQuadstrip;
class GFXMaterial;
class BoundingBox;

using XMLSupport::EnumMap;
using XMLSupport::AttributeList;

class Mesh:public Primitive
{
private:
  enum ElementNames {
    UNKNOWN, MESH, POINTS, POINT, LOCATION, NORMAL, POLYGONS, TRI, QUAD, VERTEX
  };
  vector<ElementNames> xml_state;
  static const EnumMap::Pair element_names[];
  static const EnumMap element_map;

  void LoadXML(char *filename);

  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  
  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);

protected:
	int refcount;
	float maxSizeX,maxSizeY,maxSizeZ,minSizeX,minSizeY,minSizeZ;
	float radialSize;
	Mesh *orig;

	int numvertex;
	//GFXVertex *vertexlist;
	float *stcoords;
	//GFXVertex *alphalist;
	GFXVertex *vertexlist;

	GFXVertexList *vlist;
	
	GFXQuadstrip **quadstrips;
	int numQuadstrips;
	
	GFXMaterial myMat;
	int myMatNum;

	int numtris;
	int numquads;
	int numlines;

  //	Vector scale; //scale that mofo in the file...no reason to have moster hellcat
 
	BOOL objtex;
	Texture *Decal;
	BOOL envMap;
  //	int texturename[2];

	BSPTree *bspTree;

	BOOL changed;
	float ymin, ymax, ycur;
	BOOL yrestricted;
	float pmin, pmax, pcur;
	BOOL prestricted;
	float rmin, rmax, rcur;
	BOOL rrestricted;

	void InitUnit();

	void SetOrientation2();
	
	void Reflect ();

	Vector p,q,r;
	Vector pp, pq, pr, ppos;
public:
	Mesh();
	Mesh(char *filename,  bool xml=false);
	~Mesh();

	virtual void Draw();
	virtual void Draw(const Vector &pp, const Vector &pq, const Vector &pr, const Vector &ppos);

	void setEnvMap(BOOL newValue) {envMap = newValue;}

	void SetOrientation(Vector &p, Vector &q, Vector &r);
	void SetOrientation();

	void RestrictYaw(float min, float max);
	void RestrictPitch(float min, float max);
	void RestrictRoll(float min, float max);

	BOOL Yaw(float rad);
	BOOL Pitch(float rad);
	BOOL Roll(float rad);
	void Destroy();

	void UpdateMatrix();
  void UpdateHudMatrix();//puts an object on the hud with the matrix
	void SetPosition(float x,float y,float z);
	void SetPosition(const Vector &origin);
	void SetPosition();

	void XSlide(float factor);
	void YSlide(float factor);
	void ZSlide(float factor);

	void Rotate(const Vector &torque);

  // void Scale(const Vector &scale) {this->scale = scale;SetOrientation();};
  BoundingBox * getBoundingBox();
  float rSize () {return radialSize;}
	bool intersects(const Vector &start, const Vector &end);
	bool intersects(const Vector &pt);
	bool intersects(Matrix t, const Vector &pt);
	bool intersects(Mesh *mesh);
	bool intersects(Matrix t, Mesh *mesh);
};
#endif
















