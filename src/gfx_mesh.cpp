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
#include "gfx_aux.h"
#include "gfx_mesh.h"
#include "gfx_transform.h"
#include "gfx_aux.h"
#include "gfx_camera.h"
#include "gfx_bounding_box.h"
#include "gfx_bsp.h"
#include <assert.h>

#include <math.h>
#include <list>
#include <string>
#include <fstream>


#include "gfxlib.h"

#include "hashtable.h"
#include "vegastrike.h"

#include <GL/gl.h>
using std::list;
Hashtable<string, Mesh, char [513]> Mesh::meshHashTable;
class OrigMeshContainer {
public:
  Mesh * orig;
  OrigMeshContainer (Mesh * tmp) {
    orig = tmp;
  }
  bool operator < (const OrigMeshContainer & b) {
    return (orig->Decal < b.orig->Decal);
  }
  bool operator == (const OrigMeshContainer &b) {
    return orig->Decal==b.orig->Decal;
  }

};
list<OrigMeshContainer> undrawn_meshes[NUM_MESH_SEQUENCE]; // lower priority means draw first
extern list<Logo*> undrawn_logos;
Vector mouseline;


void Mesh::SetPosition (const Vector &k) {
  local_pos = k;
}

void Mesh::ProcessUndrawnMeshes() {
  GFXEnable(DEPTHWRITE);
  GFXEnable(DEPTHTEST);
  for(int a=0; a<NUM_MESH_SEQUENCE; a++) {
    undrawn_meshes[a].sort();//sort by texture address
    while(undrawn_meshes[a].size()) {
      Mesh *m = undrawn_meshes[a].back().orig;
      undrawn_meshes[a].pop_back();
      m->ProcessDrawQueue();
      m->will_be_drawn = false;
    }
  }
  while(undrawn_logos.size()) {
    Logo *l = undrawn_logos.back();
    undrawn_logos.pop_back();
    l->ProcessDrawQueue();
    l->will_be_drawn = false;
    }
}



void Mesh::InitUnit() {
	forcelogos = NULL;
	squadlogos = NULL;

	local_pos = Vector (0,0,0);
	blendSrc=ONE;
	blendDst=ZERO;
	vlist=NULL;
	
	radialSize=minSizeX=minSizeY=minSizeZ=maxSizeY=maxSizeZ=maxSizeX=0;
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
	will_be_drawn = false;
	draw_sequence = 0;
}

Mesh::Mesh()
{
	InitUnit();
}

Mesh:: Mesh(const char * filename, bool xml):hash_name(filename)
{
  InitUnit();
  Mesh *oldmesh;
  if(0 != (oldmesh = meshHashTable.Get(string(filename)))) {
    *this = *oldmesh;
    oldmesh->refcount++;
    orig = oldmesh;
    return;
  } else {
    //oldmesh = (Mesh*)malloc(sizeof(Mesh));
    oldmesh = new Mesh();
    meshHashTable.Put(string(filename), oldmesh);
    draw_queue = new vector<MeshDrawContext>;
  }
  if(xml) {
    LoadXML(filename, oldmesh);
  } else {
    this->xml= NULL;
    LoadBinary(filename,oldmesh);
  }
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

void Mesh::Draw(const Transformation &trans, const Matrix m)
{
  Vector pos (local_pos.Transform(m));
  MeshDrawContext c(m);
  c.mat[12]=pos.i;
  c.mat[13]=pos.j;
  c.mat[14]=pos.k;
  orig->draw_queue->push_back(c);
  if(!orig->will_be_drawn) {
    orig->will_be_drawn = true;
    undrawn_meshes[draw_sequence].push_back(OrigMeshContainer(orig));
  }
}

void Mesh::ProcessDrawQueue() {
  assert(draw_queue->size());
  GFXSelectMaterial(myMatNum);
  //GFXEnable(LIGHTING);
  GFXDisable (LIGHTING);
  
  GFXEnable(TEXTURE0);
  GFXEnable(CULLFACE);
  if(envMap) {
    GFXEnable(TEXTURE1);
  } else {
    GFXDisable(TEXTURE1);
  }
  if(Decal)
    Decal->MakeActive();
  
  GFXSelectTexcoordSet(0, 0);
  if(envMap) {
    _Universe->activateLightMap();
    GFXSelectTexcoordSet(1, 1);
  }
  GFXBlendMode(blendSrc, blendDst);
  vlist->LoadDrawState();	
  while(draw_queue->size()) {
    MeshDrawContext c = draw_queue->back();
    draw_queue->pop_back();
    GFXLoadMatrix(MODEL, c.mat);
    GFXPickLights (c.mat/*GetPosition()*/);
    vlist->Draw();
    if(0!=forcelogos) {
      forcelogos->Draw(c.mat);
    }
    if (0!=squadlogos){
      squadlogos->Draw(c.mat);
    }
  }
}

bool queryBoundingBox (const Vector & start, const Vector & end) {
  //normal = 

}
bool queryBoundingBox (const Vector & start) {


}

BoundingBox * Mesh::getBoundingBox() {
  
  BoundingBox * tbox = new BoundingBox (Vector (minSizeX,0,0)+local_pos,Vector (maxSizeX,0,0)+local_pos,
					Vector (0,minSizeY,0)+local_pos,Vector (0,maxSizeY,0)+local_pos,
					Vector (0,0,minSizeZ)+local_pos,Vector (0,0,maxSizeZ)+local_pos);
  return tbox;
}
