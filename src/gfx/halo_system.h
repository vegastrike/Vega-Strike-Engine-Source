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
  Mesh * mesh;
 public:
  HaloSystem ();
  ~HaloSystem();

  unsigned int  AddHalo (const char * filename, const QVector &loc, const Vector &size, const GFXColor & col);
  void Draw (const Matrix & trans, const Vector & scale, short halo_alpha, float nebdist);
  //  unsigned int size(); {return halo.size();}
  void SetPosition (unsigned int which, const QVector &loc);
  void SetSize (unsigned int which, const Vector &scale);
};
#endif
