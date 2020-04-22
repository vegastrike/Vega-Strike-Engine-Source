//
// C++ Implementation: Audio::SimpleSource
//

#include "SimpleSource.h"
#include "config.h"

namespace Audio {

    SimpleSource::~SimpleSource()
    {
    }

    SimpleSource::SimpleSource(SharedPtr<Sound> sound, bool looping) :
        Source(sound, looping),
        playing(false),
        scene(0)
    {
    }
    
    void SimpleSource::notifySceneAttached(SimpleScene *scn)
    {
        scene = scn;
    }
    
    SimpleScene* SimpleSource::getScene() const
    {
        return scene;
    }
    
    void SimpleSource::startPlayingImpl(Timestamp start)
    {
        // If it's playing, must stop and restart - cannot simply play again.
        if (isPlaying())
            stopPlaying();
        
        setLastKnownPlayingTime(start);
        playing = true;
        
        if (getScene())
            getScene()->notifySourcePlaying(shared_from_this(), true);
    }
    
    void SimpleSource::stopPlayingImpl()
    {
        if (isPlaying()) {
            playing = false;
            
            if (getScene())
                getScene()->notifySourcePlaying(shared_from_this(), false);
        }
    }
    
    bool SimpleSource::isPlayingImpl() const
    {
        return playing;
    }

};
