#include "python_compile.h"
#include <compile.h>
#include <eval.h>
#include "cmd/unit_generic.h"
#include "configxml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "init.h"
#include "universe_util.h"
#include "in_kb_data.h"
Hashtable <string,PyCodeObject,1023> compiled_python;


char * LoadString (const char * filename) {
  FILE * fp = VSFileSystem::vs_open (filename,"r");
  if (!fp) {
    return NULL;
  }
  VSFileSystem::vs_fseek (fp,0,SEEK_END);
  long len = VSFileSystem::vs_ftell(fp);
  char * retval = NULL;
  VSFileSystem::vs_fseek (fp,0,SEEK_SET);
  if (len) {
    retval = (char *)malloc ((len+1)*sizeof (char));
    VSFileSystem::vs_read (retval,len,1,fp);
    retval[len]='\0';
  }
  VSFileSystem::vs_close (fp);
  return retval;
}
std::string getCompilingName (const std::string &name) {
   std::string compiling_name = VSFileSystem::homedir+DELIMSTR+name+"~";
   return compiling_name;
}

void InterpretPython (const std::string &name) {
  
    char * temp = strdup(getCompilingName (name).c_str());
    FILE * fp =VSFileSystem::vs_open (name.c_str(),"r");
    if (fp) {
      PyRun_SimpleFile(fp,temp);
      Python::reseterrors();
      VSFileSystem::vs_close (fp);
    }
    free (temp);
}
PyCodeObject *CompilePython (const std::string &name) {
  Python::reseterrors();
  PyCodeObject * retval = compiled_python.Get (name);
  Python::reseterrors();
  if (retval) {
    return retval;
  }
  char * str  = LoadString (name.c_str());
  if (str) {
    std::string compiling_name = getCompilingName(name).c_str();
    char * temp = strdup(compiling_name.c_str());

    retval = (PyCodeObject *) Py_CompileString (str,temp,Py_file_input);
    if (retval) {
      compiled_python.Put(name,retval);
    }
    free (temp);
    free (str);
  }
  return retval;
}


void CompileRunPython (const std::string &filename) {
  static bool ndebug_libs = XMLSupport::parse_bool(vs_config->getVariable ("AI","compile_python","true"));
//  if (ndebug_libs) {
  if (0) {
    Python::reseterrors();
    PyCodeObject *CompiledProgram = CompilePython (filename);
    Python::reseterrors();
    if (CompiledProgram) {
      PyObject *m, *d;
      if ((m = PyImport_AddModule("__main__")) != NULL)
	{
	  PyObject *localdict = PyDict_New ();
	  if ((d = PyModule_GetDict(m)) != NULL)
	    {
	      PyObject * exe=PyEval_EvalCode(CompiledProgram, d, localdict);      
	      Py_XDECREF (exe);
	      //unref exe?
	    }
	  Py_DECREF (localdict);
	}    
    } 
  }else {
    Python::reseterrors();
    InterpretPython (filename);
    Python::reseterrors();
  }


}

PyObject * CreateTuple (const std::vector <PythonBasicType> & values) {
  PyObject * retval = PyTuple_New (values.size());
  for (unsigned int i=0;i<values.size();i++) {
    PyObject *val = values[i].NewObject();
    if (val) {
      PyTuple_SET_ITEM (retval,i,val);
    }
  }
  return retval;
}
static void pySetScratchVector(const KBSTATE k) {
  switch (k) {
  case PRESS:
    UniverseUtil::setScratchVector(Vector(1,1,0));
    break;
  case RELEASE:
    UniverseUtil::setScratchVector(Vector(0,1,0));
    break;
  case UP:
    UniverseUtil::setScratchVector(Vector(0,0,0));
    break;
  case DOWN:
    UniverseUtil::setScratchVector(Vector(1,0,0));
    break;
  default:
    break;
  }
}
void RunPythonPress(const KBData&s,KBSTATE k){
  if (k==PRESS&&s.data.length()){
    pySetScratchVector(k);
    CompileRunPython(s.data);
    UniverseUtil::setScratchVector(Vector(0,0,0));
  }
}
void RunPythonRelease(const KBData&s,KBSTATE k){
  if (k==RELEASE&&s.data.length()){
    pySetScratchVector(k);
    CompileRunPython(s.data);
    UniverseUtil::setScratchVector(Vector(0,0,0));
  }
}
void RunPythonToggle(const KBData&s,KBSTATE k){
  if ((k==RELEASE||k==PRESS)&&s.data.length()){
    pySetScratchVector(k);
    CompileRunPython(s.data);
    UniverseUtil::setScratchVector(Vector(0,0,0));
  }
  
}
void RunPythonPhysicsFrame(const KBData&s,KBSTATE k){
  if ((k==DOWN||k==UP)&&s.data.length()){
    pySetScratchVector(k);
    CompileRunPython(s.data);
    UniverseUtil::setScratchVector(Vector(0,0,0));
  }  
}
