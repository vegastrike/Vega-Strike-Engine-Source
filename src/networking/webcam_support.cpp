#include <iostream>
#include "webcam_support.h"
#include "lin_time.h"
#include "vs_path.h"
#include "gldrv/winsys.h"

using std::cerr;
using std::endl;
using std::hex;
extern bool cleanexit;

#ifdef DSHOW
/**************************************************************************************************/
/**** DirectShow Callback : SampleGrabberCallback                                              ****/
/**************************************************************************************************/

STDMETHODIMP SampleGrabberCallback::BufferCB( double Time, BYTE *pBuffer, long BufferLen )
{
	cerr<<"\t\tProcessing a frame"<<endl;
		// Check if it is time to send capture to communicating client(s)
		//if( ws->isReady())
		{
			AM_MEDIA_TYPE MediaType; 
			ZeroMemory(&MediaType,sizeof(MediaType)); 
			HRESULT hr = ws->pSampleGrabber->GetConnectedMediaType(&MediaType); 
			if (FAILED(hr)) 
			{
				cerr<<"GetConnectedMediaType"<<endl;
				cleanexit = true;
				winsys_exit(1);
			}
			if(MediaType.majortype != MEDIATYPE_Video)
			{
				cerr<<"INVALID MEDIA TYPE 1"<<endl;
				cleanexit = true;
				winsys_exit(1);
				return VFW_E_INVALIDMEDIATYPE;
			}
			if(MediaType.formattype != FORMAT_VideoInfo)
			{
				cerr<<"INVALID MEDIA TYPE 2"<<endl;
				cleanexit = true;
				winsys_exit(1);
				return VFW_E_INVALIDMEDIATYPE;
			}
			if(MediaType.cbFormat < sizeof(VIDEOINFOHEADER))
			{
				cerr<<"INVALID MEDIA TYPE 3"<<endl;
				cleanexit = true;
				winsys_exit(1);
				return VFW_E_INVALIDMEDIATYPE;
			}
			if(MediaType.pbFormat == NULL)
			{
				cerr<<"INVALID MEDIA TYPE 4"<<endl;
				cleanexit = true;
				winsys_exit(1);
				return VFW_E_INVALIDMEDIATYPE;
			}

			VIDEOINFOHEADER *pVideoHeader = (VIDEOINFOHEADER*)MediaType.pbFormat; 
			if (pVideoHeader == NULL) 
				return E_FAIL; 
			// The video header contains the bitmap information. 
			// Copy it into a BITMAPINFO structure. 
			BITMAPINFO BitmapInfo; 
			ZeroMemory(&BitmapInfo, sizeof(BitmapInfo)); 
			CopyMemory(&BitmapInfo.bmiHeader, &(pVideoHeader->bmiHeader), 
					   sizeof(BITMAPINFOHEADER)); 

			// Create a DIB from the bitmap header, and get a pointer to the buffer. 
			void *buffer = NULL; 
			HBITMAP hBitmap = ::CreateDIBSection(0, &BitmapInfo, DIB_RGB_COLORS, (void **)&pBuffer, 
                                     NULL, 0);
			// Copy the image into the buffer. 
			long size = 0; 
			/*
			hr = ws->pSampleGrabber->GetCurrentBuffer(&size,(long *)buffer);   
			if (FAILED(hr)) 
			{
				cerr<<"GETCURRENTBUFFER";
				cerr<<" - code : "<<hr<<" HEX : ";
				cerr<<hex<<hr<<endl;
				cleanexit = true;
				winsys_exit(1);
			}
			*/
			/*
			long cbBitmapInfoSize = g_StillMediaType.cbFormat - SIZE_PREHEADER;
			BITMAPFILEHEADER bfh;
			ZeroMemory(&bfh, sizeof(bfh));
			bfh.bfType = 'MB';  // Little-endian for "MB".
			bfh.bfSize = sizeof( bfh ) + BufferLen + cbBitmapInfoSize;
			bfh.bfOffBits = sizeof( BITMAPFILEHEADER ) + cbBitmapInfoSize;

		    LPBITMAPINFOHEADER lpbi = HEADER( pVideoHeader);
			*/
			// The picture is composed like this :
			// - bfh : bitmap file header
			// - HEADER(pVideoHeader) : bitmap info header
			// - pBuffer (length BufferLen) : picture data
			// Copy the actual jpeg in the old buffer before getting the new one
			if( ws->old_buffer)
			{
				delete ws->old_buffer;
				ws->old_buffer = NULL;
			}
			if( ws->jpeg_buffer)
			{
				ws->old_buffer = ws->jpeg_buffer;
				ws->old_size = ws->jpeg_size;
			}
			
			// Write p to a file for testing
			char file[256];
			memset( file, 0, 256);
			sprintf( file, "%s%d%s", "testcam", ws->nbframes, ".bmp");
			//string path = datadir+"testcam"+string( ws->nbframes)+".jpg";
			string path = datadir+file;
			FILE * fp;
			fp = fopen( path.c_str(), "w");
			if( !fp)
			{
				cerr<<"opening jpeg file failed"<<endl;
				exit(1);
			}
			if( fwrite( &BitmapInfo, 1, sizeof(BITMAPINFOHEADER), fp)!=sizeof(BITMAPINFOHEADER))
			{
				cerr<<"!!! ERROR : writing jpeg description to file 1"<<endl;
				exit(1);
			}
			if( fwrite( pBuffer, 1, BufferLen, fp)!=BufferLen)
			{
				cerr<<"!!! ERROR : writing jpeg description to file"<<endl;
				exit(1);
			}
			fclose( fp);

			// JpegFromBmp should allocate the needed buffer;
			//ws->jpeg_buffer = JpegFromBmp( bfh, lpbi, pBuffer, BufferLen, &ws->jpeg_size, ws->jpeg_quality, "c:\test.jpg");
			//FreeMediaType(MediaType);
		}

		return S_OK;
	}
