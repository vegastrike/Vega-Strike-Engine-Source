#include "python_class.h"

#include "cmd/container.h"
#include <string>
#include "init.h"
#include "gfx/vec.h"
#include "cmd/unit_generic.h"
#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#include <boost/python.hpp>
typedef boost::python::dict BoostPythonDictionary ;
#else
#include <boost/python/objects.hpp>
typedef boost::python::dictionary BoostPythonDictionary ;
#endif

#include "universe_util.h"
#include "cmd/unit_util.h"
#include "faction_generic.h"
#include "cmd/ai/fire.h"


#include "unit_wrapper_class.h"
#include "unit_from_to_python.h"
extern void ExportUnitFIRST (class boost::python::class_builder <class UnitWrapper> &Class);
extern void ExportUnitSECOND (boost::python::class_builder <UnitWrapper> &Class);
extern void ExportUnitTHIRD (boost::python::class_builder <UnitWrapper> &Class);


void ExportUnit (boost::python::class_builder <UnitWrapper> &Class) {
	ExportUnitFIRST (Class);
	ExportUnitSECOND (Class);
	ExportUnitTHIRD (Class);

}
