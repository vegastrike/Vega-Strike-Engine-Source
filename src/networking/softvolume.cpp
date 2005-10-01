#if !defined (SDL_MIX_MAXVOLUME)
#define SDL_MIX_MAXVOLUME 128
#endif
#ifdef HAVE_SDL
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mixer.h>
#else
typedef int Mix_Music;
#endif

#include <stdlib.h>
#include <map>
#include <memory.h>
#include <stdio.h>
#include <math.h>
#include <limits>

#include "softvolume.h"

#define UC8_MIN ((unsigned long)std::numeric_limits<unsigned char>::min())
#define UC8_MAX ((unsigned long)std::numeric_limits<unsigned char>::max())
#define SC8_MIN ((signed long)std::numeric_limits<signed char>::min())
#define SC8_MAX ((signed long)std::numeric_limits<signed char>::max())
#define US16_MIN ((unsigned long)std::numeric_limits<unsigned short>::min())
#define US16_MAX ((unsigned long)std::numeric_limits<unsigned short>::max())
#define SS16_MIN ((signed long)std::numeric_limits<signed short>::min())
#define SS16_MAX ((signed long)std::numeric_limits<signed short>::max())

// -144db is, for all practical purposes, -inf
#define DB_INF -144.0

#if defined(_WIN32)&&defined(_WINDOWS)
FILE *anotherstdout=stdout;
#define STD_ERR anotherstdout
#define STD_OUT anotherstdout
#else
#define STD_ERR stderr
#define STD_OUT stdout
#endif


#define SHAPE_SAMPLE_ORDER 10
#define SHAPE_SAMPLES (1<<SHAPE_SAMPLE_ORDER)
#define SHAPE_SAMPLE_SAFEPOS(p) ((p)&((1<<SHAPE_SAMPLE_ORDER)-1))

#define RNG_COUNT (1<<16)
#define RNG_MASK ((1<<16)-1)

template<typename T> T min(const T a, const T b)
{   if (a<b) return a; else return b;   }

template<typename T> T max(const T a, const T b)
{   if (a>b) return a; else return b;   }

class t_rng16{
public:
    t_rng16() : pos(0) { for (int i=0; i<RNG_COUNT; i++) _data[i]=(rand()&0xFFFF) - 0x8000; }; //NOTE: It would be a good idea to perform a highpass - perhaps later

    signed long get() { return _data[pos]; pos = (pos+1)&RNG_MASK; };

private:
    signed long _data[RNG_COUNT]; //wastes memory, but improves performance
    int pos;
} rng16;

#if (defined(SDL_HAS_64BIT_TYPE) && !defined(USE_FAST_F16MATH))

inline unsigned long interpolateF16F16(unsigned long a, unsigned long b, unsigned long t)
{   return (unsigned long)((a*(SDL_HAS_64BIT_TYPE)(0x10000-t) + b*(SDL_HAS_64BIT_TYPE)t) >> 16);   }

#else

#if !defined(USE_FAST_F16MATH)
#pragma message ( "64-bit type unavailable - softvolume will have lower quality" )
#endif

inline unsigned long interpolateF16F16(unsigned long a, unsigned long b, unsigned long t)
{   return (a>>8)*((0x10000-t)>>8) + (b>>8)*(t>>8);   }

#endif

class shape_sampler{
public:
    shape_sampler() : pos(0),step(0) {};

    void init(unsigned long total_samples) {
        if (total_samples==0) total_samples=1;
        this->step = (SHAPE_SAMPLES<<16) / total_samples;
        this->pos  = 0;
    };
    void close() { this->pos=this->step=0; } 

    void operator++() { this->pos += this->step; };
    int done() { return ((this->pos >> 16) & ~((1<<SHAPE_SAMPLE_ORDER)-1)) != 0; };
    unsigned long operator*() { return interpolateF16F16(shape[SHAPE_SAMPLE_SAFEPOS(this->pos >> 16)],shape[SHAPE_SAMPLE_SAFEPOS(this->pos >> 16) + 1],this->pos&0xFFFF); };
    unsigned long finalsample() const { return shape[SHAPE_SAMPLES]; };
    
private:
    unsigned long pos; //16.16 fixed point
    unsigned long step;//16.16 fixed point

public:
    unsigned long shape[SHAPE_SAMPLES+1];
};

