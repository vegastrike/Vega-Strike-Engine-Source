#ifndef __WEBCAM_SUPPORT_H
#define __WEBCAM_SUPPORT_H

#include <string>
#if defined( _WIN32) && !defined( __CYGWIN__)
#ifndef DSHOW
#include <windows.h>
#include <vfw.h>
//#include <afx.h>
#else
#include <dshow.h>
#include <qedit.h>
#ifdef DSHOW
class WebcamSupport;

// Global DShow variable
extern AM_MEDIA_TYPE g_StillMediaType;
// DirectShow works with a callback interface...
class SampleGrabberCallback : public ISampleGrabberCB
{
public:
	WebcamSupport * ws;
	STDMETHODIMP_(ULONG) AddRef()	{ return 1; }
	STDMETHODIMP_(ULONG) Release()	{ return 2; }

	STDMETHODIMP QueryInterface( REFIID riid, void **ppvObject )
	{
		if( NULL == ppvObject ) return E_POINTER;
		if( riid == __uuidof( IUnknown ) )
		{
			*ppvObject = static_cast<IUnknown*>( this );
			return S_OK;
		}
		if( riid == __uuidof( ISampleGrabberCB ) )
		{
			*ppvObject = static_cast<ISampleGrabberCB*>( this );
			return S_OK;
		}
		return E_NOTIMPL;
	}

	STDMETHODIMP SampleCB( double Time, IMediaSample *pSample )
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP BufferCB( double Time, BYTE *pBuffer, long BufferLen );
};
#endif
#endif
#endif
#ifdef __APPLE__
#include <QuickTimeComponents.h>
pascal OSErr processFrame( SGChannel c, Ptr p, long len, long * offset, long chRefCon, TimeValue time, short writeType, long refcon);
#endif
#define DEFAULT_CAPTURE_DRIVER 0
// This is a limit for jpeg size so we just don't consider bigger shots (should even be less)
#define MAX_JPEG_SIZE 5000

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

#ifdef linux
/* Framegrabber device structure */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/types.h>

#include <linux/videodev.h>

#include <pthread.h>

/* ------ Defines used for framegrabber setup */

/* SETTING_ used for fg_get_setting() and fg_set_setting() */

#define SETTING_BRIGHTNESS	0
#define SETTING_HUE		1
#define SETTING_COLOUR		2
#define SETTING_CONTRAST	3

/* CHANNEL_ used for fg_set_channel() */

#define CHANNEL_TUNER		0
#define CHANNEL_COMPOSITE	1
#define CHANNEL_SVIDEO		2

/* VIDEOMODE_ used for fg_set_channel() */

#define VIDEOMODE_PAL		VIDEO_MODE_PAL			
#define VIDEOMODE_NTSC		VIDEO_MODE_NTSC
#define VIDEOMODE_SECAM		VIDEO_MODE_SECAM

/* FORMAT_ used by fg_start_grab_image() */

#define FORMAT_GREY		VIDEO_PALETTE_GREY
#define FORMAT_RGB565		VIDEO_PALETTE_RGB565
#define FORMAT_RGB24		VIDEO_PALETTE_RGB24
#define FORMAT_RGB32		VIDEO_PALETTE_RGB32
#define FORMAT_YUV422P		VIDEO_PALETTE_YUV422P
#define FORMAT_YUV420P		VIDEO_PALETTE_YUV420P

/* REGION_ used by fg_set_frequency() */

#define REGION_NTSC_BROADCAST		0
#define REGION_NTSC_CABLE		1
#define REGION_NTSC_CABLE_HRC		2
#define REGION_NTSC_BROADCAST_JAPAN	3
#define REGION_NTSC_CABLE_JAPAN		4
#define REGION_PAL_EUROPE		5
#define REGION_PAL_EUROPE_EAST		6
#define REGION_PAL_ITALY		7
#define REGION_PAL_NEWZEALAND		8
#define REGION_PAL_AUSTRALIA		9
#define REGION_PAL_IRELAND		10

/* ------- Defines for internal use  */

#define IMAGE_BUFFER_EMPTY	0
#define IMAGE_BUFFER_FULL	1
#define IMAGE_BUFFER_INUSE	2

static const int error_exit_status = -1; 

/* Channel frequency tables */

#define NUM_CHANNEL_LISTS	11
struct CHANLIST {
    char *name;
    int   freq;
};
struct CHANLISTS {
    char             *name;
    struct CHANLIST  *list;
    int               count;
};
#define CHAN_COUNT(x) (sizeof(x)/sizeof(struct CHANLIST))

