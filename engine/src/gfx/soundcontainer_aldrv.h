#ifndef _SOUNDCONTAINER_ALDRV_H_
#define _SOUNDCONTAINER_ALDRV_H_

#include <string>

#include "soundcontainer.h"

/**
 * A reference to a soundfile that can be lazily loaded.
 * 
 * This class implements the operations on sounds using aldrv
 */
class AldrvSoundContainer : public GameSoundContainer
{
private:
    int sound;
    
public:
    /**
     * Create a refernece to an optionally-looping sound file.
     */
    AldrvSoundContainer( const SoundContainer &other );
    
    virtual ~AldrvSoundContainer();
    
protected:
    virtual void loadImpl();
    virtual void unloadImpl();
    virtual void playImpl();
    virtual void stopImpl();
    virtual bool isPlayingImpl() const;
};

#endif

