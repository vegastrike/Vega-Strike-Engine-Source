#include "vs_globals.h"

/*
 * Globals 
 */
game_data_t g_game;

NetClient * Network;

Universe *_Universe;
FILE * fpread=NULL;

float simulation_atom_var=(float)(1.0/10.0);
Mission *mission=NULL;