/*
 * al_sound.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "src/audiolib.h"
#include "src/hashtable.h"
#include "root_generic/vsfilesystem.h"
#include "src/vs_logging.h"
#include <string>
#include "al_globals.h"
#include <stdio.h>
#include <stdlib.h>
#include "cmd/unit_generic.h"
#include "gfx_generic/cockpit_generic.h"
#include "root_generic/options.h"
#include "root_generic/posh.h"
#include "src/universe.h"

#ifdef HAVE_AL

typedef struct                                   /* WAV File-header */
{
    ALubyte Id[4];
    ALsizei Size;
    ALubyte Type[4];
} WAVFileHdr_Struct;

typedef struct                                   /* WAV Fmt-header */
{
    ALushort Format;
    ALushort Channels;
    ALuint SamplesPerSec;
    ALuint BytesPerSec;
    ALushort BlockAlign;
    ALushort BitsPerSample;
} WAVFmtHdr_Struct;

typedef struct                                                                   /* WAV FmtEx-header */
{
    ALushort Size;
    ALushort SamplesPerBlock;
} WAVFmtExHdr_Struct;

typedef struct                                   /* WAV Smpl-header */
{
    ALuint Manufacturer;
    ALuint Product;
    ALuint SamplePeriod;
    ALuint Note;
    ALuint FineTune;
    ALuint SMPTEFormat;
    ALuint SMPTEOffest;
    ALuint Loops;
    ALuint SamplerData;
    struct {
        ALuint Identifier;
        ALuint Type;
        ALuint Start;
        ALuint End;
        ALuint Fraction;
        ALuint Count;
    } Loop[1];
} WAVSmplHdr_Struct;

typedef struct                                   /* WAV Chunk-header */
{
    ALubyte Id[4];
    ALuint Size;
} WAVChunkHdr_Struct;

void SwapWords(unsigned int *puint) {
    unsigned int tempint = POSH_LittleU32(*puint);
    *puint = tempint;
}

void SwapBytes(unsigned short *pshort) {
    unsigned short tempshort = POSH_LittleU16(*pshort);
    *pshort = tempshort;
}

