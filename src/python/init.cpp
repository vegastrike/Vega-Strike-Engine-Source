
#ifdef HAVE_PYTHON
#include <stdio.h>
#include <Python.h>
#include <pyerrors.h>
#include <pythonrun.h>
#include <compile.h>
#include <eval.h>
#ifdef USE_BOOST_129
#include <boost/python.hpp>
#include <boost/python/converter/from_python.hpp>
#else
#include <boost/python/class_builder.hpp>
#include <boost/python/detail/extension_class.hpp>
#endif
#include "configxml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "init.h"
#include "python_compile.h"
#include "python_class.h"
#include "cmd/unit_generic.h"
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <direct.h>
#endif
class Unit;
//FROM_PYTHON_SMART_POINTER(Unit)
#ifdef OLD_PYTHON_TEST
     class hello {
		 std::string country;
      public:
         hello(const std::string& country) { this->country = country; }
         virtual std::string greet() const { return "Hello from " + country; }
         virtual ~hello(){VSFileSystem::vs_fprintf (stderr,"NO HELLO %d",this);} // Good practice 
     };
	 struct hello_callback : hello
     {
         // hello constructor storing initial self_ parameter
         hello_callback(PyObject* self_, const std::string& x) // 2
             : hello(x), self(self_) {}

         // In case hello is returned by-value from a wrapped function
         hello_callback(PyObject* self_, const hello& x) // 3
             : hello(x), self(self_) {}

         // Override greet to call back into Python
         std::string greet() const // 4
             { return boost::python::callback<std::string>::call_method(self, "greet"); }
             virtual ~hello_callback () {VSFileSystem::vs_fprintf (stderr,"NO CALLBAC %d",this);}
         // Supplies the default implementation of greet
         static std::string default_greet(const hello& self_) //const // 5
             { return self_.hello::greet(); }
      private:
         PyObject* self; // 1
     };
class MyBaseClass {
private:
protected:
	int val;
public:
	MyBaseClass(int set) {val=set;}
	virtual void foo(int set){val=0;}
	virtual int get(){return -4364;}
	virtual ~MyBaseClass() {}
};

class MyDerivedClass : MyBaseClass {
private:
public:
	MyDerivedClass(int set) :MyBaseClass(set) {val=4364;}
	virtual void foo(int set) {val=set;}
	virtual int get(){return val;}
	virtual ~MyDerivedClass() {}
};

/* Simple Python configuration modifier. It modifies the attributes 
in the config_xml DOM; currently only works for variables in section
'data', and doesn't create new variables if necessary */

