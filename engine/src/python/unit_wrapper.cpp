/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
#include <boost/python.hpp>
typedef boost::python::dict BoostPythonDictionary;

#include "python_class.h"
#include "cmd/container.h"
#include <string>
#include "init.h"
#include "gfx/vec.h"
#include "cmd/unit_generic.h"
#include "universe_util.h"
#include "cmd/unit_util.h"
#include "faction_generic.h"
#include "cmd/ai/fire.h"
#include "cmd/engineVersion.h"
#include "unit_wrapper_class.h"
#include "unit_from_to_python.h"
#include "weapon_info.h"
#if _MSC_VER <= 1200
#else
#include "define_odd_unit_functions.h"
#endif

extern void StarSystemExports();

//makes to_python for both vector and qvector turn them into tuples :-)
using namespace UnitUtil;
using namespace UniverseUtil;
using Orders::FireAt;

extern void DefineOddUnitFunctions(boost::python::class_builder<UnitWrapper> &Class);
extern void ExportUnit(boost::python::class_builder<UnitWrapper> &Class);

namespace UniverseUtil {
UnitWrapper newGetPlayer()
{
    return UniverseUtil::getPlayer();
}

void setOwner(int obj, UnitWrapper un)
{
    setOwnerII(obj, un.GetUnit());
}
}
PYTHON_INIT_INHERIT_GLOBALS(VS, FireAt);
PYTHON_INIT_INHERIT_GLOBALS(VS, VegaStrike::EngineVersionData);
PYTHON_BEGIN_MODULE(VS)

//#undef EXPORT_UTIL
//#undef voidEXPORT_UTIL
#define EXPORT_UTIL(name, aff) PYTHON_DEFINE_GLOBAL( VS, &UniverseUtil::name, #name );
#define voidEXPORT_UTIL(name) EXPORT_UTIL( name, 0 )
//#undef EXPORT_FACTION
//#undef voidEXPORT_FACTION
#define EXPORT_FACTION(name, aff) PYTHON_DEFINE_GLOBAL( VS, &FactionUtil::name, #name );
#define voidEXPORT_FACTION(name) EXPORT_FACTION( name, 0 )
    EXPORT_UTIL(getUnitList, PythonUnitIter())
    EXPORT_UTIL(getScratchUnit, Unit())
    voidEXPORT_UTIL(setScratchUnit)
    EXPORT_UTIL(getNumPlayers, 1)
    EXPORT_UTIL(getVariable, "")
    EXPORT_UTIL(getSubVariable, "")
    EXPORT_UTIL(newGetPlayer, Unit())
    EXPORT_UTIL(GetContrabandList, Unit())
    EXPORT_UTIL(getUnit, Unit())
    EXPORT_UTIL(getUnitByName, Unit())
    EXPORT_UTIL(launchJumppoint, Unit())
    EXPORT_UTIL(launch, Unit())
    EXPORT_UTIL(getPlayer, Unit())
    EXPORT_UTIL(getPlayerX, Unit())
    EXPORT_UTIL(GetMasterPartList, Unit())
    voidEXPORT_UTIL(setOwner)
    EXPORT_UTIL(getOwner, Unit())
    StarSystemExports();
