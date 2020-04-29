#include "config.h"
#ifdef HAVE_PYTHON
#include <boost/version.hpp>
#if defined (_MSC_VER) && _MSC_VER <= 1200
#define Vector Vactor
#endif
#include <boost/python.hpp>
#include <boost/python/converter/from_python.hpp>
#if defined (_MSC_VER) && _MSC_VER <= 1200
#undef Vector
#endif
#include "cs_python.h"
#include <pyerrors.h>
#include <pythonrun.h>
#include <compile.h>
#include <eval.h>
#include <stdio.h>
#include <math.h>

#include "configxml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "init.h"
#include "python_compile.h"
#include "python_class.h"
#include "cmd/unit_generic.h"
#if defined (_WIN32) && !defined (__CYGWIN__)
#include <direct.h>
#endif
class Unit;
//FROM_PYTHON_SMART_POINTER(Unit)

#include "vsfilesystem.h"

#ifdef _WIN32
//Python 2.5 doesn't seem to like forward-slashes.
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

void Python::initpaths()
{
    /*
     *  char pwd[2048];
     *  getcwd (pwd,2047);
     *  pwd[2047]='\0';
     *  for (int i=0;pwd[i]!='\0';i++) {
     *       if (pwd[i]=='\\')
     *               pwd[i]=DELIM;
     *  }
     */
    std::string moduledir( vs_config->getVariable( "data", "python_modules", "modules" ) );
    std::string basesdir( vs_config->getVariable( "data", "python_bases", "bases" ) );

    /*
     *  std::string changepath ("import sys\nprint sys.path\nsys.path = ["
     *                       "\""+std::string(pwd)+DELIMSTR"modules"DELIMSTR"builtin\""
     *                       ",\""+std::string(pwd)+DELIMSTR+moduledir+string("\"")+
     *                       ",\""+std::string(pwd)+DELIMSTR+basesdir + string("\"")+
     *                       "]\n");
     */
    std::string modpaths( "\"\"," );
    //Find all the mods dir (ignore homedir)
    for (unsigned int i = 1; i < VSFileSystem::Rootdir.size(); i++) {
        modpaths += "r\""+VSFileSystem::Rootdir[i]+PATHSEP+moduledir+PATHSEP "builtin\",";
        modpaths += "r\""+VSFileSystem::Rootdir[i]+PATHSEP+moduledir+PATHSEP "quests\",";
        modpaths += "r\""+VSFileSystem::Rootdir[i]+PATHSEP+moduledir+PATHSEP "missions\",";
        modpaths += "r\""+VSFileSystem::Rootdir[i]+PATHSEP+moduledir+PATHSEP "ai\",";
        modpaths += "r\""+VSFileSystem::Rootdir[i]+PATHSEP+moduledir+"\",";
        modpaths += "r\""+VSFileSystem::Rootdir[i]+PATHSEP+basesdir+"\"";
        if ( i+1 < VSFileSystem::Rootdir.size() )
            modpaths += ",";
    }
    /*
     *  string::size_type backslash;
     *  while ((backslash=modpaths.find("\\"))!=std::string::npos) {
     *  modpaths[backslash]='/';
     *  }*/
    std::string changepath( "import sys\nprint(sys.path)\nsys.path = ["+modpaths+"] + sys.path\n" );
    /*
     *  std::string changepath ("import sys\nprint sys.path\nsys.path = ["
     *                       "\""+VSFileSystem::datadir+DELIMSTR"modules"DELIMSTR"builtin\""
     *                       ",\""+VSFileSystem::datadir+DELIMSTR+moduledir+string("\"")+
     *                       ",\""+VSFileSystem::datadir+DELIMSTR+basesdir + string("\"")+
     *                       "]\n");
     */
    VSFileSystem::vs_fprintf( stderr, "running %s", changepath.c_str() );
    char *temppython = strdup( changepath.c_str() );
    PyRun_SimpleString( temppython );
    Python::reseterrors();
    free( temppython );
}

void Python::reseterrors()
{
    if ( PyErr_Occurred() ) {
        PyErr_Print();
        PyErr_Clear();
        fflush( stderr );
        fflush( stdout );
    }
#ifdef _DEBUG
    fflush( stderr );
#endif
}

#if BOOST_VERSION != 102800
static void * Vector_convertible( PyObject *p )
{
    return PyTuple_Check( p ) ? p : 0;
}


