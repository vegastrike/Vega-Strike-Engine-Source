/*
 * music.cpp
 *
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Evert Vorster, and other Vega Strike contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include <map>
#include <set>
#include <algorithm>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif //tells VCC not to generate min/max macros
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <string.h>
#include <stdio.h>
#include <direct.h>
#include <stdlib.h>
#endif

#include "src/vegastrike.h"
#include "root_generic/vs_globals.h"

#include "src/audiolib.h"
#include "src/universe.h"
#include "src/star_system.h"
#include "root_generic/vs_globals.h"
#include "src/config_xml.h"
#include "root_generic/lin_time.h"
#include "cmd/collection.h"
#include "cmd/unit_generic.h"
#include "root_generic/vsfilesystem.h"
#include "src/vs_logging.h"
#include "cmd/music.h"
#include "cmd/base.h"

#include "src/python/python_compile.h"

//To allow for loading in another thread, we must handle some AL vars ourselves...
#include "aldrv/al_globals.h"
#include "root_generic/options.h"

#define MAX_RECENT_HISTORY "5"

Music *muzak = NULL;
int muzak_count = 0;
int muzak_cross_index = 0;

static void print_check_err(int errorcode, const char *str) {
#ifndef _WIN32
    if (errorcode) {
        static char const unknown_error[16] = "Unknown error"; //added by chuck_starchaser to get rid of warning
        char const *err = strerror(errorcode);
        if (!err) {
            err = unknown_error;
        }
        VS_LOG(error, (boost::format("ERROR IN PTHREAD FUNCTION %1%: %2% (%3%)\n") % str % err % errorcode));
    }
#endif
}

//where func is the evaluation of func, and #func is the string form.
#define checkerr(func) do{print_check_err(((func)),#func);}while(0)

Music::Music(Unit *parent) : random(false), p(parent), song(-1), thread_initialized(false) {
    loopsleft = 0;
    socketw = socketr = -1;
    music_load_info = NULL;
    music_loaded = false;
    music_loading = false;
    killthread = 0;
    threadalive = 0;
    freeWav = true;
#ifdef HAVE_AL
    music_load_info = new AUDSoundProperties;
#endif

#ifdef _WIN32
    musicinfo_mutex = CreateMutex( NULL, TRUE, NULL );
#else //_WIN32
#ifdef ERRORCHECK_MUTEX
    pthread_mutexattr_t checkme;
    pthread_mutexattr_init( &checkme );
    pthread_mutexattr_settype( &checkme, PTHREAD_MUTEX_ERRORCHECK );
    checkerr( pthread_mutex_init( &musicinfo_mutex, &checkme ) );
#else //ERRORCHECK_MUTEX
    checkerr(pthread_mutex_init(&musicinfo_mutex, NULL));
#endif //!ERRORCHECK_MUTEX

    //Lock it immediately, since the loader will want to wait for its first data upon creation.
    checkerr(pthread_mutex_lock(&musicinfo_mutex));
#endif //!_WIN32
    if (!game_options()->Music) {
        return;
    }
    lastlist = PEACELIST;
    int i;
    const char *listvars[MAXLIST] =
            {"battleplaylist", "peaceplaylist", "panicplaylist", "victoryplaylist", "lossplaylist"};
    const char *deflistvars[MAXLIST] = {"battle.m3u", "peace.m3u", "panic.m3u", "victory.m3u", "loss.m3u"};
    for (i = 0; i < MAXLIST; i++) {
        LoadMusic(vs_config->getVariable("audio", listvars[i], deflistvars[i]).c_str());
    }
    soft_vol_up_latency = vega_config::config->audio.music_volume_up_latency;
    soft_vol_down_latency = vega_config::config->audio.music_volume_down_latency;
    //Hardware volume = 1
    _SetVolume(1, true);
    //Software volume = from config
    _SetVolume(vega_config::config->audio.music_volume /* default: ".5" */, false);
}

void Music::ChangeVolume(float inc, int layer) {
    if (!game_options()->Music) {
        return;
    }
    if (muzak) {
        if (layer < 0) {
            for (int i = 0; i < muzak_count; i++) {
                muzak[i]._SetVolume(muzak[i].soft_vol + inc, false, 0.1);
            }
        } else if ((layer >= 0) && (layer < muzak_count)) {
            muzak[layer]._SetVolume(muzak[layer].soft_vol + inc, false, 0.1);
        }
    }
}