/* ------- Framegrabber device structure */
struct fgdevice {
			int video_dev;
			int width;
			int height;
			int input;
			int format;
			struct video_mmap vid_mmap[2];
			int current_grab_number;
			struct video_mbuf vid_mbuf;
			char *video_map;
			int grabbing_active;
			int have_new_frame;
			void *current_image;
			pthread_mutex_t buffer_mutex;
			pthread_t grab_thread;
			pthread_cond_t buffer_cond;
			int totalframecount;
			int image_size;
			int image_pixels;
			int framecount;
			int fps_update_interval;
			double fps;
			double lasttime;
			unsigned short *y8_to_rgb565;
			unsigned char  *rgb565_to_y8;
		};

/* --------- Function prototypes */

#ifdef __cplusplus
extern "C"
{
#endif

int fg_open_device (struct fgdevice *fg, const char *devicename);
int fg_print_info(struct fgdevice *fg); 
int fg_close_device (struct fgdevice *fg);

void fg_set_fps_interval(struct fgdevice *fg, int interval);
double fg_get_fps(struct fgdevice *fg);

int fg_set_channel(struct fgdevice *fg, int channel, int videomode);
int fg_set_frequency(struct fgdevice *fg, int region, int index);

int fg_get_setting(struct fgdevice *fg, int which_setting);
int fg_set_setting(struct fgdevice *fg, int which_setting, int value);

int fg_start_grab_image (struct fgdevice *fg, int width, int height, int format);
int fg_stop_grab_image (struct fgdevice *fg);
void * fg_get_next_image(struct fgdevice *fg);

double timeGet(void);

int getFrequency(int region, int channel);

#ifdef __cplusplus
}
#endif
#endif

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
#ifndef DSHOW
		CAPSTATUS		gCapStatus ;
		CAPDRIVERCAPS	gCapDriverCaps ;
		HWND			capvideo, hwndCap;
		CAPTUREPARMS	capparam;
#else
		SampleGrabberCallback g_SampleCB;
		IGraphBuilder *pGraph;
		ICaptureGraphBuilder2 *pCaptureGraph;
		ICreateDevEnum *pDevEnum;
		IEnumMoniker *pEnum;
		IMoniker *pMoniker;
		IBaseFilter *pCap;
		IBaseFilter *pNull;
		IMediaControl *pControl;
		ISampleGrabber *pSampleGrabber;

		friend class SampleGrabberCallback;
#endif
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
		int		jpeg_quality;

	public:
		// Buffer containing current jpeg image capture
		char *	old_buffer;
		int		old_size;
		char *	jpeg_buffer;
		int		jpeg_size;
		int		nbframes;

		WebcamSupport();
		WebcamSupport( int f, int w, int h);

		int		Init();
		void	Shutdown();
		void	GetInfo();
		int		GetFps() { return this->fps;}
		bool	isReady();

		void		StartCapture();
		void		EndCapture();
		std::string	CaptureImage();
		int			GetCapturedSize();

		int		CopyImage();
		void	DoError( long code, char * msg);
};

// Windows specific stuff to convert BMP to JPEG
#if defined( _WIN32) && !defined( __CYGWIN__)
#ifndef HAVE_BOOLEAN
#define HAVE_BOOLEAN
#define FALSE 0
#define TRUE 1
typedef unsigned char boolean;
#endif
#ifndef XMD_H
#define XMD_H
typedef int INT32;
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#include <string>

extern "C" { 
#include "jpeglib.h" 
}

char * JpegFromCapture(HANDLE     hDib, BYTE * bmpBuffer, long bmpLength, int * jpegLength, int nQuality, std::string csJpeg="");
char * JpegFromBmp( BITMAPFILEHEADER & bfh, LPBITMAPINFOHEADER & lpbi, BYTE * bmpbuffer, long BufferLen, int * jpglength, int quality, std::string csJpeg);
BOOL JpegFromDib(HANDLE     hDib,     //Handle to DIB
                 int        nQuality, //JPEG quality (0-100)
                 std::string    csJpeg,   //Pathname to target jpeg file
                 std::string*   pcsMsg);  //Error msg to return

BOOL DibToSamps2( BITMAPFILEHEADER & bfh, LPBITMAPINFOHEADER & pbBmHdr, BYTE * bmpbuffer, int nSampsPerRow, struct jpeg_compress_struct cinfo, JSAMPARRAY jsmpPixels);
BOOL DibToSamps(HANDLE                      hDib,
                int                         nSampsPerRow,
                struct jpeg_compress_struct cinfo,
                JSAMPARRAY                  jsmpPixels,
                std::string*                    pcsMsg);

RGBQUAD QuadFromWord(WORD b16);
#endif
#ifdef DSHOW


#endif

#endif
