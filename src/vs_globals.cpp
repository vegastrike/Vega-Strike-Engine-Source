#include "vs_globals.h"

/*
 * Globals 
 */
game_data_t g_game;

NetClient * Network;

FILE * fpread=NULL;

float simulation_atom_var=(float)(1.0/10.0);
Mission *mission=NULL;