#include "gfxlib.h"
#include "gfx_click_list.h"
#include "cmd_unit.h"


bool ClickList::queryShip (int mouseX, int mouseY,Unit *ship) {   
  if (ship->querySphere(mouseX,mouseY,0)){
    //fprintf (stderr,"bingo A");
    //find some nice mouseX,mouseY translations:
    Vector mousePoint (MouseCoordinate (mouseX,mouseY,1));
    //mousePoint.k= -mousePoint.k;
    Vector CamP,CamQ,CamR;
    _GFX->AccessCamera()->GetPQR(CamP,CamQ,CamR);
    mousePoint = Transform (CamP,CamQ,CamR,mousePoint);	
    _GFX->AccessCamera()->GetPosition(CamP);    
     if (ship->queryBoundingBox(CamP,mousePoint,0)) {
      //fprintf (stderr,"BONGO BOB!!!!!");
      return true;
    }
  }
  return false;
}

ClickList::ClickList (UnitCollection *parIter) {
    parentIter = parIter;
}

UnitCollection * ClickList::requestIterator (int minX,int minY, int maxX, int maxY) {
 
    UnitCollection * uc = new UnitCollection;
    if (minX==maxX||minY==maxY)
      return uc;//nothing in it
    UnitCollection::UnitIterator * UAye = uc->createIterator();
    UnitCollection::UnitIterator * myParent = parentIter->createIterator();
    float view[16];
    float frustmat [16];
    float l, r, b, t , n, f;

    GFXGetFrustumVars (true,&l,&r,&b,&t,&n,&f);
    GFXFrustum (frustmat, view,l*(-2.*minX/g_game.x_resolution+1) /*  *g_game.MouseSensitivityX*/,r*(2.*maxX/g_game.x_resolution-1)/*  *g_game.MouseSensitivityX*/,t*(-2.*minY/g_game.y_resolution+1) /*  *g_game.MouseSensitivityY*/,b*(2.*maxY/g_game.y_resolution-1)/*  *g_game.MouseSensitivityY*/,n,f);
    GFXGetMatrix (VIEW,view);
    float frustum [6][4];
    GFXCalculateFrustum(frustum,view,frustmat);
    while (myParent->advance()) {
      if (myParent->current()->queryFrustum(frustum)) {
	UAye->insert (myParent->current());
      }
    }
    return uc;
}

UnitCollection * ClickList::requestIterator (int mouseX, int mouseY) {

    UnitCollection * uc = new UnitCollection;
    UnitCollection::UnitIterator * UAye = uc->createIterator();
    UnitCollection::UnitIterator * myParent = parentIter->createIterator();
    while (myParent->advance()) {
      if (queryShip(mouseX,mouseY,myParent->current()))
	UAye->insert (myParent->current());
    }
    return uc;
}
