#include "vegastrike.h"
#include "sdl_audio_data.h"
#include "sdl_audio.h"
#include "sdl_init.h"
#include "joystick.h"
#if defined (HAVE_SDL) && defined (HAVE_SDL_MIXER)

void sdl_init() {
  int hz, channels, buffer;
  Uint16 format;
  Uint32 flags;

  flags = 0;

  if ( g_game.sound_enabled ) {
    flags |= SDL_INIT_AUDIO;
  }
 
#ifdef HAVE_SDL_JOYSTICKOPEN
    flags |= SDL_INIT_JOYSTICK;
#endif

    /*
     * Initialize SDL
     */
    if ( SDL_Init(flags) < 0 ) {
	handle_error(1, "Couldn't initialize SDL: %s\n",SDL_GetError());
    }

    if ( g_game.sound_enabled) {
	/* Open the audio device */
	switch (g_game.audio_frequency_mode%3) {
	case 0:
	    hz = 11025;
	    break;
	case 1:
	    hz = 22050;
	    break;
	case 2:
	    hz = 44100;
	    break;
	default:
	    hz = 22050;
	}

	switch ( (g_game.audio_frequency_mode/3)%2 ) {
	case 0:
	    format = AUDIO_U8;
	    break;
	case 1:
	    format = AUDIO_S16SYS;
	    break;
	default:
	    format = AUDIO_S16SYS;
	}

	if ( g_game.audio_frequency_mode/6 ) {
	    channels = 1;
	} else {
	    channels = 2;
	}

	buffer = 2048;//getparam_audio_buffer_size();

	if ( Mix_OpenAudio(hz, format, channels, buffer) < 0 ) {
	    print_warning( 1,
			   "Warning: Couldn't set %d Hz %d-bit audio\n"
			   "  Reason: %s\n", 
			   hz,  
			   (g_game.audio_frequency_mode/3)%2 == 0 ? 8 : 16,
			   SDL_GetError());
	} else {
	    print_debug( DEBUG_SOUND,
			 "Opened audio device at %d Hz %d-bit audio",
			 hz, 
			 (g_game.audio_frequency_mode/3)%2 == 0 ? 8 : 16);
			 
	}
    }

    atexit(SDL_Quit);
}

#endif /* defined (HAVE_SDL) && defined (HAVE_SDL_MIXER) */
