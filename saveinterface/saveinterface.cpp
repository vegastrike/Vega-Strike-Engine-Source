/* example-start entry entry.c */

#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#ifdef _WIN32
extern void GetRidOfConsole ();
#endif
void LoadSaveDialog (char *,int);
void LoadAutoDialog (char *);

static const char * titles [] = {"New Game","Open Game","Recover From Autosave","Help Button"};
static const char * helps [] = {
  "[INSERT NEW GAME HELPXXXXXX\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX...]",
  "[INSERT OPEN GAME HELPXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX...]",
  "[INSERT AUTOSAVE GAMXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX...]"
};

int win_close( GtkWidget *w, void *)
{
    return FALSE;
}

void Help (const char *title, const char *text) {
    GtkWidget *window;
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 300,0);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(win_close), NULL);
    GtkWidget *label = gtk_label_new(text);
    gtk_container_add (GTK_CONTAINER (window), label);
    gtk_widget_show (label);
    gtk_widget_show (window);
}

void save_stuff(char *filename) {
    FILE *file=fopen("../save.txt","wt");
    if (file) {
      fprintf (file, "%s%c", filename,0);
      fclose(file);
    } else {
      Help("Error","ERROR: Unable to open ../save.txt");
    }
}

int lastSlash (const char * c) {
      int last=0;
      char temp;
      for (unsigned int i=0;(temp=c[i])!='\0';i++) {
        if (temp=='\\'||temp=='/') {
          last = i+1;
        }
      }
      return last;
}
void help_func( GtkWidget *w, int i)
{

  if (i<0||i>2) {
      Help("Help",(std::string(helps[0])+"\n"+helps[1]+"\n"+helps[2]).c_str());
    } else {
      Help(titles[i],helps[i]);
    }

}
void file_ok_sel( GtkWidget        *w,
                  GtkFileSelection *fs )
{
    if ((gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)))[0]!='\0') {
      save_stuff(gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs))+lastSlash(gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs))));
    }
    //    gtk_main_quit ();
}
void file_ok_auto_sel( GtkWidget        *w,
                  GtkFileSelection *fs )
{
    if ((gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)))[0]!='\0') {
      char *name=new char [strlen(gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)))+2];
      strcpy(name+1,gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs))+lastSlash (gtk_file_selection_get_filename (GTK_FILE_SELECTION(fs))));
      name[0]='~';
      save_stuff(name);
    }
    //    gtk_main_quit ();
}

void hello( GtkWidget *widget, gpointer   data ) {
    int i=(int)data;
    switch (i) {
    case 0:
      LoadSaveDialog("New Game",0);
      break;
    case 1:
      LoadSaveDialog("Open Game",1);
      break;
    case 2:
      LoadAutoDialog("Open Autosave Game");
      break;
    case 3:
      help_func(NULL,-1);
      break;
    default:
      printf ("\nERROR...");
      gtk_main_quit();
      break;
    }
}

int main( int   argc,
          char *argv[] )
{
#if defined(WITH_MACOSX_BUNDLE)||defined(_WIN32)
    // We need to set the path back 2 to make everything ok.
    char *parentdir;
	int pathlen=strlen(argv[0]);
	parentdir=new char[pathlen+1];
    char *c;
    strncpy ( parentdir, argv[0], pathlen+1 );
    c = (char*) parentdir;

    while (*c != '\0')     /* go to end */
        c++;
    
    while ((*c != '/')&&(*c != '\\'))      /* back up to parent */
        c--;
    
    *c++ = '\0';             /* cut off last part (binary name) */
  
    chdir (parentdir);/* chdir to the binary app's parent */
	delete []parentdir;
#if defined(WITH_MACOSX_BUNDLE)
    chdir ("../../../");/* chdir to the .app's parent */
#endif
#endif
    printf ("\n\n\n\n\n\n\nLoading...");
    chdir ("./.vegastrike/save");
    gtk_init (&argc, &argv);
    GtkWidget *window;
    GtkWidget *button;
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 300,200);
    gtk_window_set_title(GTK_WINDOW(window), "Saved Game Selector");
    GtkWidget *vbox=gtk_vbox_new(FALSE, 3);
     /* When the window is given the "delete_event" signal (this is given
      * by the window manager, usually by the "close" option, or on the
      * titlebar), we ask it to call the delete_event () function
      * as defined above. The data passed to the callback
      * function is NULL and is ignored in the callback function. */
    gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(gtk_exit), NULL);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(gtk_exit), NULL);
    for (int i=0;i<4;i++) {
        button = gtk_button_new_with_label (titles[i]);
         
         /* When the button receives the "clicked" signal, it will call the
          * function hello() passing it NULL as its argument.  The hello()
          * function is defined above. */
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (hello), (void*)i);
        gtk_container_add (GTK_CONTAINER (vbox), button);
        gtk_widget_show (button);
    }
    gtk_widget_show (vbox);
    gtk_container_add (GTK_CONTAINER (window), vbox);
         
         /* and the window */
         gtk_widget_show (window);
#ifdef _WIN32
	GetRidOfConsole();
#else
	printf ("                      Done");
#endif
    gtk_main();
    return(0);
}

void LoadSaveFunction (char *Filename, int i, GtkSignalFunc func) {
     GtkWidget *filew;
    filew = gtk_file_selection_new (Filename);
    
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
                        "clicked", (GtkSignalFunc) func, filew );
    GTK_FILE_SELECTION(filew)->help_button=gtk_button_new_with_label ("Help");
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->help_button),
                        "clicked", (GtkSignalFunc) help_func, (void*)i );

    gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
                                            (filew)->cancel_button),
                               "clicked", (GtkSignalFunc) gtk_widget_destroy,
                               GTK_OBJECT (filew));
    
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), "");
    
    gtk_widget_show(filew);
    gtk_container_add (GTK_CONTAINER (GTK_FILE_SELECTION(filew)->button_area),GTK_FILE_SELECTION (filew)->help_button);
    gtk_widget_show(GTK_FILE_SELECTION(filew)->help_button);
}
void LoadSaveDialog (char *Filename,int i) {
  LoadSaveFunction (Filename,i,(GtkSignalFunc) file_ok_sel);
}
void LoadAutoDialog (char *Filename) {
  LoadSaveFunction (Filename,2,(GtkSignalFunc)file_ok_auto_sel);
}
/* example-end */

