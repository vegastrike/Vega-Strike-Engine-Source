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
#include "aux_texture.h"
#include "aux_logo.h"
#include "mesh.h"
#include "matrix.h"
#include "camera.h"
#include "bounding_box.h"
#include "bsp.h"
#include <assert.h>
#include <math.h>
#include <list>
#include <string>
#include <fstream>

#include "lin_time.h"
#include "gfxlib.h"

#include "hashtable.h"
#include "vegastrike.h"

#include <GL/gl.h>
using std::list;
Hashtable<string, Mesh, char [127]> Mesh::meshHashTable;
class OrigMeshContainer {
public:
  Mesh * orig;
  OrigMeshContainer (Mesh * tmp) {
    orig = tmp;
  }
  bool operator < (const OrigMeshContainer & b) {
    return ((*orig->Decal) < (*b.orig->Decal));
  }
  bool operator == (const OrigMeshContainer &b) {
    return (*orig->Decal)==*b.orig->Decal;
  }

};
list<OrigMeshContainer> undrawn_meshes[NUM_MESH_SEQUENCE]; // lower priority means draw first
extern list<Logo*> undrawn_logos;
Vector mouseline;






void Mesh::InitUnit() {
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

	Decal = NULL;
	
	alphalist = NULL;
	
	//	texturename[0] = -1;
	numforcelogo = numsquadlogo = 0;
	myMatNum = 0;//default material!
	//	scale = Vector(1.0,1.0,1.0);
	refcount = 1;  //FIXME VEGASTRIKE  THIS _WAS_ zero...NOW ONE
	orig = NULL;
	
	envMap = GFXTRUE;
	draw_queue = NULL;
	will_be_drawn = GFXFALSE;
	draw_sequence = 0;
}

Mesh::Mesh()
{
	InitUnit();
}

bool Mesh::LoadExistant (const char * filehash) {
  Mesh * oldmesh;
  if(0 != (oldmesh = meshHashTable.Get(string (filehash)))) {
    *this = *oldmesh;
    oldmesh->refcount++;
    orig = oldmesh;
    return true;
  }
  return false;
}

Mesh:: Mesh(const char * filename, bool xml, int faction):hash_name(filename)
{
  InitUnit();
  Mesh *oldmesh;
  if (LoadExistant (filename)) {
    return;
  }
  oldmesh = new Mesh();
  meshHashTable.Put(string(filename), oldmesh);
  draw_queue = new vector<MeshDrawContext>;
  
  if(xml) {
    LoadXML(filename,faction);
  } else {
    this->xml= NULL;
    LoadBinary(filename,faction);
  }
  this->orig = oldmesh;
  *oldmesh=*this;
  oldmesh->orig = NULL;
  oldmesh->refcount++;

}

Mesh::~Mesh()
{
	if(!orig||orig==this)
	{
	  delete vlist;
	  if(Decal != NULL) {
	    delete Decal;
	    Decal = NULL;
	  }
	  if (squadlogos!=NULL) {
	    delete squadlogos;
	    squadlogos= NULL;
	  }
	  if (forcelogos!=NULL) {
	    delete forcelogos;
	    forcelogos = NULL;
	  }
	  meshHashTable.Delete(hash_name);
	  
	  if(draw_queue!=NULL)
	    delete draw_queue;
	} else {
	  orig->refcount--;
	  //printf ("orig refcount: %d",refcount);
	  if(orig->refcount == 0)
	    delete orig;
	}
}
float const ooPI = 1.00F/3.1415926535F;

//#include "d3d_internal.h"

void Mesh::SetMaterial (const GFXMaterial & mat) {
  GFXSetMaterial (myMatNum,mat);
  if (orig)
    orig->myMatNum = myMatNum;
}

void Mesh::Draw(const Transformation &trans, const Matrix m)
{
  //  Vector pos (local_pos.Transform(m));
  MeshDrawContext c(m);
  UpdateFX(GetElapsedTime());
  c.SpecialFX = &LocalFX;
  //  c.mat[12]=pos.i;
  //  c.mat[13]=pos.j;
  //  c.mat[14]=pos.k;//to translate to local_pos which is now obsolete!
  orig->draw_queue->push_back(c);
  if(!orig->will_be_drawn) {
    orig->will_be_drawn = GFXTRUE;
    undrawn_meshes[draw_sequence].push_back(OrigMeshContainer(orig));
  }
  will_be_drawn=GFXTRUE;
}


