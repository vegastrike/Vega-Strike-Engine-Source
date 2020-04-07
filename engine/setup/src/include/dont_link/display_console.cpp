/***************************************************************************
 *                           display_console.cpp  -  description
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

#include "display_console.h"
//#ifdef CONSOLE

void InitGraphics(void) {
	//Nothing to do. This function is here so the installer can easily switch display libraries
	cout << "Welcome to " << CONFIG.program_name << " Configuration\n";
}

void ShowMain(void) {
	struct group *CURRENT;
	int count = 1;
	char choice = '\0';
	int number = 0;
	int loop = 1;
	CURRENT = &GROUPS;
	cout << "___________________________________\n";
	do {
		if (CURRENT->name == NULL) { continue; }
		cout << count << ") " << CURRENT->name << " [" << GetInfo(CURRENT->setting) << "]\n";
		count++;
	} while ((CURRENT = CURRENT->next) != NULL);
	cout << "Q) Quit\n";
	count--;
	while (loop) {
		choice = '\0';
		number = 0;
		cout << "Choice: ";
		cin >> choice;
		number = atoi(&choice);
		if (choice == 'q' || choice == 'Q') { cout << "Thank you for using " << CONFIG.program_name << " Configuration\n"; exit(0); }
		if (choice == '\0' || number > count) { continue; }
		loop = 0;
	}
	ShowChoices(number);
	ShowMain();
}


void ShowChoices(int val) {
	struct group *CURRENT, *CHOICE;
	struct catagory *CUR, *CHOICE_CAT;
	int count = 1;
	char choice = '\0';
	int number = 0;
	int loop = 1;
	int setted = 0;
	CURRENT = &GROUPS;
	CUR = &CATS;
	CHOICE = 0;
	CHOICE_CAT = 0;
	do {
		if (CURRENT->name == NULL) { continue; }
		if (count == val) { CHOICE = CURRENT; break; }
		count++;
	} while ((CURRENT = CURRENT->next) != NULL);
	if (CHOICE == 0) { return; }
	cout << "__________\n";
	cout << "Displaying choices for " << CHOICE->name << "\n";
	count = 1;
	do {
		if (CUR->name == NULL) { continue; }
		if (strcmp(CUR->group, CHOICE->name) != 0) { continue; }
		cout << count << ") ";
		if (strcmp(CHOICE->setting, CUR->name) == 0) { cout << "[" << GetInfo(CUR->name) << "]\n"; setted = count; CHOICE_CAT = CUR; }
		else { cout << GetInfo(CUR->name) << "\n"; }
		count++;
	} while ((CUR = CUR->next) != NULL);
	cout << "M) Main Menu\nQ) Quit\n";
	count--;
	while (loop) {
		number = 0;
		choice = '\0';
		cin >> choice;
		number = atoi(&choice);
		if (choice == 'q' || choice == 'Q') { cout << "Thank you for using " << CONFIG.program_name << " Configuration\n"; exit(0); }
		if (choice == 'm' || choice == 'M') { return; }
		if (number == 0 || number > count) { continue; }
		loop = 0;
	}
	if (setted == number) { return; }    // No change made
	if (CHOICE_CAT > 0) { DisableSetting(CHOICE_CAT->name, CHOICE_CAT->group); }
	count = 1;
	CUR = &CATS;
	do {
		if (CUR->name == NULL) { continue; }
		if (strcmp(CUR->group, CHOICE->name) != 0) { continue; }
		if (count != number) { count++; continue; }
		EnableSetting(CUR->name, CUR->group);
		CHOICE->setting = CUR->name;
		break;
	} while ((CUR = CUR->next) != NULL);
	return;
}
//#endif    //CONSOLE