void Music::_SetVolume(float vol, bool hardware, float latency_override) {
    if (!game_options()->Music) {
        return;
    }
    if (vol < 0) {
        vol = 0;
    }
    this->vol = vol;
    this->soft_vol = vol;     //for now fixme for fading
    for (std::list<int>::const_iterator iter = playingSource.begin(); iter != playingSource.end(); iter++) {
        AUDSoundGain(*iter, soft_vol, true);
    }

}

bool Music::LoadMusic(const char *file) {
    using namespace VSFileSystem;
    if (!game_options()->Music) {
        return true;
    }
    //Loads a playlist so try to open a file in data_dir or homedir
    VSFile f;
    VSError err = f.OpenReadOnly(file, UnknownFile);
    if (err > Ok) {
        err = f.OpenReadOnly(VSFileSystem::HOMESUBDIR + "/" + file, UnknownFile);
    }
    char songname[1024];
    this->playlist.push_back(PlayList());
    if (err <= Ok) {
        while (!f.Eof()) {
            songname[0] = '\0';
            f.ReadLine(songname, 1022);
            int size = strlen(songname);
            if (size >= 1) {
                if (songname[size - 1] == '\n') {
                    songname[size - 1] = '\0';
                }
            }
            if (size > 1) {
                if (songname[size - 2] == '\r' || songname[size - 2] == '\n') {
                    songname[size - 2] = '\0';
                }
            }
            if (songname[0] == '\0') {
                continue;
            }
            if (songname[0] == '#') {
                if (strncmp(songname, "#pragma ", 8) == 0) {
                    char *sep = strchr(songname + 8, ' ');
                    if (sep) {
                        *sep = 0;
                        this->playlist.back().pragmas[songname + 8] = sep + 1;
                    } else if (songname[8]) {
                        this->playlist.back().pragmas[songname + 8] = "1";
                    }
                }
                continue;
            }
            this->playlist.back().songs.push_back(std::string(songname));
        }
        f.Close();
    } else {
        return false;
    }
    return true;
}

static int randInt(int max) {
    int ans = int((((double) rand()) / ((double) RAND_MAX)) * max);
    if (ans == max) {
        return max - 1;
    }
    return ans;
}

int Music::SelectTracks(int layer) {
    if (!game_options()->Music) {
        return 0;
    }
    const bool random = vega_config::config->audio.shuffle_songs;
    const size_t maxrecent = vega_config::config->audio.shuffle_songs_section.history_depth;
    const std::string dj_script = vega_config::config->audio.dj_script; // default: "modules/dj.py"
    if ((BaseInterface::CurrentBase || loopsleft > 0) && lastlist < (int) playlist.size() && lastlist >= 0) {
        if (loopsleft > 0) {
            loopsleft--;
        }
        if (!playlist[lastlist].empty() && !playlist[lastlist].haspragma("norepeat")) {
            int whichsong = (random ? rand() : playlist[lastlist].counter++) % playlist[lastlist].size();
            int spincount = 10;
            std::list<std::string> &recent = muzak[(layer >= 0) ? layer : 0].recent_songs;
            while (random && (--spincount > 0)
                    && (std::find(recent.begin(), recent.end(), playlist[lastlist][whichsong]) != recent.end())) {
                whichsong = (random ? rand() : playlist[lastlist].counter++) % playlist[lastlist].size();
            }
            if (spincount <= 0) {
                recent.clear();
            }
            recent.push_back(playlist[lastlist][whichsong]);
            while (recent.size() > maxrecent) {
                recent.pop_front();
            }
            GotoSong(lastlist, whichsong, true, layer);
            return whichsong;
        }
    }
    if (_Universe && _Universe->activeStarSystem() != NULL && _Universe->numPlayers()) {
        CompileRunPython(dj_script);
    } else {
        const std::string loading_tune = vega_config::config->audio.loading_sound;
        GotoSong(loading_tune, layer);
    }
    return 0;
}

