#ifndef __PYTHON_CLASS_H__
#define __PYTHON_CLASS_H__
#include <Python.h>
/*namespace boost{namespace python{
template <class T> struct type;
}}
#define DEC_FROM_PYTHON_SMART_POINTER(Pointer) \
class Pointer *from_python(PyObject *p,boost::python::type<class Pointer *>);
class Unit *from_python(PyObject *p,boost::python::type<class Unit *>);
//DEC_FROM_PYTHON_SMART_POINTER(Unit)
*/
#ifndef USE_BOOST_128
#include "boost/python/object.hpp"
#include "boost/python/class.hpp"
#include "boost/python/call_method.hpp"
#include "boost/python.hpp"///module.hpp>
#define class_builder class_
#else
#include "boost/python/objects.hpp"
#include "boost/python/class_builder.hpp"
#include "boost/python/detail/extension_class.hpp"
#endif

#include "init.h"
#include "cmd/script/pythonmission.h"
#include <compile.h>
#include <eval.h>
#include "python/python_compile.h"
#include "cmd/ai/fire.h"
#include <memory>
/*
These following #defines will create a module for python
call them with:

PYTHON_BEGIN_MODULE(VS)
	PYTHON_BEGIN_INHERIT_CLASS(VS,FireAt,"PythonFire") //begins an inherited class with a virtual Execute function...
	//You can call any other virtual functions by defining:
	//	void callFunction(std::string name){}
	//in your base class...  To use it, use:
	//	MyClass->callFunction("Execute").
	//That will do the same thing as:
	//	"MyClass->Execute()"
	//
	PYTHON_END_CLASS(VS,FireAt)
	PYTHON_BEGIN_INHERIT_CLASS(VS,BaseClass,"PythonVirtualClassName")
	PYTHON_END_CLASS(VS,BaseClass)
	PYTHON_BASE_BEGIN_CLASS(VS,MyClass,"PythonClassName")
	Class.def(boost::python::constructor<int,float,string>); //this will define a constructor that takes an int, float and a string.
	Class.def(&MyClass::MyFunc,"FunctionName");
	PYTHON_END_CLASS(VS,MyClass)
	PYTHON_BEGIN_CLASS(VS,MyOtherClass,"DefaultConstructorPythonClassName") //this will automaticly define a default constructor
	Class.def(&MyOtherClass::MyOtherFunc,"FunctionName");
	PYTHON_END_CLASS(VS,MyOtherClass)
	VS.def(&MyGlobalFunction,"GlobalFunc") //the global functions are easiest; you can call these in python with VS.globalfunc
PYTHON_END_MODULE(VS)
...
int main (int argc,char *argv[]) {
	...
	PYTHON_INIT_MODULE(VS);
	...
	return 0;
}

*/
#define TO_PYTHON_SMART_POINTER(Pointer) \
BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE \
inline PyObject* to_python(class Pointer* x) {return boost::python::python_extension_class_converters<Pointer>::smart_ptr_to_python(x);} \
inline PyObject* to_python(const class Pointer* p) {return to_python(const_cast<class Pointer*>(p));} \
BOOST_PYTHON_END_CONVERSION_NAMESPACE

//		return from_python(p,boost::python::type<SuperClass &>());
//	namespace boost{namespace python{
//	}}
#define ADD_FROM_PYTHON_FUNCTION(SuperClass) \
BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE \
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
	}BOOST_PYTHON_END_CONVERSION_NAMESPACE

//non_null_from_python
#ifdef FROM_PYTHON_ERRORS
#define PYTHON_INIT_INHERIT_GLOBALS(name,SuperClass) PythonClass <SuperClass> *PythonClass< SuperClass >::last_instance = NULL; \
	ADD_FROM_PYTHON_FUNCTION(SuperClass)
