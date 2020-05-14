/***************************************************************************
 *                           display_gtk.cpp  -  description
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
#include "display.h"
#ifdef GTK
void AddCats(GtkWidget *vbox, char *group, char *def);
void ClickButton(GtkWidget *w, struct catagory *CUR);

#ifdef _WIN32
//#include <process.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#else
#include <unistd.h>
#endif

#include <string>

//#define USE_RADIO


static GtkWidget *window, *main_vbox;

void exit_0(GtkWidget *w, void *arg) {
  exit(0);
}

void InitGraphics(int *argc, char*** argv) {
	gtk_init(argc, argv);
        GET_TITLE; // sets title;
        GET_STATIC_TEXT;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
	gtk_window_set_title(GTK_WINDOW(window), title);

	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(exit_0), NULL);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(exit_0), NULL);

	gtk_container_set_border_width(GTK_CONTAINER(window), 0);

	main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 1);
	gtk_container_add(GTK_CONTAINER(window), main_vbox);

        GtkWidget *lbl=gtk_label_new(static_text);
	gtk_container_add(GTK_CONTAINER(main_vbox),lbl);
	gtk_widget_show(lbl);
	gtk_widget_show(main_vbox);
	gtk_widget_realize(window);

	gtk_widget_show(window);
}

void myexit(int exitval){
    std::string readme_path(CONFIG.data_path);
#ifdef _WIN32
    readme_path += "\\documentation\\readme.txt";
	int err=(int)ShellExecute(NULL,"open",readme_path.c_str(),"","",1);
#else
    readme_path += "/documentation/readme.txt";
	execlp("xdg-open", "xdg-open", readme_path.c_str(), NULL);
#endif
	exit(0);//exitval);
}

void ShowMain(void) {
	struct group *CURRENT;
	int count = 1;
	int column = 1;
	GtkWidget *hbox, *vbox, *label, *button;
	CURRENT = &GROUPS;
	hbox = 0;
	do {
		if (CURRENT->name == NULL) { continue; }
//		cout << count << ") " << CURRENT->name << " [" << GetInfo(CURRENT->setting) << "]\n";
		count++;
		if (column == 1) {
			hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
		}
		vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		label = gtk_label_new(CURRENT->name);
		gtk_container_add(GTK_CONTAINER(vbox), label);
		gtk_widget_show(label);
#ifndef USE_RADIO
		GtkWidget *menu=gtk_combo_box_text_new_with_entry();
		gtk_combo_box_text_prepend(GTK_COMBO_BOX_TEXT(menu), CURRENT->name, CURRENT->setting);
		AddCats(menu, CURRENT->name, CURRENT->setting);
		gtk_combo_box_set_active(GTK_COMBO_BOX(menu), 0);
		gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(menu), 0);
//		This entry would usually set the default selected item.
		/* This packs the button into the window (a gtk container). */
		catagory *NEWCUR=&CATS;
		int i=0;
		do {
//			printf(" %d",i);
			if (NEWCUR->name == NULL) { continue; }
			if (strcmp(CURRENT->name, NEWCUR->group) != 0) { continue; }
			if (strcmp(NEWCUR->name, CURRENT->setting) == 0) {
//				printf("|||%s|||",GetInfo(NEWCUR->name));
//				gtk_option_menu_set_history(GTK_OPTION_MENU(menu),i);
				break;
			}
			i++;
		} while ((NEWCUR = NEWCUR->next) != nullptr);
//		printf("\n\n");
		gtk_widget_show (menu);
		gtk_container_add(GTK_CONTAINER(vbox), menu);
#else
		AddCats(vbox, CURRENT->name, CURRENT->setting);
#endif
		gtk_container_add(GTK_CONTAINER(hbox), vbox);
		gtk_widget_show(vbox);
		if (column == CONFIG.columns) {
			column = 0;
			gtk_container_add(GTK_CONTAINER(main_vbox), hbox);
			gtk_widget_show(hbox);
		}
		column++;
	} while ((CURRENT = CURRENT->next) != nullptr);
	if (column != 1) {
		gtk_container_add(GTK_CONTAINER(main_vbox), hbox);
		gtk_widget_show(hbox);
	}
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
	button = gtk_button_new_with_label("Save Settings And View Readme");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(myexit), NULL);
	gtk_widget_show(button);
	gtk_container_add(GTK_CONTAINER(vbox), button);
	button = gtk_button_new_with_label("Save Settings and Exit");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(exit_0), NULL);
	gtk_widget_show(button);
	gtk_container_add(GTK_CONTAINER(vbox), button);
	gtk_container_add(GTK_CONTAINER(main_vbox), vbox);
	gtk_widget_show(vbox);
	gtk_main();

}


void AddCats(GtkWidget *vbox, char *group, char *def) {
	struct catagory *CUR;
#ifdef USE_RADIO
	GSList *radiogroup = NULL;
#endif
	CUR = &CATS;
	do {
		GtkWidget *button;
		if (CUR->name == NULL) { continue; }
		if (strcmp(group, CUR->group) != 0) { continue; }
#ifndef USE_RADIO
		button=gtk_menu_item_new_with_label(GetInfo(CUR->name));
		gtk_widget_show(button);
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(vbox),CUR->name, CUR->name);
			g_signal_connect(G_OBJECT(button), "activate", G_CALLBACK(ClickButton), CUR);
#else
		if (strcmp(CUR->name, def) == 0) {
			int length = strlen(GetInfo(CUR->name))+3;
			char *new_text = (char *)malloc(length+1);
			sprintf(new_text, "[%s]", GetInfo(CUR->name));
			new_text[length] = '\0';
			button = gtk_radio_button_new_with_label(radiogroup, new_text);
			free(new_text);
		}
		else {
			button=gtk_radio_button_new_with_label (radiogroup, GetInfo(CUR->name));
		}
		radiogroup = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
		gtk_widget_show (button);
		gtk_container_add(GTK_CONTAINER(vbox), button);
		if (strcmp(CUR->name, def) == 0) {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
		}
		g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(ClickButton), CUR);
#endif
		CUR->button = button;
	} while ((CUR = CUR->next) != nullptr);
}

void ClickButton(GtkWidget *w, struct catagory *CUR) {
	struct catagory *OLD;
	struct group *NEW;
	char *new_text, *old;

#ifdef USE_RADIO
	int length;
	length = strlen(GetInfo(CUR->name))+3;

	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w))) {
		// Deactivate event--we don't care.
		return;
	}
	GtkWidget *label = GTK_BIN(CUR->button)->child;
#endif


	old = GetSetting(CUR->group);
	OLD = GetCatStruct(old);
	NEW = GetGroupStruct(CUR->group);

	if (OLD == CUR) { return; }
#ifdef USE_RADIO
	new_text = (char *)malloc(length+1);
	sprintf(new_text, "[%s]", GetInfo(CUR->name));
	new_text[length] = '\0';
	gtk_label_set_text(GTK_LABEL(label), new_text);

	// Reallocate just in case the GetInfo() is smaller than the name
	free(new_text);
#endif
	new_text = (char *)malloc(strlen(CUR->name)+1);
	sprintf(new_text, "%s", CUR->name);

	NEW->setting = new_text;

#ifdef USE_RADIO
	label = GTK_BIN(OLD->button)->child;
	gtk_label_set_text(GTK_LABEL(label), GetInfo(OLD->name));
#endif
	DisableSetting(OLD->name, OLD->group);
	EnableSetting(CUR->name, CUR->group);
}
#endif
