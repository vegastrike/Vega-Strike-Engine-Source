//
// C++ Implementation: Audio::Renderer
//

#include "Renderer.h"

namespace Audio {

    Renderer::Renderer()
    {
    }
    
    Renderer::~Renderer()
    {
    }
    
    void Renderer::setMeterDistance(Scalar distance) 
    {
        meterDistance = distance;
    }
    
    Scalar Renderer::getMeterDistance() const 
    {
        return meterDistance;
    }
    
    void Renderer::setDopplerFactor(Scalar factor) 
    {
        dopplerFactor = factor;
    }
    
    Scalar Renderer::getDopplerFactor() const 
    {
        return dopplerFactor;
    }
    
    void Renderer::setOutputFormat(const Format &format) 
    {
        outputFormat = format;
    }
    
    const Format& Renderer::getOutputFormat() const 
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
