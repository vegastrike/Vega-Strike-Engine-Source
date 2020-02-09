#ifndef _SOUNDCONTAINER_GENERIC_H_
#define _SOUNDCONTAINER_GENERIC_H_

#include <string>

#include "SharedPool.h"

/**
 * A reference to a soundfile that can be lazily loaded.
 * 
 * This base class implement no actions on it, see concrete subclasses for that,
 * but it does contain all common data points.
 */
class SoundContainer
{
private:
    StringPool::Reference soundFile;
    float gain;
    bool looping;
    
protected:
    void init( const StringPool::Reference &soundfile, bool looping, float gain );
    
public:
    /**
     * Create a refernece to an optionally-looping sound file.
     */
    SoundContainer( const SoundContainer &other );
    explicit SoundContainer( const std::string &soundfile, bool looping = false, float gain = 1.0f );
    explicit SoundContainer( const StringPool::Reference &soundfile, bool looping = false, float gain = 1.0f );
    
    const std::string& getSoundFile() const 
    { 
        return soundFile.get(); 
    }
    
    const bool isLooping() const
    {
        return looping;
    }
    
    const bool getGain() const
    {
        return gain;
    }
    
    virtual ~SoundContainer();
};

#endif

