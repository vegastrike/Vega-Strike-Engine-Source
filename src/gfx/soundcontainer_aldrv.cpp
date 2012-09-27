#include "config.h"

#include "soundcontainer_aldrv.h"
#include "audiolib.h"

AldrvSoundContainer::AldrvSoundContainer( const SoundContainer &other )
    : GameSoundContainer(other)
    , sound(-2)
{
}
    
AldrvSoundContainer::~AldrvSoundContainer()
{
    // unload already takes care
}
    
void AldrvSoundContainer::loadImpl()
{
    sound = AUDCreateSoundWAV( getSoundFile(), isLooping() );
    AUDSoundGain( sound, getGain() );
}

void AldrvSoundContainer::unloadImpl()
{
    if (sound >= 0) {
        stopImpl();
        AUDDeleteSound( sound, false );
        sound = -2;
    }
}

void AldrvSoundContainer::playImpl()
{
    AUDStartPlaying(sound);
}

void AldrvSoundContainer::stopImpl()
{
    AUDStopPlaying(sound);
}

bool AldrvSoundContainer::isPlayingImpl() const
{
    return AUDIsPlaying(sound);
}

