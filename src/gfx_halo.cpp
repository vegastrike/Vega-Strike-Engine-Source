#include "gfx_halo.h"
#include "gfxlib.h"
#include "gfx_aux_texture.h"
#include "vegastrike.h"
static vector <Texture *> HaloDecal;
static vector <int> HaloDecalRef;
static vector <GFXQuadList *> halodrawqueue;

Halo::  Halo (const char * texture, const GFXColor &col, const Vector & pos, float sizx, float sizy ){
  string texname (texture);
  position = pos;
	sizex = sizx;
	sizey = sizy;

  Texture * tmpDecal = Texture::Exists(texname);
  unsigned int i=0;
  int nullio=-1;
  if (tmpDecal) {
    for (;i<HaloDecal.size();i++) {
      if (HaloDecal[i]==tmpDecal) {
	decal = i;
	HaloDecalRef[i]++;
	break;
      }
      if (HaloDecal[i]==NULL&&HaloDecalRef[i]==0)
	nullio=i;
    }
  }
  if (!tmpDecal||i==HaloDecal.size()) {
    if (nullio!=-1) {
      decal=nullio;
      HaloDecal[nullio]=new Texture (texname.c_str());
      HaloDecalRef[nullio]=1;
      halodrawqueue[nullio]=new GFXQuadList(GFXTRUE);
    }else {
      decal = HaloDecal.size();
      HaloDecal.push_back(new Texture (texname.c_str()));//make sure we have our very own texture to delete in case some other dude is using it
      halodrawqueue.push_back (new GFXQuadList(GFXTRUE));
      HaloDecalRef.push_back (1);
    }
  }
  GFXColor coltmp [4] = {GFXColor(col),GFXColor(col),GFXColor(col),GFXColor(col)};
  quadnum = halodrawqueue[decal]->AddQuad (NULL,coltmp);  
}

Halo::~Halo () {
  halodrawqueue[decal]->DelQuad (quadnum);
  HaloDecalRef[decal]--;
  if (HaloDecalRef[decal]<=0) {
    delete HaloDecal[decal];
    HaloDecal[decal]=NULL;
    delete halodrawqueue[decal];//deletes the quad 
    halodrawqueue[decal]=NULL;
  }
}
void Halo::Draw (const Transformation &quat, const Matrix m) {
  Vector pos,p,q,r, offset;

  pos=  position.Transform(m);
  offset = (_Universe->AccessCamera()->GetPosition()-pos);
  offset.Normalize();
  offset*=(sizex>sizey?sizex:sizey);
  _Universe->AccessCamera()->GetPQR(p,q,r);
  p=p*sizex;
  r =-r;
  q=q*sizey;
  //  offset = r*(sizex>sizey?sizex:sizey); //screws up cus of perspective
  GFXVertex tmp[4] = {GFXVertex(pos-p-q+offset,r,0,1),
		       GFXVertex(pos+p-q+offset,r,1,1),
		       GFXVertex(pos+p+q+offset,r,1,0),
		       GFXVertex(pos-p+q+offset,r,0,0)};
  halodrawqueue[decal]->ModQuad(quadnum,tmp,NULL);
}
void Halo::SetColor (const GFXColor &col){
  GFXColor coltmp [4] = {GFXColor(col),GFXColor(col),GFXColor(col),GFXColor(col)};  
  halodrawqueue[decal]->ModQuad (quadnum,NULL,coltmp);
}

void Halo::ProcessDrawQueue() {
  GFXDisable (LIGHTING);
  GFXDisable (DEPTHWRITE);
  GFXPushBlendMode();
  GFXBlendMode(ONE,ONE);
  GFXEnable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXLoadIdentity(MODEL);
  for (unsigned int decal = 0;decal < halodrawqueue.size();decal++) {	
    HaloDecal[decal]->MakeActive();
    halodrawqueue[decal]->Draw();    
  }

  GFXEnable (DEPTHWRITE);
  GFXEnable (CULLFACE);
  GFXDisable (LIGHTING);
  GFXPopBlendMode();
}
