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
#include "vs_path.h"
#include "lin_time.h"
#include "gfxlib.h"

#include "hashtable.h"
#include "vegastrike.h"

#include <GL/gl.h>
#include <float.h>
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
  oldmesh = meshHashTable.Get(GetHashName(filehash));
  if (oldmesh==0) {
    oldmesh = meshHashTable.Get(GetSharedMeshHashName(filehash));  
  }
  if(0 != oldmesh) {
    *this = *oldmesh;
    oldmesh->refcount++;
    orig = oldmesh;
    return true;
  }
  return false;
}

Mesh:: Mesh(const char * filename, bool xml, int faction, bool orig):hash_name(filename)
{
  InitUnit();
  Mesh *oldmesh;
  if (LoadExistant (filename)) {
    return;
  }
  bool shared=false;
  FILE * fp= fopen (filename,"r");
  if (fp==NULL) {
    shared=true;
  }else {
    fclose (fp);
  }

  if(xml) {
    LoadXML(shared?GetSharedMeshPath(filename).c_str():filename,faction);
    oldmesh = this->orig;
  } else {
    this->xml= NULL;
    LoadBinary(shared?GetSharedMeshPath(filename).c_str():filename,faction);
    oldmesh = new Mesh[1];
  }
  draw_queue = new vector<MeshDrawContext>;
  if (!orig) {
    hash_name =shared?GetSharedMeshHashName (filename):GetHashName(filename);
    meshHashTable.Put(hash_name, oldmesh);
    oldmesh[0]=*this;
    oldmesh->orig = NULL;
    oldmesh->refcount++;
  } else {
    this->orig=NULL;
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
	  if(orig->refcount == 0) {
	    delete [] orig;	      
	  }
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
void Mesh::Draw(float lod, const Transformation &trans, const Matrix m, short cloak, float nebdist)
{
  //  Vector pos (local_pos.Transform(m));
  MeshDrawContext c(m);
  UpdateFX(GetElapsedTime());
  c.SpecialFX = &LocalFX;
  c.mesh_seq=draw_sequence;
  c.cloaked=MeshDrawContext::NONE;
  if (cloak>=0) {
    c.cloaked|=MeshDrawContext::CLOAK;
    if (cloak<16384) {
      c.cloaked|=MeshDrawContext::NEARINVIS;
    }
    c.mesh_seq=MESH_SPECIAL_FX_ONLY;//draw near the end with lights
    float tmp = ((float)cloak)/32767;
    c.CloakFX.r = tmp;
    c.CloakFX.g = tmp;
    c.CloakFX.b = tmp;
    c.CloakFX.a = tmp;
    /*
    c.CloakNebFX.ambient[0]=((float)cloak)/32767;
    c.CloakNebFX.ag=((float)cloak)/32767;
    c.CloakNebFX.ab=((float)cloak)/32767;
    c.CloakNebFX.aa=((float)cloak)/32767;
    */
    ///all else == defaults, only ambient
  }
  //  c.mat[12]=pos.i;
  //  c.mat[13]=pos.j;
  //  c.mat[14]=pos.k;//to translate to local_pos which is now obsolete!
  Mesh *origmesh = getLOD (lod);
  origmesh->draw_queue->push_back(c);
  if(!(origmesh->will_be_drawn&(1<<c.mesh_seq))) {
    origmesh->will_be_drawn |= (1<<c.mesh_seq);
    undrawn_meshes[c.mesh_seq].push_back(OrigMeshContainer(origmesh));
  }
  will_be_drawn |= (1<<c.mesh_seq);
}
void Mesh::DrawNow(float lod,  bool centered, const Transformation &transform /*= identity_transformation*/, const Matrix m, short cloak, float nebdist) {
  Mesh *o = getLOD (lod);
  //fixme: cloaking not delt with.... not needed for backgroudn anyway
  if (centered) {
    float m1[16];
    memcpy (m1,m,sizeof (float)*16);
    Vector pos(_Universe->AccessCamera()->GetPosition().Transform(m1));
    m1[12]=pos.i;
    m1[13]=pos.j;
    m1[14]=pos.k;
    Transformation tmp = transform;
    tmp.position = pos;
    GFXLoadMatrix (MODEL,m1);    
  } else {	
    if (o->draw_sequence!=MESH_SPECIAL_FX_ONLY) {
      GFXLoadIdentity(MODEL);
      GFXPickLights (Vector (m[12],m[13],m[14]),rSize());
    }
    GFXLoadMatrix (MODEL,m);
  } 
  vector <int> specialfxlight;
  unsigned int i;
  for ( i=0;i<LocalFX.size();i++) {
    int ligh;
    GFXCreateLight (ligh,(LocalFX)[i],true);
    specialfxlight.push_back(ligh);
  }

  GFXSelectMaterial(o->myMatNum);
  if (blendSrc!=SRCALPHA&&blendDst!=ZERO) 
    GFXDisable(DEPTHWRITE);
  GFXBlendMode(blendSrc, blendDst);

  if (o->Decal)
    o->Decal->MakeActive();
  o->vlist->DrawOnce();
  for ( i=0;i<specialfxlight.size();i++) {
    GFXDeleteLight (specialfxlight[i]);
  }
  if (cloak>=0&&cloak<32767) {
    GFXEnable (TEXTURE1);
  }
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
    if (!undrawn_meshes[a].empty()) {
      undrawn_meshes[a].back().orig->vlist->LoadDrawState();
    }
    while(undrawn_meshes[a].size()) {
      Mesh *m = undrawn_meshes[a].back().orig;
      undrawn_meshes[a].pop_back();
      m->ProcessDrawQueue(a);
      m->will_be_drawn &= (~(1<<a));
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

void Mesh::ProcessDrawQueue(int whichdrawqueue) {
  //  assert(draw_queue->size());
  if (draw_queue->empty()) {
    fprintf (stderr,"cloaking queues issue! Report to hellcatv@hotmail.com\nn%d\n%s",whichdrawqueue,hash_name.c_str());
    return;
  }
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

  vlist->BeginDrawState();	
  while(draw_queue->size()) {
    MeshDrawContext c = draw_queue->back();
    draw_queue->pop_back();
    if (c.mesh_seq!=whichdrawqueue)
      continue;
    if (whichdrawqueue!=MESH_SPECIAL_FX_ONLY) {
      GFXLoadIdentity(MODEL);
      GFXPickLights (Vector (c.mat[12],c.mat[13],c.mat[14]),rSize());
    }
    vector <int> specialfxlight;
    GFXLoadMatrix(MODEL, c.mat);
    if (c.cloaked&MeshDrawContext::CLOAK) {
      GFXPushBlendMode ();
      //	GFXColor4f (1,1,1,.25);
#if 0
      //SLOW on TNT
      GFXBlendColor (c.CloakFX);
      GFXBlendMode (CONSTCOLOR,INVCONSTCOLOR);
#else
      //      if (c.cloaked&MeshDrawContext::NEARINVIS)
	GFXDisable (TEXTURE1);
      int ligh;
      GFXCreateLight (ligh,GFXLight (true,GFXColor(0,0,0,1),GFXColor (0,0,0,1),GFXColor (0,0,0,1),c.CloakFX,GFXColor(1,0,0)),true);
      specialfxlight.push_back (ligh);
      GFXBlendMode (ONE,ONE);
#endif
    }


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
    if (c.cloaked&MeshDrawContext::CLOAK) {
#if 0
      GFXBlendColor (GFXColor(1,1,1,1));
#endif
      if (envMap)
      	GFXEnable (TEXTURE1);
      GFXPopBlendMode ();
    }
    if(0!=forcelogos&&!(c.cloaked&MeshDrawContext::NEARINVIS)) {
      forcelogos->Draw(c.mat);
    }
    if (0!=squadlogos&&!(c.cloaked&MeshDrawContext::NEARINVIS)){
      squadlogos->Draw(c.mat);
    }
  }
  vlist->EndDrawState();
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
