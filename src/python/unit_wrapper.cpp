#ifndef PYTHON_STUB
#include "cmd/container.h"
#include <string>
#include "init.h"
#include "gfx/vec.h"
#include "cmd/unit_generic.h"
#include "python_class.h"

#include <boost/python/objects.hpp>
#include "universe_util.h"
#include "cmd/unit_util.h"
#include "faction.h"
#include "cmd/ai/fire.h"
//makes to_python for both vector and qvector turn them into tuples :-)
using namespace UnitUtil;
using namespace UniverseUtil;
using Orders::FireAt;
/*BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE
PyObject *to_python (Vector vec) {
	return to_python(boost::python::tuple((double)vec.i,(double)vec.j,(double)vec.k));
}
PyObject *to_python (QVector vec) {
	return to_python(boost::python::tuple((double)vec.i,(double)vec.j,(double)vec.k));
}

Vector from_python(PyObject *p,boost::python::type<Vector>) {
	Vector vec(0,0,0);
	PyArg_ParseTuple(p,"fff",&vec.i,&vec.j,&vec.k);
	return vec;
}
QVector from_python(PyObject *p,boost::python::type<QVector>) {
	QVector vec(0,0,0);
	PyArg_ParseTuple(p,"ddd",&vec.i,&vec.j,&vec.k);
	return vec;
}
BOOST_PYTHON_END_CONVERSION_NAMESPACE*/

