#include "star.h"
#include <assert.h>
#include "vegastrike.h"
#include "vs_globals.h"
#include "gfx/camera.h"
#if defined(__APPLE__) || defined(MACOSX)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif
//#include "cmd/unit.h"
#define SINX 1
#define SINY 2
#define SINZ 4

//extern Unit ** fighters;

StarVlist::StarVlist (int num ,float spread) {
	this->spread=spread;
	GFXVertex * tmpvertex = new GFXVertex[num*2];
	memset (tmpvertex,0,sizeof(GFXVertex)*num*2);
	for (int y=0;y<num;++y) {
		int j= 2*y;
		tmpvertex[j].x = -.5*spread+rand()*1.2*((float)spread/RAND_MAX);
		tmpvertex[j].y = -.5*spread+rand()*1.2*((float)spread/RAND_MAX);
		tmpvertex[j].z = -.5*spread+rand()*1.2*((float)spread/RAND_MAX);
		tmpvertex[j+1].x =tmpvertex[j].x;
		tmpvertex[j+1].y =tmpvertex[j].y;
		tmpvertex[j+1].z =tmpvertex[j].z;
		
	}
	vlist= new GFXVertexList (GFXLINE,2*num,tmpvertex, 2*num, true,0);  
	delete []tmpvertex;
}
void StarVlist::BeginDrawState () {
	vlist->LoadDrawState();
	vlist->BeginDrawState();
}
void StarVlist::Draw() {
	vlist->Draw();
	vlist->Draw(GFXPOINT,vlist->GetNumVertices());
}
void StarVlist::EndDrawState() {
	vlist->EndDrawState();
}
Stars::Stars(int num, float spread): vlist((num/STARnumvlist)+1,spread),spread(spread){
  int curnum = num/STARnumvlist+1;
  fade = blend=true;
  ResetPosition(QVector(0,0,0));
}
void Stars::SetBlend(bool blendit, bool fadeit) {
	blend = blendit;
	fade = fadeit;
}
void Stars::Draw() {
  const QVector cp (_Universe->AccessCamera()->GetPosition());
  UpdatePosition(cp);
  //  GFXLightContextAmbient(GFXColor(0,0,0,1));
  GFXColor (1,1,1,1);
  GFXLoadIdentity(MODEL);
  GFXEnable(DEPTHWRITE);
  GFXDisable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXEnable (DEPTHTEST);
  if (blend) {
	GFXBlendMode (ONE,ONE);
  } else {
	GFXBlendMode (ONE,ZERO);
  }
  int ligh;
  GFXSelectMaterial (0);
  if (fade) {
    GFXLight FadeLight (true, GFXColor (cp.i,cp.j,cp.k), GFXColor (0,0,0,1), GFXColor (0,0,0,1), GFXColor (1,1,1,1), GFXColor (.01,0,1/(.4*spread*spread)));
    GFXCreateLight (ligh,FadeLight,true);
    GFXEnable (LIGHTING);
  } else {
    GFXDisable (LIGHTING);
  }
  vlist.BeginDrawState();
  for (int i=0;i<STARnumvlist;i++) {
    if (i>=1)
      GFXTranslateModel (pos[i]-pos[i-1]);
    else
      GFXTranslateModel (pos[i]);
    vlist.Draw();
  }
  vlist.EndDrawState();
  GFXEnable (TEXTURE0);
  GFXEnable (TEXTURE1);
  if (fade)
    GFXDeleteLight (ligh);
  GFXLoadIdentity(MODEL);
}
static void upd (double &a, double &b, double &c, double &d, double &e, double &f, double &g, double &h, double &i, const double cp, const float spread) {
  //  assert (a==b&&b==c&&c==d&&d==e&&e==f);	
  if (a!=b||a!=c||a!=d||a!=e||a!=f||!FINITE (a)) {
    a=b=c=d=e=f=0;
  }
  while (a-cp > 1.5*spread) {
    a-=3*spread;
    b-=3*spread;
    c-=3*spread;
    d-=3*spread;
    e-=3*spread;
    f-=3*spread;
    g-=3*spread;
    h-=3*spread;
    i-=3*spread;
  }
  while (a-cp < -1.5*spread) {
    a+=3*spread;
    b+=3*spread;
    c+=3*spread;
    d+=3*spread;
    e+=3*spread;
    f+=3*spread;
    g+=3*spread;
    h+=3*spread;
    i+=3*spread;
  }
}

void Stars::ResetPosition (const QVector &cent){
  for (int i=0;i<3;i++) {
    for (int j=0;j<3;j++) {
      for (int k=0;k<3;k++) {
	pos[i*9+j*3+k].Set ((i-1)*spread,(j-1)*spread,(k-1)*spread);
	pos[i*9+j*3+k]+=cent;
      }
    }
  }
}
void Stars::UpdatePosition(const QVector & cp) {

  if (fabs(pos[0].i-cp.i)>3*spread||fabs(pos[0].j-cp.j)>3*spread||fabs(pos[0].k-cp.k)>3*spread) {
    ResetPosition(cp);
    return;
  }
  upd (pos[0].i,pos[1].i,pos[2].i, pos[3].i,pos[4].i,pos[5].i,pos[6].i, pos[7].i,pos[8].i, cp.i, spread);
  upd (pos[9].i,pos[10].i,pos[11].i, pos[12].i,pos[13].i,pos[14].i,pos[15].i, pos[16].i,pos[17].i, cp.i, spread);
  upd (pos[18].i,pos[19].i,pos[20].i, pos[21].i,pos[22].i,pos[23].i,pos[24].i, pos[25].i,pos[26].i, cp.i, spread);

  upd (pos[0].j,pos[1].j,pos[2].j, pos[9].j,pos[10].j,pos[11].j,pos[18].j, pos[19].j,pos[20].j, cp.j, spread);
  upd (pos[3].j,pos[4].j,pos[5].j, pos[12].j,pos[13].j,pos[14].j,pos[21].j, pos[22].j,pos[23].j, cp.j, spread);
  upd (pos[6].j,pos[7].j,pos[8].j, pos[15].j,pos[16].j,pos[17].j,pos[24].j, pos[25].j,pos[26].j, cp.j, spread);

  upd (pos[0].k,pos[3].k,pos[6].k, pos[9].k,pos[12].k,pos[15].k,pos[18].k, pos[21].k,pos[24].k, cp.k, spread);
  upd (pos[1].k,pos[4].k,pos[7].k, pos[10].k,pos[13].k,pos[16].k,pos[19].k, pos[22].k,pos[25].k, cp.k, spread);
  upd (pos[2].k,pos[5].k,pos[8].k, pos[11].k,pos[14].k,pos[17].k,pos[20].k, pos[23].k,pos[26].k, cp.k, spread);

}

StarVlist::~StarVlist () {
  delete vlist;

}
