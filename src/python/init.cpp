#ifdef HAVE_BOOST
#include <stdio.h>
#include <Python.h>
#include "init.h"
#include <boost/python/class_builder.hpp>
#include "config_xml.h"
#include "vs_globals.h"
#include "vs_path.h"
#include <strstream>

class PythonConfig {
	void foo();
};

enum Sections {
	NONE=0,
	DATA=1
};
const std::string Sections[] = {"","data"};
const std::string SubSections[] = {"","data"};

/* Simple Python configuration modifier. It modifies the attributes 
in the config_xml DOM; currently only works for variables in section
'data', and doesn't create new variables if necessary */

template <enum Sections SecNum, enum Sections SubSecNum=NONE>
class PythonVarConfig {
public:
	static void setVariable(PythonVarConfig &self,const std::string& name,const std::string& value) {
		std::string Section = Sections[SecNum];
		std::string SubSection = Sections[SubSecNum];
		if(SubSection.length()) {
			vs_config->setVariable(Section,SubSection,name,value);
		}
		else {
			vs_config->setVariable(Section,name,value);
		}
	}
	static std::string getVariable(PythonVarConfig &self,const std::string& name) {
		std::string Section = Sections[SecNum];
		std::string SubSection = Sections[SubSecNum];

		std::string rval;
		if(SubSection.length()) {
			rval = vs_config->getVariable(Section,SubSection,name,string("NOVALUE"));
		}
		else {
			rval = vs_config->getVariable(Section,name,string("NOVALUE"));
		}
		return rval;
	}
};

class PythonIOString {
public:
	static std::strstream buffer;
	static void write(PythonIOString &self, string data) {
		buffer << data;
	}
};
std::strstream PythonIOString::buffer;

/* Basic mode of operation:
  Define a module_builder per module and class_builders for each 
  class to be exported to python

  <module_builder|class_builder>.def defines a function in the module or class
  <module_builder|class_builder>.add adds a variable to the module or class 

From the boost documentation (I haven't used this yet)

Direct Access to Data Members 
Boost.Python uses the special __xxxattr__<name>__ functionality described above to allow direct access to data members through the following special functions on class_builder<> and extension_class<>: 

def_getter(pointer-to-member, name) // read access to the member via attribute name 
def_setter(pointer-to-member, name) // write access to the member via attribute name 
def_readonly(pointer-to-member, name) // read-only access to the member via attribute name 
def_read_write(pointer-to-member, name) // read/write access to the member via attribute name 

  ( Pointer to member is &Class::member )

  */

BOOST_PYTHON_MODULE_INIT(Vegastrike)
{
	/* Create a new module VS in python */
    boost::python::module_builder vs("VS");

	/* Map class PythonConfig to python class "Config"
	*/
    boost::python::class_builder<PythonConfig>
        Config(vs, "Config");

	/* Map class PythonVarConfig<DATA> to python class "Var"
	*/
    boost::python::class_builder<PythonVarConfig<DATA> >
        Var(vs, "Var");
	/* Define a constructor. To define a constructor with multiple arguments,
	do <classbuilder_type>.def(boost::python::constructor<type1,type2,...>() */
	Var.def(boost::python::constructor<>());
	
	/* Override __getattr__ and __setattr__ so that assignments to unbound variables in 
	a Var will be redirected to the config assignment functions */
	Var.def(PythonVarConfig<DATA>::getVariable,"__getattr__");
	Var.def(PythonVarConfig<DATA>::setVariable,"__setattr__");

	/* ... */
	boost::python::class_builder<PythonIOString >
		IO(vs, "IO");
	IO.def(boost::python::constructor<>());
	/* Implement a function that implements the same interface as the write file 
	I/O function in python. This is used to redirect output. A similar technique
	can be used to redirect input */
	IO.def(PythonIOString::write,"write");
}

void Python::init() {
	/* initialize python library */
	Py_Initialize();

	/* initialize vegastrike module so that 
	'import VS' works in python */
	initVegastrike();
	/* There should be a python script automatically executed right after 
	   initVegastrike to rebind some of the objects into a better hierarchy,
	   and to install wrappers for class methods, etc.

This script should look something like <<EOF
import VS
import sys

#Set up output redirection
sys.stdout = VS.IO()
sys.stderr = sys.stdout

#Make Var look like a nested 'class'
VS.Config.Var = VS.Var()

EOF
  This should be executed with PyRun_SimpleString(char *command)
  See defs.py for some recipes that should go in there

  Other useful Python functions:

PyObject* Py_CompileString(char *str, char *filename, int start) 
   Return value: New reference. 
   Parse and compile the Python source code in str, returning the resulting code object. The start token is given by start; this can be used to constrain the code which can be compiled and should be Py_eval_input, Py_file_input, or Py_single_input. The filename specified by filename is used to construct the code object and may appear in tracebacks or SyntaxError exception messages. This returns NULL if the code cannot be parsed or compiled. 

  This would be the preferred mode of operation for AI scripts
*/

	FILE *fp = fopen("config.py","r");

#ifdef WIN32
	freopen("stderr","w",stderr);
	freopen("stdout","w",stdout);
#endif
	changehome(true);
	FILE *fp1 = fopen("config.py","r");
	returnfromhome();
	if(fp1!=NULL) {
		PyRun_SimpleFile(fp, "config.py");
		fclose(fp1);
	}
	else if(fp!=NULL) {
	  /*PyRun_SimpleFile(fp1, "config.py");*/
	  PyRun_SimpleString(
"import VS\n"
"import sys\n"
"sys.stderr.write('asdf')\n"
);
		fclose(fp);
	}
	char buffer[128];
	PythonIOString::buffer << endl << '\0';
	fprintf(stdout, "%s", vs_config->getVariable("data","test", string()).c_str());
	fprintf(stdout, "output %s\n", PythonIOString::buffer.str());
	fflush(stderr);
	fflush(stdout);
}

#endif
