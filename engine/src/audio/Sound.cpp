//
// C++ Implementation: Audio::Sound
//

#include "Sound.h"

#include "utils.h"

namespace Audio {

    Sound::Sound(const std::string& _name, bool streaming) :
        name(_name)
    {
        flags.loaded = false;
        flags.loading = false;
        flags.streaming = streaming;
    }
    
    Sound::~Sound()
    {
        unload();
    }

    void Sound::load(bool wait)
    {
        if (!isLoaded()) {
            if (!isLoading())
                loadImpl(wait);
            if (wait && !isLoaded())
                waitLoad();
        }
    }
    
    void Sound::waitLoad() 
    {
        while (isLoading())
            Audio::sleep(10);
    }
    
    void Sound::unload() 
    {
        if (isLoading()) {
            abortLoad();
            if (isLoading())
                waitLoad();
        }
        if (isLoaded()) {
            unloadImpl();
            flags.loaded = false;
        }
    }

    void Sound::onLoaded(bool success) 
    {
        flags.loaded = success;
        flags.loading = false;
    }
    
    void Sound::abortLoad() 
    {
        // Do nothing, there's no background load
    }

};