#endif
#ifdef __APPLE__
/**************************************************************************************************/
/**** QuickTime Callback : Process data written to the video channel                           ****/
/**************************************************************************************************/
pascal OSErr processFrame( SGChannel c, Ptr p, long len, long * offset, long chRefCon, TimeValue time, short writeType, long refCon)
{
	//cerr<<"\t\tProcessing a frame"<<endl;
	// HERE SHOULD ONLY COPY THE IMAGE INTO THE JPEG BUFFER
	WebcamSupport * ws = (WebcamSupport *) refCon;
	// Sometimes on bad capture a black image can be caught with a size og about 45kB !
	// So we don't take it into account
	if( len<MAX_JPEG_SIZE)
	{
		// Clear the old buffer
		if( ws->old_buffer)
		{
			delete ws->old_buffer;
			ws->old_buffer = NULL;
		}
		if( ws->jpeg_buffer)
		{
			ws->old_buffer = ws->jpeg_buffer;
			ws->old_size = ws->jpeg_size;
		}
		ws->jpeg_buffer = new char[len+1];
		memcpy( ws->jpeg_buffer, p, len);
		ws->jpeg_buffer[len] = 0;
		ws->jpeg_size = len;
		/*
		if( ws)
		{
			// Write p to a file for testing
			char file[256];
			memset( file, 0, 256);
			sprintf( file, "%s%d%s", "testcam", ws->nbframes, ".jpg");
			//string path = datadir+"testcam"+string( ws->nbframes)+".jpg";
			string path = datadir+file;
			FILE * fp;
			fp = fopen( path.c_str(), "w");
			if( !fp)
			{
				cerr<<"opening jpeg file failed"<<endl;
				exit(1);
			}
			if( fwrite( p, 1, len, fp)!=len)
			{
				cerr<<"writing jpeg description to file"<<endl;
				exit(1);
			}
			fclose( fp);
		}
		*/
	}

	return noErr;
}
#endif


/**************************************************************************************************/
/**** DoError : Close webcam stuff and exits with given code                                   ****/
/**************************************************************************************************/
void	WebcamSupport::DoError( long error, char * message)
{
	if( error)
	{
		cerr<<"!!! ERROR : "<<message<<" - code : "<<error<<" HEX : ";
		cerr<<hex<<error<<endl;
		this->Shutdown();
		cleanexit = true;
		winsys_exit(1);
	}
}

/**************************************************************************************************/
/**** WebcamSupport default constructor                                                        ****/
/**************************************************************************************************/
WebcamSupport::WebcamSupport()
{
	this->width = 120;
	this->height = 90;
	this->fps = 1;
	this->grabbing = false;
	this->depth = 16;
	this->nbframes = 0;
	this->jpeg_quality = 20;

	this->last_time = 0;
	// Get the period between 2 captured images
	period = 1./(double)this->fps;

	this->old_buffer = NULL;
	this->old_size = 0;
	this->jpeg_buffer = NULL;
	this->jpeg_size = 0;

#ifdef __APPLE__
	OSErr				iErr = noErr;
	GDHandle			saveDevice;
	CGrafPtr			savePort;
	long				qtVers;

	GetGWorld( &savePort, &saveDevice);
	
	iErr = Gestalt( gestaltQuickTime, &qtVers);
	DoError( iErr, "initialising Quicktime");
	iErr = EnterMovies();
	DoError( iErr, "initialising Quicktime part 2");
	SetGWorld( savePort, saveDevice);

	this->gQuicktimeInitialized = true;
	this->video = NULL;
#endif
#ifdef DSHOW
	pCaptureGraph = NULL;
	pDevEnum = NULL;
	pEnum = NULL;
	pMoniker = NULL;
	pCap = NULL;
	pControl = NULL;
	pGraph = NULL;
#endif
}

/**************************************************************************************************/
/**** WebcamSupport constructor specifying capture size and fps                                ****/
/**************************************************************************************************/
WebcamSupport::WebcamSupport( int f, int w, int h)
{
	WebcamSupport::WebcamSupport();
	this->width = w;
	this->height = h;
	this->fps = f;
	if( this->Init() == -1)
		DoError( -1, "initializing webcam");
}

