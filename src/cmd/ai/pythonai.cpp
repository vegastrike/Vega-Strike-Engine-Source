#include <stdio.h>

#include <boost/python/class_builder.hpp>
#include <boost/python/detail/extension_class.hpp>
#include "python/python_compile.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vs_path.h"
#include "pythonai.h"
using namespace Orders;

PythonAI::PythonAI(PyObject * self_, BasicPointer <Order *> whoami,float reaction_time, float aggressivity):FireAt (reaction_time,aggressivity) {
  self = self_;
  // boost::python:
  Py_XINCREF(self);//by passing this to whoami, we are counting on them to Destruct us
  (*whoami)=this;
}
void PythonAI::Destruct() {
  Py_XDECREF(self);//this should destroy SELF
}
void PythonAI::default_Execute (FireAt &self_) {
  (self_).FireAt::Execute();
}

Order * PythonAI::Factory (const std::string &filename) {
  PyObject *CompiledProgram = CompilePython (filename);
  Order * myai=NULL;
  if (CompiledProgram) {
    
  } 
  return myai;
}
void PythonAI::Execute() {
  boost::python::callback <void>::call_method (self,"Execute");
}
void PythonAI::InitModuleAI () {
  boost::python::module_builder ai_builder("AI");
  boost::python::class_builder <BasicPointer<Order *> > OrderPointerClass (ai_builder,"OrderPointer");
  boost::python::class_builder <FireAt,PythonAI> BaseClass (ai_builder,"FireAt");

  BaseClass.def (boost::python::constructor<BasicPointer<Order *>, float,float>());
  BaseClass.def (&FireAt::Execute,"PythonAI",PythonAI::default_Execute);
  
}
PythonAI::~PythonAI () {
  fprintf (stderr,"Destruct called. If called from C++ this is death %d",this);
}

