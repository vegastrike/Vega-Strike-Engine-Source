#ifndef LIMITS_H
#define LIMITS_H

#include "gfx/vec.h"

class Limits
{
public:
    // Init overrides default values of 0 with the following values
//max ypr--both pos/neg are symmetrical
    float  yaw = 2.55;
    float  pitch = 2.55;
    float  roll = 2.55;
//side-side engine thrust max
    float  lateral = 2;
//vertical engine thrust max
    float  vertical = 8;
//forward engine thrust max
    float  forward = 2;
//reverse engine thrust max
    float  retro = 2;
//after burner acceleration max
    float  afterburn = 5;
//the vector denoting the "front" of the turret cone!
    // Again, an inconsistency between constructor and Init(). Chose Init
    // value as it comes later
    Vector structurelimits = Vector(0,0,1);
//the minimum dot that the current heading can have with the structurelimit
    float  limitmin = -1;
};

#endif // LIMITS_H
