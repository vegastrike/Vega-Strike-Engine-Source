#include "networking/webcam_support.h"

void	WebcamSupport::Init()
{
#ifndef _WIN32 // Does not work under Cygwin
	if (fg_open_device (&fg, "/dev/video")!=0)
		exit(-1); 	

	if (fg_set_channel (&fg, CHANNEL_TUNER, VIDEOMODE_NTSC)!=0)
		exit(-1);
	region=REGION_NTSC_CABLE;
	if (fg_set_frequency (&fg, region, channel)!=0)
		exit(-1);
#else
#endif
}

void	WebcamSupport::GetInfo()
{
#ifndef _WIN32 // Does not work under Cygwin
	if (fg_print_info (&fg)!=0) exit(-1);
#else
#endif
}

void	WebcamSupport::StartCapture( int fps)
{
#ifndef _WIN32 // Does not work under Cygwin
	if (fg_start_grab_image(&fg, WIDTH, HEIGHT, FORMAT_RGB565)!=0)
		exit(-1);
	fg_set_fps_interval(&fg,fps);
#else
#endif
}

void	WebcamSupport::EndCapture()
{
#ifndef _WIN32 // Does not work under Cygwin
	if (fg_stop_grab_image(&fg)!=0)
		exit(-1);
#else
#endif
}

void	WebcamSupport::CaptureImage()
{
#ifndef _WIN32 // Does not work under Cygwin
 	fg_get_next_image(&fg);
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