namespace Muzak {
std::map<std::string, AUDSoundProperties> cachedSongs;

#ifndef _WIN32

void *
#else
DWORD WINAPI
#endif
readerThread(
#ifdef _WIN32
        PVOID
#else
        void *
#endif
        input) {
    Music *me = (Music *) input;
    me->threadalive = 1;
    while (!me->killthread) {
#ifdef _WIN32
        WaitForSingleObject( me->musicinfo_mutex, INFINITE );
#else
        checkerr(pthread_mutex_lock(&me->musicinfo_mutex));
#endif
        if (me->killthread) {
            break;
        }
        me->music_loading = true;
        me->music_loaded = false;
        me->music_load_info->success = false;
        size_t len = me->music_load_info->hashname.length();
        char *songname = (char *) malloc(len + 1);
        songname[len] = '\0';
        memcpy(songname, me->music_load_info->hashname.data(), len);
        std::map<std::string, AUDSoundProperties>::iterator wherecache = cachedSongs.find(songname);
        bool foundcache = wherecache != cachedSongs.end();
        static std::string cachable_songs = vega_config::config->audio.cache_songs;
        bool docacheme = cachable_songs.find(songname) != std::string::npos;
        if (foundcache == false && docacheme) {
            me->music_load_info->wave = NULL;
            cachedSongs[songname] = *me->music_load_info;
            wherecache = cachedSongs.find(songname);
        }
#ifdef _WIN32
            ReleaseMutex( me->musicinfo_mutex );
#else
        checkerr(pthread_mutex_unlock(&me->musicinfo_mutex));
#endif
        {
            me->freeWav = true;
            if (foundcache) {
                *me->music_load_info = wherecache->second;
                me->freeWav = false;
            } else if (!AUDLoadSoundFile(songname, me->music_load_info, true)) {
                VS_LOG(info, (boost::format("Failed to load music file \"%1%\"") % songname));
            }
        }
        if (me->freeWav && docacheme) {
            me->freeWav = false;
            wherecache->second = *me->music_load_info;
        }
        free(songname);
        me->music_loaded = true;
        while (me->music_loaded) {
            micro_sleep(10000); //10ms of busywait for now... wait until end of frame.
        }
    }

    me->threadalive = 0;
    return NULL;
}
}

void Music::_LoadLastSongAsync() {

#ifdef HAVE_AL
    if (!game_options()->Music || !music_load_info || music_loading) {
        //No touching anything here!
        return;
    }
    std::string song = music_load_list.back();

    std::map<std::string, AUDSoundProperties>::iterator where = Muzak::cachedSongs.find(song);
    if (where != Muzak::cachedSongs.end()) {
        if (where->second.wave != NULL) {
            int source = AUDBufferSound(&where->second, true);
            AUDStreamingSound(source);

            music_load_info->wave = NULL;
            if (source != -1) {
                playingSource.push_back(source);
            }
            if (playingSource.size() == 1) {
                //Start playing if first in list.
                _StopNow();
                AUDStartPlaying(playingSource.front());
                AUDSoundGain(playingSource.front(), vol, true);
            }
            return;
        }
    }
    music_load_info->hashname = song;
#endif
    music_loading = true;
#ifdef _WIN32
    ReleaseMutex( musicinfo_mutex );
#else
    checkerr(pthread_mutex_unlock(&musicinfo_mutex));
#endif

}

