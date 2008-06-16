//
// C++ Implementation: Audio::Source
//

#include "Source.h"
#include "config.h"
#include "utils.h"

#include <math.h>

namespace Audio {

    Source::Source(SharedPtr<Sound> sound, bool _looping) throw() :
        soundPtr(sound),
        cosAngleRange(1,1),
        pfRadiusRatios(1,1),
        referenceFreqs(250,5000),
        gain(1),
        looping(_looping),
        lastKnownPlayingTime(0),
        lastKnownPlayingTimeTime( getRealTime() )
    {
    }
    
    Source::~Source()
    {
    }

    Timestamp Source::setLastKnownPlayingTime(Timestamp timestamp) throw()
    {
        lastKnownPlayingTime = timestamp;
        lastKnownPlayingTimeTime = getRealTime();
        return timestamp;
    }

    void Source::startPlaying() throw(Exception)
    {
        startPlayingImpl( setLastKnownPlayingTime(0) );
    }
    
    void Source::stopPlaying() throw()
    {
        // Pause first to stop the renderable
        pausePlaying();
        stopPlayingImpl();
    }
    
    void Source::pausePlaying() throw()
    {
        if (rendererDataPtr.get() && isActive()) {
            try {
                setLastKnownPlayingTime( getPlayingTime() );
            } catch(Exception e) { }
            
            rendererDataPtr->stopPlaying();
        }
    }
    
    void Source::continuePlaying() throw(Exception)
    {
        if (rendererDataPtr.get() && isPlaying() && !isActive())
            rendererDataPtr->startPlaying( getWouldbePlayingTime() );
    }
    
    Timestamp Source::getPlayingTime() const throw()
    {
        try {
            if (rendererDataPtr.get() && isActive())
                return rendererDataPtr->getPlayingTime(); else
                return lastKnownPlayingTime;
        } catch(Exception e) {
            return lastKnownPlayingTime;
        }
    }

    Timestamp Source::getWouldbePlayingTime() const throw()
    {
        try {
            if (rendererDataPtr.get() && isActive())
                return rendererDataPtr->getPlayingTime();
        } catch(Exception e) { }
        return lastKnownPlayingTime + getRealTime() - lastKnownPlayingTimeTime;
    }

    bool Source::isPlaying() const throw()
    {
        try {
            return isPlayingImpl();
        } catch(Exception e) {
            return false;
        }
    }

    bool Source::isActive() const throw()
    {
        try {
            return rendererDataPtr.get() && rendererDataPtr->isPlaying();
        } catch(Exception e) {
            return false;
        }
    }

    Range<Scalar> Source::getAngleRange() const throw() 
    { 
        return Range<Scalar>(Scalar(acos(cosAngleRange.min)), 
                             Scalar(acos(cosAngleRange.max))); 
    }
    
    void Source::setAngleRange(Range<Scalar> r) throw() 
    { 
        cosAngleRange.min = Scalar(cos(r.min)); 
        cosAngleRange.max = Scalar(cos(r.max));
        dirty.attributes = 1; 
    }
    
    void Source::updateRenderable(RenderableSource::UpdateFlags flags) const 
        throw()
    {
        if (rendererDataPtr.get() && isActive())
            rendererDataPtr->update(flags);
    }

};
