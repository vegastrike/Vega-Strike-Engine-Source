//
// C++ Implementation: Audio::RenderableSource
//

#include "RenderableSource.h"
#include <stdio.h>

namespace Audio {

    RenderableSource::RenderableSource(Source *_source) :
        source(_source)
    {
    }
    
    RenderableSource::~RenderableSource()
    {
        // Just in case.
        source = 0;
    }
    
    void RenderableSource::startPlaying(Timestamp start) 
    {
        try {
            startPlayingImpl(start);
        } catch(const EndOfStreamException&) {
            // Silently discard EOS, results in the more transparent
            // behavior of simply notifying listeners of source
            // termination ASAP, which is also accurate.
        };
    }
    
    void RenderableSource::stopPlaying() 
    {
        // Cannot be playing if an exception rises, 
        // as that implies a problem with the underlying API
        try {
            if (isPlaying())
                stopPlayingImpl();
        } catch(const Exception& e) {}
    }
    
    bool RenderableSource::isPlaying() const 
    {
        try {
            return isPlayingImpl();
        } catch(const Exception& e) {
            // Cannot be playing if an exception rises, 
            // as that implies a problem with the underlying API
            return false;
        }
    }
    
    Timestamp RenderableSource::getPlayingTime() const 
    {
        return getPlayingTimeImpl();
    }
    
    void RenderableSource::update(int flags, const Listener& sceneListener) 
    {
        try {
            updateImpl(flags, sceneListener);
        } catch(const Exception& e) {
            fprintf(stderr, "Ignoring exception in renderable update: %s", e.what());
        }
    }
    
    void RenderableSource::seek(Timestamp time) 
    {
        seekImpl(time);
    }

};