void Music::Listen() {
    if (game_options()->Music) {
        if (!music_load_list.empty()) {
            if (music_loaded) {
#ifdef _WIN32
                int trylock_ret = 0;
                if (WaitForSingleObject( musicinfo_mutex, 0 ) == WAIT_TIMEOUT) {
#else
                int trylock_ret = pthread_mutex_trylock(&musicinfo_mutex);
                if (trylock_ret == EBUSY) {
#endif
                    VS_LOG(warning, "Failed to lock music loading mutex despite loaded flag being set...\n");
                    return;
                } else {
                    checkerr(trylock_ret);
                }
                music_loading = false;
                music_loaded =
                        false;                 //once the loading thread sees this, it will try to grab a lock and wait.
                //The lock will only be achieved once the next song is put in the queue.

#ifdef HAVE_AL
                if (music_load_info->success && music_load_info->wave) {
                    int source = AUDBufferSound(music_load_info, true);
                    if (freeWav) {
                        free(music_load_info->wave);
                        music_load_info->wave = nullptr;
                    }
                    if (source != -1) {
                        playingSource.push_back(source);
                    }
                }
#endif
                if (playingSource.size() == 1) {
                    //Start playing if first in list.
                    _StopNow();
                    AUDStartPlaying(playingSource.front());
                    AUDStreamingSound(playingSource.front());
                    AUDSoundGain(playingSource.front(), vol, true);
                }
                music_load_list.pop_back();
                if (!music_load_list.empty()) {
                    _LoadLastSongAsync();
                }
                return;                  //Returns if finished loading, since the AUDIsPlaying() could fail right now.
            }
        }
        if (!playingSource.empty()) {
            if (!AUDIsPlaying(playingSource.front())) {
                AUDDeleteSound(playingSource.front(), true);
                playingSource.pop_front();
                if (!playingSource.empty()) {
                    _StopNow();
                    AUDStartPlaying(playingSource.front());
                    AUDSoundGain(playingSource.front(), vol, true);
                }
            }
        }
        if (playingSource.empty() && muzak[muzak_cross_index].playingSource.empty()
                && music_load_list.empty() && muzak[muzak_cross_index].music_load_list.empty()) {
            cur_song_file = "";
            _Skip();
        }
    }
}

void Music::GotoSong(std::string mus, int layer) {
    if (!game_options()->Music) {
        return;
    }
    const bool cross = vega_config::config->audio.cross_fade_music;
    if (cross && (muzak_count >= 2)) {
        if (layer < 0) {
            if (mus == muzak[muzak_cross_index].cur_song_file) {
                return;
            }
            muzak[muzak_cross_index]._StopLater();
            muzak_cross_index = (muzak_cross_index ^ 1);
            muzak[muzak_cross_index]._GotoSong(mus);
        } else if ((layer >= 0) && (layer < muzak_count)) {
            if (mus == muzak[layer].cur_song_file) {
                return;
            }
            muzak[layer]._GotoSong(mus);
        }
    } else {
        muzak->_GotoSong(mus);
    }
}

std::vector<std::string> rsplit(std::string tmpstr, std::string splitter) {
    std::string::size_type where;
    std::vector<std::string> ret;
    while ((where = tmpstr.rfind(splitter)) != std::string::npos) {
        ret.push_back(tmpstr.substr(where + 1));
        tmpstr = tmpstr.substr(0, where);
    }
    if (tmpstr.length()) {
        ret.push_back(tmpstr);
    }
    return ret;
}

void Music::_GotoSong(std::string mus) {
    if (game_options()->Music) {
        if (mus == cur_song_file || mus.length() == 0) {
            return;
        }
        cur_song_file = mus;
        _StopLater();         //Kill all our currently playing songs.

        music_load_list = rsplit(mus, "|");         //reverse order.
        if (!thread_initialized) {
#ifdef _WIN32
            a_thread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) Muzak::readerThread, (PVOID) this, 0, NULL );
            if (a_thread){
                thread_initialized = true;
            } else {
                VS_LOG(error, (boost::format("Error creating music load thread: %1$d\n") % GetLastError()));
            }
#else
            int thread_create_ret = pthread_create(&a_thread, NULL, Muzak::readerThread, this);
            if (thread_create_ret == 0) {
                thread_initialized = true;
            } else {
                checkerr(thread_create_ret);
            }
#endif
        }
        _LoadLastSongAsync();

    }
}

void Music::GotoSong(int whichlist, int whichsong, bool skip, int layer) {
    if (game_options()->Music) {
        if (whichsong != NOLIST && whichlist != NOLIST && whichlist < (int) playlist.size() && whichsong
                < (int) playlist[whichlist].size()) {
            if (muzak[(layer >= 0) ? layer : 0].lastlist != whichlist) {
                if (vega_config::config->audio.shuffle_songs_section.clear_history_on_list_change) {
                    std::list<std::string> &recent = muzak[(layer >= 0) ? layer : 0].recent_songs;
                    recent.clear();
                }
            }
            if ((layer < 0) && (muzak_count >= 2)) {
                muzak[0].lastlist = muzak[1].lastlist = whichlist;
            } else {
                lastlist = whichlist;
            }
            GotoSong(playlist[whichlist][whichsong], layer);
        } else {
            _SkipRandList(layer);
        }
    }
}

