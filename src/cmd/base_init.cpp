#include <Python.h>
#include "python/python_class.h"
#include "base.h"
#include "base_util.h"
#include "vs_path.h"
#include <boost/python/objects.hpp>

static boost::python::tuple GetRandomBarMessage () {
	gameMessage * last;
	int i=0;
	vector <std::string> who;
	vector <std::string> say;
	vector <std::string> sounds;
	string newmsg;
	string newsound;
	who.push_back ("bar");
	while ((last= mission->msgcenter->last(i++,who))!=NULL) {
		newmsg=last->message;
		newsound="";
		int first=newmsg.find_first_of("[");
		int last=newmsg.find_first_of("]");
		if (first!=string::npos&&(first+1)<newmsg.size()) {
			newsound=newmsg.substr(first+1,last-first-1);
			newmsg=newmsg.substr(0,first);
		}
		sounds.push_back(newsound);
		say.push_back(newmsg);
	}
	if (say.size()) {
		int index=rand()%say.size();
		return boost::python::tuple(say[index],sounds[index]);
	} else {
		return boost::python::tuple("","");
	}
}

PYTHON_BEGIN_MODULE(Base)
	Base.def (&BaseUtil::Room,"Room");
	Base.def (&BaseUtil::GetCurRoom,"GetCurRoom");
	Base.def (&BaseUtil::GetNumRoom,"GetNumRoom");
	Base.def (&BaseUtil::Comp,"Comp");
	Base.def (&BaseUtil::CompPython,"CompPython");
	Base.def (&BaseUtil::Launch,"Launch");
	Base.def (&BaseUtil::LaunchPython,"LaunchPython");
	Base.def (&BaseUtil::Link,"Link");
	Base.def (&BaseUtil::LinkPython,"LinkPython");
	Base.def (&BaseUtil::Python,"Python");
	Base.def (&BaseUtil::EraseLink,"EraseLink");
	Base.def (&BaseUtil::Ship,"Ship");
	Base.def (&BaseUtil::Texture,"Texture");
	Base.def (&BaseUtil::Message,"Message");
	Base.def (&BaseUtil::EnqueueMessage,"EnqueueMessage");
	Base.def (&BaseUtil::EraseObj,"EraseObj");
	Base.def (&GetRandomBarMessage,"GetRandomBarMessage");
PYTHON_END_MODULE(Base)

void InitBase() {
	Python::reseterrors();
	PYTHON_INIT_MODULE(Base);
	Python::reseterrors();
}