#undef EXPORT_UTIL
#undef voidEXPORT_UTIL
#undef EXPORT_FACTION
#undef voidEXPORT_FACTION
    PYTHON_BASE_BEGIN_CLASS(VS, Cargo, "Cargo")
                , boost::python::init<std::string, std::string, float, int, float, float>());
        PYTHON_DEFINE_METHOD(Class, &Cargo::SetPrice, "SetPrice");
        PYTHON_DEFINE_METHOD(Class, &Cargo::GetPrice, "GetPrice");
        PYTHON_DEFINE_METHOD(Class, &Cargo::SetMass, "SetMass");
        PYTHON_DEFINE_METHOD(Class, &Cargo::GetMass, "GetMass");
        PYTHON_DEFINE_METHOD(Class, &Cargo::SetVolume, "SetVolume");
        PYTHON_DEFINE_METHOD(Class, &Cargo::GetVolume, "GetVolume");
        PYTHON_DEFINE_METHOD(Class, &Cargo::SetQuantity, "SetQuantity");
        PYTHON_DEFINE_METHOD(Class, &Cargo::GetQuantity, "GetQuantity");
        PYTHON_DEFINE_METHOD(Class, &Cargo::SetContent, "SetContent");
        PYTHON_DEFINE_METHOD(Class, &Cargo::GetContentPython, "GetContent");
        PYTHON_DEFINE_METHOD(Class, &Cargo::SetCategory, "SetCategory");
        PYTHON_DEFINE_METHOD(Class, &Cargo::GetCategoryPython, "GetCategory");
        PYTHON_DEFINE_METHOD(Class, &Cargo::SetMissionFlag, "SetMissionFlag");
        PYTHON_DEFINE_METHOD(Class, &Cargo::GetMissionFlag, "GetMissionFlag");
        PYTHON_DEFINE_METHOD(Class, &Cargo::GetDescriptionPython, "GetDescription");
        PYTHON_DEFINE_METHOD(Class, &Cargo::SetFunctionality, "SetFunctionality");
        PYTHON_DEFINE_METHOD(Class, &Cargo::GetFunctionality, "GetFunctionality");
        PYTHON_DEFINE_METHOD(Class, &Cargo::SetMaxFunctionality, "SetMaxFunctionality");
        PYTHON_DEFINE_METHOD(Class, &Cargo::GetMaxFunctionality, "GetMaxFunctionality");

    PYTHON_END_CLASS(VS, Cargo)
    PYTHON_BEGIN_CLASS(VS, UnitWrapper, "Unit")
        PYTHON_DEFINE_METHOD(Class, &UnitWrapper::getSubUnits, "getSubUnits");
        ExportUnit(Class);
//WARNING: Macro City 2 ahead.  Please skip this section, also if you don't like macros.

//End of Macro City 2
//#if BOOST_VERSION != 102800
        PYTHON_DEFINE_METHOD(Class, &UnitWrapper::equal, "__eq__");
        PYTHON_DEFINE_METHOD(Class, &UnitWrapper::notequal, "__ne__");
