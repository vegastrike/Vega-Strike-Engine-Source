#if !defined(NETCOMM_NOWEBCAM)

#include <iostream>
#include "webcam_support.h"
#include "lin_time.h"
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "gldrv/winsys.h"

using std::cerr;
using std::endl;
using std::hex;
extern bool cleanexit;

typedef void* (*VoidVoidFuncType)( void* );

#ifdef DSHOW
#include "gfx/jpeg_memory.h"
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
				VSExit(1);
			}
			if(MediaType.majortype != MEDIATYPE_Video)
			{
				cerr<<"INVALID MEDIA TYPE 1"<<endl;
				cleanexit = true;
				VSExit(1);
				return VFW_E_INVALIDMEDIATYPE;
			}
			if(MediaType.formattype != FORMAT_VideoInfo)
			{
				cerr<<"INVALID MEDIA TYPE 2"<<endl;
				cleanexit = true;
				VSExit(1);
				return VFW_E_INVALIDMEDIATYPE;
			}
			if(MediaType.cbFormat < sizeof(VIDEOINFOHEADER))
			{
				cerr<<"INVALID MEDIA TYPE 3"<<endl;
				cleanexit = true;
				VSExit(1);
				return VFW_E_INVALIDMEDIATYPE;
			}
			if(MediaType.pbFormat == NULL)
			{
				cerr<<"INVALID MEDIA TYPE 4"<<endl;
				cleanexit = true;
				VSExit(1);
				return VFW_E_INVALIDMEDIATYPE;
			}

			/*
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
			*/
			/*
			hr = ws->pSampleGrabber->GetCurrentBuffer(&size,(long *)buffer);   
			if (FAILED(hr)) 
			{
				cerr<<"GETCURRENTBUFFER";
				cerr<<" - code : "<<hr<<" HEX : ";
				cerr<<hex<<hr<<endl;
				cleanexit = true;
				VSExit(1);
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
			
			/*
			// Write p to a file for testing
			char file[256];
			memset( file, 0, 256);
			sprintf( file, "%s%d%s", "testcam", ws->nbframes, ".bmp");
			//string path = VSFileSystem::datadir+"testcam"+string( ws->nbframes)+".jpg";
			string path = VSFileSystem::datadir+file;
			FILE * fp;
			fp = VSFileSystem::vs_open( path.c_str(), "wb");
			if( !fp)
			{
				cerr<<"opening jpeg file failed"<<endl;
				VSExit(1);
			}
			if( VSFileSystem::Write( &BitmapInfo, 1, sizeof(BITMAPINFOHEADER), fp)!=sizeof(BITMAPINFOHEADER))
			{
				cerr<<"!!! ERROR : writing jpeg description to file 1"<<endl;
				VSExit(1);
			}
			if( VSFileSystem::Write( pBuffer, 1, BufferLen, fp)!=BufferLen)
			{
				cerr<<"!!! ERROR : writing jpeg description to file"<<endl;
				VSExit(1);
			}
			VSFileSystem::vs_close( fp);
			*/

			// jpeg_compress doesn't allocate any data so we have to use a buffer for destination
			// Here we can't 
			char file[256];
			char fullfile[256];
			memset( file, 0, 256);
			memset( fullfile, 0, 256);
			sprintf( file, "%s%d%s", "testcam", ws->nbframes, ".jpg");
			strcat( fullfile, VSFileSystem::datadir.c_str());
			strcat( fullfile, file);
/*
METHODDEF(void) init_destination (j_compress_ptr cinfo);
METHODDEF(boolean) empty_output_buffer (j_compress_ptr cinfo);
METHODDEF(void) term_destination (j_compress_ptr cinfo);
GLOBAL(void) jpeg_memory_dest(j_compress_ptr cinfo, JOCTET *buffer,int bufsize);
int jpeg_compress(char *dst, char *src, int width, int height, int dstsize, int quality);
int jpeg_compress_to_file(char *src, char *file, int width, int height, int quality);
extern void jpeg_memory_src(j_decompress_ptr cinfo, unsigned char *ptr, size_t size);
void jpeg_decompress(unsigned char *dst, unsigned char *src, int size, int *w, int *h);
void jpeg_decompress_from_file(unsigned char *dst, char *file, int size, int *w, int *h);
*/
			char * tmpBuffer = new char[MAXBUFFER];
			cerr<<"Trying to save cam shot to : "<<file<<endl;
			jpeg_compress_to_file( (char *)pBuffer, fullfile, ws->width, ws->height, ws->jpeg_quality);
			cerr<<"Trying to compress webcam buffer"<<endl;
			ws->jpeg_size = jpeg_compress( tmpBuffer, (char *)pBuffer, ws->width, ws->height, MAXBUFFER, ws->jpeg_quality);
			cerr<<"\tCompressed into "<<ws->jpeg_size<<" bytes"<<endl;
			ws->jpeg_buffer = new char[ws->jpeg_size];
			memcpy( ws->jpeg_buffer, tmpBuffer, ws->jpeg_size);
			ws->nbframes++;
			
			//ws->jpeg_buffer = JpegFromCapture( hBitmap, pBuffer, BufferLen, &ws->jpeg_size, ws->jpeg_quality, "c:\test.jpg");
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
			//string path = VSFileSystem::datadir+"testcam"+string( ws->nbframes)+".jpg";
			string path = VSFileSystem::datadir+file;
			FILE * fp;
			fp = VSFileSystem::vs_open( path.c_str(), "wb");
			if( !fp)
			{
				cerr<<"opening jpeg file failed"<<endl;
				VSExit(1);
			}
			if( VSFileSystem::Write( p, 1, len, fp)!=len)
			{
				cerr<<"writing jpeg description to file"<<endl;
				VSExit(1);
			}
			VSFileSystem::vs_close( fp);
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
		VSExit(1);
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
#endif
	return 0;
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
		VSExit(1);
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

    if ((pOutFile = VSFileSystem::vs_open(csJpeg.c_str(), "wb")) == NULL)
	{
		cerr<<"opening of jpeg file failed.";
		VSExit(1);
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
    VSFileSystem::vs_close(pOutFile);
    jpeg_destroy_compress(&cinfo); //Free resources

	return NULL;
}

