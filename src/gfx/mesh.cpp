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
#include <memory.h>
#include "animation.h"
#include "aux_logo.h"
#include "mesh.h"
#include "matrix.h"
#include "camera.h"
#include "bounding_box.h"
#include "bsp.h"
#include <assert.h>
#include <math.h>
#include "cmd/nebula_generic.h"
#include <list>
#include <string>
#include <fstream>
#include "vs_path.h"
#include "lin_time.h"
#include "gfxlib.h"
#include "vs_globals.h"
#include "configxml.h"
#include "hashtable.h"
#include "vegastrike.h"
#include "sphere.h"
#if defined(__APPLE__) || defined(MACOSX)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif
#include <float.h>
#include <algorithm>
using std::list;
Hashtable<string, Mesh, char [127]> Mesh::meshHashTable;
Vector mouseline;



void Mesh::InitUnit() {
	polygon_offset=0;
  numlods=1;
  lodsize=FLT_MAX;
	forcelogos = NULL;
	squadlogos = NULL;
	local_pos = Vector (0,0,0);
	blendSrc=ONE;
	blendDst=ZERO;
	vlist=NULL;
	mn = Vector (0,0,0);
	mx = Vector (0,0,0);
	radialSize=0;
	GFXVertex *alphalist;
	if (Decal.empty())
	  Decal.push_back (NULL);
	
	alphalist = NULL;
	
	//	texturename[0] = -1;
	numforcelogo = numsquadlogo = 0;
	myMatNum = 0;//default material!
	//	scale = Vector(1.0,1.0,1.0);
	refcount = 1;  //FIXME VEGASTRIKE  THIS _WAS_ zero...NOW ONE
	orig = NULL;
	
	envMapAndLit =0x3;
	setEnvMap(GFXTRUE);
	setLighting(GFXTRUE);
	
	draw_queue = NULL;
	will_be_drawn = GFXFALSE;
	draw_sequence = 0;
}
Mesh::Mesh()
{
	InitUnit();
}
bool Mesh::LoadExistant (Mesh * oldmesh) {
    *this = *oldmesh;
    oldmesh->refcount++;
    orig = oldmesh;
    return true;
}
Vector Mesh::GetVertex (int index) const {
	if (vlist->hasColor()) {
		const GFXColorVertex * v=vlist->GetColorVertex(index);
		return Vector(v->x,v->y,v->z);
	}else {
		const GFXVertex * v=vlist->GetVertex(index);
		return Vector(v->x,v->y,v->z);
	}
}
int Mesh::numVertices() const {
	return vlist->GetNumVertices();
}
bool Mesh::LoadExistant (const string filehash, const Vector& scale, int faction) {
  Mesh * oldmesh;

  hash_name = GetHashName (filehash,scale,faction);
  oldmesh = meshHashTable.Get(hash_name);

  if (oldmesh==0) {
    hash_name =GetSharedMeshHashName(filehash,scale,faction);
    oldmesh = meshHashTable.Get(hash_name);  
  }
  if(0 != oldmesh) {
    return LoadExistant(oldmesh);
  }
  //  fprintf (stderr,"cannot cache %s",GetSharedMeshHashName(filehash,scale,faction).c_str());
  return false;
}
Mesh::Mesh (const Mesh & m) {
  this->orig=NULL;
  this->hash_name = m.hash_name;
  InitUnit();
  Mesh * oldmesh = meshHashTable.Get (hash_name);
  if (LoadExistant (oldmesh->orig!=NULL?oldmesh->orig:oldmesh)) {
    return;
  }
}
Mesh:: Mesh(const char * filename,const Vector & scale, int faction, Flightgroup *fg, bool orig):hash_name(filename)
{
  this->orig=NULL;
  InitUnit();
  Mesh *oldmesh;
  if (LoadExistant (filename,scale,faction)) {
    return;
  }
  bool shared=false;
  FILE * fp= fopen (filename,"r");
  if (fp==NULL) {
    shared=true;
  }else {
    fclose (fp);
  }
  bool xml=true;
  if(xml) {
    LoadXML(shared?GetSharedMeshPath(filename).c_str():filename,scale,faction,fg,orig);
    oldmesh = this->orig;
  } else {
    this->xml= NULL;
    LoadBinary(shared?GetSharedMeshPath(filename).c_str():filename,faction);
    oldmesh = new Mesh[1];
  }
  draw_queue = new vector<MeshDrawContext>;
  if (!orig) {
    hash_name =shared?GetSharedMeshHashName (filename,scale,faction):GetHashName(filename,scale,faction);
    meshHashTable.Put(hash_name, oldmesh);
    //oldmesh[0]=*this;
    *oldmesh=*this;
    oldmesh->orig = NULL;
    oldmesh->refcount++;
  } else {
    this->orig=NULL;
  }
}
float const ooPI = 1.00F/3.1415926535F;
//#include "d3d_internal.h"
void Mesh::SetMaterial (const GFXMaterial & mat) {
  GFXSetMaterial (myMatNum,mat);
  if (orig) {
    for (int i=0;i<numlods;i++) {
      orig[i].myMatNum = myMatNum;
    }
  }
}
Mesh * Mesh::getLOD (float lod) {
  if (!orig)
    return this;
  Mesh * retval =&orig[0];
  for (int i=1;i<numlods;i++) {
    if (lod<orig[i].lodsize) {
      retval = &orig[i];
    } else {
      break;
    }
  }
  return retval;
}