//#else
//Class.def (boost::python::operators< (boost::python::op_eq | boost::python::op_ne) >(), boost::python::right_operand<UnitWrapper>());
//#endif
        DefineOddUnitFunctions(Class);
    PYTHON_END_CLASS(VS, UnitWrapper)

    PYTHON_BEGIN_CLASS(VS, UniverseUtil::PythonUnitIter, "un_iter")

        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::current, "current");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::isDone, "isDone");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::advance, "advance");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::advanceN, "advanceN");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::advanceSignificant, "advanceSignificant");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::advanceNSignificant, "advanceNSignificant");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::advanceInsignificant, "advanceInsignificant");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::advanceNInsignificant, "advanceNInsignificant");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::advancePlanet, "advancePlanet");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::advanceNPlanet, "advanceNPlanet");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::advanceJumppoint, "advanceJumppoint");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::advanceNJumppoint, "advanceNJumppoint");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::size, "size");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::next, "next");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::next, "__next__");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::current, "__iter__");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::remove, "remove");
        PYTHON_DEFINE_METHOD(Class, &UniverseUtil::PythonUnitIter::preinsert, "preinsert");
    PYTHON_END_CLASS(VS, UniverseUtil::PythonUnitIter)
    typedef PythonAI<FireAt> PythonAIFireAt;
    PYTHON_BEGIN_INHERIT_CLASS(VS, PythonAIFireAt, FireAt, "PythonAI")
        PYTHON_DEFINE_METHOD_DEFAULT(Class, &FireAt::Execute, "Execute", PythonAI<FireAt>::default_Execute);
        PYTHON_DEFINE_METHOD_DEFAULT(Class,
                                     &FireAt::ChooseTarget,
                                     "ChooseTarget",
                                     PythonAI<FireAt>::default_ChooseTarget);
        PYTHON_DEFINE_METHOD_DEFAULT(Class, &FireAt::SetParent, "init", PythonAI<FireAt>::default_SetParent);
        PYTHON_DEFINE_METHOD(Class, &FireAt::GetParent, "GetParent");
        PYTHON_DEFINE_METHOD(Class, &FireAt::AddReplaceLastOrder, "AddReplaceLastOrder");
        PYTHON_DEFINE_METHOD(Class, &FireAt::ExecuteLastScriptFor, "ExecuteLastScriptFor");
        PYTHON_DEFINE_METHOD(Class, &FireAt::FaceTarget, "FaceTarget");
        PYTHON_DEFINE_METHOD(Class, &FireAt::FaceTargetITTS, "FaceTargetITTS");
        PYTHON_DEFINE_METHOD(Class, &FireAt::MatchLinearVelocity, "MatchLinearVelocity");
        PYTHON_DEFINE_METHOD(Class, &FireAt::MatchAngularVelocity, "MatchAngularVelocity");
        PYTHON_DEFINE_METHOD(Class, &FireAt::ChangeHeading, "ChangeHeading");
        PYTHON_DEFINE_METHOD(Class, &FireAt::ChangeLocalDirection, "ChangeLocalDirection");
        PYTHON_DEFINE_METHOD(Class, &FireAt::MoveTo, "MoveTo");
        PYTHON_DEFINE_METHOD(Class, &FireAt::MatchVelocity, "MatchVelocity");
        PYTHON_DEFINE_METHOD(Class, &FireAt::Cloak, "Cloak");
        PYTHON_DEFINE_METHOD(Class, &FireAt::FormUp, "FormUp");
        PYTHON_DEFINE_METHOD(Class, &FireAt::FormUpToOwner, "FormUpToOwner");
        PYTHON_DEFINE_METHOD(Class, &FireAt::FaceDirection, "FaceDirection");
        PYTHON_DEFINE_METHOD(Class, &FireAt::XMLScript, "XMLScript");
        PYTHON_DEFINE_METHOD(Class, &FireAt::LastPythonScript, "LastPythonScript");
    PYTHON_END_CLASS(VS, FireAt)

    PYTHON_BEGIN_CLASS(VS, VegaStrike::EngineVersionData, "EngineVersion")
        PYTHON_DEFINE_METHOD(Class, &VegaStrike::EngineVersionData::GetVersion, "GetVersion");
        PYTHON_DEFINE_METHOD(Class, &VegaStrike::EngineVersionData::GetAssetAPIVersion, "GetAssetAPIVersion");
    PYTHON_END_CLASS(VS, VegaStrike::EngineVersionData)

PYTHON_END_MODULE(VS)

void InitVS()
{
    PyImport_AppendInittab("VS", PYTHON_MODULE_INIT_FUNCTION(VS));
}

void InitVS2()
{
    Python::reseterrors();
    PYTHON_INIT_MODULE(VS);
    Python::reseterrors();
}

static std::string ParseSizeFlags(int size)
{
    static const std::pair<int, std::string> masks[] = {
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::LIGHT), " LIGHT"),
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::MEDIUM), " MEDIUM"),
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::HEAVY), " HEAVY"),
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::CAPSHIPLIGHT), " CAPSHIPLIGHT"),
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::CAPSHIPHEAVY), " CAPSHIPHEAVY"),
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::SPECIAL), " SPECIAL"),
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::LIGHTMISSILE), " LIGHTMISSILE"),
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::MEDIUMMISSILE), " MEDIUMMISSILE"),
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::HEAVYMISSILE), " HEAVYMISSILE"),
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::CAPSHIPLIGHTMISSILE), " CAPSHIPLIGHTMISSILE"),
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::CAPSHIPHEAVYMISSILE), " CAPSHIPHEAVYMISSILE"),
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::SPECIALMISSILE), " SPECIALMISSILE"),
            std::pair<int, std::string>(as_integer(MOUNT_SIZE::AUTOTRACKING), " AUTOTRACKING")
    };
    std::string rv;
    for (unsigned int i = 0; i < sizeof(masks) / sizeof(*masks); ++i) {
        if (size & masks[i].first) {
            rv += masks[i].second;
        }
    }
    if (!rv.empty()) {
        return rv.substr(1);
    } else {
        return std::string("NOWEAP");
    }
}

