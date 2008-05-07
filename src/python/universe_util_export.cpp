#include "python_class.h"
#include <boost/version.hpp>
/* ToDo: Fix so Boost 1.28 is supported */
#if BOOST_VERSION != 102800
#if BOOST_VERSION != 102800
#include <boost/python.hpp>
#else
#include <boost/python/objects.hpp>
#endif

#include "cmd/container.h"
#include "init.h"
#include "gfx/vec.h"
#include "cmd/unit_generic.h"
#include "universe_util.h"
#include "cmd/unit_util.h"
#include "faction_generic.h"
#include "cmd/ai/fire.h"
void StarSystemExports () {
#define EXPORT_UTIL(name,aff) PYTHON_DEFINE_GLOBAL(VS,&UniverseUtil::name,#name);
#define voidEXPORT_UTIL(name) EXPORT_UTIL(name,0)
#undef EXPORT_FACTION
#undef voidEXPORT_FACTION
#define EXPORT_FACTION(name,aff) PYTHON_DEFINE_GLOBAL(VS,&FactionUtil::name,#name);
#define voidEXPORT_FACTION(name) EXPORT_FACTION(name,0)

	#include "star_system_exports.h"

}
#include <string>
#endif
