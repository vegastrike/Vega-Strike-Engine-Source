#include <Python.h>
/*namespace boost{namespace python{
template <class T> struct type;
}}
#define DEC_FROM_PYTHON_SMART_POINTER(Pointer) \
class Pointer *from_python(PyObject *p,boost::python::type<class Pointer *>);
class Unit *from_python(PyObject *p,boost::python::type<class Unit *>);
//DEC_FROM_PYTHON_SMART_POINTER(Unit)
*/
#include <boost/python/class_builder.hpp>
#include <boost/python/detail/extension_class.hpp>
#include "python/init.h"
#include <compile.h>
#include <eval.h>
#include "python/python_compile.h"
#include "cmd/ai/fire.h"
#include <memory>
/*
These following #defines will create a module for python
call them with:

PYTHON_BEGIN_MODULE(VS)
	PYTHON_BEGIN_INHERIT_CLASS(VS,FireAt,"PythonFire")
	PYTHON_END_CLASS(VS,FireAt)
	PYTHON_BEGIN_INHERIT_CLASS(VS,BaseClass,"PythonClassName")
	PYTHON_END_CLASS(VS,BaseClass)
	PYTHON_BEGIN_CLASS(VS,MyClass,"PythonClassName")
	Class.def(&MyClass::MyFunc,"FunctionName");
	PYTHON_END_CLASS(VS,MyClass)
PYTHON_END_MODULE(VS)
...
int main (int argc,char *argv[]) {
	...
	PYTHON_INIT_MODULE(VS);
	...
	return 0;
}

*/
std::auto_ptr<boost::python::detail::instance_holder_base>::~auto_ptr() {printf("autoptrdestructor");}
#define TO_PYTHON_SMART_POINTER(Pointer) \
BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE \
PyObject* to_python(class Pointer* x) {return boost::python::python_extension_class_converters<Pointer>::smart_ptr_to_python(x);} \
PyObject* to_python(const class Pointer* p) {return to_python(const_cast<class Pointer*>(p));} \
BOOST_PYTHON_END_CONVERSION_NAMESPACE
#ifdef _WIN32
//		return from_python(p,boost::python::type<SuperClass &>());
//	namespace boost{namespace python{
//	}}
#define ADD_FROM_PYTHON_FUNCTION(SuperClass) \
	SuperClass & from_python(PyObject *obj,boost::python::type<SuperClass &>) { \
        boost::python::detail::extension_instance* self = boost::python::detail::get_extension_instance(obj); \
        typedef std::vector<boost::python::detail::instance_holder_base*>::const_iterator iterator; \
        for (iterator p = self->wrapped_objects().begin(); \
             p != self->wrapped_objects().end(); ++p) \
        { \
            boost::python::detail::instance_holder<SuperClass>* held = dynamic_cast<boost::python::detail::instance_holder<SuperClass>*>(*p); \
            if (held != 0) \
                return *held->target(); \
            void* target = boost::python::detail::class_registry<SuperClass>::class_object()->try_class_conversions(*p); \
            if(target)  \
				return *boost::python::detail::check_non_null(static_cast<SuperClass*>(target)); \
        } \
        boost::python::detail::report_missing_instance_data(self, boost::python::detail::class_registry<SuperClass>::class_object(), typeid(SuperClass)); \
        boost::python::throw_argument_error(); \
        return *((SuperClass*)0); \
	}
//non_null_from_python
#define PYTHON_INIT_INHERIT_GLOBALS(name,SuperClass) PythonClass <SuperClass> *PythonClass< SuperClass >::last_instance = NULL; \
	ADD_FROM_PYTHON_FUNCTION(SuperClass)
#define PYTHON_INIT_GLOBALS(name,Class) ADD_FROM_PYTHON_FUNCTION(Class)
#else
#define PYTHON_INIT_INHERIT_GLOBALS(name,SuperClass) PythonClass <SuperClass> *PythonClass< SuperClass >::last_instance = NULL; 
#define PYTHON_INIT_GLOBALS(name,Class) 
#endif
//These two functions purposely have opening/closing braces that don't match up
#define PYTHON_BEGIN_MODULE(name) BOOST_PYTHON_MODULE_INIT(name) {boost::python::module_builder name(#name);
#define PYTHON_END_MODULE(name) }
#define PYTHON_INIT_MODULE(name) init##name()
#define PYTHON_BEGIN_INHERIT_CLASS(name,SuperClass,myclass) { \
    boost::python::class_builder <SuperClass,PythonClass< SuperClass > > Class (name,myclass); \
    Class.def (boost::python::constructor<>()); \
    Class.def (&SuperClass::Execute,"Execute",PythonClass< SuperClass >::default_Execute);
#define PYTHON_BASE_BEGIN_CLASS(name,CLASS,myclass) { \
    boost::python::class_builder <CLASS> Class (name,myclass);
#define PYTHON_BEGIN_CLASS(name,CLASS,myclass) PYTHON_BASE_BEGIN_CLASS(name,CLASS,myclass) \
    Class.def (boost::python::constructor<>());
#define PYTHON_END_CLASS(name,SuperClass) }
/*    BaseClass.def (&PythonClass<SuperClass>::IncRef,"IncRef"); \
      boost::python::class_builder <SuperClass> TempClass (name,"SuperClass"); */


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
    Py_XINCREF(self);
    last_instance=this;
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