/**************************************************************************************************/
/**** Initialize the webcam                                                                    ****/
/**************************************************************************************************/
int		WebcamSupport::Init()
{
#ifdef linux
	region = 0;
	channel = 1;
	oldchannel = 1;
	if (fg_open_device (&fg, "/dev/video")!=0)
		return -1; 	

	if (fg_set_channel (&fg, CHANNEL_TUNER, VIDEOMODE_NTSC)!=0)
		DoError( -1, "fg_set_channel failed");
	region=REGION_NTSC_CABLE;
	if (fg_set_frequency (&fg, region, channel)!=0)
		DoError( -1, "fg_set_frequency failed");

	fg_set_fps_interval(&fg,this->fps);
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
#ifndef DSHOW
	//capCaptureGetSetup(capvideo, &capparam, sizeof(CAPTUREPARMS));

	// Should put the params in gCapParams here

	//GetDesktopWindow()->GetSafeHwnd()
	HWND hWnd=capCreateCaptureWindow( "WebCam_Hidden_Window",
        							WS_CHILD|WS_CLIPSIBLINGS, 0, 0, this->width, this->height, 
									GetDesktopWindow(), 0xffff);
	if(!hWnd)
		DoError( -1, " creating capture window");

	if( !capDriverConnect(hWnd, DEFAULT_CAPTURE_DRIVER))
		DoError( -1, " connecting to capture device");
    //capDriverGetCaps(hwndCap, &gCapDriverCaps, sizeof(CAPDRIVERCAPS));
    //capGetStatus(hwndCap, &gCapStatus , sizeof(gCapStatus));

	capCaptureGetSetup(capvideo,&capparam,sizeof(capparam));
	int period = (int) (1000000 / fps);
	capparam.dwRequestMicroSecPerFrame = period;
	capCaptureSetSetup(capvideo,&capparam,sizeof(capparam)) ;
#else
	HRESULT hr = CoInitialize( NULL );
	if( FAILED( hr ) )
		DoError( hr, "Failed to initialise COM");

	// Create the filter graph
	hr = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **)&pGraph );
	if( FAILED( hr ) )
		DoError( hr, "Failed to create filter graph");

	// Create the capture graph
	hr = CoCreateInstance( CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
		IID_ICaptureGraphBuilder2, (void**)&pCaptureGraph );
	if( FAILED( hr ) )
		DoError( hr, "Couldn't create capture graph");

	hr = pCaptureGraph->SetFiltergraph( pGraph );
	if( FAILED( hr ) )
		DoError( hr, "Couldn't set filter graph");

	// Select a capture device
	hr = CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, reinterpret_cast<void **>( &pDevEnum ) );
	if( FAILED( hr ) )
		DoError( hr, "CoCreateInstance failed");
	hr = pDevEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pEnum, 0 );
	if( FAILED( hr ) )
		DoError( hr, "CreateClassEnumerator failed");
	hr = pEnum->Next( 1, &pMoniker, NULL );
	if( FAILED( hr ) )
		DoError( hr, "Next failed");
	hr = pMoniker->BindToObject( 0, 0, IID_IBaseFilter, (void **)&pCap );
	if( FAILED( hr ) )
		DoError( hr, "BindToObject failed");

	// Add input filter to graph
	hr = pGraph->AddFilter( pCap, L"Capture Filter" );
	if( FAILED( hr ) )
		DoError( hr, "AddFilter failed");

	// Add sample grabber filter to graph
	IBaseFilter *pSampleGrabberF = NULL;
	hr = CoCreateInstance( CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void **)&pSampleGrabberF );
	if( FAILED( hr ) )
		DoError( hr, "CoCreateInstance2 failed");
	hr = pGraph->AddFilter( pSampleGrabberF, L"Sample grabber" );
	if( FAILED( hr ) )
		DoError( hr, "AddFilter2 failed");
	hr = pSampleGrabberF->QueryInterface( IID_ISampleGrabber, (void **)&pSampleGrabber );
	if( FAILED( hr ) )
		DoError( hr, "QueryInterface failed");

	// Set some sample grabber variables
	AM_MEDIA_TYPE mt;
	ZeroMemory( &mt, sizeof( AM_MEDIA_TYPE ) );
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;

	// Try to specify a framerate for capture
	// NOT SURE THIS HAS A REAL EFFECT SO WE MIGHT WANT TO COMMENT IT SINCE IT ALLOCATES MEMORY
	VIDEOINFOHEADER * pVidH = new VIDEOINFOHEADER;
	ZeroMemory(pVidH, sizeof(VIDEOINFOHEADER)); 
	// Compute the time between 2 frames in nano seconds according to the specified fps
	pVidH->AvgTimePerFrame = (1./(float)this->fps)*1000000.;
	mt.pbFormat = (unsigned char *) pVidH;

	hr = pSampleGrabber->SetMediaType( &mt );
	if( FAILED( hr ) )
		DoError( hr, "SetMediaType failed");
	hr = pSampleGrabber->SetOneShot( false );
	if( FAILED( hr ) )
		DoError( hr, "SetOneShot failed");
	hr = pSampleGrabber->SetBufferSamples( true );
	if( FAILED( hr ) )
		DoError( hr, "SetBufferSamples failed");
	// Associate the webcamsupport to the callback
	g_SampleCB.ws = this;
	hr = pSampleGrabber->SetCallback( &g_SampleCB, 1 );
	if( FAILED( hr ) )
		DoError( hr, "SetCallback failed");

	// Add Null renderer
	hr = CoCreateInstance( CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void **)&pNull );
	if( FAILED( hr ) )
		DoError( hr, "CoCreateInstance3 failed");
	hr = pGraph->AddFilter( pNull, L"Null renderer" );
	if( FAILED( hr ) )
		DoError( hr, "AddFilter3 failed");

	// Build video preview graph
	hr = pCaptureGraph->RenderStream( &PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pCap, pSampleGrabberF, NULL );
	if( FAILED( hr ) )
		DoError( hr, "Failed on RenderStream");

	// Create media control and run graph
	hr = pGraph->QueryInterface( IID_IMediaControl, (void **)&pControl );
	if( FAILED( hr ) )
		DoError( hr, "pGraph->QueryInterface failed");

	/*
	hr = pSampleGrabber->GetConnectedMediaType(&g_StillMediaType);
	if( FAILED( hr ) )
		DoError( hr, "GetConnectedMediaType failed");
	*/
#endif
	return 0;
#endif
#ifdef __APPLE__
	video = new videoRec;
	video->video_width = this->width;
	video->video_height = this->height;

	//	get video prefs, if any
	/*
	if (GetPrefHandle(kVideoServerPrefsSig, kVideoServerPrefsID, &data))
		{
		if (NewUserDataFromHandle(data, &settings))
			settings = NULL;
		DisposeHandle(data);
		}
	*/
	
	//	configure video
	Rect				r;
	OSErr				iErr = noErr;
	ComponentResult		component_error = noErr;
	//	allocate gworld
	r.left = r.top = 0;
	r.right = video -> video_width;
	r.bottom = video -> video_height;
	iErr = QTNewGWorld(&video -> sg_world, this->depth, &r, 0, NULL, 0);
	DoError(iErr, "NewGWorld failed - Trying giving me more memory or use a sensible video size");
	if( !LockPixels( GetGWorldPixMap( video -> sg_world)))
		DoError( -1, "locking pixmap");

	//	open default sequence grabber
	video -> sg_component = OpenDefaultComponent('barg', 0);
	if (!video -> sg_component)
		DoError(iErr, "OpenDefaultComponent failed");
	
	//	initialise the sequence grabber
	component_error = SGInitialize(video -> sg_component);
	DoError(component_error, "SGInitialize failed");
	
	//	set the sequence grabber's grab area to our offscreen area
	component_error = SGSetGWorld(video -> sg_component, video -> sg_world, NULL);
	DoError(component_error, "SGSetGWorld failed");
	
	//  set destination data reference
	component_error = SGSetDataRef( video->sg_component, 0, 0, seqGrabDontMakeMovie);
	DoError( component_error, "SGSetDataRef failed");

	//	set output settings
	//component_error = SGSetDataOutput( video->sg_component, NULL, seqGrabToMemory | seqGrabDontMakeMovie);
	//DoError(component_error, "SGSetDataOutput failed");

	//	get a new sequence grabber channel
	component_error = SGNewChannel(video -> sg_component, VideoMediaType, &video -> sg_channel);
	DoError(component_error, "SGNewChannel failed");
	
	//	set sequence grabber bounds
	//component_error = SGSetChannelBounds(video -> sg_channel, &video -> sg_world -> portRect);
	component_error = SGSetChannelBounds(video -> sg_channel, &r);
	DoError(component_error, "SGSetChannelBounds failed");
	
	//	set sequence grabber usage
	component_error = SGSetChannelUsage(video -> sg_channel, seqGrabRecord); // maybe seqGrabLowLatencyCapture ?
	DoError(component_error, "SGSetChannelUsage failed");
	
	//	configure for JPEG capture
	component_error = SGSetVideoCompressor(video -> sg_channel, this->depth, 'jpeg', codecLowQuality, codecNormalQuality, 5);
	DoError(component_error, "SGSetVideoCompressorType failed");

	// tell we won't render on screen
	component_error = SGSetUseScreenBuffer(video -> sg_channel, false);
	DoError(component_error, "SGSetUseScreenBuffer failed");

	// specify the framerate (only to reduce CPU usage we set FPS to twice the real FPS)
	// if we don't do that the FPS will be the fastest QuickTime can use
	component_error = SGSetFrameRate(video -> sg_channel, this->fps*2);
	DoError(component_error, "SGSetUseScreenBuffer failed");

	// Set the callback
	iErr = SGSetDataProc( video->sg_component, NewSGDataUPP( processFrame), (long)this);
	// Prepare for recording
	component_error = SGPrepare( video->sg_component, false, true);
	DoError( component_error, "SGPrepare failed");

	return 0;
