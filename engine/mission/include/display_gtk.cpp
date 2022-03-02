/*
 * Copyright (C) 2001-2022 Daniel Horn, David Ranger, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#if defined(_WIN32) && _MSC_VER > 1300
#define __restrict
#endif
#include "display_gtk.h"

GtkWidget *window, *main_vbox, *scrolled_win, *tree, *main_hbox;
GtkWidget *labels[4];    // Array of changable lables to display the mission information

void InitGraphics(void) {
    char title[1024];
    GtkWidget *label;

    /* Create a new GTK window */
    sprintf(title, "Mission Selector - %s - Version %s", MISSION_PROGRAM, MISSION_VERSION);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    gtk_window_set_title(GTK_WINDOW(window), title);

    gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(gtk_exit), NULL);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(gtk_exit), NULL);

    gtk_container_set_border_width(GTK_CONTAINER(window), 0);

    /* The main vertical box to store the header, the horizontal box (mission tree and description) and the bottom buttons) */
    main_vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_border_width(GTK_CONTAINER(main_vbox), 1);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    /* A label with the program name at the top */
    sprintf(title, "%s Mission Selector %s", MISSION_PROGRAM, MISSION_VERSION);
    label = gtk_label_new(title);
    gtk_container_add(GTK_CONTAINER(main_vbox), label);
    gtk_widget_show(label);

    /* A horizontal box to store the mission tree and the description labels */
    main_hbox = gtk_hbox_new(FALSE, 1);
    gtk_container_add(GTK_CONTAINER(main_vbox), main_hbox);
    gtk_widget_show(main_hbox);

    gtk_widget_show(main_vbox);
    gtk_widget_realize(window);

    gtk_widget_show(window);
}

void ShowMain(int run_vegastrike) {
    GtkWidget *button;
    /* A generic scrolled window */
    scrolled_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_usize(scrolled_win, 150, 200);
    gtk_container_add(GTK_CONTAINER(main_hbox), scrolled_win);
    gtk_widget_show(scrolled_win);

    /* Create the root tree */
    tree = gtk_tree_new();

    /* connect all GtkTree:: signals */
    gtk_signal_connect(GTK_OBJECT(tree), "select_child", GTK_SIGNAL_FUNC(cb_select_child), tree);
    gtk_signal_connect(GTK_OBJECT(tree), "unselect_child", GTK_SIGNAL_FUNC(cb_unselect_child), tree);
    gtk_signal_connect(GTK_OBJECT(tree), "selection_changed", GTK_SIGNAL_FUNC(cb_selection_changed), tree);

    /* Add it to the scrolled window */
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_win), tree);

    /* Set the selection mode */
    gtk_tree_set_selection_mode(GTK_TREE(tree), GTK_SELECTION_SINGLE);

    /* Show it */
    gtk_widget_show(tree);

    AddMainLabels();

    AddMissionsToTree(DIR_MISSION, tree, 1);

    button = gtk_button_new_with_label("Run Mission");
    gtk_signal_connect(GTK_OBJECT(button), "clicked", RunMission, NULL);
    gtk_container_add(GTK_CONTAINER(main_vbox), button);
    gtk_widget_show(button);
}

void AddMainLabels(void) {
    GtkWidget *vbox;

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_hbox), vbox);
    gtk_widget_show(vbox);

    /* Adds a new row (2 labels, 1 static, 1 changable) to display the information */
    AddLabelRow("Mission Name: ", 0, vbox);
    AddLabelRow("Author: ", 1, vbox);
    AddLabelRow("Description: ", 2, vbox);
    AddLabelRow("Briefing: ", 3, vbox);
}

void AddLabelRow(char *desc, int count, GtkWidget *vbox) {
    GtkWidget *hbox, *label;

    hbox = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(vbox), hbox);
    gtk_widget_show(hbox);

    label = gtk_label_new(desc);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(label), 2, 2);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    gtk_container_add(GTK_CONTAINER(hbox), label);
    gtk_widget_show(label);

    labels[count] = gtk_label_new("");
    gtk_misc_set_alignment(GTK_MISC(labels[count]), 0, 0.5);
    gtk_label_set_justify(GTK_LABEL(labels[count]), GTK_JUSTIFY_LEFT);
    gtk_container_add(GTK_CONTAINER(hbox), labels[count]);
    gtk_widget_show(labels[count]);
}

