#include "python_compile.h"
#include <compile.h>
#include <eval.h>
#include "config_xml.h"
#include "vs_globals.h"
#include "vs_path.h"
Hashtable <string,PyCodeObject ,char[1023]> compiled_python;


char * LoadString (const char * filename) {
  FILE * fp = fopen (filename,"r");
  if (!fp) {
    return NULL;
  }
  fseek (fp,0,SEEK_END);
  long len = ftell(fp);
  char * retval = NULL;
  fseek (fp,0,SEEK_SET);
  if (len) {
    retval = (char *)malloc ((len+1)*sizeof (char));
    fread (retval,len,1,fp);
    retval[len]='\0';
  }
  fclose (fp);
  return retval;
}
std::string getCompilingName (const std::string &name) {
   std::string compiling_name = homedir+DELIMSTR+name+"~";
   return compiling_name;
}
void InterpretPython (const std::string &name) {
  
    char * temp = strdup(getCompilingName (name).c_str());
    FILE * fp =fopen (name.c_str(),"r");
    if (fp) {
      PyRun_SimpleFile(fp,temp);
      fclose (fp);
    }
    free (temp);
}
PyCodeObject *CompilePython (const std::string &name) {
  PyCodeObject * retval = compiled_python.Get (name);
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
  if (ndebug_libs) {
    PyCodeObject *CompiledProgram = CompilePython (filename);
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
    InterpretPython (filename);
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
