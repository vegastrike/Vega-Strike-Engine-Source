#include "gfx_location_select.h"
#include "gfxlib.h"
#include "in.h"
#include "vegastrike.h"

LocationSelect::LocationSelect (Vector start, Vector Plane1, Vector Plane2/*, System * par */): Primitive() {
  //  parentScene = par;
  MoveLocation (start,Plane1,Plane2);
}
LocationSelect::LocationSelect (Vector start,Vector Plane1, Vector Plane2, Vector Plane3/*, Scene* par */): Primitive() {
  //  parentScene=par;
  MoveLocation (start,Plane1,Plane2,Plane3);
}
extern KBSTATE keyState[KEYMAP_SIZE];
Vector DeltaPosition(0,0,0);
bool changed = false;

void MouseMoveHandle (KBSTATE,int x, int y, int delx, int dely, int mod) {
  if (keyState['z']==DOWN) {
    DeltaPosition.k+=dely;
  } else {
    if (delx||dely) {
      changed=true;
      DeltaPosition.i=delx;
      DeltaPosition.j=dely;
    }
  }
}

void LocationSelect::MoveLocation (Vector start,Vector Plane1, Vector Plane2) {    
  BindKey (2,::MouseMoveHandle);
  LocalPosition = Vector(0,0,0);
  r = Plane1.Cross(Plane2);
  Orthogonize (Plane1,Plane2,r);
  p = Plane1;
  q = Plane2;
  r = r;
  pos= start;
  UpdateMatrix();
}
void LocationSelect::MoveLocation (Vector start,Vector Plane1, Vector Plane2, Vector Plane3) {    
  BindKey (2,::MouseMoveHandle);
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
  UnbindMouse (1);
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

  if (DeltaPosition.k!=0) {
    LocalPosition.k+=.5*DeltaPosition.k;
    DeltaPosition.k=0;
  }
  if (changed&&fabs (DeltaPosition.i<10)&&fabs(DeltaPosition.j<10)) {
    float t[16];
    float m[16];
    float v[16];

    GFXGetMatrix (VIEW,v);
    GFXGetMatrix (MODEL,m);
    MultMatrix(t,v,m);

    //the location in camera coordinates of the beginning of the location select
    Vector tLocation (t[12],t[13],t[14]);
    Vector tP (t[0],t[1],t[2]);//the p vector of the plane being selected on
    Vector tQ (t[4],t[5],t[6]);//the q vector of the plane being selected on

    //the approximate distance away that the cursor is
    float zvalueXY = tLocation.k+LocalPosition.i*tP.k+LocalPosition.j*tQ.k;
    
    LocalPosition.i+= fabs(zvalueXY)*.01*((DeltaPosition.i/tP.i)+(-DeltaPosition.j/tP.j));
    LocalPosition.j+= fabs(zvalueXY)*.01*((DeltaPosition.i/tQ.i)+(-DeltaPosition.j/tQ.j));
    
    DeltaPosition= Vector(0,0,0);
    //    Vector TransPQR (t[0]*i+t[4]*LocalPosition.j+t[8]*LocalPosition.k+t[12],t[1]*LocalPosition.i+t[5]*LocalPosition.j+t[9]*LocalPosition.k+t[13],t[2]*LocalPosition.i+t[6]*LocalPosition.j+t[10]*LocalPosition.k+t[14]);

    changed=false;
  }
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
