/*
 * Copyright (C) 2001-2023 Daniel Horn, David Ranger, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
#ifndef VEGA_STRIKE_ENGINE_MISSION_DISPLAY_GTK_H
#define VEGA_STRIKE_ENGINE_MISSION_DISPLAY_GTK_H

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
void cb_select_child(GtkWidget *root_tree, GtkWidget *child, GtkWidget *subtree);
void cb_selection_changed(GtkWidget *tree);

#endif //VEGA_STRIKE_ENGINE_MISSION_DISPLAY_GTK_H
