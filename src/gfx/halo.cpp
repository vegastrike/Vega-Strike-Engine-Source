#include "halo.h"
#include "gfxlib.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include "aux_texture.h"
#include "decalqueue.h"
#include "config_xml.h"
#include "xml_support.h"
static DecalQueue halodecal;
static vector <GFXQuadList *> halodrawqueue;

Halo::  Halo (const char * texture, const GFXColor &col, const Vector & pos, float sizx, float sizy ){
  string texname (texture);
  position = pos;
  sizex = sizx;
  sizey = sizy;
  decal = halodecal.AddTexture (texture,MIPMAP);
  if (decal>=halodrawqueue.size()) {
    halodrawqueue.push_back (new GFXQuadList(GFXTRUE));
  }
  GFXColorVertex coltmp [4];
  coltmp[0].SetColor (col);
  coltmp[1].SetColor (col);
  coltmp[2].SetColor (col);
  coltmp[3].SetColor (col);

  quadnum = halodrawqueue[decal]->AddQuad (NULL,coltmp);  
}

Halo::~Halo () {
  halodrawqueue[decal]->DelQuad (quadnum);
  if (halodecal.DelTexture (decal)) {
    delete halodrawqueue[decal];//deletes the quad 
    halodrawqueue[decal]=NULL;
  }
}
void Halo::Draw (const Transformation &quat, const Matrix m, float alpha) {
  Vector pos,p,q,r, offset;
  static float HaloOffset = XMLSupport::parse_float(vs_config->getVariable ("graphics","HaloOffset",".1"));
  pos=  position.Transform(m);
  offset = (_Universe->AccessCamera()->GetPosition()-pos);
  //Matrix HACK  pos = -offset;
  offset.Normalize();
  offset*=HaloOffset*(sizex>sizey?sizex:sizey);
  _Universe->AccessCamera()->GetPQR(p,q,r);
  p=p*sizex;
  r =-r;
  q=q*sizey;
  //  offset = r*(sizex>sizey?sizex:sizey); //screws up cus of perspective
  GFXVertex tmp[4] = {GFXVertex(pos-p-q+offset,r,0,1),
		       GFXVertex(pos+p-q+offset,r,1,1),
		       GFXVertex(pos+p+q+offset,r,1,0),
		       GFXVertex(pos-p+q+offset,r,0,0)};
  halodrawqueue[decal]->ModQuad(quadnum,tmp,alpha);
}
void Halo::SetColor (const GFXColor &col){
  GFXColorVertex coltmp [4];
  coltmp[0].SetColor (col);
  coltmp[1].SetColor (col);
  coltmp[2].SetColor (col);
  coltmp[3].SetColor (col);

  halodrawqueue[decal]->ModQuad (quadnum,coltmp);
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
    if (halodecal.GetTexture (decal)) {
      halodecal.GetTexture(decal)->MakeActive();
      halodrawqueue[decal]->Draw(); 
    }   
  }

  GFXEnable (DEPTHWRITE);
  GFXEnable (CULLFACE);
  GFXDisable (LIGHTING);
  GFXPopBlendMode();
}
