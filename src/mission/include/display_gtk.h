/***************************************************************************
 *                           display_gtk.h  -  description
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

// Check to see if we're already loaded
#ifndef DISPLAY_GTK_H
#define DISPLAY_GTK_H

#include "central.h"

void InitGraphics();
void ShowMain(int run_vegastrike);

void AddMainLabels(void);
void AddLabelRow(char *desc, int count, GtkWidget *vbox);
void AddMissionsToTree(char *path, GtkWidget *tree, int is_parent);
GtkWidget *AddItem(GtkWidget *tree, char *name, char *filename);

void select_item(GtkWidget *item, gchar *filename);
void deselect_item(GtkWidget *item, gchar *filename);

void cb_unselect_child(GtkWidget *root_tree, GtkWidget *child, GtkWidget *subtree);
void cb_select_child (GtkWidget *root_tree, GtkWidget *child, GtkWidget *subtree);
void cb_selection_changed(GtkWidget *tree);

#endif    //DISPLAY_GTK_H
