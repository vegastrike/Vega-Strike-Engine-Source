#ifndef LIMITS_H
#define LIMITS_H

#include "gfx/vec.h"

class Limits
{
public:
//max ypr--both pos/neg are symmetrical
    float  yaw;
    float  pitch;
    float  roll;
//side-side engine thrust max
    float  lateral;
//vertical engine thrust max
    float  vertical;
//forward engine thrust max
    float  forward;
//reverse engine thrust max
    float  retro;
//after burner acceleration max
    float  afterburn;
//the vector denoting the "front" of the turret cone!
    Vector structurelimits;
//the minimum dot that the current heading can have with the structurelimit
    float  limitmin;

    Limits() : yaw( 0 )
        , pitch( 0 )
        , roll( 0 )
        , lateral( 0 )
        , vertical( 0 )
        , forward( 0 )
        , retro( 0 )
        , afterburn( 0 )
        , structurelimits( 0, 0, 0 )
        , limitmin( 0 ) {}
};

#endif // LIMITS_H
