#ifndef _SDDS_H_
#define _SDDS_H_
#include "gfxlib_struct.h"

/*
	input is the compressed dxt file, already read in by vsimage.
	output is an empty pointer created in the calling function.
	format is the bit format of the compressed texture (rgba)
	height and width are self explanatory
	
	when function returns, output will contain the uncompressed first mipmap of the dxt image. 
	the calling function will have to replace the input pointer with the output pointer and set
	the texture format to rgba.
*/


void ddsDecompress(unsigned char *&input,unsigned char *&output,TEXTUREFORMAT format, int height, int width);

#endif