void Music::SkipRandSong(int whichlist, int layer) {
    if (muzak) {
        if (layer < 0) {
            if (muzak_count >= 2) {
                muzak[muzak_cross_index]._SkipRandSong(whichlist);
            } else {
                muzak->_SkipRandSong(whichlist);
            }
        } else if ((layer >= 0) && (layer < muzak_count)) {
            muzak[layer]._SkipRandSong(whichlist, layer);
        }
    }
}

void Music::_SkipRandSong(int whichlist, int layer) {
    if (!game_options()->Music) {
        return;
    }
    if (whichlist != NOLIST && whichlist >= 0 && whichlist < (int) playlist.size()) {
        lastlist = whichlist;
        const bool random = vega_config::config->audio.shuffle_songs;
        if (playlist[whichlist].size()) {
            GotoSong(whichlist, random ? randInt(playlist[whichlist].size()) : playlist[whichlist].counter++
                    % playlist[whichlist].size(), true, layer);
        } else {
            VS_LOG(error, (boost::format("Error no songs in playlist %1$d\n") % whichlist));
        }
    }
    _SkipRandList(layer);
}

void Music::SkipRandList(int layer) {
    if (!game_options()->Music) {
        return;
    }
    if (muzak) {
        if (layer < 0) {
            if (muzak_count >= 2) {
                muzak[muzak_cross_index]._SkipRandList();
            } else {
                muzak->_SkipRandList();
            }
        } else if ((layer >= 0) && (layer < muzak_count)) {
            muzak[layer]._SkipRandList(layer);
        }
    }
}

void Music::_SkipRandList(int layer) {
    if (!game_options()->Music) {
        return;
    }
    for (unsigned int i = 0; i < playlist.size(); i++) {
        const bool random = vega_config::config->audio.shuffle_songs;
        if (!playlist[i].empty()) {
            GotoSong(i,
                    random ? randInt(playlist[i].size()) : playlist[i].counter++ % playlist[i].size(),
                    false,
                    layer);
        }
    }
}

int Music::Addlist(std::string listfile) {
    if (!game_options()->Music) {
        return -1;
    }
    int res = -1;
    if (muzak) {
        res = muzak->_Addlist(listfile);
    }
    if (muzak) {
        for (int i = 1; i < muzak_count; i++) {
            muzak[i]._Addlist(listfile);
        }
    }
    return res;
}

int Music::_Addlist(std::string listfile) {
    if (!game_options()->Music) {
        return -1;
    }
    bool retval = LoadMusic(listfile.c_str());
    if (retval) {
        return playlist.size() - 1;
    } else {
        return -1;
    }
}

void Music::Skip(int layer) {
    if (!game_options()->Music) {
        return;
    }
    if (muzak) {
        if (layer < 0) {
            if (muzak_count >= 2) {
                muzak[muzak_cross_index]._Skip();
            } else {
                muzak->_Skip();
            }
        } else if ((layer >= 0) && (layer < muzak_count)) {
            muzak[layer]._Skip(layer);
        }
    }
}

void Music::_Skip(int layer) {
    if (game_options()->Music) {
        SelectTracks(layer);
    }
}

Music::~Music() {
    if (threadalive && thread_initialized) {
        killthread = 1;
#ifdef _WIN32
        ReleaseMutex( musicinfo_mutex );
#else
        checkerr(pthread_mutex_unlock(&musicinfo_mutex));
#endif
        int spindown = 50;         //Thread has 5 seconds to close down.
        while (threadalive && (spindown-- > 0)) {
            micro_sleep(100000);
        }
        if (threadalive) {
            threadalive = false;
        }
    }
    //Kill the thread.
}

void incmusicvol(const KBData &, KBSTATE a) {
    if (a == PRESS) {
        Music::ChangeVolume(.0625);
    }
}

void decmusicvol(const KBData &, KBSTATE a) {
    if (a == PRESS) {
        Music::ChangeVolume(-.0625);
    }
}

void Music::SetParent(Unit *parent) {
    p = parent;
}

