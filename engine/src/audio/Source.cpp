//
// C++ Implementation: Audio::Source
//

#include "Source.h"
#include "SourceListener.h"
#include "utils.h"

#include <math.h>

namespace Audio {

    Source::Source(SharedPtr<Sound> sound, bool _looping) :
        soundPtr(sound),
        
        // Some safe defaults
        position(0,0,0),
        direction(0,0,1),
        velocity(0,0,0),
        
        cosAngleRange(-1,-1),
        
        radius(1),
        
        pfRadiusRatios(1,1),
        referenceFreqs(250,5000),
        
        gain(1),
        
        lastKnownPlayingTime(0),
        lastKnownPlayingTimeTime( getRealTime() )
    {
        // Some default flags
        setLooping(_looping);
        setRelative(false);
        setAttenuated(true);
    }
    
    Source::~Source()
    {
    }

    Timestamp Source::setLastKnownPlayingTime(Timestamp timestamp)
    {
        lastKnownPlayingTime = timestamp;
        lastKnownPlayingTimeTime = getRealTime();
        return timestamp;
    }

    void Source::startPlaying(Timestamp start)
    {
        dirty.setAll();
        startPlayingImpl( setLastKnownPlayingTime(start) );
    }
    
    void Source::stopPlaying()
    {
        // Pause first to stop the renderable
        pausePlaying();
        stopPlayingImpl();
    }
    
    void Source::pausePlaying()
    {
        if (rendererDataPtr.get() && isActive()) {
            try {
                setLastKnownPlayingTime( getPlayingTime() );
            } catch(const Exception& e) { }
            
            // Must notify the listener, if any
            if (sourceListenerPtr.get() != 0 && sourceListenerPtr->wantPlayEvents())
                sourceListenerPtr->onPrePlay(*this, false);
            
            rendererDataPtr->stopPlaying();
            
            // Must notify the listener, if any
            if (sourceListenerPtr.get() != 0 && sourceListenerPtr->wantPlayEvents())
                sourceListenerPtr->onPostPlay(*this, false);
        }
    }
    
    void Source::continuePlaying()
    {
        if (rendererDataPtr.get() && isPlaying() && !isActive()) {
            // Must notify the listener, if any
            if (sourceListenerPtr.get() != 0 && sourceListenerPtr->wantPlayEvents())
                sourceListenerPtr->onPrePlay(*this, true);
            
            rendererDataPtr->startPlaying( getWouldbePlayingTime() );
            
            // Must notify the listener, if any
            if (sourceListenerPtr.get() != 0 && sourceListenerPtr->wantPlayEvents())
                sourceListenerPtr->onPrePlay(*this, true);
            
        }
    }
    
    Timestamp Source::getPlayingTime() const
    {
        try {
            if (rendererDataPtr.get() && isActive())
                return rendererDataPtr->getPlayingTime();
            else
                return lastKnownPlayingTime;
        } catch(const Exception& e) {
            return lastKnownPlayingTime;
        }
    }

    Timestamp Source::getWouldbePlayingTime() const
    {
        try {
            if (rendererDataPtr.get() && isActive())
                return rendererDataPtr->getPlayingTime();
        } catch(const Exception& e) { }
        return lastKnownPlayingTime + getRealTime() - lastKnownPlayingTimeTime;
    }

    bool Source::isPlaying() const
    {
        try {
            return isPlayingImpl();
        } catch(const Exception& e) {
            return false;
        }
    }

    bool Source::isActive() const
    {
        try {
            return rendererDataPtr.get() && rendererDataPtr->isPlaying();
        } catch(const Exception& e) {
            return false;
        }
    }

    Range<Scalar> Source::getAngleRange() const
    { 
        return Range<Scalar>(Scalar(acos(cosAngleRange.min)), 
                             Scalar(acos(cosAngleRange.max))); 
    }
    
    void Source::setAngleRange(Range<Scalar> r)
    { 
        cosAngleRange.min = Scalar(cos(r.min)); 
        cosAngleRange.max = Scalar(cos(r.max));
        dirty.attributes = 1; 
    }
    
    void Source::updateRenderable(int flags, const Listener& sceneListener) 
    {
        if (rendererDataPtr.get()) {
            int oflags = flags;
            
            if (!dirty.attributes)
                flags &= ~RenderableSource::UPDATE_ATTRIBUTES;
            if (!dirty.gain)
                flags &= ~RenderableSource::UPDATE_GAIN;
            
            // Must always update location... listeners might move around.
            flags |= RenderableSource::UPDATE_LOCATION;
            
            // Must nofity listener, if any
            if (sourceListenerPtr.get() != 0 && sourceListenerPtr->wantUpdateEvents())
                sourceListenerPtr->onUpdate(*this, flags);
            
            rendererDataPtr->update(flags, sceneListener);
            
            if (oflags == RenderableSource::UPDATE_ALL) {
                dirty.reset();
            } else {
                if (flags & RenderableSource::UPDATE_LOCATION)
                    dirty.location = 0;
                if (flags & RenderableSource::UPDATE_ATTRIBUTES)
                    dirty.attributes = 0;
                if (flags & RenderableSource::UPDATE_GAIN)
                    dirty.gain = 0;
            }
            
            switch(flags) {
            case RenderableSource::UPDATE_ALL:
                dirty.reset();
                break;
            case RenderableSource::UPDATE_EFFECTS:
            case RenderableSource::UPDATE_ATTRIBUTES:
                dirty.attributes = 0;
            case RenderableSource::UPDATE_LOCATION:
                dirty.location = 0;
            };
        }
    }
    
    void Source::setRenderable(SharedPtr<RenderableSource> ptr) 
    { 
        // Notify at/detachment to listener, if any
        if (sourceListenerPtr.get() != 0 && sourceListenerPtr->wantAttachEvents())
            sourceListenerPtr->onPreAttach(*this, ptr.get() != 0);
        
        rendererDataPtr = ptr; 
        
        // Notify at/detachment to listener, if any
        if (sourceListenerPtr.get() != 0 && sourceListenerPtr->wantAttachEvents())
            sourceListenerPtr->onPostAttach(*this, ptr.get() != 0);
    }
    
    void Source::seek(Timestamp time) 
    {
        if (rendererDataPtr.get() && isPlaying() && isActive()) {
            rendererDataPtr->seek(time);
        } else {
            setLastKnownPlayingTime(time);
        }
    }

};
