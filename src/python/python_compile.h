#ifndef _PYTHON_COMPILE_H_
#define _PYTHON_COMPILE_H_
#include <Python.h>
#include "hashtable.h"
#include <string>
#include <compile.h>
extern Hashtable < std::string, PyCodeObject, 1023> compiled_python;
class PythonBasicType {
 public:
  string objects;
  long objecti;
  double objectd;
  enum {MYSTRING, MYLONG, MYDOUBLE} type;
  PythonBasicType (const std::string &mystr) {
    type=MYSTRING;
    objects = mystr;
  }
  PythonBasicType (const long mystr) {
    type=MYLONG;
    objecti = mystr;
  }
  PythonBasicType (const float mystr) {
    type=MYDOUBLE;
    objectd = mystr;
  }
  PyObject * NewObject () const{
    switch (type) {
    case MYSTRING:
      return PyString_FromString (objects.c_str());
    case MYLONG:
      return PyLong_FromLong (objecti);      
    case MYDOUBLE:
      return PyFloat_FromDouble (objectd);
    }
    return NULL;
  }
};
void InterpretPython (const std::string &filename);
PyCodeObject *CompilePython (const std::string &filename);
void CompileRunPython (const std::string &filename);
PyObject *CreateTuple (const vector <PythonBasicType> &values);

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
#endif