#endif
}

/**************************************************************************************************/
/**** Tell if it is time to grab a new frame                                                   ****/
/**************************************************************************************************/
bool	WebcamSupport::isReady()
{
	bool ret;
	// Current time in seconds with microseconds
	double curtime = getNewTime();
	// Check that the webcam is grabbin info and that the elapsed time since last capture is bigger than period
	if( grabbing && (curtime-last_time>period) )
	{
		ret = true;
		last_time = curtime;
	}
	else
		ret = false;

	// Each time we call this function we'll give a little time to the grabbing mecanism
#ifdef __APPLE__
	ComponentResult		component_error = noErr;
	// Update the offscreen GWorld
	component_error = SGIdle(video -> sg_channel);
	if (component_error)
		DoError(component_error, "SGIdle failed");
#endif

	return ret;
}

/**************************************************************************************************/
/**** Display webcam driver info                                                               ****/
/**************************************************************************************************/
void	WebcamSupport::GetInfo()
{
#ifdef linux
	if (fg_print_info (&fg)!=0) exit(-1);
#endif
}

/**************************************************************************************************/
/**** Start the capture mode                                                                   ****/
/**************************************************************************************************/
void	WebcamSupport::StartCapture()
{
	grabbing = true;
#ifdef linux
	if (fg_start_grab_image(&fg, this->width, this->height, FORMAT_RGB565)!=0)
		DoError( -1, "starting grabbing image failed");
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
#ifndef DSHOW
	capCaptureSequenceNoFile(capvideo) ;
	//int (*fcallback) ();
	//fcallback = CopyImage();
	//capSetCallbackOnVideoStream(capvideo, CopyImage());
#else
	HRESULT hr = pControl->Run();
	if( FAILED( hr))
		DoError( hr, "pControl->Run() failed");
#endif
#endif
#ifdef __APPLE__
	ComponentResult		component_error = noErr;
	// Start record
	component_error = SGStartRecord( video->sg_component);
	DoError( component_error, "SGStartRecord failed");
#endif
}

/**************************************************************************************************/
/**** Copy an image... not used for now                                                        ****/
/**************************************************************************************************/
int		WebcamSupport::CopyImage()
{
	return 0;
}

/**************************************************************************************************/
/**** End capture mode                                                                         ****/
/**************************************************************************************************/
void	WebcamSupport::EndCapture()
{
if( grabbing)
{
	grabbing = false;
#ifdef linux
	if (fg_stop_grab_image(&fg)!=0)
		exit(-1);
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
#ifndef DSHOW
    capDriverDisconnect (capvideo);
#else
	HRESULT hr = pControl->Stop();
#endif
#endif
#ifdef __APPLE__
	ComponentResult		component_error = noErr;
	component_error = SGStop( video->sg_component);
	DoError( component_error, "stopping video capture");
#endif
}
}

/**************************************************************************************************/
/**** Get the latest grabbed image                                                             ****/
/**************************************************************************************************/
std::string	WebcamSupport::CaptureImage()
{
if( grabbing)
{
	this->nbframes++;
#ifdef linux // Does not work under Cygwin
	// Returns the image buffer
	char * strptr = (char *) fg_get_next_image(&fg);
	return string( strptr);
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
#ifndef DSHOW
	// Open and empty the clipboard
	if ( !OpenClipboard(NULL) )
		DoError( 0, "!!! ERROR opening windows clipboard !!!");
	if ( !EmptyClipboard() )
		DoError( 0, "!!! ERROR emptying the clipboard !!!");

	// Test to see supported clipboard formats
	UINT format = 0;
	bool t = false;
	while( !t && format != 0)
	{
		t = true;
		format = EnumClipboardFormats( format);
	}

	// Copy the webcam frame in the clipboard
	capEditCopy( hwndCap);

	// Get the bitmap from clipboard
	//HGLOBAL img_buffer = GetClipboardData( CF_BITMAP);
	//HGLOBAL pal_buffer = GetClipboardData( CF_PALETTE);
    
	HANDLE  hDib = GetClipboardData( CF_DIB);
	/*
	HANDLE  hDib = DDBToDIB((HBITMAP)img_buffer,
                             BI_RGB,
                             NULL);  //Use default palette
	*/
    //Turn DIB into JPEG file
    if (hDib != NULL)
    {
		std::string  csMsg = "";
        if (!JpegFromDib(hDib, 70/*Quality*/, "c:\temp\vstest.jpg", &csMsg))
        {
            cerr<<csMsg.c_str()<<endl;
        }

        else
            cerr<<"jpeg file created"<<endl;

        ::GlobalFree(hDib);
    }

    else
        cerr<<"Failed to load IDB_TEST"<<endl;

	// Close the clipboard
	CloseClipboard();
#else
	// DirectShow uses a callback interface so there is nothing to do here
	// We just return the allocated buffer for jpeg file
	if( !jpeg_buffer)
	{
		cerr<<"Frame #"<<nbframes<<" !!! JPEG BUFFER EMPTY !!!"<<endl;
		return string("");
	}
	cerr<<"Frame #"<<nbframes<<" --== JPEG BUFFER OK ==--"<<endl;
	return string( jpeg_buffer);
#endif
#endif
#ifdef __APPLE__
	ComponentResult		component_error = noErr;
	// Update the offscreen GWorld
	component_error = SGIdle( video->sg_channel);
	if (component_error)
		DoError(component_error, "SGIdle failed");
	// Just return the jpeg_bufer filled by the callback function
	if( !jpeg_buffer)
	{
		//cerr<<"Frame #"<<nbframes<<" !!! JPEG BUFFER EMPTY !!!"<<endl;
		return string("");
	}
	//cerr<<"Frame #"<<nbframes<<" --== JPEG BUFFER OK ==--"<<endl;
	return string( jpeg_buffer);
#endif
}
else
	cerr<<"!!! WARNING Webcam not in grabbing mode !!!"<<endl;
return NULL;
}

