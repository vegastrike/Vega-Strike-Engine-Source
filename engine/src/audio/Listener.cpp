//
// C++ Implementation: Audio::Source
//

#include "Listener.h"
#include "config.h"

#include <math.h>

namespace Audio {

    Listener::Listener() throw() :
        cosAngleRange(-1,-1),
        position(0,0,0),
        atDirection(0,0,-1),
        upDirection(0,1,0),
        velocity(0,0,0),
        radius(1),
        gain(1),
        worldToLocal(1) // set to identity, given default settings
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

    void Listener::update(int flags) throw()
    {
        if (!dirty.attributes)
            flags &= ~RenderableListener::UPDATE_ATTRIBUTES;
        if (!dirty.location)
            flags &= ~RenderableListener::UPDATE_LOCATION;
        if (!dirty.gain)
            flags &= ~RenderableListener::UPDATE_GAIN;
        
        if (getRenderable().get() != 0)
            getRenderable()->update(flags);
        
        if (flags & RenderableListener::UPDATE_ATTRIBUTES)
            dirty.attributes = 0;
        if (flags & RenderableListener::UPDATE_GAIN)
            dirty.gain = 0;
        if (flags & RenderableListener::UPDATE_LOCATION) {
            worldToLocal = 
                Matrix3(
                    atDirection.cross(upDirection),
                    upDirection,
                    -atDirection
                ).inverse();
            dirty.location = 0;
        }
    }

    Vector3 Listener::toLocalDirection(Vector3 dir) const throw()
    {
        return worldToLocal * dir;
    }

};
