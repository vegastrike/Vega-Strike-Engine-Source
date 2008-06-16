//
// C++ Implementation: Audio::Sound
//

#include "Sound.h"
#include "config.h"

namespace Audio {

    Sound::Sound(const std::string& _name) throw() :
        name(_name)
    {
    }
    
    Sound::~Sound()
    {
    }

    void Sound::load() throw(Exception)
    {
        if (!isLoaded()) {
            loadImpl();
            loaded = true;
        }
    }
    
    void Sound::unload() throw()
    {
        if (isLoaded()) {
            unloadImpl();
            loaded = false;
        }
    }

};
