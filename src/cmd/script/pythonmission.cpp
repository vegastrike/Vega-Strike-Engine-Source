#include <math.h>
#include "pythonmission.h"
#include <Python.h>
#include "python/python_compile.h"
#include <string>
#include "pythonmission.h"
#include "python/init.h"
#ifndef USE_BOOST_129
#include <boost/python/reference.hpp>
#include <boost/python/callback.hpp>
#endif
#include "vsfilesystem.h"
PythonMissionBaseClass::PythonMissionBaseClass () {
}
void PythonMissionBaseClass::Destructor () {
	delete this;
}
PythonMissionBaseClass::~PythonMissionBaseClass(){
	VSFileSystem::vs_fprintf (stderr,"BASE Destruct called. If called from C++ this is death %ld (0x%x)",(unsigned long)this,(unsigned int)this);
}

void PythonMissionBaseClass::Execute () {
}
void PythonMissionBaseClass::callFunction (std::string) {
}

std::string PythonMissionBaseClass::Pickle() {
	return std::string();
}

void PythonMissionBaseClass::UnPickle(std::string s)  {
}