using std::string;
//WARNING: Macro City ahead.  Please skip this section if you don't like macros.
const char* error="\nERROR: NULL Unit used in Python script; returning default value...";
#define CHECKME }Unit * me=GetUnit();if (!me){fprintf(stderr,error); return 
#define WRAPPED0(type,name,def) type name (){{CHECKME def;} return me -> name ();}  
#define WRAPPED1(type,name,atype,a,def) type name ( atype a ){{CHECKME def;} return me -> name ( a );}  
#define WRAPPED2(type,name,atype,a,btype,b,def) type name ( atype a, btype b ){{CHECKME def;} return me -> name ( a , b );}  
#define WRAPPED3(type,name,atype,a,btype,b,ctype,c,def) type name ( atype a, btype b , ctype c ){{CHECKME def;} return me -> name ( a , b , c );}  
#define voidWRAPPED0(name) void name (){{CHECKME;} me -> name ();}  
#define voidWRAPPED1(name,atype,a) void name ( atype a){{CHECKME;} me -> name ( a );}  
#define voidWRAPPED2(name,atype,a,btype,b) void name ( atype a, btype b ){{CHECKME;} me -> name ( a , b );}  
#define voidWRAPPED3(name,atype,a,btype,b,ctype,c) void name ( atype a, btype b, ctype c ){{CHECKME;} me -> name ( a , b , c );}  
#define voidWRAPPED5(name,atype,a,btype,b,ctype,c,dtype,d,etype,e) void name ( atype a, btype b, ctype c , dtype d , etype e ){{CHECKME;} me -> name ( a , b , c , d , e );}  
#define EXPORT_UTIL(name,aff)
#define voidEXPORT_UTIL(name) EXPORT_UTIL(name,0)
//End of Macro City
class UnitWrapper : public UnitContainer{
public:
#include "python_unit_wrap.h"
///////////////////////////////MANUAL WRAP//////
  void Kill () {{CHECKME;}unit->Kill(true);}
  UnitWrapper GetTarget () {{CHECKME 0;}return unit->Target();}
  UnitWrapper GetVelocityReference() {{CHECKME 0;}return unit->VelocityReference();}
  void SetVelocityReference(UnitWrapper targ) {{CHECKME;}unit->VelocityReference(targ);}
  void SetTarget (UnitWrapper targ) {{CHECKME;}unit->Target(targ);}
  boost::python::tuple GetOrientation() {{CHECKME boost::python::tuple(Vector(0,0,0),Vector(0,0,0),Vector(0,0,0));}Vector p,q,r; unit->GetOrientation(p,q,r); return boost::python::tuple(p,q,r);}
  boost::python::tuple queryBSP (QVector st, QVector en, bool ShieldBSP) {{CHECKME boost::python::tuple(0,Vector(0,0,1),0);}float dist; UnitWrapper un; Vector nml; un=unit->queryBSP(st,en,nml,dist,ShieldBSP); boost::python::tuple ret (un,nml,dist); return ret;}
  boost::python::tuple cosAngleToITTS (UnitWrapper target, float speed, float range) {{CHECKME boost::python::tuple(0,0);}float dist; float ret=unit->cosAngleTo(target,dist,speed,range);return boost::python::tuple (ret,dist);}
  boost::python::tuple cosAngleTo (UnitWrapper target) {{CHECKME boost::python::tuple(0,0);}float dist; float ret=unit->cosAngleTo(target,dist);return boost::python::tuple (ret,dist);}
  boost::python::tuple cosAngleFromMountTo (UnitWrapper target) {{CHECKME boost::python::tuple(0,0);}float dist; float ret=unit->cosAngleFromMountTo(target,dist);return boost::python::tuple (ret,dist);}
  boost::python::tuple getAverageGunSpeed () {{CHECKME boost::python::tuple(0,0);}float speed, range;unit->getAverageGunSpeed(speed,range);return boost::python::tuple(speed,range);}
  boost::python::tuple InsideCollideTree (UnitWrapper smaller) {{CHECKME boost::python::tuple(QVector(0,0,0),Vector(0,0,0),QVector(0,0,0),Vector(0,0,0));}QVector bigpos, smallpos; Vector bigNormal, smallNormal; if(!unit->InsideCollideTree(smaller,bigpos,bigNormal,smallpos,smallNormal)){bigpos=smallpos=QVector(0,0,0);} boost::python::tuple tup (bigpos,bigNormal,smallpos,smallNormal); return tup;}
//  UnitWrapper getSubUnit(int which) {{CHECKME 0;}un_iter it=unit->getSubUnits(); for (int i=0;i<which;i++) {it.advance();}return it.current();}
  UnitWrapper getFlightgroupLeader () {{CHECKME 0;}Flightgroup *group=unit->getFlightgroup();if (group) return group->leader; else return 0;}
  void setFlightgroupLeader (Unit * un) {{CHECKME;}Flightgroup *group=unit->getFlightgroup();if (group) group->leader.SetUnit(un);}
  float GetVelocityDifficultyMult() {{CHECKME 0;}float diff=1;unit->GetVelocityDifficultyMult(diff);return diff;}
  int GetJumpStatus(){{CHECKME -1;} return unit->GetJumpStatus().drive;}
  void ApplyDamage (Vector pnt,Vector normal, float amt, UnitWrapper dealer, float phasedamage, float r, float g, float b, float a) {
	  {CHECKME;}
	  unit->ApplyDamage(pnt,normal,amt,unit,GFXColor(r,g,b,a),dealer.GetUnit(),phasedamage);
  }
/////////////////////////////MANUAL WRAP//////
/*
  WRAPPED1(bool,TransferUnitToSystem,class StarSystem *,NewSystem,false)
  bool InCorrectStarSystem (StarSystem *active)
//  Class.def(&TransferUnitToSystem(unsigned int whichJumpQueue, class StarSystem * previouslyActiveStarSystem, bool DoSightAndSound) {{CHECKME 0;}class StarSystem * othActiveStarSystem=previouslyActiveStarSystem; unit->TransferUnitToSystem(whichJumpQueue,othActiveStarSystem,DoSightAndSound); return othActiveStarSystem;}
//  class StarSystem * TransferUnitToSystem(unsigned int whichJumpQueue, class StarSystem * previouslyActiveStarSystem, bool DoSightAndSound) {{CHECKME 0;}class StarSystem * othActiveStarSystem=previouslyActiveStarSystem; unit->TransferUnitToSystem(whichJumpQueue,othActiveStarSystem,DoSightAndSound); return othActiveStarSystem;}
*/
/////////////////////////////////NO WRAP//////
  UnitWrapper(UnitContainer cont) : UnitContainer(cont){}
  UnitWrapper(Unit *un=0) : UnitContainer(un){}
  operator Unit* () {return unit;}
  bool isNull () {return GetUnit()==0;}
  bool notNull () {return !isNull();}
  void setNull () {SetUnit(0);}
};
PYTHON_INIT_INHERIT_GLOBALS(VS,FireAt);
PYTHON_BEGIN_MODULE(VS)
#undef EXPORT_UTIL
#undef voidEXPORT_UTIL
#define EXPORT_UTIL(name,aff) VS.def(&UniverseUtil::name,#name);
#define voidEXPORT_UTIL(name) EXPORT_UTIL(name,0)
#undef EXPORT_FACTION
#undef voidEXPORT_FACTION
#define EXPORT_FACTION(name,aff) VS.def(&FactionUtil::name,#name);
#define voidEXPORT_FACTION(name) EXPORT_FACTION(name,0)
EXPORT_UTIL(SafeEntrancePoint,(10000000,0,0))
voidEXPORT_UTIL(pushSystem)
voidEXPORT_UTIL(popSystem)
EXPORT_UTIL(getSystemFile,"")
EXPORT_UTIL(getSystemName,"")
EXPORT_UTIL(getUnitList,un_iter())
EXPORT_UTIL(getUnit,Unit())
EXPORT_UTIL(getNumUnits,0)
EXPORT_FACTION(GetFactionName,"")
EXPORT_FACTION(GetFactionIndex,-1)
EXPORT_FACTION(GetRelation,0)
voidEXPORT_FACTION(AdjustRelation)
EXPORT_FACTION(GetNumFactions,0)
EXPORT_UTIL(GetAdjacentSystem,"")
EXPORT_UTIL(GetGalaxyProperty,"")
EXPORT_UTIL(GetNumAdjacentSystems,0)
voidEXPORT_UTIL(terminateMission)
EXPORT_UTIL(addObjective,0)
voidEXPORT_UTIL(setObjective)
voidEXPORT_UTIL(setCompleteness)
EXPORT_UTIL(getCompleteness,0)
voidEXPORT_UTIL(setOwner)
EXPORT_UTIL(getOwner,0)
voidEXPORT_UTIL(IOmessage)
EXPORT_UTIL(GetContrabandList,Unit())
EXPORT_UTIL(numActiveMissions,1)
voidEXPORT_UTIL(SetAutoStatus)
voidEXPORT_UTIL(LoadMission)
voidEXPORT_UTIL(setMissionOwner)
EXPORT_UTIL(getMissionOwner,1)
EXPORT_UTIL(GetDifficulty,1)
voidEXPORT_UTIL(SetDifficulty)
EXPORT_UTIL(GetGameTime,0)
voidEXPORT_UTIL(SetTimeCompression)
EXPORT_UTIL(launchJumppoint,Unit())
EXPORT_UTIL(launch,Unit())
EXPORT_UTIL(getRandCargo,Cargo())
EXPORT_UTIL(musicAddList,0)
voidEXPORT_UTIL(musicPlaySong)
voidEXPORT_UTIL(musicSkip)
voidEXPORT_UTIL(musicPlayList)
voidEXPORT_UTIL(musicLoopList)
voidEXPORT_UTIL(playSound)
voidEXPORT_UTIL(cacheAnimation)
voidEXPORT_UTIL(playAnimation)
voidEXPORT_UTIL(playAnimationGrow)
EXPORT_UTIL(getPlayer,Unit())
EXPORT_UTIL(getPlayerX,Unit())
EXPORT_UTIL(getCurrentPlayer,0)
EXPORT_UTIL(getNumPlayers,1)
EXPORT_UTIL(GetMasterPartList,Unit())
#undef EXPORT_UTIL
#undef voidEXPORT_UTIL
#undef EXPORT_FACTION
#undef voidEXPORT_FACTION

