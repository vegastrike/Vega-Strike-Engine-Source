#ifndef _POINT_TO_CAM_H_
#define _POINT_TO_CAM_H_


/**
 * Calculates the way a point-like sphere should be oriented, with a given 
 * input position.  Pos will be tweaked, and p,q,r to be used will be returned.
 * true will be returned if it is close enough to be rendered
 * Uses current camera orientation
 * If local_transformation is used, the object will attempt to use that to keep
 * Its own up alignment, otherwise it will match the camera up
 * If offsetbyradius is used the animation will appear closer than it is
 * If moveiftoofar is true and also offset by radius is true then the item will
 * be moved closer to the cam if it is too far and shrunken appropriately
 */
inline bool CalculateOrientation (Vector & Pos, Vector & p, Vector & q, Vector & r, float &wid, float & hei, float OffsetByThisPercent,bool moveiftoofar, float * local_transformation=NULL) {
  const float kkkk=3;//this seems to work for no good reason
  _Universe->AccessCamera()->GetPQR (p,q,r);
  Vector offset (_Universe->AccessCamera()->GetPosition()-Pos);
  float offz = -r.Dot (offset);
  if (OffsetByThisPercent!=0) {
    float offmag = offset.Magnitude();
    float rad = wid>hei?wid:hei;
      offset*=1./offmag;
      if ((!moveiftoofar)||offz<rad+.4*g_game.zfar) {
	if (offz-OffsetByThisPercent*rad<2*g_game.znear) {
	  rad = (offz-2*g_game.znear)/OffsetByThisPercent;
	}
	offset*=OffsetByThisPercent*rad;
      }else {
	offset *= (offmag/offz)*(offz-2*g_game.znear);//-rad-.4*g_game.zfar);
	wid/=((offz)/(kkkk*g_game.znear));//it's 1 time away from znear 
	hei/=((offz)/(kkkk*g_game.znear));
      }
      Pos+=offset;
      offz+=r.Dot (offset);//coming closer so this means that offz is less
  }
  if (local_transformation) {
    Vector q1 (local_transformation[1],local_transformation[5],local_transformation[9]);
    Vector p1 ((q1.Dot(q))*q);
    q = (q1.Dot(p))*p+p1;			
    Vector posit= _Universe->AccessCamera()->GetPosition();
    r.i = -local_transformation[12]+posit.i;
    r.j = -local_transformation[13]+posit.j;
    r.k = -local_transformation[14]+posit.k;
    Normalize (r);
    ScaledCrossProduct (q,r,p);		
    ScaledCrossProduct (r,p,q); 
    //if the vectors are linearly dependant we're phucked :) fun fun fun
  }
  return offz<.4*g_game.zfar;
}



#endif
