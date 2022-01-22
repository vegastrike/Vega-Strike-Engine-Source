#ifndef COCKPITAUDIO_H
#define COCKPITAUDIO_H

#include <string>

/** used to contain static sounds that will only be
 * created once and will get deleted automatically
 */
struct soundContainer
{
    int sound;

    soundContainer()
    {
        sound = -2;
    }

    void loadsound( std::string sound_file, bool looping = false );
    void playsound();
    ~soundContainer();
};

struct soundArray
{
    soundContainer *ptr;
    soundArray()
    {
        ptr = NULL;
    }
    void deallocate()
    {
        if (ptr != NULL) {
            delete[] ptr;
            ptr = NULL;
        }
    }
    void allocate( int siz )
    {
        deallocate();
        ptr = new soundContainer[siz];
    }
    ~soundArray()
    {
        deallocate();
    }
};

void UpdateTimeCompressionSounds();

#endif // COCKPITAUDIO_H
