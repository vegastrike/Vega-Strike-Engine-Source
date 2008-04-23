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

	gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(exit_0), NULL);
	gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(exit_0), NULL);

	gtk_container_set_border_width(GTK_CONTAINER(window), 0);

	main_vbox = gtk_vbox_new(FALSE, 1);
	gtk_container_border_width(GTK_CONTAINER(main_vbox), 1);
	gtk_container_add(GTK_CONTAINER(window), main_vbox);

        GtkWidget *lbl=gtk_label_new(static_text);
	gtk_container_add(GTK_CONTAINER(main_vbox),lbl);
	gtk_widget_show(lbl);
	gtk_widget_show(main_vbox);
	gtk_widget_realize(window);

	gtk_widget_show(window);
}
void myexit(int exitval){
#ifdef _WIN32
	int err=(int)ShellExecute(NULL,"open","documentation\\readme.txt","","",1);
#else
	execlp("less", "less","readme.txt", NULL); //Will this work in Linux?
#endif
	exit(0);//exitval);
}

void ShowMain(void) {
	struct group *CURRENT;
	struct catagory *CUR;
	int count = 1;
	int column = 1;
	GtkWidget *hbox, *vbox, *label, *button;
	CURRENT = &GROUPS;
	CUR = &CATS;
	hbox = 0;
	do {
		if (CURRENT->name == NULL) { continue; }
//		cout << count << ") " << CURRENT->name << " [" << GetInfo(CURRENT->setting) << "]\n";
		count++;
		if (column == 1) {
			hbox = gtk_hbox_new(FALSE, 2);
		}
		vbox = gtk_vbox_new(FALSE, 0);
		label = gtk_label_new(CURRENT->name);
		gtk_container_add(GTK_CONTAINER(vbox), label);
		gtk_widget_show(label);
#ifndef USE_RADIO
		GtkWidget *menu=gtk_option_menu_new();
		GtkWidget *my_menu=gtk_menu_new();
		AddCats(my_menu, CURRENT->name, CURRENT->setting);
		gtk_option_menu_set_menu(GTK_OPTION_MENU(menu),my_menu);
		/* This packs the button into the window (a gtk container). */
		catagory *NEWCUR=&CATS;
		int i=0;
		do {
//			printf(" %d",i);
			if (NEWCUR->name == NULL) { continue; }
			if (strcmp(CURRENT->name, NEWCUR->group) != 0) { continue; }
			if (strcmp(NEWCUR->name, CURRENT->setting) == 0) {
//				printf("|||%s|||",GetInfo(NEWCUR->name));
				gtk_option_menu_set_history(GTK_OPTION_MENU(menu),i);
				break;
			}
			i++;
		} while ((NEWCUR = NEWCUR->next) > 0);
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
	} while ((CURRENT = CURRENT->next) > 0);
	if (column != 1) {
		gtk_container_add(GTK_CONTAINER(main_vbox), hbox);
		gtk_widget_show(hbox);
	}
	vbox = gtk_vbox_new(FALSE, 2);
	button = gtk_button_new_with_label("Save Settings And View Readme");
	gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(myexit), NULL);
	gtk_widget_show(button);
	gtk_container_add(GTK_CONTAINER(vbox), button);
	button = gtk_button_new_with_label("Save Settings and Exit");
	gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(exit_0), NULL);
	gtk_widget_show(button);
	gtk_container_add(GTK_CONTAINER(vbox), button);
	gtk_container_add(GTK_CONTAINER(main_vbox), vbox);
	gtk_widget_show(vbox);
	gtk_main();

}


void AddCats(GtkWidget *vbox, char *group, char *def) {
	struct catagory *CUR;
	GSList *radiogroup = NULL;
	CUR = &CATS;
	do {
		GtkWidget *button;
		if (CUR->name == NULL) { continue; }
		if (strcmp(group, CUR->group) != 0) { continue; }
#ifndef USE_RADIO
		button=gtk_menu_item_new_with_label(GetInfo(CUR->name));
		gtk_widget_show(button);
		gtk_menu_append(GTK_MENU(vbox),button);
		gtk_signal_connect(GTK_OBJECT(button), "activate", GTK_SIGNAL_FUNC(ClickButton), CUR);
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
		gtk_signal_connect(GTK_OBJECT(button), "toggled", GTK_SIGNAL_FUNC(ClickButton), CUR);
//		gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(ClickButton), CUR);
#endif
		CUR->button = button;
	} while ((CUR = CUR->next) > 0);
}

void ClickButton(GtkWidget *w, struct catagory *CUR) {
	struct catagory *OLD;
	struct group *NEW;
	char *new_text, *old;
	int length;
	length = strlen(GetInfo(CUR->name))+3;

#ifdef USE_RADIO
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