void blutLoadWAVMemory(ALbyte *memory, ALenum
*format, ALvoid **data, ALsizei *size, ALsizei *freq, ALboolean *loop) {
    WAVChunkHdr_Struct ChunkHdr;
    WAVFmtExHdr_Struct FmtExHdr;
    WAVFileHdr_Struct FileHdr;
    WAVSmplHdr_Struct SmplHdr;
    WAVFmtHdr_Struct FmtHdr;
    ALbyte *Stream;

    *format = AL_FORMAT_MONO16;
    *data = NULL;
    *size = 0;
    *freq = 22050;
    *loop = AL_FALSE;
    if (memory) {
        Stream = memory;
        if (Stream) {
            memcpy(&FileHdr, Stream, sizeof(WAVFileHdr_Struct));
            Stream += sizeof(WAVFileHdr_Struct);
            SwapWords((unsigned int *) &FileHdr.Size);
            FileHdr.Size = ((FileHdr.Size + 1) & ~1) - 4;
            while ((FileHdr.Size != 0) && (memcpy(&ChunkHdr, Stream, sizeof(WAVChunkHdr_Struct)))) {
                Stream += sizeof(WAVChunkHdr_Struct);
                SwapWords(&ChunkHdr.Size);
                if ((ChunkHdr.Id[0] == 'f') && (ChunkHdr.Id[1] == 'm') && (ChunkHdr.Id[2] == 't')
                        && (ChunkHdr.Id[3] == ' ')) {
                    memcpy(&FmtHdr, Stream, sizeof(WAVFmtHdr_Struct));
                    SwapBytes(&FmtHdr.Format);
                    if (FmtHdr.Format == 0x0001) {
                        SwapBytes(&FmtHdr.Channels);
                        SwapBytes(&FmtHdr.BitsPerSample);
                        SwapWords(&FmtHdr.SamplesPerSec);
                        SwapBytes(&FmtHdr.BlockAlign);

                        *format = (FmtHdr.Channels == 1
                                ? (FmtHdr.BitsPerSample == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16)
                                : (FmtHdr.BitsPerSample == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16));
                        *freq = FmtHdr.SamplesPerSec;
                        Stream += ChunkHdr.Size;
                    } else {
                        memcpy(&FmtExHdr, Stream, sizeof(WAVFmtExHdr_Struct));
                        Stream += ChunkHdr.Size;
                    }
                } else if ((ChunkHdr.Id[0] == 'd') && (ChunkHdr.Id[1] == 'a') && (ChunkHdr.Id[2] == 't')
                        && (ChunkHdr.Id[3] == 'a')) {
                    if (FmtHdr.Format == 0x0001) {
                        *size = ChunkHdr.Size;
                        if (*data == NULL) {
                            *data = malloc(ChunkHdr.Size + 31);
                        } else {
                            *data = realloc(*data, ChunkHdr.Size + 31);
                        }
                        if (*data) {
                            memcpy(*data, Stream, ChunkHdr.Size);
                            memset(((char *) *data) + ChunkHdr.Size, 0, 31);
                            Stream += ChunkHdr.Size;
                            if (FmtHdr.BitsPerSample == 16) {
                                for (size_t i = 0; i < (ChunkHdr.Size / 2); ++i) {
                                    SwapBytes(&(*(unsigned short **) data)[i]);
                                }
                            }
                        }
                    } else if (FmtHdr.Format == 0x0011) {
                        //IMA ADPCM
                    } else if (FmtHdr.Format == 0x0055) {
                        //MP3 WAVE
                    }
                } else if ((ChunkHdr.Id[0] == 's') && (ChunkHdr.Id[1] == 'm') && (ChunkHdr.Id[2] == 'p')
                        && (ChunkHdr.Id[3] == 'l')) {
                    memcpy(&SmplHdr, Stream, sizeof(WAVSmplHdr_Struct));
                    Stream += ChunkHdr.Size;
                } else {
                    Stream += ChunkHdr.Size;
                }
                Stream += ChunkHdr.Size & 1;
                FileHdr.Size -= (((ChunkHdr.Size + 1) & ~1) + 8);
            }
        }
    }
}

#if defined(__APPLE__) && defined (__MACH__)
#include <al.h>
#include <alc.h>
//#include <alut.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
//their LoadWav is b0rken seriously!!!!!!

#else
#include <AL/al.h>
#include <AL/alc.h>

#endif
//#include <AL/alext.h>
#endif
#include <vector>
#include "root_generic/vs_globals.h"
#include <algorithm>
#include <stdio.h>
#ifdef HAVE_AL
#ifdef HAVE_OGG

#include <vorbis/vorbisfile.h>
#endif
std::vector<unsigned int> dirtysounds;
std::vector<OurSound> sounds;
std::vector<ALuint> buffers;

static void convertToLittle(unsigned int tmp, char *data) {
    data[0] = (char) (tmp % 256);
    data[1] = (char) ((tmp / 256) % 256);
    data[2] = (char) ((tmp / 65536) % 256);
    data[3] = (char) ((tmp / 65536) / 256);
}

#ifdef HAVE_OGG
struct fake_file {
    char *data;
    size_t size;
    size_t loc;
};

size_t mem_read(void *ptr, size_t size, size_t nmemb, void *datasource) {
    fake_file *fp = (fake_file *) datasource;
    if (fp->loc + size > fp->size) {
        size_t tmp = fp->size - fp->loc;
        if (tmp) {
            memcpy(ptr, fp->data + fp->loc, tmp);
        }
        fp->loc = fp->size;
        return tmp;
    } else {
        memcpy(ptr, fp->data + fp->loc, size);
        fp->loc += size;
        return size;
    }
}

int mem_close(void *) {
    return 0;
}

long mem_tell(void *datasource) {
    fake_file *fp = (fake_file *) datasource;
    return (long) fp->loc;
}

int cant_seek(void *datasource, ogg_int64_t offset, int whence) {
    return -1;
}

