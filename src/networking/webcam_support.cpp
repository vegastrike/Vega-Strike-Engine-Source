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
AM_MEDIA_TYPE g_StillMediaType;
STDMETHODIMP SampleGrabberCallback::BufferCB( double Time, BYTE *pBuffer, long BufferLen )
	{
		// Check if it is time to send capture to communicating client(s)
		if( ws->isReady())
		{
			if ((g_StillMediaType.majortype != MEDIATYPE_Video) ||
				(g_StillMediaType.formattype != FORMAT_VideoInfo) ||
				(g_StillMediaType.cbFormat < sizeof(VIDEOINFOHEADER)) ||
				(g_StillMediaType.pbFormat == NULL))
			{
				return VFW_E_INVALIDMEDIATYPE;
			}
			long cbBitmapInfoSize = g_StillMediaType.cbFormat - SIZE_PREHEADER;
			VIDEOINFOHEADER *pVideoHeader =
			   (VIDEOINFOHEADER*)g_StillMediaType.pbFormat;

			BITMAPFILEHEADER bfh;
			ZeroMemory(&bfh, sizeof(bfh));
			bfh.bfType = 'MB';  // Little-endian for "MB".
			bfh.bfSize = sizeof( bfh ) + BufferLen + cbBitmapInfoSize;
			bfh.bfOffBits = sizeof( BITMAPFILEHEADER ) + cbBitmapInfoSize;

		    LPBITMAPINFOHEADER lpbi = HEADER( pVideoHeader);

			// The picture is composed like this :
			// - bfh : bitmap file header
			// - HEADER(pVideoHeader) : bitmap info header
			// - pBuffer (length BufferLen) : picture data
			// Copy the actual jpeg in the old buffer before getting the new one
			if( ws->old_buffer)
				delete ws->old_buffer;
			ws->old_size = ws->jpeg_size;
			ws->old_buffer = ws_jpeg_buffer;
			// JpegFromBmp should allocate the needed buffer;
			ws->jpeg_buffer = JpegFromBmp( bfh, lpbi, pBuffer, BufferLen, &ws->jpeg_size, 40, "c:\test.jpg");
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
	// HERE SHOULD ONLY COPY THE IMAGE INTO THE JPEG BUFFER
	WebcamSupport * ws = (WebcamSupport *) refCon;
	if( ws->old_buffer)
		delete ws->old_buffer;
	ws->old_buffer = ws->jpeg_buffer;
	ws->old_size = ws->jpeg_size;
	ws->jpeg_buffer = new char[len];
	memcpy( ws->jpeg_buffer, p, len);
	ws->jpeg_size = len;
	if( ws && ws->isReady())
	{
		// Write p to a file for testing
		string path = datadir+"testcam.jpg";
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
		cerr<<"!!! ERROR : "<<message<<" - code : "<<error<<endl;
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
	this->width = 160;
	this->height = 120;
	this->fps = 5;
	this->grabbing = false;
	this->depth = 16;

	this->last_time = 0;
	// Get the period between 2 captured images
	period = 1000000./(double)this->fps;

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
	this->Init();
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
		DoError( -1, "Failed to initialise COM");

	// Create the filter graph
	IGraphBuilder *pGraph;
	hr = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **)&pGraph );
	if( FAILED( hr ) )
		DoError( -1, "Failed to create filter graph");

	// Create the capture graph
	ICaptureGraphBuilder2 *pCaptureGraph = NULL;
	hr = CoCreateInstance( CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
		IID_ICaptureGraphBuilder2, (void**)&pCaptureGraph );
	if( FAILED( hr ) )
		DoError( -1, "Couldn't create capture graph");

	pCaptureGraph->SetFiltergraph( pGraph );

	if( FAILED( hr ) )
		DoError( -1, "Couldn't set filter graph");

	// Select a capture device
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	IMoniker *pMoniker = NULL;
	IBaseFilter *pCap = NULL;

	hr = CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, reinterpret_cast<void **>( &pDevEnum ) );
	hr = pDevEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pEnum, 0 );
	hr = pEnum->Next( 1, &pMoniker, NULL );
	hr = pMoniker->BindToObject( 0, 0, IID_IBaseFilter, (void **)&pCap );

	// Add input filter to graph
	hr = pGraph->AddFilter( pCap, L"Capture Filter" );

	// Add sample grabber filter to graph
	IBaseFilter *pSampleGrabberF;
	ISampleGrabber *pSampleGrabber;
	hr = CoCreateInstance( CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void **)&pSampleGrabberF );
	hr = pGraph->AddFilter( pSampleGrabberF, L"Sample grabber" );
	pSampleGrabberF->QueryInterface( IID_ISampleGrabber, (void **)&pSampleGrabber );

	// Set some sample grabber variables
	AM_MEDIA_TYPE mt;
	ZeroMemory( &mt, sizeof( AM_MEDIA_TYPE ) );
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	hr = pSampleGrabber->SetMediaType( &mt );
	hr = pSampleGrabber->SetOneShot( false );
	hr = pSampleGrabber->SetBufferSamples( true );
	// Associate the webcamsupport to the callback
	g_SampleCB.ws = this;
	hr = pSampleGrabber->SetCallback( &g_SampleCB, 1 );

	// Add Null renderer
	hr = CoCreateInstance( CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void **)&pNull );
	hr = pGraph->AddFilter( pNull, L"Null renderer" );

	hr = pSampleGrabber->GetConnectedMediaType(&g_StillMediaType);
	pSampleGrabber->Release();

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
	iErr = NewGWorld(&video -> sg_world, this->depth, &r, NULL, NULL, 0);
	DoError(iErr, "NewGWorld failed - Trying giving me more memory or use a sensible video size");
	LockPixels( GetGWorldPixMap( video -> sg_world));

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
	component_error = SGSetDataOutput( video->sg_component, NULL, seqGrabToMemory | seqGrabDontMakeMovie);
	DoError(component_error, "SGSetDataOutput failed");

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
	component_error = SGSetVideoCompressorType(video -> sg_channel, 'jpeg');
	DoError(component_error, "SGSetVideoCompressorType failed");

	// tell we won't render on screen
	component_error = SGSetUseScreenBuffer(video -> sg_channel, false);
	DoError(component_error, "SGSetUseScreenBuffer failed");

	// specify the framerate (not necessary now)
	component_error = SGSetFrameRate(video -> sg_channel, this->fps);
	DoError(component_error, "SGSetUseScreenBuffer failed");

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
	if( grabbing && (last_time-curtime>period) )
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
#endif
#endif
#ifdef __APPLE__
	ComponentResult		component_error = noErr;
	OSErr	iErr;

	// Set the callback
	iErr = SGSetDataProc( video->sg_component, NewSGDataUPP( processFrame), (long)this->video);
	// Prepare for recording
	component_error = SGPrepare( video->sg_component, false, true);
	DoError( component_error, "SGPrepare failed");

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
#ifdef linux
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
	// Should copy the image data from memory buffer ... where ??