#define PYTHON_INIT_GLOBALS(name,Class) ADD_FROM_PYTHON_FUNCTION(Class)
#else
#define PYTHON_INIT_INHERIT_GLOBALS(name,SuperClass) PythonClass <SuperClass> *PythonClass< SuperClass >::last_instance = NULL;
#define PYTHON_INIT_GLOBALS(name,Class)
#endif
//These two functions purposely have opening/closing braces that don't match up
#ifndef USE_BOOST_128
#define PYTHON_BEGIN_MODULE(name) BOOST_PYTHON_MODULE(name) {
#define PYTHON_DEFINE_GLOBAL(modul,fun,funname) boost::python::def (funname,fun)
#define VS_BOOST_MAKE_TUPLE(a,b,c) boost::python::make_tuple(a,b,c)
#define VS_BOOST_MAKE_TUPLE_2(a,b) boost::python::make_tuple(a,b)
#define VS_BOOST_MAKE_TUPLE_4(a,b,c,d) boost::python::make_tuple(a,b,c,d)
#else
#define PYTHON_BEGIN_MODULE(name) BOOST_PYTHON_MODULE_INIT(name) {boost::python::module_builder name(#name);
#define PYTHON_DEFINE_GLOBAL(modul,fun,funname) modul.def (fun,funname)
#define VS_BOOST_MAKE_TUPLE(a,b,c) boost::python::tuple(a,b,c)
#define VS_BOOST_MAKE_TUPLE_2(a,b) boost::python::tuple(a,b)
#define VS_BOOST_MAKE_TUPLE_4(a,b,c,d) boost::python::tuple(a,b,c,d)
#endif
#define PYTHON_END_MODULE(name) }
#define PYTHON_INIT_MODULE(name) init##name()
#ifndef USE_BOOST_128

#define PYTHON_BASE_BEGIN_INHERIT_CLASS(name,NewClass,SuperClass,myclass) { \
boost::python::class_builder <SuperClass, NewClass, boost::noncopyable > Class (myclass
#define PYTHON_BEGIN_INHERIT_CLASS(name,NewClass,SuperClass,myclass) PYTHON_BASE_BEGIN_INHERIT_CLASS(name,NewClass,SuperClass,myclass) \
);

#define PYTHON_BASE_BEGIN_CLASS(name,CLASS,myclass) { \
    boost::python::class_builder <CLASS> Class (myclass
#define PYTHON_BEGIN_CLASS(name,CLASS,myclass) PYTHON_BASE_BEGIN_CLASS(name,CLASS,myclass) \
);
#define PYTHON_DEFINE_METHOD(modul,fun,funname) modul.def (funname,fun)
#define PYTHON_DEFINE_METHOD_DEFAULT(modul,fun,funname,deflt) modul.def (funname,deflt)
#else

#define PYTHON_BASE_BEGIN_INHERIT_CLASS(name,NewClass,SuperClass,myclass) { \
    boost::python::class_builder <SuperClass ,NewClass> Class (name,myclass);
#define PYTHON_BEGIN_INHERIT_CLASS(name,NewClass,SuperClass,myclass) PYTHON_BASE_BEGIN_INHERIT_CLASS(name,NewClass,SuperClass,myclass) \
    Class.def (boost::python::constructor<>());

#define PYTHON_BASE_BEGIN_CLASS(name,CLASS,myclass) { \
    boost::python::class_builder <CLASS> Class (name,myclass);
#define PYTHON_BEGIN_CLASS(name,CLASS,myclass) PYTHON_BASE_BEGIN_CLASS(name,CLASS,myclass) \
    Class.def (boost::python::constructor<>());
#define PYTHON_DEFINE_METHOD(modul,fun,funname) modul.def (fun,funname)
#define PYTHON_DEFINE_METHOD_DEFAULT(modul,fun,funname,defaultfun) modul.def (fun,funname,defaultfun)
#endif
#define PYTHON_END_CLASS(name,SuperClass) }
/*    BaseClass.def (&PythonClass<SuperClass>::IncRef,"IncRef"); \
      boost::python::class_builder <SuperClass> TempClass (name,"SuperClass"); */


