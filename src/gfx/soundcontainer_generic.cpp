#include "config.h"

#include <string>

#include "soundcontainer_generic.h"

#include "SharedPool.h"

void SoundContainer::init( const StringPool::Reference &_soundfile, bool _looping )
{
    this->soundFile = _soundfile;
    this->looping = _looping;
}

SoundContainer::SoundContainer( const SoundContainer &other )
{
    init(other.soundFile, other.looping);
}
    
SoundContainer::SoundContainer( const std::string &_soundfile, bool _looping )
{
    init(stringPool.get(_soundfile), _looping);
}

SoundContainer::SoundContainer( const StringPool::Reference &_soundfile, bool _looping )
{
    init(_soundfile, _looping);
}
    
SoundContainer::~SoundContainer()
{
}