int mem_seek(void *datasource, ogg_int64_t offset, int whence) {
    fake_file *fp = (fake_file *) datasource;
    if (whence == SEEK_END) {
        if (offset < 0) {
            if (fp->size < (size_t) -offset) {
                return -1;
            } else {
                fp->loc = fp->size + offset;
                return 0;
            }
        } else if (offset == 0) {
            fp->loc = fp->size;
        } else {
            return -1;
        }
    } else if (whence == SEEK_CUR) {
        if (offset < 0) {
            if (fp->loc < (size_t) -offset) {
                return -1;
            } else {
                fp->loc += offset;
                return 0;
            }
        } else {
            if (fp->loc + offset > fp->size) {
                return -1;
            } else {
                fp->loc += offset;
                return 0;
            }
        }
    } else if (whence == SEEK_SET) {
        if (offset > static_cast<int>(fp->size)) {
            return -1;
        }
        fp->loc = offset;
        return 0;
    }
    return -1;
}

#endif

static void ConvertFormat(vector<char> &ogg) {
    vector<char> converted;
    if (ogg.size() > 4) {
        if (ogg[0] == 'O' && ogg[1] == 'g' && ogg[2] == 'g' && ogg[3] == 'S') {
#ifdef HAVE_OGG
            OggVorbis_File vf;
            ov_callbacks callbacks;
            fake_file ff;

            ff.data = &ogg[0];
            ff.loc = 0;
            ff.size = ogg.size();
            callbacks.read_func = &mem_read;
            callbacks.seek_func = &mem_seek;
            callbacks.close_func = &mem_close;
            callbacks.tell_func = &mem_tell;
            if (ov_open_callbacks(&ff, &vf, NULL, 0, callbacks)) {
                ogg.clear();
            } else {
                long bytesread = 0;
                vorbis_info *info = ov_info(&vf, -1);
                const int segmentsize = 65536 * 32;
                const int samples = 16;
                converted.push_back('R');
                converted.push_back('I');
                converted.push_back('F');
                converted.push_back('F');
                converted.push_back(0);
                converted.push_back(0);
                converted.push_back(0);
                converted.push_back(0);                 //fill in with weight;
                converted.push_back('W');
                converted.push_back('A');
                converted.push_back('V');
                converted.push_back('E');
                converted.push_back('f');
                converted.push_back('m');
                converted.push_back('t');
                converted.push_back(' ');

                converted.push_back(18);                 //size of header (16 bytes)
                converted.push_back(0);
                converted.push_back(0);
                converted.push_back(0);

                converted.push_back(1);                 //compression code
                converted.push_back(0);

                converted.push_back((char) (info->channels % 256));                   //num channels;
                converted.push_back((char) (info->channels / 256));

                converted.push_back(0);                 //sample rate
                converted.push_back(0);                 //sample rate
                converted.push_back(0);                 //sample rate
                converted.push_back(0);                 //sample rate
                convertToLittle(info->rate, &converted[converted.size() - 4]);

                long byterate = info->rate * info->channels * samples / 8;
                converted.push_back(0);                 //bytes per second rate
                converted.push_back(0);
                converted.push_back(0);
                converted.push_back(0);
                convertToLittle(byterate, &converted[converted.size() - 4]);

                converted.push_back((char) ((info->channels * samples / 8)
                        % 256));                   //num_channels*16 bits/8
                converted.push_back((char) ((info->channels * samples / 8) / 256));

                converted.push_back(samples);                 //16 bit samples
                converted.push_back(0);
                converted.push_back(0);
                converted.push_back(0);

                //PCM header
                converted.push_back('d');
                converted.push_back('a');
                converted.push_back('t');
                converted.push_back('a');

                converted.push_back(0);
                converted.push_back(0);
                converted.push_back(0);
                converted.push_back(0);
                ogg_int64_t pcmsizestart = converted.size();
                converted.resize(converted.size() + segmentsize);
                int signedvalue = 1;
                int bitstream = 0;
                while ((bytesread =
                        ov_read(&vf,
                                &converted[converted.size() - segmentsize],
                                segmentsize,
                                0,
                                samples / 8,
                                signedvalue,
                                &bitstream)) > 0) {
                    int numtoerase = 0;
                    if (bytesread < segmentsize) {
                        numtoerase = segmentsize - bytesread;
                    }

                    converted.resize(converted.size() + segmentsize - numtoerase);
                }
                converted.resize(converted.size() - segmentsize);
                convertToLittle(converted.size() - 8, &converted[4]);
                convertToLittle(converted.size() - pcmsizestart, &converted[pcmsizestart - 4]);

                converted.swap(ogg);
            }
            ov_clear(&vf);
#else
            ogg.clear();
#endif
        }
    }
}

