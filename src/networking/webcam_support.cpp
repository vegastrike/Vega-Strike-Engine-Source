#include <iostream>
#include "networking/webcam_support.h"
#include "lin_time.h"

using std::cerr;
using std::endl;

WebcamSupport::WebcamSupport()
{
	this->width = 160;
	this->height = 120;
	this->fps = 5;
	this->grabbing = false;

	this->last_time = 0;
	// Get the period between 2 captured images
	period = 1000000./(double)this->fps;
}

WebcamSupport::WebcamSupport( int f, int w, int h)
{
	this->width = w;
	this->height = h;
	this->fps = f;
	this->grabbing = false;

	this->last_time = 0;
	// Get the period between 2 captured images
	period = 1000000./(double)this->fps;
}

int		WebcamSupport::Init()
{
#ifdef linux
	region = 0;
	channel = 1;
	oldchannel = 1;
	if (fg_open_device (&fg, "/dev/video")!=0)
		return -1; 	

	if (fg_set_channel (&fg, CHANNEL_TUNER, VIDEOMODE_NTSC)!=0)
	{
		this->Shutdown();
		return -1;
	}
	region=REGION_NTSC_CABLE;
	if (fg_set_frequency (&fg, region, channel)!=0)
	{
		this->Shutdown();
		return -1;
	}
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
	capCaptureGetSetup(capvideo, &capparam, sizeof(CAPTUREPARMS));
	
	// Should put the params in gCapParams here

	if( !capDriverConnect(capvideo, DEFAULT_CAPTURE_DRIVER))
		exit(-1);
    capDriverGetCaps(hwndCap, &gCapDriverCaps, sizeof(CAPDRIVERCAPS));
    capGetStatus(hwndCap, &gCapStatus , sizeof(gCapStatus));

	return 0;
#endif
}

bool	WebcamSupport::isReady()
{
	bool ret;
	// Current time in seconds with microseconds
	double curtime = getNewTime();
	// Check that the webcam is grabbin info and that the elapsed time since last capture is bigger than period
	if( grabbing && (last_time-curtime>period) )
	{
		ret = true;
		last_time = curtime;
	}
	else
		ret = false;
	return ret;
}

void	WebcamSupport::GetInfo()
{
#ifdef linux
	if (fg_print_info (&fg)!=0) exit(-1);
#endif
}

void	WebcamSupport::StartCapture()
{
	grabbing = true;
#ifdef linux
	if (fg_start_grab_image(&fg, this->width, this->height, FORMAT_RGB565)!=0)
		exit(-1);

	fg_set_fps_interval(&fg,this->fps);
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
	capCaptureGetSetup(capvideo,&capparam,sizeof(capparam));
	int period = (int) (1000000 / fps);
	capparam.dwRequestMicroSecPerFrame = period;
	capCaptureSetSetup(capvideo,&capparam,sizeof(capparam)) ;
	capCaptureSequenceNoFile(capvideo) ;
	//int (*fcallback) ();
	//fcallback = CopyImage();
	capSetCallbackOnVideoStream(capvideo, CopyImage());
#endif
}

int		WebcamSupport::CopyImage()
{
#ifdef linux
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
#endif
	return 0;
}

void	WebcamSupport::EndCapture()
{
	grabbing = false;
#ifdef linux
	if (fg_stop_grab_image(&fg)!=0)
		exit(-1);
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
    capDriverDisconnect (capvideo);
#endif
}

char *	WebcamSupport::CaptureImage()
{
#ifdef linux // Does not work under Cygwin
	// Returns the image buffer
	if( grabbing)
	{
	 	return (char *) fg_get_next_image(&fg);
	}
	else
		cerr<<"!!! WARNING Webcam not in grabbing mode !!!"<<endl;
	return NULL;
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
	return 0;
#endif
}

int		WebcamSupport::GetCapturedSize()
{
#ifdef linux
	if( grabbing)
	{
	 	return fg.image_size;
	}
	else
		cerr<<"!!! WARNING Webcam not grabbing !!!"<<endl;
	return NULL;
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
	return 0;
#endif
}

void	WebcamSupport::Shutdown()
{
#ifdef linux
	if (fg_close_device (&fg)!=0)
		exit(-1);
#endif
}

