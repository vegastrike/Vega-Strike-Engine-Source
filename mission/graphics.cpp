/***************************************************************************
 *                           graphics.cpp  -  description
 *                           --------------------------
 *                           begin                : December 28, 2001
 *                           copyright            : (C) 2001 by David Ranger
 *                           email                : ussreliant@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 ***************************************************************************/

#include "graphics.h"

int frame=0, timebase=0;

TextArea *MissionArea;
TextArea *ShowInfo;
TextArea *ShowDesc;
TextArea *ShowBrief;

Button *OK, *Quit;

int screen_width = WIDTH;
int screen_height = HEIGHT;

void InitGraphics(void) {

	#ifdef DOUBLE_BUFFER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	#else
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
	#endif    // DOUBLE_BUFFER
	glutInitWindowSize(WIDTH, HEIGHT);

	glutCreateWindow("Vega Strike Mission Selector");

	MissionArea = new TextArea(-1, 1, 1, 1.8, 1);

	ShowInfo = new TextArea(0, 1, 1, 0.25, 0);
	ShowDesc = new TextArea(0, 0.75, 1, 0.775, 1);
	ShowBrief = new TextArea(0, -0.025, 1, 0.775, 1);

	OK = new Button(-0.5, -0.85, 0.35, 0.1, "Load Mission");
	Quit = new Button(0.5, -0.85, 0.12, 0.1, "Quit");

	MissionArea->RenderText();

	ShowInfo->AddTextItem("name", "Mission:", NULL);
	ShowInfo->RenderText();

	ShowInfo->AddTextItem("author", "Author: ", NULL);
	ShowInfo->RenderText();

	ShowDesc->SetText("Description: ");
	ShowDesc->RenderText();

	ShowBrief->SetText("Briefing: ");
	ShowBrief->RenderText();

	AddMissionsToTree(DIR_MISSION, NULL);
}

void show_main(void) {
	StartFrame();
	ShowColor(-1,-1,2,2, 0,0,0,1);
	MissionArea->Refresh();
	ShowInfo->Refresh();
	ShowDesc->Refresh();
	ShowBrief->Refresh();
	OK->Refresh();
	Quit->Refresh();
	EndFrame();
}

