#define JPEG_SUPPORT
#include "endianness.h"
#include <png.h>
#include <stdlib.h>
#include "png_texture.h"
#include <assert.h>
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#ifdef JPEG_SUPPORT
extern "C" {
#include <jpeglib.h>
}
#endif

//#define PNGDEBUG
int PNG_HAS_PALETTE =1;
int PNG_HAS_COLOR=2;
int PNG_HAS_ALPHA=4;
unsigned char * texTransform (int &bpp, int &color_type, unsigned long &width, unsigned long &height, unsigned char ** rp){
  unsigned char * data;
  unsigned int row_size;
  assert (bpp==8);
  if ((color_type&PNG_HAS_PALETTE)||(!(color_type&PNG_HAS_COLOR))) {
    row_size =width*(bpp/8)*sizeof (unsigned char)*((color_type&PNG_HAS_ALPHA)?2:1);
  } else {
    row_size = width*(bpp/8)*sizeof(unsigned char)*((color_type&PNG_HAS_ALPHA)?4:3);
  }
  data = (unsigned char *)malloc (row_size*height);
  for (unsigned int i=0;i<height;i++) {
    memcpy  (data+i*row_size,rp[i],row_size);
  }
  return data;
}

unsigned char * heightmapTransform (int &bpp, int &color_type, unsigned long &width, unsigned long &height, unsigned char ** row_pointers) {
  unsigned short * dat = (unsigned short *) malloc (sizeof (unsigned short)*width*height);
  if ((bpp==8&&color_type==PNG_COLOR_TYPE_RGB_ALPHA)||color_type==PNG_COLOR_TYPE_GRAY ||color_type==PNG_COLOR_TYPE_GRAY_ALPHA) {
    if (bpp==8&&color_type==PNG_COLOR_TYPE_GRAY) {
      for (unsigned int i=0;i<height;i++) {
	unsigned long iwid = i*width;
	for (unsigned int j=0;j<width;j++) {
	  dat[iwid+j] = row_pointers[i][j];
	}
      }
    } else {
      if ((bpp==16&&color_type==PNG_COLOR_TYPE_GRAY)||(bpp==8&&color_type==PNG_COLOR_TYPE_GRAY_ALPHA)) {
	for (unsigned int i=0;i<height;i++) {
	  memcpy (&dat[i*width],row_pointers[i], sizeof (unsigned short)*width);
	}
      } else {
	//type is RGBA32 or GrayA32
	for (unsigned int i=0;i<height;i++) {
	  unsigned long iwid = i*width;
	  for (unsigned int j=0;j<width;j++) {
	    dat[iwid+j]= (((unsigned short *)row_pointers[i])[j*2]);
	  }
	}
      }
    }
  } else {
    if (color_type==PNG_COLOR_TYPE_RGB) {
      unsigned int coloffset = (bpp==8)?3:6;
      for (unsigned int i=0;i<height;i++) {
	unsigned long iwid = i*width;
	for (unsigned int j=0;j<width;j++) {
	  dat[iwid+j]= * ((unsigned short *)(&(row_pointers[i][j*coloffset])));
	}
      }
      
    }else if (color_type== PNG_COLOR_TYPE_RGB_ALPHA) {///16 bit colors...take Red
      for (unsigned int i=0;i<height;i++) {
	unsigned long iwid = i*width;
	for (unsigned int j=0;j<width;j++) {
	  dat[iwid+j]= (((unsigned short *)row_pointers[i])[j*4]);
	}
      }	
    }
  }
  bpp = 16;
  color_type = PNG_COLOR_TYPE_GRAY;
  return (unsigned char *)dat;
}


