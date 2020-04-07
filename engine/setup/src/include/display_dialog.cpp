#include "display.h"

#ifdef CONSOLE
#include <dialog.h>

//#define COLS 80
//#define ROWS 24

#define WIDTH (COLS-20)
#define HEIGHT (LINES-6)
#define LIST_HEIGHT (HEIGHT-6)

void InitGraphics (int* argc,char*** argv){
  init_dialog (stdin, stdout);
}

int getLength(char **list) {
  int count;
  for (count=0;
       list[0]!=NULL;
       list++,
         count++){

  }
  return count;
}

int ShowMainMenu(char ** menuitem_list, char * def, char *retbuffer) {
  GET_TITLE;
  char static_text[]="";
  memset(&dialog_vars, 0, sizeof(dialog_vars));
  memset(&dialog_state,0, sizeof(dialog_state));
  dialog_vars.input_result = retbuffer;
  dialog_vars.input_result[0] = '\0';
  dialog_state.use_shadow=TRUE;
  dialog_vars.default_item=def;
  dialog_vars.ok_label="Edit Option";
  dialog_vars.cancel_label="Save and Exit";
  dialog_vars.title=title;
  dialog_vars.backtitle="Vega Strike Configurator";
  dlg_put_backtitle();
  int retval=dialog_menu(title,static_text,HEIGHT,WIDTH,LIST_HEIGHT,getLength(menuitem_list)/2,menuitem_list);
  return retval;
}

int ShowSubMenu(char ** menuitem_list, char * def, char *retbuffer, const char *name) {
  char * title="Set Option -";
  char static_text[]="";
  memset(&dialog_vars, 0, sizeof(dialog_vars));
  memset(&dialog_state,0, sizeof(dialog_state));
  dialog_vars.input_result = retbuffer;
  dialog_vars.input_result[0] = '\0';
  dialog_state.use_shadow=TRUE;
  dialog_vars.default_item=def;
  dialog_vars.title=title;
  dialog_vars.backtitle="Vega Strike Configurator";
  char *subtitle= new char[strlen(title)+2+strlen(name)];
  sprintf(subtitle, "%s %s", title, name);
  dlg_put_backtitle();
  int retval=dialog_menu(subtitle,static_text,HEIGHT,WIDTH,LIST_HEIGHT,getLength(menuitem_list)/2,menuitem_list);
  delete []subtitle;
  return retval;
}

void SetOption(struct group *CURRENT, char *selectedstr) {
  struct catagory *OLD=&CATS;
  struct catagory *NEW=&CATS;
  do {
    if (!OLD->name) { continue; }
    if (strcmp(OLD->group,CURRENT->name)!=0) { continue; }
    if (strcmp(OLD->name,CURRENT->setting)==0) break;
  } while((OLD=OLD->next) != NULL);
  if (!OLD) return;
  do {
    if (!NEW->name) { continue; }
    if (strcmp(NEW->group,CURRENT->name)!=0) { continue; }
    if (strcmp(GetInfo(NEW->name),selectedstr)==0) break;
  } while((NEW=NEW->next) != NULL);
  if (!NEW) return;

  if (OLD == NEW) { return; }

  CURRENT->setting = strdup(NEW->name);

  DisableSetting(OLD->name, OLD->group);
  EnableSetting(NEW->name, NEW->group);
}


void ShowMain() {
  static char *ON=" <-- ";
  static char *OFF="     ";
  char selectedstr[1000];
  int selected=0;
  int subselected=0;
  int ret=0;
  char ** menuitem_list;
  char *** group_options_list;
  char ** menu_titles;
  struct group *CURRENT;
  struct catagory *CUR;
  int count = 1;
  CURRENT = &GROUPS;
  do {
    if (CURRENT->name == NULL) { continue; }
    ++count;
  } while ((CURRENT = CURRENT->next) != NULL);
  menuitem_list=new char*[count*2+1];
  group_options_list=new char**[count+1];
  menu_titles=new char*[count+1];
  memset(menuitem_list,0,sizeof(char*)*count*2+1);
  memset(group_options_list,0,sizeof(char**)*(count+1));
  memset(menu_titles,0,sizeof(char*)*(count+1));
  CURRENT = &GROUPS;
  count=0;
  do {
    if (CURRENT->name == NULL) { continue; }
    menuitem_list[count*2]=CURRENT->name;
    menuitem_list[count*2+1]=GetInfo(CURRENT->setting);
    /* This packs the button into the window (a gtk container). */
    CUR=&CATS;
    int i=0;
    do {
      if (CUR->name == NULL) { continue; }
      if (strcmp(CURRENT->name, CUR->group) != 0) { continue; }
      i++;
    } while ((CUR = CUR->next) != NULL);
    group_options_list[count]=new char*[i*2+1];
    memset(group_options_list[count],0,sizeof(char*)*(i*2+1));
    CUR=&CATS;
    i=0;
    do {
      if (CUR->name == NULL) { continue; }
      if (strcmp(CURRENT->name, CUR->group) != 0) { continue; }
      group_options_list[count][i*2]=GetInfo(CUR->name);
      group_options_list[count][i*2+1]=NULL;
      i++;
    } while ((CUR = CUR->next) != NULL);
    count++;
  } while ((CURRENT = CURRENT->next) != NULL);
  while (true) {
    ret=ShowMainMenu(menuitem_list, menuitem_list[selected*2], selectedstr);
    if (ret==DLG_EXIT_CANCEL||ret==DLG_EXIT_ESC) {
      break;
    }
    if (ret==DLG_EXIT_ERROR||ret==DLG_EXIT_UNKNOWN) {
      fprintf(stderr,"Main dialog error...\n");
      break;
    }
    selected=0;
    subselected=0;
    CURRENT = &GROUPS;
    do {
      if (CURRENT->name == NULL) { continue; }
      if (strcmp(CURRENT->name,selectedstr)==0) break;
      ++selected;
    } while ((CURRENT = CURRENT->next) != NULL);
    CUR = &CATS;
    int i=0;
    do {
      if (CUR->name == NULL) { continue; }
      if (strcmp(CURRENT->name, CUR->group) != 0) { continue; }
      if (strcmp(CUR->name, CURRENT->setting) == 0) {
        group_options_list[selected][i*2+1]=strdup(ON);
        subselected=i;
      } else {
        group_options_list[selected][i*2+1]=strdup(OFF);
      }
      i++;
    } while ((CUR = CUR->next) != NULL);
    ret=ShowSubMenu(group_options_list[selected],group_options_list[selected][subselected*2],selectedstr,menuitem_list[selected*2]);
    if (ret==DLG_EXIT_CANCEL||ret==DLG_EXIT_ESC) {
      continue;
    }
    if (ret==DLG_EXIT_ERROR||ret==DLG_EXIT_UNKNOWN) {
      fprintf(stderr,"Sub dialog error...\n");
      continue;
    }
    SetOption(CURRENT,selectedstr);
    menuitem_list[selected*2+1]=strdup(selectedstr);
  }
  dlg_clear();
  dlg_killall_bg(&ret);
  end_dialog();
  initscr();
  refresh();
  endwin();
#ifndef _WIN32
  if (fork()!=0)execlp("/usr/bin/reset","/usr/bin/reset",NULL);
#endif
  dlg_exit(0);
}
#endif