inline unsigned long dtof16(double f) { return (unsigned long)max(0.0f,min((float)0xFFFFFFFF,(float)(f*0x10000))); };
inline double f16tod(unsigned long f16) { return f16/(double)0x10000; };

#define LOG_10 2.3025850929940456840179914546844
#define INV_LOG_10 (1.0/LOG_10)
#define C_PI 3.1415926535897932384626433832795

inline double log2linear(double lg) { return exp(lg*0.05*LOG_10); };
inline double linear2log(double lin){ return (lin<=0)?DB_INF:(20*log(lin)*INV_LOG_10); };

#if (defined(SDL_HAS_64BIT_TYPE) && !defined(USE_FAST_F16MATH))

inline unsigned short mpyUS16F16(unsigned short a, unsigned long f16)
{   return (unsigned short)min((unsigned SDL_HAS_64BIT_TYPE)US16_MAX,max((unsigned SDL_HAS_64BIT_TYPE)US16_MIN, (unsigned SDL_HAS_64BIT_TYPE)(((((((signed SDL_HAS_64BIT_TYPE)a)-0x8000)*f16) + rng16.get()) >> 16) + 0x8000) ));   }

inline signed short mpySS16F16(signed short a, unsigned long f16)
{   return (signed short)min((signed SDL_HAS_64BIT_TYPE)SS16_MAX,max((signed SDL_HAS_64BIT_TYPE)SS16_MIN,((signed SDL_HAS_64BIT_TYPE)((((signed SDL_HAS_64BIT_TYPE)a))*f16) + rng16.get()) / (1<<16) ));   }

#else

#if !defined(USE_FAST_F16MATH)
#pragma message ( "64-bit type unavailable - softvolume will have lower quality" )
#endif

inline unsigned short mpyUS16F16(unsigned short a, unsigned long f16)
{   return (unsigned short)min(US16_MAX,max(US16_MIN, (unsigned long)(((((((signed long)a)-0x8000)*(f16>>8)) + rng16.get()) >> 8) + 0x8000) ));   }

inline signed short mpySS16F16(signed short a, unsigned long f16)
{   return (signed short)min(SS16_MAX,max(SS16_MIN,((signed long)((((signed long)a))*(f16>>8)) + rng16.get()) / (1<<8)) ));   }

#endif

inline unsigned char mpyUC8F16(unsigned char a, unsigned long f16)
{   return (unsigned char)min(UC8_MAX,max(UC8_MIN, (unsigned long)(((((((signed long)a)-0x80)*f16) + rng16.get()) >> 16) + 0x80) ));   }

inline signed char mpySC8F16(signed char a, unsigned long f16)
{   return (signed char)min(SC8_MAX,max(SC8_MIN, ((signed long)((((signed long)a)*f16) + rng16.get()) / (1<<16)) ));   }

#if (SDL_BYTEORDER==SDL_LIL_ENDIAN)
inline signed short big_endian_to_native(signed short x) { return (x>>8)|(x<<8); };
inline unsigned short big_endian_to_native(unsigned short x) { return (x>>8)|(x<<8); };
#define lil_endian_to_native(x) x
#define native_to_big_endian(x) big_endian_to_native(x)
#define native_to_lil_endian(x) lil_endian_to_native(x)
#else
inline signed short lil_endian_to_native(signed short x) { return (x>>8)|(x<<8); };
inline unsigned short lil_endian_to_native(unsigned short x) { return (x>>8)|(x<<8); };
#define big_endian_to_native(x) x
#define native_to_big_endian(x) big_endian_to_native(x)
#define native_to_lil_endian(x) lil_endian_to_native(x)
#endif