PYTHON_BEGIN_CLASS(VS,UnitWrapper,"Unit")
PYTHON_BASE_BEGIN_CLASS(VS,Cargo,"Cargo")
Class.def(boost::python::constructor<std::string,std::string,float,int,float,float>());
Class.def (&Cargo::SetPrice,"SetPrice");
Class.def (&Cargo::GetPrice,"GetPrice");
Class.def (&Cargo::SetMass,"SetMass");
Class.def (&Cargo::GetMass,"GetMass");
Class.def (&Cargo::SetVolume,"SetVolume");
Class.def (&Cargo::GetVolume,"GetVolume");
Class.def (&Cargo::SetQuantity,"SetQuantity");
Class.def (&Cargo::GetQuantity,"GetQuantity");
Class.def (&Cargo::SetContent,"SetContent");
Class.def (&Cargo::GetContent,"GetContent");
Class.def (&Cargo::SetCategory,"SetCategory");
Class.def (&Cargo::GetCategory,"GetCategory");
Class.def (&Cargo::SetMissionFlag,"SetMissionFlag");
Class.def (&Cargo::GetMissionFlag,"GetMissionFlag");
Class.def (&Cargo::GetDescription,"GetDescription");


PYTHON_END_CLASS(VS,Cargo)
//WARNING: Macro City 2 ahead.  Please skip this section, also if you don't like macros.
#undef CHECKME
#undef WRAPPED0
#undef WRAPPED1
#undef WRAPPED2
#undef WRAPPED3
#undef voidWRAPPED0
#undef voidWRAPPED1
#undef voidWRAPPED2
#undef voidWRAPPED3
#undef voidWRAPPED5
#define WRAPPED0(type,name,nada) Class.def(&UnitWrapper::name,#name);
#define WRAPPED1(type,name,atype,a,def) WRAPPED0(type,name,def)
#define WRAPPED2(type,name,atype,a,btype,b,def) WRAPPED0(type,name,def)
#define WRAPPED3(type,name,atype,a,btype,b,ctype,c,def) WRAPPED0(type,name,def)
#define voidWRAPPED0(name) WRAPPED0(void,name,0)
#define voidWRAPPED1(name,atype,a) WRAPPED0(void,name,0)
#define voidWRAPPED2(name,atype,a,btype,b) WRAPPED0(void,name,0)
#define voidWRAPPED3(name,atype,a,btype,b,ctype,c) WRAPPED0(void,name,0)
#define voidWRAPPED5(name,atype,a,btype,b,ctype,c,dtype,d,etype,e) WRAPPED0(void,name,0)
#define EXPORT_UTIL(name,aff) Class.def(&UnitUtil::name,#name);
#define voidEXPORT_UTIL(name) EXPORT_UTIL(name,0)
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
#undef EXPORT_UTIL
#undef voidEXPORT_UTIL
//End of Macro City 2
  Class.def(boost::python::operators< (boost::python::op_eq | boost::python::op_ne) >(), boost::python::right_operand<UnitWrapper>());
  Class.def(&UnitWrapper::setNull,"setNull");
  Class.def(&UnitWrapper::isNull,"isNull");
  Class.def(&UnitWrapper::notNull,"__nonzero__");
  Class.def(&UnitWrapper::Kill,"Kill");
  Class.def(&UnitWrapper::SetTarget,"SetTarget");
  Class.def(&UnitWrapper::GetTarget,"GetTarget");
  Class.def(&UnitWrapper::SetVelocityReference,"SetVelocityReference");
  Class.def(&UnitWrapper::GetVelocityReference,"GetVelocityReference");
  Class.def(&UnitWrapper::GetOrientation,"GetOrientation");
  Class.def(&UnitWrapper::queryBSP,"queryBSP");
  Class.def(&UnitWrapper::cosAngleTo,"cosAngleTo");
  Class.def(&UnitWrapper::cosAngleToITTS,"cosAngleToITTS");
  Class.def(&UnitWrapper::cosAngleFromMountTo,"cosAngleFromMountTo");
  Class.def(&UnitWrapper::getAverageGunSpeed,"getAverageGunSpeed");
  Class.def(&UnitWrapper::InsideCollideTree,"InsideCollideTree");
  Class.def(&UnitWrapper::getFlightgroupLeader,"getFlightgroupLeader");
  Class.def(&UnitWrapper::setFlightgroupLeader,"setFlightgroupLeader");
  Class.def(&UnitWrapper::GetVelocityDifficultyMult,"GetVelocityDifficultyMult");
  Class.def(&UnitWrapper::GetJumpStatus,"GetJumpStatus");
  Class.def(&UnitWrapper::ApplyDamage,"ApplyDamage");