static void Vector_construct( PyObject *source, boost::python::converter::rvalue_from_python_stage1_data *data )
{
    void*const storage = ( (boost::python::converter::rvalue_from_python_storage< Vector >*)data )->storage.bytes;
    new (storage) Vector( 0, 0, 0 );
    //Fill in QVector values from source tuple here
    //details left to reader.
    Vector *vec = (Vector*) storage;
    static char fff[4] = "fff"; //added by chuck_starchaser, to kill a warning
    PyArg_ParseTuple( source, fff, &vec->i, &vec->j, &vec->k );
    data->convertible = storage;
}

static void QVector_construct( PyObject *source, boost::python::converter::rvalue_from_python_stage1_data *data )
{
    void*const storage = ( (boost::python::converter::rvalue_from_python_storage< QVector >*)data )->storage.bytes;
    new (storage) QVector( 0, 0, 0 );
    //Fill in QVector values from source tuple here
    //details left to reader.
    QVector *vec = (QVector*) storage;
    static char ddd[4] = "ddd"; //added by chuck_starchaser, to kill a warning
    PyArg_ParseTuple( source, ddd, &vec->i, &vec->j, &vec->k );
    data->convertible = storage;
}
#endif

void Python::init()
{
    static bool isinit = false;
    if (isinit)
        return;
    isinit = true;
//initialize python library
    Py_NoSiteFlag = 1;

// These functions add these modules to the builtin package
    InitVS();
    InitBriefing();
    InitBase();
    InitDirector();

// Now we can do python things about them and initialize them
    Py_Initialize();
    initpaths();

#if BOOST_VERSION != 102800
    boost::python::converter::registry::insert( Vector_convertible, QVector_construct, boost::python::type_id< QVector > () );
    boost::python::converter::registry::insert( Vector_convertible, Vector_construct, boost::python::type_id< Vector > () );
#endif
#if (PY_VERSION_HEX < 0x03000000)
    InitBriefing2();
    InitVS2();
#endif
    VSFileSystem::vs_fprintf( stderr, "testing VS random" );
    std::string changepath( "import sys\nprint(sys.path)\n" );
    VSFileSystem::vs_fprintf( stderr, "running %s", changepath.c_str() );
    char *temppython = strdup( changepath.c_str() );
    PyRun_SimpleString( temppython );
    Python::reseterrors();
    free( temppython );
#if (PY_VERSION_HEX < 0x03000000)
    InitDirector2();
    InitBase2();
#endif
}

void Python::test()
{
    /* initialize vegastrike module so that
     *  'import VS' works in python */

    /* There should be a python script automatically executed right after
     *  initVegastrike to rebind some of the objects into a better hierarchy,
     *  and to install wrappers for class methods, etc.
     *
     *  This script should look something like <<EOF
     *  import VS
     *  import sys
     *
     * #Set up output redirection
     *  sys.stdout = VS.IO()
     *  sys.stderr = sys.stdout
     *
     * #Make Var look like a nested 'class'
     *  VS.Config.Var = VS.Var()
     *
     *  EOF
     *  This should be executed with PyRun_SimpleString(char *command)
     *  See defs.py for some recipes that should go in there
     *
     *  Other useful Python functions:
     *
     *  PyObject* Py_CompileString(char *str, char *filename, int start)
     *  Return value: New reference.
     *  Parse and compile the Python source code in str, returning the resulting code object. The start token is given by start; this can be used to constrain the code which can be compiled and should be Py_eval_input, Py_file_input, or Py_single_input. The filename specified by filename is used to construct the code object and may appear in tracebacks or SyntaxError exception messages. This returns NULL if the code cannot be parsed or compiled.
     *
     *  This would be the preferred mode of operation for AI scripts
     */

#if 0 //defined(WIN32)
    FILE *fp = VSFileSystem::OpenFile( "config.py", "r" );

    freopen( "stderr", "w", stderr );
    freopen( "stdout", "w", stdout );
    changehome( true );
    FILE *fp1 = VSFileSystem::OpenFile( "config.py", "r" );
    returnfromhome();
    if (fp1 == NULL)
        fp1 = fp;
    if (fp1 != NULL) {
        PyRun_SimpleFile( fp, "config.py" );
        VSFileSystem::Close( fp1 );
    }
#endif
//char buffer[128];
//PythonIOString::buffer << endl << '\0';
//vs_config->setVariable("data","test","NULL");
//VSFileSystem::vs_fprintf(stdout, "%s", vs_config->getVariable("data","test", string()).c_str());
//VSFileSystem::vs_fprintf(stdout, "output %s\n", PythonIOString::buffer.str());
    fflush( stderr );
    fflush( stdout );
}

#endif