void Mesh::SetBlendMode (BLENDFUNC src, BLENDFUNC dst) {
  blendSrc = src;
  blendDst = dst;
  draw_sequence=0;
  if (blendDst!=ZERO) {
    draw_sequence++;
    if (blendDst!=ONE)
      draw_sequence++;
  }
  if (orig) {
    orig->draw_sequence = draw_sequence;
    orig->blendSrc = src;
    orig->blendDst = dst;
  }
}
enum EX_EXCLUSION {EX_X, EX_Y, EX_Z};
inline bool OpenWithin (const QVector &query, const Vector &mn, const Vector &mx, const float err, enum EX_EXCLUSION excludeWhich) {
  switch (excludeWhich) {
  case EX_X:
    return (query.j>=mn.j-err)&&(query.k>=mn.k-err)&&(query.j<=mx.j+err)&&(query.k<=mx.k+err);
  case EX_Y:
    return (query.i>=mn.i-err)&&(query.k>=mn.k-err)&&(query.i<=mx.i+err)&&(query.k<=mx.k+err);
  case EX_Z:
  default:
    return (query.j>=mn.j-err)&&(query.i>=mn.i-err)&&(query.j<=mx.j+err)&&(query.i<=mx.i+err);
  }
} 
bool Mesh::queryBoundingBox (const QVector & eye, const QVector & end, const float err) {
  QVector slope (end-eye);
  QVector IntersectXYZ;
  double k = ((mn.i-eye.i)/slope.i);
  IntersectXYZ= eye + k*slope;//(Normal dot (mn-eye)/div)*slope
  if (OpenWithin (IntersectXYZ,mn,mx,err,EX_X))
    return true;
  k = ((mx.i-eye.i)/slope.i);
  if (k>=0) {
    IntersectXYZ = eye + k*slope;
    if (OpenWithin (IntersectXYZ,mn,mx,err,EX_X))
      return true;
  }
  k=((mn.j-eye.j)/slope.j);
  if (k>=0) {
    IntersectXYZ = eye + k*slope;
    if (OpenWithin (IntersectXYZ,mn,mx,err,EX_Y))
      return true;
  }
  k=((mx.j-eye.j)/slope.j);
  if (k>=0) {
    IntersectXYZ = eye + k*slope;
    if (OpenWithin (IntersectXYZ,mn,mx,err,EX_Y)) 
      return true;
  }
  k=((mn.k-eye.k)/slope.k);
  if (k>=0) {
    IntersectXYZ = eye + k*slope;
    if (OpenWithin (IntersectXYZ,mn,mx,err,EX_Z))     
      return true;
  }
  k=((mx.k-eye.k)/slope.k);
  if (k>=0) {
    IntersectXYZ = eye + k*slope;
    if (OpenWithin (IntersectXYZ,mn,mx,err,EX_Z)) 
      return true;
  }
  return false;
  
}
bool Mesh::queryBoundingBox (const QVector & start,const float err) {
  return start.i>=mn.i-err&&start.j>=mn.j-err&&start.k>=mn.k-err&&
         start.i<=mx.i+err&&start.j<=mx.j+err&&start.k<=mx.k+err;
}
BoundingBox * Mesh::getBoundingBox() {
  
  BoundingBox * tbox = new BoundingBox (QVector (mn.i,0,0),QVector (mx.i,0,0),
					QVector (0,mn.j,0),QVector (0,mx.j,0),
					QVector (0,0,mn.k),QVector (0,0,mx.k));
  return tbox;
}

