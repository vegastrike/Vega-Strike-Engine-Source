#include "vs_globals.h"
#include "vs_random.h"
#include <time.h>
/*
 * Globals 
 */
game_data_t g_game;
ForceFeedback *forcefeedback;
VegaConfig * vs_config;
bool cleanexit=false;
bool run_only_player_starsystem=true;

NetClient * Network=NULL;
NetServer * Server=NULL;

FILE * fpread=NULL;

float simulation_atom_var=(float)(1.0/10.0);
Mission *mission=NULL;

double benchmark=-1.0;
bool STATIC_VARS_DESTROYED=false;
const char *mission_key="unit_to_dock_with";

/* for speed test */
int loop_count=0;
double avg_loop=0;
int nb_checks=1;
double last_check=1;
double cur_check=1;
VSRandom vsrandom(time(NULL));
