#include "config.h"

#include <string>

#include "soundcontainer.h"

GameSoundContainer::GameSoundContainer( const SoundContainer &other )
    : SoundContainer( other )
    , triggered( false )
    , loaded( false )
{
}

GameSoundContainer::~GameSoundContainer()
{
    unload();
}
    
void GameSoundContainer::load()
{
    if (!loaded) {
        loadImpl();
        loaded = true;
    }
}
    
void GameSoundContainer::unload()
{
    if (loaded) {
        unloadImpl();
        loaded = false;
    }
}
    
void GameSoundContainer::play()
{
    if (!loaded)
        load();
    
    if (!isLooping() || !triggered) {
        playImpl();
        triggered = true;
    }
}
    
void GameSoundContainer::stop()
{
    if (loaded && triggered) {
        stopImpl();
        triggered = false;
    }
}
    
bool GameSoundContainer::isPlaying() const
{
    if (!loaded || !triggered)
        return false;
    else
        return isPlayingImpl();
}
