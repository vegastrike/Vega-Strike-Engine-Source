/* example-start entry entry.c */

#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#ifdef _WIN32
#include <direct.h>
#else
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#ifdef _WIN32
extern void GetRidOfConsole ();
#endif
void LoadMissionDialog (char * Filename,int i);
void LoadSaveDialog (char *,int);
void LoadAutoDialog (char *,int);

static const char * titles [] = {"Select Mission", "New Game","Open Game","Recover From Autosave","Launch No Savegame","Help Button"};
string my_mission ("mission/exploration/explore_universe.mission");
static const char * helps [] = {
  "[SELECT MISSIONXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX...]",
  "[INSERT NEW GAME HELPXXXXXX\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX...]",
  "[INSERT OPEN GAME HELPXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX...]",
"[INSERT AUTOSAVE GAMXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX...]"
};


char * prog_arg=NULL;
std::string ParentDir () {
  static char * final=NULL;
  string mypwd;
  if (final==NULL) {
    if (prog_arg!=NULL) {
      // We need to set the path back 2 to make everything ok.
      char *parentdir;
      int pathlen=strlen(prog_arg);
      parentdir=new char[pathlen+1];
      char *c;
      strncpy ( parentdir, prog_arg, pathlen+1 );
      c = (char*) parentdir;
      
      while (*c != '\0')     /* go to end */
        c++;
      
      while ((*c != '/')&&(*c != '\\'))      /* back up to parent */
        c--;
      if (c>parentdir+1) {
	if (*(c-1)=='.'&&*(c-2)=='/') {
	  c-=2;
	}
      }
      *c = '\0';             /* cut off last part (binary name) */
      
      mypwd = parentdir;
      delete []parentdir;
      final = (char *)malloc(1000);
      final[999]='\0';
      parentdir = new char [1000];
      parentdir[999]='\0';
      getcwd (parentdir,999);
      chdir (mypwd.c_str());
      getcwd (final,999);
      chdir (parentdir);
      delete [] parentdir;
    }
  }else {
    mypwd = string(final);
  }
  return mypwd;
}
void GoToParentDir () {
  string par = ParentDir ();
  //  fprintf (stderr,"changing to %s",par.c_str());
  chdir (par.c_str());
}
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
      //      fprintf (file, "%s%c", filename,0);
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

  if (i<0||i>3) {
      Help("Help",(std::string(helps[0])+"\n"+helps[1]+"\n"+helps[2]).c_str());
    } else {
      Help(titles[i],helps[i]);
    }

}
void launch_mission () {
  GoToParentDir();
  int player = my_mission.find ("player");
  if (player>0&&player!=string::npos) {
   char  num [4]={'-','m',(*(my_mission.begin()+(player-1))),'\0'};
   printf ("./vegastrike %s %s",num,my_mission.c_str());
   fflush (stdout);
   execlp ("./vegastrike","./vegastrike",num,my_mission.c_str(),NULL);   
  } else {
   printf ("./vegastrike %s",my_mission.c_str());
   fflush (stdout);
    execlp ("./vegastrike","./vegastrike",my_mission.c_str(),NULL);
  }
}
void file_mission_sel (GtkWidget *w, GtkFileSelection *fs) {
  string tmp = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  FILE * fp =(fopen (tmp.c_str(),"r"));
  if (fp!=NULL) {
    fclose (fp);
    my_mission=tmp;
  }
  GdkWindow * ww=gtk_widget_get_parent_window(w);
  gdk_window_destroy(ww);
}
void file_ok_sel( GtkWidget        *w,
                  GtkFileSelection *fs )
{
    if ((gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)))[0]!='\0') {
      save_stuff(gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs))+lastSlash(gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs))));
    }
    launch_mission();
    GdkWindow * ww=gtk_widget_get_parent_window(w);
    gdk_window_destroy(ww);
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
    launch_mission();
    GdkWindow * ww = gtk_widget_get_parent_window(w);
    gdk_window_destroy(ww);

    //    gtk_main_quit ();
}

void hello( GtkWidget *widget, gpointer   data ) {
    int i=(int)data;
    switch (i) {
    case 0:
      LoadMissionDialog("Select Mission",i);
      break;
    case 1:
      LoadSaveDialog("New Game",i);
      break;
    case 2:
      LoadSaveDialog("Open Game",i);
      break;
    case 3:
      LoadAutoDialog("Open Autosave Game",i);
      break;
    case 4:
      launch_mission();
      break;
    case 5:
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


    prog_arg = argv[0];
    GoToParentDir ();
    //    chdir ("./.vegastrike/save");
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
    for (int i=0;i<6;i++) {
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
	printf (my_mission.c_str());
#endif
    gtk_main();
    return(0);
}

void LoadSaveFunction (char *Filename, int i, GtkSignalFunc func,const char * default_thing="") {
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
    if (default_thing[0]!='\0') {
      gtk_file_selection_complete (GTK_FILE_SELECTION(filew), default_thing);
    }else {
      gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), default_thing);
    }
    gtk_widget_show(filew);
    gtk_container_add (GTK_CONTAINER (GTK_FILE_SELECTION(filew)->button_area),GTK_FILE_SELECTION (filew)->help_button);
    gtk_widget_show(GTK_FILE_SELECTION(filew)->help_button);
}
void LoadMissionDialog (char * Filename,int i) {
  GoToParentDir ();
  chdir ("mission");
  char mypwd [1000];
  getcwd (mypwd,1000);
  //  fprintf (stderr,mypwd);
  LoadSaveFunction (Filename,i,(GtkSignalFunc) file_mission_sel,/*my_mission.c_str()*/""/*(ParentDir()+"/mission").c_str()*/);
}
#define HOMESUBDIR ".vegastrike"
void changehome() {
  static char pw_dir[2000];
#ifndef _WIN32
  struct passwd *pwent;
  pwent = getpwuid (getuid());
  chdir (pwent->pw_dir);
#else
  GoToParentDir ();
#endif
  if (chdir (HOMESUBDIR)==-1) {
    mkdir (HOMESUBDIR
#ifndef _WIN32		  
	     , 0xFFFFFFFF
#endif		  
	     );
    chdir (HOMESUBDIR);
  }
  if (chdir ("save")==-1) {
    mkdir ("save"
#ifndef _WIN32
	   , 0xFFFFFFFF
#endif		  
	   );
    //system ("mkdir " HOMESUBDIR "/generatedbsp");
    chdir ("save");
  }
}


void LoadSaveDialog (char *Filename,int i) {
  changehome();
  LoadSaveFunction (Filename,i,(GtkSignalFunc) file_ok_sel);
}
void LoadAutoDialog (char *Filename, int i) {
  changehome();
  LoadSaveFunction (Filename,i,(GtkSignalFunc)file_ok_auto_sel);
}
/* example-end */

