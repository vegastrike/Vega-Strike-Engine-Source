#include <png.h> 
#include <stdio.h>
#include <stdlib.h>
#include "endianness.h"



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