template <class SuperClass> class PythonClass:public SuperClass {
 protected:
  PyObject * self;
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
  static PythonClass * LastPythonClass(){
    PythonClass * myclass = last_instance;
    last_instance=NULL;
    return myclass;
  }
  virtual void callFunction (std::string str) {
#ifndef USE_BOOST_128
	  boost::python::call_method<void>(self,str.c_str());
#else
	  boost::python::callback<void>::call_method(self,str.c_str());
#endif
  }
  static PythonClass * Factory(const std::string &file) {
    CompileRunPython (file);
    return LastPythonClass();
  }
  static PythonClass * FactoryString(char * code) {
	Python::reseterrors();
	PyRun_SimpleString (code); //For some reason, PyRun_SimpleString() takes in a char *, not a const char *
	Python::reseterrors();
	return LastPythonClass();
  }
  virtual ~PythonClass(){
    fprintf (stderr,"Destruct called. If called from C++ this is death %ld (0x%lx)",(unsigned long)this,(unsigned long)this);
  }
};
template <class SuperClass> class PythonAI: public PythonClass <SuperClass> {
public:
  PythonAI (PyObject * self_):PythonClass<SuperClass>(self_) {
  }
  virtual void Execute () {
#ifndef USE_BOOST_128
    boost::python::call_method<void> (self,"Execute");
#else
    boost::python::callback <void>::call_method (self,"Execute");
#endif
  }
  virtual void ChooseTarget () {
#ifndef USE_BOOST_128
    boost::python::call_method<void> (self,"ChooseTarget");
#else
    boost::python::callback <void>::call_method (self,"ChooseTarget");
#endif
  }
  virtual void SetParent (Unit * parent) {
    SuperClass::SetParent (parent);
#ifndef USE_BOOST_128
    boost::python::call_method<void> (self,"init",parent);
#else
    boost::python::callback<void>::call_method (self,"init",parent);
#endif
  }
  static void default_Execute(SuperClass & self_) {
    (self_).SuperClass::Execute();
  }
  static void default_ChooseTarget(SuperClass & self_) {
    (self_).SuperClass::ChooseTarget();
  }
  static void default_SetParent (SuperClass &self_, Unit * parent) {
  }
  static PythonClass<SuperClass> * LastPythonClass () {
	  return PythonClass<SuperClass>::LastPythonClass();	  
  }
  static PythonClass<SuperClass> * Factory (const std::string &file) {
	  return PythonClass<SuperClass>::Factory(file);
  }
  static PythonClass<SuperClass> * FactoryString (char *code) {
	  return PythonClass<SuperClass>::FactoryString(code);
  }
};

class pythonMission: public PythonClass <PythonMissionBaseClass> {
public:
  pythonMission (PyObject * self_):PythonClass<PythonMissionBaseClass>(self_) {
  }
  virtual void Execute () {
#ifndef USE_BOOST_128
    boost::python::call_method<void> (self,"Execute");
#else
    boost::python::callback <void>::call_method (self,"Execute");
#endif
    Python::reseterrors();
  }
  virtual std::string Pickle() {
    Python::reseterrors();
#ifndef USE_BOOST_128
	std::string ret=boost::python::call_method<std::string> (self,"Pickle");
#else
	std::string ret=boost::python::callback <std::string>::call_method (self,"Pickle");
#endif
    Python::reseterrors();
	return ret;
  }
  virtual void UnPickle(std::string s)  {
    Python::reseterrors();
#ifndef USE_BOOST_128
    boost::python::call_method<void>(self,"UnPickle",s);
#else
    boost::python::callback<void>::call_method(self,"UnPickle",s);
#endif
    Python::reseterrors();
  }
  static void default_Execute(PythonMissionBaseClass & self_) {
    (self_).PythonMissionBaseClass::Execute();
  }
  static std::string default_Pickle(PythonMissionBaseClass & self_) {
    return (self_).PythonMissionBaseClass::Pickle();
  }
  static void default_UnPickle(PythonMissionBaseClass & self_, std::string str) {
    (self_).PythonMissionBaseClass::UnPickle(str);
  }
  static PythonClass<PythonMissionBaseClass> * Factory (const std::string &file) {
	  return PythonClass<PythonMissionBaseClass>::Factory(file);
  }  
  static PythonClass<PythonMissionBaseClass> * FactoryString (char *code) {
	  return PythonClass<PythonMissionBaseClass>::FactoryString(code);
  }
};
#endif