static int LoadSound(ALuint buffer, bool looping, bool music) {
    unsigned int i;
    if (!dirtysounds.empty()) {
        i = dirtysounds.back();
        dirtysounds.pop_back();
        //assert (sounds[i].buffer==(ALuint)0);
        //if (sounds[i].buffer != (ALuint)0) {
        //    VS_LOG(trace, (boost::format("using claimed buffer %1%") % sounds[i].buffer));
        //}
        sounds[i].buffer = buffer;
    } else {
        i = sounds.size();
        sounds.push_back(OurSound(0, buffer));
    }
    sounds[i].source = (ALuint) 0;
    sounds[i].looping = looping ? AL_TRUE : AL_FALSE;
    sounds[i].music = music;
#ifdef SOUND_DEBUG
    VS_LOG(trace, (boost::format(" with buffer %1% and looping property %2%") % i % ((int) looping)));
#endif
    //limited number of sources
    //alGenSources( 1, &sounds[i].source);
    //alSourcei(sounds[i].source, AL_BUFFER, buffer );
    //alSourcei(sounds[i].source, AL_LOOPING, looping ?AL_TRUE:AL_FALSE);
    return i;
}

#endif

using namespace VSFileSystem;

bool AUDLoadSoundFile(const char *s, struct AUDSoundProperties *info, bool use_fileptr) {
    VS_LOG(trace, (boost::format("Loading sound file %1%") % s));

    info->success = false;

#ifdef HAVE_AL
    vector<char> dat;
    if (use_fileptr) {
        FILE *f = fopen(s, "rb");
        if (!f) {
            std::string path = std::string("sounds/") + s;
            f = fopen(path.c_str(), "rb");
        }
        if (!f) {
            std::string path = std::string("music/") + s;
            f = fopen(path.c_str(), "rb");
        }
        if (f) {
            fseek(f, 0, SEEK_END);
            size_t siz = ftell(f);
            fseek(f, 0, SEEK_SET);
            dat.resize(siz);
            if (fread(&dat[0], 1, siz, f) != siz) {
                return false;
            }
            info->hashname = s;
            info->shared = false;
            fclose(f);
        } else {
            return false;
        }
    } else {
        VSFile f;
        VSError error = f.OpenReadOnly(s, SoundFile);
        if (error > Ok) {
            error = f.OpenReadOnly(s, UnknownFile);
        }
        info->shared = (error == Shared);
        if (info->shared) {
            info->hashname = VSFileSystem::GetSharedSoundHashName(s);
        } else {
            info->hashname = VSFileSystem::GetHashName(s);
        }
        if (error > Ok) {
            return false;
        }
#ifdef SOUND_DEBUG
        VS_LOG(trace, (boost::format("Sound %1$s created with and alBuffer %2$d") % s, % wavbuf));
#endif
        dat.resize(f.Size());
        f.Read(&dat[0], f.Size());
        f.Close();
    }
    ConvertFormat(dat);
    if (dat.size() == 0) {          //conversion messed up
        return false;
    }
    //blutLoadWAVMemory((ALbyte *)&dat[0], &format, &wave, &size, &freq, &looping);

    blutLoadWAVMemory((ALbyte *) &dat[0], &info->format, &info->wave, &info->size, &info->freq, &info->looping);
    if (!info->wave) {
        return false;          //failure.
    }

    info->success = true;
    return true;
#else
    return false;
#endif
}

