#ifndef _CMD_NAVIGATION_ORDERS_H_
#define _CMD_NAVIGATION_ORDERS_H_


#include "order.h"
#include "vegastrike.h"
#include "vs_globals.h"

namespace Orders {
  const float bleed_threshold = 0.0001;
  const float THRESHOLD = 0.01;
  const unsigned char ABURN = 1;

  

  /**
   * The moveto order attempts to calculate the best way to apply thrust (within the computer bound limits) to get a starship to place B and stopped.
   * It uses an integral of acceleration and velocity over time to solve for 
   * time when to decelerate.  Is  inaccurate within 1 physics frame, and must
   * use switchbacks and then once they have been met sets terminating X,Y, and Z 
   * to figure out how many switchbacks it has made
   * , missing the target and coming back over it.
   */
class MoveTo : public Order {
  unsigned char afterburnAndSwitchbacks;//don't need the lowest order bit
  unsigned char terminatingX;
  unsigned char terminatingY;
  unsigned char terminatingZ;
  ///The last_velocity keeps track of the previous velocity so the script may determine if it has crossed over 0 this frame or not
  Vector last_velocity;
  bool OptimizeSpeed (float v, float &a);
  bool Done (const Vector &);
  bool selfterminating;
public:
  void SetAfterburn(bool tf) {
    if (tf) {
      afterburnAndSwitchbacks|=1;
    }else {
      afterburnAndSwitchbacks&= (~1);
    }
  }
  ///takes in the destination target, whether afterburners should be applied, and the ammount of accuracy (how many times it shoudl miss destination and come back) should be used
  MoveTo(const QVector &target, bool aft, unsigned char numswitchbacks, bool terminating=true) : Order(MOVEMENT,SLOCATION), afterburnAndSwitchbacks(aft+(numswitchbacks<<1)),terminatingX(0), terminatingY(0), terminatingZ(0), last_velocity(0,0,0), selfterminating(terminating) {
    targetlocation = target;
    done=false;
  }
  void SetDest (const QVector&);
  virtual void Execute();
  virtual ~MoveTo();
  virtual string getOrderDescription() { return "moveto"; };
};
/**
 * This AI script attempts to change headings to face a given direction
 * again it is inaccurate to within 1 physics frame, though calculating thrust
 * at 1/3 the way through a physics frame has made this effect of wobbling
 * all but subside! switchbacks keep track of how many times it has almost
 * but passed over target destination
 */
class ChangeHeading : public Order {
  float turningspeed;
  unsigned char switchbacks;//don't need the lowest order bit
  unsigned char terminatingX;
  unsigned char terminatingY;
  Vector last_velocity;
  QVector final_heading;
  bool terminating;
  bool OptimizeAngSpeed(float limit, float v, float &a);
  bool Done (const Vector &);
  void TurnToward (float angle, float ang_vel, float &torque);
protected:
  void ResetDone () {done = false; terminatingX=terminatingY=0;}
 
 public:
  ///takes in the destination target, and the ammount of accuracy (how many times it should miss destination and come back) should be used
   ChangeHeading(const QVector &final_heading, int switchback, float turning_speed=1) : Order(FACING,SLOCATION), turningspeed(turning_speed), switchbacks(switchback),terminatingX(0),terminatingY(0),last_velocity(0,0,0),final_heading(final_heading), terminating(false) {}
  void SetDest (const QVector&);
  virtual void Execute();
  virtual string getOrderDescription() { return "chhead"; };
  virtual ~ChangeHeading();
};
/**
 * This class analyzes a Unit's position and adjusts ChangeHeading to face
 * that target's center
 */
class FaceTarget : public ChangeHeading {
  bool finish;
public:
  FaceTarget (bool fini=false, int accuracy =3);
  virtual void Execute ();
  virtual string getOrderDescription() { return "facet"; };
  virtual ~FaceTarget();
};
/**
 * This class analyzes a Unit's position and adjusts ChangeHeading to face
 * that target's ITTS indicator for best firing solution
 */
class FaceTargetITTS : public ChangeHeading {
  bool finish;
  ///The average speed of this target's guns
  float speed;
  ///The range of this target's guns
  float range;
public:
  FaceTargetITTS (bool fini=false, int accuracy = 3);
  virtual void Execute();
  virtual string getOrderDescription() { return "faceitts"; }
  virtual ~FaceTargetITTS();
};
 class FormUp : public MoveTo {
   QVector Pos;
 public:
   FormUp(const QVector &Position);
   void SetPos (const QVector &);
  virtual void SetParent (Unit * parent1);
   virtual void Execute();
   virtual string getOrderDescription () {return "formup";}
   virtual ~FormUp();
 };
 class FaceDirection: public ChangeHeading {
  bool finish;
  float dist;
public:
  FaceDirection (float distToMatchFacing, bool fini=false, int accuracy = 3);
  virtual void SetParent (Unit * parent1);
  virtual void Execute();
  virtual string getOrderDescription() { return "facedir"; }
  virtual ~FaceDirection();
 };

}
#endif