char * JpegFromCapture(HANDLE     hDib, BYTE * bmpBuffer, long bmpLength, int jpegLength, int nQuality,
				 std::string    csJpeg)
{
    //Basic sanity checks...
    if (nQuality < 0 || nQuality > 100 ||
        hDib   == NULL ||
        (!csJpeg.size()))
    {
		cerr<<"!!! ERROR JpegFromDib : Bad Parameters"<<endl;
		VSExit(1);
    }

    LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)hDib;

    JOCTET *jpegBuffer = NULL;

    //Use libjpeg functions to write scanlines to disk in JPEG format
    struct jpeg_decompress_struct cdinfo;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;

    FILE*      pOutFile;     //Target file 
    int        nSampsPerRow; //Physical row width in image buffer 
    JSAMPARRAY jsmpArray;    //Pixel RGB buffer for JPEG file

    cinfo.err = jpeg_std_error(&jerr); //Use default error handling (ugly!)

    init_destination(&cinfo);

	// This one causes problem but we are not using that JpegFromCapture function anyway
	//empty_output_buffer(&cinfo);

    jpeg_memory_dest(&cinfo, jpegBuffer, jpegLength);

	/* Try with jpeg_memory stuff */
	jpeg_memory_src(&cdinfo, bmpBuffer, bmpLength);

	/*
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

    jpeg_destroy_compress(&cinfo); //Free resources
	*/
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

    if ((pOutFile = VSFileSystem::vs_open(csJpeg.c_str(), "wb")) == NULL)
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

    VSFileSystem::vs_close(pOutFile);

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
		VSExit(1);
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
		  VSExit(1);
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

#ifdef linux

/* NOTE: Requires BTTV compatible framegrabber card for full functionality. */
/*       Might work with other cards/input devices.                         */

/* Channel Frequencies */
/* US broadcast */
static struct CHANLIST ntsc_bcast[] = {
    { "2",	 55250 },
    { "3",	 61250 },
    { "4",	 67250 },
    { "5",	 77250 },
    { "6",	 83250 },
    { "7",	175250 },
    { "8",	181250 },
    { "9",	187250 },
    { "10",	193250 },
    { "11",	199250 },
    { "12",	205250 },
    { "13",	211250 },
    { "14",	471250 },
    { "15",	477250 },
    { "16",	483250 },
    { "17",	489250 },
    { "18",	495250 },
    { "19",	501250 },
    { "20",	507250 },
    { "21",	513250 },
    { "22",	519250 },
    { "23",	525250 },
    { "24",	531250 },
    { "25",	537250 },
    { "26",	543250 },
    { "27",	549250 },
    { "28",	555250 },
    { "29",	561250 },
    { "30",	567250 },
    { "31",	573250 },
    { "32",	579250 },
    { "33",	585250 },
    { "34",	591250 },
    { "35",	597250 },
    { "36",	603250 },
    { "37",	609250 },
    { "38",	615250 },
    { "39",	621250 },
    { "40",	627250 },
    { "41",	633250 },
    { "42",	639250 },
    { "43",	645250 },
    { "44",	651250 },
    { "45",	657250 },
    { "46",	663250 },
    { "47",	669250 },
    { "48",	675250 },
    { "49",	681250 },
    { "50",	687250 },
    { "51",	693250 },
    { "52",	699250 },
    { "53",	705250 },
    { "54",	711250 },
    { "55",	717250 },
    { "56",	723250 },
    { "57",	729250 },
    { "58",	735250 },
    { "59",	741250 },
    { "60",	747250 },
    { "61",	753250 },
    { "62",	759250 },
    { "63",	765250 },
    { "64",	771250 },
    { "65",	777250 },
    { "66",	783250 },
    { "67",	789250 },
    { "68",	795250 },
    { "69",	801250 },
    { "70",	807250 },
    { "71",	813250 },
    { "72",	819250 },
    { "73",	825250 },
    { "74",	831250 },
    { "75",	837250 },
    { "76",	843250 },
    { "77",	849250 },
    { "78",	855250 },
    { "79",	861250 },
    { "80",	867250 },
    { "81",	873250 },
    { "82",	879250 },
    { "83",	885250 },
};

/* US cable */
static struct CHANLIST ntsc_cable[] = {
    { "1",	 73250 },
    { "2",	 55250 },
    { "3",	 61250 },
    { "4",	 67250 },
    { "5",	 77250 },
    { "6",	 83250 },
    { "7",	175250 },
    { "8",	181250 },
    { "9",	187250 },
    { "10",	193250 },
    { "11",	199250 },
    { "12",	205250 },
    { "13",	211250 },
    { "14",	121250 },
    { "15",	127250 },
    { "16",	133250 },
    { "17",	139250 },
    { "18",	145250 },
    { "19",	151250 },
    { "20",	157250 },
    { "21",	163250 },
    { "22",	169250 },
    { "23",	217250 },
    { "24",	223250 },
    { "25",	229250 },
    { "26",	235250 },
    { "27",	241250 },
    { "28",	247250 },
    { "29",	253250 },
    { "30",	259250 },
    { "31",	265250 },
    { "32",	271250 },
    { "33",	277250 },
    { "34",	283250 },
    { "35",	289250 },
    { "36",	295250 },
    { "37",	301250 },
    { "38",	307250 },
    { "39",	313250 },
    { "40",	319250 },
    { "41",	325250 },
    { "42",	331250 },
    { "43",	337250 },
    { "44",	343250 },
    { "45",	349250 },
    { "46",	355250 },
    { "47",	361250 },
    { "48",	367250 },
    { "49",	373250 },
    { "50",	379250 },
    { "51",	385250 },
    { "52",	391250 },
    { "53",	397250 },
    { "54",	403250 },
    { "55",	409250 },
    { "56",	415250 },
    { "57",	421250 },
    { "58",	427250 },
    { "59",	433250 },
    { "60",	439250 },
    { "61",	445250 },
    { "62",	451250 },
    { "63",	457250 },
    { "64",	463250 },
    { "65",	469250 },
    { "66",	475250 },
    { "67",	481250 },
    { "68",	487250 },
    { "69",	493250 },
    { "70",	499250 },
    { "71",	505250 },
    { "72",	511250 },
    { "73",	517250 },
    { "74",	523250 },
    { "75",	529250 },
    { "76",	535250 },
    { "77",	541250 },
    { "78",	547250 },
    { "79",	553250 },
    { "80",	559250 },
    { "81",	565250 },
    { "82",	571250 },
    { "83",	577250 },
    { "84",	583250 },
    { "85",	589250 },
    { "86",	595250 },
    { "87",	601250 },
    { "88",	607250 },
    { "89",	613250 },
    { "90",	619250 },
    { "91",	625250 },
    { "92",	631250 },
    { "93",	637250 },
    { "94",	643250 },
    { "95",	 91250 },
    { "96",	 97250 },
    { "97",	103250 },
    { "98",	109250 },
    { "99",	115250 },
    { "100",	649250 },
    { "101",	655250 },
    { "102",	661250 },
    { "103",	667250 },
    { "104",	673250 },
    { "105",	679250 },
    { "106",	685250 },
    { "107",	691250 },
    { "108",	697250 },
    { "109",	703250 },
    { "110",	709250 },
    { "111",	715250 },
    { "112",	721250 },
    { "113",	727250 },
    { "114",	733250 },
    { "115",	739250 },
    { "116",	745250 },
    { "117",	751250 },
    { "118",	757250 },
    { "119",	763250 },
    { "120",	769250 },
    { "121",	775250 },
    { "122",	781250 },
    { "123",	787250 },
    { "124",	793250 },
    { "125",	799250 },
    { "T7", 	  8250 },
    { "T8",	 14250 },
    { "T9",	 20250 },
    { "T10",	 26250 },
    { "T11",	 32250 },
    { "T12",	 38250 },
    { "T13",	 44250 },
    { "T14",	 50250 }
};