int AUDBufferSound(const struct AUDSoundProperties *info, bool music) {
#ifdef HAVE_AL
    ALuint wavbuf = 0;
    alGenBuffers(1, &wavbuf);
    if (!wavbuf) {
        VS_LOG(error, (boost::format("OpenAL Error in alGenBuffers: %1$d") % alGetError()));
    }
    alBufferData(wavbuf, info->format, info->wave, info->size, info->freq);
    return LoadSound(wavbuf, info->looping, music);
#else
    return -1;
#endif
}

#ifdef HAVE_AL
ALuint
#else
unsigned int
#endif
        nil_wavebuf = 0;

int AUDCreateSoundWAV(const std::string &s, const bool music, const bool LOOP) {
#ifdef HAVE_AL
#ifdef SOUND_DEBUG
    VS_LOG(trace, "AUDCreateSoundWAV:: ");
#endif
    if ((game_options()->Music && !music) || (game_options()->Music && music)) {
        ALuint *wavbuf = NULL;
        std::string hashname;
        if (!music) {
            hashname = VSFileSystem::GetHashName(s);
            wavbuf = soundHash.Get(hashname);
            if (!wavbuf) {
                hashname = VSFileSystem::GetSharedSoundHashName(s);
                wavbuf = soundHash.Get(hashname);
            }
            if (wavbuf == &nil_wavebuf) {
                return -1;
            }                  //404
        }
        if (wavbuf) {
#ifdef SOUND_DEBUG
            VS_LOG(trace, (boost::format("Sound %1$s restored with alBuffer %2$d") % s % *wavbuf));
#endif
        }
        if (wavbuf == NULL) {
            AUDSoundProperties info;
            if (!AUDLoadSoundFile(s.c_str(), &info)) {
                soundHash.Put(info.hashname, &nil_wavebuf);
                return -1;
            }
            wavbuf = (ALuint *) malloc(sizeof(ALuint));
            alGenBuffers(1, wavbuf);
            alBufferData(*wavbuf, info.format, info.wave, info.size, info.freq);
            free(info.wave);             //alutUnloadWAV(format,wave,size,freq);
            if (!music) {
                soundHash.Put(info.hashname, wavbuf);
                buffers.push_back(*wavbuf);
            }
        }
        return LoadSound(*wavbuf, LOOP, music);
    }
#endif
    return -1;
}

int AUDCreateSoundWAV(const std::string &s, const bool LOOP) {
    return AUDCreateSoundWAV(s, false, LOOP);
}

int AUDCreateMusicWAV(const std::string &s, const bool LOOP) {
    return AUDCreateSoundWAV(s, true, LOOP);
}

int AUDCreateSoundMP3(const std::string &s, const bool music, const bool LOOP) {
#ifdef HAVE_AL
    assert(0);
    if ((game_options()->Music && !music) || (game_options()->Music && music)) {
        VSFile f;
        VSError error = f.OpenReadOnly(s.c_str(), SoundFile);
        bool shared = (error == Shared);
        std::string nam(s);
        ALuint *mp3buf = NULL;
        std::string hashname;
        if (!music) {
            hashname = shared ? VSFileSystem::GetSharedSoundHashName(s) : VSFileSystem::GetHashName(s);
            mp3buf = soundHash.Get(hashname);
        }
        if (error > Ok) {
            return -1;
        }
#ifdef _WIN32
        return -1;
#endif
        if (mp3buf == NULL) {
            char *data = new char[f.Size()];
            f.Read(data, f.Size());
            mp3buf = (ALuint *) malloc(sizeof(ALuint));
            alGenBuffers(1, mp3buf);
            /*
             *  if ((*alutLoadMP3p)(*mp3buf,data,f.Size())!=AL_TRUE) {
             *     delete []data;
             *         return -1;
             *         }*/
            delete[] data;
            if (!music) {
                soundHash.Put(hashname, mp3buf);
                buffers.push_back(*mp3buf);
            }
        } else {
            f.Close();
        }
        return LoadSound(*mp3buf, LOOP, music);
    }
#endif
    return -1;
}

