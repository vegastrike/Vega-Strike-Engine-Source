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
typedef unsigned char * (textureTransform) (int &bpp, int &color_type, unsigned long &width, unsigned long &height, unsigned char ** row_pointers);
textureTransform heightmapTransform;
textureTransform terrainTransform;
textureTransform texTransform;

void png_write (const char * myfile, unsigned char * data, unsigned int width, unsigned int height, bool alpha, char bpp);

using namespace VSFileSystem;

enum	VSImageType { PngImage, BmpImage, JpegImage, Unrecognized };

typedef struct {
    char *Buffer;
    int  Pos;
} TPngFileBuffer;

class VSImage
{
		VSFile *			img_file;
		VSFile *			img_file2;
		textureTransform *	tt;
		VSImageType			img_type;

		int					img_depth;
		int					img_color_type;
		bool				img_alpha;
		bool				strip_16;
                bool flip;
	public:
		///if we statically allocate it, then gl_texture will kill it when destructor is called...and if we delete this texture we be messed
		unsigned char *		palette;
		///The bitmode of this texture
		enum {_8BIT, _24BIT, _24BITRGBA} mode;
		///the dimensions of the texture
		unsigned long		sizeX;
		unsigned long		sizeY;

	private:
		void	Init();
		void	Init( VSFile * f, textureTransform * t=NULL, bool strip=false, VSFile * f2 = NULL);

		VSError	CheckPNGSignature( VSFile * file);
		VSError	CheckJPEGSignature( VSFile * file);
		VSError	CheckBMPSignature( VSFile * file);
		void	CheckFormat( VSFile * file);

		unsigned char *	ReadPNG();
		unsigned char *	ReadJPEG();
		unsigned char *	ReadBMP();

		VSError	WritePNG( unsigned char * data);
		VSError	WriteJPEG(unsigned char * data);
		VSError	WriteBMP( unsigned char * data);

		void	AllocatePalette();

	public:
		// Defined for gcc which pads the size of structs
		//const static int SIZEOF_BITMAPFILEHEADER;
        LOCALCONST_DECL(int,SIZEOF_BITMAPFILEHEADER,sizeof(WORD)+sizeof(DWORD)+sizeof(WORD)+sizeof(WORD)+sizeof(DWORD))
		// Defined for gcc which pads the size of structs
        LOCALCONST_DECL(int,SIZEOF_BITMAPINFOHEADER,sizeof(DWORD)+sizeof(LONG)+sizeof(LONG)+2*sizeof(WORD)+2*sizeof(DWORD)+2*sizeof(LONG)+2*sizeof(DWORD))
		//const static int SIZEOF_BITMAPINFOHEADER;
		// Defined for gcc which pads size of structs (not entirely necessary)
		//const static int SIZEOF_RGBQUAD;
        LOCALCONST_DECL(int,SIZEOF_RGBQUAD,sizeof(BYTE)*4)

		VSImage::VSImage();
		// f2 is needed for bmp loading
		VSImage::VSImage( VSFile * f, textureTransform * t=NULL, bool strip=false, VSFile * f2 = NULL);
		VSImage::~VSImage();

		// f2 is needed for bmp loading
		unsigned char*	ReadImage( VSFile * f, textureTransform * t=NULL, bool strip=false, VSFile * f2 = NULL);
		VSError	WriteImage( char * filename, unsigned char * data, VSImageType type, unsigned int width, unsigned int height,
							bool alpha=1, char bpp=16, VSFileType ft=::VSFileSystem::UnknownFile, bool flip=false);
		VSError	WriteImage( VSFile * pf, unsigned char * data, VSImageType type, unsigned int width, unsigned int height,
							bool alpha=1, char bpp=16, bool flip=false);

		int		Depth() { return this->img_depth; }
		int		Format() { return this->img_color_type; }
};

#endif

