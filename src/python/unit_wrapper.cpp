#include "cmd/container.h"
#include <string>
#include "init.h"
#include "gfx/vec.h"
#include "cmd/unit_generic.h"
#include "python_class.h"
#ifndef USE_BOOST_128
#include <boost/python.hpp>
#else
#include <boost/python/objects.hpp>
#endif
#include "universe_util.h"
#include "cmd/unit_util.h"
#include "faction_generic.h"
#include "cmd/ai/fire.h"
extern void AUDStopAllSounds();
extern void StarSystemExports();

//makes to_python for both vector and qvector turn them into tuples :-)
using namespace UnitUtil;
using namespace UniverseUtil;
using Orders::FireAt;


#include "unit_wrapper_class.h"
extern void DefineOddUnitFunctions (boost::python::class_builder <UnitWrapper> &Class);
extern void ExportUnit (boost::python::class_builder <UnitWrapper> &Class);
#include "unit_from_to_python.h"
#if _MSC_VER <=1200
#else
#include "define_odd_unit_functions.h"
#endif

namespace UniverseUtil {

	UnitWrapper newGetPlayer() {
		return UniverseUtil::getPlayer();
	}
	void setOwner(int obj,UnitWrapper un) {
		setOwnerII(obj,un.GetUnit());
	}
}
PYTHON_INIT_INHERIT_GLOBALS(VS,FireAt);
PYTHON_BEGIN_MODULE(VS)
#define EXPORT_GLOBAL(name,aff) PYTHON_DEFINE_GLOBAL(VS,&name,#name);
#define voidEXPORT_GLOBAL(name) EXPORT_GLOBAL(name,0)

//#undef EXPORT_UTIL
//#undef voidEXPORT_UTIL
#define EXPORT_UTIL(name,aff) PYTHON_DEFINE_GLOBAL(VS,&UniverseUtil::name,#name);
#define voidEXPORT_UTIL(name) EXPORT_UTIL(name,0)
//#undef EXPORT_FACTION
//#undef voidEXPORT_FACTION
#define EXPORT_FACTION(name,aff) PYTHON_DEFINE_GLOBAL(VS,&FactionUtil::name,#name);
#define voidEXPORT_FACTION(name) EXPORT_FACTION(name,0)
EXPORT_UTIL(getUnitList,un_iter())
EXPORT_UTIL(getScratchUnit,Unit())
voidEXPORT_UTIL(setScratchUnit)
EXPORT_UTIL(getNumPlayers,1)
EXPORT_UTIL(getVariable,"")
EXPORT_UTIL(getSubVariable,"")
EXPORT_UTIL(newGetPlayer,Unit())
EXPORT_UTIL(GetContrabandList,Unit())
EXPORT_UTIL(getUnit,Unit())
EXPORT_UTIL(launchJumppoint,Unit())
EXPORT_UTIL(launch,Unit())
EXPORT_UTIL(getPlayer,Unit())
EXPORT_UTIL(getPlayerX,Unit())
EXPORT_UTIL(GetMasterPartList,Unit())
voidEXPORT_UTIL(setOwner)
EXPORT_UTIL(getOwner,Unit())
#ifndef USE_BOOST_128
StarSystemExports();
#else
#include "star_system_exports.h"
#endif
#undef EXPORT_UTIL
#undef voidEXPORT_UTIL
#undef EXPORT_FACTION
#undef voidEXPORT_FACTION
PYTHON_BASE_BEGIN_CLASS(VS,Cargo,"Cargo")
#ifndef USE_BOOST_128
, boost::python::init<std::string,std::string,float,int,float,float>());
#else
Class.def(boost::python::constructor<std::string,std::string,float,int,float,float>());
#endif
PYTHON_DEFINE_METHOD(Class,&Cargo::SetPrice,"SetPrice");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetPrice,"GetPrice");
PYTHON_DEFINE_METHOD(Class,&Cargo::SetMass,"SetMass");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetMass,"GetMass");
PYTHON_DEFINE_METHOD(Class,&Cargo::SetVolume,"SetVolume");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetVolume,"GetVolume");
PYTHON_DEFINE_METHOD(Class,&Cargo::SetQuantity,"SetQuantity");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetQuantity,"GetQuantity");
PYTHON_DEFINE_METHOD(Class,&Cargo::SetContent,"SetContent");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetContent,"GetContent");
PYTHON_DEFINE_METHOD(Class,&Cargo::SetCategory,"SetCategory");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetCategory,"GetCategory");
PYTHON_DEFINE_METHOD(Class,&Cargo::SetMissionFlag,"SetMissionFlag");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetMissionFlag,"GetMissionFlag");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetDescription,"GetDescription");
PYTHON_DEFINE_METHOD(Class,&Cargo::SetFunctionality,"SetFunctionality");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetFunctionality,"GetFunctionality");
PYTHON_DEFINE_METHOD(Class,&Cargo::SetMaxFunctionality,"SetMaxFunctionality");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetMaxFunctionality,"GetMaxFunctionality");