int AUDCreateSoundMP3(const std::string &s, const bool LOOP) {
    return AUDCreateSoundMP3(s, false, LOOP);
}

int AUDCreateMusicMP3(const std::string &s, const bool LOOP) {
    return AUDCreateSoundMP3(s, true, LOOP);
}

int AUDCreateSound(const std::string &s, const bool LOOP) {
    if (s.end() - 1 >= s.begin()) {
        if (*(s.end() - 1) == '3') {
            return AUDCreateSoundMP3(s, LOOP);
        } else {
            return AUDCreateSoundWAV(s, LOOP);
        }
    }
    return -1;
}

int AUDCreateMusic(const std::string &s, const bool LOOP) {
    if (s.end() - 1 >= s.begin()) {
        if (*(s.end() - 1) == 'v') {
            return AUDCreateMusicWAV(s, LOOP);
        } else {
            return AUDCreateMusicMP3(s, LOOP);
        }
    }
    return -1;
}

///copies other sound loaded through AUDCreateSound
int AUDCreateSound(int sound, const bool LOOP /*=false*/ ) {
#ifdef HAVE_AL
    if (AUDIsPlaying(sound)) {
        AUDStopPlaying(sound);
    }
    if (sound >= 0 && sound < (int) sounds.size()) {
        return LoadSound(sounds[sound].buffer, LOOP, false);
    }
#endif
    return -1;
}

extern std::vector<int> soundstodelete;

void AUDDeleteSound(int sound, bool music) {
#ifdef HAVE_AL
    if (sound >= 0 && sound < (int) sounds.size()) {
        if (AUDIsPlaying(sound)) {
            if (!music) {
#ifdef SOUND_DEBUG
                VS_LOG(trace, (boost::format("AUDDeleteSound: Sound Playing enqueue soundstodelete %1$d %2$d")
                        % sounds[sound].source
                        % sounds[sound].buffer));
#endif
                soundstodelete.push_back(sound);
                return;
            } else {
                AUDStopPlaying(sound);
            }
        }
#ifdef SOUND_DEBUG
        VS_LOG(debug, (boost::format("AUDDeleteSound: Sound Not Playing push back to unused src %1$d %2$d") % sounds[sound].source % sounds[sound].buffer));
#endif
        if (sounds[sound].source) {
            unusedsrcs.push_back(sounds[sound].source);
            alSourcei(sounds[sound].source, AL_BUFFER, 0);             //decrement the source refcount
            sounds[sound].source = (ALuint) 0;
        }
#ifdef SOUND_DEBUG
        if ( std::find( dirtysounds.begin(), dirtysounds.end(), sound ) == dirtysounds.end() ) {
#endif
        dirtysounds.push_back(sound);
#ifdef SOUND_DEBUG
        } else {
            VS_LOG(error, (boost::format("double delete of sound %1$d") % sound));
            return;
        }
#endif
        //FIXME??
        //alDeleteSources(1,&sounds[sound].source);
        if (music) {
            alDeleteBuffers(1, &sounds[sound].buffer);
        }
        sounds[sound].buffer = (ALuint) 0;
    }
#endif
}

void AUDAdjustSound(const int sound, const QVector &pos, const Vector &vel) {

#ifdef HAVE_AL
    if (sound >= 0 && sound < (int) sounds.size()) {
        float p[] = {
                static_cast<float>(scalepos * pos.i),
                static_cast<float>(scalepos * pos.j),
                static_cast<float>(scalepos * pos.k)
        };
        float v[] = {scalevel * vel.i, scalevel * vel.j, scalevel * vel.k};
        sounds[sound].pos = pos.Cast();
        sounds[sound].vel = vel;
        if (usepositional && sounds[sound].source) {
            alSourcefv(sounds[sound].source, AL_POSITION, p);
            bool relative = (p[0] == 0 && p[1] == 0 && p[2] == 0);
            alSourcei(sounds[sound].source, AL_SOURCE_RELATIVE, relative);
            if (!relative) {
                // Set rolloff factrs
                alSourcef(sounds[sound].source, AL_MAX_DISTANCE, scalepos * game_options()->audio_max_distance);
                alSourcef(sounds[sound].source, AL_REFERENCE_DISTANCE, scalepos * game_options()->audio_ref_distance);
                alSourcef(sounds[sound].source, AL_ROLLOFF_FACTOR, 1.f);
            }
        }
        if (usedoppler && sounds[sound].source) {
            alSourcefv(sounds[sound].source, AL_VELOCITY, v);
        }
    }
#endif
}

