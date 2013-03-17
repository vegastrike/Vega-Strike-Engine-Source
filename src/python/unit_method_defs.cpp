#if _MSC_VER <= 1200
#include "config.h"
#include <boost/version.hpp>
#include <boost/python.hpp>
typedef boost::python::dict       BoostPythonDictionary;

#include "cmd/container.h"
#include <string>
#include "init.h"
#include "gfx/vec.h"
#include "cmd/unit_generic.h"
#include "universe_util.h"
#include "cmd/unit_util.h"
#include "faction_generic.h"
#include "cmd/ai/fire.h"
#include "python_class.h"
#include "unit_wrapper_class.h"
#include "unit_from_to_python.h"

#include "define_odd_unit_functions.h"
#endif