/**************************************************************************************************/
/**** GetCapturedSize : not used                                                              ****/
/**************************************************************************************************/
int		WebcamSupport::GetCapturedSize()
{
#ifdef linux
	if( grabbing)
	{
	 	return fg.image_size;
	}
	else
		cerr<<"!!! WARNING Webcam not grabbing !!!"<<endl;
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
#endif
	return 0;
}

/**************************************************************************************************/
/**** Close devices, free memory correctly (well soon :))                                      ****/
/**************************************************************************************************/
void	WebcamSupport::Shutdown()
{
	if( grabbing)
		this->EndCapture();
#ifdef linux
	if (fg_close_device (&fg)!=0)
		exit(-1);
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
#ifndef DSHOW
#else
	pSampleGrabber->Release();
	pDevEnum->Release();
	pEnum->Release();
	pMoniker->Release();
	pCap->Release();
	pCaptureGraph->Release();
	pGraph->Release();
	//FreeMediaType(g_StillMediaType);
	CoUninitialize();
#endif
#endif
#ifdef __APPLE__
	SGDisposeChannel( video->sg_component, video->sg_channel);
	CloseComponent( video->sg_component);
	delete video;
#endif
	if( old_buffer)
		delete old_buffer;
	old_buffer = NULL;
	old_size = 0;
	if( jpeg_buffer)
		delete jpeg_buffer;
	jpeg_buffer = NULL;
	jpeg_size = 0;
}

/***********************************************************************************/
/**** END OF WEBCAMSUPPORT CLASS                                                ****/
/**** JPEG OS SPECIFIC STUFF HERE                                               ****/
/***********************************************************************************/

// Windows specific stuff used to convert image formats
#if defined( _WIN32) && !defined( __CYGWIN__)
extern "C"
{
#include "jpeglib.h"
#include <setjmp.h>
}

struct ima_error_mgr
{
  struct jpeg_error_mgr pub;  //"public" fields
  jmp_buf setjmp_buffer;      //for return to caller
};

////////////////////////////////////////////////////////////////////////////
//This function takes the contents of a DIB
//and turns it into a JPEG file.
//
//The DIB may be monochrome, 16-color, 256-color, or 24-bit color.
//
//Any functions or data items beginning with "jpeg_" belong to jpeg.lib,
//and are not included here.
//
//The function assumes 3 color components per pixel.
/////////////////////////////////////////////////////////////////////////////
char * JpegFromBmp( BITMAPFILEHEADER & bfh, LPBITMAPINFOHEADER & lpbi, BYTE * bmpbuffer, long BufferLen, int * jpglength, int quality, std::string csJpeg)
{
    if (quality < 0 || quality > 100 || bmpbuffer == NULL || (!csJpeg.size()) )
	{
		cerr<<"Bad parameters";
		exit(1);
	}

    byte *buf2 = 0;

    //Use libjpeg functions to write scanlines to disk in JPEG format
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;

    FILE*      pOutFile;     //Target file 
    int        nSampsPerRow; //Physical row width in image buffer 
    JSAMPARRAY jsmpArray;    //Pixel RGB buffer for JPEG file

    cinfo.err = jpeg_std_error(&jerr); //Use default error handling (ugly!)
    jpeg_create_compress(&cinfo);

    if ((pOutFile = fopen(csJpeg.c_str(), "wb")) == NULL)
	{
		cerr<<"opening of jpeg file failed.";
		exit(1);
	}

    jpeg_stdio_dest(&cinfo, pOutFile);

    cinfo.image_width      = lpbi->biWidth;  //Image width and height, in pixels 
    cinfo.image_height     = lpbi->biHeight;
    cinfo.input_components = 3;              //Color components per pixel
                                             //(RGB_PIXELSIZE - see jmorecfg.h)
    cinfo.in_color_space   = JCS_RGB; 	     //Colorspace of input image

    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo,
                     quality, //Quality: 0-100 scale
                     TRUE);    //Limit to baseline-JPEG values

    jpeg_start_compress(&cinfo, TRUE);

    //JSAMPLEs per row in output buffer
    nSampsPerRow = cinfo.image_width * cinfo.input_components; 

    //Allocate array of pixel RGB values
    jsmpArray = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, nSampsPerRow, cinfo.image_height);

    if (DibToSamps2(bfh, lpbi, bmpbuffer, nSampsPerRow, cinfo, jsmpArray))
    {
        //Write the array of scan lines to the JPEG file
        (void)jpeg_write_scanlines(&cinfo, jsmpArray, cinfo.image_height);
    }

    jpeg_finish_compress(&cinfo); //Always finish
    fclose(pOutFile);
    jpeg_destroy_compress(&cinfo); //Free resources

	return NULL;
}

