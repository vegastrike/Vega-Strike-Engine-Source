#include <algorithm>
#include "mesh.h"
#include "aux_texture.h"
#include "aux_logo.h"
#include "lin_time.h"
#include "configxml.h"
#include "vs_globals.h"
#include "cmd/nebula_generic.h"
#include "gfx/camera.h"
#include "gfx/animation.h"
#include "mesh_xml.h"
#include "gldrv/gl_globals.h"
#if defined(CG_SUPPORT)
#include "gldrv/gl_light.h"
#include "cg_global.h"
#endif

extern vector<Logo*> undrawn_logos;
void UpdateShaderConstants(char *program);
#define Z_PARTITION_HEADROOM 0.1f
#define MAX_ZRATIO 0.004f
#define MIN_ZRATIO 0.100f

#ifdef PARTITIONED_Z_BUFFER
class OrigMeshContainer {
public:
  float zmin,zmax;
  unsigned int s;
  Mesh * orig;

  OrigMeshContainer(){ orig=NULL; zmin=zmax=0; s=0; };
  OrigMeshContainer (Mesh * orig, float zmin, float zmax, unsigned int s) {
    this->orig = orig;
    this->zmin = zmin;
    this->zmax = zmax;
    this->s = s;
  }
  bool operator < (const OrigMeshContainer &b) const { return (zmax>b.zmax)||((zmax==b.zmax)&&(s<b.s)); }
  bool operator == (const OrigMeshContainer &b) const { return (s==b.s)&&(zmax==b.zmax); }
  OrigMeshContainer& operator = (const OrigMeshContainer &b) { orig=b.orig; zmin=b.zmin; zmax=b.zmax; s=b.s; return *this; };
};

class Meshvs_decalsort { 
public:
  Meshvs_decalsort() {}
  //sort by decal similarity
  bool operator () (const OrigMeshContainer & a, const OrigMeshContainer & b) {
      if (a.s<b.s) return true;
      if (a.s==b.s) for (int i=0; (i<a.orig->Decal.size())&&(i<b.orig->Decal.size()); i++) {
          if ((a.orig->Decal[i])&&(b.orig->Decal[i])) {
            if (   *(a.orig->Decal[i]) <  *(b.orig->Decal[i]))   return true;
            if (!((*(a.orig->Decal[i]) == *(b.orig->Decal[i])))) return false;
          } else {
              if (a.orig->Decal[i]) return true;
              if (b.orig->Decal[i]) return false;
          };
          return (a.orig<b.orig);
      } else return false;
  }
};
#else
class OrigMeshContainer {
public:
  float d;
  Mesh * orig;
  OrigMeshContainer(){ orig=NULL; };
  OrigMeshContainer (Mesh * orig, float d) {
    this->orig = orig;
    this->d = d;
  }
  bool operator < (const OrigMeshContainer & b) const {
    return (orig->Decal.size() < b.orig->Decal.size())
        ||(  (orig->Decal.size() == b.orig->Decal.size())
           &&(orig->Decal.size() > 0)
           &&(orig->Decal[0] < b.orig->Decal[0])  );
  }
  bool operator == (const OrigMeshContainer &b) const {
    return (orig->Decal.size() == b.orig->Decal.size())
        && (  (orig->Decal.size() == 0)
            ||(orig->Decal[0] == b.orig->Decal[0])  );
  }
};

class Meshvs_closer { 
public:
  Meshvs_closer () {}
  static bool FilterCompare (Mesh * a, Mesh * b) {
	  if (a) {
		  return a->getBlendDst()==ZERO;
	  }
	  return false;
  }
  ///approximate closness based on center o matrix (which is gonna be center for spheres and convex objects most likely)
  bool operator () (const OrigMeshContainer & a, const OrigMeshContainer & b) {
	float tmp = a.d-b.d;
	if (tmp*tmp<.001)
		return FilterCompare(a.orig,b.orig);
    //    return a.d+a.orig->rSize() > b.d+b.orig->rSize();//draw from outside in :-)
    return tmp>0.0;//draw from outside in :-)
  }
};
#endif



typedef std::vector<OrigMeshContainer> OrigMeshVector;
#define NUM_PASSES 4
#define BASE_PASS 0
#define ENVSPEC_PASS 1
#define DAMAGE_PASS 2
#define GLOW_PASS 3
#define BASE_TEX 0
#define ENVSPEC_TEX 1
#define DAMAGE_TEX 2
#define GLOW_TEX 3
#define NORMAL_TEX 4
#define NUM_TEX 5
const int UNDRAWN_MESHES_SIZE= NUM_MESH_SEQUENCE*NUM_PASSES;

#ifdef PARTITIONED_Z_BUFFER
OrigMeshVector undrawn_meshes;
#else
OrigMeshVector undrawn_meshes[NUM_MESH_SEQUENCE][NUM_PASSES]; // lower priority means draw first
#endif

