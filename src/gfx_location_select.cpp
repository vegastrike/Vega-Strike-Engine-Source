#include "gfx_location_select.h"
#include "gfxlib.h"

LocationSelect::LocationSelect (Vector start, Vector Plane1, Vector Plane2/*, System * par */): Primitive() {
  //  parentScene = par;
  MoveLocation (start,Plane1,Plane2);
}
LocationSelect::LocationSelect (Vector start,Vector Plane1, Vector Plane2, Vector Plane3/*, Scene* par */): Primitive() {
  //  parentScene=par;
  MoveLocation (start,Plane1,Plane2,Plane3);
}
void LocationSelect::MoveLocation (Vector start,Vector Plane1, Vector Plane2) {    
  LocalPosition = Vector(30,20,40);
  r = Plane1.Cross(Plane2);
  Orthogonize (Plane1,Plane2,r);
  p = Plane1;
  q = Plane2;
  r = r;
  pos= start;
  UpdateMatrix();
}
void LocationSelect::MoveLocation (Vector start,Vector Plane1, Vector Plane2, Vector Plane3) {    
  LocalPosition = Vector(0,10,1);
  r = Plane3;
  p = Plane1;
  q = Plane2;
  pos= start;
  UpdateMatrix();
}
void LocationSelect::UpdateMatrix() {
  VectorToMatrix(orientation,p,q,r);
  Translate (translation,pos.i,pos.j,pos.k); 
  MultMatrix (transformation,translation,orientation);
}
LocationSelect::~LocationSelect() {
}


#define POSITION_GFXVertex(x,y,z) (GFXVertex3f(LocalPosition.i+CrosshairSize* (x) ,LocalPosition.j+CrosshairSize* (y) , LocalPosition.k+CrosshairSize* (z) ))

