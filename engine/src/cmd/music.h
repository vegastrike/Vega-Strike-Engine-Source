/**
* music.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _MUSIC_H_
#define _MUSIC_H_
#include "container.h"
#include <vector>
#include <string>
#include <list>
#if defined(__HAIKU__)
#include <pthread.h>
#endif

namespace Muzak
{
#ifndef _WIN32
extern void * readerThread( void *input );
#else
extern DWORD WINAPI readerThread( PVOID input );
#endif
};

class Music
{
    bool  random;
    float maxhull;
    int   socketr;
    int   socketw;
    UnitContainer p;
    int   song; //currently playing song
    int   lastlist;
    float vol;
    float soft_vol;
    float soft_vol_up_latency;
    float soft_vol_down_latency;

    std::list< int >playingSource;

    bool LoadMusic( const char *file );
    struct PlayList
    {
        typedef std::vector< std::string >          SongList;
        typedef std::map< std::string, std::string >PragmaList;

        SongList     songs;
        PragmaList   pragmas;

        unsigned int counter;
        PlayList()
        {
            counter = 0;
        }
        bool empty() const
        {
            return songs.empty();
        }
        size_t size() const
        {
            return songs.size();
        }
        std::string&operator[]( size_t index )
        {
            return songs[index];
        }
        const std::string&operator[]( size_t index ) const
        {
            return songs[index];
        }
        void push_back( const std::string &s )
        {
            songs.push_back( s );
        }

        bool haspragma( const std::string &name ) const
        {
            return pragmas.find( name ) != pragmas.end();
        }
        const std::string& pragma( const std::string &name, const std::string &def ) const
        {
            PragmaList::const_iterator it = pragmas.find( name );
            if ( it != pragmas.end() )
                return it->second;

            else
                return def;
        }
    };
    std::vector< PlayList >playlist;
public:
    enum Playlist {NOLIST=-1, BATTLELIST=0, PEACELIST, PANICLIST, VICTORYLIST, LOSSLIST, MAXLIST};
    Music( Unit *parent = NULL );
    int loopsleft;
    void Listen();
    ~Music();
    void SetParent( Unit *parent );

    void GotoSong( int whichlist, int whichsong, bool skip, int layer = -1 );

    static int Addlist( std::string listfile );
    static void SetLoops( int numloops, int layer = -1 );
    static void ChangeVolume( float inc = 0, int layer = -1 );

    static void Skip( int layer = -1 );
    static void Stop( int layer = -1 );
    static void SkipRandSong( int whichlist, int layer = -1 );
    static void SkipRandList( int layer = -1 );

    static void GotoSong( std::string mus, int layer = -1 );

    static void InitMuzak();
    static void CleanupMuzak();
    static void MuzakCycle();

    static void SetVolume( float vol, int layer = -1, bool hardware = false, float latency_override = -1 );
    static void Mute( bool mute = true, int layer = -1 );
    std::vector< int >sounds_to_stop;
private:
    void _StopLater();
    void _StopNow();
    void _GotoSong( std::string mus );
    int _Addlist( std::string listfile );
    void _SetVolume( float vol = 0, bool hardware = false, float latency_override = -1 );
    void _SkipRandSong( int whichlist, int layer = -1 );
    void _SkipRandList( int layer = -1 );
    void _Skip( int layer = -1 );
    void _Stop();

    std::list< std::string >recent_songs;

    int SelectTracks( int layer = -1 );

#ifndef _WIN32
    friend void* Muzak::readerThread( void *input );
#else
    friend DWORD WINAPI Muzak::readerThread( PVOID input );
#endif

#ifdef _WIN32
    void     *a_thread;
    HANDLE    musicinfo_mutex;
#else
    pthread_t a_thread;
    pthread_mutex_t musicinfo_mutex;
#endif

    int thread_initialized;
    volatile bool moredata;
    volatile bool music_loaded;
    volatile bool killthread;
    volatile bool threadalive;
    volatile bool music_loading; //Opposite order of music_loaded.
    bool freeWav;
    struct AUDSoundProperties *music_load_info;
    vector< std::string >music_load_list; //reverse order.

    void _LoadLastSongAsync();

    std::string cur_song_file;
};
extern Music *muzak;
extern int    muzak_count;

#endif

