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
#include "cmd/nebula.h"
#include <list>
#include <string>
#include <fstream>
#include "vs_path.h"
#include "lin_time.h"
#include "gfxlib.h"
#include "vs_globals.h"
#include "config_xml.h"
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
class OrigMeshContainer {
public:
  float d;
  Mesh * orig;
  OrigMeshContainer(){ orig=NULL; };
  OrigMeshContainer (Mesh * tmp, float d) {
    orig = tmp;
    this->d = d;
  }
  bool operator < (const OrigMeshContainer & b) const {
    if(orig->Decal==NULL || b.orig->Decal==NULL){
      cout << "DEcal is nulll" << endl;
      return b.orig->Decal!=NULL;
    }
    return ((*orig->Decal) < (*b.orig->Decal));
  }
  bool operator == (const OrigMeshContainer &b) const {
    return (*orig->Decal)==*b.orig->Decal;
  }
};
typedef std::vector<OrigMeshContainer> OrigMeshVector;
OrigMeshVector undrawn_meshes[NUM_MESH_SEQUENCE]; // lower priority means draw first
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
bool Mesh::LoadExistant (const string filehash, float scale) {
  Mesh * oldmesh;

  hash_name = GetHashName (filehash,scale);
  oldmesh = meshHashTable.Get(hash_name);

  if (oldmesh==0) {
    hash_name =GetSharedMeshHashName(filehash,scale);
    oldmesh = meshHashTable.Get(hash_name);  
  }
  if(0 != oldmesh) {
    return LoadExistant(oldmesh);
  }
  //  fprintf (stderr,"cannot cache %s",GetSharedMeshHashName(filehash,scale).c_str());
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
Mesh:: Mesh(const char * filename,const float scale, int faction, Flightgroup *fg, bool orig):hash_name(filename)
{
  this->orig=NULL;
  InitUnit();
  Mesh *oldmesh;
  if (LoadExistant (filename,scale)) {
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
    LoadXML(shared?GetSharedMeshPath(filename).c_str():filename,scale,faction,fg);
    oldmesh = this->orig;
  } else {
    this->xml= NULL;
    LoadBinary(shared?GetSharedMeshPath(filename).c_str():filename,faction);
    oldmesh = new Mesh[1];
  }
  draw_queue = new vector<MeshDrawContext>;
  if (!orig) {
    hash_name =shared?GetSharedMeshHashName (filename,scale):GetHashName(filename,scale);
    meshHashTable.Put(hash_name, oldmesh);
    //oldmesh[0]=*this;
    *oldmesh=*this;
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
	  for (int j=0;j<NUM_MESH_SEQUENCE;j++) {
	    for (unsigned int i=0;i<undrawn_meshes[j].size();i++) {
	      if (undrawn_meshes[j][i].orig==this) {
		undrawn_meshes[j].erase(undrawn_meshes[j].begin()+i);
		i--;
		fprintf (stderr,"stale mesh found in draw queue--removed!\n");
	      }
	    }
	  }
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
	  if (meshHashTable.Get(hash_name)==this)
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



void Mesh::Draw(float lod, const Matrix &m, float toofar, short cloak, float nebdist)
{
  //  Vector pos (local_pos.Transform(m));
  MeshDrawContext c(m);
  UpdateFX(GetElapsedTime());
  c.SpecialFX = &LocalFX;
  static float too_far_dist = XMLSupport::parse_float (vs_config->getVariable ("graphics","mesh_far_percent",".8"));
  c.mesh_seq=((toofar+rSize()>too_far_dist*g_game.zfar)/*&&draw_sequence==0*/)?NUM_ZBUF_SEQ:draw_sequence;
  c.cloaked=MeshDrawContext::NONE;
  if (nebdist<0) {
    c.cloaked|=MeshDrawContext::FOG;
  }
  if (cloak>=0) {
    c.cloaked|=MeshDrawContext::CLOAK;
    if ((cloak&0x1)) {
      c.cloaked |= MeshDrawContext::GLASSCLOAK;
      c.mesh_seq=MESH_SPECIAL_FX_ONLY;//draw near the end with lights
    } else {
      c.mesh_seq =2;
    }
    if (cloak<16384) {
      c.cloaked|=MeshDrawContext::NEARINVIS;
    }
    float tmp = ((float)cloak)/32767;
    c.CloakFX.r = (c.cloaked&MeshDrawContext::GLASSCLOAK)?tmp:1;
    c.CloakFX.g = (c.cloaked&MeshDrawContext::GLASSCLOAK)?tmp:1;
    c.CloakFX.b = (c.cloaked&MeshDrawContext::GLASSCLOAK)?tmp:1;
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
    //    fprintf (stderr,"origmesh %x",origmesh);
    undrawn_meshes[c.mesh_seq].push_back(OrigMeshContainer(origmesh,toofar-rSize()));//FIXME will not work if many of hte same mesh are blocking each other
  }
  will_be_drawn |= (1<<c.mesh_seq);
}
void Mesh::DrawNow(float lod,  bool centered, const Matrix &m, short cloak, float nebdist) {
  Mesh *o = getLOD (lod);
  //fixme: cloaking not delt with.... not needed for backgroudn anyway
  if (nebdist<0) {
    Nebula * t=_Universe.AccessCamera()->GetNebula();
    if (t) {
      t->SetFogState();
    }
  } else {
    GFXFogMode(FOG_OFF);
  }
  if (centered) {
    //    Matrix m1 (m);
    //Vector pos(_Universe.AccessCamera()->GetPosition().Transform(m1));
    //m1[12]=pos.i;
    //m1[13]=pos.j;
    //m1[14]=pos.k;
    GFXCenterCamera (true);
    GFXLoadMatrixModel (m);    
  } else {	
    if (o->draw_sequence!=MESH_SPECIAL_FX_ONLY) {
      GFXLoadIdentity(MODEL);
      GFXPickLights (Vector (m.p.i,m.p.j,m.p.k),rSize());
    }
    GFXLoadMatrixModel (m);
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
  if (centered) {
    GFXCenterCamera(false);
  }
  for ( i=0;i<specialfxlight.size();i++) {
    GFXDeleteLight (specialfxlight[i]);
  }
  if (cloak>=0&&cloak<32767) {
    GFXEnable (TEXTURE1);
  }
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
static GFXColor getMeshColor () {
   float color[4];
  vs_config->getColor ("unit", "ship_ambient",color);
  GFXColor tmp (color[0],color[1],color[2],color[3]);
  return tmp;
}
class MeshCloser { 
public:
  MeshCloser () {}
  ///approximate closness based on center o matrix (which is gonna be center for spheres and convex objects most likely)
  bool operator () (const OrigMeshContainer & a, const OrigMeshContainer & b) {
    //    return a.d+a.orig->rSize() > b.d+b.orig->rSize();//draw from outside in :-)
    return a.d > b.d;//draw from outside in :-)
  }
};
void Mesh::ProcessZFarMeshes () {
  static GFXColor meshcolor (getMeshColor());
  GFXLightContextAmbient(meshcolor);
  _Universe.AccessCamera()->UpdateGFX (GFXFALSE, GFXFALSE);
  GFXEnable(LIGHTING);
  GFXEnable(CULLFACE);
  GFXDisable (DEPTHTEST);
  GFXDisable (DEPTHWRITE);
  ///sort meshes  
  //std::sort<OrigMeshVector::iterator,MeshCloser>(undrawn_meshes[NUM_ZBUF_SEQ].begin(),undrawn_meshes[NUM_ZBUF_SEQ].end(),MeshCloser());
  std::sort(undrawn_meshes[NUM_ZBUF_SEQ].begin(),undrawn_meshes[NUM_ZBUF_SEQ].end(),MeshCloser());
  for (OrigMeshVector::iterator i=undrawn_meshes[NUM_ZBUF_SEQ].begin();i!=undrawn_meshes[NUM_ZBUF_SEQ].end();i++) {
    i->orig->ProcessDrawQueue (NUM_ZBUF_SEQ);
    i->orig->will_be_drawn &= (~(1<<NUM_ZBUF_SEQ));
  }
  undrawn_meshes[NUM_ZBUF_SEQ].clear();
  GFXFogMode(FOG_OFF);
  Animation::ProcessFarDrawQueue(-FLT_MAX);
  _Universe.AccessCamera()->UpdateGFX (GFXTRUE, GFXFALSE);
  GFXEnable (DEPTHTEST);
  GFXEnable (DEPTHWRITE);
}

void Mesh::ProcessUndrawnMeshes(bool pushSpecialEffects) {
  static GFXColor meshcolor (getMeshColor());
  GFXLightContextAmbient(meshcolor);
  GFXEnable(DEPTHWRITE);
  GFXEnable(DEPTHTEST);
  GFXEnable(LIGHTING);
  GFXEnable(CULLFACE);

  for(int a=0; a<NUM_ZBUF_SEQ; a++) {
    if (a==MESH_SPECIAL_FX_ONLY) {
      
      GFXPushGlobalEffects();
      GFXDisable(DEPTHWRITE);
    } else {
    }
    if (!undrawn_meshes[a].empty()) {
      // shouldn't the sort - if any - be placed here??
      std::sort(undrawn_meshes[a].begin(),undrawn_meshes[a].end());//sort by texture address
      undrawn_meshes[a].back().orig->vlist->LoadDrawState();
    }
    while(!undrawn_meshes[a].empty()) {
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
  
    while(undrawn_logos.size()) {
      Logo *l = undrawn_logos.back();
      undrawn_logos.pop_back();
      l->ProcessDrawQueue();
      l->will_be_drawn = false;
    }
  }
}
void Mesh::ProcessDrawQueue(int whichdrawqueue) {
  //  assert(draw_queue->size());

  if (draw_queue->empty()) {
    fprintf (stderr,"cloaking queues issue! Report to hellcatv@hotmail.com\nn%d\n%s",whichdrawqueue,hash_name.c_str());
    return;
  }
  if (whichdrawqueue==NUM_ZBUF_SEQ) {
	  for (unsigned int i=0;i<draw_queue->size();i++) {
		MeshDrawContext * c = &((*draw_queue)[i]);
	    if (c->mesh_seq==whichdrawqueue) {
	      Animation::ProcessFarDrawQueue ((_Universe.AccessCamera()->GetPosition()-c->mat.p).Magnitude()/*+this->radialSize*/);		
		}
	  }
      GFXEnable(LIGHTING);
      GFXEnable(CULLFACE);
  }

  if (getLighting()) {
    GFXSelectMaterial(myMatNum);
  }else {
    GFXDisable (LIGHTING);
    GFXColor4f(1,1,1,1);
  }
  if (blendDst!=ZERO&&whichdrawqueue!=NUM_ZBUF_SEQ) {
    //    
    GFXDisable(DEPTHWRITE);
    if (blendSrc!=SRCALPHA) {
      GFXDisable(CULLFACE);
    }
  }
  GFXBlendMode(blendSrc, blendDst);
  GFXEnable(TEXTURE0);
  if(Decal)
    Decal->MakeActive();
  GFXSelectTexcoordSet(0, 0);
  if(getEnvMap()) {
    GFXEnable(TEXTURE1);
    _Universe.activateLightMap();
    GFXSelectTexcoordSet(1, 1);
  } else {
    GFXDisable(TEXTURE1);
  }
  vlist->BeginDrawState();	
  vector<MeshDrawContext> tmp_draw_queue;
  while(draw_queue->size()) {
	  
	MeshDrawContext c = draw_queue->back();
    draw_queue->pop_back();
    if (c.mesh_seq!=whichdrawqueue) {
      tmp_draw_queue.push_back (c);
      continue;
    }
    if (whichdrawqueue!=MESH_SPECIAL_FX_ONLY) {
      GFXLoadIdentity(MODEL);
      GFXPickLights (Vector (c.mat.p.i,c.mat.p.j,c.mat.p.k),rSize());
    }
    vector <int> specialfxlight;
    GFXLoadMatrixModel ( c.mat);
    if (c.cloaked&MeshDrawContext::CLOAK) {
      GFXPushBlendMode ();
      //	GFXColor4f (1,1,1,.25);
#if 0
      //SLOW on TNT
      GFXBlendColor (c.CloakFX);
      GFXBlendMode (CONSTCOLOR,INVCONSTCOLOR);
#else
      if (c.cloaked&MeshDrawContext::GLASSCLOAK) {
	GFXDisable (TEXTURE1);
	int ligh;
	GFXCreateLight (ligh,GFXLight (true,GFXColor(0,0,0,1),GFXColor (0,0,0,1),GFXColor (0,0,0,1),c.CloakFX,GFXColor(1,0,0)),true);
	specialfxlight.push_back (ligh);
	GFXBlendMode (ONE,ONE);
      }else {
	if (c.cloaked&MeshDrawContext::NEARINVIS) {      
	  //NOT sure I like teh jump this produces	GFXDisable (TEXTURE1);
	}
	GFXBlendMode (SRCALPHA, INVSRCALPHA);
	GFXColorMaterial (AMBIENT|DIFFUSE);
	GFXColorf(c.CloakFX);
      }
#endif
    }

    unsigned int i;
    for ( i=0;i<c.SpecialFX->size();i++) {
      int ligh;
      GFXCreateLight (ligh,(*c.SpecialFX)[i],true);
      specialfxlight.push_back(ligh);
    }
    if (c.cloaked&MeshDrawContext::FOG) {
      Nebula *t=_Universe.AccessCamera()->GetNebula();
      if (t) {
	t->SetFogState();
      }
    } else {
      GFXFogMode (FOG_OFF);
    }
    vlist->Draw();
    for ( i=0;i<specialfxlight.size();i++) {
      GFXDeleteLight (specialfxlight[i]);
    }
    if (c.cloaked&MeshDrawContext::CLOAK) {
#if 0
      GFXBlendColor (GFXColor(1,1,1,1));
#else
      GFXColorMaterial (0);
#endif
      if (getEnvMap())
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
  if (!getLighting()) {
    GFXEnable(LIGHTING);
  }
  while (tmp_draw_queue.size()) {
    draw_queue->push_back (tmp_draw_queue.back());
    tmp_draw_queue.pop_back();
  }
  if (blendDst!=ZERO &&whichdrawqueue!=NUM_ZBUF_SEQ) {
    
    GFXEnable(DEPTHWRITE);//risky--for instance logos might be fubar!
    if (blendSrc!=SRCALPHA) {
      GFXEnable (CULLFACE);
    }
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

