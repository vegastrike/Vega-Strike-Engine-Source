#include "gfx_halo.h"
#include "gfxlib.h"
#include "gfx_aux_texture.h"
#include "vegastrike.h"
static vector <Texture *> HaloDecal;
static vector <int> HaloDecalRef;
static vector <GFXQuadList> halodrawqueue;

Halo::Halo (const char * txtr, const GFXColor &col, const Vector &pos,float sizx,float sizy):position(pos),sizex(sizx),sizey(sizy){
  string texname (txtr);
  Texture * tmpDecal = Texture::Exists(texname);
  unsigned int i=0;
  if (tmpDecal) {
    for (;i<HaloDecal.size();i++) {
      if (HaloDecal[i]==tmpDecal) {
	decal = i;
	HaloDecalRef[i]++;
	break;
      }
    }
  }
  if (!tmpDecal||i==HaloDecal.size()) {
    decal = HaloDecal.size();
    HaloDecal.push_back(new Texture (texname.c_str()));//make sure we have our very own texture to delete in case some other dude is using it
    halodrawqueue.push_back (GFXQuadList(GFXTRUE));
    HaloDecalRef.push_back (1);
  }
  GFXColor coltmp [4] = {GFXColor(col),GFXColor(col),GFXColor(col),GFXColor(col)};
  quadnum = halodrawqueue[decal].AddQuad (NULL,coltmp);  
}
Halo::~Halo () {
  halodrawqueue[decal].DelQuad (quadnum);
  HaloDecalRef[decal]--;
  if (HaloDecalRef[decal]<=0) {
    delete HaloDecal[decal];
    vector <Texture *>::iterator iter = HaloDecal.begin();
    iter+=decal;
    HaloDecal.erase(iter);
    vector <GFXQuadList>::iterator iter1 = halodrawqueue.begin();
    iter1+=decal;
    halodrawqueue.erase(iter1);
    vector <int>::iterator iter2 = HaloDecalRef.begin();
    iter2+=decal;
    HaloDecalRef.erase (iter2);
  }
}
void Halo::Draw (const Transformation &quat, const Matrix m) {
  Vector pos,p,q,r;
  _GFX->AccessCamera()->GetPQR(p,q,r);
  pos=  quat.position+position.Transform(m);
  p=p*sizex;
  r =-r;
  q=q*sizey;
  GFXVertex tmp[4] = {GFXVertex(pos-p-q,r,0,1),
		       GFXVertex(pos+p-q,r,1,1),
		       GFXVertex(pos+p+q,r,1,0),
		       GFXVertex(pos-p+q,r,1,1)};
  halodrawqueue[decal].ModQuad(quadnum,tmp,NULL);
}
void Halo::SetColor (const GFXColor &col){
  GFXColor coltmp [4] = {GFXColor(col),GFXColor(col),GFXColor(col),GFXColor(col)};  
  halodrawqueue[decal].ModQuad (quadnum,NULL,coltmp);
}

void Halo::ProcessDrawQueue() {
  GFXDisable (LIGHTING);
  GFXDisable (CULLFACE);//don't want lighting on this baby
  GFXDisable (DEPTHWRITE);
  GFXPushBlendMode();
  GFXBlendMode(SRCALPHA,INVSRCALPHA);
  GFXEnable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXLoadIdentity(MODEL);
  for (unsigned int decal = 0;decal < halodrawqueue.size();decal++) {	
    HaloDecal[decal]->MakeActive();
    halodrawqueue[decal].Draw();    
  }

  GFXEnable (DEPTHWRITE);
  GFXEnable (CULLFACE);
  GFXDisable (LIGHTING);
  GFXPopBlendMode();
}
