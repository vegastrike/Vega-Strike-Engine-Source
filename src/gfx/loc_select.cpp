#include "cmd/unit_generic.h"
#include "loc_select.h"
#include "gfxlib.h"
#include "in_kb.h"
#include "vs_globals.h"
#include <stdio.h>
LocationSelect::LocationSelect (Vector start, Vector Plane1, Vector Plane2/*, System * par */): LocSelAni ("locationselect.ani",true,.5,MIPMAP,true),LocSelUpAni("locationselect_up.ani",true,.5,MIPMAP,false) {
  //  parentScene = par;
  CrosshairSize=2;
  MoveLocation (start,Plane1,Plane2);
}
LocationSelect::LocationSelect (Vector start,Vector Plane1, Vector Plane2, Vector Plane3/*, Scene* par */): LocSelAni ("locationselect.ani",true,.5,MIPMAP,true),LocSelUpAni("locationselect_up.ani",true,.5,MIPMAP,false) {
  //  parentScene=par;
  CrosshairSize=2;
  MoveLocation (start,Plane1,Plane2,Plane3);
}
extern KBSTATE keyState[KEYMAP_SIZE];
Vector DeltaPosition(0,0,0);
bool changed = false;
bool vert = false;
#define DELTA_MOVEMENT


void LocationSelect::MouseMoveHandle (KBSTATE kk,int x, int y, int delx, int dely, int mod) {
  if (keyState['z']==DOWN) {
#ifdef DELTA_MOVEMENT
    if (kk==PRESS) {
      DeltaPosition.k=dely;
      vert = true;
    }
    if (dely) {
      DeltaPosition.k=dely;

    }
#else
    if (kk==PRESS){
      DeltaPosition.k=y;
    }
    else if (delx||dely) {
      DeltaPosition.i=x;
      DeltaPosition.j=y;
      vert = true;
    }
#endif
  } else {
    if (delx||dely) {
      DeltaPosition.i=x;
      DeltaPosition.j=y;
      changed=true;
    }
  }
}

void LocationSelect::SetPosition (float x,float y, float z) {
  local_transformation.position = QVector (x,y,z);
}
void LocationSelect::SetPosition (const Vector &k) {
  local_transformation.position = k.Cast();
}
void LocationSelect::SetOrientation(const Vector &p, const Vector &q, const Vector &r)
{	
  local_transformation.orientation = Quaternion::from_vectors(p,q,r);
}

QVector &LocationSelect::Position()
{
	return local_transformation.position;
}


void LocationSelect::MoveLocation (Vector start,Vector Plane1, Vector Plane2) {    
  //BindKey (1,LocationSelect::MouseMoveHandle);
  //UnbindMouse (getMouseDrawFunc()); //don't draw the mouse
  //BindKey (']',::incConstant);
  //BindKey ('[',::decConstant);
  LocalPosition = QVector(0,0,0);
  MakeRVector (Plane1,Plane2,r);
  p = Plane1;
  q = Plane2;


  local_transformation.position = start.Cast();
}
void LocationSelect::MoveLocation (Vector start,Vector Plane1, Vector Plane2, Vector Plane3) {    
  //BindKey (1,::MouseMoveHandle);
  //UnbindMouse (getMouseDrawFunc());
  LocalPosition = QVector(0,0,0);
  r = Plane3;
  p = Plane1;
  q = Plane2;
  local_transformation.position= start.Cast();
}

LocationSelect::~LocationSelect() {
  UnbindMouse (1);
}


#define POSITION_GFXVertex(x,y,z) (GFXVertex3f(LocalPosition.i+CrosshairSize* (x) ,LocalPosition.j+CrosshairSize* (y) , LocalPosition.k+CrosshairSize* (z) ))