unsigned char * terrainTransform (int &bpp, int &color_type, unsigned long &width, unsigned long &height, unsigned char ** row_pointers) {
  unsigned char * dat = (unsigned char *) malloc (sizeof (unsigned char)*width*height);
  if ((bpp==8&&color_type==PNG_COLOR_TYPE_RGB_ALPHA)||color_type==PNG_COLOR_TYPE_GRAY ||color_type==PNG_COLOR_TYPE_GRAY_ALPHA) {
    if (bpp==8&&color_type==PNG_COLOR_TYPE_GRAY) {
      for (unsigned int i=0;i<height;i++) {
	memcpy (&dat[i*width],row_pointers[i], sizeof (unsigned char)*width);
      }
    } else {
      if ((bpp==16&&color_type==PNG_COLOR_TYPE_GRAY)||(bpp==8&&color_type==PNG_COLOR_TYPE_GRAY_ALPHA)) {
	for (unsigned int i=0;i<height;i++) {
	  unsigned long iwid = i*width;
	  for (unsigned int j=0;j<width;j++) {
	    dat[iwid+j] = (row_pointers[i])[j*2];
	  }
	}
      } else {
	//type is RGBA32 or GrayA32
	for (unsigned int i=0;i<height;i++) {
	  unsigned long iwid = i*width;
	  for (unsigned int j=0;j<width;j++) {
	    dat[iwid+j]= ((row_pointers[i])[j*4]);
	  }
	}
      }
    }
  } else {
    if (color_type==PNG_COLOR_TYPE_RGB) {
      unsigned int coloffset = (bpp==8)?3:6;
      for (unsigned int i=0;i<height;i++) {
	unsigned long iwid = i*width;
	for (unsigned int j=0;j<width;j++) {
	  dat[iwid+j]= * ((unsigned char *)(&(row_pointers[i][j*coloffset])));
	}
      }
      
    }else if (color_type== PNG_COLOR_TYPE_RGB_ALPHA) {///16 bit colors...take Red
      for (unsigned int i=0;i<height;i++) {
	unsigned long iwid = i*width;
	for (unsigned int j=0;j<width;j++) {
	  dat[iwid+j]= (((unsigned short *)row_pointers[i])[j*4])/256;
	}
      }	
    }
  }
  bpp = 8;
  color_type = PNG_COLOR_TYPE_GRAY;

  return (unsigned char *)dat;
}
static void
png_cexcept_error(png_structp png_ptr, png_const_charp msg)
{
   if(png_ptr)
     ;
#ifndef PNG_NO_CONSOLE_IO
   fprintf(stderr, "libpng error: %s\n", msg);
#endif

}





#ifdef JPEG_SUPPORT

struct my_error_mgr
{
  struct jpeg_error_mgr pub;// "public" fields
  jmp_buf setjmp_buffer;      // for return to caller
};


METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
  // cinfo->err really points to a my_error_mgr struct, so coerce pointer
  my_error_mgr * myerr = (my_error_mgr *) cinfo->err;

  // Always display the message.
  // We could postpone this until after returning, if we chose.
  (*cinfo->err->output_message) (cinfo);

  // Return control to the setjmp point
  longjmp(myerr->setjmp_buffer, 1);
}
unsigned char * readVSJpeg (FILE *fp, int & bpp, int &color_type, unsigned long &width, unsigned long &height, textureTransform * tt) {
  bpp = 8;
   jpeg_decompress_struct cinfo;

   my_error_mgr jerr;
   JSAMPARRAY row_pointers=NULL;// Output row buffer

   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = my_error_exit;
   if (setjmp(jerr.setjmp_buffer)) {
       // If we get here, the JPEG code has signaled an error.
       // We need to clean up the JPEG object, close the input file, and return.
     jpeg_destroy_decompress(&cinfo);
     return NULL;
   }

   jpeg_create_decompress(&cinfo);

   jpeg_stdio_src((j_decompress_ptr)&cinfo, fp);

   (void) jpeg_read_header(&cinfo, TRUE);
   width = cinfo.image_width;
   height = cinfo.image_height;

   (void) jpeg_start_decompress(&cinfo);


   color_type = PNG_COLOR_TYPE_RGB;
   if (cinfo.output_components == 1)
     color_type = PNG_COLOR_TYPE_GRAY;
   else if (cinfo.output_components==4)
     color_type = PNG_COLOR_TYPE_RGB_ALPHA;
   else if (cinfo.output_components== 2)
     color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
   
   row_pointers = (unsigned char **)malloc (sizeof (unsigned char *) * cinfo.image_height);

   bpp=8;
   int numchan =cinfo.output_components;

   unsigned long stride = numchan*sizeof (unsigned char)*bpp/8;
   unsigned char * image = (unsigned char *) malloc (stride*cinfo.image_width*cinfo.image_height);

   for (unsigned int i=0;i<cinfo.image_height;i++) {
     row_pointers[i] = &image[i*stride*cinfo.image_width];

   }
   int count=0;
   while (count<height) {
     count+= jpeg_read_scanlines(&cinfo,&( row_pointers[count]), height-count);
   }


   (void) jpeg_finish_decompress(&cinfo);

   jpeg_destroy_decompress(&cinfo);

   unsigned char * result=image;
   if (tt) {
     result = (*tt) (bpp,color_type,width,height,row_pointers);
     free (image);
   }
   free (row_pointers);
   return result;
}