typedef struct t_SoftVolume_State {
    t_SoftVolume_State() : current_volume(0x10000), in_transition(0), autoStopMusic(0) {};

    unsigned long current_volume; //16.16 fixed point

    int autoStopMusic; //nonzero: stop when volume reaches 0

    int in_transition; //nonzero: transition in progress, otherwise, stable at current_volume
    shape_sampler t_shape;
} SoftVolume_State;


std::map<int,SoftVolume_State> channel_state;
int g_sdl_frequency;
Uint16 g_sdl_format;
int g_sdl_channels;
int g_sdl_init=0;
Mix_EffectFunc_t g_sdl_effect_func=0;

void sdl_softvolume_effect_U8(int chan, void *stream, int len, void *)
{
    SoftVolume_State &cstat=channel_state[chan];
    unsigned char *buf=(unsigned char*)stream;

    if (cstat.in_transition) {
        while (!cstat.t_shape.done()&&(len>0)) {
            for (int i=g_sdl_channels; i>0; i--, buf++, len-=sizeof(*buf))
                *buf = mpyUC8F16(*buf,*cstat.t_shape);
            ++cstat.t_shape;
        }
        cstat.current_volume = (cstat.t_shape.done()?cstat.t_shape.finalsample():*cstat.t_shape);
        cstat.in_transition = !cstat.t_shape.done();
    }
    if (cstat.current_volume != 0x10000) while (len>0) {
        *buf = mpyUC8F16(*buf,cstat.current_volume);
        buf++, len -= sizeof(*buf);
    }
    if ((cstat.current_volume == 0)&&cstat.autoStopMusic)
        Mix_HaltMusic();
}
void sdl_softvolume_effect_S8(int chan, void *stream, int len, void *)
{
    SoftVolume_State &cstat=channel_state[chan];
    signed char *buf=(signed char*)stream;

    if (cstat.in_transition) {
        while (!cstat.t_shape.done()&&(len>0)) {
            for (int i=g_sdl_channels; i>0; i--, buf++, len-=sizeof(*buf))
                *buf = mpySC8F16(*buf,*cstat.t_shape);
            ++cstat.t_shape;
        }
        cstat.current_volume = (cstat.t_shape.done()?cstat.t_shape.finalsample():*cstat.t_shape);
        cstat.in_transition = !cstat.t_shape.done();
    }
    if (cstat.current_volume == 0) {
        memset(buf,0,len);
    } else if (cstat.current_volume != 0x10000) while (len>0) {
        *buf = mpySC8F16(*buf,cstat.current_volume);
        buf++, len -= sizeof(*buf);
    }
    if ((cstat.current_volume == 0)&&cstat.autoStopMusic)
        Mix_HaltMusic();
}
void sdl_softvolume_effect_U16LSB(int chan, void *stream, int len, void *)
{
    SoftVolume_State &cstat=channel_state[chan];
    unsigned short *buf=(unsigned short*)stream;
    if (cstat.in_transition) {
        while (!cstat.t_shape.done()&&(len>0)) {
            for (int i=g_sdl_channels; i>0; i--, buf++, len-=sizeof(*buf))
                *buf = native_to_lil_endian(mpyUS16F16(lil_endian_to_native(*buf),*cstat.t_shape));
            ++cstat.t_shape;
        }
        cstat.current_volume = (cstat.t_shape.done()?cstat.t_shape.finalsample():*cstat.t_shape);
        cstat.in_transition = !cstat.t_shape.done();
    }
    if (cstat.current_volume == 0) {
        unsigned short ct=native_to_lil_endian((unsigned short)0x8000);
        while (len>0) *(buf++)=ct,len-=sizeof(*buf);
    } else if (cstat.current_volume != 0x10000) while (len>0) {
        *buf = native_to_lil_endian(mpyUS16F16(lil_endian_to_native(*buf),cstat.current_volume));
        buf++, len -= sizeof(*buf);
    }
    if ((cstat.current_volume == 0)&&cstat.autoStopMusic)
        Mix_HaltMusic();
}