void LocationSelect:: Draw () {

  Matrix transformation;
  local_transformation.to_matrix(transformation);
  
  GFXLoadIdentity(MODEL);
  GFXMultMatrixModel (transformation);
  /*
    GFXEnable(DEPTHWRITE);
    GFXEnable(DEPTHTEST);
    GFXDisable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXDisable(LIGHTING);
    GFXPushBlendMode();
    //  GFXBlendMode(SRCALPHA,INVSRCALPHA);
    //GFXColor4f (parentScene->HUDColor.r, parentScene->HUDColor.g, parentScene->HUDColor.b, parentScene->HUDColor.a);

    GFXColor4f (0,.5,0,.3);
*/
#ifdef DELTA_MOVEMENT
  if (vert) {
    LocalPosition.k=0;
    
  }
  vert=false;

  if (DeltaPosition.k) {
    LocalPosition.k-=DeltaPosition.k*.3;
    DeltaPosition.k=0;
  }
#endif

  if (changed||vert) {
    Matrix t,m;
    Matrix v;

    GFXGetMatrixView (v);

    GFXGetMatrixModel (m);
    MultMatrix(t,v,m);

    //following translates 'z'...not sure it's necessary
    //    Translate (v,0,0,LocalPosition.k);
    //    MultMatrix (m,t,v);

    //the location in camera coordinates of the beginning of the location select
    Vector tLocation (t.p.Cast());
    Vector tP (t.getP());//the p vector of the plane being selected on
    Vector tQ (t.getQ());//the q vector of the plane being selected on
    ///unused    Vector tR (t[8],t[9],t[10]);//the q vector of the plane being selected on
    //VSFileSystem::Fprintf (stderr,"<%f,%f,%f>",t[0],t[1],t[2]);
    //VSFileSystem::Fprintf (stderr,"<%f,%f,%f>",t[4],t[5],t[6]);
    //VSFileSystem::Fprintf (stderr,"<%f,%f,%f>",t[8],t[9],t[10]);
#ifdef DELTA_MOVEMENT
    float zvalueXY = tLocation.k+LocalPosition.i*tP.k+LocalPosition.j*tQ.k; // z val of the parallelogram
#else
    float zvalueXY = tLocation.k+LocalPosition.i*tP.k+LocalPosition.j*tQ.k+LocalPosition.k*tR.k; //zvalue of the cursor
#endif

    if (changed&&!vert) {    //planar movement
    
    if (zvalueXY >1000)  /// zfar
      zvalueXY = 1000;
    if (zvalueXY<-1000)
      zvalueXY = -1000;

      LocalPosition.i= fabs(zvalueXY)*(((2*DeltaPosition.i/g_game.x_resolution - 1)*g_game.MouseSensitivityX*GFXGetXInvPerspective()*tP.i)-(1-(2*DeltaPosition.j/g_game.y_resolution)*g_game.MouseSensitivityY*GFXGetYInvPerspective()*tP.j));
      LocalPosition.j= fabs(zvalueXY)*(((2*DeltaPosition.i/g_game.x_resolution - 1)*g_game.MouseSensitivityX*GFXGetXInvPerspective()*tQ.i)-(1-(2*DeltaPosition.j/g_game.y_resolution)*tQ.j*g_game.MouseSensitivityY*GFXGetYInvPerspective()));
      DeltaPosition= Vector(0,0,0);
      //    Vector TransPQR (t[0]*i+t[4]*LocalPosition.j+t[8]*LocalPosition.k+t[12],t[1]*LocalPosition.i+t[5]*LocalPosition.j+t[9]*LocalPosition.k+t[13],t[2]*LocalPosition.i+t[6]*LocalPosition.j+t[10]*LocalPosition.k+t[14]);
      changed=false;
    }
#ifndef DELTA_MOVEMENT 
    else { //vertical movement

    if (zvalueXY >1000)  /// zfar
      zvalueXY = 1000;
    if (zvalueXY<-1000)
      zvalueXY = -1000;

      LocalPosition.k= fabs(zvalueXY)*(((2*DeltaPosition.i/g_game.x_resolution - 1)*g_game.MouseSensitivityX*GFXGetXInvPerspective()*tR.i)-((2*DeltaPosition.j/g_game.y_resolution -1)*g_game.MouseSensitivityY*GFXGetYInvPerspective()*tR.j));
      if (DeltaPosition.k) {
	LocalPosition.k=0;
	DeltaPosition.k=0;
      }
      vert =false;
      changed=false;
    }
#endif
  }

  //draw the animation
  LocSelUpAni.SetPosition (QVector (LocalPosition.i,LocalPosition.j,0));
  LocSelUpAni.Draw();
  LocSelAni.SetPosition(LocalPosition);
  LocSelAni.Draw();
  

  /*
  GFXBegin(TRIANGLES);
  if (fabs(LocalPosition.k-CrosshairSize)>CrosshairSize) {
    int tmp;
    if (LocalPosition.k>=0)
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
*/

  GFXPopBlendMode();


}


#undef POSITION_GFXVertex
