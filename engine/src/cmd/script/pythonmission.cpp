#include <boost/version.hpp>
#include "cs_python.h"
#include <math.h>
#include "pythonmission.h"
#include "python/python_compile.h"
#include <string>
#include "python/init.h"

#include "vsfilesystem.h"
#include "cmd/container.h"
PythonMissionBaseClass::PythonMissionBaseClass() {}
void PythonMissionBaseClass::Destructor()
{
    delete this;
}
PythonMissionBaseClass::~PythonMissionBaseClass()
{
    for (unsigned int i = 0; i < relevant_units.size(); ++i) {
        relevant_units[i]->SetUnit( NULL );
        delete relevant_units[i];
    }
    relevant_units.clear();
    VSFileSystem::vs_fprintf( stderr, "BASE Destruct called. If called from C++ this is death %ld (0x%x)",
                              (unsigned long) (size_t) this, (unsigned int) (size_t) this );
}

void PythonMissionBaseClass::Execute() {}
void PythonMissionBaseClass::callFunction( std::string ) {}

std::string PythonMissionBaseClass::Pickle()
{
    return std::string();
}

void PythonMissionBaseClass::UnPickle( std::string s ) {}

