#ifndef _IKARUS_H_
#define _IKARUS_H_
namespace Orders {
class Ikarus: public AggressiveAI {
    void ExecuteStrategy (Unit * target);
    void DecideTarget();
    void WillFire(Unit * target);
    double last_time;
    double cur_time;
public:
Ikarus();
  virtual void Execute ();
  virtual string getOrderDescription() { return "ikarus"; };
};
}

#endif
