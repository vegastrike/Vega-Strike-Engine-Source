//
// C++ Implementation: Audio::RenderableSource
//

#include "RenderableSource.h"
#include "config.h"

namespace Audio {

    RenderableSource::RenderableSource(Source *_source) throw() : 
        source(_source)
    {
    }
    
    RenderableSource::~RenderableSource()
    {
        // Just in case.
        source = 0;
    }
    
    void RenderableSource::startPlaying(Timestamp start) 
        throw(Exception)
    {
        startPlayingImpl(start);
    }
    
    void RenderableSource::stopPlaying() 
        throw()
    {
        // Cannot be playing if an exception rises, 
        // as that implies a problem with the underlying API
        try {
            if (isPlaying())
                stopPlayingImpl();
        } catch(Exception e) {}
    }
    
    bool RenderableSource::isPlaying() const 
        throw()
    {
        try {
            return isPlayingImpl();
        } catch(Exception e) {
            // Cannot be playing if an exception rises, 
            // as that implies a problem with the underlying API
            return false;
        }
    }
    
    Timestamp RenderableSource::getPlayingTime() const 
        throw(Exception)
    {
        return getPlayingTimeImpl();
    }
    
    void RenderableSource::update(UpdateFlags flags) 
        throw()
    {
        try {
            updateImpl(flags);
        } catch(Exception e) {}
    }

};