/* Things to add:
 *   Check sub directories to see if there are files before putting it in the list
 *   Chcek sub_dir/sub_dir.campaign for campaign info
 */

void AddMissionsToTree(char *path, GtkWidget *tree, int is_parent) {
    glob_t *search;
    unsigned int length;
    int count, max;
    char *file, *filename;
    GtkWidget *subtree, *item;

    // First we check for sub directories. stick them at the top
    // For some reason, glob(,,GLOB_ONLYDIR,) doesn't seem to only match directories,
    // so FindDirs() currently returns everything. Check the last char for a /
    // That will be the directory.

    search = FindDirs(path);
    max = search->gl_pathc - 1;    // search->gl_pathc is a uint. If there's no files, it's 0.
    for (count = 0; count <= max; count++) {
        file = search->gl_pathv[count];
        length = strlen(file);
        if (file[length - 1] != SEPERATOR) {
            continue;
        }    // Verify it's a directory and not a file

        filename = _strdup(file);
        filename = StripPath(filename);
        if (strcmp("CVS", filename) == 0) {
            continue;
        }    // Don't need to display this directory

        item = AddItem(tree, filename, "dir");

        subtree = gtk_tree_new();
        gtk_signal_connect(GTK_OBJECT(subtree), "select_child", GTK_SIGNAL_FUNC(cb_select_child), subtree);
        gtk_signal_connect(GTK_OBJECT(subtree), "unselect_child", GTK_SIGNAL_FUNC(cb_unselect_child), subtree);

        gtk_tree_set_selection_mode(GTK_TREE(subtree), GTK_SELECTION_SINGLE);
        gtk_tree_set_view_mode(GTK_TREE(subtree), GTK_TREE_VIEW_ITEM);
        gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), subtree);

        AddMissionsToTree(file, subtree, 0);

    }

    search = FindFiles(path, EXT_MISSION);
    max = search->gl_pathc - 1;
    for (count = 0; count <= max; count++) {
        file = search->gl_pathv[count];
        length = strlen(file);
        if (file[length - 1] == SEPERATOR) {
            continue;
        }

        filename = _strdup(file);
        filename = StripPath(filename);
        StripExtension(filename);

        AddItem(tree, filename, file);
    }
    return;
}

GtkWidget *AddItem(GtkWidget *tree, char *name, char *filename) {
    GtkWidget *item;

    item = gtk_tree_item_new_with_label(name);
    gtk_signal_connect(GTK_OBJECT(item), "select", GTK_SIGNAL_FUNC(select_item), filename);
    gtk_signal_connect(GTK_OBJECT(item), "deselect", GTK_SIGNAL_FUNC(deselect_item), filename);

    // You can also add the following signals: toggle, expand, collapse

    gtk_tree_append(GTK_TREE(tree), item);
    gtk_widget_show(item);

    return item;
}

/* Things to do:
 *   Struct elements can be handled with pointers. Use that to make it more dynamic
 */

void select_item(GtkWidget *item, gchar *filename) {
    if (strcmp("dir", filename) == 0) {
        deselect_item(item, filename);
        return;
    }
    LoadMission(filename);

    gtk_label_set_text(GTK_LABEL(labels[0]), DATA.name);
    gtk_label_set_text(GTK_LABEL(labels[1]), DATA.author);
    gtk_label_set_text(GTK_LABEL(labels[2]), DATA.description);
    gtk_label_set_text(GTK_LABEL(labels[3]), DATA.briefing);
}

void deselect_item(GtkWidget *item, gchar *filename) {
}

/* These would be useful for campaigns. Don't have anything for them now */
void cb_unselect_child(GtkWidget *root_tree, GtkWidget *child, GtkWidget *subtree) {

}

void cb_select_child(GtkWidget *root_tree, GtkWidget *child, GtkWidget *subtree) {

}

void cb_selection_changed(GtkWidget *tree) {

}