void LocationSelect:: Draw () {
  GFXLoadIdentity(MODEL);
  GFXMultMatrix(MODEL, transformation);
  GFXEnable(DEPTHWRITE);
  GFXEnable(DEPTHTEST);
  GFXDisable(TEXTURE0);
  GFXDisable(TEXTURE1);
  GFXDisable(LIGHTING);
  GFXPushBlendMode();
  //  GFXBlendMode(SRCALPHA,INVSRCALPHA);
  //GFXColor4f (parentScene->HUDColor.r, parentScene->HUDColor.g, parentScene->HUDColor.b, parentScene->HUDColor.a);

  GFXColor4f (0,.5,0,.3);
  GFXBegin(TRIANGLES);
  if (fabs(LocalPosition.k-CrosshairSize)>CrosshairSize) {
    int tmp;
    if (LocalPosition.k>0)
      tmp =1;
    else
      tmp =-1;
    GFXVertex3f (LocalPosition.i,LocalPosition.j,LocalPosition.k-tmp*CrosshairSize);
    GFXVertex3f (LocalPosition.i,LocalPosition.j+CrosshairSize*.125,0);
    GFXVertex3f (LocalPosition.i,LocalPosition.j-CrosshairSize*.125,0);

    GFXVertex3f (LocalPosition.i,LocalPosition.j-CrosshairSize*.125,0);
    GFXVertex3f (LocalPosition.i,LocalPosition.j+CrosshairSize*.125,0);
    GFXVertex3f (LocalPosition.i,LocalPosition.j,LocalPosition.k-tmp*CrosshairSize);

    GFXVertex3f (LocalPosition.i,LocalPosition.j,LocalPosition.k-tmp*CrosshairSize);
    GFXVertex3f (LocalPosition.i+.125*CrosshairSize,LocalPosition.j,0);
    GFXVertex3f (LocalPosition.i-CrosshairSize*.125,LocalPosition.j,0);

    GFXVertex3f (LocalPosition.i-CrosshairSize*.125,LocalPosition.j,0);
    GFXVertex3f (LocalPosition.i+CrosshairSize*.125,LocalPosition.j,0);
    GFXVertex3f (LocalPosition.i,LocalPosition.j,LocalPosition.k-tmp*CrosshairSize);

 }
  if (fabs(LocalPosition.i)+fabs(LocalPosition.j)>CrosshairSize) {

    GFXVertex3f (0,0,0);
    GFXVertex3f (LocalPosition.i,LocalPosition.j,CrosshairSize*.125);
    GFXVertex3f (LocalPosition.i,LocalPosition.j,CrosshairSize*-.125);

    GFXVertex3f (LocalPosition.i,LocalPosition.j,CrosshairSize*-.125);
    GFXVertex3f (LocalPosition.i,LocalPosition.j,CrosshairSize*.125);
    GFXVertex3f (0,0,0);

  }
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (1,.1,.1);	
  POSITION_GFXVertex (1,.1,-.1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (1,-.1,-.1);	
  POSITION_GFXVertex (1,-.1,.1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (1,-.1,.1);
  POSITION_GFXVertex (1,.1,.1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (1,.1,-.1);
  POSITION_GFXVertex (1,-.1,-.1);//one of the arrows to point
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (-1,.1,-.1);	
  POSITION_GFXVertex (-1,.1,.1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (-1,-.1,.1);	
  POSITION_GFXVertex (-1,-.1,-.1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (-1,.1,.1);
  POSITION_GFXVertex (-1,-.1,.1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (-1,-.1,-.1);
  POSITION_GFXVertex (-1,.1,-.1);//one of the arrows to point

  //vertical
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (.1,.1,1);	
  POSITION_GFXVertex (.1,-.1,1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (-.1,-.1,1);	
  POSITION_GFXVertex (-.1,.1,1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (-.1,.1,1);
  POSITION_GFXVertex (.1,.1,1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (.1,-.1,1);
  POSITION_GFXVertex(-.1,-.1,1);//one of the arrows to point
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (.1,-.1,-1);	
  POSITION_GFXVertex (.1,.1,-1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (-.1,.1,-1);	
  POSITION_GFXVertex (-.1,-.1,-1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (.1,.1,-1);
  POSITION_GFXVertex (-.1,.1,-1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (-.1,-.1,-1);
  POSITION_GFXVertex (.1,-.1,-1);//one of the arrows to point


  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (.1,-1,.1);	
  POSITION_GFXVertex (.1,-1,-.1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (-.1,-1,-.1);	
  POSITION_GFXVertex (-.1,-1,.1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (-.1,-1,.1);
  POSITION_GFXVertex (.1,-1,.1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (.1,-1,-.1);
  POSITION_GFXVertex (-.1,-1,-.1);//one of the arrows to point
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (.1,1,-.1);	
  POSITION_GFXVertex (.1,1,.1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (-.1,1,.1);	
  POSITION_GFXVertex (-.1,1,-.1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (.1,1,.1);
  POSITION_GFXVertex (-.1,1,.1);
  POSITION_GFXVertex (0,0,0);
  POSITION_GFXVertex (-.1,1,-.1);
  POSITION_GFXVertex (.1,1,-.1);//one of the arrows to point


 GFXEnd();
  GFXBegin (QUADS);
  //GFXColor4f (parentScene->HUDColor.r, parentScene->HUDColor.g, parentScene->HUDColor.b, 1.5*parentScene->HUDColor.a);

  GFXVertex3f(.5*CrosshairSize +LocalPosition.i,LocalPosition.j,0);
  GFXVertex3f(LocalPosition.i,.5*CrosshairSize+LocalPosition.j,0);
  GFXVertex3f(-.5*CrosshairSize +LocalPosition.i,LocalPosition.j,0);
  GFXVertex3f(LocalPosition.i,-.5*CrosshairSize+LocalPosition.j,0);

  GFXVertex3f(LocalPosition.i,-.5*CrosshairSize+LocalPosition.j,0);
  GFXVertex3f(-.5*CrosshairSize +LocalPosition.i,LocalPosition.j,0);
  GFXVertex3f(LocalPosition.i,.5*CrosshairSize+LocalPosition.j,0);
  GFXVertex3f(.5*CrosshairSize +LocalPosition.i,LocalPosition.j,0);

  GFXColor4f (0,.5,0,.5);
  POSITION_GFXVertex (1,.1,.1); //cap
  POSITION_GFXVertex (1,-.1,.1);
  POSITION_GFXVertex (1,-.1,-.1);
  POSITION_GFXVertex (1,.1,-.1);

  POSITION_GFXVertex (-1,.1,-.1);
  POSITION_GFXVertex (-1,-.1,-.1);
  POSITION_GFXVertex (-1,-.1,.1);
  POSITION_GFXVertex (-1,.1,.1); //cap


  POSITION_GFXVertex (.1,-1,.1); //cap
  POSITION_GFXVertex (-.1,-1,.1);
  POSITION_GFXVertex (-.1,-1,-.1);
  POSITION_GFXVertex (.1,-1,-.1);

  POSITION_GFXVertex (.1,1,-.1);
  POSITION_GFXVertex (-.1,1,-.1);
  POSITION_GFXVertex (-.1,1,.1);
  POSITION_GFXVertex (.1,1,.1); //cap


  POSITION_GFXVertex (.1,.1,1); //cap
  POSITION_GFXVertex (-.1,.1,1);
  POSITION_GFXVertex (-.1,-.1,1);
  POSITION_GFXVertex (.1,-.1,1);

  POSITION_GFXVertex (.1,-.1,-1);
  POSITION_GFXVertex (-.1,-.1,-1);
  POSITION_GFXVertex (-.1,.1,-1);
  POSITION_GFXVertex (.1,.1,-1); //cap



  GFXEnd();
  //  GFXPopBlendMode();
}
#undef POSITION_GFXVertex
