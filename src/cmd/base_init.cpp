#include <Python.h>
#include "python/python_class.h"
#include "base.h"
#include "base_util.h"
#include "vs_path.h"
#ifdef USE_BOOST_129
#include <boost/python/object.hpp>
#else
#include <boost/python/objects.hpp>
#endif

static boost::python::tuple GetRandomBarMessage () {
	gameMessage last;
	int i=0;
	vector <std::string> who;
	vector <std::string> say;
	vector <std::string> sounds;
	string newmsg;
	string newsound;
	who.push_back ("bar");
	while (( mission->msgcenter->last(i++,last,who))) {
		newmsg=last.message;
		newsound="";
		int first=newmsg.find_first_of("[");
		{
			int last=newmsg.find_first_of("]");
			if (first!=string::npos&&(first+1)<newmsg.size()) {
				newsound=newmsg.substr(first+1,last-first-1);
				newmsg=newmsg.substr(0,first);
			}
		}
		sounds.push_back(newsound);
		say.push_back(newmsg);
	}
	if (say.size()) {
		int index=rand()%say.size();
		return  VS_BOOST_MAKE_TUPLE_2(say[index],sounds[index]);
	} else {
		return  VS_BOOST_MAKE_TUPLE_2("","");
	}
}

PYTHON_BEGIN_MODULE(Base)
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::Room,"Room");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::GetCurRoom,"GetCurRoom");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::GetNumRoom,"GetNumRoom");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::Comp,"Comp");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::CompPython,"CompPython");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::Launch,"Launch");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::LaunchPython,"LaunchPython");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::Link,"Link");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::LinkPython,"LinkPython");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::Python,"Python");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::EraseLink,"EraseLink");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::Ship,"Ship");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::Texture,"Texture");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::Message,"Message");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::EnqueueMessage,"EnqueueMessage");
	PYTHON_DEFINE_GLOBAL(Base,&BaseUtil::EraseObj,"EraseObj");
	PYTHON_DEFINE_GLOBAL(Base,&GetRandomBarMessage,"GetRandomBarMessage");
PYTHON_END_MODULE(Base)

void InitBase() {
	Python::reseterrors();
	PYTHON_INIT_MODULE(Base);
	Python::reseterrors();
}
