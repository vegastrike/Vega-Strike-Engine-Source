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
#ifndef _WIN32 // Does not work under Cygwin
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
#else
#if 0
	HWND capvideo = capCreATEcAPTureWindow(NULL,0,0,0,width,height,NULL,1);
	if( !capDriverConnect(capvideo, DEFAULT_CAPTURE_DRIVER))
		exit(-1);
#endif
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
#ifndef _WIN32 // Does not work under Cygwin
	if (fg_print_info (&fg)!=0) exit(-1);
#else
#endif
}

void	WebcamSupport::StartCapture()
{
	grabbing = true;
#ifndef _WIN32 // Does not work under Cygwin
	if (fg_start_grab_image(&fg, this->width, this->height, FORMAT_RGB565)!=0)
		exit(-1);

	fg_set_fps_interval(&fg,this->fps);
#else
#if 0
	CAPTUREPARMS capparam;
	capCaptureGetSetup(capvideo,&capparam,sizeof(capparam));
	int period = (int) (1000000 / fps);
	capparam.dwRequestMicroSecPerFrame = period;
	capCaptureSetSetup(capvideo,&capparam,sizeof(capparam)) ;
	capCaptureSequenceNoFile(capvideo) ;
	capSetCallbackOnVideoStream(capvideo, this->CopyImage());
#endif
#endif
}

void	WebcamSupport::CopyImage()
{
#ifndef _WIN32
#else
#endif
}

void	WebcamSupport::EndCapture()
{
	grabbing = false;
#ifndef _WIN32 // Does not work under Cygwin
	if (fg_stop_grab_image(&fg)!=0)
		exit(-1);
#else
#endif
}

char *	WebcamSupport::CaptureImage()
{
#ifndef _WIN32 // Does not work under Cygwin
	// Returns the image buffer
	if( grabbing)
	{
	 	return (char *) fg_get_next_image(&fg);
	}
	else
		cerr<<"!!! WARNING Webcam not grabbing !!!"<<endl;
	return NULL;
#else
#endif
}

int		WebcamSupport::GetCapturedSize()
{
#ifndef _WIN32
	if( grabbing)
	{
	 	return fg.image_size;
	}
	else
		cerr<<"!!! WARNING Webcam not grabbing !!!"<<endl;
	return NULL;
#else
#endif
}

void	WebcamSupport::Shutdown()
{
#ifndef _WIN32 // Does not work under Cygwin
	if (fg_close_device (&fg)!=0)
		exit(-1);
#else
#endif
}

