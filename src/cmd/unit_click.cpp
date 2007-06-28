#include "vegastrike.h"
#include "gfx/mesh.h"
//#include "unit.h"
//#include "unit_template.h"
#include "gfx/bounding_box.h"
extern Vector mouseline;
extern vector<Vector> perplines;
Vector MouseCoordinate (int mouseX, int mouseY);

template <class UnitType>
bool GameUnit<UnitType>::querySphereClickList (int mouseX, int mouseY, float err, Camera * activeCam) {
  int i;
  Matrix vw;
  _Universe->AccessCamera()->GetView (vw);
  Vector mousePoint;
#ifdef VARIABLE_LENGTH_PQR
  Vector TargetPoint (cumulative_transformation_matrix[0],cumulative_transformation_matrix[1],cumulative_transformation_matrix[2]);
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));
#else
  Vector TargetPoint;
#endif

  Vector CamP,CamQ,CamR;
  for (i=0;i<nummesh();i++) {
    //cerr << "pretransform position: " << meshdata[i]->Position() << endl;
    TargetPoint = Transform(this->cumulative_transformation_matrix,this->meshdata[i]->Position());
    
    mousePoint = Transform (vw,TargetPoint);
    if (mousePoint.k>0) { //z coordinate reversed  -  is in front of camera
      continue;
    }
    mousePoint = MouseCoordinate (mouseX,mouseY);
    
    activeCam->GetPQR(CamP,CamQ,CamR);
    mousePoint = Transform (CamP,CamQ,CamR,mousePoint);	
    CamP = activeCam->GetPosition().Cast();    
    mousePoint +=CamP; 
    
    
    
    TargetPoint =TargetPoint-mousePoint;
    if (TargetPoint.Dot (TargetPoint)< 
	err*err+
	this->meshdata[i]->rSize()*this->meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
	*SizeScaleFactor*SizeScaleFactor
#endif
	+
#ifdef VARIABLE_LENGTH_PQR
	SizeScaleFactor*
#endif
	2*err*this->meshdata[i]->rSize()
	)
      return true;
  }
  Unit * su;
  for(un_iter ui = this->getSubUnits();su = *ui;++ui){
    if ((su)->querySphereClickList (mouseX,mouseY,err,activeCam)) {
      return true;
    }
  }
  return false;
}
