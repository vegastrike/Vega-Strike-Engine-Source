#include <Python.h>
#include <boost/python/class_builder.hpp>
#include <boost/python/detail/extension_class.hpp>
#include "python/init.h"
#include <compile.h>
#include <eval.h>
#include "python/python_compile.h"
#define INIT_PYTHON_CLASS(superclass,mymodule,myclass) PythonClass< superclass > *PythonClass<superclass>::last_instance = NULL; static bool initsuccess=PythonClass< superclass >::InitModule (mymodule,myclass); 

template <class SuperClass> class PythonClass:public SuperClass {
  PyObject * self;
  static PythonClass * last_instance;
 protected:
  virtual void Destructor() {
    Py_XDECREF(self);
  }
 public:
  PythonClass (PyObject * self_):SuperClass() {
    self = self_;
    Py_XINCREF(self);
    last_instance=this;
  }
  virtual void Execute () {
    boost::python::callback <void>::call_method (self,"Execute");
  }
  static void default_Execute(SuperClass & self_) {
    (self_).SuperClass::Execute();
  }
  static bool InitModule (std::string mymodule, std::string myclass) {
    Python::init();
    boost::python::module_builder ai_builder(mymodule.c_str());
    boost::python::class_builder <SuperClass,PythonClass> BaseClass (ai_builder,myclass.c_str());
    BaseClass.def (boost::python::constructor<>());
    BaseClass.def (&SuperClass::Execute,myclass.c_str(),PythonClass::default_Execute);
    return true;
  }
  static PythonClass * LastPythonClass(){
    PythonClass * myclass = last_instance;
    last_instance=NULL;
    return myclass;
  }
  static PythonClass * Factory(const std::string &file) {
    CompileRunPython (file);
    return LastPythonClass();
  }
  virtual ~PythonClass(){
    fprintf (stderr,"Destruct called. If called from C++ this is death %ld (0x%x)",(unsigned long)this,(unsigned int)this);
  }
};
