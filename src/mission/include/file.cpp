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

	free(file);

	if (top == NULL) { cout << "Unable to load mission file\n"; return; }

	read = top;

	ScanNode(NULL, top);
}

void ScanNode (string *parent, easyDomNode *node) {
	vector<easyDomNode *>::const_iterator siter;
	string scan_name, current;

	current = node->Name();
	for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
		ScanNode(&current, *siter);
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
	free(param);
}