/* US HRC */
static struct CHANLIST ntsc_hrc[] = {
    { "1",	  72000 },
    { "2",	  54000 }, 
    { "3",	  60000 }, 
    { "4",	  66000 }, 
    { "5",	  78000 }, 
    { "6",	  84000 }, 
    { "7",	 174000 },
    { "8",	 180000 },
    { "9",	 186000 },
    { "10",	 192000 },
    { "11",	 198000 },
    { "12",	 204000 },
    { "13",	 210000 },
    { "14",	 120000 },
    { "15",	 126000 },
    { "16",	 132000 },
    { "17",	 138000 },
    { "18",	 144000 },
    { "19",	 150000 },
    { "20",	 156000 },
    { "21",	 162000 },
    { "22",	 168000 },
    { "23",	 216000 },
    { "24",	 222000 },
    { "25",	 228000 },
    { "26",	 234000 },
    { "27",	 240000 },
    { "28",	 246000 },
    { "29",	 252000 },
    { "30",	 258000 },
    { "31",	 264000 },
    { "32",	 270000 },
    { "33",	 276000 },
    { "34",	 282000 },
    { "35",	 288000 },
    { "36",	 294000 },
    { "37",	 300000 },
    { "38",	 306000 },
    { "39",	 312000 },
    { "40",	 318000 },
    { "41",	 324000 },
    { "42",	 330000 },
    { "43",	 336000 },
    { "44",	 342000 },
    { "45",	 348000 },
    { "46",	 354000 },
    { "47",	 360000 },
    { "48",	 366000 },
    { "49",	 372000 },
    { "50",	 378000 },
    { "51",	 384000 },
    { "52",	 390000 },
    { "53",	 396000 },
    { "54",	 402000 },
    { "55",	 408000 },
    { "56",	 414000 },
    { "57",	 420000 },
    { "58",	 426000 },
    { "59",	 432000 },
    { "60",	 438000 },
    { "61",	 444000 },
    { "62",	 450000 },
    { "63",	 456000 },
    { "64",	 462000 },
    { "65",	 468000 },
    { "66",	 474000 },
    { "67",	 480000 },
    { "68",	 486000 },
    { "69",	 492000 },
    { "70",	 498000 },
    { "71",	 504000 },
    { "72",	 510000 },
    { "73",	 516000 },
    { "74",	 522000 },
    { "75",	 528000 },
    { "76",	 534000 },
    { "77",	 540000 },
    { "78",	 546000 },
    { "79",	 552000 },
    { "80",	 558000 },
    { "81",	 564000 },
    { "82",	 570000 },
    { "83",	 576000 },
    { "84",	 582000 },
    { "85",	 588000 },
    { "86",	 594000 },
    { "87",	 600000 },
    { "88",	 606000 },
    { "89",	 612000 },
    { "90",	 618000 },
    { "91",	 624000 },
    { "92",	 630000 },
    { "93",	 636000 },
    { "94",	 642000 },
    { "95",	 900000 },
    { "96",	 960000 },
    { "97",	 102000 },
    { "98",	 108000 },
    { "99",	 114000 },
    { "100",	 648000 },
    { "101",	 654000 },
    { "102",	 660000 },
    { "103",	 666000 },
    { "104",	 672000 },
    { "105",	 678000 },
    { "106",	 684000 },
    { "107",	 690000 },
    { "108",	 696000 },
    { "109",	 702000 },
    { "110",	 708000 },
    { "111",	 714000 },
    { "112",	 720000 },
    { "113",	 726000 },
    { "114",	 732000 },
    { "115",	 738000 },
    { "116",	 744000 },
    { "117",	 750000 },
    { "118",	 756000 },
    { "119",	 762000 },
    { "120",	 768000 },
    { "121",	 774000 },
    { "122",	 780000 },
    { "123",	 786000 },
    { "124",	 792000 },
    { "125",	 798000 },
    { "T7",	   7000 },  
    { "T8",	  13000 }, 
    { "T9",	  19000 }, 
    { "T10",	  25000 }, 
    { "T11",	  31000 }, 
    { "T12",	  37000 }, 
    { "T13",	  43000 }, 
    { "T14",	  49000 }, 
};

/* --------------------------------------------------------------------- */

/* JP broadcast */
static struct CHANLIST ntsc_bcast_jp[] = {
    { "1",   91250 },
    { "2",   97250 },
    { "3",  103250 },
    { "4",  171250 },
    { "5",  177250 },
    { "6",  183250 },
    { "7",  189250 },
    { "8",  193250 },
    { "9",  199250 },
    { "10", 205250 },
    { "11", 211250 },
    { "12", 217250 },
    { "13", 471250 },
    { "14", 477250 },
    { "15", 483250 },
    { "16", 489250 },
    { "17", 495250 },
    { "18", 501250 },
    { "19", 507250 },
    { "20", 513250 },
    { "21", 519250 },
    { "22", 525250 },
    { "23", 531250 },
    { "24", 537250 },
    { "25", 543250 },
    { "26", 549250 },
    { "27", 555250 },
    { "28", 561250 },
    { "29", 567250 },
    { "30", 573250 },
    { "31", 579250 },
    { "32", 585250 },
    { "33", 591250 },
    { "34", 597250 },
    { "35", 603250 },
    { "36", 609250 },
    { "37", 615250 },
    { "38", 621250 },
    { "39", 627250 },
    { "40", 633250 },
    { "41", 639250 },
    { "42", 645250 },
    { "43", 651250 },
    { "44", 657250 },
    { "45", 663250 },
    { "46", 669250 },
    { "47", 675250 },
    { "48", 681250 },
    { "49", 687250 },
    { "50", 693250 },
    { "51", 699250 },
    { "52", 705250 },
    { "53", 711250 },
    { "54", 717250 },
    { "55", 723250 },
    { "56", 729250 },
    { "57", 735250 },
    { "58", 741250 },
    { "59", 747250 },
    { "60", 753250 },
    { "61", 759250 },
    { "62", 765250 },
};

