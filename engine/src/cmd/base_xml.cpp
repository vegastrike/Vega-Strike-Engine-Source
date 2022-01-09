/**
* base_xml.cpp
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
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

#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#include <boost/python/object.hpp>
#else
#include <boost/python/objects.hpp>
#endif

#include <Python.h>
#include <vector>
#include <string>
#include <math.h>
#include "python/python_class.h"
#include "base.h"
#include "base_util.h"
#include "vsfilesystem.h"


static FILE * withAndWithout( std::string filename, std::string time_of_day_hint )
{
    string with( filename+"_"+time_of_day_hint+BASE_EXTENSION );
    FILE  *fp = VSFileSystem::vs_open( with.c_str(), "r" );
    if (!fp) {
        string without( filename+BASE_EXTENSION );
        fp = VSFileSystem::vs_open( without.c_str(), "r" );
    }
    return fp;
}
static FILE * getFullFile( std::string filename, std::string time_of_day_hint, std::string faction )
{
    FILE *fp = withAndWithout( filename+"_"+faction, time_of_day_hint );
    if (!fp)
        fp = withAndWithout( filename, time_of_day_hint );
    return fp;
}
void BaseInterface::Load( const char *filename, const char *time_of_day_hint, const char *faction )
{
#if 0
    std::string full_filename     = string( "bases/" )+filename;
    std::string daynight_filename = full_filename+"_"+string( time_of_day_hint );
    full_filename     += BASE_EXTENSION;
    daynight_filename += BASE_EXTENSION;
    std::string newfile = daynight_filename;
    BOOST_LOG_TRIVIAL(trace) << "BaseInterface::LoadXML "<<full_filename<<endl;
    FILE *inFile = VSFileSystem::vs_open( daynight_filename.c_str(), "r" );
    if (!inFile) {
        newfile = full_filename;
        inFile  = VSFileSystem::vs_open( full_filename.c_str(), "r" );
    }
    if (!inFile) {
        Unit *baseun = this->baseun.GetUnit();
        if (baseun) {
            if (baseun->isUnit() == _UnitType::planet) {
                daynight_filename = string( "bases/planet_" )+time_of_day_hint+string( BASE_EXTENSION );
                inFile  = VSFileSystem::vs_open( daynight_filename.c_str(), "r" );
                newfile = daynight_filename;
                if (!inFile) {
                    newfile = "bases/planet" BASE_EXTENSION;
                    inFile  = VSFileSystem::vs_open( newfile.c_str(), "r" );
                }
            } else {
                daynight_filename = string( "bases/unit_" )+time_of_day_hint+string( BASE_EXTENSION );
                inFile  = VSFileSystem::vs_open( daynight_filename.c_str(), "r" );
                newfile = daynight_filename;
                if (!inFile) {
                    newfile = "bases/unit" BASE_EXTENSION;
                    inFile  = VSFileSystem::vs_open( newfile.c_str(), "r" );
                }
            }
        }
        if (!inFile)
            return;
    }
#else
    FILE *inFile = getFullFile( string( "bases/" )+filename, time_of_day_hint, faction );
    if (!inFile) {
        bool   planet = false;
        Unit  *baseun = this->baseun.GetUnit();
        if (baseun)
            planet = (baseun->isUnit() == _UnitType::planet);
        string basestring( "bases/unit" );
        if (planet)
            basestring = "bases/planet";
        inFile = getFullFile( basestring, time_of_day_hint, faction );
        if (!inFile)
            return;
    }
#endif
    //now that we have a FILE * named inFile and a std::string named newfile we can finally begin the python
    string compilefile = string( filename )+time_of_day_hint+string( faction )+BASE_EXTENSION;
    Python::reseterrors();
    PyRun_SimpleFile( inFile, compilefile.c_str() );
    Python::reseterrors();
    VSFileSystem::vs_close( inFile );
}

