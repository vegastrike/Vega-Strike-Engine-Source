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
#include <iostream>
#include "webcam_support.h"
#include "lin_time.h"

using std::cerr;
using std::endl;
using std::hex;

#ifdef __APPLE__
//#include <Quickdraw.h>
//#include <ApplicationServices/ApplicationServices.h>
//#include <Carbon.h>
OSErr	ExhaustiveError(void)
{
	OSErr iErr;
	iErr = ResError();
	if( !iErr)
		iErr = MemError();
	else
		iErr = -1;
	return iErr;
}
void	DoError( long error, char * message)
{
	if( error)
	{
		cerr<<"!!! ERROR : "<<message<<" - code : "<<hex<<error<<endl;
		exit(1);
	}
}
#endif

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

#ifdef __APPLE__
	OSErr				iErr = noErr;
	GDHandle			saveDevice;
	CGrafPtr			savePort;
	long				qtVers;

	GetGWorld( &savePort, &saveDevice);
	
	iErr = Gestalt( gestaltQuickTime, &qtVers);
	if( iErr)
	{
		cerr<<"!!! ERROR initialising Quicktime... please check it is installed !!!"<<endl;
		exit(1);
	}
	iErr = EnterMovies();
	if( iErr)
	{
		cerr<<"!!! ERROR initialising Quicktime... please check it is installed !!!"<<endl;
		exit(1);
	}
	SetGWorld( savePort, saveDevice);

	this->gQuicktimeInitialized = true;
	this->video = NULL;
#endif
}

WebcamSupport::WebcamSupport( int f, int w, int h)
{
	this->width = w;
	this->height = h;
	this->fps = f;
	this->grabbing = false;
	this->depth = 16;

	this->last_time = 0;
	// Get the period between 2 captured images
	period = 1000000./(double)this->fps;

#ifdef __APPLE__
	OSErr				iErr = noErr;
	GDHandle			saveDevice;
	CGrafPtr			savePort;
	long				qtVers;

	GetGWorld( &savePort, &saveDevice);
	
	iErr = Gestalt( gestaltQuickTime, &qtVers);
	if( iErr)
	{
		cerr<<"!!! ERROR initialising Quicktime... please check it is installed !!!"<<endl;
		exit(1);
	}
	iErr = EnterMovies();
	if( iErr)
	{
		cerr<<"!!! ERROR initialising Quicktime... please check it is installed !!!"<<endl;
		exit(1);
	}
	SetGWorld( savePort, saveDevice);

	this->gQuicktimeInitialized = true;
	this->video = NULL;
#endif
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
	if (iErr)
	{
		iErr = ExhaustiveError();
		DoError(iErr, "NewGWorld failed.\rTrying giving me more memory or use a sensible video size");
		exit(1);
	}
	LockPixels( GetGWorldPixMap( video -> sg_world));

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

	//	get a new sequence grabber channel
	component_error = SGNewChannel(video -> sg_component, VideoMediaType, &video -> sg_channel);
	if (component_error)
		{
		DoError(component_error, "SGNewChannel failed");
		goto bail;
		}
	
	//	set channel settings
	/*
	if (settings)
		{
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
	*/
	
	//	set sequence grabber bounds
	//component_error = SGSetChannelBounds(video -> sg_channel, &video -> sg_world -> portRect);
	component_error = SGSetChannelBounds(video -> sg_channel, &r);
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
	//capSetCallbackOnVideoStream(capvideo, CopyImage());
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
	CloseComponent( video->sg_component);
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
	// Open and empty the clipboard
	if ( !OpenClipboard(NULL) )
	{
		cerr<<"!!! ERROR opening windows clipboard !!!"<<endl;
		exit(1);
	}
	if ( !EmptyClipboard() )
	{
		cerr<<"!!! ERROR emptying the clipboard !!!"<<endl;
		exit(1);
	}
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

	return 0;
#endif
#ifdef __APPLE__
	// Get Buffer Info and see...
	ComponentResult		component_error = noErr;
	component_error = SGIdle(video -> sg_channel);
	if (component_error)
		{
		DoError(component_error, "SGIdle failed");
		exit(1);
		}
	Ptr pixmap_base = GetPixBaseAddr( GetGWorldPixMap( video->sg_world));
	// Writes the image to a test jpeg file
	Rect r;
	r.top = r.left = 0;
	r.right = this->width;
	r.bottom = this->height;
	Ptr	jpeg_data;
	ImageDescriptionHandle desc;
	OSErr iErr = CompressImage( GetGWorldPixMap( video->sg_world), &r, codecNormalQuality, kJPEGCodecType, desc, jpeg_data);
	FSSpec spec;
	short fp;
	long nbwritten;
	// Open for writing
	iErr = FSpOpenDF( &spec, fsWrPerm, &fp);
	// Write the jpeg data
	iErr = FSWrite( fp, &nbwritten, jpeg_data);
	// Close the file
	FSClose( fp);
	/*
	int x = GetGWorldPixMap( video->sg_world)->bounds.right;
	int y = GetGWorldPixMap( video->sg_world)->bounds.bottom;
	int rowb = GetGWorldPixMap( video->sg_world)->rowBytes;
	cerr<<"\t\tCaptured "<<x<<"x"<<y<<" size with "<<hex<<rowb<<" row bytes"<<endl;
	*/
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
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
#endif
	return 0;
}

void	WebcamSupport::Shutdown()
{
#ifdef linux
	if (fg_close_device (&fg)!=0)
		exit(-1);
#endif
#ifdef __APPLE__
	delete video;
#endif
}

/***********************************************************************************/
/**** END OF WEBCAMSUPPORT CLASS                                                ****/
/**** OS SPECIFIC STUFF HERE                                                    ****/
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