Texture * Mesh::TempGetTexture(MeshXML * xml, std::string filename, std::string factionname, GFXBOOL detail) const{
	static FILTER fil = XMLSupport::parse_bool(vs_config->getVariable("graphics","detail_texture_trilinear","true"))?TRILINEAR:MIPMAP;
	static bool factionalize_textures = XMLSupport::parse_bool( vs_config->getVariable("graphics","faction_dependant_textures","true") );
	string faction_prefix = (factionalize_textures?(factionname+"_"):string());
	Texture * ret=NULL;
	string facplus = faction_prefix+filename;
	if (filename.find(".ani")!=string::npos) {
	    ret = new AnimatedTexture(facplus.c_str(),1,fil,detail);
		if (!ret->LoadSuccess()) {
			delete ret;
			ret = new AnimatedTexture(filename.c_str(),1,fil,detail);
			if (!ret->LoadSuccess()) {
				delete ret;
				ret=NULL;
			}else {
				return ret;
			}
		}else {
			return ret;
		}
	}
	ret = new Texture (facplus.c_str(),1,fil,TEXTURE2D,TEXTURE_2D,GFXFALSE,65536,detail);
	if (!ret->LoadSuccess()) {
		delete ret;
		ret = new Texture (filename.c_str(),1,fil,TEXTURE2D,TEXTURE_2D,GFXFALSE,65536,detail);
	}
	return ret;
}
 int  Mesh::getNumTextureFrames() {
	if (Decal.size())
		if (Decal[0]) {
			return Decal[0]->numFrames();
		}
	return 1;
}
double Mesh::getTextureCumulativeTime() {
	if (Decal.size())
		if (Decal[0]) {
			return Decal[0]->curTime();
		}
	return 0;
}
float Mesh::getTextureFramesPerSecond(){
	if (Decal.size())
		if (Decal[0]) {
			return Decal[0]->framesPerSecond();
		}
	return 0;
}
void Mesh::setTextureCumulativeTime(double d) {
	for (unsigned int i=0;i<Decal.size();++i) {
		if (Decal[i])
			Decal[i]->setTime(d);
	}
}
Texture * Mesh::TempGetTexture (MeshXML * xml, int index, std::string factionname)const {
	static bool factionalize_textures = XMLSupport::parse_bool( vs_config->getVariable("graphics","faction_dependant_textures","true") );
	string faction_prefix = (factionalize_textures?(factionname+"_"):string());
    Texture *tex=NULL;
    assert (index<(int)xml->decals.size());
    MeshXML::ZeTexture * zt = &(xml->decals[index]);
    if (zt->animated_name.length()) {
        string tempani = faction_prefix+zt->animated_name;
        tex = new AnimatedTexture (tempani.c_str(),0,BILINEAR);
        if (!tex->LoadSuccess()) {
            delete tex;
            tex = new AnimatedTexture (zt->animated_name.c_str(),0,BILINEAR);
        }
    }else if (zt->decal_name.length()==0) {
        tex = NULL;
    } else {
        if (zt->alpha_name.length()==0) {
            string temptex = faction_prefix+zt->decal_name;
            tex = new Texture(temptex.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,(g_game.use_ship_textures||xml->force_texture)?GFXTRUE:GFXFALSE);
            if (!tex->LoadSuccess()) {
                delete tex;
                tex = new Texture(zt->decal_name.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,(g_game.use_ship_textures||xml->force_texture)?GFXTRUE:GFXFALSE);
            }
        }else {
            string temptex = faction_prefix+zt->decal_name;
            string tempalp = faction_prefix+zt->alpha_name;
            tex = new Texture(temptex.c_str(), tempalp.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,1,0,(g_game.use_ship_textures||xml->force_texture)?GFXTRUE:GFXFALSE);
            if (!tex->LoadSuccess()) {
                delete tex;
                tex = new Texture(zt->decal_name.c_str(), zt->alpha_name.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,1,0,(g_game.use_ship_textures||xml->force_texture)?GFXTRUE:GFXFALSE);
            }

        }
    }
    return tex;
    
}
Texture * createTexture( const char * filename, int stage=0,enum FILTER f1= MIPMAP,enum TEXTURE_TARGET t0=TEXTURE2D,enum TEXTURE_IMAGE_TARGET t=TEXTURE_2D,unsigned char c=GFXFALSE,int i=65536)
{
	return new Texture( filename, stage, f1, t0, t, c, i);
}
Logo * createLogo(int numberlogos,Vector* center, Vector* normal, float* sizes, float* rotations, float offset, Texture * Dec, Vector *Ref)
{
	return new Logo(numberlogos,center,normal,sizes ,rotations, offset, Dec ,Ref);
}
Texture * createTexture( char const * ccc,char const * cc,int k= 0,enum FILTER f1= MIPMAP,enum TEXTURE_TARGET t0=TEXTURE2D,enum TEXTURE_IMAGE_TARGET t=TEXTURE_2D,float f=1,int j=0,unsigned char c=GFXFALSE,int i=65536)
{
	return new Texture( ccc, cc, k, f1, t0, t, f, j, c, i);
}
AnimatedTexture * createAnimatedTexture( char const * c,int i,enum FILTER f)
{
	return new AnimatedTexture( c, i, f);
}
extern Hashtable<std::string, std::vector <Mesh*>, MESH_HASTHABLE_SIZE> bfxmHashTable;
Mesh::~Mesh()
{
	if(!orig||orig==this)
	{
#ifdef PARTITIONED_Z_BUFFER
      { for (unsigned int i=0;i<undrawn_meshes.size();i++) {
		  if (undrawn_meshes[i].orig==this) {
			  undrawn_meshes.erase(undrawn_meshes.begin()+i);
			  i--;
			  VSFileSystem::vs_fprintf (stderr,"stale mesh found in draw queue--removed!\n");
		  }
      } }
#else
	  for (int j=0;j<NUM_MESH_SEQUENCE;j++) {
		  for (int k=0;k<NUM_PASSES;++k) {
			  for (unsigned int i=0;i<undrawn_meshes[j][k].size();i++) {
				  if (undrawn_meshes[j][k][i].orig==this) {
					  undrawn_meshes[j][k].erase(undrawn_meshes[j][k].begin()+i);
					  i--;
					  VSFileSystem::vs_fprintf (stderr,"stale mesh found in draw queue--removed!\n");
				  }
			  }
		  }
	  }
#endif
	  delete vlist;
	  for (unsigned int i=0;i<Decal.size();i++) {
	    if(Decal[i] != NULL) {
	      delete Decal[i];
	      Decal[i] = NULL;
	    }
	  }
	  if (squadlogos!=NULL) {
	    delete squadlogos;
	    squadlogos= NULL;
	  }
	  if (forcelogos!=NULL) {
	    delete forcelogos;
	    forcelogos = NULL;
	  }
	  if (meshHashTable.Get(hash_name)==this){
	    meshHashTable.Delete(hash_name);
	  }
          vector <Mesh *>* hashers = bfxmHashTable.Get(hash_name);
          vector <Mesh *>::iterator finder;
          if (hashers) {
            for (int i=hashers->size()-1;i>=0;--i) {
              if ((*hashers)[i]==this) {
                hashers->erase (hashers->begin()+i);
                if (hashers->empty()) {
                  bfxmHashTable.Delete(hash_name);
                  delete hashers;
                }
              }
            }
          }
	  if(draw_queue!=NULL)
	    delete[] draw_queue;
	} else {
	  orig->refcount--;
	  //printf ("orig refcount: %d",refcount);
	  if(orig->refcount == 0) {
	    delete [] orig;	      
	  }
	}
}
void Mesh::Draw(float lod, const Matrix &m, float toofar, int cloak, float nebdist,unsigned char hulldamage, bool renormalize) //short fix
{
  //  Vector pos (local_pos.Transform(m));
  MeshDrawContext c(m);
  UpdateFX(GetElapsedTime());
  c.SpecialFX = &LocalFX;
  c.damage=hulldamage;
  static float too_far_dist = XMLSupport::parse_float (vs_config->getVariable ("graphics","mesh_far_percent",".8"));

  //c.mesh_seq=((toofar+rSize()>too_far_dist*g_game.zfar)/*&&draw_sequence==0*/)?NUM_ZBUF_SEQ:draw_sequence;
#ifdef PARTITIONED_Z_BUFFER
  Camera *cam = _Universe->AccessCamera();
  float rs = clipRadialSize();
  c.mesh_seq=draw_sequence;
  c.zmin = cam->GetZDist(m.p);
  c.zmax = c.zmin + rs;
  c.zmin -= rs;
#else
  c.mesh_seq=((toofar+((getConvex()==1)?0:rSize())>too_far_dist*g_game.zfar)/*&&draw_sequence==0*/)?NUM_ZBUF_SEQ:draw_sequence;
#endif
  c.cloaked=MeshDrawContext::NONE;
  if (nebdist<0) {
    c.cloaked|=MeshDrawContext::FOG;
  }
  if (renormalize) {
	  c.cloaked|=MeshDrawContext::RENORMALIZE;
  }
  if (cloak>=0) {
    c.cloaked|=MeshDrawContext::CLOAK;
    if ((cloak&0x1)) {
      c.cloaked |= MeshDrawContext::GLASSCLOAK;
      c.mesh_seq=MESH_SPECIAL_FX_ONLY;//draw near the end with lights
    } else {
      c.mesh_seq =2;
    }
    if (cloak<=2147483647/2) {
      c.cloaked|=MeshDrawContext::NEARINVIS;
    }
    float tmp = ((float)cloak)/2147483647;
    c.CloakFX.r = (c.cloaked&MeshDrawContext::GLASSCLOAK)?tmp:1;
    c.CloakFX.g = (c.cloaked&MeshDrawContext::GLASSCLOAK)?tmp:1;
    c.CloakFX.b = (c.cloaked&MeshDrawContext::GLASSCLOAK)?tmp:1;
    c.CloakFX.a = tmp;
    /*
    c.CloakNebFX.ambient[0]=((float)cloak)/2147483647;
    c.CloakNebFX.ag=((float)cloak)/2147483647;
    c.CloakNebFX.ab=((float)cloak)/2147483647;
    c.CloakNebFX.aa=((float)cloak)/2147483647;
    */
    ///all else == defaults, only ambient
  } 
  //  c.mat[12]=pos.i;
  //  c.mat[13]=pos.j;
  //  c.mat[14]=pos.k;//to translate to local_pos which is now obsolete!
  Mesh *origmesh = getLOD (lod);
  origmesh->draw_queue[c.mesh_seq].push_back(c);
#ifndef PARTITIONED_Z_BUFFER
  if(!(origmesh->will_be_drawn&(1<<c.mesh_seq))) {
    origmesh->will_be_drawn |= (1<<c.mesh_seq);
    //FIXME will not work if many of hte same mesh are blocking each other
    //Sequence 0 already does all other passes
    undrawn_meshes[c.mesh_seq][0].push_back(OrigMeshContainer(origmesh,toofar-rSize()));
  }
#else
  undrawn_meshes.push_back(OrigMeshContainer(origmesh,c.zmin,c.zmax,c.mesh_seq));//FIXME will not work if many of hte same mesh are blocking each other
#endif
  will_be_drawn |= (1<<c.mesh_seq);
}
void Mesh::DrawNow(float lod,  bool centered, const Matrix &m, int cloak, float nebdist) { //short fix
  Mesh *o = getLOD (lod);
  //fixme: cloaking not delt with.... not needed for backgroudn anyway
  if (nebdist<0) {
    Nebula * t=_Universe->AccessCamera()->GetNebula();
    if (t) {
      t->SetFogState();
    }
  } else {
    GFXFogMode(FOG_OFF);
  }
  if (centered) {
    //    Matrix m1 (m);
    //Vector pos(_Universe->AccessCamera()->GetPosition().Transform(m1));
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

  //Making it static avoids frequent reallocations - although may be troublesome for thread safety
  //but... WTH... nothing is thread safe in VS.
  //Also: Be careful with reentrancy... right now, this section is not reentrant.
  static vector <int> specialfxlight;

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
  if (o->Decal.size() && o->Decal[0])
    o->Decal[0]->MakeActive(); 
  GFXTextureEnv(0,GFXMODULATETEXTURE); //Default diffuse mode
  GFXTextureEnv(1,GFXADDTEXTURE);      //Default envmap mode
  GFXToggleTexture(bool(o->Decal.size() && o->Decal[0]) ,0);
  o->vlist->DrawOnce();
  if (centered) {
    GFXCenterCamera(false);
  }
  for ( i=0;i<specialfxlight.size();i++) {
    GFXDeleteLight (specialfxlight[i]);
  }
  if (cloak>=0&&cloak<2147483647) {
    GFXEnable (TEXTURE1);
  }
}
static GFXColor getMeshColor () {
   float color[4];
  vs_config->getColor ("unit", "ship_ambient",color);
  GFXColor tmp (color[0],color[1],color[2],color[3]);
  return tmp;
}
void Mesh::ProcessZFarMeshes (bool nocamerasetup) {
#ifndef PARTITIONED_Z_BUFFER
  int defaultprogram=GFXActivateShader(NULL);
  UpdateShaderConstants(NULL);
  static GFXColor meshcolor (getMeshColor());
  GFXLightContextAmbient(meshcolor);
  if (!nocamerasetup)
      _Universe->AccessCamera()->UpdateGFX (GFXFALSE, GFXFALSE);
  _Universe->activateLightMap();
  GFXEnable(LIGHTING);
  GFXEnable(CULLFACE);
  GFXDisable (DEPTHTEST);
  GFXDisable (DEPTHWRITE);
  //std::sort<OrigMeshVector::iterator,Meshvs_closer>(undrawn_meshes[NUM_ZBUF_SEQ].begin(),undrawn_meshes[NUM_ZBUF_SEQ].end(),Meshvs_closer());
  for (int k=0;k<NUM_PASSES;++k) {
	  std::sort(undrawn_meshes[NUM_ZBUF_SEQ][k].begin(),undrawn_meshes[NUM_ZBUF_SEQ][k].end(),Meshvs_closer());
	  
	  for (OrigMeshVector::iterator i=undrawn_meshes[NUM_ZBUF_SEQ][k].begin();i!=undrawn_meshes[NUM_ZBUF_SEQ][k].end();i++) {
		  i->orig->ProcessDrawQueue (k,NUM_ZBUF_SEQ);
		  i->orig->will_be_drawn &= (~(1<<NUM_ZBUF_SEQ));//not accurate any more
	  }
	  undrawn_meshes[NUM_ZBUF_SEQ][k].clear();	  
  }

  GFXFogMode(FOG_OFF);
  GFXDeactivateShader();
  Animation::ProcessFarDrawQueue(-FLT_MAX);
  if (!nocamerasetup)
      _Universe->AccessCamera()->UpdateGFX (GFXTRUE, GFXFALSE);
  GFXEnable (DEPTHTEST);
  GFXEnable (DEPTHWRITE);
#endif
}

const GFXMaterial &Mesh::GetMaterial () {
   return GFXGetMaterial (myMatNum);
}

template<typename T> inline bool rangesOverlap(T min1, T max1, T min2, T max2) {  
    return !(  ((min1<min2)==(max1<min2))
             &&((min1<max2)==(max1<max2))
             &&((min1<min2)==(min1<max2))  );
}

void Mesh::ProcessUndrawnMeshes(bool pushSpecialEffects,bool nocamerasetup) {
  int defaultprogram=GFXActivateShader(NULL);
  static GFXColor meshcolor (getMeshColor());

  _Universe->activateLightMap();

#ifdef PARTITIONED_Z_BUFFER
  bool spefx=false;
  std::sort(undrawn_meshes.begin(),undrawn_meshes.end());
  OrigMeshVector auxlist;
  OrigMeshVector::iterator itstart=undrawn_meshes.begin(), itend=itstart, realend=undrawn_meshes.end(), it;
  Mesh *om,*m;
  unsigned int os=~0;
  while (itstart<realend) {
      float zmin=FLT_MAX, zmax=FLT_MIN;
      auxlist.clear();
      itend=itstart;
      om = NULL;
      while (itend<realend) {
          if ((zmin>zmax)||(zmax < g_game.znear)||rangesOverlap(zmin,zmax,itend->zmin,itend->zmax)) {
              if (itend->zmin<zmin) zmin = itend->zmin;
              if (itend->zmax>zmax) zmax = itend->zmax;
              //if (zmin>zmax/2) zmin=zmax/2; 
              if ((om!=itend->orig)||(os!=itend->s)) { 
                  //Push each mesh only once, 
                  //as ProcessDrawQueue() already renders all repetitions
                  auxlist.push_back(*itend); 
                  om=itend->orig; 
                  os=itend->s;
              }; 
              itend++;
          } else break;
      };
      itstart = itend;

      //Don't sort if they won't be visible
      if (zmax>0) std::sort(auxlist.begin(),auxlist.end(),Meshvs_decalsort());

      float fzmin = zmin;
      float fzmax = zmax;
      float fzrng = fzmax-fzmin;
      fzmin -= fzrng*Z_PARTITION_HEADROOM;
      fzmax += fzrng*Z_PARTITION_HEADROOM;
      if (fzmin < g_game.znear) fzmin = g_game.znear;
      if (fzmax < g_game.znear) fzmax = g_game.znear;
      if (fzmin==fzmax) fzmax += 1; // Never let them be equal!

      float fzmin2 = fzmin;
      float fzmax2 = fzmax;

      do {
          fzmin2 = max(fzmin,fzmax2*MAX_ZRATIO);
          if (!nocamerasetup)
              _Universe->AccessCamera()->UpdateGFX(GFXTRUE,GFXTRUE,GFXFALSE,GFXTRUE,fzmin2,fzmax2); else
              _Universe->AccessCamera()->UpdateGFXFrustum(GFXTRUE,fzmin2,fzmax2);
          GFXClear(GFXFALSE);

          GFXLightContextAmbient(meshcolor);
          GFXEnable(DEPTHWRITE);
          GFXEnable(DEPTHTEST);
          GFXEnable(LIGHTING);
          GFXEnable(CULLFACE);

          itend=auxlist.end();
          it=auxlist.begin();
 
          while (it < itend) {
              if (it->s == MESH_SPECIAL_FX_ONLY) {
                  if (!spefx) {
                      GFXPushGlobalEffects();
                      GFXDisable(DEPTHWRITE);
                      spefx=true;
                  };
              } else if (spefx) {
                  GFXPopGlobalEffects();
                  GFXEnable(DEPTHWRITE);
                  spefx=false;
              };
              it->orig->ProcessDrawQueue(0,it->s,fzmin2,fzmax2);
              it++;
          };

          for (vector<Logo*>::iterator ait=undrawn_logos.begin(); ait<undrawn_logos.end(); ait++) {
              Logo *l = *ait;
              l->ProcessDrawQueue();
              l->will_be_drawn = false;
          }

          fzmax2=fzmin2;
      } while (fzmin2 > fzmin);
  };
  if (spefx&&!pushSpecialEffects) GFXPopGlobalEffects();
  for (it=undrawn_meshes.begin(),itend=undrawn_meshes.end(); it < itend; it++) {
      for (int i=0; i<NUM_ZBUF_SEQ; i++) it->orig->draw_queue[i].clear();
      it->orig->will_be_drawn = 0;
  };
  _Universe->AccessCamera()->UpdateGFX(GFXTRUE,GFXTRUE,GFXFALSE,GFXFALSE,0,1000000);
  GFXClear(GFXFALSE);
  undrawn_meshes.clear();
  undrawn_logos.clear();
#else
  for(int a=0; a<NUM_ZBUF_SEQ; a++) {
      if (a==MESH_SPECIAL_FX_ONLY) {
          GFXPushGlobalEffects();
          GFXDisable(DEPTHWRITE);
      } else {
      }
      for (int k=0;k<NUM_PASSES;++k) {
          if (!undrawn_meshes[a][k].empty()) {
              std::sort(undrawn_meshes[a][k].begin(),undrawn_meshes[a][k].end());//sort by texture address
          }
          for (OrigMeshVector::iterator it=undrawn_meshes[a][k].begin(); it<undrawn_meshes[a][k].end(); it++) {
              Mesh *m = (*it).orig;
              m->ProcessDrawQueue(k,a);
              m->will_be_drawn &= (~(1<<a));//not accurate any more
          }
          undrawn_meshes[a][k].clear();
      }

      if (a==MESH_SPECIAL_FX_ONLY) {
          if (!pushSpecialEffects) {
              GFXPopGlobalEffects();
          }
          GFXEnable(DEPTHWRITE);
      }
      
      for (vector<Logo*>::iterator it=undrawn_logos.begin(); it<undrawn_logos.end(); it++) {
          Logo *l = *it;
          l->ProcessDrawQueue();
          l->will_be_drawn = false;
      }
      undrawn_logos.clear();
  }
#endif
  GFXDeactivateShader();
}
void Mesh::RestoreCullFace (int whichdrawqueue) {
  if (blendDst!=ZERO &&whichdrawqueue!=NUM_ZBUF_SEQ||getCullFaceForcedOff()) {
    if (blendSrc!=SRCALPHA) {
      GFXEnable (CULLFACE);
    }
  }
}
void Mesh::SelectCullFace (int whichdrawqueue) {
  if (whichdrawqueue==NUM_ZBUF_SEQ) {
    GFXEnable(CULLFACE);
  }else {
    if (getCullFaceForcedOn()) {
      GFXEnable(CULLFACE);
    }else if (getCullFaceForcedOff()) {
      GFXDisable(CULLFACE);
    }
  }
  if (blendDst!=ZERO&&whichdrawqueue!=NUM_ZBUF_SEQ) {
    //    
    GFXDisable(DEPTHWRITE);
    if (blendSrc!=SRCALPHA||getCullFaceForcedOff()) {
      GFXDisable(CULLFACE);
    }
  }
  
}
void SetupCloakState (char cloaked,const GFXColor & CloakFX, vector <int> &specialfxlight, unsigned char hulldamage,unsigned int matnum) {
    if (cloaked&MeshDrawContext::CLOAK) {
        GFXPushBlendMode ();
	GFXDisable(CULLFACE);
	/**/
	GFXEnable(LIGHTING);
	GFXEnable(TEXTURE0);
	GFXEnable(TEXTURE1);
	
	/**/
        if (cloaked&MeshDrawContext::GLASSCLOAK) {
            GFXDisable (TEXTURE1);
            int ligh;
            GFXCreateLight (ligh,GFXLight (true,GFXColor(0,0,0,1),GFXColor (0,0,0,1),GFXColor (0,0,0,1),CloakFX,GFXColor(1,0,0)),true);
            specialfxlight.push_back (ligh);
            GFXBlendMode (ONE,ONE);
            GFXSelectMaterialHighlights(matnum,
                                        GFXColor(1,1,1,1),
                                        GFXColor(1,1,1,1),
                                        GFXColor(1,1,1,1),
                                        CloakFX);
        }else {
	  GFXEnable(TEXTURE1);
	  if (cloaked&MeshDrawContext::NEARINVIS) {      
	    //NOT sure I like teh jump this produces	GFXDisable (TEXTURE1);
	  }
	  GFXBlendMode (SRCALPHA, INVSRCALPHA);
	  GFXColorMaterial (AMBIENT|DIFFUSE);
	  
	  if (hulldamage) {
	    GFXColor4f(CloakFX.r,CloakFX.g,CloakFX.b,CloakFX.a*hulldamage/255);
	  }else
	    GFXColorf(CloakFX);
        }
	
    }else if (hulldamage) {
      //ok now we go in and do the dirtying
      GFXColorMaterial (AMBIENT|DIFFUSE);
      GFXColor4f(1,1,1,hulldamage/255.);
    }
}
static void RestoreCloakState (char cloaked, bool envMap,unsigned char damage) {
    if (cloaked&MeshDrawContext::CLOAK) {
        GFXColorMaterial (0);
        if (envMap)
            GFXEnable (TEXTURE1);
        GFXPopBlendMode ();
#if defined(CG_SUPPORT)
	     cgGLDisableProfile(cloak_cg->vertexProfile);
#endif
    }
	if (damage) {
		GFXColorMaterial(0);
	}
}
static void SetupFogState (char cloaked) {
    if (cloaked&MeshDrawContext::FOG) {
        Nebula *t=_Universe->AccessCamera()->GetNebula();
        if (t) {
            t->SetFogState();
        }
    } else {
        GFXFogMode (FOG_OFF);
    }    
}
bool SetupSpecMapFirstPass (vector <Texture *> &decal, unsigned int mat, bool envMap,float polygon_offset,Texture *detailTexture,const vector<Vector> &detailPlanes,bool &skip_glowpass,bool nomultienv) {
    GFXPushBlendMode();

    static bool separatespec = XMLSupport::parse_bool (vs_config->getVariable ("graphics","separatespecularcolor","false"))?GFXTRUE:GFXFALSE;
    GFXSetSeparateSpecularColor(separatespec);

    static bool multitex_glowpass=XMLSupport::parse_bool(vs_config->getVariable("graphics","multitexture_glowmaps","true"));
	if (polygon_offset){
		float a,b;
		GFXGetPolygonOffset(&a,&b);
		GFXPolygonOffset (a, b-polygon_offset);
	}
    if (nomultienv) {
        GFXSelectMaterialHighlights(mat,
                                    GFXColor(1,1,1,1),
                                    GFXColor(1,1,1,1),
                                    GFXColor(0,0,0,0),
                                    GFXColor(0,0,0,0));
        GFXBlendMode(ONE,ONE);
        GFXDepthFunc(EQUAL);
    }
    bool retval=false;
    skip_glowpass = false;
	int detailoffset=2;
    if (!nomultienv&&(decal.size()>1)&&(decal[1])) {
		detailoffset=1;
        GFXSelectMaterialHighlights(mat,
                                    GFXColor(1,1,1,1),
                                    GFXColor(1,1,1,1),
                                    GFXColor(0,0,0,0),
                                    GFXColor(0,0,0,0));
        retval=true;
        if (envMap&&detailTexture==NULL) {
            if ((decal.size()>GLOW_PASS)&&decal[GLOW_PASS]&&gl_options.Multitexture&&multitex_glowpass) {
                decal[GLOW_PASS]->MakeActive(1);
                GFXTextureEnv(1,GFXADDTEXTURE);
                GFXToggleTexture(true,1);
				GFXTextureCoordGenMode(1,NO_GEN,NULL,NULL);
                skip_glowpass=true;
            } else GFXDisable(TEXTURE1);
        }
        if (decal[0]) {
            decal[0]->MakeActive(0);
            GFXToggleTexture(true,0);
        } else {
            GFXToggleTexture(false,0);
        }
    } else if (decal.size() && decal[0]) {
        GFXToggleTexture(true,0);
        decal[0]->MakeActive(0);
    } else {
        GFXToggleTexture(false,0);
    }
	if (detailTexture&&(gl_options.Multitexture>detailoffset)) {
			for (unsigned int i=1;i<detailPlanes.size();i+=2) {
				int stage = (i/2)+detailoffset;
				const float params[4]={detailPlanes[i-1].i,detailPlanes[i-1].j,detailPlanes[i-1].k,0};
				const float paramt[4]={detailPlanes[i].i,detailPlanes[i].j,detailPlanes[i].k,0};
				GFXTextureCoordGenMode(stage,OBJECT_LINEAR_GEN,params,paramt);
				detailTexture->MakeActive(stage);
				GFXTextureEnv(stage,GFXDETAILTEXTURE);
				GFXToggleTexture(true,stage);
			}
	}
	
    return retval;
}
enum ShaderConstantEnum{
  kDIFFUSEMAP,
  kENVMAP,
  kSPECMAP,
  kNORMALMAP,
  kGLOWMAP,
  kDAMAGEMAP,
  kDETAIL0MAP,
  kDETAIL1MAP,
  kCLOAKING,
  kDAMAGED,
  kDETAIL0PLANE,
  kDETAIL1PLANE,
  kNUMLIGHTS,
  kACTIVELIGHTS,
  kENVCOLOR,
  NUM_SHADER_CONSTANTS
};
struct ShaderConstantLookup{
  int id;
  char *name;
  ShaderConstantLookup(int id, char*name){this->id=id;this->name=name;}
};
ShaderConstantLookup shaderConstants[NUM_SHADER_CONSTANTS]={
  ShaderConstantLookup(-1,"diffuseMap"),
  ShaderConstantLookup(-1,"envMap"),
  ShaderConstantLookup(-1,"specMap"),
  ShaderConstantLookup(-1,"normalMap"),
  ShaderConstantLookup(-1,"glowMap"),
  ShaderConstantLookup(-1,"damageMap"),
  ShaderConstantLookup(-1,"detail0Map"),
  ShaderConstantLookup(-1,"detail1Map"),
  ShaderConstantLookup(-1,"cloaking"),
  ShaderConstantLookup(-1,"damaged"),
  ShaderConstantLookup(-1,"detail0Plane"),
  ShaderConstantLookup(-1,"detail1Plane"),
  ShaderConstantLookup(-1,"max_light_enabled"),
  ShaderConstantLookup(-1,"light_enabled"),
  ShaderConstantLookup(-1,"envColor")
};
void UpdateShaderConstants(char *program) {
  if (GFXShaderReloaded()) {
    for (int i=0;i<NUM_SHADER_CONSTANTS;++i) {
      shaderConstants[i].id=GFXNamedShaderConstant(program,shaderConstants[i].name);
    }
  }
}
void SetupShaders (vector <Texture *> &Decal, unsigned int mat, bool envMap,float polygon_offset,Texture *detailTexture,const vector<Vector> &detailPlanes, Texture*black, Texture*white){
  GFXPushBlendMode();
  
  static bool separatespec = XMLSupport::parse_bool (vs_config->getVariable ("graphics","separatespecularcolor","false"))?GFXTRUE:GFXFALSE;
  static Texture * blue=new Texture("blue.png");
  GFXSetSeparateSpecularColor(separatespec);
  if (polygon_offset){
    float a,b;
    GFXGetPolygonOffset(&a,&b);
    GFXPolygonOffset (a, b-polygon_offset);
  }
  
  GFXSelectMaterial(mat);
#define SAFEDECAL(pass,deflt) ((Decal.size()>pass&&Decal[pass])?Decal[pass]:deflt)

#define BASE_UNIT 0
#define ENV_UNIT 1
#define SPEC_UNIT 2
#define GLOW_UNIT 4
#define NORMAL_UNIT 3
#define DAMAGE_UNIT 5
#define DETAIL_UNIT_0 6
#define DETAIL_UNIT_1 7
  
  _Universe->activeStarSystem()->activateLightMap();
  SAFEDECAL(BASE_TEX,white)->MakeActive(BASE_UNIT);
  SAFEDECAL(ENVSPEC_TEX,white)->MakeActive(SPEC_UNIT);
  SAFEDECAL(GLOW_TEX,black)->MakeActive(GLOW_UNIT);
  SAFEDECAL(NORMAL_TEX,blue)->MakeActive(NORMAL_UNIT);
  SAFEDECAL(DAMAGE_TEX,SAFEDECAL(BASE_TEX,white))->MakeActive(DAMAGE_UNIT);
    
  GFXToggleTexture(true,0);
  GFXToggleTexture(true,1);
  GFXToggleTexture(true,2);
  GFXToggleTexture(true,3);
  GFXToggleTexture(true,4);
  GFXToggleTexture(true,5);
  int detailoffset=6;
  if (detailTexture&&(gl_options.Multitexture>detailoffset)) {
    for (unsigned int i=1;i<detailPlanes.size();i+=2) {
      int stage = (i/2)+detailoffset;
      const float params[4]={detailPlanes[i-1].i,detailPlanes[i-1].j,detailPlanes[i-1].k,0};
      const float paramt[4]={detailPlanes[i].i,detailPlanes[i].j,detailPlanes[i].k,0};
      GFXTextureCoordGenMode(stage,OBJECT_LINEAR_GEN,params,paramt);
      detailTexture->MakeActive(stage);
      GFXTextureEnv(stage,GFXDETAILTEXTURE);
      GFXToggleTexture(true,stage);
    }
  }
  GFXShaderConstant(shaderConstants[kDIFFUSEMAP].id,BASE_UNIT);
  GFXShaderConstant(shaderConstants[kENVMAP].id,ENV_UNIT);
  GFXShaderConstant(shaderConstants[kSPECMAP].id,SPEC_UNIT);
  GFXShaderConstant(shaderConstants[kNORMALMAP].id,NORMAL_UNIT);
  GFXShaderConstant(shaderConstants[kGLOWMAP].id,GLOW_UNIT);
  GFXShaderConstant(shaderConstants[kDAMAGEMAP].id,DAMAGE_UNIT);
  GFXShaderConstant(shaderConstants[kDETAIL0MAP].id,DETAIL_UNIT_0);
  GFXShaderConstant(shaderConstants[kDETAIL1MAP].id,DETAIL_UNIT_1);
  float shaderPlane0[4]={0,0,0,0};
  float shaderPlane1[4]={0,0,0,0};
  float envmaprgba[4]={1,1,1,0};
  float noenvmaprgba[4]={0.5,0.5,0.5,1.0};
  if (detailPlanes.size()>0) {
    shaderPlane0[0]=detailPlanes[0].i;
    shaderPlane0[1]=detailPlanes[0].j;
    shaderPlane0[2]=detailPlanes[0].k;
  }
  if (detailPlanes.size()>0) {
    shaderPlane1[0]=detailPlanes[1].i;
    shaderPlane1[1]=detailPlanes[1].j;
    shaderPlane1[2]=detailPlanes[1].k;
  }
  GFXShaderConstant(shaderConstants[kDETAIL0PLANE].id,shaderPlane0);
  GFXShaderConstant(shaderConstants[kDETAIL1PLANE].id,shaderPlane1);
  GFXShaderConstant(shaderConstants[kENVCOLOR].id,envMap?envmaprgba:noenvmaprgba);
#undef SAFEDECAL
}


void RestoreShaders() {
  GFXPopBlendMode();
  for (unsigned int i=2;i<16;i++) {
    GFXToggleTexture(false,i);
  }
}





void RestoreFirstPassState(Texture * detailTexture, const vector<Vector> & detailPlanes, bool skipped_glowpass, bool nomultienv) {
    GFXPopBlendMode();
    if (!nomultienv) GFXDepthFunc(LESS);
    if (detailTexture||skipped_glowpass) {
		static float tempo[4]={1,0,0,0};
		_Universe->activeStarSystem()->activateLightMap();
    };
    if (detailTexture) {
		unsigned int sizeplus1=detailPlanes.size()/2+1;
		for (unsigned int i=1;i<sizeplus1;i++) {
			GFXToggleTexture(false,i+1);//turn off high detial tex
		}
    }
}
void SetupEnvmapPass(Texture * decal, unsigned int mat, int passno) {
    //This is only used when there's no multitexturing... so don't use multitexturing
    GFXPushBlendMode();
    GFXSelectMaterialHighlights(mat,
                                GFXColor(0,0,0,0),
                                GFXColor(0,0,0,0),
                                (passno==2)?GFXColor(1,1,1,1):GFXColor(0,0,0,0),
                                (passno==2)?GFXColor(0,0,0,0):GFXColor(1,1,1,1));
    if (passno==2)
        GFXSetSeparateSpecularColor(GFXFALSE);
    if (passno!=2)
        GFXDisable(LIGHTING);
    if (decal)
        GFXBlendMode((passno!=1)?ONE:ZERO,(passno==1)?SRCCOLOR:(passno?ONE:ZERO)); else
        GFXBlendMode(ONE,ONE);
    if (decal&&((passno==0)||(passno==2))) {
        decal->MakeActive(0);
        GFXToggleTexture(true,0);
    } else if (passno==1) {
        _Universe->activateLightMap(0);
    } else {
        GFXToggleTexture(false,0);
    }
    GFXTextureEnv(0,((passno!=2)?GFXREPLACETEXTURE:GFXMODULATETEXTURE));
    GFXDepthFunc(passno?EQUAL:LESS);
}
void RestoreEnvmapState() {
    float dummy[4];
    static bool separatespec = XMLSupport::parse_bool (vs_config->getVariable ("graphics","separatespecularcolor","false"))?GFXTRUE:GFXFALSE;
    GFXSetSeparateSpecularColor(separatespec);
    GFXEnable(LIGHTING);
    GFXTextureCoordGenMode(0,NO_GEN,dummy,dummy);
    GFXTextureEnv(0,GFXMODULATETEXTURE);
    GFXDepthFunc(LESS);
    GFXPopBlendMode();
    GFXToggleTexture(false,0);
}
void SetupSpecMapSecondPass(Texture * decal,unsigned int mat,BLENDFUNC blendsrc, bool envMap, const GFXColor &cloakFX, float polygon_offset) {
	GFXPushBlendMode();			
    GFXSelectMaterialHighlights(mat,
                                GFXColor(0,0,0,0),
                                GFXColor(0,0,0,0),
                                cloakFX,
                                (envMap?GFXColor (1,1,1,1):GFXColor(0,0,0,0)));
    GFXBlendMode (ONE,ONE);
    if (!envMap||gl_options.Multitexture)
        if (decal) decal->MakeActive();
    //float a,b;
    //GFXGetPolygonOffset(&a,&b);
    //GFXPolygonOffset (a, b-1-polygon_offset); //Not needed, since we use GL_EQUAL and appeal to invariance
    GFXDepthFunc(EQUAL); //By Klauss - this, with invariance, assures correct rendering (and avoids z-buffer artifacts at low res)
    GFXDisable(DEPTHWRITE);
    if (envMap){
      int stage=gl_options.Multitexture?1:0;
      if (!gl_options.Multitexture)
          _Universe->activateLightMap(0);
      GFXEnable(TEXTURE0);
      GFXActiveTexture(stage);
      GFXTextureEnv(stage,GFXMODULATETEXTURE);
      if (gl_options.Multitexture)
          GFXEnable(TEXTURE1);
    }
    else {
      GFXSetSeparateSpecularColor(GFXFALSE);
      GFXTextureEnv(0,GFXMODULATETEXTURE); 
      GFXEnable(TEXTURE0);
      GFXActiveTexture(1);
      GFXDisable(TEXTURE1);
    }
}
void SetupGlowMapFourthPass(Texture * decal,unsigned int mat,BLENDFUNC blendsrc, const GFXColor &cloakFX, float polygon_offset) {
	GFXPushBlendMode();			
    GFXSelectMaterialHighlights(mat,
                                GFXColor(0,0,0,0),
                                GFXColor(0,0,0,0),
								GFXColor(0,0,0,0),
                                cloakFX);
    GFXBlendMode (ONE,ONE);
    if (decal) decal->MakeActive();
    //float a,b;
    //GFXGetPolygonOffset(&a,&b);
    //GFXPolygonOffset (a, b-2-polygon_offset);
    GFXDepthFunc(EQUAL); //By Klauss - this, with invariance, assures correct rendering (and avoids z-buffer artifacts at low res)
    GFXDisable(DEPTHWRITE);
	GFXDisable(TEXTURE1);
}
extern void GFXSelectMaterialAlpha(const unsigned int, float);
void SetupDamageMapThirdPass(Texture * decal,unsigned int mat, float polygon_offset) {
	GFXPushBlendMode();			
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
    if (decal) decal->MakeActive();
    //float a,b;
    //GFXGetPolygonOffset(&a,&b);
    //GFXPolygonOffset (a, b-DAMAGE_PASS-polygon_offset);
    GFXDepthFunc(EQUAL); //By Klauss - this, with invariance, assures correct rendering (and avoids z-buffer artifacts at low res)
    GFXDisable(DEPTHWRITE);
	GFXDisable(TEXTURE1);
}

void RestoreGlowMapState(bool write_to_depthmap, float polygonoffset,float NOT_USED_BUT_BY_HELPER=3) { 
  float a,b;
    //GFXGetPolygonOffset(&a,&b);
    //GFXPolygonOffset (a, b+polygonoffset+NOT_USED_BUT_BY_HELPER);
    GFXDepthFunc(LESS); //By Klauss - restore original depth function
	static bool force_write_to_depthmap=XMLSupport::parse_bool (vs_config->getVariable("graphics","force_glowmap_restore_write_to_depthmap","true"));
	if (force_write_to_depthmap||write_to_depthmap) {
		GFXEnable(DEPTHWRITE);
	}
	GFXEnable(TEXTURE1);
	GFXPopBlendMode();				
}
void RestoreDamageMapState(bool write_to_depthmap, float polygonoffset) {
	RestoreGlowMapState(write_to_depthmap,polygonoffset,DAMAGE_PASS);
}
void RestoreSpecMapState(bool envMap, bool write_to_depthmap, float polygonoffset) { 
    //float a,b;
    //GFXGetPolygonOffset(&a,&b);
    //GFXPolygonOffset (a, b+1+polygonoffset); //Not needed anymore, since InitSpecMapSecondPass() no longer messes with polygon offsets
    GFXDepthFunc(LESS); //By Klauss - restore original depth function
    if (envMap) {
        if (gl_options.Multitexture) {
            GFXActiveTexture(1);
            GFXTextureEnv(1,GFXADDTEXTURE); //restore modulate
        } else {
            float dummy[4];
            GFXTextureCoordGenMode(0,NO_GEN,dummy,dummy);
        }
    }else {
       static bool separatespec = XMLSupport::parse_bool (vs_config->getVariable ("graphics","separatespecularcolor","false"))?GFXTRUE:GFXFALSE;
       GFXSetSeparateSpecularColor(separatespec);
    }
    if (write_to_depthmap) {
        GFXEnable(DEPTHWRITE);
    }
    GFXEnable(TEXTURE0);
	GFXPopBlendMode(); 	
}
extern bool AnimationsLeftInFarQueue();
#ifdef PARTITIONED_Z_BUFFER
void Mesh::ProcessDrawQueue(int whichpass,int whichdrawqueue,float zmin, float zmax) {
#else
void Mesh::ProcessDrawQueue(int whichpass,int whichdrawqueue) {
#endif
  bool shouldsupportshaders=GFXDefaultShaderSupported();
  bool shaders=((Decal.size()>1) || getEnvMap()) && shouldsupportshaders;//for now
  if ((whichpass==ENVSPEC_PASS)||(whichpass==GLOW_PASS)||(whichpass==DAMAGE_PASS)) return;
  if (whichpass!=0&&shaders) return;
  //  assert(draw_queue->size());
  if (!shaders) {
    if (whichpass && (whichpass>=(int)Decal.size())) {
      static bool thiserrdone=false; //Avoid filling up stderr.txt with this thing (it would be output at least once per frame)
      if (!thiserrdone) VSFileSystem::vs_fprintf (stderr,"Fatal error: drawing ship that has a nonexistant tex");
      thiserrdone=true;
      return;
    }
    if ((whichpass < Decal.size()) && (Decal[whichpass]==NULL)) {
      static bool thiserrdone=false; //Avoid filling up stderr.txt with this thing (it would be output at least once per frame)
      if (!thiserrdone) VSFileSystem::vs_fprintf (stderr,"Less Fatal error: drawing ship that has a nonexistant tex");
      thiserrdone=true;
      //return; // the code can handle this
    }
  }
  if (draw_queue[whichdrawqueue].empty()) {
    static bool thiserrdone=false; //Avoid filling up stderr.txt with this thing (it would be output at least once per frame)
    if (!thiserrdone) VSFileSystem::vs_fprintf (stderr,"cloaking queues issue! Report to hellcatv@hotmail.com\nn%d\n%s",whichdrawqueue,hash_name.c_str());
    thiserrdone=true;
    return;
  }
  
#ifdef PARTITIONED_Z_BUFFER
  //Early Z-Cull && S-Cull
  {
      bool cull=true;
      for(unsigned int draw_queue_index=0;cull&&(draw_queue_index<draw_queue[whichdrawqueue].size());++draw_queue_index) {	  
          MeshDrawContext &c =draw_queue[whichdrawqueue][draw_queue_index];
          cull = cull && ((c.mesh_seq!=whichdrawqueue) || !rangesOverlap(zmin,zmax,c.zmin,c.zmax));
      }
      if (cull) return;
  }
#endif
  if (shouldsupportshaders&&!shaders) {
    GFXDeactivateShader();
  }

  bool last_pass = !(whichpass < Decal.size());
  int curdqi=0;
  vector<MeshDrawContext> *cur_draw_queue = &draw_queue[whichdrawqueue];

#ifndef PARTITIONED_Z_BUFFER
  if (whichdrawqueue==NUM_ZBUF_SEQ) {
    for (unsigned int i=0;i<cur_draw_queue->size();i++) {
      MeshDrawContext * c = &((*cur_draw_queue)[i]);
      if (c->mesh_seq==whichdrawqueue) {
	float zfar=(_Universe->AccessCamera()->GetPosition()-c->mat.p).Magnitude()/*+this->radialSize*/;
	if (AnimationsLeftInFarQueue()) {
          if (shaders)
            GFXDeactivateShader();	  
	  Animation::ProcessFarDrawQueue (zfar);
          if (shaders)
            GFXActivateShader(NULL);	  
	}
      }
    }
    GFXEnable(LIGHTING);
  }
#endif

  if (getLighting()) {
    GFXSelectMaterial(myMatNum);
    GFXEnable (LIGHTING);
  }else {
    GFXDisable (LIGHTING);
    GFXColor4f(1,1,1,1);
  }
  bool write_to_depthmap=!(blendDst!=ZERO&&whichdrawqueue!=NUM_ZBUF_SEQ);
  if (!write_to_depthmap) {
    GFXDisable(DEPTHWRITE);
  }
  SelectCullFace(whichdrawqueue);
  GFXBlendMode(blendSrc, blendDst);
  GFXEnable(TEXTURE0);
  if (alphatest)
    GFXAlphaTest(GEQUAL,alphatest/255.0);
  static Texture *white=new Texture("white.png");
  static Texture *black=new Texture("blackclear.png");
  if(Decal.size() && Decal[0])
    Decal[0]->MakeActive();
  GFXTextureEnv(0,GFXMODULATETEXTURE); //Default diffuse mode
  GFXTextureEnv(1,GFXADDTEXTURE);      //Default envmap mode
  
  GFXToggleTexture(bool(Decal.size()>0),0);
  if(getEnvMap()) {
    GFXEnable(TEXTURE1);
    _Universe->activateLightMap();
  } else {
    GFXDisable(TEXTURE1);
  }


  const GFXMaterial &mat=GFXGetMaterial(myMatNum);
  static bool wantsplitpass1 = XMLSupport::parse_bool( vs_config->getVariable("graphics","specmap_with_reflection","false") );
  bool splitpass1 = (wantsplitpass1&&getEnvMap()&&((mat.sa!=0)&&((mat.sr!=0)||(mat.sg!=0)||(mat.sb!=0)))) || (getEnvMap()&&!gl_options.Multitexture&&(ENVSPEC_PASS<Decal.size())&&(Decal[ENVSPEC_PASS]==NULL)); //For now, no PPL reflections with no multitexturing - There is a way, however, doing it before the diffuse texture (odd, I know). If you see this, remind me to do it (Klauss).
  bool skipglowpass = false;
  bool nomultienv = false;
  int nomultienv_passno = 0;

#define SAFEDECAL(pass) ((Decal.size()>pass)?Decal[pass]:black)

  if (!gl_options.Multitexture&&getEnvMap()&&(whichpass==BASE_PASS)) {
      if (SAFEDECAL(ENVSPEC_PASS)) {
          whichpass = ENVSPEC_PASS;
          nomultienv_passno = 0;
      } else {
          nomultienv_passno = 1;
      }
      nomultienv = true;
  }

  while (shaders||((nomultienv&&(whichpass == ENVSPEC_PASS)) || !whichpass || (whichpass < Decal.size()))) {
      if ((whichpass==GLOW_PASS)&&skipglowpass) {
          whichpass++;
          continue;
      }
      if (shaders || (nomultienv&&whichpass==ENVSPEC_PASS) || !whichpass || SAFEDECAL(whichpass)) {
	if (shaders) {
	  SetupShaders(Decal,myMatNum,getEnvMap(),polygon_offset,detailTexture,detailPlanes,black,white);//eventually pass in shader ID for unique shaders
	}else switch (whichpass) {
	case BASE_PASS:
	  SetupSpecMapFirstPass (Decal,myMatNum,getEnvMap(),polygon_offset,detailTexture,detailPlanes,skipglowpass,nomultienv&&SAFEDECAL(ENVSPEC_PASS));
	  break;
	case ENVSPEC_PASS: 
	  if (!nomultienv)
	    SetupSpecMapSecondPass(SAFEDECAL(ENVSPEC_PASS),myMatNum,blendSrc,(splitpass1?false:getEnvMap()), GFXColor(1,1,1,1),polygon_offset); else
	    SetupEnvmapPass(SAFEDECAL(ENVSPEC_PASS),myMatNum,nomultienv_passno);
	  break;
	case DAMAGE_PASS:
	  SetupDamageMapThirdPass(SAFEDECAL(DAMAGE_PASS),myMatNum,polygon_offset);
	  break;
	case GLOW_PASS:
	  SetupGlowMapFourthPass (SAFEDECAL(GLOW_PASS),myMatNum,ONE,GFXColor(1,1,1,1),polygon_offset);
	  break;
	}	
	vlist->BeginDrawState();	
	for(unsigned int draw_queue_index=0;draw_queue_index<cur_draw_queue->size();++draw_queue_index) {	  
	  //Making it static avoids frequent reallocations - although may be troublesome for thread safety
	  //but... WTH... nothing is thread safe in VS.
	  //Also: Be careful with reentrancy... right now, this section is not reentrant.
	  static vector <int> specialfxlight;
	  
	  MeshDrawContext &c =(*cur_draw_queue)[draw_queue_index];
	  if (c.mesh_seq!=whichdrawqueue) 
	    continue;
	  if (c.damage==0&&whichpass==DAMAGE_PASS)
	    continue; //No damage, so why draw it...
	  if ((c.cloaked&MeshDrawContext::CLOAK)&&whichpass!=0)
	    continue; //Cloaking, there are no multiple passes...
	  if (whichdrawqueue!=MESH_SPECIAL_FX_ONLY) {
	    GFXLoadIdentity(MODEL);
	    GFXPickLights (Vector (c.mat.p.i,c.mat.p.j,c.mat.p.k),rSize());
	  }
	  specialfxlight.clear();
	  GFXLoadMatrixModel ( c.mat);
	  unsigned char damaged=((whichpass==DAMAGE_PASS)?c.damage:0);
          if (!shaders)
            SetupCloakState (c.cloaked,c.CloakFX,specialfxlight,damaged,myMatNum);
	  unsigned int i;
	  for ( i=0;i<c.SpecialFX->size();i++) {
	    int ligh;
	    GFXCreateLight (ligh,(*c.SpecialFX)[i],true);
	    specialfxlight.push_back(ligh);
	  }
	  SetupFogState(c.cloaked);
	  if (c.cloaked&MeshDrawContext::RENORMALIZE)
	    glEnable(GL_NORMALIZE);
          if (shaders) {
            if(c.cloaked&MeshDrawContext::CLOAK) {
              GFXPushBlendMode();
              if (c.cloaked&MeshDrawContext::GLASSCLOAK)
                GFXBlendMode(ONE,ONE);
              else
                GFXBlendMode(SRCALPHA,INVSRCALPHA);
            }
            GFXUploadLightState(shaderConstants[kNUMLIGHTS].id,shaderConstants[kACTIVELIGHTS].id,shaders);
            float cloakdata[4]={c.CloakFX.r,c.CloakFX.a,(c.cloaked&MeshDrawContext::CLOAK)?1.0:0.0,(c.cloaked&MeshDrawContext::GLASSCLOAK)?1.0:0.0};
            float damagedata[4]={(float)c.damage/255.0f,0.0,0.0};
            //FIXME should be made static when done debuging
            GFXShaderConstant(shaderConstants[kCLOAKING].id,cloakdata);
            GFXShaderConstant(shaderConstants[kDAMAGED].id,damagedata);                        
          }
	  vlist->Draw();
	  if (c.cloaked&MeshDrawContext::RENORMALIZE)
	    glDisable(GL_NORMALIZE);
	  
	  for ( i=0;i<specialfxlight.size();i++) {
	    GFXDeleteLight (specialfxlight[i]);
	  }
	  RestoreCloakState(c.cloaked,getEnvMap(),damaged);
          
          
	  if(0!=forcelogos&&whichpass==BASE_PASS&&!(c.cloaked&MeshDrawContext::NEARINVIS)) {
	    forcelogos->Draw(c.mat);
	  }
	  if (0!=squadlogos&&whichpass==BASE_PASS&&!(c.cloaked&MeshDrawContext::NEARINVIS)){
	    squadlogos->Draw(c.mat);
	  }
	}
	vlist->EndDrawState();
	
	if (shaders) {
	  RestoreShaders();
	}else switch(whichpass) {
	case BASE_PASS:
	  RestoreFirstPassState(detailTexture,detailPlanes,skipglowpass,nomultienv);
	  break;
	case ENVSPEC_PASS:
	  if (!nomultienv)
	    RestoreSpecMapState((splitpass1?false:getEnvMap()),write_to_depthmap,polygon_offset); else
	    RestoreEnvmapState();
	  break;
	case DAMAGE_PASS:
	  RestoreDamageMapState(write_to_depthmap,polygon_offset);//nothin
	  break;
	case GLOW_PASS:
	  RestoreGlowMapState(write_to_depthmap,polygon_offset);
	  break;
	}  
      }
      switch (whichpass) {
      case BASE_PASS:
          if (Decal.size()>(whichpass=((nomultienv&&SAFEDECAL(ENVSPEC_PASS))?DAMAGE_PASS:ENVSPEC_PASS))) {
              if ((nomultienv&&whichpass==ENVSPEC_PASS)||SAFEDECAL(whichpass)) break;
          } else break;
      case ENVSPEC_PASS:
          if (!nomultienv) {
              if (splitpass1) { 
                  splitpass1=false; 
                  break;
              } else if (Decal.size()>(whichpass=DAMAGE_PASS)) {
                  if (Decal[whichpass]) break;
              } else break;
          } else {
              nomultienv_passno++;
              if (nomultienv_passno>(2-((splitpass1||!SAFEDECAL(ENVSPEC_PASS))?0:1)))
                  whichpass=SAFEDECAL(ENVSPEC_PASS)?BASE_PASS:GLOW_PASS;
              break;
          }
      case DAMAGE_PASS:
          if (Decal.size()>(whichpass=GLOW_PASS)) {
              if (Decal[whichpass]) break;
          } else break;
      default: whichpass++; //always increment pass number, otherwise infinite loop espresso
      }
      if (shaders) break;//only support 1-pass shaders now
  }
  draw_queue[whichdrawqueue].clear();

#undef SAFEDECAL

  if (alphatest) GFXAlphaTest(ALWAYS,0); // Are you sure it was supposed to be after vlist->EndDrawState()? It makes more sense to put it here...

  if (!getLighting()) GFXEnable(LIGHTING);
  if (!write_to_depthmap) GFXEnable(DEPTHWRITE); //risky--for instance logos might be fubar!
  RestoreCullFace(whichdrawqueue);
  if (shouldsupportshaders&&!shaders) {
    GFXActivateShader(NULL);
  }
}
void Mesh::CreateLogos(MeshXML * xml , int faction, Flightgroup * fg) {
  numforcelogo=numsquadlogo =0;
  unsigned int index;
  for (index=0;index<xml->logos.size();index++) {
    if (xml->logos[index].type==0)
      numforcelogo++;
    if (xml->logos[index].type==1)
      numsquadlogo++;
  }
  unsigned int nfl=numforcelogo;
  Logo ** tmplogo=NULL;
  Texture * Dec=NULL;
  for (index=0,nfl=numforcelogo,tmplogo=&forcelogos,Dec=FactionUtil::getForceLogo(faction);index<2;index++,nfl=numsquadlogo,tmplogo=&squadlogos,Dec=(fg==NULL?FactionUtil::getSquadLogo(faction):fg->squadLogo)) {
    if (Dec==NULL) {
      Dec = FactionUtil::getSquadLogo(faction);
    }
    if (nfl==0)
      continue;
    Vector *PolyNormal = new Vector [nfl];
    Vector *center = new Vector [nfl];
    float *sizes = new float [nfl];
    float *rotations = new float [nfl];
    float *offset = new float [nfl];
    Vector *Ref = new Vector [nfl];
    Vector norm1,norm2,norm;
    int ri=0;
    float totoffset=0;
    for (unsigned int ind=0;ind<xml->logos.size();ind++) {
      if (xml->logos[ind].type==index) {
	float weight=0;
	norm1.Set(0,1,0);
	norm2.Set(1,0,0);
	if (xml->logos[ind].refpnt.size()>2) {
	  if (xml->logos[ind].refpnt[0]<xml->vertices.size()&&
	      xml->logos[ind].refpnt[0]>=0&&
	      xml->logos[ind].refpnt[1]<xml->vertices.size()&&
	      xml->logos[ind].refpnt[1]>=0&&
	      xml->logos[ind].refpnt[2]<xml->vertices.size()&&
	      xml->logos[ind].refpnt[2]>=0) {	      
	    norm2=Vector (xml->vertices[xml->logos[ind].refpnt[1]].x-
			  xml->vertices[xml->logos[ind].refpnt[0]].x,
			  xml->vertices[xml->logos[ind].refpnt[1]].y-
			  xml->vertices[xml->logos[ind].refpnt[0]].y,
			  xml->vertices[xml->logos[ind].refpnt[1]].z-
			  xml->vertices[xml->logos[ind].refpnt[0]].z);
	    norm1=Vector (xml->vertices[xml->logos[ind].refpnt[2]].x-
			  xml->vertices[xml->logos[ind].refpnt[0]].x,
			  xml->vertices[xml->logos[ind].refpnt[2]].y-
			  xml->vertices[xml->logos[ind].refpnt[0]].y,
			  xml->vertices[xml->logos[ind].refpnt[2]].z-
			  xml->vertices[xml->logos[ind].refpnt[0]].z);
	  }
	}
	CrossProduct (norm2,norm1,norm);
	
	Normalize(norm);//norm is our normal vect, norm1 is our reference vect
	Vector Cent(0,0,0);
	for (unsigned int rj=0;rj<xml->logos[ind].refpnt.size();rj++) {
	  weight+=xml->logos[ind].refweight[rj];
	  Cent += Vector (xml->vertices[xml->logos[ind].refpnt[rj]].x*xml->logos[ind].refweight[rj],
			  xml->vertices[xml->logos[ind].refpnt[rj]].y*xml->logos[ind].refweight[rj],
			  xml->vertices[xml->logos[ind].refpnt[rj]].z*xml->logos[ind].refweight[rj]);
	}	
	if (weight!=0) {
	  Cent.i/=weight;
	  Cent.j/=weight;
	  Cent.k/=weight;
	}
	//Cent.i-=x_center;
	//Cent.j-=y_center;
	//Cent.k-=z_center;
	Ref[ri]=norm2;
	PolyNormal[ri]=norm;
	center[ri] = Cent;
	sizes[ri]=xml->logos[ind].size*xml->scale.k;
	rotations[ri]=xml->logos[ind].rotate;
	offset[ri]=xml->logos[ind].offset;
	totoffset+=offset[ri];
	ri++;
      }
    }
    totoffset/=nfl;
    *tmplogo= new Logo (nfl,center,PolyNormal,sizes,rotations, totoffset,Dec,Ref);
    delete [] Ref;
    delete []PolyNormal;
    delete []center;
    delete [] sizes;
    delete [] rotations;
    delete [] offset;
  }
}

