//
// C++ Implementation: Audio::SimpleSource
//

#include "SimpleSource.h"
#include "config.h"

namespace Audio {

    SimpleSource::~SimpleSource()
    {
    }

    SimpleSource::SimpleSource(SharedPtr<Sound> sound, bool looping) throw() :
        Source(sound, looping),
        scene(0),
        playing(false)
    {
    }
    
    void SimpleSource::notifySceneAttached(SimpleScene *scn) throw()
    {
        scene = scn;
    }
    
    SimpleScene* SimpleSource::getScene() const throw()
    {
        return scene;
    }
    
    void SimpleSource::startPlayingImpl(Timestamp start) throw(Exception)
    {
        // If it's playing, must stop and restart - cannot simply play again.
        if (isPlaying())
            stopPlaying();
        
        setLastKnownPlayingTime(start);
        playing = true;
        
        if (getScene())
            getScene()->notifySourcePlaying(shared_from_this(), true);
    }
    
    void SimpleSource::stopPlayingImpl() throw(Exception)
    {
        if (isPlaying()) {
            playing = false;
            
            if (getScene())
                getScene()->notifySourcePlaying(shared_from_this(), false);
        }
    }
    
    bool SimpleSource::isPlayingImpl() const throw(Exception)
    {
        return playing;
    }

};