/* JP cable */
static struct CHANLIST ntsc_cable_jp[] = {
    { "13",	109250 },
    { "14",	115250 },
    { "15",	121250 },
    { "16",	127250 },
    { "17",	133250 },
    { "18",	139250 },
    { "19",	145250 },
    { "20",	151250 }, 
    { "21",	157250 },
    { "22",	165250 },
    { "23",	223250 },
    { "24",	231250 },
    { "25",	237250 },
    { "26",	243250 },
    { "27",	249250 },
    { "28",	253250 },
    { "29",	259250 },
    { "30",	265250 },
    { "31",	271250 },
    { "32",	277250 },
    { "33",	283250 },
    { "34",	289250 },
    { "35",	295250 },
    { "36",	301250 },
    { "37",	307250 },
    { "38",	313250 },
    { "39",	319250 },
    { "40",	325250 },
    { "41",	331250 },
    { "42",	337250 },
    { "43",	343250 },
    { "44",	349250 },
    { "45", 	355250 },
    { "46", 	361250 },
    { "47", 	367250 },
    { "48", 	373250 },
    { "49", 	379250 },
    { "50", 	385250 },
    { "51", 	391250 },
    { "52", 	397250 },
    { "53", 	403250 },
    { "54", 	409250 },
    { "55", 	415250 },
    { "56", 	421250 },
    { "57", 	427250 },
    { "58", 	433250 },
    { "59", 	439250 },
    { "60", 	445250 },
    { "61", 	451250 },
    { "62", 	457250 },
    { "63",	463250 },
};

/* --------------------------------------------------------------------- */

/* australia */
static struct CHANLIST pal_australia[] = {
    { "0",	 46250 },
    { "1",	 57250 },
    { "2",	 64250 },
    { "3",	 86250 },
    { "4",  	 95250 },
    { "5",  	102250 },
    { "6",  	175250 },
    { "7",  	182250 },
    { "8",  	189250 },
    { "9",  	196250 },
    { "10", 	209250 },
    { "11",	216250 },
    { "28",	527250 },
    { "29",	534250 },
    { "30",	541250 },
    { "31",	548250 },
    { "32",	555250 },
    { "33",	562250 },
    { "34",	569250 },
    { "35",	576250 },
    { "39",	604250 },
    { "40",	611250 },
    { "41",	618250 },
    { "42",	625250 },
    { "43",	632250 },
    { "44",	639250 },
    { "45",	646250 },
    { "46",	653250 },
    { "47",	660250 },
    { "48",	667250 },
    { "49",	674250 },
    { "50",	681250 },
    { "51",	688250 },
    { "52",	695250 },
    { "53",	702250 },
    { "54",	709250 },
    { "55",	716250 },
    { "56",	723250 },
    { "57",	730250 },
    { "58",	737250 },
    { "59",	744250 },
    { "60",	751250 },
    { "61",	758250 },
    { "62",	765250 },
    { "63",	772250 },
    { "64",	779250 },
    { "65",	786250 },
    { "66",	793250 },
    { "67",	800250 },
    { "68",	807250 },
    { "69",	814250 },
};

/* --------------------------------------------------------------------- */
/* europe                                                                */

/* CCIR frequencies */

#define FREQ_CCIR_I_III		\
    { "E2",	  48250 },	\
    { "E3",	  55250 },	\
    { "E4",	  62250 },	\
				\
    { "S01",	  69250 },	\
    { "S02",	  76250 },	\
    { "S03",	  83250 },	\
				\
    { "E5",	 175250 },	\
    { "E6",	 182250 },	\
    { "E7",	 189250 },	\
    { "E8",	 196250 },	\
    { "E9",	 203250 },	\
    { "E10",	 210250 },	\
    { "E11",	 217250 },	\
    { "E12",	 224250 }

#define FREQ_CCIR_SL_SH		\
    { "SE1",	 105250 },	\
    { "SE2",	 112250 },	\
    { "SE3",	 119250 },	\
    { "SE4",	 126250 },	\
    { "SE5",	 133250 },	\
    { "SE6",	 140250 },	\
    { "SE7",	 147250 },	\
    { "SE8",	 154250 },	\
    { "SE9",	 161250 },	\
    { "SE10",    168250 },	\
				\
    { "SE11",    231250 },	\
    { "SE12",    238250 },	\
    { "SE13",    245250 },	\
    { "SE14",    252250 },	\
    { "SE15",    259250 },	\
    { "SE16",    266250 },	\
    { "SE17",    273250 },	\
    { "SE18",    280250 },	\
    { "SE19",    287250 },	\
    { "SE20",    294250 }

#define FREQ_CCIR_H	\
    { "S21", 303250 },	\
    { "S22", 311250 },	\
    { "S23", 319250 },	\
    { "S24", 327250 },	\
    { "S25", 335250 },	\
    { "S26", 343250 },	\
    { "S27", 351250 },	\
    { "S28", 359250 },	\
    { "S29", 367250 },	\
    { "S30", 375250 },	\
    { "S31", 383250 },	\
    { "S32", 391250 },	\
    { "S33", 399250 },	\
    { "S34", 407250 },	\
    { "S35", 415250 },	\
    { "S36", 423250 },	\
    { "S37", 431250 },	\
    { "S38", 439250 },	\
    { "S39", 447250 },	\
    { "S40", 455250 },	\
    { "S41", 463250 }

/* OIRT frequencies */

#define FREQ_OIRT_I_III		\
    { "R1",       49750 },	\
    { "R2",       59250 },	\
				\
    { "R3",       77250 },	\
    { "R4",       84250 },	\
    { "R5",       93250 },	\
				\
    { "R6",	 175250 },	\
    { "R7",	 183250 },	\
    { "R8",	 191250 },	\
    { "R9",	 199250 },	\
    { "R10",	 207250 },	\
    { "R11",	 215250 },	\
    { "R12",	 223250 }