#else
unsigned char * readVSJpeg (FILE *fp, int & bpp, int &color_type, unsigned long &width, unsigned long &height, textureTransform * tt) {
  return NULL;
}
#endif // JPEG_SUPPORT

unsigned char * readImage (FILE *fp, int & bpp, int &color_type, unsigned long &width, unsigned long &height, unsigned char * &palette, textureTransform * tt, bool strip_16) {
  palette = NULL;
  unsigned char sig[8];
  png_structp png_ptr;
  png_bytepp row_pointers;
  png_infop info_ptr;
  int  interlace_type;
  fread(sig, 1, 8, fp);
  if (!png_check_sig(sig, 8)) {
    fseek (fp,0,SEEK_SET);
    return readVSJpeg (fp,bpp,color_type,width,height,tt);
  }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
      (png_error_ptr)png_cexcept_error, 
	  (png_error_ptr)NULL);
	if (png_ptr == NULL)
   {
      return NULL;
   }
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      return NULL;
   }
   if (setjmp(png_jmpbuf(png_ptr))) {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      /* If we get here, we had a problem reading the file */
      return NULL;
   }
   png_init_io(png_ptr, fp);
   png_set_sig_bytes(png_ptr, 8);
#ifdef PNGDEBUG
   fprintf (stderr,"Loading Done. Decompressing\n");
#endif
   png_read_info(png_ptr, info_ptr);  /* read all PNG info up to image data */
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&width, (png_uint_32 *)&height, &bpp, &color_type, &interlace_type, NULL, NULL);
# if __BYTE_ORDER != __BIG_ENDIAN
   if (bpp==16)
     png_set_swap (png_ptr);
#endif

   if (bpp==16&&strip_16)
     png_set_strip_16(png_ptr);
   if (strip_16&&color_type == PNG_COLOR_TYPE_PALETTE)
     png_set_palette_to_rgb(png_ptr);
   
   if (color_type == PNG_COLOR_TYPE_GRAY &&
       bpp < 8) png_set_gray_1_2_4_to_8(png_ptr);
   


   png_set_expand (png_ptr);
   png_read_update_info (png_ptr,info_ptr);
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&width, (png_uint_32 *)&height, &bpp, &color_type, &interlace_type, NULL, NULL);
   row_pointers = (unsigned char **)malloc (sizeof (unsigned char *) *height);
   int numchan=1;
   if (color_type&PNG_COLOR_MASK_COLOR)
     numchan =3;
   if (color_type &PNG_COLOR_MASK_PALETTE)
     numchan =1;
   if (color_type&PNG_COLOR_MASK_ALPHA)
     numchan++;
   unsigned long stride = numchan*sizeof (unsigned char)*bpp/8;
   unsigned char * image = (unsigned char *) malloc (stride*width*height);
   for (unsigned int i=0;i<height;i++) {
     row_pointers[i] = &image[i*stride*width];
   }
   png_read_image (png_ptr,row_pointers);
   //   png_read_image(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND , NULL);
   //row_pointers = png_get_rows(png_ptr, info_ptr);
   unsigned char * result;
   if (tt) {
     result = (*tt) (bpp,color_type,width,height,row_pointers);
     
     free (image);
   }else {
     result = image;
   }
   free (row_pointers);
   //   png_infop end_info;
   png_read_end(png_ptr, info_ptr);
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
#ifdef PNGDEBUG
   fprintf (stderr,"Decompressing Done.\n");
#endif
   /* close the file */
   return result;
}

#if 0
int main () {
  const char nam []="test.png"; int  bpp; int channels; unsigned int width; unsigned int height; unsigned char * palette;
  unsigned char * tmp = readImage(nam,bpp,channels,width,height,palette, &heightmapTransform);
  free (tmp);
  
  tmp = readTexture(nam,bpp,channels,width,height,palette, &terrainTransform);
  free (tmp);

}
#endif
