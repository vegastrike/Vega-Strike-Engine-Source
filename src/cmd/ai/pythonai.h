#include <Python.h>
#include "fire.h"
#include <assert.h>


using Orders::FireAt;
class PythonAI:public FireAt {
  PyObject * self;
  PythonAI (const PythonAI &a):FireAt (a) {assert(0);}
  static PythonAI * last_ai;
 protected:
  virtual void Destruct();
 public:
  PythonAI (PyObject * self, float reaction_time, float agressivity);
  virtual void Execute ();
  static void default_Execute(FireAt & pay);
  static void InitModuleAI ();
  static PythonAI * LastAI();
  static PythonAI * Factory(const std::string &file);
  virtual ~PythonAI();
};