#define FREQ_OIRT_SL_SH		\
    { "SR1",	 111250 },	\
    { "SR2",	 119250 },	\
    { "SR3",	 127250 },	\
    { "SR4",	 135250 },	\
    { "SR5",	 143250 },	\
    { "SR6",	 151250 },	\
    { "SR7",	 159250 },	\
    { "SR8",	 167250 },	\
				\
    { "SR11",    231250 },	\
    { "SR12",    239250 },	\
    { "SR13",    247250 },	\
    { "SR14",    255250 },	\
    { "SR15",    263250 },	\
    { "SR16",    271250 },	\
    { "SR17",    279250 },	\
    { "SR18",    287250 },	\
    { "SR19",    295250 }

#define FREQ_UHF	\
    { "21",  471250 },	\
    { "22",  479250 },	\
    { "23",  487250 },	\
    { "24",  495250 },	\
    { "25",  503250 },	\
    { "26",  511250 },	\
    { "27",  519250 },	\
    { "28",  527250 },	\
    { "29",  535250 },	\
    { "30",  543250 },	\
    { "31",  551250 },	\
    { "32",  559250 },	\
    { "33",  567250 },	\
    { "34",  575250 },	\
    { "35",  583250 },	\
    { "36",  591250 },	\
    { "37",  599250 },	\
    { "38",  607250 },	\
    { "39",  615250 },	\
    { "40",  623250 },	\
    { "41",  631250 },	\
    { "42",  639250 },	\
    { "43",  647250 },	\
    { "44",  655250 },	\
    { "45",  663250 },	\
    { "46",  671250 },	\
    { "47",  679250 },	\
    { "48",  687250 },	\
    { "49",  695250 },	\
    { "50",  703250 },	\
    { "51",  711250 },	\
    { "52",  719250 },	\
    { "53",  727250 },	\
    { "54",  735250 },	\
    { "55",  743250 },	\
    { "56",  751250 },	\
    { "57",  759250 },	\
    { "58",  767250 },	\
    { "59",  775250 },	\
    { "60",  783250 },	\
    { "61",  791250 },	\
    { "62",  799250 },	\
    { "63",  807250 },	\
    { "64",  815250 },	\
    { "65",  823250 },	\
    { "66",  831250 },	\
    { "67",  839250 },	\
    { "68",  847250 },	\
    { "69",  855250 }

static struct CHANLIST pal_europe[] = {
    FREQ_CCIR_I_III,
    FREQ_CCIR_SL_SH,
    FREQ_CCIR_H,
    FREQ_UHF
};

static struct CHANLIST pal_europe_east[] = {
    FREQ_OIRT_I_III,
    FREQ_OIRT_SL_SH,
    FREQ_CCIR_H,
    FREQ_UHF
};

static struct CHANLIST pal_italy[] = {
    { "2",	 53750 },
    { "3",	 62250 },
    { "4",	 82250 },
    { "5",	175250 },
    { "6",	183750 },
    { "7",	192250 },
    { "8",	201250 },
    { "9",	210250 },
    { "10",	210250 },
    { "11",	217250 },
    { "12",	224250 },
};

static struct CHANLIST pal_ireland[] = {
    { "0",    45750 },
    { "1",    53750 },
    { "2",    61750 },
    { "3",   175250 },
    { "4",   183250 },
    { "5",   191250 },
    { "6",   199250 },
    { "7",   207250 },
    { "8",   215250 },
    FREQ_UHF,
};

/* --------------------------------------------------------------------- */

static struct CHANLIST pal_newzealand[] = {
    { "1", 	  45250 }, 
    { "2",	  55250 }, 
    { "3",	  62250 },
    { "4",	 175250 },
    { "5",	 182250 },
    { "5A",	 138250 },
    { "6",	 189250 },
    { "7",	 196250 },
    { "8",	 203250 },
    { "9",	 210250 },
    { "10",	 217250 },
};

/* --------------------------------------------------------------------- */

struct CHANLISTS chanlists[NUM_CHANNEL_LISTS] = {
    { "ntsc-bcast",      ntsc_bcast,      CHAN_COUNT(ntsc_bcast)      },
    { "ntsc-cable",      ntsc_cable,      CHAN_COUNT(ntsc_cable)      },
    { "ntsc-cable-hrc",  ntsc_hrc,        CHAN_COUNT(ntsc_hrc)        },
    { "ntsc-bcast-jp",   ntsc_bcast_jp,   CHAN_COUNT(ntsc_bcast_jp)   },
    { "ntsc-cable-jp",   ntsc_cable_jp,   CHAN_COUNT(ntsc_cable_jp)   },
    { "pal-europe",      pal_europe,      CHAN_COUNT(pal_europe)      },
    { "pal-europe-east", pal_europe_east, CHAN_COUNT(pal_europe_east) },
    { "pal-italy",	 pal_italy,       CHAN_COUNT(pal_italy)       },
    { "pal-newzealand",  pal_newzealand,  CHAN_COUNT(pal_newzealand)  },
    { "pal-australia",   pal_australia,   CHAN_COUNT(pal_australia)   },
    { "pal-ireland",     pal_ireland,     CHAN_COUNT(pal_ireland)     },
};

/* Routine to calculate the frequency for a channel */

int getFrequency(int region, int index)
{
 if ((region>=0) && (region<NUM_CHANNEL_LISTS)) {
  if ((index>=0) && (index<chanlists[region].count)) {  
   VSFileSystem::vs_fprintf(stderr,"Region %s : Channel %s\n",chanlists[region].name,chanlists[region].list[index].name);
   return(chanlists[region].list[index].freq);
  }
 }  
 return(0);
}

/* Routine to get the time as float */
double timeGet(void)
{
  struct timeval tv;
  double curtime;
  
  /* get wallclock time */
  gettimeofday(&tv, NULL);
  curtime=(double)tv.tv_sec+1.0e-6*(double)tv.tv_usec;
  return (curtime); 
}

/* Call for each frame to get FPS calculation */
void calc_fps(struct fgdevice *fg)
{
 double curtime;
 
 /* Check interval */
 if (fg->fps_update_interval<1) {
  fg->fps=0.0;
 } else {
  if ( (fg->framecount<0) || (fg->framecount>=fg->fps_update_interval)) {
   /* Initialize counter */
   fg->framecount=0;
   fg->lasttime=timeGet();
   fg->fps=0.0;
  } else {
   fg->framecount++;
   if (fg->framecount==fg->fps_update_interval) {
    curtime=timeGet();
    fg->fps=(double)fg->framecount/(curtime-fg->lasttime);
    fg->lasttime=curtime;
    fg->framecount=0;
   }
  }
 }  
}

