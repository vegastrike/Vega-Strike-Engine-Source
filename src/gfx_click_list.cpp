#include "gfxlib.h"
#include "gfx_click_list.h"
#include "cmd_unit.h"

extern Vector mouseline;
extern vector<Vector> perplines;

Vector MouseCoordinate (int mouseX, int mouseY) {
  float l, r, b, t , n, f;
  GFXGetFrustumVars (true,&l,&r,&b,&t,&n,&f);
  //fprintf (stderr, "N: %f\n",n);
  //Vector mousePoint (MouseCoordinate (mouseX,mouseY,1));
  /*  cerr.form("%f, %f, %f, %f\n", l,r,b,t);
  cerr << "top - bottom " <<  t-b << endl;
  cerr << "right - left " <<  r-l << endl;
  */
  return Vector ((l + (r-l) * float(mouseX)/g_game.x_resolution),
		 (t + (b-t) * float(mouseY)/g_game.y_resolution),
		 n);

}


bool ClickList::queryShip (int mouseX, int mouseY,Unit *ship) {   
  Vector mousePoint = MouseCoordinate (mouseX, mouseY);

  cerr << "Mousepoint: " << mousePoint << endl;
    //mousePoint.k= -mousePoint.k;
  Vector CamP,CamQ,CamR;
  _GFX->AccessCamera()->GetPQR(CamP,CamQ,CamR);
  mousePoint = Transform (CamP,CamQ,CamR,mousePoint);	
  cerr << "Transformed Mousepoint: " << mousePoint << endl;
  _GFX->AccessCamera()->GetPosition(CamP);    
  //  if (ship->querySphere(CamP,mousePoint,0)){  //FIXME  bounding spheres seem to be broken
  mousePoint.Normalize();
  mouseline =mousePoint + CamP;
  if (ship->querySphere(CamP+CamR,mousePoint,0)){  // camera position is not actually the center of the camera
      return true;
						//  if (ship->queryBoundingBox(CamP,mousePoint,0)) {
    }
    //}
  return false;
}

ClickList::ClickList ( StarSystem *parSystem, UnitCollection *parIter) {
  lastSelected = NULL;
  lastCollection = NULL;
  parentSystem = parSystem;
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
    _GFX->AccessCamera()->GetView (view);
    float frustum [6][4];
    GFXCalculateFrustum(frustum,view,frustmat);
    Unit * un;
    while (un=myParent->current()) {
      if (un->queryFrustum(frustum)) {
	UAye->insert (un);
      }
      myParent->advance(); 
    }
    delete myParent;
    delete UAye;
    return uc;
    
}

UnitCollection * ClickList::requestIterator (int mouseX, int mouseY) {
  perplines = vector<Vector>();
    UnitCollection * uc = new UnitCollection;
    UnitCollection::UnitIterator * UAye = uc->createIterator();
    UnitCollection::UnitIterator * myParent = parentIter->createIterator();
    Unit * un;
    while (un = myParent->current()) {
	if (queryShip(mouseX,mouseY,un))
	  UAye->insert (un);
	myParent->advance();
    }
    delete myParent;
    delete UAye;
    return uc;
}


Unit * ClickList::requestShip (int mouseX, int mouseY) {
  bool equalCheck=false;
  UnitCollection *uc = requestIterator (mouseX,mouseY);
  UnitCollection::UnitIterator * UAye = NULL;
  Unit *un;
  if (lastCollection!=NULL) {
    equalCheck=true;
    UAye=uc->createIterator();
    UnitCollection::UnitIterator *lastiter = lastCollection->createIterator();
    Unit *lastun;
    while (equalCheck&& (un = UAye->current())&&(lastun=lastiter->current())) {
      if (un !=lastun) {
	equalCheck=false;
      }
      UAye->advance();
      lastiter->advance();
    }    
    delete lastiter;
    delete lastCollection;
    delete UAye;
  }
  float minDistance=1e+10;
  float tmpdis;
  Unit * targetUnit=NULL;
  if (equalCheck&&lastSelected) {//the person clicked the same place and wishes to cycle through units from front to back
    float morethan = lastSelected->getMinDis(parentSystem->AccessCamera()->GetPosition());
    UAye = uc->createIterator();
    while (un=UAye->current()) {
      tmpdis = un->getMinDis (parentSystem->AccessCamera()->GetPosition());
      if (tmpdis>morethan&&tmpdis<minDistance) {
	minDistance=tmpdis;
	targetUnit=un;
      }
      UAye->advance();
    }
    delete UAye;
  }
  if (targetUnit==NULL) {//ok the click location is either different, or 
    //he clicked on the back of the list and wishes to start over
    UAye = uc->createIterator();
    while (un=UAye->current()) {
      tmpdis = un->getMinDis (parentSystem->AccessCamera()->GetPosition());
      if (tmpdis<minDistance) {
	minDistance=tmpdis;
	targetUnit=un;
      }
      UAye->advance();
    }
   
    delete UAye;  
  }
  lastCollection = uc;
  lastSelected = targetUnit;
  return targetUnit;
}