PYTHON_END_CLASS(VS,UnitWrapper)
PYTHON_BEGIN_CLASS(VS,UnitCollection::UnitIterator,"un_iter")
  Class.def (&UnitCollection::UnitIterator::current,"current");
  Class.def (&UnitCollection::UnitIterator::advance,"advance");
  Class.def (&UnitCollection::UnitIterator::remove,"remove");
  Class.def (&UnitCollection::UnitIterator::preinsert,"preinsert");
PYTHON_END_CLASS(VS,UnitCollection::UnitIterator)
typedef PythonAI<FireAt> PythonAIFireAt;
PYTHON_BEGIN_INHERIT_CLASS(VS,PythonAIFireAt ,FireAt,"PythonAI")
  Class.def (&FireAt::Execute,"Execute",PythonAI< FireAt >::default_Execute);
  Class.def (&FireAt::ChooseTarget,"ChooseTarget",PythonAI< FireAt >::default_ChooseTarget);
  Class.def (&FireAt::SetParent,"init",PythonAI< FireAt >::default_SetParent);
  Class.def (&FireAt::GetParent,"GetParent");
  Class.def (&FireAt::AddReplaceLastOrder,"AddReplaceLastOrder");
  Class.def (&FireAt::ExecuteLastScriptFor,"ExecuteLastScriptFor");
  Class.def (&FireAt::FaceTarget,"FaceTarget");
  Class.def (&FireAt::FaceTargetITTS,"FaceTargetITTS");
  Class.def (&FireAt::MatchLinearVelocity,"MatchLinearVelocity");
  Class.def (&FireAt::MatchAngularVelocity,"MatchAngularVelocity");
  Class.def (&FireAt::ChangeHeading,"ChangeHeading");
  Class.def (&FireAt::ChangeLocalDirection,"ChangeLocalDirection");
  Class.def (&FireAt::MoveTo,"MoveTo");
  Class.def (&FireAt::MatchVelocity,"MatchVelocity");
  Class.def (&FireAt::Cloak,"Cloak");
  Class.def (&FireAt::FormUp,"FormUp");
  Class.def (&FireAt::FaceDirection,"FaceDirection");
  Class.def (&FireAt::XMLScript,"XMLScript");
  Class.def (&FireAt::LastPythonScript,"LastPythonScript");