void sdl_softvolume_effect_S16LSB(int chan, void *stream, int len, void *)
{
    SoftVolume_State &cstat=channel_state[chan];
    signed short *buf=(signed short*)stream;
    if (cstat.in_transition) {
        while (!cstat.t_shape.done()&&(len>0)) {
            for (int i=g_sdl_channels; i>0; i--, buf++, len-=sizeof(*buf))
                *buf = native_to_lil_endian(mpySS16F16(lil_endian_to_native(*buf),*cstat.t_shape));
            ++cstat.t_shape;
        }
        cstat.current_volume = (cstat.t_shape.done()?cstat.t_shape.finalsample():*cstat.t_shape);
        cstat.in_transition = !cstat.t_shape.done();
    }
    if (cstat.current_volume == 0) {
        memset(buf,0,len);
    } else if (cstat.current_volume != 0x10000) while (len>0) {
        *buf = native_to_lil_endian(mpySS16F16(lil_endian_to_native(*buf),cstat.current_volume));
        buf++, len -= sizeof(*buf);
    }
    if ((cstat.current_volume == 0)&&cstat.autoStopMusic)
        Mix_HaltMusic();
}
void sdl_softvolume_effect_U16MSB(int chan, void *stream, int len, void *)
{
    SoftVolume_State &cstat=channel_state[chan];
    unsigned short *buf=(unsigned short*)stream;
    if (cstat.in_transition) {
        while (!cstat.t_shape.done()&&(len>0)) {
            for (int i=g_sdl_channels; i>0; i--, buf++, len-=sizeof(*buf))
                *buf = native_to_big_endian(mpyUS16F16(big_endian_to_native(*buf),*cstat.t_shape));
            ++cstat.t_shape;
        }
        cstat.current_volume = (cstat.t_shape.done()?cstat.t_shape.finalsample():*cstat.t_shape);
        cstat.in_transition = !cstat.t_shape.done();
    }
    if (cstat.current_volume == 0) {
        unsigned short ct=native_to_big_endian((unsigned short)0x8000);
        while (len>0) *(buf++)=ct,len-=sizeof(*buf);
    } else if (cstat.current_volume != 0x10000) while (len>0) {
        *buf = native_to_big_endian(mpyUS16F16(big_endian_to_native(*buf),cstat.current_volume));
        buf++, len -= sizeof(*buf);
    }
    if ((cstat.current_volume == 0)&&cstat.autoStopMusic)
        Mix_HaltMusic();
}
void sdl_softvolume_effect_S16MSB(int chan, void *stream, int len, void *)
{
    SoftVolume_State &cstat=channel_state[chan];
    signed short *buf=(signed short*)stream;
    if (cstat.in_transition) {
        while (!cstat.t_shape.done()&&(len>0)) {
            for (int i=g_sdl_channels; i>0; i--, buf++, len-=sizeof(*buf))
                *buf = native_to_big_endian(mpySS16F16(big_endian_to_native(*buf),*cstat.t_shape));
            ++cstat.t_shape;
        }
        cstat.current_volume = (cstat.t_shape.done()?cstat.t_shape.finalsample():*cstat.t_shape);
        cstat.in_transition = !cstat.t_shape.done();
    }
    if (cstat.current_volume == 0) {
        memset(buf,0,len);
    } else if (cstat.current_volume != 0x10000) while (len>0) {
        *buf = native_to_big_endian(mpySS16F16(big_endian_to_native(*buf),cstat.current_volume));
        buf++, len -= sizeof(*buf);
    }
    if ((cstat.current_volume == 0)&&cstat.autoStopMusic)
        Mix_HaltMusic();
}

