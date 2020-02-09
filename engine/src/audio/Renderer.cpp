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
    
    void Renderer::setDopplerFactor(Scalar factor) 
        throw()
    {
        dopplerFactor = factor;
    }
    
    Scalar Renderer::getDopplerFactor() const 
        throw()
    {
        return dopplerFactor;
    }
    
    void Renderer::setOutputFormat(const Format &format) 
        throw(Exception)
    {
        outputFormat = format;
    }
    
    const Format& Renderer::getOutputFormat() const 
        throw()
    {
        return outputFormat;
    }
    
    void Renderer::beginTransaction() 
        throw(Exception)
    {
        // intentionally blank
    }
    
    void Renderer::commitTransaction() 
        throw(Exception)
    {
        // intentionally blank
    }
};