PYTHON_END_CLASS(VS,FireAt)


PYTHON_END_MODULE(VS)
TO_PYTHON_SMART_POINTER(UnitWrapper);
TO_PYTHON_SMART_POINTER(Cargo);

PYTHON_INIT_GLOBALS(Unit,UnitWrapper)
BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE
PyObject *to_python(Unit * un) {
  return to_python (UnitWrapper(un));
}
Unit * from_python(PyObject *p,boost::python::type<Unit *>) {
  UnitWrapper uw =(from_python (p,boost::python::type<UnitWrapper&>()));
  return uw.GetUnit();
}
BOOST_PYTHON_END_CONVERSION_NAMESPACE

void InitVS() {
	Python::reseterrors();
	PYTHON_INIT_MODULE(VS);
	Python::reseterrors();
}









///////////////////////////////////////////////
// END C++
///////////////////////////////////////////////
//below replace ~ with enter
#else
#define false 0
#define true 1
#define MYPRINT(nam) print #nam
#define WRAPPED0(type,name,aff) def name(self):~    MYPRINT(name) ~    return aff
#define WRAPPED1(type,name,atype,a,aff) def name(self,a): ~    MYPRINT(name)  ~    return aff
#define WRAPPED2(type,name,atype,a,btype,b,aff) def name(self,a,b): ~    MYPRINT(name) ~    return aff
#define WRAPPED3(type,name,atype,a,btype,b,ctype,c,aff) def name(self,a,b,c): ~    MYPRINT(name) ~    return aff
#define voidWRAPPED0(name) def name(self): ~    MYPRINT(name)
#define voidWRAPPED1(name,atype,a) def name(self,a): ~    MYPRINT(name)
#define voidWRAPPED2(name,atype,a,btype,b) def name(self,a,b): ~    MYPRINT(name)
#define voidWRAPPED3(name,atype,a,btype,b,ctype,c) def name(self,a,b,c): ~    MYPRINT(name)
#define voidWRAPPED4(name,atype,a,btype,b,ctype,c,dtype,d) def name(self,a,b,c,d): ~    MYPRINT(name)
#define voidWRAPPED5(name,atype,a,btype,b,ctype,c,dtype,d,etype,e) def name(self,a,b,c,d,e): ~    MYPRINT(name)
#define voidEXPORT_UTIL(name) def name(a=None,b=None,c=None,d=None,e=None,f=None,g=None,h=None,i=None,j=None): ~    MYPRINT(name)
#define EXPORT_UTIL(name,aff) voidEXPORT_UTIL(name) ~    return aff
#define Vector
#define QVector


