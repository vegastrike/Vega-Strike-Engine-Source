#ifndef __WEBCAM_SUPPORT_H
#define __WEBCAM_SUPPORT_H

#ifdef linux
#include "bgrab.h"
#endif
#if defined( _WIN32) && !defined( __CYGWIN__)
#include <windows.h>
#include <vfw.h>
//#include <afx.h>
#endif
#ifdef __APPLE__
#include <QuickTimeComponents.h>
#endif
#define DEFAULT_CAPTURE_DRIVER 0

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

class	WebcamSupport
{
	private:
#ifdef linux
		struct	fgdevice fg;

		int		region;
		int		channel;
		int		oldchannel;
		char	channeltext[128];
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
		CAPSTATUS		gCapStatus ;
		CAPDRIVERCAPS	gCapDriverCaps ;
		HWND			capvideo, hwndCap;
		CAPTUREPARMS	capparam;
#endif
#ifdef __APPLE__
		typedef struct
		{
			SeqGrabComponent	sg_component;
			SGChannel			sg_channel;
			GWorldPtr			sg_world;
			short				video_width, video_height;
		} videoRec;
		Boolean				gQuicktimeInitialized;
		// Sequence Grabber info
		videoRec *			video;
#endif

		int		width;
		int		height;
		int		depth;
		int		fps;
		double	last_time;
		double	period;
		bool	grabbing;

		char *	image_buffer;

	public:
		WebcamSupport();
		WebcamSupport( int f, int w, int h);

		int		Init();
		void	Shutdown();
		void	GetInfo();
		int		GetFps() { return this->fps;}
		bool	isReady();

		void	StartCapture();
		void	EndCapture();
		char *	CaptureImage();
		int		GetCapturedSize();

		int		CopyImage();
};

// Windows specific stuff to convert images formats
#if defined( _WIN32) && !defined( __CYGWIN__)
extern "C" { 
#include "jpeglib.h" 
}

BOOL JpegFromDib(HANDLE     hDib,     //Handle to DIB
                 int        nQuality, //JPEG quality (0-100)
                 std::string    csJpeg,   //Pathname to target jpeg file
                 std::string*   pcsMsg);  //Error msg to return

BOOL BuildSamps(HANDLE                      hDib,
                int                         nSampsPerRow,
                struct jpeg_compress_struct cinfo,
                JSAMPARRAY                  jsmpArray,
                std::string*                    pcsMsg);

BOOL DibToSamps(HANDLE                      hDib,
                int                         nSampsPerRow,
                struct jpeg_compress_struct cinfo,
                JSAMPARRAY                  jsmpPixels,
                std::string*                    pcsMsg);

RGBQUAD QuadFromWord(WORD b16);
#endif

#endif
