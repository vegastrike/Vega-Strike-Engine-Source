#ifndef __WEBCAM_SUPPORT_H
#define __WEBCAM_SUPPORT_H

#include "bgrab.h"

// CAPTURE WINDOW SIZE MUST BE DETERMINED AND THEREFORE WILL NOT BE ADJUSTABLE

/*
 * Webcam Support : class that provide webcam image capture for both win32 and Linux for now
 *                  the use of this class requires a few parameters in vegastrike.config :
 *                      FOR LINUX :
 *                      - video_device (/dev/video is default if none provided)
 *                      - video_mode (NTSC is default)
 *                      - channel_mode (defaults to CHANNEL_TUNER)
 *                      - region (defaults to REGION_NTSC_CABLE)
 *
 */

class	WebcamCapture
{
	private:
#ifndef _WIN32 // Does not work under Cygwin
		struct fgdevice fg;
		struct xwinbuffer xwin;

		int region=0;
		int channel=1;
		int oldchannel=1;
		char channeltext[128];
#else
#endif
	public:
		WebcamSupport();

		void	Init();
		void	Shutdown();
		void	GetInfo();

		void	StartCapture( int fps);
		void	EndCapture();
		void	CaptureImage();
};

#endif
