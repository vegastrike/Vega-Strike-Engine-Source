#include "bolt.h"
#include "gfxlib.h"
#include "gfx/mesh.h"
#include "gfxlib_struct.h"
#include "gfx/aux_texture.h"
#include "gfx/animation.h"
#include "gfx/decalqueue.h"
#include <vector>

#include <string>
#include <algorithm>
#include "unit.h"
#include "audiolib.h"
#include "config_xml.h"
using std::vector;
using std::string;
GFXVertexList * bolt_draw::boltmesh=NULL;
bolt_draw::~bolt_draw () {

  unsigned int i;
  for (i=0;i<cachedecals.size();i++) {
    boltdecals->DelTexture (cachedecals[i]);
  }
  cachedecals.clear();
  //if (boltmesh)
  //  delete boltmesh;
  //boltmesh = NULL;
  for (i=0;i<animations.size();i++) {
    delete animations[i];
  }
  for (i=0;i<balls.size();i++) {
    for (unsigned int j=0;j<balls[i].size();j++) {
      delete balls[i][j];
    }
  }
  for (i=0;i<bolts.size();i++) {
    for (unsigned int j=0;j<bolts[i].size();j++) {
      delete bolts[i][j];
    }
  }
  delete boltdecals;
}
bolt_draw::bolt_draw () {
  boltdecals = new DecalQueue;
  if (!boltmesh) {
    static float beam_offset =XMLSupport::parse_float (vs_config->getVariable("graphics","bolt_offset","-.2"));
    GFXVertex vtx[12];
#define V(ii,xx,yy,zz,ss,tt) vtx[ii].x=xx;vtx[ii].y=yy;vtx[ii].z=zz+beam_offset+.875;vtx[ii].i=0;vtx[ii].j=0;vtx[ii].k=1;vtx[ii].s=ss;vtx[ii].t=tt;
    V(0,0,0,-.875,0,.5);
    V(1,0,-1,0,.875,1);
    V(2,0,0,.125,1,.5);
    V(3,0,1,0,.875,0);
    V(4,0,0,-.875,0,.5);
    V(5,-1,0,0,.875,1);
    V(6,0,0,.125,1,.5);
    V(7,1,0,0,.875,0);
    V(8,1,0,0,.1875,0);
    V(9,0,1,0,.375,.1875);
    V(10,-1,0,0,.1875,.375);
    V(11,0,-1,0,0,.1875);
    boltmesh = new GFXVertexList (GFXQUAD,12,vtx,12,false);//not mutable;
  }
}

extern double interpolation_blend_factor;

inline void BlendTrans (Matrix & drawmat, const QVector & cur_position, const QVector & prev_position) {
    drawmat.p = prev_position.Scale(1-interpolation_blend_factor) + cur_position.Scale(interpolation_blend_factor);    
}

Bolt::Bolt (const weapon_info & typ, const Matrix &orientationpos,  const Vector & shipspeed, Unit * owner): col (typ.r,typ.g,typ.b,typ.a), cur_position (orientationpos.p), ShipSpeed (shipspeed) {
  VSCONSTRUCT2('t')
  bolt_draw *q= _Universe->activeStarSystem()->bolts;
  prev_position= cur_position;
  this->owner = owner;
  type = typ.type;
  damage = typ.Damage+typ.PhaseDamage;
  if (damage) 
    percentphase=(unsigned char) (255.*typ.PhaseDamage/damage);
  else
    percentphase=0;
  longrange = typ.Longrange;
  radius = typ.Radius;
  speed = typ.Speed/(type==weapon_info::BOLT?typ.Length:typ.Radius);//will scale it by length long!
  range = typ.Range/(type==weapon_info::BOLT?typ.Length:typ.Radius);
  curdist = 0;
  CopyMatrix (drawmat,orientationpos);
  
  if (type==weapon_info::BOLT) {
    ScaleMatrix (drawmat,Vector (typ.Radius,typ.Radius,typ.Length));
    //    if (q->boltmesh==NULL) {
    //      CreateBoltMesh();
    //    }
    decal = q->boltdecals->AddTexture (typ.file.c_str(),MIPMAP);
    if (decal>=(int)q->bolts.size()) {
      q->bolts.push_back (vector <Bolt *>());
      int blargh = q->boltdecals->AddTexture (typ.file.c_str(),MIPMAP);
      if (blargh>=(int)q->bolts.size()) {
	q->bolts.push_back (vector <Bolt *>());	
      }
      q->cachedecals.push_back (blargh);
    }
    q->bolts[decal].push_back (this);
  } else {
    ScaleMatrix (drawmat,Vector (typ.Radius,typ.Radius,typ.Radius));
    decal=-1;
    for (unsigned int i=0;i<q->animationname.size();i++) {
      if (typ.file==q->animationname[i]) {
	decal=i;
      }
    }
    if (decal==-1) {
      decal = q->animations.size();
      q->animationname.push_back (typ.file);
      q->animations.push_back (new Animation (typ.file.c_str(), true,.1,MIPMAP,false));//balls have their own orientation
      q->animations.back()->SetPosition (cur_position);
      q->balls.push_back (vector <Bolt *> ());
    }
    q->balls[decal].push_back (this);
  }
#ifdef PERBOLTSOUND
  sound = AUDCreateSound (typ.sound,false);
  AUDAdjustSound (sound,cur_position,shipspeed+drawmat.getR().Scale(speed));
#endif
}

