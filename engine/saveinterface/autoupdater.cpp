/*
 * autoupdater.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
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

/* example-start entry entry.c */

#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#ifdef _WIN32
#include <direct.h>
#include <process.h>
#else
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#ifdef _WIN32
extern void GetRidOfConsole ();
extern void my_sleep (int i);
#endif
void LoadMissionDialog(char *Filename, int i);
void LoadSaveDialog(char *, int);
void LoadAutoDialog(char *, int);

std::vector<std::string> buttonvec;

int win_close(GtkWidget *w, void *) {
    return FALSE;
}

GdkWindow *Help(const char *title, const char *text) {
    GtkWidget *window;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 0);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(win_close), NULL);
    GtkWidget *label = gtk_label_new(text);
    gtk_container_add(GTK_CONTAINER(window), label);
    gtk_widget_show(label);
    gtk_widget_show(window);
    return (GdkWindow *) window;
}

bool checked = false;
bool progress = true;

void toggle_button_callback(GtkWidget *widget, gpointer data) {
    checked = (GTK_TOGGLE_BUTTON(widget)->active);
}

#define CVSROOT "-d:pserver:anonymous@cvs.vegastrike.sourceforge.net:/cvsroot/vegastrike"
volatile HANDLE hMutex; // Global hMutex Object
DWORD WINAPI
DrawStartupDialog(LPVOID
lpParameter) {
char *str = (char *) lpParameter;
progress = false;
Help ("Please Wait While Downloading Data...","Please Wait While Downloading Data...");
spawnl (P_WAIT,
"cvs","cvs","-z9","update","-kb","-d","-r",str,checked?"-C":NULL,NULL);
spawnl (P_WAIT,
"cvs","cvs","-z9","update","-kb","-d","-r",str,"-C","vegastrike.config","vegastrile.exe",NULL);
free (str);
progress = true;
g_print("\nDone updating data...\n");
spawnl (P_WAIT,
"setup","setup",NULL);
return 0;
}

void CvsUpdate(int index) {
    if (progress) {
        char *str = strdup(buttonvec[index].c_str());
        printf("cvs update -kb -r %s\n", str);
        DWORD id;
        HANDLE hThr = CreateThread(NULL, 0, DrawStartupDialog, str, 0, &id);
        fflush(stdout);
    }
}

void hello(GtkWidget *widget, gpointer data) {
    int i = (int) data;
    CvsUpdate(i);
}

std::string scanInString(FILE *fp) {
    std::string str;
    char c[2] = {'\n', '\0'};
    while ((isspace(c[0])) && (!(feof(fp)))) {
        if (1 != fscanf(fp, "%c", &c[0])) {
            break;
        }
    }
    while ((!isspace(c[0])) && (!(feof(fp)))) {
        str += c;
        if (1 != fscanf(fp, "%c", &c[0])) {
            break;
        }
    }
    return str;
}

std::vector<std::string> GetButtons() {
    std::vector<std::string> vec;
    FILE *fp = fopen("update_list.txt", "rt");

    vec.push_back("HEAD");
    if (fp) {
        while (!(feof(fp))) {
            std::string str = scanInString(fp);
            vec.push_back(str);
        }
        fclose(fp);
    } else {
        //g_error ("file not found");
    }
    return vec;
}

void main2() {
    GtkWidget *window;
    GtkWidget *button;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 350);
    gtk_window_set_title(GTK_WINDOW(window), "Get Recent Update");
    GtkWidget *check;
    check = gtk_check_button_new_with_label("Restore all files to factory defaults (will not delete saved games).");
    GtkWidget *vbox = gtk_vbox_new(FALSE, 3);
    /* When the window is given the "delete_event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(gtk_exit), NULL);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(gtk_exit), NULL);
    gtk_container_add(GTK_CONTAINER(vbox), check);
    buttonvec = GetButtons();
    button = gtk_button_new_with_label("Update to: Recent experimental data");
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
            GTK_SIGNAL_FUNC(hello), (void *) 0);
    gtk_container_add(GTK_CONTAINER(vbox), button);
    gtk_widget_show(button);
    for (int i = 1; i < buttonvec.size(); i++) {
        button = gtk_button_new_with_label((std::string("Update to: ") + buttonvec[i]).c_str());

        /* When the button receives the "clicked" signal, it will call the
         * function hello() passing it NULL as its argument.  The hello()
         * function is defined above. */
        gtk_signal_connect(GTK_OBJECT(button), "clicked",
                GTK_SIGNAL_FUNC(hello), (void *) i);
        gtk_container_add(GTK_CONTAINER(vbox), button);
        gtk_widget_show(button);
    }
    gtk_widget_show(vbox);
    gtk_widget_show(check);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(window);
    /* and the window */
}

