#ifdef JPEG_SUPPORT
#ifndef __JPEG_MEMORY_H
#define __JPEG_MEMORY_H

#include <jpeglib.h>

METHODDEF(void) init_destination (j_compress_ptr cinfo);
METHODDEF(boolean) empty_output_buffer (j_compress_ptr cinfo);
METHODDEF(void) term_destination (j_compress_ptr cinfo);
GLOBAL(void) jpeg_memory_dest(j_compress_ptr cinfo, JOCTET *buffer,int bufsize);
int jpeg_compress(char *dst, char *src, int width, int height, int dstsize, int quality);
int jpeg_compress_to_file(char *src, char *file, int width, int height, int quality);
extern void jpeg_memory_src(j_decompress_ptr cinfo, unsigned char *ptr, size_t size);
void jpeg_decompress(unsigned char *dst, unsigned char *src, int size, int *w, int *h);
void jpeg_decompress_from_file(unsigned char *dst, char *file, int size, int *w, int *h);

#endif
#endif