void Music::InitMuzak() {
    muzak_count = vega_config::config->audio.music_layers;
    muzak = new Music[muzak_count];
}

void Music::CleanupMuzak() {
    if (muzak) {
        //Multithreading issues... don't care to waste time here waiting to get the lock back.
        //Let the OS clean up this mess!
        muzak = NULL;
        muzak_count = 0;
    }
}

void Music::MuzakCycle() {
    if (muzak) {
        if (BaseInterface::CurrentBase != NULL) {
            if (!BaseInterface::CurrentBase->isDJEnabled()) {
                // Bail out... they don't want us running
                return;
            }
        }
        for (int i = 0; i < muzak_count; i++) {
            muzak[i].Listen();
        }
    }
}

void Music::Stop(int layer) {
    if (muzak) {
        if (layer < 0) {
            if (muzak_count >= 2) {
                muzak[muzak_cross_index]._Stop();
            } else {
                muzak->_Stop();
            }
        } else if ((layer >= 0) && (layer < muzak_count)) {
            muzak[layer]._Stop();
        }
    }
}

void Music::_StopNow() {
    if (game_options()->Music) {
        for (std::vector<int>::const_iterator iter = sounds_to_stop.begin(); iter != sounds_to_stop.end(); iter++) {
            int sound = *iter;
            AUDStopPlaying(sound);
            AUDDeleteSound(sound, true);
        }
        sounds_to_stop.clear();
    }
}

void Music::_StopLater() {
    if (game_options()->Music) {
        for (std::list<int>::const_iterator iter = playingSource.begin(); iter != playingSource.end(); iter++) {
            int sound = *iter;
            sounds_to_stop.push_back(sound);
        }
        playingSource.clear();
    }
}

void Music::_Stop() {
    if (game_options()->Music) {
        for (std::list<int>::const_iterator iter = playingSource.begin(); iter != playingSource.end(); iter++) {
            int sound = *iter;
            AUDStopPlaying(sound);
            AUDDeleteSound(sound, true);
        }
        playingSource.clear();
    }
}

void Music::SetVolume(float vol, int layer, bool hardware, float latency_override) {
    if (muzak) {
        if (layer < 0) {
            for (int i = 0; i < muzak_count; i++) {
                muzak[i]._SetVolume(vol, hardware, latency_override);
            }
        } else if ((layer >= 0) && (layer < muzak_count)) {
            muzak[layer]._SetVolume(vol, hardware, latency_override);
        }
    }
}

void Music::Mute(bool mute, int layer) {
    static vector<float> saved_vol;
    saved_vol.resize(muzak_count, -1);
    if (!game_options()->Music) {
        return;
    }
    if (muzak) {
        const float muting_fadeout = vega_config::config->audio.music_muting_fade_out;
        const float muting_fadein = vega_config::config->audio.music_muting_fade_out;
        if (layer < 0) {
            for (int i = 0; i < muzak_count; i++) {
                if (mute) {
                    if (muzak[i].soft_vol != 0) {
                        saved_vol[i] = muzak[i].soft_vol;
                        muzak[i]._SetVolume(0, false, muting_fadeout);
                    }
                } else if (saved_vol[i] >= 0) {
                    muzak[i]._SetVolume(saved_vol[i], false, muting_fadein);
                }
            }
        } else if ((layer >= 0) && (layer < muzak_count)) {
            if (mute) {
                if (muzak[layer].soft_vol != 0) {
                    saved_vol[layer] = muzak[layer].soft_vol;
                    muzak[layer]._SetVolume(0, false, muting_fadeout);
                }
            } else if (saved_vol[layer] >= 0) {
                muzak[layer]._SetVolume(saved_vol[layer], false, muting_fadein);
            }
        }
    }
}

void Music::SetLoops(int numloops, int layer) {
    if (!game_options()->Music) {
        return;
    }
    if (muzak) {
        if (layer < 0) {
            //This only will apply to the crossfading channel (layers 0 && 1)
            SetLoops(numloops, 0);
            SetLoops(numloops, 1);
        } else if ((layer >= 0) && (layer < muzak_count)) {
            //Specific channel
            muzak[layer].loopsleft = numloops;
        }
    }
}

