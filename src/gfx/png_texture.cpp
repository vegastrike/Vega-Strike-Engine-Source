#include "endianness.h"
#include <png.h>
#include <stdlib.h>
#include "png_texture.h"
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

int PNG_HAS_PALETTE =1;
int PNG_HAS_COLOR=2;
int PNG_HAS_ALPHA=4;

unsigned char * heightmapTransform (int &bpp, int &color_type, unsigned int &width, unsigned int &height, unsigned char ** row_pointers) {
  unsigned short * dat = (unsigned short *) malloc (sizeof (unsigned short)*width*height);
  if ((bpp==8&&color_type==PNG_COLOR_TYPE_RGB_ALPHA)||color_type==PNG_COLOR_TYPE_GRAY ||color_type==PNG_COLOR_TYPE_GRAY_ALPHA) {
    if (bpp==8&&color_type==PNG_COLOR_TYPE_GRAY) {
      for (unsigned int i=0;i<height;i++) {
	unsigned int iwid = i*width;
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
	  unsigned int iwid = i*width;
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
	unsigned int iwid = i*width;
	for (unsigned int j=0;j<width;j++) {
	  dat[iwid+j]= * ((unsigned short *)(&(row_pointers[i][j*coloffset])));
	}
      }
      
    }else if (color_type== PNG_COLOR_TYPE_RGB_ALPHA) {///16 bit colors...take Red
      for (unsigned int i=0;i<height;i++) {
	unsigned int iwid = i*width;
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


unsigned char * terrainTransform (int &bpp, int &color_type, unsigned int &width, unsigned int &height, unsigned char ** row_pointers) {
  unsigned char * dat = (unsigned char *) malloc (sizeof (unsigned char)*width*height);
  if ((bpp==8&&color_type==PNG_COLOR_TYPE_RGB_ALPHA)||color_type==PNG_COLOR_TYPE_GRAY ||color_type==PNG_COLOR_TYPE_GRAY_ALPHA) {
    if (bpp==8&&color_type==PNG_COLOR_TYPE_GRAY) {
      for (unsigned int i=0;i<height;i++) {
	memcpy (&dat[i*width],row_pointers[i], sizeof (unsigned char)*width);
      }
    } else {
      if ((bpp==16&&color_type==PNG_COLOR_TYPE_GRAY)||(bpp==8&&color_type==PNG_COLOR_TYPE_GRAY_ALPHA)) {
	for (unsigned int i=0;i<height;i++) {
	  unsigned int iwid = i*width;
	  for (unsigned int j=0;j<width;j++) {
	    dat[iwid+j] = (row_pointers[i])[j*2];
	  }
	}
      } else {
	//type is RGBA32 or GrayA32
	for (unsigned int i=0;i<height;i++) {
	  unsigned int iwid = i*width;
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
	unsigned int iwid = i*width;
	for (unsigned int j=0;j<width;j++) {
	  dat[iwid+j]= * ((unsigned char *)(&(row_pointers[i][j*coloffset])));
	}
      }
      
    }else if (color_type== PNG_COLOR_TYPE_RGB_ALPHA) {///16 bit colors...take Red
      for (unsigned int i=0;i<height;i++) {
	unsigned int iwid = i*width;
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

unsigned char * readImage (const char * name, int & bpp, int &color_type, unsigned int &width, unsigned int &height, unsigned char * &palette, textureTransform * tt) {
  palette = NULL;
  unsigned char sig[8];
  fprintf (stderr,"Loading %s",name);
  png_structp png_ptr;
  png_bytepp row_pointers;
  png_infop info_ptr;
  int  interlace_type;
   FILE *fp;
   if ((fp = fopen(name, "rb")) == NULL)
      return NULL;
   fread(sig, 1, 8, fp);
  if (!png_check_sig(sig, 8))
       return NULL;   /* bad signature */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
      (png_error_ptr)png_cexcept_error, 
	  (png_error_ptr)NULL);
	if (png_ptr == NULL)
   {
      fclose(fp);
      return NULL;
   }
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      return NULL;
   }
   if (setjmp(png_jmpbuf(png_ptr))) {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      fclose(fp);
      /* If we get here, we had a problem reading the file */
      return NULL;
   }
   png_init_io(png_ptr, fp);
   png_set_sig_bytes(png_ptr, 8);
   fprintf (stderr,"Loading Done. Decompressing\n");
   png_read_info(png_ptr, info_ptr);  /* read all PNG info up to image data */
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&width, (png_uint_32 *)&height, &bpp, &color_type, &interlace_type, NULL, NULL);
# if __BYTE_ORDER != __BIG_ENDIAN

   if (bpp==16)
     png_set_swap (png_ptr);
#endif
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


   unsigned char * result = (*tt) (bpp,color_type,width,height,row_pointers);
   free (row_pointers);
   free (image);
   png_infop end_info;
   png_read_end(png_ptr, info_ptr);
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   fprintf (stderr,"Decompressing Done.\n");

   /* close the file */
   fclose(fp);
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