void StartFrame(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	#ifdef DOUBLE_BUFFER
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix();
	#endif    // DOUBLE_BUFFER
	glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void EndFrame(void) {
	#ifdef FPS
	float fps = 0;
	int time = 0;
	#endif
	//glDisable(GL_TEXTURE_2D);
	#ifdef DOUBLE_BUFFER
	glPopMatrix();
	glutSwapBuffers();
	#endif    // DOUBLE_BUFFER
	glFlush();
}

void ProcessMouseClick(int button, int state, int x, int y) {
	float cur_x = 0, cur_y = 0, new_x = x, new_y = y;
	int ours, show_nothing = 0;
	char *choice;

	StartFrame();

	// Convert from an int x,y coord to a -1 to 1 float
	cur_x = ((new_x / screen_width) * 2) - 1;
	cur_y = ((new_y / screen_height) * -2) + 1;

	ours = MissionArea->MouseClick(button, state, cur_x, cur_y);
	if (ours > 0 && state == GLUT_UP) {
		choice = MissionArea->GetSelectedItemName();
		if (choice != 0 && choice[0] != '\0') { show_nothing = LoadMission(choice); }
		else { show_nothing = 1; }
 
		if (show_nothing == 1) {
			ShowInfo->ChangeTextItem("name", "Mission:");
			ShowInfo->ChangeTextItem("author", "Author:");
			ShowDesc->SetText("Description:");
		}
		else {
			char NameString[strlen(DATA.name) + strlen("Mission: ")+1];
			char AuthString[strlen(DATA.author) + strlen("Author: ")+1];
			sprintf(NameString, "Mission: %s", DATA.name);
			sprintf(AuthString, "Author: %s", DATA.author);
			ShowInfo->ChangeTextItem("name", NameString);
			ShowInfo->ChangeTextItem("author", AuthString);

			ShowDesc->SetText(DATA.description);
			ShowBrief->SetText(DATA.briefing);
		}
	}
	ours = OK->MouseClick(button, state, cur_x, cur_y);
	if (ours) {
		choice = MissionArea->GetSelectedItemName();
		if (choice != 0 || strcmp(choice, "text_area_master") != 0) {
			RunMission();
		} else {
			fprintf(stderr, "No mission selected\n");
		}
	}
	if (ours == 0) { ours = Quit->MouseClick(button, state, cur_x, cur_y); }
	if (ours) { exit(0); }
	if (ours == 0) { ours = ShowDesc->MouseClick(button, state, cur_x, cur_y); }
	if (ours == 0) { ours = ShowBrief->MouseClick(button, state, cur_x, cur_y); }
	MissionArea->Refresh();

	ShowInfo->Refresh();
	ShowDesc->Refresh();
	ShowBrief->Refresh();
	OK->Refresh();
	Quit->Refresh();

	EndFrame();
}

void ProcessMouseMovePassive(int x, int y) {
	float cur_x = 0, cur_y = 0, new_x = x, new_y = y;
	int ours;

	StartFrame();

	cur_x = ((new_x / screen_width) * 2) - 1;
	cur_y = ((new_y / screen_height) * -2) + 1;

	ours = MissionArea->MouseMove(cur_x, cur_y);
	if (ours == 0) { ours = OK->MouseMove(cur_x, cur_y); }
	if (ours == 0) { ours = Quit->MouseMove(cur_x, cur_y); }
	if (ours == 0) { ours = ShowDesc->MouseMove(cur_x, cur_y); }
	if (ours == 0) { ours = ShowBrief->MouseMove(cur_x, cur_y); }
	MissionArea->Refresh();
	ShowInfo->Refresh();
	ShowDesc->Refresh();
	ShowBrief->Refresh();
	OK->Refresh();
	Quit->Refresh();

	EndFrame();
}
void ProcessMouseMoveActive(int x, int y) {
	float cur_x = 0, cur_y = 0, new_x = x, new_y = y;
	int ours;

	StartFrame();

	cur_x = ((new_x / screen_width) * 2) - 1;
	cur_y = ((new_y / screen_height) * -2) + 1;

	ours = MissionArea->MouseMoveClick(cur_x, cur_y);
	if (ours == 0) { ours = ShowDesc->MouseMoveClick(cur_x, cur_y); }
	if (ours == 0) { ours = ShowBrief->MouseMoveClick(cur_x, cur_y); }
	MissionArea->Refresh();
	ShowInfo->Refresh();
	ShowDesc->Refresh();
	ShowBrief->Refresh();
	OK->Refresh();
	Quit->Refresh();

	EndFrame();
}

void ChangeSize(int wid, int hei) {
	if (hei == 0) { hei = 1; }
	screen_width = wid;
	screen_height = hei;
	float ratio = 1.0 * wid / hei;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glViewport(0, 0, wid, hei);
	gluPerspective(45, ratio, 1, 1000);
	EndFrame();
}


void AddMissionsToTree(char *path, char *parent) {
        glob_t *search;
        unsigned int length;
        int count, max;
        char *file, *filename;

        // First we check for sub directories. stick them at the top
        // For some reason, glob(,,GLOB_ONLYDIR,) doesn't seem to only match directories,
        // so FindDirs() currently returns everything. Check the last char for a /
        // That will be the directory.

        search = FindDirs(path);
        max = search->gl_pathc - 1;     // search->gl_pathc is a uint. If there's no files, it's 0.
        for (count = 0; count <= max; count++) {
                file = search->gl_pathv[count];
                length = strlen(file);
                if (file[length-1] != SEPERATOR) { continue; }  // Verify it's a directory and not a file

                filename = strdup(file);
                filename = StripPath(filename);
                if (strcmp("CVS", filename) == 0) { continue; } // Don't need to display this directory
//		MissionArea->AddTextItem(file, filename, parent);
/*
                item = AddItem(tree, filename, "dir");

                subtree = gtk_tree_new();
                gtk_signal_connect(GTK_OBJECT(subtree), "select_child", GTK_SIGNAL_FUNC(cb_select_child), subtree);
                gtk_signal_connect(GTK_OBJECT(subtree), "unselect_child", GTK_SIGNAL_FUNC(cb_unselect_child), subtree);

                gtk_tree_set_selection_mode(GTK_TREE(subtree), GTK_SELECTION_SINGLE);
                gtk_tree_set_view_mode(GTK_TREE(subtree), GTK_TREE_VIEW_ITEM);
                gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), subtree);
*/
                AddMissionsToTree(file, filename);

        }

        search = FindFiles(path, EXT_MISSION);
        max = search->gl_pathc - 1;
        for (count = 0; count <= max; count++) {
                file = search->gl_pathv[count];
                length = strlen(file);
                if (file[length-1] == SEPERATOR) { continue; }

                filename = strdup(file);
                filename = StripPath(filename);
                StripExtension(filename);
		replace(filename, "_", " ", strlen(filename));
		MissionArea->AddTextItem(file, filename, parent);
//                AddItem(tree, filename, file);
        }
        return;
}

