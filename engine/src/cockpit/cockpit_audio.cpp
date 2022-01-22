#include "cockpit_audio.h"
#include "vsfilesystem.h"
#include "options.h"
#include "audiolib.h"
#include "configuration/game_config.h"

#include <string>

extern int timecount;


inline void loadsounds( const std::string &str, const int max, soundArray &snds, bool loop = false )
{
    char addstr[2] = {'\0'};
    snds.allocate( max );
    for (int i = 0; i < max; ++i) {
        addstr[0] = '1'+i;
        std::string mynewstr = str;
        while (1) {
            std::string::size_type found = mynewstr.find( '?' );
            if (found != std::string::npos) {
                mynewstr[found] = addstr[0];
            } else {
                break;
            }
        }
        snds.ptr[i].loadsound( mynewstr, loop );
    }
}

void UpdateTimeCompressionSounds()
{
    static int lasttimecompress = 0;
    if ( (timecount != lasttimecompress) && (game_options.compress_max > 0) ) {
        static bool inittimecompresssounds = false;
        static soundArray loop_snds;
        static soundArray burst_snds;
        static soundArray end_snds;
        if (inittimecompresssounds == false) {
            loadsounds( game_options.compress_loop, game_options.compress_max, loop_snds, true );
            loadsounds( game_options.compress_stop, game_options.compress_max, end_snds );
            loadsounds( game_options.compress_change, game_options.compress_max, burst_snds );
            inittimecompresssounds = true;
        }
        int soundfile     = (timecount-1)/game_options.compress_interval;
        int lastsoundfile = (lasttimecompress-1)/game_options.compress_interval;
        if (timecount > 0 && lasttimecompress >= 0) {
            if ( (soundfile+1) >= game_options.compress_max ) {
                burst_snds.ptr[game_options.compress_max-1].playsound();
            } else {
                if ( lasttimecompress > 0 && loop_snds.ptr[lastsoundfile].sound >= 0
                    && AUDIsPlaying( loop_snds.ptr[lastsoundfile].sound ) )
                    AUDStopPlaying( loop_snds.ptr[lastsoundfile].sound );
                loop_snds.ptr[soundfile].playsound();
                burst_snds.ptr[soundfile].playsound();
            }
        } else if (lasttimecompress > 0 && timecount == 0) {
            for (int i = 0; i < game_options.compress_max; ++i)
                if ( loop_snds.ptr[i].sound >= 0 && AUDIsPlaying( loop_snds.ptr[i].sound ) )
                    AUDStopPlaying( loop_snds.ptr[i].sound );
            if (lastsoundfile >= game_options.compress_max)
                end_snds.ptr[game_options.compress_max-1].playsound();
            else
                end_snds.ptr[lastsoundfile].playsound();
        }
        lasttimecompress = timecount;
    }
}

// TODO: refactor this into modern c++
// Is it trying to get the file suffix?
std::string getsoundending( int which )
{
    static bool   gotten = false;
    static std::string strs[9];
    if (gotten == false) {
        char tmpstr[2] = {'\0'};
        for (int i = 0; i < 9; i++) {
            tmpstr[0] = i+'1';
            std::string vsconfigvar = std::string( "sounds_extension_" )+tmpstr;
            strs[i]   = GameConfig::GetVariable("cockpitaudio", vsconfigvar, std::string("\n"));
                    //vs_config->getVariable( "cockpitaudio", vsconfigvar, "\n" );
            if (strs[i] == "\n") {
                strs[i] = "";
                break;
            }
        }
        gotten = true;
    }
    return strs[which];
}


std::string getsoundfile( std::string sound )
{
    bool   ok = false;
    int    i;
    std::string lastsound     = "";
    std::string anothertmpstr = "";
    for (i = 0; i < 9 && !ok; i++) {
        anothertmpstr = getsoundending( i );
        bool foundyet = false;
        while (1) {
            std::string::iterator found = std::find( anothertmpstr.begin(), anothertmpstr.end(), '*' );
            if ( found != anothertmpstr.end() ) {
                anothertmpstr.erase( found );
                anothertmpstr.insert( ( found-anothertmpstr.begin() ), sound );
                foundyet = true;
            } else {
                if (!foundyet)
                    anothertmpstr = sound+anothertmpstr;
                break;
            }
        }
        if (VSFileSystem::LookForFile( anothertmpstr, VSFileSystem::SoundFile ) < VSFileSystem::Ok) {
            ok = true;
        }
    }
    if (ok) {
        //return lastsound;
        return anothertmpstr;
    } else {
        return "";
    }
}

void soundContainer::loadsound( std::string sound_file, bool looping )
{
    if ( this->sound == -2 && sound_file.size() ) {
        std::string sound = getsoundfile( sound_file );
        if ( sound.size() )
            this->sound = AUDCreateSoundWAV( sound, looping );
        else
            this->sound = -1;
    }
}

void soundContainer::playsound()
{
    if (sound >= 0) {
        AUDAdjustSound( sound, QVector( 0, 0, 0 ), Vector( 0, 0, 0 ) );
        AUDPlay( sound, QVector( 0, 0, 0 ), Vector( 0, 0, 0 ), 1 );
    }
}

soundContainer::~soundContainer()
{
    if (sound >= 0) {
        sound = -2;
    }
}
