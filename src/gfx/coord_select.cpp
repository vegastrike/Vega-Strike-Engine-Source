#include "star_system.h"
#include "loc_select.h"
#include "coord_select.h"
#include "vs_globals.h"
#include "in_kb.h"
#include "gfx/camera.h"
int CoordinateSelectChange =0;
int  CoordinateSelectmousex ;
int  CoordinateSelectmousey ;
extern Vector MouseCoordinate (int mouseX, int mouseY);
extern KBSTATE keyState[KEYMAP_SIZE];
void CoordinateSelect::MouseMoveHandle (KBSTATE,int x,int y,int,int,int) {
  if (keyState['z']==DOWN) {
    CoordinateSelectChange = 2;
  } else {  
    CoordinateSelectChange = 1;
  }
  CoordinateSelectmousex = x;
  CoordinateSelectmousey = y;
}




CoordinateSelect::CoordinateSelect (QVector start):LocSelAni ("locationselect.ani",true,.5,MIPMAP,true), LocalPosition(start) {
  CrosshairSize=2;
  CoordinateSelectmousex = g_game.x_resolution/2;
  CoordinateSelectmousey = g_game.y_resolution/2;
  CoordinateSelectChange = 1;
}
void CoordinateSelect::UpdateMouse() {
  if (CoordinateSelectChange==1) {
    Vector CamPos;
    Vector CamQ,CamR;
    _Universe->AccessCamera()->GetPQR(CamPos,CamQ,CamR);

    Vector mousePoint ( MouseCoordinate(CoordinateSelectmousex,CoordinateSelectmousey));
    float mouseDistance= mousePoint.k*mousePoint.k;
    mousePoint = Transform (CamPos,CamQ,CamR,mousePoint);
    //    QVector cp;
    CamPos=_Universe->AccessCamera()->GetPosition ();  
    //float mouseDistance = mousePoint.Dot (CamR); 
    //distance out into z...straight line...


    float distance = CamR.Dot ((LocalPosition-CamPos).Cast());//distance out into z...straight line...
    //    VSFileSystem::Fprintf (stderr, "distance:%f\n",distance);
    //VSFileSystem::Fprintf (stderr, "mdistance:%f %f\n",mouseDistance,TMD);
    if (mouseDistance!=0) {
      LocalPosition = mousePoint*(distance/mouseDistance)+CamPos.Cast();
    } else {
      LocalPosition = 2*CamR+CamPos.Cast();
    }
    CoordinateSelectChange = 0;
  }
  if (CoordinateSelectChange ==2) {
    Vector CamPos, CamQ,CamR;
    _Universe->AccessCamera()->GetPQR(CamPos,CamQ,CamR);
    CamPos=_Universe->AccessCamera()->GetPosition ();

    LocalPosition = LocalPosition - CamPos;
    float distance = sqrt (CamR.Dot (LocalPosition));//distance out into z...straight line...
    //make it a ratio btw top and bottom.... for near and far;
    float ratio = float(g_game.y_resolution - CoordinateSelectmousey)/g_game.y_resolution;
    float  tmp, n, f;
    GFXGetFrustumVars (true,&tmp,&tmp,&tmp,&tmp,&n,&f);///unkind call :-D
    tmp = n+ratio*ratio*ratio*(f-n);//how far n^3 law
    if (distance!=0) {
      LocalPosition = LocalPosition*(tmp/distance)+CamPos;
    } else {
      LocalPosition = CamPos+CamR*n;
    }
    CoordinateSelectChange = 0;
  }
}
void CoordinateSelect::Draw () {
  if (CoordinateSelectChange) {
    UpdateMouse();
  }
  GFXLoadIdentity (MODEL);
  
  GFXPushBlendMode();
  //  VSFileSystem::Fprintf (stderr,"Location: %f %f %f", LocalPosition.i, LocalPosition.j, LocalPosition.k);
  GFXBlendMode(ONE,ONE);
  LocSelAni.SetPosition(LocalPosition);
  LocSelAni.Draw();
  GFXPopBlendMode();

}
