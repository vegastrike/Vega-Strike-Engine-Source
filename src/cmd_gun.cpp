#include "cmd_gun.h"
#include "cmd_ai.h"

Missile::Missile(char *filename):Unit(filename)
{
	//FILE *fp = fopen(filename, "r");
	//fclose(fp);
	

	// might want to put in code to self-destruct if filename is bad

	aistate = new FlyStraight(0.1, 6);
	aistate->SetParent(this);
}

Gun::Gun(char *filename)
{
	num_proj = 0;
#ifdef WIN32
	ZeroMemory(&projectiles, sizeof(projectiles));
#else
	bzero (&projectiles,sizeof(projectiles));
#endif
	FILE *fp = fopen(filename, "r");
	fscanf(fp, "%s", missilename);
	fclose(fp);
	
	/*
	fp = fopen(missilename, "r");

	fclose(fp);
	*/
}

void Gun::Draw()
{
	Unit::Draw();
	int proj_index;
	for(proj_index = 0; proj_index < num_proj; proj_index++)
	{
		Missile* currproj = projectiles[proj_index];
		if(currproj)
		{
			currproj->Draw();
			currproj = (Missile *)currproj->Update();
		}
	}
	while(projectiles[proj_index-1] == NULL) // decrement proj_index
		proj_index--;
}

void Gun::Fire()
{
  //////??????????? FIXME VEGASTRIKE
	Vector pos = ppos + this->pos;
	projectiles[num_proj] = new Missile(missilename);
	num_proj++;
}