void AUDStreamingSound(const int sound) {
#ifdef HAVE_AL
    if (sound >= 0 && sound < (int) sounds.size() && sounds[sound].source) {
        alSource3f(sounds[sound].source, AL_POSITION, 0.0, 0.0, 0.0);
        alSource3f(sounds[sound].source, AL_VELOCITY, 0.0, 0.0, 0.0);
        alSource3f(sounds[sound].source, AL_DIRECTION, 0.0, 0.0, 0.0);
        alSourcef(sounds[sound].source, AL_ROLLOFF_FACTOR, 0.0);
        alSourcei(sounds[sound].source, AL_SOURCE_RELATIVE, AL_TRUE);
    }
#endif
}

bool starSystemOK() {
    if (!_Universe || !_Universe->AccessCockpit(0)) {
        return true;
    }          //No Universe yet, so game is loading.

    Unit *playa = _Universe->AccessCockpit(0)->GetParent();
    if (!playa) {
        return false;
    }
    return playa->getStarSystem() == _Universe->activeStarSystem();
}

int AUDHighestSoundPlaying() {
    int retval = -1;
#ifdef HAVE_AL
    unsigned int s = ::sounds.size();
    for (unsigned int i = 0; i < s; ++i) {
        if (false == ::sounds[i].music && AUDIsPlaying(i) && false == ::sounds[i].looping) {
            retval = i;
        }
    }
#endif
    return retval;
}

void AUDStopAllSounds(int except_this_one) {
#ifdef HAVE_AL
    unsigned int s = ::sounds.size();
    for (unsigned int i = 0; i < s; ++i) {
        if ((int) i != except_this_one && false == ::sounds[i].music && AUDIsPlaying(i)) {
            AUDStopPlaying(i);
        }
    }
#endif
}

bool AUDIsPlaying(const int sound) {
#ifdef HAVE_AL
    if (sound >= 0 && sound < (int) sounds.size()) {
        if (!sounds[sound].source) {
            return false;
        }
        ALint state;
#if defined (_WIN32) || defined (__APPLE__)
        alGetSourcei( sounds[sound].source, AL_SOURCE_STATE, &state );         //Obtiene el estado de la fuente para windows
#else
        alGetSourceiv(sounds[sound].source, AL_SOURCE_STATE, &state);
#endif

        return state == AL_PLAYING;
    }
#endif
    return false;
}

void AUDStopPlaying(const int sound) {
#ifdef HAVE_AL
    if (sound >= 0 && sound < (int) sounds.size()) {
#ifdef SOUND_DEBUG
        VS_LOG(trace, (boost::format("AUDStopPlaying sound %1$d source(releasing): %2$d buffer: %3$d")
                                    % sound % sounds[sound].source % sounds[sound].buffer));
#endif
        if (sounds[sound].source != 0) {
            alSourceStop(sounds[sound].source);
            unusedsrcs.push_back(sounds[sound].source);
            alSourcei(sounds[sound].source, AL_BUFFER, 0);             //decrement refcount
        }
        sounds[sound].source = (ALuint) 0;
    }
#endif
}

