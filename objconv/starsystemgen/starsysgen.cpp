#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

using namespace std;




const int PLANET = 2;
const int GAS = 1;
const int STAR = 0;
const int MOON=3;

vector <string> entities[4];
vector <string> units [2];
vector <string> background;


void CreateStarSystem (int numsuns, int numplanets, float nummoons) {


}


void readentity (vector <string> & entity,const char * filename) {
  FILE * fp= fopen (filename,"r");
  if (!fp) {
    return;
  }
  ///warning... obvious vulnerability
  char input_buffer[1000];
  while (1==fscanf (fp,"%s", input_buffer)) {
    entity.push_back (input_buffer);
  }
  fclose (fp);
}

int main (int argc, char ** argv) {
  readentity (entities[0],"stars.txt");
  readentity (entities[1],"planets.txt");
  readentity (entities[2],"gas_giants.txt");
  readentity (entities[3],"moons.txt");
  readentity (units[0],"bigunits.txt");
  readentity (units[1],"smallunits.txt");
  readentity (background,"background.txt");
  



}
