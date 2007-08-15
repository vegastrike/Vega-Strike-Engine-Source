#ifndef __SOUNDSERVER_SOFT_VOLUME__
#define __SOUNDSERVER_SOFT_VOLUME__

typedef enum { 
    MIX_SV_SHAPE_LINEAR,
    MIX_SV_SHAPE_EXP,
    MIX_SV_SHAPE_EASED,
    MIX_SV_SHAPE_EASED_EXP
} Mix_SoftVolume_Shape;

//newvolume: normalized volume (multiplier in [0,inf), 1=no change)
//time: total change time (in seconds)
//shape: transition shape (see enumeration)
//
//Gradually changes volume
extern void Mix_SoftVolume_Change(int chan, double newvolume, double time, Mix_SoftVolume_Shape shape);

//newvolume: normalized volume (multiplier in [0,inf), 1=no change)
//
//Makes the volume change instantaneous - useful for initialization 
extern void Mix_SoftVolume_Force(int chan, double newvolume);

//newvolume: normalized volume (multiplier in [0,inf), 1=no change)
//
//Makes the volume change instantaneous - useful for initialization 
extern double Mix_SoftVolume_GetCurrentVolume(int chan);

//Enables autostop: when volume reaches 0 as a consequence of 
//Mix_SoftVolume_Change, it will automatically halt the music.
//
//NOTE: Gets reset upon Mix_SoftVolume_Change, so call this
//  AFTER Change() - but doesn't get reset by Force().
extern void Mix_SoftVolume_AutoStopMusic(int chan, int enable=1);

#endif//__SOUNDSERVER_SOFT_VOLUME__
