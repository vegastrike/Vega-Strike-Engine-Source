#include "python_class.h"

#include <boost/version.hpp>
#if BOOST_VERSION != 102800
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
EXPORT_UTIL(getUnitByName,Unit())
EXPORT_UTIL(launchJumppoint,Unit())
EXPORT_UTIL(launch,Unit())
EXPORT_UTIL(getPlayer,Unit())
EXPORT_UTIL(getPlayerX,Unit())
EXPORT_UTIL(GetMasterPartList,Unit())
voidEXPORT_UTIL(setOwner)
EXPORT_UTIL(getOwner,Unit())
#if BOOST_VERSION != 102800
StarSystemExports();
#else
#include "star_system_exports.h"
#endif
#undef EXPORT_UTIL
#undef voidEXPORT_UTIL
#undef EXPORT_FACTION
#undef voidEXPORT_FACTION
PYTHON_BASE_BEGIN_CLASS(VS,Cargo,"Cargo")
#if BOOST_VERSION != 102800
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
PYTHON_DEFINE_METHOD(Class,&Cargo::GetContentPython,"GetContent");
PYTHON_DEFINE_METHOD(Class,&Cargo::SetCategory,"SetCategory");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetCategoryPython,"GetCategory");
PYTHON_DEFINE_METHOD(Class,&Cargo::SetMissionFlag,"SetMissionFlag");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetMissionFlag,"GetMissionFlag");
PYTHON_DEFINE_METHOD(Class,&Cargo::GetDescriptionPython,"GetDescription");
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
//#if BOOST_VERSION != 102800
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
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::advanceN,"advanceN");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::advanceSignificant,"advanceSignificant");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::advanceNSignificant,"advanceNSignificant");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::advanceInsignificant,"advanceInsignificant");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::advanceNInsignificant,"advanceNInsignificant");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::advancePlanet,"advancePlanet");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::advanceNPlanet,"advanceNPlanet");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::advanceJumppoint,"advanceJumppoint");
  PYTHON_DEFINE_METHOD(Class,&UniverseUtil::PythonUnitIter::advanceNJumppoint,"advanceNJumppoint");
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
  PYTHON_DEFINE_METHOD(Class,&FireAt::FormUpToOwner,"FormUpToOwner");
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

static std::string ParseSizeFlags(int size)
{
	static const std::pair<int,std::string> masks[] = {
		std::pair<int,std::string>(weapon_info::LIGHT," LIGHT"),
		std::pair<int,std::string>(weapon_info::MEDIUM," MEDIUM"),
		std::pair<int,std::string>(weapon_info::HEAVY," HEAVY"),
		std::pair<int,std::string>(weapon_info::CAPSHIPLIGHT," CAPSHIPLIGHT"),
		std::pair<int,std::string>(weapon_info::CAPSHIPHEAVY," CAPSHIPHEAVY"),
		std::pair<int,std::string>(weapon_info::SPECIAL," SPECIAL"),
		std::pair<int,std::string>(weapon_info::LIGHTMISSILE," LIGHTMISSILE"),
		std::pair<int,std::string>(weapon_info::MEDIUMMISSILE," MEDIUMMISSILE"),
		std::pair<int,std::string>(weapon_info::HEAVYMISSILE," HEAVYMISSILE"),
		std::pair<int,std::string>(weapon_info::CAPSHIPLIGHTMISSILE," CAPSHIPLIGHTMISSILE"),
		std::pair<int,std::string>(weapon_info::CAPSHIPHEAVYMISSILE," CAPSHIPHEAVYMISSILE"),
		std::pair<int,std::string>(weapon_info::SPECIALMISSILE," SPECIALMISSILE"),
		std::pair<int,std::string>(weapon_info::AUTOTRACKING," AUTOTRACKING")
	};
	std::string rv;
	for (int i=0; i<sizeof(masks)/sizeof(*masks); ++i)
		if (size & masks[i].first)
			rv += masks[i].second;
	if (!rv.empty())
		return rv.substr(1); else
		return std::string("NOWEAP");
}

#define PARSE_CASE(Class,Enum) case Class::Enum: return std::string(#Enum)
#define PARSE_CASE_DEFAULT default: return std::string("UNDEFINED")

static std::string ParseMountStatus(Mount::STATUS status)
{
	switch (status) {
	PARSE_CASE(Mount,ACTIVE);
	PARSE_CASE(Mount,INACTIVE);
	PARSE_CASE(Mount,DESTROYED);
	PARSE_CASE(Mount,UNCHOSEN);
	PARSE_CASE_DEFAULT;
	}
}

static std::string ParseWeaponType(weapon_info::WEAPON_TYPE type)
{
	switch (type) {
	PARSE_CASE(weapon_info,BEAM);
	PARSE_CASE(weapon_info,BALL);
	PARSE_CASE(weapon_info,BOLT);
	PARSE_CASE(weapon_info,PROJECTILE);
	PARSE_CASE_DEFAULT;
	}
}

static boost::python::dictionary GatherWeaponInfo(const weapon_info *wi)
{
	boost::python::dictionary rv;
	if (wi) {
		rv["type"] = ParseWeaponType(wi->type);
		rv["speed"] = wi->Speed;
		rv["range"] = wi->Range;
		rv["damage"] = wi->Damage;
		rv["phaseDamage"] = wi->PhaseDamage;
		rv["stability"] = wi->Stability;
		rv["longRange"] = wi->Longrange;
		rv["lockTime"] = wi->LockTime;
		rv["energyRate"] = wi->EnergyRate;
		rv["refire"] = wi->Refire();
		rv["volume"] = wi->volume;
		rv["name"] = wi->weapon_name;
	}
	return rv;
}

boost::python::dictionary UnitWrapper::GetMountInfo(int index) const
{
	boost::python::dictionary rv;
	if ((index>=0) && (index<unit->mounts.size())) {
		Mount &mnt = unit->mounts[index];

		Vector pos = mnt.GetMountLocation();
		Vector scale = Vector(mnt.xyscale,mnt.xyscale,mnt.zscale);
		Quaternion orientation = mnt.GetMountOrientation();

		rv["position"] = VS_BOOST_MAKE_TUPLE(pos.i,pos.j,pos.k);
		rv["orientation"] = VS_BOOST_MAKE_TUPLE_4(orientation.v.i,orientation.v.j,orientation.v.k,orientation.s);
		rv["scale"] = VS_BOOST_MAKE_TUPLE(scale.i,scale.j,scale.k);

		rv["empty"] = mnt.IsEmpty();
		rv["volume"] = mnt.volume;
		rv["ammo"] = mnt.ammo;
		rv["size"] = mnt.size;
		rv["size_flags"] = ParseSizeFlags(mnt.size);
		rv["bank"] = mnt.bank;
		rv["functionality"] = mnt.functionality;
		rv["maxfunctionality"] = mnt.maxfunctionality;

		rv["status"] = ParseMountStatus(mnt.status);

		if (!mnt.IsEmpty())
			rv["weapon_info"] = GatherWeaponInfo(mnt.type);
	} else {
		rv["empty"] = true;
		rv["status"] = std::string("UNDEFINED");
	}
	return rv;
}
