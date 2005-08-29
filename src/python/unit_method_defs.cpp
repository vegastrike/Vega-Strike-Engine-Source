#if _MSC_VER <=1200
#include "python_class.h"
#ifndef USE_BOOST_128
#include <boost/python.hpp>
#else
#include <boost/python/objects.hpp>
#endif

#include "cmd/container.h"
#include <string>
#include "init.h"
#include "gfx/vec.h"
#include "cmd/unit_generic.h"
#include "universe_util.h"
#include "cmd/unit_util.h"
#include "faction_generic.h"
#include "cmd/ai/fire.h"


#include "unit_wrapper_class.h"
#include "unit_from_to_python.h"

#include "define_odd_unit_functions.h"
#endif
