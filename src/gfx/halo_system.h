#ifndef HALOSYSTEM_H_
#define HALOSYSTEM_H_
#include <vector>
class Mesh;
class Matrix;
struct GFXColor;
using std::vector;
class MyIndHalo {
 public:
  Vector size;
  QVector loc;
  MyIndHalo (const QVector &loc, const Vector &size);
};

class HaloSystem {
 private:
  std::vector <MyIndHalo> halo;
#ifdef CAR_SIM
  std::vector <class Animation *> ani;
  std::vector <int> halo_type;
#endif
  Mesh * mesh;
  float activation;
 public:
  HaloSystem ();
  ~HaloSystem();
  QVector& HaloLoc(unsigned int i) {
    return halo[i].loc;
  }
  unsigned int numhalo () {
    return halo.size();
  }
  unsigned int  AddHalo (const char * filename, const QVector &loc, const Vector &size, const GFXColor & col, std::string halo_type/*when it grows*/, float activation_speed);
  bool ShouldDraw (float speedsquared);
  void Draw (const Matrix & trans, const Vector & scale, short halo_alpha, float nebdist, float hullpercentage, const Vector & velocity, int faction);//draws smoke and damage fx
  //  unsigned int size(); {return halo.size();}
  void SetPosition (unsigned int which, const QVector &loc);
  void SetSize (unsigned int which, const Vector &scale);
};
#endif


