#include "cmd/container.h"
#include <string>
#include "gfx/vec.h"
#include "cmd/unit.h"
#include "python_class.h"
#include <boost/python/objects.hpp>

//makes to_python for both vector and qvector turn them into tuples :-)

BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE
PyObject *to_python (Vector vec) {
	return to_python(boost::python::tuple((double)vec.i,(double)vec.j,(double)vec.k));
}
PyObject *to_python (QVector vec) {
	return to_python(boost::python::tuple((double)vec.i,(double)vec.j,(double)vec.k));
}
BOOST_PYTHON_END_CONVERSION_NAMESPACE

using std::string;
//WARNING: Macro City ahead.  Please skip this section if you don't like macros.
#define CHECKME }Unit * me=GetUnit();if (!me){fprintf(stderr,"\nERROR: NULL Unit used in Python script; returning default value..."); return 
#define WRAPPED0(type,name,def) type name (){{CHECKME def;} return me -> name ();}  
#define WRAPPED1(type,name,atype,a,def) type name ( atype a ){{CHECKME def;} return me -> name ( a );}  
#define WRAPPED2(type,name,atype,a,btype,b,def) type name ( atype a, btype b ){{CHECKME def;} return me -> name ( a , b );}  
#define WRAPPED3(type,name,atype,a,btype,b,ctype,c,def) type name ( atype a, btype b , ctype c ){{CHECKME def;} return me -> name ( a , b , c );}  
#define voidWRAPPED0(name) void name (){{CHECKME;} me -> name ();}  
#define voidWRAPPED1(name,atype,a) void name ( atype a){{CHECKME;} me -> name ( a );}  
#define voidWRAPPED2(name,atype,a,btype,b) void name ( atype a, btype b ){{CHECKME;} me -> name ( a , b );}  
#define voidWRAPPED3(name,atype,a,btype,b,ctype,c) void name ( atype a, btype b, ctype c ){{CHECKME;} me -> name ( a , b , c );}  
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
  boost::python::tuple GetOrientation() {{CHECKME boost::python::tuple();}Vector p,q,r; unit->GetOrientation(p,q,r); return boost::python::tuple(p,q,r);}
  boost::python::tuple queryBSP (bool ShieldBSP) {{CHECKME boost::python::tuple();}float dist; UnitWrapper un; QVector st,en; Vector nml; un=unit->queryBSP(st,en,nml,dist,ShieldBSP); boost::python::tuple ret (un,st,en,nml); ret.set_item(4,dist); return ret;}
  boost::python::tuple cosAngleTo (UnitWrapper target, float speed, float range) {{CHECKME boost::python::tuple();}float dist; float ret=unit->cosAngleTo(target,dist,speed,range);return boost::python::tuple (ret,dist);}
  boost::python::tuple cosAngleFromMountTo (UnitWrapper target) {{CHECKME boost::python::tuple();}float dist; float ret=unit->cosAngleFromMountTo(target,dist);return boost::python::tuple (ret,dist);}
  boost::python::tuple getAverageGunSpeed () {{CHECKME boost::python::tuple();}float speed, range;unit->getAverageGunSpeed(speed,range);return boost::python::tuple(speed,range);}
  boost::python::tuple InsideCollideTree (UnitWrapper smaller) {{CHECKME boost::python::tuple();}QVector bigpos, smallpos; Vector bigNormal, smallNormal; bool ret=unit->InsideCollideTree(smaller,bigpos,bigNormal,smallpos,smallNormal); boost::python::tuple tup (bigpos,bigNormal,smallpos,smallNormal); tup.set_item(4,ret); return tup;}
  UnitWrapper getSubUnit(int which) {{CHECKME 0;}un_iter it=unit->getSubUnits(); for (int i=0;i<which;i++) {it.advance();}return it.current();}
  UnitWrapper getFlightgroupLeader () {{CHECKME 0;}Flightgroup *group=unit->getFlightgroup();if (group) return group->leader; else return 0;}
  float GetVelocityDifficultyMult() {{CHECKME 0;}float diff=1;unit->GetVelocityDifficultyMult(diff);return diff;}
  int GetJumpStatus(){{CHECKME -1;} return unit->GetJumpStatus().drive;}
  void ApplyDamage (const Vector & pnt, const Vector & normal, float amt, UnitWrapper dealer, float phasedamage, float r, float g, float b, float a) {
	  {CHECKME;}
	  unit->ApplyDamage(pnt,normal,amt,unit,GFXColor(r,g,b,a),dealer.GetUnit(),phasedamage);
  }
