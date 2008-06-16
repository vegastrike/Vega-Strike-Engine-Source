//
// C++ Implementation: Audio::Source
//

#include "Listener.h"
#include "config.h"

#include <math.h>

namespace Audio {

    Listener::Listener() throw() :
        cosAngleRange(1,1),
        position(0,0,0),
        atDirection(0,0,1),
        upDirection(0,1,0),
        velocity(0,0,0),
        radius(1)
    {
    }
    
    Listener::~Listener()
    {
    }

    Range<Scalar> Listener::getAngleRange() const throw() 
    { 
        return Range<Scalar>(Scalar(acos(cosAngleRange.min)), 
                             Scalar(acos(cosAngleRange.max))); 
    }
    
    void Listener::setAngleRange(Range<Scalar> r) throw() 
    { 
        cosAngleRange.min = Scalar(cos(r.min)); 
        cosAngleRange.max = Scalar(cos(r.max));
        dirty.attributes = 1; 
    }

};
