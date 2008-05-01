#ifndef __VSIMAGE__H
#define __VSIMAGE__H

#include "vsfilesystem.h"

extern int PNG_HAS_PALETTE;
extern int PNG_HAS_COLOR;
extern int PNG_HAS_ALPHA;

#ifndef WIN32
typedef unsigned int DWORD;
typedef int  LONG;
typedef unsigned short WORD;
typedef unsigned char BYTE;
/**
 * Windows Bitmap format.  Caution about mips systems which cannot handle
 * misaligned structs
 * Caution about big endian systems (use endianness.h to read in things)
 */
typedef struct {
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

/**
 * File header of a bitmap. Won't work on mips architecture with 
 * misaligned structs
 */
typedef struct {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;
/**
 *  The color data of a bitmap
 */
typedef struct {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;
#else
#include <windows.h>
#include <wingdi.h>
#endif

/*
 * Standard DDS formats. 
 */

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT                                         
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0                                
#endif                                                                          
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                                        
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1                                
#endif                                                                          
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                                        
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2                                
#endif                                                                          
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                                        
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3                                
#endif                                                                          
#ifndef GL_TEXTURE_COMPRESSION_HINT_ARB                                         
#define GL_TEXTURE_COMPRESSION_HINT_ARB   0x84EF                                
#endif                                                                          
 

/*
 * DDS header structs
 * Some DDS files may not fill in all data members. 
 */
typedef struct
{
	char fourcc[4];
	int bpp;
} pxlformat;
typedef struct
{
	int size;
	int flags;
	int height;
	int width;
	int linsize;
	int depth;
	int nmips;
	pxlformat pixelFormat;
} ddsHeader;
// End DDS header	


typedef struct {
    char *Buffer;
    int  Pos;
} TPngFileBuffer;

typedef unsigned char * (textureTransform) (int &bpp, int &color_type, unsigned long &width, unsigned long &height, unsigned char ** row_pointers);
textureTransform heightmapTransform;
textureTransform terrainTransform;
textureTransform texTransform;

void png_write (const char * myfile, unsigned char * data, unsigned int width, unsigned int height, bool alpha, char bpp);


enum VSImageType { PngImage, BmpImage, JpegImage, DdsImage, Unrecognized };


/*
 * VSImage is a container and low level api to texture input data. 
 * Image loading is initiated with a call to VSImage->ReadImage().  
 * ReadImage then calls CheckFormat to see if the image file supplied is a valid format.
 * img_type is set to the detected format.
 * Depending on the format found, it calls the correct ReadType function.  
 * The ReadType function reads in data with img_file->Read and converts the data into 
 * a format that GL understands, either uncompressed image data or S3TC compressed data.
 * ReadType will also set img_depth,sizeX,sizeY,img_alpha,type,mode.
 * That data is then returned to ReadImage which then returns that data to the caller. 
 */
class VSImage
{
	private:
		VSFileSystem::VSFile*		img_file;
		VSFileSystem::VSFile*		img_file2;
		textureTransform*		tt;
		VSImageType			img_type;

		int				img_depth;
		int				img_color_type;
		bool				img_alpha;
		bool				strip_16;
                bool				flip;

		void	Init();
		void	Init( VSFileSystem::VSFile * f, textureTransform * t=NULL, bool strip=false, 
				VSFileSystem::VSFile * f2 = NULL);

		VSFileSystem::VSError	CheckPNGSignature( VSFileSystem::VSFile * file);
		VSFileSystem::VSError	CheckJPEGSignature( VSFileSystem::VSFile * file);
		VSFileSystem::VSError	CheckBMPSignature( VSFileSystem::VSFile * file);
		VSFileSystem::VSError CheckDDSSignature(VSFileSystem::VSFile * file);
		
		/*
		 * Calls above Check methods to determine if mime type matches supported format. 
		 * Sets img_type to correct type.
		 */
		void	CheckFormat( VSFileSystem::VSFile * file);

		/*
		 * The following are format specific read functions called by ReadImage().  
		 * They are responsible for returning usable image data, either 
		 * an uncompressed data of supported depth/type or DDS 
		 */
		unsigned char *	ReadPNG();
		unsigned char *	ReadJPEG();
		unsigned char *	ReadBMP();
		unsigned char * ReadDDS();

		VSFileSystem::VSError	WritePNG( unsigned char * data);
		VSFileSystem::VSError	WriteJPEG( unsigned char * data);
		VSFileSystem::VSError	WriteBMP( unsigned char * data);

		void	AllocatePalette();

	public:
		VSImage();
		// f2 is needed for bmp loading
		VSImage( VSFileSystem::VSFile * f, textureTransform * t=NULL, bool strip=false, VSFileSystem::VSFile * f2 = NULL);
		~VSImage();

		//if we statically allocate it, then gl_texture will kill it when destructor is called...and if we delete this texture we be messed
		unsigned char *		palette;

		/*
		 * Position 3 and greater of VSImageMode are helper modes to differentiate the correct modes of DDS and PNG files.
		 * DDS files are assumed to be 24 or 32 bit.  
		 */
		enum VSImageMode {_8BIT, _24BIT, _24BITRGBA,_DXT1,_DXT1RGBA,_DXT3,_DXT5} mode;
		
		///the dimensions of the texture
		unsigned long		sizeX;
		unsigned long		sizeY;

		// Defined for gcc which pads the size of structs
		//const static int SIZEOF_BITMAPFILEHEADER;
		LOCALCONST_DECL(int,SIZEOF_BITMAPFILEHEADER,sizeof(WORD)+sizeof(DWORD)+sizeof(WORD)+sizeof(WORD)+sizeof(DWORD))
		// Defined for gcc which pads the size of structs
		LOCALCONST_DECL(int,SIZEOF_BITMAPINFOHEADER,sizeof(DWORD)+sizeof(LONG)+sizeof(LONG)+2*sizeof(WORD)+2*sizeof(DWORD)+2*sizeof(LONG)+2*sizeof(DWORD))
		//const static int SIZEOF_BITMAPINFOHEADER;
		// Defined for gcc which pads size of structs (not entirely necessary)
		//const static int SIZEOF_RGBQUAD;
		LOCALCONST_DECL(int,SIZEOF_RGBQUAD,sizeof(BYTE)*4)

		// f2 is needed for bmp loading
		unsigned char*	ReadImage( VSFileSystem::VSFile * f, textureTransform * t=NULL, bool strip=false, VSFileSystem::VSFile * f2 = NULL);
		
		VSFileSystem::VSError	WriteImage( char * filename, unsigned char * data, VSImageType type, unsigned int width, unsigned int height,
								bool alpha=1, char bpp=16, VSFileSystem::VSFileType ft=VSFileSystem::UnknownFile, bool flip=false);
		VSFileSystem::VSError	WriteImage( VSFileSystem::VSFile * pf, unsigned char * data, VSImageType type, unsigned int width, unsigned int height,
								bool alpha=1, char bpp=16, bool flip=false);

		const int		Depth() const { return this->img_depth; }
		const int		Format() const { return this->img_color_type; }
};

#endif

