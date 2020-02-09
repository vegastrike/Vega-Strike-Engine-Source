#include "config.h"

#include <string>

#include "soundcontainer_generic.h"

#include "SharedPool.h"

void SoundContainer::init( const StringPool::Reference &_soundfile, bool _looping, float _gain )
{
    this->soundFile = _soundfile;
    this->looping = _looping;
    this->gain = _gain;
}

SoundContainer::SoundContainer( const SoundContainer &other )
{
    init(other.soundFile, other.looping, other.gain);
}
    
SoundContainer::SoundContainer( const std::string &_soundfile, bool _looping, float _gain )
{
    init(stringPool.get(_soundfile), _looping, _gain);
}

SoundContainer::SoundContainer( const StringPool::Reference &_soundfile, bool _looping, float _gain )
{
    init(_soundfile, _looping, _gain);
}
    
SoundContainer::~SoundContainer()
{
}