#endif
	return 0;
}

/**************************************************************************************************/
/**** End capture mode                                                                         ****/
/**************************************************************************************************/
void	WebcamSupport::EndCapture()
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
#endif
#endif
#ifdef __APPLE__
	ComponentResult		component_error = noErr;
	component_error = SGStop( video->sg_component);
	DoError( component_error, "stopping video capture");
	CloseComponent( video->sg_component);
#endif
}

/**************************************************************************************************/
/**** Get the latest grabbed image                                                             ****/
/**************************************************************************************************/
std::string	WebcamSupport::CaptureImage()
{
if( grabbing)
{
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
	return string( jpeg_buffer);
#endif
#endif
#ifdef __APPLE__
	// Just return the jpeg_bufer filled by the callback function
	if( !jpeg_buffer)
	{
		cerr<<"!!! JPEG BUFFER EMPTY !!!"<<endl;
		return string("");
	}
	cerr<<"--== JPEG BUFFER OK ==--"<<endl;
	return string( jpeg_buffer);

	/* BAD WAY TO TRY CAPTURE : ACCESSING DIRECTLY GWORLD BUFFER
	OSErr iErr;
	ComponentResult		component_error = noErr;
	// Update the offscreen GWorld
	component_error = SGIdle(video -> sg_channel);
	if (component_error)
		DoError(component_error, "SGIdle failed");
	iErr = SGUpdate( video->sg_component, 0);
	if( iErr)
		DoError( iErr, "updating sg_component failed.");
	// Get the pixmap associated with the GWorld
	PixMapHandle pix = GetGWorldPixMap( video->sg_world);
	if( pix==NULL || pix==nil)
		DoError( -1, "PixMap is NULL");
	// Get the base address of the pixel array
	Ptr pixmap_base = GetPixBaseAddr( pix);
	if( pixmap_base==NULL || pixmap_base==nil)
		DoError( -1, "PixMapBaseAddress is NULL");
	// Writes the image to a test jpeg file
	Rect r = (**pix).bounds;
	cerr<<"Captured image : "<<r.right<<"x"<<r.bottom<<endl;

	// Get the size of the compressed file
	long	maxCompressionSize;
	iErr = GetMaxCompressionSize( pix, &r, 0, codecNormalQuality, kJPEGCodecType, (CodecComponent) anyCodec, &maxCompressionSize);
		DoError( iErr, "GetMaxCompressionSize failed.");
	// Compress the image and get the its description
	ImageDescriptionHandle desc = (ImageDescriptionHandle) NewHandle(0);
	Handle	jpeg_handle = NewHandle(maxCompressionSize);
	Ptr jpeg_data = *jpeg_handle;
	MoveHHi( jpeg_handle);
	HLock( jpeg_handle);
	iErr = CompressImage( GetGWorldPixMap( video->sg_world), &r, codecNormalQuality, kJPEGCodecType, desc, jpeg_data);
	if (iErr!=noErr)
		DoError( iErr, "CompressImage failed.");
	// Associate the current image capture buffer with what we just compressed into a jpeg buffer
	this->jpeg_buffer = (char *) jpeg_data;
	this->jpeg_size = maxCompressionSize;

	// THE FOLLOWING PART IS FOR TESTING PURPOSE ONLY
	// Write the jpeg_data into a file
	int jpeg_length = maxCompressionSize;
	cerr<<"MaxCompressionSize = "<<maxCompressionSize<<endl;
	string path = datadir+"testcam.jpg";
	FILE * fp;
	fp = fopen( path.c_str(), "w");
	if( !fp)
		DoError( -1, "opening jpeg file failed");
	if( fwrite( jpeg_data, 1, maxCompressionSize, fp)!=maxCompressionSize)
		DoError( -1, "writing jpeg description to file");
	fclose( fp);
	*/
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
		DoError( -1, "Bad parameters");

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
		DoError( -1, "opening of jpeg file failed.");

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
		DoError( -1, "DibToSamps failed");

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
		  DoError( -1, "Invalid bitmap bit count");
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

