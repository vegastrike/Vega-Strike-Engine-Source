/**
* unit_exports.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

#include "python_class.h"

#include "cmd/container.h"
#include <string>
#include "init.h"
#include "gfx/vec.h"
#include "cmd/unit_generic.h"
#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#include <boost/python.hpp>
typedef boost::python::dict       BoostPythonDictionary;
#else
#include <boost/python/objects.hpp>
typedef boost::python::dictionary BoostPythonDictionary;
#endif
#include "universe_util.h"
#include "cmd/unit_util.h"
#include "faction_generic.h"
#include "cmd/ai/fire.h"

#include "unit_wrapper_class.h"
#include "unit_from_to_python.h"
//void ExportUnit (boost::python::class_builder <UnitWrapper> &Class) {
//ExportUnit1(Class);
//ExportUnit2(Class);
//ExportUnit3(Class);
//}

void EXP_UN( boost::python::class_builder< UnitWrapper > &Class )
{
#undef CHECKME
#undef WRAPPED0
#undef WRAPPED1
#undef WRAPPED2
#undef WRAPPED3
#undef WRAPPED4
#undef voidWRAPPED0
#undef voidWRAPPED1
#undef voidWRAPPED2
#undef voidWRAPPED3
#undef voidWRAPPED5
#undef EXPORT_UTIL0
#undef EXPORT_UTIL1
#undef EXPORT_UTIL2
#undef EXPORT_UTIL3
#undef EXPORT_UTIL5
#undef voidEXPORT_UTIL0
#undef voidEXPORT_UTIL1
#undef voidEXPORT_UTIL2
#undef voidEXPORT_UTIL3
#undef voidEXPORT_UTIL5
#define WRAPPED0( type, name, nada ) PYTHON_DEFINE_METHOD( Class, &UnitWrapper::name, #name );
#define WRAPPED1( type, name, atype, a, def ) WRAPPED0( type, name, def )
#define WRAPPED2( type, name, atype, a, btype, b, def ) WRAPPED0( type, name, def )
#define WRAPPED3( type, name, atype, a, btype, b, ctype, c, def ) WRAPPED0( type, name, def )
#define WRAPPED4( type, name, atype, a, btype, b, ctype, c, dtype, d, def ) WRAPPED0( type, name, def )
#define voidWRAPPED0( name ) WRAPPED0( void, name, 0 )
#define voidWRAPPED1( name, atype, a ) WRAPPED0( void, name, 0 )
#define voidWRAPPED2( name, atype, a, btype, b ) WRAPPED0( void, name, 0 )
#define voidWRAPPED3( name, atype, a, btype, b, ctype, c ) WRAPPED0( void, name, 0 )
#define voidWRAPPED5( name, atype, a, btype, b, ctype, c, dtype, d, etype, e ) WRAPPED0( void, name, 0 )
#define EXPORT_UTIL0( type, name ) WRAPPED0( type, name, 0 )
#define EXPORT_UTIL1( type, name, atype, a ) WRAPPED0( type, name, 0 )
#define EXPORT_UTIL2( type, name, atype, a, btype, b ) WRAPPED0( type, name, 0 )
#define EXPORT_UTIL3( type, name, atype, a, btype, b, ctype, c ) WRAPPED0( type, name, 0 )
#define EXPORT_UTIL5( type, name, atype, a, btype, b, ctype, c, dtype, d, etype, e ) WRAPPED0( type, name, 0 )
#define voidEXPORT_UTIL0( name ) WRAPPED0( void, name, 0 )
#define voidEXPORT_UTIL1( name, atype, a ) WRAPPED0( void, name, 0 )
#define voidEXPORT_UTIL2( name, atype, a, btype, b ) WRAPPED0( void, name, 0 )
#define voidEXPORT_UTIL3( name, atype, a, btype, b, ctype, c ) WRAPPED0( void, name, 0 )
#define voidEXPORT_UTIL5( name, atype, a, btype, b, ctype, c, dtype, d, etype, e ) WRAPPED0( void, name, 0 )
#include "python_unit_wrap.h"
#undef WRAPPED0
#undef WRAPPED1
#undef WRAPPED2
#undef WRAPPED3
#undef voidWRAPPED0
#undef voidWRAPPED1
#undef voidWRAPPED2
#undef voidWRAPPED3
#undef voidWRAPPED5
#undef EXPORT_UTIL0
#undef EXPORT_UTIL1
#undef EXPORT_UTIL2
#undef EXPORT_UTIL3
#undef EXPORT_UTIL5
#undef voidEXPORT_UTIL0
#undef voidEXPORT_UTIL1
#undef voidEXPORT_UTIL2
#undef voidEXPORT_UTIL3
}

