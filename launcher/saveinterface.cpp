/* example-start entry entry.c */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#ifdef _WIN32
#include <direct.h>
#include <process.h>
#else
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#endif
#ifdef _WIN32
extern void GetRidOfConsole ();
extern void my_sleep (int i);
#endif
void LoadMissionDialog (char * Filename,int i);
void LoadSaveDialog (char *,int);
void LoadAutoDialog (char *,int);
#define NUM_TITLES 8
static const char * titles [NUM_TITLES] = {"Select Mission", "New Game","Load Saved Game","Recover From Autosave","Launch Last Savegame", "Launch No Savegame","Help","Exit Launcher"};
std::string my_mission ("mission/exploration/explore_universe.mission");
#define NUM_HELPS 6
static const char * helps [NUM_HELPS] = {
  "|SELECT MISSION BUTTON|\nThis allows you to select which mission vegastrike\nwill start the next time you press one\nof the keys below it. Most missions do not involve\nsave games and will ignore those options,\nhowever the default, in the mission/exploration folder will\nindeed ustilize the save games you specify.\nIf you ignore this option you begin in the standard\ntrading/bounty hunting mission.",
  "|START A NEW GAME BUTTON|\nStart a new game in the Vegastrike universe.\nYou start with a dinged up old wayfarer\nand head from the vega sector with the hope of finding\nprofit and adventure on the frontier.\nTo begin afresh you must choose a new saved game.",
  "|LOAD GAME BUTTON|\nThis opens up a saved game you had finished playing before.\nTo save you must dock at the base and\nclick on the save/load button and choose the save option.",
  "|RECOVER AUTOSAVE BUTTON|\nThis button allows a player to recover their most recently\nplayed game into the selected save game upon next run.\nIf the player quits or the player docks, and then dies,\nit will restore to the last saved position.",
  "|LAUNCH LAST SAVEGAME BUTTON|\nUse this button to launch Vegastrike with from a saved\ngame or mission. If you do not choose a mission, you\nwill start in the standard trading/bounty hunting mission.",
  "|LAUNCH NO SAVEGAME BUTTON|\nThis button allows you to launch the selected\nmission without using a saved game."
};


char * prog_arg=NULL;
#ifdef _WIN32
std::string ParentDir () {
  static char * final=NULL;
  std::string mypwd;
  if (final==NULL) {
    if (prog_arg!=NULL) {
      // We need to set the path back 2 to make everything ok.
      char *parentdir;
      int pathlen=strlen(prog_arg);
      parentdir=new char[pathlen+1];
      char *c;
      parentdir[pathlen]='\0';
      strncpy ( parentdir, prog_arg, pathlen );
      c = (char*) parentdir;
      
      while (*c != '\0')     /* go to end */
        c++;
      
      while ((*c != '/')&&(*c != '\\')&&(c>parentdir))      /* back up to parent */
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
      if (mypwd.length()>0) {
        chdir (mypwd.c_str());
      }
      getcwd (final,999);
      if (strlen(parentdir)>0) {
        chdir (parentdir);
      }
      delete [] parentdir;
    }
  }else {
    mypwd = std::string(final);
  }
  return mypwd;
}
void GoToParentDir () {
  std::string par = ParentDir ();
  //  fprintf (stderr,"changing to %s",par.c_str());
  chdir (par.c_str());
}
#endif // _WIN32

int win_close( GtkWidget *w, void *)
{
    return FALSE;
}
void changehome();
GdkWindow * Help (const char *title, const char *text) {
    GtkWidget *window;
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 300,0);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(win_close), NULL);
    GtkWidget *label = gtk_label_new(text);
    gtk_container_add (GTK_CONTAINER (window), label);
    gtk_widget_show (label);
    gtk_widget_show (window);
    return (GdkWindow*)window;
}

void save_stuff(char *filename) {
  changehome();
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

#if NUM_HELPS>0
  if (i<0||i>(NUM_HELPS-1)) {
      std::string str=helps[0];
      for (int i=1;i<NUM_HELPS;i++) {
        str+=std::string("\n")+helps[i];
      }
      Help("Help",str.c_str());
    } else {
      Help(titles[i],helps[i]);
    }
#endif

}