BOOL JpegFromDib(HANDLE     hDib,     //Handle to DIB
                 int        nQuality, //JPEG quality (0-100)
				 std::string    csJpeg,   //Pathname to jpeg file
				 std::string*   pcsMsg)   //Error msg to return
{
    //Basic sanity checks...
    if (nQuality < 0 || nQuality > 100 ||
        hDib   == NULL ||
        pcsMsg == NULL ||
        (!csJpeg.size()))
    {
        if (pcsMsg != NULL)
            *pcsMsg = "Invalid input data";

        return FALSE;
    }

    *pcsMsg = "";

    LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)hDib;

    byte *buf2 = 0;

    //Use libjpeg functions to write scanlines to disk in JPEG format
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;

    FILE*      pOutFile;     //Target file 
    int        nSampsPerRow; //Physical row width in image buffer 
    JSAMPARRAY jsmpArray;    //Pixel RGB buffer for JPEG file

    cinfo.err = jpeg_std_error(&jerr); //Use default error handling (ugly!)

    jpeg_create_compress(&cinfo);

    if ((pOutFile = fopen(csJpeg.c_str(), "wb")) == NULL)
    {
        *pcsMsg = "Cannot open ";
		*pcsMsg += csJpeg;
        jpeg_destroy_compress(&cinfo);
        return FALSE;
    }

    jpeg_stdio_dest(&cinfo, pOutFile);

    cinfo.image_width      = lpbi->biWidth;  //Image width and height, in pixels 
    cinfo.image_height     = lpbi->biHeight;
    cinfo.input_components = 3;              //Color components per pixel
                                             //(RGB_PIXELSIZE - see jmorecfg.h)
    cinfo.in_color_space   = JCS_RGB; 	     //Colorspace of input image

    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo,
                     nQuality, //Quality: 0-100 scale
                     TRUE);    //Limit to baseline-JPEG values

    jpeg_start_compress(&cinfo, TRUE);

    //JSAMPLEs per row in output buffer
    nSampsPerRow = cinfo.image_width * cinfo.input_components; 

    //Allocate array of pixel RGB values
    jsmpArray = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr) &cinfo,
                 JPOOL_IMAGE,
                 nSampsPerRow,
                 cinfo.image_height);

    if (DibToSamps(hDib,
                   nSampsPerRow,
                   cinfo,
                   jsmpArray,
                   pcsMsg))
    {
        //Write the array of scan lines to the JPEG file
        (void)jpeg_write_scanlines(&cinfo,
                                   jsmpArray,
                                   cinfo.image_height);
    }

    jpeg_finish_compress(&cinfo); //Always finish

    fclose(pOutFile);

    jpeg_destroy_compress(&cinfo); //Free resources

    if (pcsMsg->size())
        return FALSE;

    else
        return TRUE;
}

