#include "cmd_unit.h"

class Missile:public Unit{
public:
	Missile(char *filename);
};

class Gun:public Unit{
	Missile *projectiles[2048];  //every gun keeps track of its bullets like so
								//Maximum of 2048
	int num_proj;
	char missilename[64];

public:
	Gun(char *filename);
	~Gun()
	{
		for(int proj_index = 0; proj_index<2048; proj_index++)
			if(projectiles[proj_index])
				delete projectiles[proj_index];
	}

	void Draw();
	void Fire();
};