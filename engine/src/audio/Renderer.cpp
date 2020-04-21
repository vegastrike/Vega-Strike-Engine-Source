//
// C++ Implementation: Audio::Renderer
//

#include "Renderer.h"
#include "config.h"

namespace Audio {

    Renderer::Renderer()
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
       
    {
        outputFormat = format;
    }
    
    const Format& Renderer::getOutputFormat() const 
        throw()
    {
        return outputFormat;
    }
    
    void Renderer::beginTransaction() 
       
    {
        // intentionally blank
    }
    
    void Renderer::commitTransaction() 
       
    {
        // intentionally blank
    }
};
