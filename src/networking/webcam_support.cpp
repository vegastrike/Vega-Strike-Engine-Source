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

#ifdef __APPLE__
#if 0
	this->gQuicktimeInitialized = false;
	this->video = NULL;
#endif
#endif
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
#ifdef __APPLE__
#if 0
	video = new videoRec;
	video->video_width = this->width;
	video->video_height = this->height;

	//	get video prefs, if any
	if (GetPrefHandle(kVideoServerPrefsSig, kVideoServerPrefsID, &data))
		{
		if (NewUserDataFromHandle(data, &settings))
			settings = NULL;
		DisposeHandle(data);
		}
	
	//	configure video
	Rect				r;
	OSErr				iErr = noErr;
	ComponentResult		component_error = noErr;
	//	allocate gworld
	r.left = r.top = 0;
	r.right = video -> video_width;
	r.bottom = video -> video_height;
	iErr = NewGWorld(&video -> sg_world, kDefaultVideoDepth, &r, NULL, NULL, 0);
	if (iErr)
	{
		iErr = ExhaustiveError();
		DoError(iErr, "NewGWorld failed.\rTrying giving me more memory or use a sensible video size");
		exit(1);
	}
	LockPixels(video -> sg_world -> portPixMap);

	//	open default sequence grabber
	video -> sg_component = OpenDefaultComponent('barg', 0);
	if (!video -> sg_component)
	{
		iErr = ExhaustiveError();
		DoError(iErr, "OpenDefaultComponent failed");
		goto bail;
	}
	
	//	initialise the sequence grabber
	component_error = SGInitialize(video -> sg_component);
	if (component_error)
		{
		DoError(component_error, "SGInitialize failed");
		goto bail;
		}
	
	//	set the sequence grabber's grab area to our offscreen area
	component_error = SGSetGWorld(video -> sg_component, video -> sg_world, NULL);
	if (component_error)
		{
		DoError(component_error, "SGSetGWorld failed");
		goto bail;
		}
	
	//	set output settings
	component_error = SGSetDataOutput( video->sg_component, NULL, seqGrabToMemory | seqGrabDontMakeMovie);
	if (component_error)
		{
		DoError(component_error, "SGSetDataOutput failed");
		goto bail;
		}

	//	set data ref
	component_error = SGSetDataOutput( video->sg_component, NULL, seqGrabToMemory | seqGrabDontMakeMovie);
	if (component_error)
		{
		DoError(component_error, "SGSetDataOutput failed");
		goto bail;
		}

	//	get a new sequence grabber channel
	component_error = SGNewChannel(video -> sg_component, VideoMediaType, &video -> sg_channel);
	if (component_error)
		{
		DoError(component_error, "SGNewChannel failed");
		goto bail;
		}
	
	//	set channel settings
	if (settings)
		{
		//	we have data...let's use it
		component_error = SGSetChannelSettings(video -> sg_component, video -> sg_channel, settings, 0);
		if (component_error)
		{
				if (component_error != userCanceledErr)
					DoError(component_error, "SGSetChannelSettings and SGSettingsDialog failed");
				goto bail;
		}
	else
	{
			if (component_error != userCanceledErr)
				DoError(component_error, "SGSettingsDialog failed");
			goto bail;
	}
	
	//	set sequence grabber bounds
	component_error = SGSetChannelBounds(video -> sg_channel, &video -> sg_world -> portRect);
	if (component_error)
		{
		DoError(component_error, "SGSetChannelBounds failed");
		goto bail;
		}
	
	//	set sequence grabber usage
	component_error = SGSetChannelUsage(video -> sg_channel, seqGrabRecord); // maybe seqGrabLowLatencyCapture ?
	if (component_error)
		{
		DoError(component_error, "SGSetChannelUsage failed");
		goto bail;
		}
	
	//	configure for JPEG capture
	component_error = SGSetVideoCompressorType(video -> sg_channel, 'jpeg');
	if (component_error)
		{
		DoError(component_error, "SGSetVideoCompressorType failed");
		goto bail;
		}

	// tell we won't render on screen
	component_error = SGSetUseScreenBuffer(video -> sg_channel, false);
	if (component_error)
		{
		DoError(component_error, "SGSetUseScreenBuffer failed");
		goto bail;
		}

	// specify the framerate (not necessary now)
	component_error = SGSetFrameRate(video -> sg_channel, this->fps);
	if (component_error)
		{
		DoError(component_error, "SGSetUseScreenBuffer failed");
		goto bail;
		}

	//	start the sequence grabber
	/*
	component_error = SGStartPreview(video -> sg_channel);
	if (component_error)
		{
		DoError(component_error, "SGStartPreview failed");
		goto bail;
		}
	*/

	//	update preview window
	/*
	component_error = SGIdle(video -> sg_channel);
	if (component_error)
		{
		DoError(component_error, "SGIdle failed");
		goto bail;
		}
	*/
	//	done it
	iErr = noErr;

	return 0;

bail:
	exit(1);

/*
	ComponentResult				myErr = noErr;
	gSeqGrabber = OpenDefaultComponent(SeqGrabComponentType, 0);
	if (gSeqGrabber == NULL) {
		cerr<<"!!! ERROR : Opening default component !!!"<<endl
		exit(1);
	}
	// initialize the sequence grabber
	myErr = SGInitialize(gSeqGrabber);
	if (myErr == noErr) {
		// create a video channel
		myErr = SGNewChannel(gSeqGrabber, VideoMediaType, &gVideoChannel);
		if ((gVideoChannel != NULL) && (myErr == noErr)) {
			Rect myrect;
			myrect.width = this->width;
			myrect.height = this->height;
			myrect.top = myrect.left = 0;
			myErr = SGSetChannelBounds(gVideoChannel, &myrect);
			myErr = SGSetChannelUsage(gVideoChannel, seqGrabRecord);
			if (myErr != noErr) {
				SGDisposeChannel(gSeqGrabber, gVideoChannel);
				gVideoChannel = NULL;
			}
		}
		//
		// create a sound channel
		myErr = SGNewChannel(gSeqGrabber, SoundMediaType, &gSoundChannel);
		if ((gSoundChannel != NULL) && (myErr == noErr)) {
			Handle		myRates = NULL;
			myErr = SGSetChannelUsage(gSoundChannel, seqGrabRecord);
			*((long *)(*myRates) + 3) = Long2Fix(8000);	// add 22kHz 
			if (myErr != noErr) {
				SGDisposeChannel(gSeqGrabber, gSoundChannel);
				gSoundChannel = NULL;
			}
*/
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
	// Should copy the image data from memory buffer ... where ??
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
#ifdef __APPLE__
#if 0
	CloseComponent( video->sg_component);
#endif
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
	// Should just return the image buffer since it should have been copied in CopyImage
	return 0;
#endif
#ifdef __APPLE__
#if 0
	// Get Buffer Info and see...
	component_error = SGIdle(video -> sg_channel);
	if (component_error)
		{
		DoError(component_error, "SGIdle failed");
		exit(1);
		}
	/*
	component_error = SGUpdate(video -> sg_component, ??????? );
	if (component_error)
		{
		DoError(component_error, "SGIdle failed");
		exit(1);
		}
	*/
	return NULL;
#endif
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
#ifdef __APPLE__
#if 0
	delete video;
#endif
#endif
}