void Bolt::Draw () {
  bolt_draw *q = _Universe->activeStarSystem()->bolts;
  GFXDisable (LIGHTING);
  GFXDisable (CULLFACE);

  static bool blendbeams = XMLSupport::parse_bool (vs_config->getVariable("graphics","BlendGuns","true"));
  if (blendbeams==true) {
    GFXBlendMode (ONE,ONE);
  }else {
    GFXBlendMode (ONE,ZERO);
  }

  //  GFXDisable(DEPTHTEST);
  GFXDisable(DEPTHWRITE);
  GFXDisable(TEXTURE1);
  GFXEnable (TEXTURE0);

  vector <vector <Bolt *> >::iterator i;
  vector <Bolt *>::iterator j;
  vector <Animation *>::iterator k = q->animations.begin();
  for (i=q->balls.begin();i!=q->balls.end();i++,k++) {
    Animation * cur= *k;
    //Matrix result;
    //FIXME::MuST USE DRAWNOTRANSFORMNOW cur->CalculateOrientation (result);
    for (j=i->begin();j!=i->end();j++) {//don't update time more than once
      Vector p,q,r;
      _Universe->AccessCamera()->GetOrientation(p,q,r);
      BlendTrans ((*j)->drawmat,(*j)->cur_position,(*j)->prev_position);
      Matrix tmp;
      VectorAndPositionToMatrix(tmp,p,q,r,(*j)->drawmat.p);
      //result[12]=(*j)->drawmat[12];
      //result[13]=(*j)->drawmat[13];
      //result[13]=(*j)->drawmat[14];
      //            cur->SetPosition (result[12],result[13],result[14]);
      cur->SetDimensions ((*j)->radius,(*j)->radius);
      //      cur->DrawNow(result);
      GFXLoadMatrixModel (tmp );
#ifdef PERBOLTSOUND
#ifdef PERFRAMESOUND
      if ((*j)->sound!=-1)
	AUDAdjustSound ((*j)->sound,Vector ((*j)->drawmat[12],(*j)->drawmat[13],(*j)->drawmat[14]),(*j)->ShipSpeed+(*j)->speed*Vector ((*j)->drawmat[8],(*j)->drawmat[9],(*j)->drawmat[10]));
#endif
#endif
      GFXColorf ((*j)->col);
      cur->DrawNoTransform(false,true);
    }
    //    cur->UpdateTime (GetElapsedTime());//update the time of the animation;
  }
  GFXDisable(DEPTHWRITE);  
  if (q->boltmesh) {
    q->boltmesh->LoadDrawState();
    q->boltmesh->BeginDrawState();
    int decal=0;
    for (i=q->bolts.begin();i!=q->bolts.end();decal++,i++) {
      Texture * dec = q->boltdecals->GetTexture(decal);
      if (dec) {
	dec->MakeActive();
	for (j=i->begin();j!=i->end();j++) {
	  BlendTrans ((*j)->drawmat,(*j)->cur_position,(*j)->prev_position);
	  GFXLoadMatrixModel ((*j)->drawmat);
	  GFXColorf ((*j)->col);
	  q->boltmesh->Draw();
	}
      }
    }
    q->boltmesh->EndDrawState();
  }
  GFXEnable  (LIGHTING);
  GFXEnable  (CULLFACE);
  GFXBlendMode (ONE,ZERO);
  GFXEnable (DEPTHTEST);
  GFXEnable(DEPTHWRITE);
  GFXEnable (TEXTURE0);
  GFXColor4f(1,1,1,1);
}

Bolt::~Bolt () {
  VSDESTRUCT2
  bolt_draw *q = _Universe->activeStarSystem()->bolts;
  vector <vector <Bolt *> > *target;
  if (type==weapon_info::BOLT) { 
    target = &q->bolts;
    q->boltdecals->DelTexture (decal);
  } else {
    target = &q->balls;
  }
  vector <Bolt *>::iterator tmp= std::find ((*target)[decal].begin(),(*target)[decal].end(),this); 
  if (tmp!=(*target)[decal].end()) {
    (*target)[decal].erase(tmp);
  } else {
    //might as well look for potential faults! Doesn't cost us time
    VSFileSystem::vs_fprintf (stderr,"Bolt Fault! Not found in draw queue! Attempting to recover\n");
    for (vector <vector <Bolt *> > *srch = &q->bolts;srch!=NULL;srch=&q->balls) {
      for (unsigned int mdecal=0;mdecal<(*srch).size();mdecal++) {
	vector <Bolt *>::iterator mtmp= (*srch)[mdecal].begin();
	while (mtmp!=(*srch)[mdecal].end()) {
	  std::find ((*srch)[mdecal].begin(),(*srch)[mdecal].end(),this);       
	  if (mtmp!=(*srch)[mdecal].end()) {
	    (*srch)[mdecal].erase (mtmp);
	    VSFileSystem::vs_fprintf (stderr,"Bolt Fault Recovered\n");
	  }
	}
      }
      if (srch==&q->balls) {
	break;
      }
    }

  }
#ifdef PERBOLTSOUND
  if (sound!=-1) {
    AUDStopPlaying(sound);
    AUDDeleteSound (sound);
  }
#endif
}
