/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  Daniel Horn
*/

#include <expat.h>
#include "xml_support.h"
#include "vsfilesystem.h"
#include "vs_globals.h"

#include "galaxy_xml.h"
#include "galaxy_gen.h"
#ifdef WRITEGALAXYCOORDS
#include "gfx/nav/navscreen.h"
#endif
#include <float.h>
using namespace XMLSupport;
using namespace VSFileSystem;

namespace GalaxyXML {
enum GalaxyNames {
	UNKNOWN,
	GALAXY,
	SYSTEMS,
	SECTOR,
	SYSTEM,
	VAR,
	NAME,
	VALUE,
        PLANETS,
        PLANET
};
const EnumMap::Pair element_names [8] = {
	EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("Galaxy",GALAXY),
    EnumMap::Pair ("Systems", SYSTEMS),
    EnumMap::Pair ("Sector", SECTOR),
    EnumMap::Pair ("System", SYSTEM),
    EnumMap::Pair ("Planets", PLANETS),
    EnumMap::Pair ("Planet", PLANET),
    EnumMap::Pair ("Var", VAR)
};
const EnumMap::Pair attribute_names [3] = {
	EnumMap::Pair ("UNKNOWN", UNKNOWN),
	EnumMap::Pair ("name",NAME),
    EnumMap::Pair ("value",VALUE)
};

const EnumMap element_map(element_names, 8);
const EnumMap attribute_map(attribute_names, 3);
	class XML {
	public:
		Galaxy * g;
		std::vector <std::string> stak;
	};
	void beginElement( void * userdata, const XML_Char * nam, const XML_Char ** atts){
		AttributeList::const_iterator iter;
		XML * xml = (XML *)userdata;
		string tname (nam);
		AttributeList attributes (atts);
		GalaxyNames elem = (GalaxyNames) element_map.lookup(tname);
		GalaxyNames attr;
		string name;
		string value;
		switch (elem) {
		case GALAXY:
			break;
		case SYSTEMS:
			xml->stak.push_back ("<planets>");
			xml->g->addSection (xml->stak);
			break;
		case SECTOR:
		case SYSTEM:
                case PLANETS:
                case PLANET:
			for (iter = attributes.begin(); iter!=attributes.end();++iter) {
				attr = (GalaxyNames)attribute_map.lookup((*iter).name);
				switch (attr) {
				case NAME:
					name = (*iter).value;
					break;
				default:
					break;
				}
			}
			xml->stak.push_back (name);
			xml->g->addSection (xml->stak);

			break;
		case VAR:
			for (iter = attributes.begin(); iter!=attributes.end();++iter) {
				attr = (GalaxyNames)attribute_map.lookup((*iter).name);
				switch(attr) {
				case NAME:
					name = (*iter).value;
					break;
				case VALUE:
					value = (*iter).value;
					break;
				default:break;
				}
			}
			xml->g->setVariable(xml->stak,name,value);
			break;
		default:break;
		}

		
	}
	void endElement( void * userdata, const XML_Char * nam){
		XML * xml = (XML *)userdata;
		string name (nam);
		GalaxyNames elem = (GalaxyNames) element_map.lookup(name);
		switch (elem) {
		case GALAXY:
		case SYSTEMS:
			break;
		case VAR:
			break;
		case SECTOR:
		case SYSTEM:
			xml->stak.pop_back();
			break;
		default:break;
		}
	}
}

using namespace GalaxyXML;
Galaxy::~Galaxy() {
	if (subheirarchy)
		delete subheirarchy;
	subheirarchy = NULL;
}
Galaxy & Galaxy::operator = (const Galaxy & g) {
	if (g.subheirarchy) {
		SubHeirarchy *temp = new SubHeirarchy (*g.subheirarchy);
		if (subheirarchy)
			delete subheirarchy;
		subheirarchy = temp;
	}else {
		if (subheirarchy) {
			delete subheirarchy;
			subheirarchy = NULL;
		}
	}
	data = g.data;
	return *this;
}
Galaxy::Galaxy (const Galaxy & g):data(g.data) {
	if (g.subheirarchy) {
		subheirarchy = new SubHeirarchy (*g.subheirarchy);
	}else
		subheirarchy = NULL;
        if (subheirarchy) {
          SubHeirarchy::iterator iter=subheirarchy->find("<planets>");
          if (iter==subheirarchy->end()) {
            planet_types = NULL;
          } else {
            planet_types = &(*iter).second;
          }
        }
}