PYTHON_END_CLASS(VS,Cargo)
PYTHON_BEGIN_CLASS(VS,UnitWrapper,"Unit")
PYTHON_DEFINE_METHOD(Class,&UnitWrapper::getSubUnits,"getSubUnits");
ExportUnit (Class);
//WARNING: Macro City 2 ahead.  Please skip this section, also if you don't like macros.


//End of Macro City 2
//#ifndef USE_BOOST_128
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::equal,"__eq__");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::notequal,"__ne__");
//#else
//  Class.def (boost::python::operators< (boost::python::op_eq | boost::python::op_ne) >(), boost::python::right_operand<UnitWrapper>());
//#endif
  DefineOddUnitFunctions(Class);
PYTHON_END_CLASS(VS,UnitWrapper)

PYTHON_BEGIN_CLASS(VS,UniverseUtil::PythonUnitIter,"un_iter")
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::current,"current");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::isDone,"isDone");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::notDone,"notDone");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::advance,"advance");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::next,"next");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::remove,"remove");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::preinsert,"preinsert");
PYTHON_END_CLASS(VS,UniverseUtil::PythonUnitIter)
typedef PythonAI<FireAt> PythonAIFireAt;
PYTHON_BEGIN_INHERIT_CLASS(VS,PythonAIFireAt ,FireAt,"PythonAI")
  PYTHON_DEFINE_METHOD_DEFAULT(Class,&FireAt::Execute,"Execute",PythonAI< FireAt >::default_Execute);
  PYTHON_DEFINE_METHOD_DEFAULT(Class,&FireAt::ChooseTarget,"ChooseTarget",PythonAI< FireAt >::default_ChooseTarget);
  PYTHON_DEFINE_METHOD_DEFAULT(Class,&FireAt::SetParent,"init",PythonAI< FireAt >::default_SetParent);
  PYTHON_DEFINE_METHOD(Class,&FireAt::GetParent,"GetParent");
  PYTHON_DEFINE_METHOD(Class,&FireAt::AddReplaceLastOrder,"AddReplaceLastOrder");
  PYTHON_DEFINE_METHOD(Class,&FireAt::ExecuteLastScriptFor,"ExecuteLastScriptFor");
  PYTHON_DEFINE_METHOD(Class,&FireAt::FaceTarget,"FaceTarget");
  PYTHON_DEFINE_METHOD(Class,&FireAt::FaceTargetITTS,"FaceTargetITTS");
  PYTHON_DEFINE_METHOD(Class,&FireAt::MatchLinearVelocity,"MatchLinearVelocity");
  PYTHON_DEFINE_METHOD(Class,&FireAt::MatchAngularVelocity,"MatchAngularVelocity");
  PYTHON_DEFINE_METHOD(Class,&FireAt::ChangeHeading,"ChangeHeading");
  PYTHON_DEFINE_METHOD(Class,&FireAt::ChangeLocalDirection,"ChangeLocalDirection");
  PYTHON_DEFINE_METHOD(Class,&FireAt::MoveTo,"MoveTo");
  PYTHON_DEFINE_METHOD(Class,&FireAt::MatchVelocity,"MatchVelocity");
  PYTHON_DEFINE_METHOD(Class,&FireAt::Cloak,"Cloak");
  PYTHON_DEFINE_METHOD(Class,&FireAt::FormUp,"FormUp");
  PYTHON_DEFINE_METHOD(Class,&FireAt::FaceDirection,"FaceDirection");
  PYTHON_DEFINE_METHOD(Class,&FireAt::XMLScript,"XMLScript");
  PYTHON_DEFINE_METHOD(Class,&FireAt::LastPythonScript,"LastPythonScript");
PYTHON_END_CLASS(VS,FireAt)


PYTHON_END_MODULE(VS)

void InitVS() {
	Python::reseterrors();
	PYTHON_INIT_MODULE(VS);
	Python::reseterrors();
}

