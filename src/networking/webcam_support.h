#ifndef __WEBCAM_SUPPORT_H
#define __WEBCAM_SUPPORT_H

#ifdef linux
#include "bgrab.h"
#endif
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

		// Buffer containing current jpeg image capture
		char*	jpeg_buffer;
		int		jpeg_size;

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