////////////////////////////////////////////////////////////////
//This function fills a jsmpArray with the RGB values
//for the CBitmap.
//
//It has been improved to handle all legal bitmap formats.
//
//A jsmpArray is a big array of RGB values, 3 bytes per value.
//
//Note that rows of pixels are processed bottom to top:
//The data in the jsamp array must be arranged top to bottom.
////////////////////////////////////////////////////////////////
BOOL DibToSamps2( BITMAPFILEHEADER & bfh, LPBITMAPINFOHEADER & pbBmHdr, BYTE * bmpbuffer, int nSampsPerRow, struct jpeg_compress_struct cinfo, JSAMPARRAY jsmpPixels)
{
   //Sanity...
   if (bmpbuffer == NULL || nSampsPerRow <= 0 ) 
   {
		cerr<<"DibToSamps failed"<<endl;
		exit(1);
   }

   int r=0, p=0, q=0, b=0, n=0, 
       nUnused=0, nBytesWide=0, nUsed=0, nLastBits=0, nLastNibs=0, nCTEntries=0,
       nRow=0, nByte=0, nPixel=0;
   BYTE bytCTEnt=0;

   switch (pbBmHdr->biBitCount)
   {
      case 1:
         nCTEntries = 2;   //Monochrome
         break;
      case 4:
         nCTEntries = 16;  //16-color
         break;
      case 8:
         nCTEntries = 256; //256-color
         break;
      case 16:
      case 24:
      case 32:
         nCTEntries = 0;   //No color table needed
         break;

      default:
		  cerr<<"Invalid bitmap bit count";
		  exit(1);
   }

   //Point to the color table and pixels
   DWORD     dwCTab = (DWORD)pbBmHdr + pbBmHdr->biSize;
   LPRGBQUAD pCTab  = (LPRGBQUAD)(dwCTab);
   LPSTR     lpBits = (LPSTR)pbBmHdr + (WORD)pbBmHdr->biSize + (WORD)(nCTEntries * sizeof(RGBQUAD));

   //Different formats for the image bits
   LPBYTE   lpPixels = (LPBYTE)  lpBits;
   RGBQUAD* pRgbQs   = (RGBQUAD*)lpBits;
   WORD*    wPixels  = (WORD*)   lpBits;

   //Set up the jsamps according to the bitmap's format.
   //Note that rows are processed bottom to top, because
   //that's how bitmaps are created.
   switch (pbBmHdr->biBitCount)
   {
      case 1:
         nUsed      = (pbBmHdr->biWidth + 7) / 8;
         nUnused    = (((nUsed + 3) / 4) * 4) - nUsed;
         nBytesWide = nUsed + nUnused;
         nLastBits  = 8 - ((nUsed * 8) - pbBmHdr->biWidth);

         for (r=0; r < pbBmHdr->biHeight; r++)
         {
            for (p=0,q=0; p < nUsed; p++) 
            { 
               nRow=(pbBmHdr->biHeight-r-1) * nBytesWide;
               nByte =  nRow + p;

               int nBUsed = (p <(nUsed-1)) ? 8 : nLastBits; for(b=0; b < nBUsed;b++) 
               { 
                  bytCTEnt = lpPixels[nByte] << b; 
                  bytCTEnt = bytCTEnt >> 7;

                  jsmpPixels[r][q+0] = pCTab[bytCTEnt].rgbRed;
                  jsmpPixels[r][q+1] = pCTab[bytCTEnt].rgbGreen;
                  jsmpPixels[r][q+2] = pCTab[bytCTEnt].rgbBlue;

                  q += 3;
               }
            }
         }
         break;

      case 4:
         nUsed      = (pbBmHdr->biWidth + 1) / 2;
         nUnused    = (((nUsed + 3) / 4) * 4) - nUsed;
         nBytesWide = nUsed + nUnused;
         nLastNibs  = 2 - ((nUsed * 2) - pbBmHdr->biWidth);

         for (r=0; r < pbBmHdr->biHeight;r++)
         {
            for (p=0,q=0; p < nUsed;p++) 
            { 
               nRow=(pbBmHdr->biHeight-r-1) * nBytesWide;
               nByte = nRow + p;
               int nNibbles = (p < nUsed - 1) ?
                  2 : nLastNibs;
               for(n=0;n < nNibbles;n++)
               {
                   bytCTEnt = lpPixels[nByte] << (n*4);
                   bytCTEnt = bytCTEnt >> (4-(n*4));
                   jsmpPixels[r][q+0] = pCTab[bytCTEnt].rgbRed;
                   jsmpPixels[r][q+1] = pCTab[bytCTEnt].rgbGreen;
                   jsmpPixels[r][q+2] = pCTab[bytCTEnt].rgbBlue;
                   q += 3;
               }
            }
         }
         break;

      default:
      case 8: //Each byte is a pointer to a pixel color
         nUnused = (((pbBmHdr->biWidth + 3) / 4) * 4) -
                   pbBmHdr->biWidth;

         for (r=0;r < pbBmHdr->biHeight; r++)
         {
            for (p=0,q=0; p < pbBmHdr->biWidth; p++,q+=3)
            {
               nRow   = (pbBmHdr->biHeight-r-1) * (pbBmHdr->biWidth + nUnused);
               nPixel =  nRow + p;

               jsmpPixels[r][q+0] = pCTab[lpPixels[nPixel]].rgbRed;
               jsmpPixels[r][q+1] = pCTab[lpPixels[nPixel]].rgbGreen;
               jsmpPixels[r][q+2] = pCTab[lpPixels[nPixel]].rgbBlue;
            }
         }
         break;

      case 16: //Hi-color (16 bits per pixel)
         for (r=0;r < pbBmHdr->biHeight; r++)
         {
            for (p=0,q=0; p < pbBmHdr->biWidth; p++,q+=3)
            {
               nRow    = (pbBmHdr->biHeight-r-1) * pbBmHdr->biWidth;
               nPixel  = nRow + p;

               RGBQUAD quad = QuadFromWord(wPixels[nPixel]);

               jsmpPixels[r][q+0] = quad.rgbRed;
               jsmpPixels[r][q+1] = quad.rgbGreen;
               jsmpPixels[r][q+2] = quad.rgbBlue;
            }
         }
         break;

      case 24:
         nBytesWide =  (pbBmHdr->biWidth*3);
         nUnused    =  (((nBytesWide + 3) / 4) * 4) -
                       nBytesWide;
         nBytesWide += nUnused;

         for (r=0;r<pbBmHdr->biHeight;r++)
         {
            for (p=0,q=0;p < (nBytesWide-nUnused); p+=3,q+=3)
            { 
               nRow = (pbBmHdr->biHeight-r-1) * nBytesWide;
               nPixel  = nRow + p;

               jsmpPixels[r][q+0] = lpPixels[nPixel+2]; //Red
               jsmpPixels[r][q+1] = lpPixels[nPixel+1]; //Green
               jsmpPixels[r][q+2] = lpPixels[nPixel+0]; //Blue
            }
         }
         break;

      case 32:
         for (r=0; r < pbBmHdr->biHeight; r++)
         {
            for (p=0,q=0; p < pbBmHdr->biWidth; p++,q+=3)
            {
               nRow    = (pbBmHdr->biHeight-r-1) *
                          pbBmHdr->biWidth;
               nPixel  = nRow + p;

               jsmpPixels[r][q+0] = pRgbQs[nPixel].rgbRed;
               jsmpPixels[r][q+1] = pRgbQs[nPixel].rgbGreen;
               jsmpPixels[r][q+2] = pRgbQs[nPixel].rgbBlue;
            }
         }
         break;
   }   //end switch

return TRUE;
}

