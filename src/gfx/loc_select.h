#include "vec.h"

#include "animation.h"
#include "in_mouse.h"
class LocationSelect {
protected:
  Transformation local_transformation;
  float CrosshairSize;
  //  System *parentSystem;
  Animation LocSelAni;
  Animation LocSelUpAni;
  Vector p,q,r;
  Vector LocalPosition;

public:
  static void MouseMoveHandle (KBSTATE,int,int,int,int,int);
  LocationSelect (Vector,Vector,Vector/*, Scene*   */);
  LocationSelect (Vector,Vector,Vector,Vector/*, Scene*   */);
  ~LocationSelect();
  inline Vector GetVector (){return /*Transform (cumulative_transformation_matrix,*/LocalPosition/*)*/;}
  void MoveLocation (Vector pos,Vector p ,Vector q);
  void MoveLocation (Vector pos, Vector p, Vector q, Vector r); //requires normalized Vector triplet pqr
  void Draw();

  QVector &Position();
  void SetPosition (float, float, float);
  void SetPosition (const Vector &);
  void SetOrientation(const Vector &p, const Vector &q, const Vector &r);


};
