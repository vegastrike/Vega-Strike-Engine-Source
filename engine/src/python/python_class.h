/*
 * python_class.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Nachum Barcohen,
 * Stephen G. Tuggy, and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef __PYTHON_CLASS_H__
#define __PYTHON_CLASS_H__
//This takes care of the fact that several systems use the _POSIX_C_SOURCES
//variable and don't set them to the same thing.
//Python.h sets and uses it
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#include <Python.h>

#include <boost/version.hpp>

#if defined (_MSC_VER) && _MSC_VER <= 1200
#define Vector Vactor
#endif
#include "boost/python/object.hpp"
#include "boost/python/class.hpp"
#include "boost/python/call_method.hpp"
#include <boost/python.hpp>///module.hpp>
#if defined (_MSC_VER) && _MSC_VER <= 1200
#undef Vector
#endif
#define class_builder class_

#include "init.h"
#include "cmd/script/pythonmission.h"
#include <compile.h>
#if PY_VERSION_HEX < 0x03110000
#include <eval.h>
#endif
#include "python/python_compile.h"
#include "cmd/ai/fire.h"
#include <memory>
#include "init.h"
#include "vs_logging.h"
#include "vega_py_run.h"

#define PYTHONCALLBACK(rtype, ptr, str) \
  boost::python::call_method<rtype>(ptr, str)
#define PYTHONCALLBACK2(rtype, ptr, str, str2) \
  boost::python::call_method<rtype>(ptr, str, str2)

/*
These following #defines will create a module for python
call them with:

PYTHON_BEGIN_MODULE(VS)
       PYTHON_BEGIN_INHERIT_CLASS(VS,FireAt,"PythonFire") //begins an inherited class with a virtual Execute function...
       //You can call any other virtual functions by defining:
       //      void callFunction(std::string name){}
       //in your base class...  To use it, use:
       //      MyClass->callFunction("Execute").
       //That will do the same thing as:
       //      "MyClass->Execute()"
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

#define ADD_FROM_PYTHON_FUNCTION(SuperClass) \
BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE \
    SuperClass & from_python(PyObject *obj,boost::python::type<SuperClass &>) { \
        boost::python::detail::extension_instance* self = boost::python::detail::get_extension_instance(obj); \
        typedef std::vector<boost::python::detail::instance_holder_base*>::const_iterator iterator; \
        for (iterator p = self->wrapped_objects().begin(); \
             p != self->wrapped_objects().end(); ++p) \
        { \
            boost::python::detail::instance_holder<SuperClass>* held = dynamic_cast<boost::python::detail::instance_holder<SuperClass>*>(*p); \
            if (held != nullptr) \
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
#define PYTHON_INIT_INHERIT_GLOBALS(name,SuperClass) template <> PythonClass <SuperClass> *PythonClass< SuperClass >::last_instance = NULL; \
    ADD_FROM_PYTHON_FUNCTION(SuperClass)
#define PYTHON_INIT_GLOBALS(name,Class) ADD_FROM_PYTHON_FUNCTION(Class)
#else
#define PYTHON_INIT_INHERIT_GLOBALS(name, SuperClass) template <> PythonClass <SuperClass> *PythonClass< SuperClass >::last_instance = NULL;
#define PYTHON_INIT_GLOBALS(name, Class)
#endif
//These two functions purposely have opening/closing braces that don't match up

#define PYTHON_BEGIN_MODULE(name) BOOST_PYTHON_MODULE(name) {
#define PYTHON_DEFINE_GLOBAL(modul, fun, funname) boost::python::def (funname,fun)
#define VS_BOOST_MAKE_TUPLE(a, b, c) boost::python::make_tuple(a,b,c)
#define VS_BOOST_MAKE_TUPLE_2(a, b) boost::python::make_tuple(a,b)
#define VS_BOOST_MAKE_TUPLE_4(a, b, c, d) boost::python::make_tuple(a,b,c,d)

#if (PY_VERSION_HEX < 0x03000000)
#define PYTHON_MODULE_INIT_FUNCTION(name) init##name
#else
#define PYTHON_MODULE_INIT_FUNCTION(name) PyInit_##name
#endif

#define PYTHON_END_MODULE(name) }
#define PYTHON_INIT_MODULE(name) PYTHON_MODULE_INIT_FUNCTION(name)()

#define PYTHON_BASE_BEGIN_INHERIT_CLASS(name, NewClass, SuperClass, myclass) { \
boost::python::class_builder <SuperClass, NewClass, boost::noncopyable > Class (myclass
#define PYTHON_BEGIN_INHERIT_CLASS(name, NewClass, SuperClass, myclass) PYTHON_BASE_BEGIN_INHERIT_CLASS(name,NewClass,SuperClass,myclass) \
);

#define PYTHON_BASE_BEGIN_CLASS(name, CLASS, myclass) { \
    boost::python::class_builder <CLASS> Class (myclass
#define PYTHON_BEGIN_CLASS(name, CLASS, myclass) PYTHON_BASE_BEGIN_CLASS(name,CLASS,myclass) \
);
#define PYTHON_DEFINE_METHOD(modul, fun, funname) modul.def (funname,fun)
#define PYTHON_DEFINE_METHOD_DEFAULT(modul, fun, funname, deflt) modul.def (funname,deflt)
#define PYTHON_END_CLASS(name, SuperClass) }

template<class SuperClass>
class PythonClass : public SuperClass {
protected:
    PyObject *self;

    virtual void Destructor() {
        Py_XDECREF(self);
    }

public:
    static PythonClass<SuperClass> *last_instance;

    PythonClass(PyObject *self_) : SuperClass() {
        self = self_;
        Py_XINCREF(self);
        last_instance = this;
    }

    static PythonClass *LastPythonClass() {
        PythonClass *myclass = last_instance;
        last_instance = NULL;
        return myclass;
    }

    virtual void callFunction(std::string str) {
        PYTHONCALLBACK(void, self, str.c_str());
    }

    static PythonClass *Factory(const std::string &file) {
        CompileRunPython(file);
        return LastPythonClass();
    }

    static PythonClass *FactoryString(char *code) {
        Python::reseterrors();
        VegaPyRunString(code);
        Python::reseterrors();
        return LastPythonClass();
    }

    virtual ~PythonClass() {
        VS_LOG(warning, (boost::format("Destruct called. If called from C++ this is death %1$x") % this));
    }
};

template<class SuperClass>
class PythonAI : public PythonClass<SuperClass> {
public:
    PythonAI(PyObject *self_) : PythonClass<SuperClass>(self_) {
    }

    virtual void Execute() {
        PYTHONCALLBACK(void, this->self, "Execute");
    }

    virtual void ChooseTarget() {
        PYTHONCALLBACK(void, this->self, "ChooseTarget");
    }

    virtual void SetParent(Unit *parent) {
        SuperClass::SetParent(parent);
        PYTHONCALLBACK2(void, this->self, "init", parent);
    }

    static void default_Execute(SuperClass &self_) {
        (self_).SuperClass::Execute();
    }

    static void default_ChooseTarget(SuperClass &self_) {
        (self_).SuperClass::ChooseTarget();
    }

    static void default_SetParent(SuperClass &self_, Unit *parent) {
    }

    static PythonClass<SuperClass> *LastPythonClass() {
        return PythonClass<SuperClass>::LastPythonClass();
    }

    static PythonClass<SuperClass> *Factory(const std::string &file) {
        return PythonClass<SuperClass>::Factory(file);
    }

    static PythonClass<SuperClass> *FactoryString(char *code) {
        return PythonClass<SuperClass>::FactoryString(code);
    }
};

class pythonMission : public PythonClass<PythonMissionBaseClass> {
public:
    pythonMission(PyObject *self_) : PythonClass<PythonMissionBaseClass>(self_) {
    }

    virtual void Execute() {
        PYTHONCALLBACK(void, self, "Execute");
        Python::reseterrors();
    }

    virtual std::string Pickle() {
        Python::reseterrors();
        std::string ret = PYTHONCALLBACK(std::string, self, "Pickle");
        Python::reseterrors();
        return ret;
    }

    virtual void UnPickle(std::string s) {
        Python::reseterrors();
        PYTHONCALLBACK2(void, self, "UnPickle", s);
        Python::reseterrors();
    }

    static void default_Execute(PythonMissionBaseClass &self_) {
        (self_).PythonMissionBaseClass::Execute();
    }

    static std::string default_Pickle(PythonMissionBaseClass &self_) {
        return (self_).PythonMissionBaseClass::Pickle();
    }

    static void default_UnPickle(PythonMissionBaseClass &self_, std::string str) {
        (self_).PythonMissionBaseClass::UnPickle(str);
    }

    static PythonClass<PythonMissionBaseClass> *Factory(const std::string &file) {
        return PythonClass<PythonMissionBaseClass>::Factory(file);
    }

    static PythonClass<PythonMissionBaseClass> *FactoryString(char *code) {
        return PythonClass<PythonMissionBaseClass>::FactoryString(code);
    }
};

#endif