void Galaxy::processSystem(string sys,const QVector &coords){
	string sector = getStarSystemSector(sys);
	sys = getStarSystemName(sys).c_str();
	char coord [65536];
	sprintf(coord,"%lf %lf %lf",coords.i,coords.j,coords.k);
	string ret = getVariable (sector,sys,"");
	if (ret.length()==0) {
		setVariable(sector,sys,"xyz",coord);
	}
}
void Galaxy::processGalaxy(string sys) {
#ifdef WRITEGALAXYCOORDS
	NavigationSystem::SystemIterator si(sys,256000);
	while (!si.done()) {
		string sys = *si;
		processSystem(sys,si.Position());
		++si;
	}
#endif
}
void dotabs (VSFileSystem::VSFile & f, unsigned int tabs) {
	for (unsigned int i=0;i<tabs;++i) {
		f.Fprintf ("\t");
	}
}
void Galaxy::writeSector(VSFileSystem::VSFile & f, int tabs, string sectorType) {
	StringMap::iterator dat;
	for (dat = data.begin();dat!=data.end();++dat) {
          if ((*dat).first!="jumps") {
		dotabs(f,tabs);
		f.Fprintf ("<var name=\"%s\" value=\"%s\"/>\n",(*dat).first.c_str(),(*dat).second.c_str());
          }
	}
        dat = data.find(std::string("jumps"));
        if (dat!=data.end()) {
          dotabs(f,tabs);
          f.Fprintf("<var name=\"jumps\" value=\"%s\"/>\n",(*dat).second.c_str());
        }
	if (subheirarchy) {
		for (SubHeirarchy::iterator it=  subheirarchy->begin();it!=subheirarchy->end();++it) {
			if (&(*it).second!=planet_types) {
				dotabs(f,tabs);
				f.Fprintf ("<%s name=\"%s\">\n",sectorType.c_str(),(*it).first.c_str());
				(*it).second.writeSector(f,tabs+1,"system");
				dotabs(f,tabs);
				f.Fprintf ("</%s>\n",sectorType.c_str());
			}
		}
	}
}
void Galaxy::writeGalaxy(VSFile &f) {
	f.Fprintf ("<galaxy>\n<systems>\n");
	writeSector(f,1);
	f.Fprintf ("</systems>\n");
	if (planet_types) {
		f.Fprintf("<planets>\n");
		planet_types->writeSector(f,1,"planet");
		f.Fprintf("</planets>\n");
	}
	f.Fprintf("</galaxy>\n");
}

Galaxy::Galaxy(const char *configfile){
  using namespace VSFileSystem;
  subheirarchy=NULL;
  planet_types=NULL;
  VSFile f;
  VSError err = f.OpenReadOnly(configfile,UniverseFile);
  if (err<=Ok) {
	  GalaxyXML::XML x;
	  x.g=this;
	  
	  XML_Parser parser = XML_ParserCreate(NULL);
	  XML_SetUserData(parser,&x);
	  XML_SetElementHandler (parser,&GalaxyXML::beginElement,&GalaxyXML::endElement);
	  XML_Parse(parser,(f.ReadFull()).c_str(),f.Size(),1);
	  /*
	  do {
		  const int chunk_size = 65536;
		  char buf[chunk_size];
		  int length;
		  length = VSFileSystem::vs_read (buf,1,chunk_size,fp);
		  XML_Parse(parser,buf,length,VSFileSystem::vs_feof(fp));
	  }while (!VSFileSystem::vs_feof(fp));
	  VSFileSystem::vs_close (fp);
	  */
	  f.Close();

	  XML_ParserFree(parser);
        if (subheirarchy) {
          SubHeirarchy::iterator iter=subheirarchy->find("<planets>");
          if (iter==subheirarchy->end()) {
            planet_types = NULL;
          } else {
            planet_types = &(*iter).second;
          }
        }

  }
  else
  {
  	if(SERVER)
	{
		cerr<<"!!! ERROR : couldn't find galaxy file : "<<configfile<<endl;
		exit(1);
	}
  }
}

Galaxy *Galaxy::getPlanetTypes() {
  return planet_types;
}
SubHeirarchy & Galaxy::getHeirarchy() {
	if (!subheirarchy) {
		subheirarchy = new SubHeirarchy;
	}
	return *subheirarchy;
}
/* *********************************************************** */