/////////////////////////////MANUAL WRAP//////
/*
  bool InCorrectStarSystem (StarSystem *active)
//  Class.def(&TransferUnitToSystem(unsigned int whichJumpQueue, class StarSystem * previouslyActiveStarSystem, bool DoSightAndSound) {{CHECKME 0;}class StarSystem * othActiveStarSystem=previouslyActiveStarSystem; unit->TransferUnitToSystem(whichJumpQueue,othActiveStarSystem,DoSightAndSound); return othActiveStarSystem;}
//  class StarSystem * TransferUnitToSystem(unsigned int whichJumpQueue, class StarSystem * previouslyActiveStarSystem, bool DoSightAndSound) {{CHECKME 0;}class StarSystem * othActiveStarSystem=previouslyActiveStarSystem; unit->TransferUnitToSystem(whichJumpQueue,othActiveStarSystem,DoSightAndSound); return othActiveStarSystem;}
*/
/////////////////////////////////NO WRAP//////
  UnitWrapper(UnitContainer cont) : UnitContainer(cont){}
  UnitWrapper(Unit *un=0) : UnitContainer(un){}
  operator Unit* () {return unit;}
};
PYTHON_BEGIN_MODULE(Unit)
PYTHON_BEGIN_CLASS(Unit,UnitWrapper,"Unit")
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
#define WRAPPED0(type,name,nada) Class.def(&UnitWrapper::name,#name);
#define WRAPPED1(type,name,atype,a,def) WRAPPED0(type,name,def)
#define WRAPPED2(type,name,atype,a,btype,b,def) WRAPPED0(type,name,def)
#define WRAPPED3(type,name,atype,a,btype,b,ctype,c,def) WRAPPED0(type,name,def)
#define voidWRAPPED0(name) WRAPPED0(void,name,0)
#define voidWRAPPED1(name,atype,a) WRAPPED0(void,name,0)
#define voidWRAPPED2(name,atype,a,btype,b) WRAPPED0(void,name,0)
#define voidWRAPPED3(name,atype,a,btype,b,ctype,c) WRAPPED0(void,name,0)
#include "python_unit_wrap.h"
#undef WRAPPED0
#undef WRAPPED1
#undef WRAPPED2
#undef WRAPPED3
#undef voidWRAPPED0
#undef voidWRAPPED1
#undef voidWRAPPED2
#undef voidWRAPPED3
//End of Macro City 2
  Class.def(&UnitWrapper::Kill,"Kill");
  Class.def(&UnitWrapper::SetTarget,"SetTarget");
  Class.def(&UnitWrapper::GetTarget,"GetTarget");
  Class.def(&UnitWrapper::SetVelocityReference,"SetVelocityReference");
  Class.def(&UnitWrapper::GetVelocityReference,"GetVelocityReference");
  Class.def(&UnitWrapper::GetOrientation,"GetOrientation");
  Class.def(&UnitWrapper::queryBSP,"queryBSP");
  Class.def(&UnitWrapper::cosAngleTo,"cosAngleTo");
  Class.def(&UnitWrapper::cosAngleFromMountTo,"cosAngleFromMountTo");
  Class.def(&UnitWrapper::getAverageGunSpeed,"getAverageGunSpeed");
  Class.def(&UnitWrapper::InsideCollideTree,"InsideCollideTree");
  Class.def(&UnitWrapper::getSubUnit,"getSubUnit");
  Class.def(&UnitWrapper::getFlightgroupLeader,"getFlightgroupLeader");
  Class.def(&UnitWrapper::GetVelocityDifficultyMult,"GetVelocityDifficultyMult");
  Class.def(&UnitWrapper::GetJumpStatus,"GetJumpStatus");
  Class.def(&UnitWrapper::ApplyDamage,"ApplyDamage");
PYTHON_END_CLASS(Unit,UnitWrapper)
PYTHON_END_MODULE(Unit)
TO_PYTHON_SMART_POINTER(UnitWrapper);