import random
import sys

voidEXPORT_UTIL(pushSystem)
voidEXPORT_UTIL(popSystem)
EXPORT_UTIL(getSystemFile,"")
EXPORT_UTIL(getSystemName,"")
EXPORT_UTIL(getUnitList,un_iter())
EXPORT_UTIL(getUnit,Unit())
EXPORT_UTIL(getNumUnits,0)
EXPORT_UTIL(launchJumppoint,Unit())
EXPORT_UTIL(launch,Unit())
EXPORT_UTIL(getRandCargo,Cargo("","",1,1,1,1))
EXPORT_UTIL(GetFactionName,"")
EXPORT_UTIL(GetFactionIndex,-1)
EXPORT_UTIL(GetRelation,0)
voidEXPORT_UTIL(AdjustRelation)
EXPORT_UTIL(GetNumFactions,0)
EXPORT_UTIL(GetGameTime,0)
voidEXPORT_UTIL(SetTimeCompression)
EXPORT_UTIL(GetAdjacentSystem,"")
EXPORT_UTIL(GetGalaxyProperty,"")
EXPORT_UTIL(GetNumAdjacentSystems,0)
EXPORT_UTIL(musicAddList,0)
EXPORT_UTIL(SafeEntrancePoint,(10000000,0,0))
voidEXPORT_UTIL(musicSkip)
voidEXPORT_UTIL(musicPlaySong)
voidEXPORT_UTIL(musicPlayList)
voidEXPORT_UTIL(musicLoopList)
EXPORT_UTIL(GetDifficulty,.88)
voidEXPORT_UTIL(SetDifficulty)
voidEXPORT_UTIL(playSound)
voidEXPORT_UTIL(cacheAnimation)
voidEXPORT_UTIL(playAnimation)
voidEXPORT_UTIL(playAnimationGrow)
voidEXPORT_UTIL(terminateMission)
EXPORT_UTIL(getPlayer,Unit())
EXPORT_UTIL(getPlayerX,Unit())
EXPORT_UTIL(getCurrentPlayer,0)
EXPORT_UTIL(getNumPlayers,2)
EXPORT_UTIL(numActiveMissions,1)
EXPORT_UTIL(addObjective,0)
voidEXPORT_UTIL(setObjective)
voidEXPORT_UTIL(setCompleteness)
EXPORT_UTIL(getCompleteness,0)
voidEXPORT_UTIL(setOwner)
EXPORT_UTIL(getOwner,Unit())
def IOmessage(time,fr,to,message):
   sys.stderr.write("IOmessage [printed in %f seconds; from %s to %s: '%s']\n" % (time,fr,to,message))
EXPORT_UTIL(GetMasterPartList,Unit())
EXPORT_UTIL(GetContrabandList,Unit())
voidEXPORT_UTIL(SetAutoStatus)
voidEXPORT_UTIL(LoadMission)
voidEXPORT_UTIL(setMissionOwner)
EXPORT_UTIL(getMissionOwner,1)

def string ():
  return ''
class Unit:
  def __init__(self):
    print 'Unit constructor called with (self)'
