#include <png.h> 
#include <stdio.h>
#include <stdlib.h>




unsigned short * Rread (const char * myfile) {
  unsigned short *tm = (unsigned short *)malloc (512*512*sizeof (unsigned short));
  FILE * fp = fopen (myfile,"rb");
  if (!fp)
    return tm;
  fread (tm,sizeof (unsigned short), 512*512,fp);
  fclose (fp);
  return tm;
}

void Wwrite (const char * myfile, unsigned short * data) {
  FILE * fp = fopen (myfile, "wb");
  png_structp png_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,NULL,NULL);
  //        user_error_fn, user_warning_fn);
    if (!png_ptr)
       return;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
       png_destroy_write_struct(&png_ptr,
         (png_infopp)NULL);
       return;
    }
    if (setjmp(png_ptr->jmpbuf)) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      fclose(fp);
      return;
    }
    png_init_io(png_ptr, fp);
  png_set_filter(png_ptr, 0,
       PNG_FILTER_NONE);
  png_set_compression_level(png_ptr,		    Z_BEST_COMPRESSION);

  /* set other zlib parameters */
  png_set_compression_mem_level(png_ptr, 8);
  png_set_compression_strategy(png_ptr,
			       Z_DEFAULT_STRATEGY);
  png_set_compression_window_bits(png_ptr, 15);
  png_set_compression_method(png_ptr, 8);
  
  const unsigned int height=512;
  const unsigned int width =512;
  png_set_IHDR(png_ptr, info_ptr, height,width,
	       16, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  
  png_write_info(png_ptr, info_ptr);
  png_set_swap(png_ptr);
  png_byte *row_pointers[height];
  for (unsigned int i=0;i<height;i++) {
    row_pointers[i]= (png_byte *)&data[i*width];
  }
  png_write_image (png_ptr,row_pointers);
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);



  // png_write_flush(png_ptr);


  fclose (fp);
  free (data);
}


int main (int argc, char **argv) {
  Wwrite (argv[2],Rread (argv[1])); 
  return 0;

}
