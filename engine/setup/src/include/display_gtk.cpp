/*
 * display_gtk.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; David Ranger specifically
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

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

void InitGraphics(int *argc, char ***argv) {
    gtk_init(argc, argv);
    GET_TITLE; // sets title; uses sprintf, not snprintf -- //[MSVC-Warn]
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

    GtkWidget *lbl = gtk_label_new(static_text);
    gtk_container_add(GTK_CONTAINER(main_vbox), lbl);
    gtk_widget_show(lbl);
    gtk_widget_show(main_vbox);
    gtk_widget_realize(window);

    gtk_widget_show(window);
}

void myexit(int exitval) {
    std::string readme_path(CONFIG.data_path);

#ifdef _WIN32
    readme_path += "\\documentation\\readme.txt";
    int err=(int)ShellExecute(NULL,"open",readme_path.c_str(),"","",1); //[MSVC-Warn] err should not be an int, should be an HINSTANCE... but this is also immediately quitting and doesn't use err- fix it in another patch
#else
    readme_path += "/documentation/readme.txt";
    execlp("xdg-open", "xdg-open", readme_path.c_str(), NULL); //Will this work in Linux?
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
        if (CURRENT->name == NULL) {
            continue;
        }
//		BOOST_LOG_TRIVIAL(trace) << count << ") " << CURRENT->name << " [" << GetInfo(CURRENT->setting) << "]\n";
        count++;
        if (column == 1) {
            hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
        }
        vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        label = gtk_label_new(CURRENT->name);
        gtk_container_add(GTK_CONTAINER(vbox), label);
        gtk_widget_show(label);
#ifndef USE_RADIO
        GtkWidget *menu = gtk_combo_box_text_new_with_entry();
        AddCats(menu, CURRENT->name, CURRENT->setting);
        /* This packs the button into the window (a gtk container). */
        catagory *NEWCUR = &CATS;
        int i = 0;
        do {
//			printf(" %d",i);
            if (NEWCUR->name == NULL) {
                continue;
            }
            if (strcmp(CURRENT->name, NEWCUR->group) != 0) {
                continue;
            }
            if (strcmp(NEWCUR->name, CURRENT->setting) == 0) {
                break;
            }
            i++;
        } while ((NEWCUR = NEWCUR->next) != nullptr);
//		printf("\n\n");
        gtk_widget_show(menu);
        gtk_container_add(GTK_CONTAINER(vbox), menu);
        g_signal_connect(G_OBJECT(menu), "changed", G_CALLBACK(ClickButton), CURRENT);
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
    button = gtk_button_new_with_label("Save Settings and View Readme");
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
    char *info = NULL;
#ifdef USE_RADIO
    GSList *radiogroup = NULL;
#endif
    CUR = &CATS;
    do {
        if (CUR->name == NULL) {
            continue;
        }
        if (strcmp(group, CUR->group) != 0) {
            continue;
        }
        if (CUR->info == NULL) {
            info = CUR->name;
        } else {
            info = CUR->info;
        }
#ifndef USE_RADIO
//		button=gtk_menu_item_new_with_label(GetInfo(info));
        //gtk_widget_show(button);
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(vbox), CUR->name, info);
        if (strcmp(CUR->name, def) == 0) {
            gtk_combo_box_set_active_id(GTK_COMBO_BOX(vbox), CUR->name);
        }
#else
        GtkWidget *button;
        if (strcmp(CUR->name, def) == 0) {
            int length = strlen(GetInfo(CUR->name))+3;
            char *new_text = (char *)malloc(length+1);
            sprintf(new_text, "[%s]", GetInfo(info));
            new_text[length] = '\0';
            button = gtk_radio_button_new_with_label(radiogroup, new_text);
            free(new_text);
        }
        else {
            button=gtk_radio_button_new_with_label (radiogroup, GetInfo(info));
        }
        radiogroup = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
        gtk_widget_show (button);
        gtk_container_add(GTK_CONTAINER(vbox), button);
        if (strcmp(CUR->name, def) == 0) {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (vbox), TRUE);
        }
        g_signal_connect(G_OBJECT(vbox), "toggled", G_CALLBACK(ClickButton), CUR);
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(ClickButton), CUR);
        CUR->button = button;
#endif
    } while ((CUR = CUR->next) != nullptr);
}

void ClickButton(GtkWidget *w, struct catagory *CUR) {
    struct catagory *OLD;
    struct group *NEW;
    char *new_value, *new_text, *old;

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

    if (OLD == CUR) {
        return;
    }
#ifdef USE_RADIO
    new_text = (char *)malloc(length+1);
    sprintf(new_text, "[%s]", GetInfo(CUR->name));
    new_text[length] = '\0';
    gtk_label_set_text(GTK_LABEL(label), new_text);

    // Reallocate just in case the GetInfo() is smaller than the name
    free(new_text);
#endif

    new_value = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(w));
    new_text = (char *) malloc(strlen(CUR->name) + 1);
    if (NULL == new_text) {
        fprintf(stderr, "FAILED TO MALLOC A SMALL STRING!\n");
        exit(-1);
    }
    sprintf(new_text, "%s", GetNameFromInfo(new_value)->name); //[MSVC-Warn] sprintf instead of snprintf
    NEW->setting = new_text;

#ifdef USE_RADIO
    label = GTK_BIN(OLD->button)->child;
    gtk_label_set_text(GTK_LABEL(label), GetInfo(OLD->name));
#endif
    DisableSetting(OLD->name, OLD->group);
    EnableSetting(CUR->name, CUR->group);
}

#endif
