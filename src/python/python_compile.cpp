#include "python_compile.h"

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
PyCodeObject *CompilePython (const std::string &name) {
  PyCodeObject * retval = compiled_python.Get (name);
  if (retval) {
    return retval;
  }
  char * str  = LoadString (name.c_str());
  if (str) {
    std::string compiling_name = name + "~";
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