#ifdef _WIN32
#include <windows.h>
bool progress=true;
struct stupod {
  char * my_mission;
  char * num;
  stupod (char * a, char *b) {
    my_mission=a;
    num=b;
  }
};
DWORD WINAPI DrawStartupDialog(LPVOID lpParameter) {
	stupod *s= (stupod*)lpParameter;
        progress=false;
        Help ("Please wait while vegastrike loads...","Please wait while vegastrike loads...");
        spawnl (P_WAIT,"./vegastrike","./vegastrike",s->num?s->num:(std::string("\"")+s->my_mission+"\"").c_str(),s->num?(std::string("\"")+s->my_mission+"\"").c_str():NULL,NULL);
        if (s->num)
          free (s->num);
        free (s->my_mission);
        delete (s);
        progress=true;
	return 0;
}
#endif
void launch_mission () {
#ifdef _WIN32
  if (!progress)
    return;
#endif
#ifdef _WIN32
  GoToParentDir();
#endif
  int player = my_mission.rfind ("player");
  if (player>0&&player!=std::string::npos) {
   char  num [4]={'-','m',(*(my_mission.begin()+(player-1))),'\0'};
   printf ("vegastrike %s %s",num,my_mission.c_str());
   fflush (stdout);
#ifndef _WIN32
   execlp ("vegastrike","/usr/local/bin/vegastrike",num,my_mission.c_str(),NULL);   
#else
   DWORD id;
   HANDLE hThr=CreateThread(NULL,0,DrawStartupDialog,(void *)new stupod (strdup (my_mission.c_str()),strdup (num)),0,&id);
#endif
  } else {
   printf ("vegastrike %s",my_mission.c_str());
   fflush (stdout);
#ifndef _WIN32
   execlp ("vegastrike","/usr/local/bin/vegastrike",my_mission.c_str(),NULL);   
#else
   DWORD id;
   HANDLE hThr=CreateThread(NULL,0,DrawStartupDialog,(void *)new stupod (strdup (my_mission.c_str()),NULL),0,&id);
#endif
  }
}
void file_mission_sel (GtkWidget *w, GtkFileSelection *fs) {
  std::string tmp = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
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
      save_stuff("");
      launch_mission();
      break;
    case 6:
      help_func(NULL,-1);
      break;
    case 7:
      gtk_main_quit();
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
#ifdef _WIN32
    GoToParentDir ();
#else
    getdatadir(); // Will change to the data dir which makes selecting missions easier.
#endif
    //    chdir ("./.vegastrike/save");
    gtk_init (&argc, &argv);
    GtkWidget *window;
    GtkWidget *button;
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 300,350);
    gtk_window_set_title(GTK_WINDOW(window), "Saved Game Selector");
    GtkWidget *vbox=gtk_vbox_new(FALSE, 3);
     /* When the window is given the "delete_event" signal (this is given
      * by the window manager, usually by the "close" option, or on the
      * titlebar), we ask it to call the delete_event () function
      * as defined above. The data passed to the callback
      * function is NULL and is ignored in the callback function. */
    gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(gtk_exit), NULL);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(gtk_exit), NULL);
    for (int i=0;i<NUM_TITLES;i++) {
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
//    if (default_thing[0]!='\0') {
//      gtk_file_selection_complete (GTK_FILE_SELECTION(filew), default_thing);
//    }else {
      gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), default_thing);
//    }
    GtkWidget *box=gtk_hbox_new (FALSE,0);
    gtk_container_add (GTK_CONTAINER (GTK_FILE_SELECTION(filew)->button_area),box);
    gtk_box_pack_end (GTK_BOX (GTK_FILE_SELECTION(filew)->button_area),GTK_FILE_SELECTION (filew)->help_button, TRUE, TRUE, 40);
    gtk_widget_show(box);
    gtk_widget_show(GTK_FILE_SELECTION(filew)->help_button);
    gtk_widget_show(filew);
}
void LoadMissionDialog (char * Filename,int i) {
#ifdef _WIN32
  GoToParentDir ();
#else
  getdatadir();
#endif
  chdir ("mission");
  char mypwd [1000];
  getcwd (mypwd,1000);
  //  fprintf (stderr,mypwd);
  LoadSaveFunction (Filename,i,(GtkSignalFunc) file_mission_sel,/*my_mission.c_str()*/"exploration/explore_universe.mission"/*(ParentDir()+"/mission").c_str()*/);
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


