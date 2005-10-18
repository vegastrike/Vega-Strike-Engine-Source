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
    for (int j=balls[i].size()-1;j>=0;j--) {
      balls[i][j].Destroy(j);
    }
  }
  for (i=0;i<bolts.size();i++) {
    for (int j=bolts[i].size()-1;j>=0;j--) {
      bolts[i][j].Destroy(j);
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

Bolt::Bolt (const weapon_info * typ, const Matrix &orientationpos,  const Vector & shipspeed, void * owner, CollideMap::iterator hint):  cur_position (orientationpos.p), ShipSpeed (shipspeed) {
  VSCONSTRUCT2('t')
  bolt_draw *q= _Universe->activeStarSystem()->bolts;
  prev_position= cur_position;
  this->owner = owner;
  this->type = typ;
  curdist = 0;
  CopyMatrix (drawmat,orientationpos);
  Vector vel=shipspeed+orientationpos.getR()*typ->Speed;
  if (typ->type==weapon_info::BOLT) {
    ScaleMatrix (drawmat,Vector (typ->Radius,typ->Radius,typ->Length));
    decal = q->boltdecals->AddTexture (typ->file.c_str(),MIPMAP);
    if (decal>=(int)q->bolts.size()) {
      q->bolts.push_back (vector <Bolt>());
      int blargh = q->boltdecals->AddTexture (typ->file.c_str(),MIPMAP);
      if (blargh>=(int)q->bolts.size()) {
	q->bolts.push_back (vector <Bolt>());	
      }
      q->cachedecals.push_back (blargh);
    }
    this->location=_Universe->activeStarSystem()->collidemap->insert(Collidable(Bolt::BoltIndex(q->bolts[decal].size(),decal,false).bolt_index,(shipspeed+orientationpos.getR()*typ->Speed).Magnitude()*.5,cur_position+vel*SIMULATION_ATOM*.5),hint);
    q->bolts[decal].push_back (*this);
  } else {
    ScaleMatrix (drawmat,Vector (typ->Radius,typ->Radius,typ->Radius));
    decal=-1;
    for (unsigned int i=0;i<q->animationname.size();i++) {
      if (typ->file==q->animationname[i]) {
	decal=i;
      }
    }
    if (decal==-1) {
      decal = q->animations.size();
      q->animationname.push_back (typ->file);
      q->animations.push_back (new Animation (typ->file.c_str(), true,.1,MIPMAP,false));//balls have their own orientation
      q->animations.back()->SetPosition (cur_position);
      q->balls.push_back (vector <Bolt> ());
    }
    this->location=_Universe->activeStarSystem()->collidemap->insert(Collidable(Bolt::BoltIndex(q->balls[decal].size(),decal,true).bolt_index,(shipspeed+orientationpos.getR()*typ->Speed).Magnitude()*.5,cur_position+vel*SIMULATION_ATOM*.5),hint);
    q->balls[decal].push_back (*this);
  }
}

void Bolt::Draw () {
  bolt_draw *q = _Universe->activeStarSystem()->bolts;
  GFXDisable (LIGHTING);
  GFXDisable (CULLFACE);

  static bool blendbeams = XMLSupport::parse_bool (vs_config->getVariable("graphics","BlendGuns","true"));
  static float stretchbolts = XMLSupport::parse_float (vs_config->getVariable("graphics","StretchBolts","0.85"));
  GFXBlendMode (ONE,blendbeams?ONE:ZERO);

  //  GFXDisable(DEPTHTEST);
  GFXDisable(DEPTHWRITE);
  GFXDisable(TEXTURE1);
  GFXEnable (TEXTURE0);
  GFXAlphaTest (GREATER,.1);
  vector <vector <Bolt> >::iterator i;
  vector <Bolt>::iterator j;
  vector <Animation *>::iterator k = q->animations.begin();
  float etime = GetElapsedTime();
  for (i=q->balls.begin();i!=q->balls.end();i++,k++) {
    Animation * cur= *k;
    //Matrix result;
    //FIXME::MuST USE DRAWNOTRANSFORMNOW cur->CalculateOrientation (result);
    for (j=i->begin();j!=i->end();j++) {//don't update time more than once
      Vector p,q,r;
      Bolt * bolt=&*j;
      const weapon_info * type = bolt->type;
      _Universe->AccessCamera()->GetOrientation(p,q,r);
      BlendTrans (bolt->drawmat,bolt->cur_position,bolt->prev_position);
      Matrix tmp;
      VectorAndPositionToMatrix(tmp,p,q,r,bolt->drawmat.p);
      //result[12]=(*j)->drawmat[12];
      //result[13]=(*j)->drawmat[13];
      //result[13]=(*j)->drawmat[14];
      //            cur->SetPosition (result[12],result[13],result[14]);
      cur->SetDimensions (bolt->type->Radius,bolt->type->Radius);
      //      cur->DrawNow(result);
      GFXLoadMatrixModel (tmp );
      GFXColor4f (type->r,type->g,type->b,type->a);
      cur->DrawNoTransform(false,true);
    }
    //    cur->UpdateTime (GetElapsedTime());//update the time of the animation;
  }
  GFXAlphaTest (ALWAYS,0);
  GFXDisable(DEPTHWRITE);
  GFXDisable(TEXTURE1);
  if (blendbeams==true) {
    GFXBlendMode (ONE,ONE);
  }else {
    GFXBlendMode (ONE,ZERO);
  }
  GFXVertexList * qmesh=q->boltmesh;
  if (qmesh) {
    qmesh->LoadDrawState();
    qmesh->BeginDrawState();
    int decal=0;
    for (i=q->bolts.begin();i!=q->bolts.end();decal++,i++) {
      Texture * dec = q->boltdecals->GetTexture(decal);
      if (dec) {
	    dec->MakeActive();
        GFXToggleTexture(true,0);
	    for (j=i->begin();j!=i->end();j++) {
          Bolt &bolt=*j;
          const weapon_info *wt=bolt.type;

	      BlendTrans (bolt.drawmat,bolt.cur_position,bolt.prev_position);
          Matrix drawmat(bolt.drawmat);
          if (stretchbolts>0)
              ScaleMatrix(drawmat,Vector(1,1,bolt.type->Speed*etime*stretchbolts/bolt.type->Length));
	      GFXLoadMatrixModel (drawmat);
	      GFXColor4f (wt->r,wt->g,wt->b,wt->a);
	      qmesh->Draw();
	    }
      }
    }
    qmesh->EndDrawState();
  }
  GFXEnable  (LIGHTING);
  GFXEnable  (CULLFACE);
  GFXBlendMode (ONE,ZERO);
  GFXEnable (DEPTHTEST);
  GFXEnable(DEPTHWRITE);
  GFXEnable (TEXTURE0);
  GFXColor4f(1,1,1,1);
}

extern void BoltDestroyGeneric(Bolt * whichbolt, int index, int decal, bool isBall);
void Bolt::Destroy (int index) {
  VSDESTRUCT2
  bolt_draw *q = _Universe->activeStarSystem()->bolts;
  bool isBall=true;
  if (type->type==weapon_info::BOLT) { 
    q->boltdecals->DelTexture (decal);
    isBall=false;
  } else {

  }
  BoltDestroyGeneric(this, index,decal,isBall);
}
