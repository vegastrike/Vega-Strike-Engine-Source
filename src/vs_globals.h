#ifndef __VS_GLOBALS_H_
#define __VS_GLOBALS_H_
/* All global data is stored in a variable of this type */
typedef struct {
  int audio_frequency_mode; //0==11025/8 1==22050/8 2==44100/8  3==11025/16 4==22050/16 5==44100/16  
  int sound_enabled;
  int music_enabled;
  int sound_volume;
  int music_volume;
  int warning_level;
  int capture_mouse;
  int fullscreen;
  int Multitexture;
  int PaletteExt;
  int display_lists;
  int mipmap;//0 = nearest 1 = linear 2 = mipmap
  int color_depth;
  int y_resolution;
  int x_resolution;  
  int cubemap;  
  float MouseSensitivityX;
  float MouseSensitivityY;   
  float fov ;
  float aspect ;
  float znear;
  float zfar ;
  float detaillevel; /*how much the LOD calculation gets multiplied by...higher="bigger" */

  //  scalar_t time;                      /* game time -- now local in lin_time.h*/
  //scalar_t time_step;                 /* size of current time step " */
  //scalar_t secs_since_start;          /* seconds since game was started " */
} game_data_t;

extern game_data_t g_game;

class VegaConfig;
extern VegaConfig *vs_config;

class Mission;
extern Mission *mission;

#endif

