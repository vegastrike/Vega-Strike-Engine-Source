#include <png.h> 
#include <stdio.h>
#include <stdlib.h>
#include "../endianness.h"
#include <assert.h>
const int ltwid = 256;
const int lthei = 256;
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
///Defined for gcc which pads the size of structs
const int SIZEOF_BITMAPFILEHEADER=sizeof(WORD)+sizeof(DWORD)+sizeof(WORD)+sizeof(WORD)+sizeof(DWORD);
///Defined for gcc which pads the size of structs
const int SIZEOF_BITMAPINFOHEADER= sizeof(DWORD)+sizeof(LONG)+sizeof(LONG)+2*sizeof(WORD)+2*sizeof(DWORD)+2*sizeof(LONG)+2*sizeof(DWORD);
///defined for gcc which pads size of structs (not entirely necessary)
const int SIZEOF_RGBQUAD=sizeof(BYTE)*4;
///holds all the textures in a huge hash table

static unsigned char *  LoadTex(char * FileName, int &sizeX, int &sizeY){

  unsigned char ctemp;
  unsigned char * data;
  FILE *fp = NULL;
  fp = fopen (FileName, "rb");
	if (!fp)
	{
		return false;
	}
	  fseek (fp,SIZEOF_BITMAPFILEHEADER,SEEK_SET);
	  long temp;
	  BITMAPINFOHEADER info;
	  fread(&info, SIZEOF_BITMAPINFOHEADER,1,fp);
	  sizeX = le32_to_cpu (info.biWidth);
	  sizeY = le32_to_cpu(info.biHeight);

	  if(le32_to_cpu(info.biBitCount) == 24)
	    {
	      data = NULL;
	      data= new unsigned char [3*sizeY*sizeX];
	      if (!data)
		return false;
	      for (int i=sizeY-1; i>=0;i--)
		{
		  int itimes3width= 3*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
		  for (int j=0; j<sizeX;j++)
		    {
				//for (int k=2; k>=0;k--)
				//{
		      fread (data+3*j+itimes3width,sizeof (unsigned char)*3,1,fp);
		      unsigned char tmp = data[3*j+itimes3width];
		      data[3*j+itimes3width]= data[3*j+itimes3width+2];
		      data[3*j+itimes3width+2]=tmp;
				//}
		      
		    }
		}
	    }
	  else if(le32_to_cpu(info.biBitCount) == 8)
	    {
	      data = NULL;
	      data= new unsigned char [sizeY*sizeX*3];
	      unsigned char palette[256*3+1];
	      unsigned char * paltemp = palette;
	      
		for(int palcount = 0; palcount < 256; palcount++)
		  {
		    fread(paltemp, sizeof(RGBQUAD), 1, fp);
		    //			ctemp = paltemp[0];//don't reverse
		    //			paltemp[0] = paltemp[2];
		    //			paltemp[2] = ctemp;
		    paltemp+=3;
		  }
		if (!data)
		  return false;
		int k=0;
		for (int i=sizeY-1; i>=0;i--)
		  {
			for (int j=0; j<sizeX;j++)
			  {
			    fread (&ctemp,sizeof (unsigned char),1,fp);
			    data [3*(i*sizeX+j)+2] = palette[((short)ctemp)*3];	
			    data [3*(i*sizeX+j)+1] = palette[((short)ctemp)*3+1];
			    data [3*(i*sizeX+j)] = palette[((short)ctemp)*3+2];
			  }
		  }
	    }
 	fclose (fp);
	return data;

}



void png_write (const char * myfile, unsigned char * data, unsigned int width, unsigned int height, bool alpha, char bpp) {
  FILE * fp = fopen (myfile, "wb");
  png_structp png_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,NULL,NULL);
  if (!png_ptr)
    return;
  
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    return;
  }
  if (setjmp(png_ptr->jmpbuf)) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    return;
  }
  png_init_io(png_ptr, fp);
  png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

  /* set other zlib parameters */
  png_set_compression_mem_level(png_ptr, 8);
  png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
  png_set_compression_window_bits(png_ptr, 15);
  png_set_compression_method(png_ptr, 8);
  
  png_set_IHDR(png_ptr, 
	       info_ptr, 
	       width,
	       height,
	       bpp, 
	       alpha?PNG_COLOR_TYPE_RGB_ALPHA:PNG_COLOR_TYPE_RGB, 
	       PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_DEFAULT, 
	       PNG_FILTER_TYPE_DEFAULT);
  
  png_write_info(png_ptr, info_ptr);
# if __BYTE_ORDER != __BIG_ENDIAN  
  if (bpp==16) {
    png_set_swap(png_ptr);
  }
#endif
  int stride = (bpp/8)*(alpha?4:3);
  png_byte **row_pointers = new png_byte*[height];
  for (unsigned int i=0;i<height;i++) {
    row_pointers[i]= (png_byte *)&data[stride*i*width];
  }
  png_write_image (png_ptr,row_pointers);
  png_write_end(png_ptr, info_ptr);
  png_write_flush(png_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  fclose (fp);
  free (data);
  delete [] row_pointers;
}


int main (int argc , char ** argv) {
  FILE * fp1, fp2;
  int out;
  unsigned char * dat1=NULL;
  int wid1=0;int hei1=0;
  unsigned char * dat2=NULL;
  int wid2=0;int hei2=0;
 
  if (argc>3) {
    dat1= LoadTex (argv[1],wid1,hei1);
    dat2= LoadTex (argv[2],wid2,hei2);
    out = 3;
  }else {
    dat1= LoadTex (argv[1],wid1,hei1);
    out = 2;
  }
  unsigned char * data = dat1;
  if (!dat1)
    return 0;
  if (argc>3&&dat2) {
    assert (wid1==wid2);
    assert (hei1==hei2);
    data = (unsigned char *)malloc (sizeof (unsigned char)*wid1*hei1*4);
    for (int i=0;i<wid1*hei1;i++) {
      data[i*4]=dat1[i*3];
      data[i*4+1]=dat1[i*3+1];
      data[i*4+2]=dat1[i*3+2];
      data[i*4+3]=dat2[i*3];
    }
  }
  png_write (argv[out], data, wid1, hei1, dat2!=NULL, 8);  
  if (argc>3&&dat2) {
    free (dat1);
  }  
  if (dat2)
    free(dat2);
  return 0;
}
