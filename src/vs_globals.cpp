#include "vs_globals.h"

/*
 * Globals 
 */
game_data_t g_game;

NetClient * Network;

FILE * fpread=NULL;

float simulation_atom_var=(float)(1.0/10.0);
Mission *mission=NULL;

/* for speed test */
int loop_count=0;
int avg_loop=0;
int nb_checks=0;
double last_check=1;
double cur_check=1;
