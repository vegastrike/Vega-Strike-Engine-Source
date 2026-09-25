#include <Python.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "python/python_class.h"
#include "cmd/base.h"
#include "in_kb_data.h"
#include "in.h"
#include "remote_control.h"
#include "cmd/script/mission.h"
#include "cmd/script/msgcenter.h"
#include "vs_globals.h"
#include "save_util.h"
#include "universe_generic.h"
#include "star_system_generic.h"
#include "cmd/unit_generic.h"
#include "cmd/container.h"
#include "python/init.h"
#include "gfx/vec.h"
#include "universe_util.h"
#include "cmd/unit_util.h"
#include "faction_generic.h"
#include "cmd/ai/fire.h"
#include "cmd/images.h"
typedef boost::python::dict BoostPythonDictionary;
#include "python/unit_wrapper_class.h"

#include <boost/python.hpp>

extern KBHandler LookupKeyCommand(const std::string & name);

namespace RemoteControl {

static int state=0; // 0: not initialized yet, 1: active, -1: disabled
static PyObject *tickfn=NULL;
static std::vector<std::string> pending_releases;

static boost::python::list GetLinks() {
	boost::python::list ret;
	BaseInterface *base=BaseInterface::CurrentBase;
	if (!base)
		return ret;
	BaseInterface::Room *room=base->rooms[base->curroom];
	for (int i=0;i<room->links.size();i++) {
		BaseInterface::Room::Link *l=room->links[i];
		if (!l)
			continue;
		std::string kind="link";
		int target=-1;
		if (BaseInterface::Room::Goto *g=dynamic_cast<BaseInterface::Room::Goto*>(l)) {
			kind="goto";
			target=g->index;
		} else if (dynamic_cast<BaseInterface::Room::Launch*>(l)) {
			kind="launch";
		} else if (dynamic_cast<BaseInterface::Room::Comp*>(l)) {
			kind="comp";
		} else if (dynamic_cast<BaseInterface::Room::Eject*>(l)) {
			kind="eject";
		} else if (dynamic_cast<BaseInterface::Room::Python*>(l)) {
			kind="python";
		}
		boost::python::dict d;
		d["num"]=i;
		d["index"]=l->index;
		d["text"]=l->text;
		d["kind"]=kind;
		d["room"]=target;
		d["eventmask"]=(int)l->eventMask;
		d["pythonfile"]=l->pythonfile;
		d["x"]=l->x;
		d["y"]=l->y;
		d["wid"]=l->wid;
		d["hei"]=l->hei;
		ret.append(d);
	}
	return ret;
}

// Texts shown in the current room: BaseText objects (the GUI screens use
// them for prices, messages...) plus the talk/status line.
static boost::python::list GetTexts() {
	boost::python::list ret;
	BaseInterface *base=BaseInterface::CurrentBase;
	if (!base)
		return ret;
	BaseInterface::Room *room=base->rooms[base->curroom];
	for (int i=0;i<room->objs.size();i++) {
		BaseInterface::Room::BaseText *t=dynamic_cast<BaseInterface::Room::BaseText*>(room->objs[i]);
		if (t)
			ret.append(VS_BOOST_MAKE_TUPLE_2(t->index,t->text.GetText()));
	}
	ret.append(VS_BOOST_MAKE_TUPLE_2(std::string("#othtext"),base->othtext.GetText()));
	return ret;
}

// Click the first link with this index in the current room.
static bool ClickLink(std::string index) {
	BaseInterface *base=BaseInterface::CurrentBase;
	if (!base||base->CallComp)
		return false;
	BaseInterface::Room *room=base->rooms[base->curroom];
	for (int i=0;i<room->links.size();i++)
		if (room->links[i]&&room->links[i]->index==index)
			return base->ScriptedClick(i);
	return false;
}

// Click link number num (the "num" of GetLinks) in the current room.
static bool ClickLinkNum(int num) {
	BaseInterface *base=BaseInterface::CurrentBase;
	if (!base||base->CallComp)
		return false;
	return base->ScriptedClick(num);
}

// (mission name, objective text, completeness) of every active mission
static boost::python::list GetObjectives() {
	boost::python::list ret;
	for (unsigned int i=0;i<active_missions.size();i++) {
		Mission *m=active_missions[i];
		for (unsigned int j=0;j<m->objectives.size();j++)
			ret.append(VS_BOOST_MAKE_TUPLE(m->getVariable("mission_name",""),
						       m->objectives[j].objective,
						       m->objectives[j].completeness));
	}
	return ret;
}

// The last n messages of the message center, newest first:
// (time, from, to, message)
static boost::python::list GetMessages(int n) {
	boost::python::list ret;
	gameMessage msg;
	for (int i=0;i<n&&mission->msgcenter->last(i,msg);i++)
		ret.append(VS_BOOST_MAKE_TUPLE_4(msg.time,msg.from.get(),msg.to.get(),msg.message.get()));
	return ret;
}

static bool InBase() {
	return BaseInterface::CurrentBase!=NULL;
}

// state: "press", "release", "down" or "tap" (press now, release next frame)
static bool KeyCommand(std::string name, std::string how) {
	KBHandler h=LookupKeyCommand(name);
	if (!h)
		return false;
	KBData data;
	if (how=="press"||how=="tap") {
		h(data,PRESS);
		if (how=="tap")
			pending_releases.push_back(name);
	} else if (how=="release") {
		h(data,RELEASE);
	} else if (how=="down") {
		h(data,DOWN);
	} else {
		return false;
	}
	return true;
}

// Raw bytes of a save string (Director.getSaveString fails on non-UTF-8)
static boost::python::object GetSaveStringBytes(int cp, std::string key, unsigned int num) {
	std::string s=getSaveString(cp,key,num);
	return boost::python::object(boost::python::handle<>(PyBytes_FromStringAndSize(s.data(),s.size())));
}

// (name, [destination systems]) of every jump point in the active system
static boost::python::list GetJumpPoints() {
	boost::python::list ret;
	StarSystem *ss=_Universe->activeStarSystem();
	if (!ss)
		return ret;
	Unit *un;
	for (un_iter i=ss->getUnitList().createIterator();(un=*i)!=NULL;++i) {
		const std::vector<std::string> &dest=un->GetDestinations();
		if (!un->isJumppoint()||dest.empty())
			continue;
		boost::python::list d;
		for (unsigned int j=0;j<dest.size();j++)
			d.append(dest[j]);
		ret.append(VS_BOOST_MAKE_TUPLE_2(un->name.get(),d));
	}
	return ret;
}

// World positions and radii of a unit's docking ports: (x, y, z, radius)
static boost::python::list GetDockingPorts(boost::python::object o) {
	boost::python::list ret;
	UnitWrapper *w=boost::python::extract<UnitWrapper*>(o);
	Unit *un=w?w->GetUnit():NULL;
	if (!un)
		return ret;
	for (unsigned int i=0;i<un->image->dockingports.size();i++) {
		const DockingPorts &d=un->image->dockingports[i];
		QVector p=Transform(un->cumulative_transformation_matrix,d.pos.Cast());
		ret.append(VS_BOOST_MAKE_TUPLE_4(p.i,p.j,p.k,d.radius));
	}
	return ret;
}

PYTHON_BEGIN_MODULE(VSRemote)
	PYTHON_DEFINE_GLOBAL(VSRemote,&GetDockingPorts,"GetDockingPorts");
	PYTHON_DEFINE_GLOBAL(VSRemote,&GetJumpPoints,"GetJumpPoints");
	PYTHON_DEFINE_GLOBAL(VSRemote,&GetSaveStringBytes,"GetSaveStringBytes");
	PYTHON_DEFINE_GLOBAL(VSRemote,&GetLinks,"GetLinks");
	PYTHON_DEFINE_GLOBAL(VSRemote,&GetTexts,"GetTexts");
	PYTHON_DEFINE_GLOBAL(VSRemote,&ClickLink,"ClickLink");
	PYTHON_DEFINE_GLOBAL(VSRemote,&ClickLinkNum,"ClickLinkNum");
	PYTHON_DEFINE_GLOBAL(VSRemote,&GetObjectives,"GetObjectives");
	PYTHON_DEFINE_GLOBAL(VSRemote,&GetMessages,"GetMessages");
	PYTHON_DEFINE_GLOBAL(VSRemote,&InBase,"InBase");
	PYTHON_DEFINE_GLOBAL(VSRemote,&KeyCommand,"KeyCommand");
PYTHON_END_MODULE(VSRemote)

static void Init() {
	state=-1;
	const char *path=getenv("VS_REMOTE_CONTROL");
	if (!path||!path[0])
		return;
	std::string file(path);
	std::string dir=".", module=file;
	std::string::size_type slash=file.rfind('/');
	if (slash!=std::string::npos) {
		dir=file.substr(0,slash);
		module=file.substr(slash+1);
	}
	if (module.size()>3&&module.substr(module.size()-3)==".py")
		module=module.substr(0,module.size()-3);
	PYTHON_INIT_MODULE(VSRemote);
	PyObject *syspath=PySys_GetObject(const_cast<char*>("path")); // borrowed
	if (syspath) {
		PyObject *d=PyUnicode_FromString(dir.c_str());
		PyList_Insert(syspath,0,d);
		Py_DECREF(d);
	}
	PyObject *mod=PyImport_ImportModule(module.c_str());
	if (!mod) {
		PyErr_Print();
		fprintf(stderr,"remote control: cannot import %s\n",path);
		return;
	}
	tickfn=PyObject_GetAttrString(mod,"tick");
	Py_DECREF(mod);
	if (!tickfn) {
		PyErr_Print();
		fprintf(stderr,"remote control: %s has no tick()\n",path);
		return;
	}
	fprintf(stderr,"remote control: using %s\n",path);
	state=1;
}

void Tick(bool docked) {
	if (state==0)
		Init();
	if (state<0)
		return;
	std::vector<std::string> releases;
	releases.swap(pending_releases);
	for (int i=0;i<releases.size();i++)
		KeyCommand(releases[i],"release");
	PyObject *ret=PyObject_CallFunction(tickfn,const_cast<char*>("i"),docked?1:0);
	if (ret)
		Py_DECREF(ret);
	else
		PyErr_Print();
}

}