class PythonVarConfig {
public:
	struct myattr {
		std::string name;
		std::string value;
		myattr(std::string nam,std::string val) {name=nam;value=val;}
	};
	std::vector <myattr> myvar;
	std::string MyGetVariable(std::string name,int &loc) const {
		std::vector<myattr>::const_iterator iter=myvar.begin();
		std::string value="";
		if (iter) {
		while (iter!=myvar.end()) {
			if ((*iter).name==name) {
				value=(*iter).value;
				break;
			}
			iter++;
		}
		}
		return value;
	}
	void setVariable(const std::string& name,const std::string& value) {
		printf("variable %s set to %s.\n",name.c_str(),value.c_str());
		int loc;
		std::string newval=MyGetVariable(name,loc);
		if (newval.empty())
			myvar.push_back(myattr(name,value));
		else
			myvar[loc].value=value;
	}
	std::string getVariable(const std::string& name) const {
		int loc;
		std::string value=MyGetVariable(name,loc);
		if (value.empty())
			value="<UNDEFINED>";
		printf("variable %s is %s\n",name.c_str(),value.c_str());
		return value;
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

#include "cmd/ai/fire.h"
class MyFA : public CommunicatingAI {
public:
	MyFA() :CommunicatingAI(0,0){}
	virtual void Execute() {printf("CommAI\n");}
	virtual ~MyFA(){}
};

BOOST_PYTHON_MODULE_INIT(Vegastrike)
{
	/* Create a new module VS in python */
	boost::python::module_builder vs("VS");

/*	boost::python::class_builder<hello,hello_callback> BaseClass(vs, "hello");
	BaseClass.def(boost::python::constructor<std::string>());
	BaseClass.def(hello::greet,"greet",hello_callback::default_greet);
*/
/*    boost::python::class_builder<PythonVarConfig>
        Var(vs, "Var");
	//Define a constructor. To define a constructor with multiple arguments,
	//do <classbuilder_type>.def(boost::python::constructor<type1,type2,...>() 
	Var.def(boost::python::constructor<>());
	
	// Override __getattr__ and __setattr__ so that assignments to unbound variables in 
	//a Var will be redirected to the config assignment functions 
	Var.def(PythonVarConfig::getVariable,"__getattr__");
	Var.def(PythonVarConfig::setVariable,"__setattr__");

*/
	boost::python::class_builder<MyFA> FA(vs, "CommAI");
	FA.def(&MyFA::Execute,"Execute");
	FA.def(boost::python::constructor<>());
	boost::python::class_builder<PythonIOString >
		IO(vs, "IO");
	IO.def(boost::python::constructor<>());
	/* Implement a function that implements the same interface as the write file 
	I/O function in python. This is used to redirect output. A similar technique
	can be used to redirect input */
	IO.def(PythonIOString::write,"write");
}
//boost::python::detail::extension_instance::wrapped_objects

/*Orders::FireAt & from_python(PyObject *p,boost::python::type<Orders::FireAt &>) {
	return from_python(p,boost::python::type<Orders::FireAt &>());
}*/
#endif
#include "vsfilesystem.h"
void Python::initpaths(){
  /*
  char pwd[2048];
  getcwd (pwd,2047);
  pwd[2047]='\0';
  for (int i=0;pwd[i]!='\0';i++) {
	  if (pwd[i]=='\\')
		  pwd[i]=DELIM;
  }
  */
  std::string moduledir (vs_config->getVariable ("data","python_modules","modules"));
  std::string basesdir (vs_config->getVariable ("data","python_bases","bases"));

  /*
   std::string changepath ("import sys\nprint sys.path\nsys.path = ["
			  "\""+std::string(pwd)+DELIMSTR"modules"DELIMSTR"builtin\""
			  ",\""+std::string(pwd)+DELIMSTR+moduledir+string("\"")+
			  ",\""+std::string(pwd)+DELIMSTR+basesdir + string("\"")+
			  "]\n");
   */
   std::string changepath ("import sys\nprint sys.path\nsys.path = ["
			  "\""+VSFileSystem::datadir+DELIMSTR"modules"DELIMSTR"builtin\""
			  ",\""+VSFileSystem::datadir+DELIMSTR+moduledir+string("\"")+
			  ",\""+VSFileSystem::datadir+DELIMSTR+basesdir + string("\"")+
			  "]\n");
  VSFileSystem::vs_fprintf (stderr,"running %s",changepath.c_str());
  char * temppython = strdup(changepath.c_str());
  PyRun_SimpleString(temppython);	
  Python::reseterrors();
  free (temppython);
}

void Python::reseterrors() {
  if (PyErr_Occurred()) {
    PyErr_Print();
    PyErr_Clear();
    fflush(stderr);

  fflush(stdout);
  }
  fflush(stderr);
}
/*
//PYTHON_INIT_GLOBALS(VS,UnitContainer);
PYTHON_INIT_GLOBALS(VS,Unit);
PYTHON_BEGIN_MODULE(VS)
PYTHON_BASE_BEGIN_CLASS(VS,UnitContainer,"StoredUnit")
Class.def(boost::python::constructor<Unit*>());
Class.def(&UnitContainer::SetUnit,"Set");
Class.def(&UnitContainer::GetUnit,"Get");
PYTHON_END_CLASS(VS,UnitContainer)
PYTHON_BASE_BEGIN_CLASS(VS,Unit,"Unit")
Class.def(boost::python::constructor<int>());
//Class.def(&UnitContainer::SetUnit,"Set");
//Class.def(&UnitContainer::GetUnit,"Get");
PYTHON_END_CLASS(VS,Unit)
PYTHON_END_MODULE(VS)
TO_PYTHON_SMART_POINTER(Unit) 
*/
#ifdef USE_BOOST_129
static void* Vector_convertible(PyObject* p) {
	return PyTuple_Check(p)?p:0;
}

static void Vector_construct(PyObject* source, boost::python::converter::rvalue_from_python_stage1_data* data) {
	void* const storage = ((boost::python::converter::rvalue_from_python_storage<Vector>*)data)->storage.bytes;
	new (storage) Vector(0,0,0);
	// Fill in QVector values from source tuple here
	// details left to reader.
	Vector * vec = (Vector *) storage;
	PyArg_ParseTuple(source,"fff",&vec->i,&vec->j,&vec->k);
	data->convertible = storage;
}

static void QVector_construct(PyObject* source, boost::python::converter::rvalue_from_python_stage1_data* data) {
	void* const storage = ((boost::python::converter::rvalue_from_python_storage<QVector>*)data)->storage.bytes;
	new (storage) QVector(0,0,0);
	// Fill in QVector values from source tuple here
	// details left to reader.
	QVector * vec = (QVector *) storage;
	PyArg_ParseTuple(source,"ddd",&vec->i,&vec->j,&vec->k);
	data->convertible = storage;
}
#endif
void Python::init() {

  static bool isinit=false;
  if (isinit) {
    return;
  }
  isinit=true;
// initialize python library
  Py_Initialize();
  initpaths();
#ifdef USE_BOOST_129
  boost::python::converter::registry::insert(Vector_convertible, QVector_construct, boost::python::type_id<QVector>());
	boost::python::converter::registry::insert(Vector_convertible, Vector_construct, boost::python::type_id<Vector>());
#endif
	InitBriefing ();
	InitVS ();
	VSFileSystem::vs_fprintf (stderr,"testing VS random");
	std::string changepath ("import sys\nprint sys.path\n");
	VSFileSystem::vs_fprintf (stderr,"running %s",changepath.c_str());
	char * temppython = strdup(changepath.c_str());
	PyRun_SimpleString(temppython);	
	Python::reseterrors();
	free (temppython);
	InitDirector ();
	InitBase ();
//  InitVegastrike();
}
void Python::test() {

	/* initialize vegastrike module so that 
	'import VS' works in python */

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

#if 0//defined(WIN32)
	FILE *fp = VSFileSystem::OpenFile("config.py","r");

	freopen("stderr","w",stderr);
	freopen("stdout","w",stdout);
	changehome(true);
	FILE *fp1 = VSFileSystem::OpenFile("config.py","r");
	returnfromhome();
	if (fp1==NULL) {
	  fp1=fp;
	}
	if(fp1!=NULL) {
		PyRun_SimpleFile(fp, "config.py");
		VSFileSystem::Close(fp1);
	}
#endif
#ifdef OLD_PYTHON_TEST
	//CompileRunPython ("simple_test.py");
    //		PyObject * arglist = CreateTuple (vector <PythonBasicType> ());
    //		PyObject * res = PyEval_CallObject(po, arglist);
    //		Py_DECREF(arglist);
		//		Py_XDECREF(res);


		PyRun_SimpleString(
	   "import VS\n"
	   "import sys\n"
	   "sys.stderr.write('asdf')\n"
//	   "VSConfig=VS.Var()\n"
//	   "VSConfig.test='hi'\n"
//	   "print VSConfig.test\n"
//	   "print VSConfig.undefinedvar\n"
//	   "VSConfig.__setattr__('undefined','An undefined variable')\n"
//	   "print VSConfig.__getattr__('undefined')\n"
	   "class MyAI(VS.CommAI):\n"
	   "   def Execute(self):\n"
	   "      sys.stdout.write('MyAI\\n')\n"
	   "hi2 = MyAI()\n"
	   "hi1 = VS.CommAI()\n"
	   "print hi1.Execute()\n"
	   "print hi2.Execute()\n"
	);
#endif
//	char buffer[128];
//	PythonIOString::buffer << endl << '\0';
//	vs_config->setVariable("data","test","NULL");
//	VSFileSystem::vs_fprintf(stdout, "%s", vs_config->getVariable("data","test", string()).c_str());
//	VSFileSystem::vs_fprintf(stdout, "output %s\n", PythonIOString::buffer.str());
	fflush(stderr);
	fflush(stdout);
}

#endif