/* Set the interval (in frames) after which the fps counter is updated */
void fg_set_fps_interval(struct fgdevice *fg, int interval)
{
 fg->fps_update_interval=interval;
 fg->framecount=-1;
}

/* Return fps */
double fg_get_fps(struct fgdevice *fg)
{
 return(fg->fps);
}
 
/* Prints device specific information to stderr */
int fg_print_info(struct fgdevice *fg)
{
 struct video_capability video_caps;
 struct video_channel video_chnl;
 struct video_audio video_aud;
 char * BooleanText[2];
 BooleanText[0] = new char[4];
 BooleanText[1] = new char[4];
 memcpy( BooleanText[0], "YES\0", 4);
 memcpy( BooleanText[1], "NO \0", 4);
 //{"YES\0","NO \0"};
 int max_tuner;
 int i;
 
 if (ioctl (fg->video_dev, VIDIOCGCAP, &video_caps) == -1) {
  perror ("ioctl (VIDIOCGCAP)");
  return(-1);
 } else {
  
  /* List capabilities */
  
  VSFileSystem::vs_fprintf (stderr,"Device Info: ");
  VSFileSystem::vs_fprintf (stderr,"%s\n",video_caps.name);

  VSFileSystem::vs_fprintf (stderr," Can capture ... : %s  ",BooleanText[((video_caps.type & VID_TYPE_CAPTURE) == 0)]);
  VSFileSystem::vs_fprintf (stderr," Can clip ...... : %s  ",BooleanText[((video_caps.type & VID_TYPE_CLIPPING) == 0)]);
  VSFileSystem::vs_fprintf (stderr," Channels ...... : %i\n",video_caps.channels);

  VSFileSystem::vs_fprintf (stderr," Has tuner ..... : %s  ",BooleanText[((video_caps.type & VID_TYPE_TUNER) == 0)]);
  VSFileSystem::vs_fprintf (stderr," Ovl overwrites  : %s  ",BooleanText[((video_caps.type & VID_TYPE_FRAMERAM) == 0)]);
  VSFileSystem::vs_fprintf (stderr," Audio devices . : %i\n",video_caps.audios);

  VSFileSystem::vs_fprintf (stderr," Has teletext .. : %s  ",BooleanText[((video_caps.type & VID_TYPE_TELETEXT) == 0)]);
  VSFileSystem::vs_fprintf (stderr," Can scale ..... : %s  ",BooleanText[((video_caps.type & VID_TYPE_SCALES) == 0)]);
  VSFileSystem::vs_fprintf (stderr," Width min-max . : %i-%i\n",video_caps.minwidth,video_caps.maxwidth);

  VSFileSystem::vs_fprintf (stderr," Can overlay ... : %s  ",BooleanText[((video_caps.type & VID_TYPE_OVERLAY) == 0)]);
  VSFileSystem::vs_fprintf (stderr," Monochrome .... : %s  ",BooleanText[((video_caps.type & VID_TYPE_MONOCHROME) == 0)]);
  VSFileSystem::vs_fprintf (stderr," Height min-max  : %i-%i\n",video_caps.minheight,video_caps.maxheight);
  
  VSFileSystem::vs_fprintf (stderr," Can chromakey . : %s  ",BooleanText[((video_caps.type & VID_TYPE_CHROMAKEY) == 0)]);
  VSFileSystem::vs_fprintf (stderr," Can subcapture  : %s\n",BooleanText[((video_caps.type & VID_TYPE_SUBCAPTURE) == 0)]);
  
  /* List input channels */
  
  max_tuner=0;
  for (i=0; i<video_caps.channels; i++) {
   video_chnl.channel = i;
   if (ioctl (fg->video_dev, VIDIOCGCHAN, &video_chnl) == -1) {
    perror ("ioctl (VIDIOCGCHAN)");
    return(-1);
   } else {
    VSFileSystem::vs_fprintf (stderr," Channel %i: %s ",i,video_chnl.name);
    if ((video_chnl.type & VIDEO_TYPE_TV)==0) {
     VSFileSystem::vs_fprintf (stderr,"(camera input)\n");
    } else {
     VSFileSystem::vs_fprintf (stderr,"(TV input)\n");
    } 
    VSFileSystem::vs_fprintf (stderr,"  Tuners : %i  ",video_chnl.tuners);
    if (video_chnl.tuners>max_tuner) max_tuner=video_chnl.tuners; 
    VSFileSystem::vs_fprintf (stderr,"  Has audio : %s\n",BooleanText[((video_chnl.flags & VIDEO_VC_AUDIO) == 0)]);
   }
  }
  
  /* Audio channels */
  for (i=0; i<video_caps.audios; i++) {
   video_aud.audio=i;
   if (ioctl (fg->video_dev, VIDIOCGAUDIO, &video_aud) == -1) {
    perror ("ioctl (VIDIOCGAUDIO)");
    return(-1);
   } else {
    VSFileSystem::vs_fprintf (stderr," Audio %i: %s\n",i,video_aud.name);
    VSFileSystem::vs_fprintf (stderr,"  Controllable: ");
    if ((video_aud.flags & VIDEO_AUDIO_MUTABLE) != 0) VSFileSystem::vs_fprintf (stderr,"Muting ");
    if ((video_aud.flags & VIDEO_AUDIO_VOLUME) != 0) VSFileSystem::vs_fprintf (stderr,"Volume ");
    if ((video_aud.flags & VIDEO_AUDIO_BASS) != 0) VSFileSystem::vs_fprintf (stderr,"Bass ");
    if ((video_aud.flags & VIDEO_AUDIO_TREBLE) != 0) VSFileSystem::vs_fprintf (stderr,"Treble ");
    VSFileSystem::vs_fprintf (stderr,"\n");
   } 
  }

  /* Retrieve sizes and offsets */
  if (ioctl (fg->video_dev, VIDIOCGMBUF, &fg->vid_mbuf) == -1) {
   perror ("ioctl (VIDIOCGMBUF)");
   return(-1);
  }
  /* Print memory info */
  VSFileSystem::vs_fprintf (stderr,"Memory Map of %i frames: %i bytes\n",fg->vid_mbuf.frames,fg->vid_mbuf.size);
  for (i=0; i<fg->vid_mbuf.frames; i++) {
   VSFileSystem::vs_fprintf (stderr," Offset of frame %i: %i\n",i,fg->vid_mbuf.offsets[i]);
  }
 }
 
 delete BooleanText[1];
 delete BooleanText[0];

 return(0);
}