static bool AUDReclaimSource(const int sound, bool high_priority = false) {
#ifdef HAVE_AL
    if (sounds[sound].source == (ALuint) 0) {
        if (!sounds[sound].buffer) {
            return false;
        }
        if (unusedsrcs.empty()) {
            if (high_priority) {
                unsigned int i;
                unsigned int candidate = 0;
                bool found = false;
                for (i = 0; i < sounds.size(); ++i) {
                    if (sounds[i].source != 0) {
                        if (sounds[i].pos.i != 0 || sounds[i].pos.j != 0 || sounds[i].pos.k != 0) {
                            if (found) {
                                if (AUDDistanceSquared(candidate) < AUDDistanceSquared(i)) {
                                    candidate = i;
                                }
                            } else {
                                candidate = i;
                            }
                            found = true;
                        }
                    }
                }
                if (!found) {
                    return false;
                } else {
                    alSourceStop(sounds[candidate].source);
                    sounds[sound].source = sounds[candidate].source;
                    alSourcei(sounds[candidate].source, AL_BUFFER, 0);                     //reclaim the source
                    sounds[candidate].source = 0;
                }
            } else {
                return false;
            }
        } else {
            sounds[sound].source = unusedsrcs.back();
            unusedsrcs.pop_back();
        }
        alSourcei(sounds[sound].source, AL_BUFFER, sounds[sound].buffer);
        alSourcei(sounds[sound].source, AL_LOOPING, sounds[sound].looping);
    }
    return true;
#endif
    return false;     //silly
}

void AUDStartPlaying(const int sound) {
#ifdef SOUND_DEBUG
    VS_LOG(trace, (boost::format("AUDStartPlaying(%1$d)") % sound));
#endif

#ifdef HAVE_AL
    if (sound >= 0 && sound < (int) sounds.size()) {
        if (sounds[sound].music || starSystemOK()) {
            if (AUDReclaimSource(sound, sounds[sound].pos == QVector(0, 0, 0))) {
#ifdef SOUND_DEBUG
                VS_LOG(trace, (boost::format("AUDStartPlaying sound %1$d source: %2$d buffer: %3$d")
                                            % sound % sounds[sound].source % sounds[sound].buffer));
#endif
                AUDAdjustSound(sound, sounds[sound].pos, sounds[sound].vel);
                AUDSoundGain(sound, sounds[sound].gain, sounds[sound].music);
                alSourcePlay(sounds[sound].source);
            }
        }
    }
#endif
}

void AUDPlay(const int sound, const QVector &pos, const Vector &vel, const float gain) {
#ifdef HAVE_AL
    char tmp;
    if (sound < 0) {
        return;
    }
    if (sounds[sound].buffer == 0) {
        return;
    }
    if (!starSystemOK() && !sounds[sound].music) {
        return;
    }
    if (AUDIsPlaying(sound)) {
        AUDStopPlaying(sound);
    }
    if ((tmp = AUDQueryAudability(sound, pos.Cast(), vel, gain)) != 0) {
        if (AUDReclaimSource(sound, pos == QVector(0, 0, 0))) {
            AUDAdjustSound(sound, pos, vel);
            AUDSoundGain(sound, gain, sounds[sound].music);
            if (tmp != 2) {
                VS_LOG(trace, (boost::format("AUDPlay sound %1% %2%") % sounds[sound].source % sounds[sound].buffer));
                AUDAddWatchedPlayed(sound, pos.Cast());
            } else {
                VS_LOG(trace,
                        (boost::format("AUDPlay stole sound %1% %2%") % sounds[sound].source % sounds[sound].buffer));
                alSourceStop(sounds[sound].source);
            }
            alSourcePlay(sounds[sound].source);
        }
    }
#endif
}

#ifndef AL_SEC_OFFSET
/* Supported on Windows, but the headers might be out of date. */
#define AL_SEC_OFFSET 0x1024
#endif

float AUDGetCurrentPosition(const int sound) {
#ifdef HAVE_AL
    ALfloat rv;
    alGetSourcef(sound, AL_SEC_OFFSET, &rv);
    return float(rv);
#else
    return 0;
#endif
}

void AUDPausePlaying(const int sound) {
#ifdef HAVE_AL
    if (sound >= 0 && sound < (int) sounds.size()) {
        //alSourcePlay( sounds[sound].source() );
    }
#endif
}
