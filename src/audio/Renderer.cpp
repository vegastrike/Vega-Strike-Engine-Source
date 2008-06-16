//
// C++ Implementation: Audio::Renderer
//

#include "Renderer.h"
#include "config.h"

namespace Audio {

    Renderer::Renderer() throw(Exception)
    {
    }
    
    Renderer::~Renderer()
    {
    }
    
    void Renderer::setMeterDistance(Scalar distance) 
        throw()
    {
        meterDistance = distance;
    }
    
    Scalar Renderer::getMeterDistance() const 
        throw()
    {
        return meterDistance;
    }
};