/* Read card settings */
int fg_get_setting(struct fgdevice *fg, int which_setting)
{
 struct video_picture video_pict;
 
 if (ioctl (fg->video_dev, VIDIOCGPICT, &video_pict) == -1) {
  perror ("ioctl (VIDIOCGPICT)");
  return(-1);
 } else {
  switch (which_setting) {
   case SETTING_BRIGHTNESS:
    return(video_pict.brightness);
    break;
   case SETTING_HUE:
    return(video_pict.hue);
    break;
   case SETTING_COLOUR:
    return(video_pict.colour);
    break;
   case SETTING_CONTRAST:
    return(video_pict.contrast);
    break;
  } 
 }
 return(-1);
}

/* Adjust card settings */
int fg_set_setting(struct fgdevice *fg, int which_setting, int value)
{
 struct video_picture video_pict;
 
 if (ioctl (fg->video_dev, VIDIOCGPICT, &video_pict) == -1) {
  perror ("ioctl (VIDIOCGPICT)");
  return(-1);
 } else {
  switch (which_setting) {
   case SETTING_BRIGHTNESS:
    video_pict.brightness=value;
    break;
   case SETTING_HUE:
    video_pict.hue=value;
    break;
   case SETTING_COLOUR:
    video_pict.colour=value;
    break;
   case SETTING_CONTRAST:
    video_pict.contrast=value;
    break;
  } 
  if (ioctl (fg->video_dev, VIDIOCSPICT, &video_pict) == -1) {
   perror ("ioctl (VIDIOCSPICT)");
   return(-1);
  } 
 }
 return(0);
}

/* Set the input channel and videomode on card */
int fg_set_channel(struct fgdevice *fg, int channel, int videomode)
{
 struct video_capability video_cap;
 struct video_channel video_chan;
 struct video_tuner tuner;
  
 /* Query for number of channels */
 if (ioctl (fg->video_dev, VIDIOCGCAP, &video_cap) == -1) {
  perror ("ioctl (VIDIOCGCAP)");
  return(-1);
 } 

 /* Clip channel to allowed range */
 if (channel<0) {
  channel=0;
 } else {
  video_cap.channels--;
  if (channel>video_cap.channels) channel=video_cap.channels;
 }

 /* Set channel number */
 video_chan.channel=0; 
 if (ioctl (fg->video_dev, VIDIOCGCHAN, &video_chan) == -1) {
  perror ("ioctl (VIDIOCGCHAN)");
  return(-1);
 } else {
  video_chan.channel=channel; 
  if (ioctl (fg->video_dev, VIDIOCSCHAN, &video_chan) == -1) {
   perror ("ioctl (VIDIOCSCHAN)");
   VSFileSystem::vs_fprintf (stderr,"Channel was %i\n",channel);
   return(-1);
  } 
 }

 /* Set videomode */
 tuner.tuner=0;
 if (channel==CHANNEL_TUNER) {
  if (ioctl (fg->video_dev, VIDIOCGTUNER, &tuner) == -1) {
   perror ("ioctl (VIDIOCGTUNER)");
   return(-1);
  }
 }
 tuner.mode=videomode;
 if (ioctl (fg->video_dev, VIDIOCSTUNER, &tuner) == -1) {
  perror ("ioctl (VIDIOCSTUNER)");
  return(-1);
 }

 return(0);
}

/* Set the tuner frequency */
int fg_set_frequency(struct fgdevice *fg, int region, int index)
{
 int frequency;
 unsigned long adjusted_frequency;
 
 frequency=getFrequency (region, index);
 if (frequency>0) {
  adjusted_frequency=frequency*16/1000;
  if (ioctl (fg->video_dev, VIDIOCSFREQ, &adjusted_frequency) == -1) {
   perror ("ioctl (VIDIOCSFREQ)");
   return(-1);
  }
 }
 
 return(0); 
}

