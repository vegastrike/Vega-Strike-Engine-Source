#include "cmd_unit.h"

class FlyStraight:public AI{
	float speed;
	float time;

public:
	FlyStraight(float speed1, float time1) {parent = NULL; speed = speed1; time = time1;};
	
	AI *Execute()
	{
		if(parent->GetTime() > time)
		{
			parent->Destroy();
			delete this;
			return NULL;
		}
		else
		{
			parent->YSlide(speed);
			return this;
		}
	}
};