void CvsUpdateList() {
    char *str = "HEAD";
    printf("cvs update -kb -r %s update_list.txt\n", str);
    fflush(stdout);
    spawnl(P_WAIT, "cvs", "cvs", "update", "-kb", "-r", "HEAD", "update_list.txt", NULL);
    g_print("\nDone updating list...\n");
    // spawnl (P_WAIT,"cvs","cvs","update","-kb","-r","HEAD","update_list.txt",NULL);
    // gdk_exit (0);
    main2();
}

void bCvsUpdateList(GtkWidget *widget, gpointer data) {
    GdkWindow *win;
    win = gtk_widget_get_parent_window(GTK_WIDGET(data));
    gdk_window_destroy(win);
    CvsUpdateList();
}

void bmain2(GtkWidget *widget, gpointer data) {
    GdkWindow *win;
    win = gtk_widget_get_parent_window(GTK_WIDGET(data));
    gdk_window_destroy(win);
    main2();
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);
#if defined(WITH_MACOSX_BUNDLE) || defined(_WIN32)
    // We need to set the path back 2 to make everything ok.
    char *parentdir;
    int pathlen=strlen(argv[0]);
    parentdir=new char[pathlen+1];
    char *c;
    strncpy ( parentdir, argv[0], pathlen+1 );
    c = (char*) parentdir;

    while (*c != '\0')     /* go to end */
        c++;

    while ((*c != '/')&&(*c != '\\')&&(c>parentdir))      /* back up to parent */
        c--;

    *c++ = '\0';             /* cut off last part (binary name) */
    if (strlen(parentdir)>0) {
      chdir (parentdir);/* chdir to the binary app's parent */
    }
    delete []parentdir;
#if defined(WITH_MACOSX_BUNDLE)
    chdir ("../../../");/* chdir to the .app's parent */
#endif
#endif
    GtkWidget *window;
    GtkWidget *button;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 0, 0);
    gtk_window_set_title(GTK_WINDOW(window), "Update");
    GtkWidget *hbox = gtk_hbox_new(FALSE, 3);
    GtkWidget *vbox = gtk_vbox_new(FALSE, 10);
    GtkWidget *label = gtk_label_new("     Do you wish to update the update list for more recent releases?     ");
    /* When the window is given the "delete_event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(gtk_exit), NULL);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(gtk_exit), NULL);
    button = gtk_button_new_with_label("Update");

    /* When the button receives the "clicked" signal, it will call the
     * function hello() passing it NULL as its argument.  The hello()
     * function is defined above. */
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
            GTK_SIGNAL_FUNC(bCvsUpdateList), vbox);
    gtk_container_add(GTK_CONTAINER(hbox), button);
    gtk_widget_show(button);
    button = gtk_button_new_with_label("Do Not Update");

    /* When the button receives the "clicked" signal, it will call the
     * function hello() passing it NULL as its argument.  The hello()
     * function is defined above. */
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
            GTK_SIGNAL_FUNC(bmain2), vbox);
    gtk_container_add(GTK_CONTAINER(hbox), button);
    gtk_widget_show(button);
    gtk_container_add(GTK_CONTAINER(vbox), label);
    gtk_container_add(GTK_CONTAINER(vbox), hbox);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(label);
    gtk_widget_show(vbox);
    gtk_widget_show(hbox);
    gtk_widget_show(window);
//	GetRidOfConsole();
    gtk_main();
    return (0);
}