#define PARSE_CASE(Class, Enum) \
case Class::Enum:                 \
    return std::string( #Enum )
#define PARSE_CASE_DEFAULT \
default:                   \
    return std::string( "UNDEFINED" )

static std::string ParseMountStatus(Mount::STATUS status)
{
    switch (status) {
        PARSE_CASE(Mount, ACTIVE);
        PARSE_CASE(Mount, INACTIVE);
        PARSE_CASE(Mount, DESTROYED);
        PARSE_CASE(Mount, UNCHOSEN);
        PARSE_CASE_DEFAULT;
    }
}

static std::string ParseWeaponType(WEAPON_TYPE type)
{
    switch (type) {
        case WEAPON_TYPE::BEAM:
            return "BEAM";
        case WEAPON_TYPE::BALL:
            return "BALL";
        case WEAPON_TYPE::BOLT:
            return "BOLT";
        case WEAPON_TYPE::PROJECTILE:
            return "PROJECTILE";
        default:
            return "UNDEFINED";
    }
}

static BoostPythonDictionary GatherWeaponInfo(const WeaponInfo *wi)
{
    BoostPythonDictionary rv;
    if (wi) {
        rv["type"] = ParseWeaponType(wi->type);
        rv["speed"] = wi->speed;
        rv["range"] = wi->range;
        rv["damage"] = wi->damage;
        rv["phaseDamage"] = wi->phase_damage;
        rv["stability"] = wi->stability;
        rv["longRange"] = wi->long_range;
        rv["lockTime"] = wi->lock_time;
        rv["energyRate"] = wi->energy_rate;
        rv["refire"] = wi->Refire();
        rv["volume"] = wi->volume;
        rv["name"] = wi->name;
    }
    return rv;
}

BoostPythonDictionary UnitWrapper::GetMountInfo(int index) const
{
    BoostPythonDictionary rv;
    if ((index >= 0) && ((unsigned) index < unit->mounts.size())) {
        Mount &mnt = unit->mounts[index];

        Vector pos = mnt.GetMountLocation();
        Vector scale = Vector(mnt.xyscale, mnt.xyscale, mnt.zscale);
        Quaternion orientation = mnt.GetMountOrientation();

        rv["position"] = VS_BOOST_MAKE_TUPLE(pos.i, pos.j, pos.k);
        rv["orientation"] = VS_BOOST_MAKE_TUPLE_4(orientation.v.i, orientation.v.j, orientation.v.k, orientation.s);
        rv["scale"] = VS_BOOST_MAKE_TUPLE(scale.i, scale.j, scale.k);

        rv["empty"] = mnt.IsEmpty();
        rv["volume"] = mnt.volume;
        rv["ammo"] = mnt.ammo;
        rv["size"] = mnt.size;
        rv["size_flags"] = ParseSizeFlags(mnt.size);
        rv["bank"] = mnt.bank;
        rv["functionality"] = mnt.functionality;
        rv["maxfunctionality"] = mnt.maxfunctionality;

        rv["status"] = ParseMountStatus(mnt.status);
        if (!mnt.IsEmpty()) {
            rv["weapon_info"] = GatherWeaponInfo(mnt.type);
        }
    } else {
        rv["empty"] = true;
        rv["status"] = std::string("UNDEFINED");
    }
    return rv;
}

