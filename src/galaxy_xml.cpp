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
#include "vs_path.h"
#include "vs_globals.h"

#include "galaxy_xml.h"
#include "galaxy_gen.h"
#ifdef WRITEGALAXYCOORDS
#include "gfx/nav/navscreen.h"
#endif
#include <float.h>
using namespace XMLSupport;
extern std::string universe_path;
#include "networking/netserver.h"


std::vector <std::string> systems_vector;

void	GalaxyXML::Galaxy::ComputeSerials( std::vector<std::string> & stak)
{
	cout<<"Generating random serial numbers :"<<endl;
	static string sys = vs_config->getVariable("data","sectors","sectors");
	for( std::string sys=stak.back(); !stak.empty(); stak.pop_back())
	{
		string relpath( universe_path+sys+"/"+sys+".system");
		string systempath( datadir+relpath);
		cout<<"\t\tcomputing serials for "<<systempath<<"...";
		
		// Read the file
		FILE * fp = fopen( systempath.c_str(), "w+b");
		if( !fp)
		{
			cerr<<"!!! ERROR : cannot open system file : "<<systempath<<endl;
			exit(1);
		}
		fseek( fp, 0, SEEK_END);
		int readsize = 0;
		int file_size = ftell( fp);
		fseek( fp, 0, SEEK_SET);
		char * systembuf = new char[file_size+1];
        readsize = fread( systembuf, sizeof( char), file_size, fp);
        if( readsize!=file_size)
        {
            cout<<"Error reading system file : "<<systempath<<" read ("<<readsize<<") -  to be read("<<file_size<<")"<<endl;
            exit( 1);
        }
		systembuf[file_size] = 0;
		string system( systembuf);

		// Now looking for "<planet ", "<Planet ", "<PLANET ", "<unit ", "<Unit ", "<UNIT ", same for nebulas
		std::vector<std::string> search_patterns;

		bool newserials = true;
		if( system.find( "serial=", 0) != std::string::npos)
		{
			newserials = false;
			cout<<"Found serial in system file : replacing serials..."<<endl;
		}
		search_patterns.push_back( "<planet ");
		search_patterns.push_back( "<Planet ");
		search_patterns.push_back( "<PLANET ");
		search_patterns.push_back( "<unit ");
		search_patterns.push_back( "<Unit ");
		search_patterns.push_back( "<UNIT ");
		search_patterns.push_back( "<nebula ");
		search_patterns.push_back( "<Nebula ");
		search_patterns.push_back( "<NEBULA ");

		for( std::vector<std::string>::iterator ti=search_patterns.begin(); ti!=search_patterns.end(); ti++)
		{
			std::string search( (*ti));
			std::string::size_type search_length = (*ti).length();
			std::string::size_type curpos = 0;
			while( (curpos = system.find( "<planet ", curpos))!=std::string::npos)
			{
				ObjSerial new_serial = getUniqueSerial();
				std::string serial_str( (*ti)+"serial="+XMLSupport::tostring5( new_serial)+" ");
				// If there are already serial in the file we replace that kind of string : <planet serial="XXXXX"
				// of length search_length + 14 (length of serial="XXXXX")
				if( newserials)
					system.replace( curpos, search_length, serial_str);
				else
					system.replace( curpos, search_length+14, serial_str);
			}
		}

		// Add the system xml string to the server
		Server->addSystem( sys, system);

		// Overwrite the system files with the buffer containing serials
		fseek( fp, 0, SEEK_SET);
		if( fwrite( system.c_str(), 1, system.length(), fp) != system.length() )
		{
			cerr<<"!!! ERROR : writing system file"<<endl;
			exit(1);
		}

		/*
		std::vector<std::string> data;
		data.clear(); // clear the vector first.
		std::ifstream f(file);
		char buf[4096]; // assume no line is longer than 4095 chars.

		while (f.getline(buf,sizeof(buf)))
		{
			data.push_back(buf);
			if (! f.ignore(4096,'\n'))
				break; // ignore the newline.
		}
		*/
		cout<<" OK !"<<endl;
		delete systembuf;
	}
	cout<<"Computing done."<<endl;
}

namespace GalaxyXML {
enum GalaxyNames {
	UNKNOWN,
	GALAXY,
	SYSTEMS,
	SECTOR,
	SYSTEM,
	VAR,
	NAME,
	VALUE
};
const EnumMap::Pair element_names [] = {
	EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("Galaxy",GALAXY),
    EnumMap::Pair ("Systems", SYSTEMS),
    EnumMap::Pair ("Sector", SECTOR),
    EnumMap::Pair ("System", SYSTEM),
    EnumMap::Pair ("Var", VAR)
};
const EnumMap::Pair attribute_names [] = {
	EnumMap::Pair ("UNKNOWN", UNKNOWN),
	EnumMap::Pair ("name",NAME),
    EnumMap::Pair ("value",VALUE)
};

const EnumMap element_map(element_names, 6);
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
			break;
		case SECTOR:
		case SYSTEM:
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
			// Add the system in the system vector so that we can later compute serials
			systems_vector.push_back( name);
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
void dotabs (FILE* fp, unsigned int tabs) {
	for (unsigned int i=0;i<tabs;++i) {
		fprintf (fp,"\t");
	}
}
void Galaxy::writeSector(FILE * fp, int tabs) {
	for (StringMap::iterator dat = data.begin();dat!=data.end();++dat) {
		dotabs(fp,tabs);
		fprintf (fp,"<var name=\"%s\" value=\"%s\"/>\n",(*dat).first.c_str(),(*dat).second.c_str());
	}
	if (subheirarchy) {
		for (SubHeirarchy::iterator it=  subheirarchy->begin();it!=subheirarchy->end();++it) {
			dotabs(fp,tabs);
			fprintf (fp,"<%s name=\"%s\">\n",tabs>1?"system":"sector",(*it).first.c_str());
			(*it).second.writeSector(fp,tabs+1);
			dotabs(fp,tabs);
			fprintf (fp,"</%s>\n",tabs>1?"system":"sector");
		}
	}
}
void Galaxy::writeGalaxy(const char * filename) {
	FILE * fp = fopen (filename,"w");
	if (fp ) {
		fprintf (fp,"<galaxy><systems>\n");
		writeSector(fp,1);
		fprintf (fp,"</systems></galaxy>\n");

	}
}
Galaxy::Galaxy(const char *configfile){
  subheirarchy=NULL;
  FILE * fp = fopen (configfile,"r");
  string cf = configfile;
  if (!fp) {
    
    //    fp = fopen ((vs_config->getVariable (
    cf=getUniversePath()+"/"+configfile;
	fp = fopen (cf.c_str(),"r");
  }
  if (fp) {
	  GalaxyXML::XML x;
	  x.g=this;
	  
	  XML_Parser parser = XML_ParserCreate(NULL);
	  XML_SetUserData(parser,&x);
	  XML_SetElementHandler (parser,&GalaxyXML::beginElement,&GalaxyXML::endElement);
	  do {
		  const int chunk_size = 65536;
		  char buf[chunk_size];
		  int length;
		  length = fread (buf,1,chunk_size,fp);
		  XML_Parse(parser,buf,length,feof(fp));
	  }while (!feof(fp));
	  fclose (fp);

	  XML_ParserFree(parser);

	  if(SERVER)
		this->ComputeSerials( systems_vector);
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


