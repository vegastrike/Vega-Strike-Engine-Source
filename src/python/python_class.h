#include <Python.h>
#include <boost/python/class_builder.hpp>
#include <boost/python/detail/extension_class.hpp>
#include "python/init.h"
#include <compile.h>
#include <eval.h>
#include "python/python_compile.h"
#include "cmd/ai/fire.h"
/*
These following #defines will create a module for python
call them with:

PYTHON_BEGIN_MODULE(VS)
	PYTHON_INIT_CLASS(VS,FireAt,"PythonFire")
	PYTHON_INIT_CLASS(VS,BaseClass,"PythonClassName")
PYTHON_END_MODULE(VS)
...
int main (int argc,char *argv[]) {
	...
	PYTHON_INIT_MODULE(VS);
	...
	return 0;
}

*/

#ifdef _WIN32
//These two functions purposely have opening/closing braces that don't match up
#define PYTHON_INIT_GLOBALS(name,SuperClass) PythonClass <SuperClass> *PythonClass< SuperClass >::last_instance = NULL; \
	SuperClass & from_python(PyObject *p,boost::python::type<SuperClass &>) { \
		return from_python(p,boost::python::type<SuperClass &>()); \
	}
#else
#define PYTHON_INIT_GLOBALS(name,SuperClass) PythonClass <SuperClass> *PythonClass< SuperClass >::last_instance = NULL; 
#endif
#define PYTHON_BEGIN_MODULE(name) BOOST_PYTHON_MODULE_INIT(name) {boost::python::module_builder name(#name);
#define PYTHON_END_MODULE(name) }
#define PYTHON_INIT_MODULE(name) init##name()
#define PYTHON_INIT_CLASS(name,SuperClass,myclass) { \
    boost::python::class_builder <SuperClass,PythonClass< SuperClass > > BaseClass (name,myclass); \
    BaseClass.def (boost::python::constructor<>()); \
    BaseClass.def (&SuperClass::Execute,"Execute",PythonClass< SuperClass >::default_Execute); \
    BaseClass.def (&SuperClass::IncRef,"IncRef"); \
}
//    boost::python::class_builder <SuperClass> TempClass (name,"SuperClass"); 


template <class SuperClass> class PythonClass:public SuperClass {
  PyObject * self;
 protected:
  virtual void Destructor() {
    Py_XDECREF(self);
  }
 public:
  static PythonClass< SuperClass > * last_instance;
  PythonClass (PyObject * self_):SuperClass() {
    self = self_;
    last_instance=this;
  }
  virtual void IncRef() {
    Py_XINCREF(self);
    boost::python::callback<void>::call_method(self, "IncRef");
  }
  virtual void Execute () {
    boost::python::callback <void>::call_method (self,"Execute");
  }
  static void default_Execute(SuperClass & self_) {
    (self_).SuperClass::Execute();
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
