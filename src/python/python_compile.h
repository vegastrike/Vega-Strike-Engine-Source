#include "hashtable.h"
#include <Python.h>
#include <string>
#include <compile.h>
extern Hashtable < std::string,PyCodeObject , char [1023]> compiled_python;
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
PyCodeObject *CompilePython (const std::string &filename);
PyObject *CreateTuple (const vector <PythonBasicType> &values);