BOOL DibToSamps(HANDLE                      hDib,
                int                         nSampsPerRow,
                struct jpeg_compress_struct cinfo,
                JSAMPARRAY                  jsmpPixels,
				std::string*                    pcsMsg)
{
   //Sanity...
   if (hDib == NULL    ||
     nSampsPerRow <= 0 || pcsMsg == NULL) 
   { 
     if (pcsMsg !=NULL) 
        *pcsMsg="Invalid input data"; 
     return FALSE; 
   } 

   int r=0, p=0, q=0, b=0, n=0, 
       nUnused=0, nBytesWide=0, nUsed=0, nLastBits=0, nLastNibs=0, nCTEntries=0,
       nRow=0, nByte=0, nPixel=0;
   BYTE bytCTEnt=0;
   LPBITMAPINFOHEADER pbBmHdr= (LPBITMAPINFOHEADER)hDib; //The bit count tells you the format of the bitmap: //Decide how many entries will be in the color table (if any) 

   switch (pbBmHdr->biBitCount)
   {
      case 1:
         nCTEntries = 2;   //Monochrome
         break;

      case 4:
         nCTEntries = 16;  //16-color
         break;

      case 8:
         nCTEntries = 256; //256-color
         break;

      case 16:
      case 24:
      case 32:
         nCTEntries = 0;   //No color table needed
         break;

      default:
         *pcsMsg = "Invalid bitmap bit count";
         return FALSE; //Unsupported format
   }

   //Point to the color table and pixels
   DWORD     dwCTab = (DWORD)pbBmHdr + pbBmHdr->biSize;
   LPRGBQUAD pCTab  = (LPRGBQUAD)(dwCTab);
   LPSTR     lpBits = (LPSTR)pbBmHdr +
                      (WORD)pbBmHdr->biSize +
                      (WORD)(nCTEntries * sizeof(RGBQUAD));

   //Different formats for the image bits
   LPBYTE   lpPixels = (LPBYTE)  lpBits;
   RGBQUAD* pRgbQs   = (RGBQUAD*)lpBits;
   WORD*    wPixels  = (WORD*)   lpBits;

   //Set up the jsamps according to the bitmap's format.
   //Note that rows are processed bottom to top, because
   //that's how bitmaps are created.
   switch (pbBmHdr->biBitCount)
   {
      case 1:
         nUsed      = (pbBmHdr->biWidth + 7) / 8;
         nUnused    = (((nUsed + 3) / 4) * 4) - nUsed;
         nBytesWide = nUsed + nUnused;
         nLastBits  = 8 - ((nUsed * 8) - pbBmHdr->biWidth);

         for (r=0; r < pbBmHdr->biHeight; r++)
         {
            for (p=0,q=0; p < nUsed; p++) 
            { 
               nRow=(pbBmHdr->biHeight-r-1) * nBytesWide;
               nByte =  nRow + p;

               int nBUsed = (p <(nUsed-1)) ? 8 : nLastBits; for(b=0; b < nBUsed;b++) 
               { 
                  bytCTEnt = lpPixels[nByte] << b; 
                  bytCTEnt = bytCTEnt >> 7;

                  jsmpPixels[r][q+0] = pCTab[bytCTEnt].rgbRed;
                  jsmpPixels[r][q+1] = pCTab[bytCTEnt].rgbGreen;
                  jsmpPixels[r][q+2] = pCTab[bytCTEnt].rgbBlue;

                  q += 3;
               }
            }
         }
         break;

      case 4:
         nUsed      = (pbBmHdr->biWidth + 1) / 2;
         nUnused    = (((nUsed + 3) / 4) * 4) - nUsed;
         nBytesWide = nUsed + nUnused;
         nLastNibs  = 2 - ((nUsed * 2) - pbBmHdr->biWidth);

         for (r=0; r < pbBmHdr->biHeight;r++)
         {
            for (p=0,q=0; p < nUsed;p++) 
            { 
               nRow=(pbBmHdr->biHeight-r-1) * nBytesWide;
               nByte = nRow + p;
               int nNibbles = (p < nUsed - 1) ?
                  2 : nLastNibs;
               for(n=0;n < nNibbles;n++)
               {
                   bytCTEnt = lpPixels[nByte] << (n*4);
                   bytCTEnt = bytCTEnt >> (4-(n*4));
                   jsmpPixels[r][q+0] = pCTab[bytCTEnt].rgbRed;
                   jsmpPixels[r][q+1] = pCTab[bytCTEnt].rgbGreen;
                   jsmpPixels[r][q+2] = pCTab[bytCTEnt].rgbBlue;
                   q += 3;
               }
            }
         }
         break;

      default:
      case 8: //Each byte is a pointer to a pixel color
         nUnused = (((pbBmHdr->biWidth + 3) / 4) * 4) -
                   pbBmHdr->biWidth;

         for (r=0;r < pbBmHdr->biHeight; r++)
         {
            for (p=0,q=0; p < pbBmHdr->biWidth; p++,q+=3)
            {
               nRow   = (pbBmHdr->biHeight-r-1) * (pbBmHdr->biWidth + nUnused);
               nPixel =  nRow + p;

               jsmpPixels[r][q+0] = pCTab[lpPixels[nPixel]].rgbRed;
               jsmpPixels[r][q+1] = pCTab[lpPixels[nPixel]].rgbGreen;
               jsmpPixels[r][q+2] = pCTab[lpPixels[nPixel]].rgbBlue;
            }
         }
         break;

      case 16: //Hi-color (16 bits per pixel)
         for (r=0;r < pbBmHdr->biHeight; r++)
         {
            for (p=0,q=0; p < pbBmHdr->biWidth; p++,q+=3)
            {
               nRow    = (pbBmHdr->biHeight-r-1) * pbBmHdr->biWidth;
               nPixel  = nRow + p;

               RGBQUAD quad = QuadFromWord(wPixels[nPixel]);

               jsmpPixels[r][q+0] = quad.rgbRed;
               jsmpPixels[r][q+1] = quad.rgbGreen;
               jsmpPixels[r][q+2] = quad.rgbBlue;
            }
         }
         break;

      case 24:
         nBytesWide =  (pbBmHdr->biWidth*3);
         nUnused    =  (((nBytesWide + 3) / 4) * 4) -
                       nBytesWide;
         nBytesWide += nUnused;

         for (r=0;r<pbBmHdr->biHeight;r++)
         {
            for (p=0,q=0;p < (nBytesWide-nUnused); p+=3,q+=3)
            { 
               nRow = (pbBmHdr->biHeight-r-1) * nBytesWide;
               nPixel  = nRow + p;

               jsmpPixels[r][q+0] = lpPixels[nPixel+2]; //Red
               jsmpPixels[r][q+1] = lpPixels[nPixel+1]; //Green
               jsmpPixels[r][q+2] = lpPixels[nPixel+0]; //Blue
            }
         }
         break;

      case 32:
         for (r=0; r < pbBmHdr->biHeight; r++)
         {
            for (p=0,q=0; p < pbBmHdr->biWidth; p++,q+=3)
            {
               nRow    = (pbBmHdr->biHeight-r-1) *
                          pbBmHdr->biWidth;
               nPixel  = nRow + p;

               jsmpPixels[r][q+0] = pRgbQs[nPixel].rgbRed;
               jsmpPixels[r][q+1] = pRgbQs[nPixel].rgbGreen;
               jsmpPixels[r][q+2] = pRgbQs[nPixel].rgbBlue;
            }
         }
         break;
   }   //end switch

return TRUE;
}
 
////////////////////////////////////////
//This function turns a 16-bit pixel
//into an RGBQUAD value.
////////////////////////////////////////
RGBQUAD QuadFromWord(WORD b16)
{
   BYTE bytVals[] =
   {
     0,  16, 24, 32,  40, 48, 56, 64,
     72, 80, 88, 96, 104,112,120,128,
     136,144,152,160,168,176,184,192,
     200,208,216,224,232,240,248,255
   };

   WORD wR = b16;
   WORD wG = b16;
   WORD wB = b16;

   wR <<= 1; wR >>= 11;
   wG <<= 6; wG >>= 11;
   wB <<= 11; wB >>= 11;

   RGBQUAD rgb;

   rgb.rgbReserved = 0;
   rgb.rgbBlue     = bytVals[wB];
   rgb.rgbGreen    = bytVals[wG];
   rgb.rgbRed      = bytVals[wR];

   return rgb;
}

#endif