#define UnitWrapper Unit
#include "python_unit_wrap.h"

  WRAPPED1(bool,__eq__,UnitWrapper,oth,false);
  WRAPPED1(bool,__ne__,UnitWrapper,oth,true);
  voidWRAPPED0(Kill);
  voidWRAPPED0(setNull);
  WRAPPED0(int,__nonzero__,random.randrange(0,2));
  WRAPPED0(int,isNull,random.randrange(0,2));
  voidWRAPPED1(SetTarget,UnitWrapper,un);
  WRAPPED0(UnitWrapper, GetTarget,Unit());
  WRAPPED0(UnitWrapper, GetVelocityReference,Unit())
  voidWRAPPED1(SetVelocityReference,UnitWrapper,un);
  WRAPPED0(Tuple,GetOrientation,((1,0,0),(0,1,0),(0,0,1)))
  WRAPPED2(Tuple,queryBSP,Vector,start,Vector,end,(un,(0,0,1),0))
  WRAPPED3(Tuple,cosAngleToITTS,UnitWrapper, un, float,speed, float, range,(.95,10000)) 
  WRAPPED1(Tuple,cosAngleTo,UnitWrapper,un,(.93,10000))
  WRAPPED1(Tuple,cosAngleFromMountTo,UnitWrapper,un,(.93,10000)) 
  WRAPPED0(Tuple,getAverageGunSpeed,(200,10000))
  WRAPPED1(Tuple,InsideCollideTree,UnitWrapper,un,((0,0,0),(0,0,1),(0,0,0),(0,1,0)))
  WRAPPED1(UnitWrapper,getSubUnit,int,which,Unit())

class un_iter:
  def __init__(self):
    print 'un_iter constructor called with (self)'
  WRAPPED0(Unit,current,Unit())
  voidWRAPPED0(advance)
  voidWRAPPED0(remove)
  voidWRAPPED1(preinsert,Unit,un)

class Cargo:
  def __init__ (self,a,b,c,d,e,f):
    print 'Cargo constructor called with (self,%s,%s,%f,%d,%f,%f)' % (a,b,c,d,e,f)
  voidWRAPPED1(SetPrice,float,price)
  WRAPPED0(float,GetPrice,1)
  voidWRAPPED1(SetMass,float,mass)
  WRAPPED0(float,GetMass,1)
  voidWRAPPED1(SetVolume,float,volume)
  WRAPPED0(float,GetVolume,1)
  voidWRAPPED1(SetQuantity,int,quantity)
  WRAPPED0(int,GetQuantity,1)
  voidWRAPPED1(SetContent,string,content)
  WRAPPED0(string,GetContent,"weapons")
  voidWRAPPED1(SetCategory,string,category)
  WRAPPED0(string,GetCategory,"contraband")
  voidWRAPPED1(SetMissionFlag,bool,flag)
  WRAPPED0(bool,GetMissionFlag,0)

  WRAPPED0(string,GetCategory,"contraband")
  WRAPPED0(string,GetDescription,"")

class PythonAI:
  voidWRAPPED1(init,Unit,un)
  voidWRAPPED0(Execute)
  voidWRAPPED0(ChooseTarget)
  WRAPPED0(Unit,GetParent,Unit())
  def __init__(self):
    print 'PythonAI constructor called with (self)'
    self.init(Unit())
  voidWRAPPED1(AddReplaceLastOrder,bool,replace)
  voidWRAPPED1(ExecuteLastScriptFor,float, time)
  voidWRAPPED1(FaceTarget,bool, end)
  voidWRAPPED1(FaceTargetITTS,bool, end)
  voidWRAPPED4(MatchLinearVelocity,bool,terminate, Vector, vec, bool ,afterburn, bool, local)
  voidWRAPPED3(MatchAngularVelocity,bool, terminate, Vector, vec, bool, local)
  voidWRAPPED1(ChangeHeading,QVector, vec)
  voidWRAPPED1(ChangeLocalDirection,Vector, vec)
  voidWRAPPED2(MoveTo,Unit *,Targ, bool, afterburn)
  voidWRAPPED5(MatchVelocity,bool, terminate, Vector, vec, Vector, angvel, bool, afterburn, bool, local)
  voidWRAPPED2(Cloak,bool, enable,float, seconds)
  voidWRAPPED1(FormUp,QVector, pos)
  voidWRAPPED2(FaceDirection ,float, distToMatchFacing, bool, finish)
  voidWRAPPED1(XMLScript,string, script)
  voidWRAPPED0(LastPythonScript)

#endif