/* Self running thread that grabbs all images */
void* grab_images_thread(struct fgdevice *fg)
{
 int in_loop;
 //struct fgdevice * fg = (fgdevice *) ptr;
 
 /* Loop ... grabbing images */
 in_loop=1;
 while (in_loop) {

  /* Lock buffers and state */
  pthread_mutex_lock(&fg->buffer_mutex);

  /* Advance grab-frame number */
  fg->current_grab_number=((fg->current_grab_number + 1) % 2);

  /* Unlock buffers and state */
  pthread_mutex_unlock(&fg->buffer_mutex);

  /* Wait for next image in the sequence to complete grabbing */
  if (ioctl (fg->video_dev, VIDIOCSYNC, &fg->vid_mmap[fg->current_grab_number]) == -1) {
   perror ("VIDIOCSYNC");
   return((void *)&error_exit_status);
  }

  /* Issue new grab command for this buffer */
  if (ioctl (fg->video_dev, VIDIOCMCAPTURE, &fg->vid_mmap[fg->current_grab_number]) == -1) {
   perror ("VIDIOCMCAPTURE");
   return((void *)&error_exit_status);
  }

  /* Lock buffers and state */
  pthread_mutex_lock(&fg->buffer_mutex);

  /* Announce that a new frame is available */
  fg->have_new_frame=1;

  /* Get loop state */
  in_loop=fg->grabbing_active;

  /* Unlock buffers and state */
  pthread_mutex_unlock(&fg->buffer_mutex);

  /* Signal potentially waiting main thread */
  pthread_cond_signal(&fg->buffer_cond);
   
 }

 
 pthread_exit(NULL);
}
typedef void * (*pthread_func_type ) (void *);
/* Initialize grabber and start grabbing-thread */
int fg_start_grab_image (struct fgdevice *fg, int width, int height, int format)
{
 int i;
 int depth;
 
 /* Store variables in structure */
 fg->width=width;
 fg->height=height;
 fg->format=format;
 
 /* Retrieve buffer size and offsets */
 if (ioctl (fg->video_dev, VIDIOCGMBUF, &fg->vid_mbuf) == -1) {
  perror ("ioctl (VIDIOCGMBUF)");
  return(-1);
 }
 
 /* Map grabber memory into user space */
 fg->video_map = (char *) mmap (0, fg->vid_mbuf.size, PROT_READ|PROT_WRITE,MAP_SHARED,fg->video_dev,0);
 if ((unsigned char *)-1 == (unsigned char *)fg->video_map) {
  perror ("mmap()");
  return(-1);
 }

 /* Determine depth from format */
 depth=2;
 switch (format) {
  case VIDEO_PALETTE_GREY:
   depth=1;
   break;
  case VIDEO_PALETTE_RGB565:
  case VIDEO_PALETTE_YUV422P:
  case VIDEO_PALETTE_YUV420P:
   depth=2;
   break;
  case VIDEO_PALETTE_RGB24:
   depth=3;
   break;
  case VIDEO_PALETTE_RGB32:
   depth=4;
   break;
  default:
   VSFileSystem::vs_fprintf (stderr,"Unknown format.\n");
   return(-1);
   break; 
 }

 /* Generate mmap records */
 for (i=0; i<2; i++) {
  fg->vid_mmap[i].format = format;
  fg->vid_mmap[i].frame  = i;
  fg->vid_mmap[i].width  = width;
  fg->vid_mmap[i].height = height;
 }
  
 /* Calculate framebuffer size and allocate memory for user frames */
 fg->image_pixels=width*height;
 fg->image_size=fg->image_pixels*depth;
 if ((fg->current_image=malloc(fg->image_size))==NULL) {
  perror ("malloc()");
  return(-1);
 }
   
 /* Initiate capture for frames */
 for (i=0; i<2; i++) {
  /* Sync old images in the sequence to complete grabbing */
  ioctl (fg->video_dev, VIDIOCSYNC, &fg->vid_mmap[i]);
  /* Start capture */
  if (ioctl (fg->video_dev, VIDIOCMCAPTURE, &fg->vid_mmap[i]) == -1) {
   perror ("VIDIOCMCAPTURE");
   return(-1);
  }
 }
 
 /* Reset grab counter variables */
 fg->current_grab_number=0;
 fg->totalframecount=0;
 fg->have_new_frame=0;
  
 /* Initialize mutex */
 if (pthread_mutex_init(&fg->buffer_mutex, NULL)==-1) {
  perror("pthread_mutex_init()");
  return(-1);
 } 

 /* Initialize conditional */
 if (pthread_cond_init(&fg->buffer_cond, NULL)==-1) {
  perror("pthread_cond_init()");
  return(-1);
 } 

 /* Set thread loop flag */
 fg->grabbing_active=1;

 /* Start grabbing thread */
 if (pthread_create( &fg->grab_thread,
						 NULL,
						 (VoidVoidFuncType)grab_images_thread,
						 (void *)fg)
				 	==-1)
 {
  perror ("pthread_create()");
  return(-1);
 } 

 
 return(0);
}

/* Stop grabbing thread */
int fg_stop_grab_image(struct fgdevice *fg)
{
 /* Set flag for thread to finish */
 fg->grabbing_active=0;

 /* Wait for grabbing thread to exit */
 if (pthread_join(fg->grab_thread,NULL)==-1) {
  perror("pthread_join()");
  return(-1);
 }

 /* Free image buffers */
 if (fg->current_image) free(fg->current_image);
 fg->current_image=NULL;

 /* Unmap memory */
 if (munmap (fg->video_map, fg->vid_mbuf.size) == -1) {
  perror ("munmap()");
  return(-1);
 }

 return(0);
}

/* Get last image, lock to be processed */ 
void * fg_get_next_image(struct fgdevice *fg)
{
 /* Check if we are grabbing */
 if (fg->grabbing_active) {

  /* Increase counter */
  fg->totalframecount++;
   
  /* Lock buffers and state */
  pthread_mutex_lock(&fg->buffer_mutex);
  
  /* Check if a new frame is available */
  if (!fg->have_new_frame) {
   /* Wait for signal - unlocking mutex in the process */
   pthread_cond_wait(&fg->buffer_cond,&fg->buffer_mutex);
  }

  /* Copy frame from other-than-grab framebuffer */
  memcpy (fg->current_image,&fg->video_map[fg->vid_mbuf.offsets[(fg->current_grab_number+1) % 2]],fg->image_size);

  /* Mark frame as used */
  fg->have_new_frame=0;
  
  /* Unlock buffers and state */
  pthread_mutex_unlock(&fg->buffer_mutex);
 
  /* FPS calculation */
  calc_fps(fg);
  
 } else {
  /* No grabbing */
  fg->current_image=NULL;
 }
  
 /* Return pointer */
 return (fg->current_image);
}

/* Open framegrabber device */
int fg_open_device (struct fgdevice *fg, const char *devicename)
{                                                                 
 int i,r,g,b;
 unsigned short *curypos;
 unsigned char *currgbpos;
 unsigned short us;

 /* Open the video4linux device */
 fg->video_dev = open (devicename, O_RDWR);
 if (fg->video_dev == -1) {
  perror("open()");
  return (-1);
 }
 
 /* Reset variables */
 fg->grabbing_active=0;
 fg->width=-1;
 fg->height=-1;
 fg->format=-1;
 fg->input=-1;
 fg->image_size=-1;
 fg->image_pixels=-1;
 fg->fps_update_interval=-1;
 
 /* Initialize color space conversion tables */

 /* Y 8bit to RGB 16bit */
 if ((fg->y8_to_rgb565=(unsigned short *)malloc(256*sizeof(short)))==NULL) {
  perror("malloc");
 }
 curypos=fg->y8_to_rgb565;
 for (i=0; i<256; i++) {  
  *curypos=((i >> 3) << 11 ) | ((i >> 2) << 5) | (i >> 3);
  curypos++; 
 } 

 /* RGB 16 bit to Y 8bit */
 if ( (fg->rgb565_to_y8=(unsigned char *)malloc(64*1024*sizeof(char)))==NULL) {
  perror("malloc");
 }
 currgbpos=fg->rgb565_to_y8;  
 for (i=0; i<(64*1024); i++) {
  us=(unsigned short)i;
  r=(us >> 11);
  g=((us >> 5) & 63);
  b=(us & 31);
  *currgbpos=(r*2450+g*2404+b*934)/1024;          
  currgbpos++;
 }

 return(0);
}

/* Close framegrabber device */
int fg_close_device (struct fgdevice *fg)
{
 /* Free allocated memory */
 if (fg->current_image) free(fg->current_image);
 if (fg->y8_to_rgb565) free(fg->y8_to_rgb565);
 if (fg->rgb565_to_y8) free(fg->rgb565_to_y8);

 /* Close video4linux device */
 return(close(fg->video_dev));
}
#endif

#endif /* NETCOMM_NOWEBCAM */