/* *********************************************************** */
string Galaxy::getVariable (std::vector<string> section, string name, string default_value) {
	Galaxy * g = this;
	for (unsigned int i=0;i<section.size();++i) {
		if (g->subheirarchy) {
			SubHeirarchy::iterator sub = subheirarchy->find (section[i]);
			if (sub!=subheirarchy->end()) {
				g=  &(*sub).second;
			}else return default_value;
		}else return default_value;
	}
	StringMap::iterator dat = data.find (name);
	if (dat!=data.end())
		return (*dat).second;
	return default_value;										  
}
void Galaxy::addSection (std::vector<string> section) {
	SubHeirarchy * temp= &getHeirarchy();
	for (unsigned int i=0;i<section.size();++i) {
		temp = &((*temp)[section[i]].getHeirarchy());
	}
}
void Galaxy::setVariable (std::vector<string> section, string name, string value) {
	Galaxy * g = this;
	for (unsigned int i=0;i<section.size();++i) {
		g= &g->getHeirarchy()[section[i]];
	}
	g->data[name]=value;
}

/* *********************************************************** */

bool Galaxy::setVariable(string section,string name,string value){
	getHeirarchy()[section].data[name]=value;
	return true;
}

bool Galaxy::setVariable(string section,string subsection,string name,string value){
	getHeirarchy()[section].getHeirarchy()[subsection].data[name]=value;
	return true;
}


string Galaxy::getRandSystem (string sect, string def) {
	Galaxy &sector= getHeirarchy ()[sect];
	if (sector.subheirarchy==NULL) {
		return def;
	}
	unsigned int size = sector.getHeirarchy().size();
    if (size>0) {
		int which = rand()%size;
		SubHeirarchy::iterator i =
			sector.getHeirarchy().begin();
		for (;which>0;which--,i++) {
		}
		return (*i).first;

    }
	return def;
}
string Galaxy::getVariable(string section,string subsection,string name,string defaultvalue){
#ifdef WRITEGALAXYCOORDS
	static bool  blah = false;
	if (!blah) {
		processGalaxy("sol_sector/sol");
		writeGalaxy("/tmp/outputgalaxy");
		blah = true;
	}
#endif
	SubHeirarchy * s = subheirarchy;
	SubHeirarchy::iterator i;
	if (s) {
		i = s->find(section);
		if (i!=s->end()) {
			s = (*i).second.subheirarchy;
			if (s) {
				i = s->find(subsection);
				if (i!=s->end()) {
					Galaxy * g = &(*i).second;
					StringMap::iterator j = g->data.find(name);
					if (j!=g->data.end())
						return (*j).second;
				}
					
			}
		}
	}
	return defaultvalue;
}



string Galaxy::getVariable(string section,string name,string defaultvalue){

	SubHeirarchy * s = subheirarchy;
	SubHeirarchy::iterator i;
	if (s) {
		i = s->find(section);
		if (i!=s->end()) {
			Galaxy * g = &(*i).second;
			StringMap::iterator j = g->data.find(name);
			if (j!=g->data.end())
				return (*j).second;
		}
		
	}
	return defaultvalue;
}


 bool Galaxy::setPlanetVariable(string name,string value){
       if (!planet_types)
               return false;
       planet_types->data[name]=value;
       return true;
 }
 
 bool Galaxy::setPlanetVariable(string section,string name,string value){
       if (!planet_types)
               return false;
       planet_types->getHeirarchy()[section].data[name]=value;
       return true;
 }
 
 string Galaxy::getPlanetVariable(string section,string name,string defaultvalue){
       if (planet_types) {
               SubHeirarchy::iterator i;
               i = planet_types->subheirarchy->find(section);
               if (i!=planet_types->subheirarchy->end()) {
                       Galaxy * g = &(*i).second;
                       StringMap::iterator j = planet_types->data.find(name);
                       if (j!=g->data.end())
                               return (*j).second;
               }
       }
       return defaultvalue;
 }
 
 
 
string Galaxy::getPlanetVariable(string name,string defaultvalue){
  if (planet_types) {
    StringMap::iterator j = planet_types->data.find(name);
    if (j!=planet_types->data.end())
      return (*j).second;
  }
}
