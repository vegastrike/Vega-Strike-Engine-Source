#include "fire.h"
#include <Python.h>
#include <assert.h>


int myassert () {
    fprintf (stderr,"Error void * Pointer not equal to long");
    assert (0);
    return 0;
}
int mypointersize = (sizeof (long)==sizeof(void *))?1:myassert();
#define MYPOINTER long

///Warning: The basic pointer class does NOTHING for the user.
///NO Refcounts...if python holds onto this for longer than it can...
///CRASH!!
template <class T> class BasicPointer {
  T * myitem;
 public:
  BasicPointer (T * myitem) {
    this->myitem = myitem;
  }
  T& operator * () {return *myitem;}
};
using Orders::FireAt;
class PythonAI:public FireAt {
  PyObject * self;
  PythonAI (const PythonAI &a):FireAt (a) {assert(0);}
 protected:
  virtual void Destruct();
 public:
  PythonAI (PyObject * self, BasicPointer <Order *>, float reaction_time, float agressivity);
  virtual void Execute ();
  static Order * Factory (const std::string &);
  static void default_Execute(FireAt & pay);
  static void InitModuleAI ();
  virtual ~PythonAI();
};
