#include "star.h"
#include "vegastrike.h"
#include <GL/gl.h>
//#include "cmd/unit.h"
#define SINX 1
#define SINY 2
#define SINZ 4
//extern Unit ** fighters;
Stars::Stars(int num, float spread): spread(spread){
  int curnum = num/STARnumvlist;
  
  GFXVertex * tmpvertex = new GFXVertex [num/STARnumvlist+num%STARnumvlist];
  memset (tmpvertex,0,sizeof (GFXVertex)*(num/STARnumvlist+num%STARnumvlist));
  ResetPosition(Vector(0,0,0));
  for (int i=0;i<STARnumvlist;i++) {
    curnum += (i==(STARnumvlist-1))?num%STARnumvlist:0;
    for (int j=0;j<curnum;j++) {
      tmpvertex[j].x = -.5*spread+rand()*1.2*((float)spread/RAND_MAX);
      tmpvertex[j].y = -.5*spread+rand()*1.2*((float)spread/RAND_MAX);
      tmpvertex[j].z = -.5*spread+rand()*1.2*((float)spread/RAND_MAX);
    }
    vlists[i]= new GFXVertexList (GFXPOINT,curnum,tmpvertex, false,0);
  }
  delete []tmpvertex;
}

void Stars::Draw() {
  const Vector cp (_Universe->AccessCamera()->GetPosition());
  UpdatePosition(cp);
  GFXColor (1,1,1,1);
  GFXLoadIdentity(MODEL);
  GFXEnable(DEPTHWRITE);
  GFXDisable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXEnable (LIGHTING);
  GFXDisable (DEPTHTEST);
  int ligh;
  GFXLight FadeLight (true, GFXColor (cp.i,cp.j,cp.k), GFXColor (0,0,0,1), GFXColor (0,0,0,1), GFXColor (1,1,1,1), GFXColor (.025,1/(.5*spread),0));
  GFXCreateLight (ligh,FadeLight,true);
  for (int i=0;i<STARnumvlist;i++) {
    if (i>=1)
      GFXTranslate (MODEL,pos[i]-pos[i-1]);
    else
      GFXTranslate (MODEL,pos[i]);
    vlists[i]->Draw();
  }
  GFXDeleteLight (ligh);
  GFXLoadIdentity(MODEL);
}
static void upd (float &a, float &b, float &c, float &d, const float cp, const float spread) {
  while (a-cp > 1*spread) {
    a-=2*spread;
    b-=2*spread;
    c-=2*spread;
    d-=2*spread;
  }
  while (a-cp < -1*spread) {
    a+=2*spread;
    b+=2*spread;
    c+=2*spread;
    d+=2*spread;
  }
}

void Stars::ResetPosition (const Vector &cent){
  for (int i=0;i<STARnumvlist;i++) {
    pos[i].Set (((i&SINX)?-1:1)*.5*spread,((i&SINY)?-1:1)*.5*spread,((i&SINZ)?-1:1)*.5*spread);
    pos[i]+=cent;
  }
}
void Stars::UpdatePosition(const Vector & cp) {
  //  float temp;
  if (fabs(pos[0].i-cp.i)>3*spread||fabs(pos[0].j-cp.j)>3*spread||fabs(pos[0].k-cp.k)>3*spread) {
    ResetPosition(cp);
    return;
  }
  //temp = _Universe->AccessCamera()->GetR().Dot(pos[i]_Universe->AccessCamera()->GetPosition())
  upd (pos[0].i,pos[2].i,pos[4].i, pos[6].i, cp.i, spread);//0 2 4 6 have same x coord...even odd
  upd (pos[1].i,pos[3].i,pos[5].i, pos[7].i, cp.i, spread);

  upd (pos[0].j,pos[1].j,pos[4].j, pos[5].j, cp.j, spread);//0,1,4,5 have same y coord (2 bit off)
  upd (pos[2].j,pos[3].j,pos[6].j, pos[7].j, cp.j, spread);
  
  upd (pos[0].k,pos[1].k,pos[2].k, pos[3].k, cp.k, spread);//1,2,3,4 have same z coord
  upd (pos[4].k,pos[5].k,pos[6].k, pos[7].k, cp.k, spread);

  //  campos = _Universe->AccessCamera()->GetPosition();//for when we move stars twice as fast as ship

}

Stars::~Stars () {
  for (int i=0;i<STARnumvlist;i++) {
    delete vlists[i];
  }
}