void Mix_SoftVolume_Init()
{
    // Query device format
    if (!g_sdl_init) {
        Mix_QuerySpec(&g_sdl_frequency,&g_sdl_format,&g_sdl_channels);
        switch (g_sdl_format) {
        case AUDIO_U8:      g_sdl_effect_func = &sdl_softvolume_effect_U8; break;
        case AUDIO_S8:      g_sdl_effect_func = &sdl_softvolume_effect_S8; break;
        case AUDIO_U16LSB:  g_sdl_effect_func = &sdl_softvolume_effect_U16LSB; break;
        case AUDIO_S16LSB:  g_sdl_effect_func = &sdl_softvolume_effect_S16LSB; break;
        case AUDIO_U16MSB:  g_sdl_effect_func = &sdl_softvolume_effect_U16MSB; break;
        case AUDIO_S16MSB:  g_sdl_effect_func = &sdl_softvolume_effect_S16MSB; break;
        }
        g_sdl_init=1;
    }
}

void Mix_SoftVolume_Change(int chan, double newvolume, double time, Mix_SoftVolume_Shape shape)
{
    Mix_SoftVolume_Init();

    // Setup transition record
    {
        SDL_LockAudio();

        SoftVolume_State &cstat = channel_state[chan];

        double cur_volume = f16tod(cstat.current_volume);
        double new_volume = newvolume;

        int i;
        cstat.in_transition = true;
        switch (shape) {
        case MIX_SV_SHAPE_LINEAR:
mix_sv_shape_linear:
            for (i=0; i<=SHAPE_SAMPLES; i++) cstat.t_shape.shape[i] = dtof16( cur_volume + (new_volume-cur_volume) * (i*(1.0/SHAPE_SAMPLES)) );
            break;
        case MIX_SV_SHAPE_EXP:
            cur_volume = linear2log(cur_volume);
            new_volume = linear2log(new_volume);
            for (i=0; i<=SHAPE_SAMPLES; i++) cstat.t_shape.shape[i] = dtof16( log2linear( cur_volume + (new_volume-cur_volume) * (i*(1.0/SHAPE_SAMPLES)) ) );
            break;
        case MIX_SV_SHAPE_EASED:
            for (i=0; i<=SHAPE_SAMPLES; i++) cstat.t_shape.shape[i] = dtof16( cur_volume + (new_volume-cur_volume) * (0.5-0.5*cos(i*(1.0/SHAPE_SAMPLES)*C_PI)) );
            break;
        case MIX_SV_SHAPE_EASED_EXP:
            cur_volume = linear2log(cur_volume);
            new_volume = linear2log(new_volume);
            for (i=0; i<=SHAPE_SAMPLES; i++) cstat.t_shape.shape[i] = dtof16( log2linear( cur_volume + (new_volume-cur_volume) * (0.5-0.5*cos(i*(1.0/SHAPE_SAMPLES)*C_PI)) ) );
            break;
        default:
            fprintf(STD_ERR, "WARNING(softvolume.cpp): unrecognized transition shape, using default (linear)\n");
            goto mix_sv_shape_linear;
            break;
        }

        cstat.t_shape.init((unsigned long)(time * g_sdl_frequency));
        cstat.autoStopMusic = 0;

        SDL_UnlockAudio();
    }

    //Register effect
    Mix_UnregisterEffect(chan,g_sdl_effect_func); //Is it necessary?
    Mix_RegisterEffect(chan,g_sdl_effect_func,0,0);
}

void Mix_SoftVolume_Force(int chan, double newvolume)
{
    Mix_SoftVolume_Init();

    SDL_LockAudio();

    SoftVolume_State &cstat = channel_state[chan];
    cstat.current_volume = dtof16(newvolume);
    cstat.in_transition = false;

    SDL_UnlockAudio();
}

double Mix_SoftVolume_GetCurrentVolume(int chan)
{
    double res;

    Mix_SoftVolume_Init();

    SDL_LockAudio();

    if (channel_state.count(chan)>0)
        res=f16tod(channel_state[chan].current_volume); else
        res=1;

    SDL_UnlockAudio();

    return res;
}

void Mix_SoftVolume_AutoStopMusic(int chan, int enable)
{
    Mix_SoftVolume_Init();

    SDL_LockAudio();

    if (channel_state.count(chan)>0)
        channel_state[chan].autoStopMusic=enable;

    SDL_UnlockAudio();
}