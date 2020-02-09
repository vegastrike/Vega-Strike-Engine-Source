//
// C++ Implementation: Audio::Sound
//

#include "Sound.h"
#include "config.h"

#include "utils.h"

namespace Audio {

    Sound::Sound(const std::string& _name, bool streaming) throw() :
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

    void Sound::load(bool wait) throw(Exception)
    {
        if (!isLoaded()) {
            if (!isLoading())
                loadImpl(wait);
            if (wait && !isLoaded())
                waitLoad();
        }
    }
    
    void Sound::waitLoad() 
        throw(Exception)
    {
        while (isLoading())
            Audio::sleep(10);
    }
    
    void Sound::unload() 
        throw()
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
        throw()
    {
        flags.loaded = success;
        flags.loading = false;
    }
    
    void Sound::abortLoad() 
        throw()
    {
        // Do nothing, there's no background load
    }

};
