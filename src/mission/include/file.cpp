/***************************************************************************
 *                           file.cpp  -  description
 *                           ----------------------------
 *                           begin                : January 18, 2002
 *                           copyright            : (C) 2002 by David Ranger
 *                           email                : sabarok@start.com.au
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 **************************************************************************/

#include "file.h"

void FindMissions(char *path) {
	MISSIONS = FindFiles(path, EXT_MISSION);
	return;
}

void LoadMission(char *filename) {
	char *file, *name;
	easyDomFactory<missionNode> *domf= new easyDomFactory<missionNode>();
	missionNode *top=domf->LoadXML(filename);
	easyDomNode *read;
	string scan_name;
//	vector<easyDomNode *>::const_iterator siter;
//	easyDomNode *sub = 0;

	file = strdup(filename);
	name = StripPath(file);
	StripExtension(name);

	SetString(&DATA.name, name);
	SetString(&DATA.author, "Anonymous");
	SetString(&DATA.description, "A standard mission");
	SetString(&DATA.briefing, "Destroy all hostiles");
	SetString(&DATA.path,filename);
	SetString(&DATA.numplayers,"1");
	free(file);

	if (top == NULL) { cout << "Unable to load mission file\n"; return; }

	read = top;

	ScanNode(NULL, top, filename);
}
string ClassName (string pythonfilename) {
  unsigned int where=pythonfilename.find_last_of ("/");
  if (where!=string::npos) {
    pythonfilename = pythonfilename.substr (where+1,pythonfilename.length()-(where+1));
  }
  while ((where=pythonfilename.find_first_of ("_"))!=string::npos) {
    string first = pythonfilename.substr (0,where);
    string last = pythonfilename.substr (where+1,pythonfilename.length()-(where+1));
    printf ("chopping %s %s",first.c_str(),last.c_str());
    first[0] = toupper (first[0]);
    last[0] = toupper (last[0]);
    pythonfilename = first+last;
  }
  return pythonfilename;
}
using std::string;
std::string PrintArg (easyDomNode *node) {
  assert (node->Name()=="const");
  string type = node->attr_value ("type");
  string def;
  string retval= node->attr_value ("value");
  if (type=="int") {
    def="0";
  }else if (type=="float") {
    def="0.0";
  }else if (type=="bool") {
    def ="0";
    if (retval=="false") {
      retval = "0";
    }else if (retval =="true") {
      retval="1";
    }
  }else if (type=="object") {
    retval = string("'")+retval+string("'");
    def="''";
  }
  if (retval.empty()) {
    retval = def;
  }
  return retval;
}
void PrintArgs(FILE *fp,easyDomNode * node) {
  for (unsigned int i=0;i<node->subnodes.size();i++) {
    string arg =PrintArg (node->subnodes[i]);
    fprintf (fp,"%s",arg.c_str());
    if (i+1<node->subnodes.size()) {
      fprintf (fp,", ");
    }
  }
}
void LoadPythonModule(string fn, string pfn) {
  FILE * fp = fopen (fn.c_str(),"r");
  if (fp) {
    fseek (fp,0,SEEK_END);
    int size = ftell (fp)+1;
    fseek (fp,0,SEEK_SET);
    char * temp = (char *)malloc (size);
    temp[size]='\0';
    fread (temp,size-1,1,fp);
    fclose (fp);
    fp = fopen (fn.c_str(),"w");
    string wholefile (temp);
    free (temp);
    unsigned int where;
    if (wholefile.find ("python")!=string::npos) {
      return;
    }
    string tofind ("\"director\"");
    while ((where =wholefile.find (tofind))!=string::npos) {
      where += tofind.length();
      fwrite (wholefile.c_str(),where,1,fp);
      string addition (" python=\""+pfn+"\"");
      fwrite (addition.c_str(),addition.length(),1,fp);
      wholefile = wholefile.substr (where,wholefile.length()-(where+1));
    }
    fwrite (wholefile.c_str(),wholefile.length(),1,fp);
    
    fclose (fp);

  }
}
void PrintPython (easyDomNode * node, string filename) {
  string pythonfilename= filename.substr(0,filename.rfind (".mission"));
 
    pythonfilename += ".py";
    LoadPythonModule (filename,pythonfilename);
  FILE * fp = fopen (pythonfilename.c_str(),"w");
  if (fp) {
    string module = node->attr_value ("module");
    string classname = ClassName (module);
    if (module.length()) {
      fprintf (fp,"import %s\n",module.c_str());
      fprintf (fp,"newmission = %s.%s()\n",module.c_str(),classname.c_str());
    }else {
      fprintf (fp,"newmission = %s()\n",classname.c_str());
    }
    string myname =node->attr_value ("name");
    fprintf (fp,"newmission.%s(",myname.c_str());
    if (node->subnodes.size()>0) {
      PrintArgs (fp,node);
    }
    fprintf (fp,")\nnewmission=0\n");
    fclose (fp);
  }
}
void ScanNode (string *parent, easyDomNode *node, string  filename) {
	vector<easyDomNode *>::const_iterator siter;
	string scan_name, current;

	current = node->Name();
	if (current=="script") {
	  if (node->attr_value("name")=="initgame") {
	    fprintf (stderr,"initgame found");
	    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
	      
	      if ((*siter)->Name()=="exec") {
		PrintPython ((*siter),filename);
	      }
	    }
	    
	  }
	}
	for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {

		ScanNode(&current, *siter, filename);
	}
	if (parent == NULL) { return; }		// We're at the top node, probably <mission></mission>
	CheckVar(*parent, current, node->attr_value("name"), node->attr_value("value"));
}

// If the line is  <mission><variable><var name="alpha" value="beta"/></variable></mission>, then
// the parameters will be:  variable, var, alpha, beta
void CheckVar(string parent, string current, string name, string value) {
	char *param;
	int i;
	if (name.empty() || value.empty()) {
		return;
	}
	param = strdup(value.c_str());
	if (name == "mission_name") {
		SetString(&DATA.name, param);
	}
	if (name == "author") {
		SetString(&DATA.author, param);
	}
	if (name == "description") {
		for (i=0;param[i]!='\0';i++) {
			if (param[i]=='\\') {
				param[i]='\n';
			}
		}
		SetString(&DATA.description, param); }
	if (name == "briefing") {
		for (i=0;param[i]!='\0';i++) {
			if (param[i]=='\\') {
				param[i]='\n';
			}
		}
		SetString(&DATA.briefing, param);
	}
	if (name=="num_players") {
		SetString(&DATA.numplayers, param);
	}
	free(param);
}