void Mesh::ProcessUndrawnMeshes(bool pushSpecialEffects) {
  GFXEnable(DEPTHWRITE);
  GFXEnable(DEPTHTEST);
  GFXEnable(LIGHTING);
  GFXEnable(CULLFACE);

  for(int a=0; a<NUM_MESH_SEQUENCE; a++) {
    if (a==MESH_SPECIAL_FX_ONLY) {
      GFXPushGlobalEffects();
      GFXDisable(DEPTHWRITE);
    } else {

    }
    undrawn_meshes[a].sort();//sort by texture address
    while(undrawn_meshes[a].size()) {
      Mesh *m = undrawn_meshes[a].back().orig;
      undrawn_meshes[a].pop_back();
      m->ProcessDrawQueue();
      m->will_be_drawn = false;
    }
    if (a==MESH_SPECIAL_FX_ONLY) {
      if (!pushSpecialEffects) {
	GFXPopGlobalEffects();
      }
      GFXEnable(DEPTHWRITE);
    }
  }
  while(undrawn_logos.size()) {
    Logo *l = undrawn_logos.back();
    undrawn_logos.pop_back();
    l->ProcessDrawQueue();
    l->will_be_drawn = false;
    }
}

void Mesh::ProcessDrawQueue() {
  assert(draw_queue->size());
  GFXSelectMaterial(myMatNum);
  if (blendSrc!=SRCALPHA&&blendDst!=ZERO) 
    GFXDisable(DEPTHWRITE);
  GFXBlendMode(blendSrc, blendDst);

  GFXEnable(TEXTURE0);
  if(Decal)
    Decal->MakeActive();
  GFXSelectTexcoordSet(0, 0);
  if(envMap) {
    GFXEnable(TEXTURE1);
    _Universe->activateLightMap();
    GFXSelectTexcoordSet(1, 1);
  } else {
    GFXDisable(TEXTURE1);
  }

  vlist->LoadDrawState();	
  while(draw_queue->size()) {
    MeshDrawContext c = draw_queue->back();
    draw_queue->pop_back();
    if (draw_sequence!=MESH_SPECIAL_FX_ONLY) {
      GFXLoadIdentity(MODEL);
      GFXPickLights (Vector (c.mat[12],c.mat[13],c.mat[14]),rSize());
    }
    GFXLoadMatrix(MODEL, c.mat);
    vector <int> specialfxlight;
    unsigned int i;
    for ( i=0;i<c.SpecialFX->size();i++) {
      int ligh;
      GFXCreateLight (ligh,(*c.SpecialFX)[i],true);
      specialfxlight.push_back(ligh);
    }
    vlist->Draw();

    for ( i=0;i<specialfxlight.size();i++) {
      GFXDeleteLight (specialfxlight[i]);
    }

    if(0!=forcelogos) {
      forcelogos->Draw(c.mat);
    }
    if (0!=squadlogos){
      squadlogos->Draw(c.mat);
    }
  }
  if (blendSrc!=SRCALPHA&&blendDst!=ZERO) 
    GFXEnable(DEPTHWRITE);//risky--for instance logos might be fubar!

}
enum EX_EXCLUSION {EX_X, EX_Y, EX_Z};
inline bool OpenWithin (const Vector &query, const Vector &mn, const Vector &mx, const float err, enum EX_EXCLUSION excludeWhich) {
  switch (excludeWhich) {
  case EX_X:
    return (query.j>=mn.j-err)&&(query.k>=mn.k-err)&&(query.j<=mx.j+err)&&(query.k<=mx.k+err);
  case EX_Y:
    return (query.i>=mn.i-err)&&(query.k>=mn.k-err)&&(query.i<=mx.i+err)&&(query.k<=mx.k+err);
  case EX_Z:
    return (query.j>=mn.j-err)&&(query.i>=mn.i-err)&&(query.j<=mx.j+err)&&(query.i<=mx.i+err);
  }
} 
bool Mesh::queryBoundingBox (const Vector & eye, const Vector & end, const float err) {
  Vector slope (end-eye);
  Vector IntersectXYZ;
  float k = ((mn.i-eye.i)/slope.i);
  IntersectXYZ= eye + k*slope;//(Normal dot (mn-eye)/div)*slope
  if (OpenWithin (IntersectXYZ,mn,mx,err,EX_X))
    return true;
  k = ((mx.i-eye.i)/slope.i);
  if (k>=0) {
    IntersectXYZ = eye + k*slope;
    if (OpenWithin (IntersectXYZ,mn,mx,err,EX_X))
      return true;
  }
  if (k>=0) {
    k=((mn.j-eye.j)/slope.j);
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
bool Mesh::queryBoundingBox (const Vector & start,float err) {
  return start.i>=mn.i-err&&start.j>=mn.j-err&&start.k>=mn.k-err&&
         start.i<=mx.i+err&&start.j<=mx.j+err&&start.k<=mx.k+err;
}

BoundingBox * Mesh::getBoundingBox() {
  
  BoundingBox * tbox = new BoundingBox (Vector (mn.i,0,0),Vector (mx.i,0,0),
					Vector (0,mn.j,0),Vector (0,mx.j,0),
					Vector (0,0,mn.k),Vector (0,0,mx.k));
  return tbox;
}
