#include "vsimage.h"
#include "vsfilesystem.h"
#include "vs_globals.h"
using namespace VSFileSystem;

#undef VSIMAGE_DEBUG

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

int PNG_HAS_PALETTE =1;
int PNG_HAS_COLOR=2;
int PNG_HAS_ALPHA=4;

LOCALCONST_DEF(VSImage, int,SIZEOF_BITMAPFILEHEADER,sizeof(WORD)+sizeof(DWORD)+sizeof(WORD)+sizeof(WORD)+sizeof(DWORD))
LOCALCONST_DEF(VSImage, int,SIZEOF_BITMAPINFOHEADER,sizeof(DWORD)+sizeof(LONG)+sizeof(LONG)+2*sizeof(WORD)+2*sizeof(DWORD)+2*sizeof(LONG)+2*sizeof(DWORD))
LOCALCONST_DEF(VSImage, int,SIZEOF_RGBQUAD,sizeof(BYTE)*4)

VSImage::VSImage()
{
	this->Init();
}

void	VSImage::Init()
{
	this->tt = NULL;
	this->img_type = Unrecognized;
	//this->tex->palette = NULL;
	this->strip_16 = false;
}

void	VSImage::Init( VSFile * f, textureTransform *t, bool strip, VSFile *f2)
{
	assert( f!=NULL);

	this->Init();

	this->img_file = f;
	this->img_file2 = f2;
	this->tt = t;
	this->strip_16 = strip;
}

VSImage::VSImage( VSFile * f, textureTransform * t, bool strip, VSFile * f2)
{
	this->Init( f, t, strip, f2);
}

VSImage::~VSImage()
{
	// img_file? and tt should not be deleted since they are passed as args to the class
}

unsigned char *	VSImage::ReadImage( VSFile * f, textureTransform * t, bool strip, VSFile * f2)
{
	this->Init( f, t, strip, f2);

	unsigned char * ret = NULL;

	CheckFormat( img_file);
	switch( this->img_type)
	{
		case PngImage :
			this->img_type = PngImage;
			ret = this->ReadPNG();
		break;
		case JpegImage :
			this->img_type = JpegImage;
			ret = this->ReadJPEG();
		break;
		case BmpImage :
			this->img_type = BmpImage;
			ret = this->ReadBMP();
		break;
		default :
			this->img_type = Unrecognized;
			cerr<<"VSImage ERROR : Unknown image format"<<endl;
			VSExit(1);
	}
	return ret;
}

VSError	VSImage::CheckPNGSignature( VSFile * file)
{
	VSError ret = Ok;

	// Do standard reading of the file
	unsigned char sig[8];
	file->Begin();
	file->Read(sig, 8);
	if (!png_check_sig(sig, 8))
		ret = BadFormat;
	
	return ret;
}

VSError	VSImage::CheckJPEGSignature( VSFile * file)
{
	VSError ret = Ok;

	// First 4 aren't known to me
	// Next 2 bytes is length
	// Next 5 are JFIF\0
	// Next 2 are version numbers
	char sig[13];
	file->Begin();
	file->Read( sig, 13);
	/*
	for( int i=0; i<13; i++)
		cerr<<sig[i]<<" ";
	cerr<<endl;
	*/
	if( strncmp( sig+6, "JFIF", 4))
		ret = BadFormat;
	file->Begin();
	return ret;
}

VSError	VSImage::CheckBMPSignature( VSFile * file)
{
	VSError ret = Ok;

	char head1;
	char head2;
	file->Begin();
	file->Read(&head1, 1);
	file->Read(&head2,1);
	if (toupper(head1)!='B'||toupper (head2)!='M')
		ret = BadFormat;
	file->Begin();

	return ret;
}

void	VSImage::CheckFormat( VSFile * file)
{
	if( this->CheckPNGSignature(file)==Ok)
	{
	#ifdef VSIMAGE_DEBUG
		cerr<<"\tFound a PNG file"<<endl;
	#endif
		this->img_type = PngImage;
		return;
	}
	if( this->CheckBMPSignature(file)==Ok)
	{
	#ifdef VSIMAGE_DEBUG
		cerr<<"\tFound a BMP file"<<endl;
	#endif
		this->img_type = BmpImage;
		return;
	}
	if( this->CheckJPEGSignature(file)==Ok)
	{
	#ifdef VSIMAGE_DEBUG
		cerr<<"\tFound a JPEG file"<<endl;
	#endif
		this->img_type = JpegImage;
		return;
	}
}


void PngReadFunc(png_struct *Png, png_bytep buf, png_size_t size)
{
#ifdef VSIMAGE_DEBUG
	cerr<<"PNG DEBUG : preparing to copy "<<size<<" bytes from PngFileBuffer"<<endl;
#endif
    TPngFileBuffer *PngFileBuffer=(TPngFileBuffer*)png_get_io_ptr(Png);
    memcpy(buf,PngFileBuffer->Buffer+PngFileBuffer->Pos,size);
    PngFileBuffer->Pos+=size;
}

/* We can't write in volumes yet so this is useless now
void PngWriteFunc(png_struct *Png, png_bytep buf, png_size_t size)
{
	cerr<<"PNG DEBUG : preparing to write "<<size<<" bytes from PngFileBuffer"<<endl;
}
*/

static void
png_cexcept_error(png_structp png_ptr, png_const_charp msg)
{
   if(png_ptr)
     ;
#ifndef PNG_NO_CONSOLE_IO
   VSFileSystem::vs_fprintf(stderr, "libpng error: %s\n", msg);
#endif

}

unsigned char *	VSImage::ReadPNG()
{
	TPngFileBuffer	PngFileBuffer = {NULL,0};
	palette = NULL;
	png_structp png_ptr;
	png_bytepp row_pointers;
	png_infop info_ptr;
	int  interlace_type;

	img_file->Begin();
	if( !CheckPNGSignature( img_file))
	{
		cerr<<"VSImage::ReadPNG() ERROR : NOT A PNG FILE"<<endl;
		VSExit(1);
	}
	// Go after sig since we already checked it
	// Only when reading from a buffer otherwise CheckPNGSignature already did the work
	if( img_file->UseVolume())
	{
		PngFileBuffer.Buffer = img_file->pk3_extracted_file;
		PngFileBuffer.Pos = 8;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)png_cexcept_error, (png_error_ptr)NULL);
	if (png_ptr == NULL)
	{
		VSExit(1);
		return NULL;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		cerr<<"VSImage ERROR : PNG info_ptr == NULL !!!"<<endl;
		VSExit(1);
		return NULL;
	}
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		/* If we get here, we had a problem reading the file */
		cerr<<"VSImage ERROR : problem reading file/buffer -> setjmp !!!"<<endl;
		VSExit(1);
		return NULL;
	}

	if( !img_file->UseVolume())
	{
		png_init_io(png_ptr, img_file->GetFP());
	}
	else
	{
		png_set_read_fn (png_ptr,(png_voidp)&PngFileBuffer,(png_rw_ptr)PngReadFunc);
	}

	png_set_sig_bytes(png_ptr, 8);
#ifdef VSIMAGE_DEBUG
	   VSFileSystem::vs_fprintf (stderr,"Loading Done. Decompressing\n");
#endif
	png_read_info(png_ptr, info_ptr);  /* read all PNG info up to image data */
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&this->sizeX, (png_uint_32 *)&this->sizeY, &this->img_depth, &this->img_color_type, &interlace_type, NULL, NULL);
#ifdef VSIMAGE_DEBUG
	cerr<<"1. Loading a PNG file : width="<<sizeX<<", height="<<sizeY<<", depth="<<img_depth<<", img_color="<<img_color_type<<", interlace="<<interlace_type<<endl;
#endif
	# if __BYTE_ORDER != __BIG_ENDIAN
	if (this->img_depth==16)
		png_set_swap (png_ptr);
	#endif

	if (this->img_depth==16&&strip_16)
		png_set_strip_16(png_ptr);
	if (strip_16&&this->img_color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);
		   
	if (this->img_color_type == PNG_COLOR_TYPE_GRAY && this->img_depth < 8)
		png_set_gray_1_2_4_to_8(png_ptr);

	png_set_expand (png_ptr);
	png_read_update_info (png_ptr,info_ptr);
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&this->sizeX, (png_uint_32 *)&this->sizeY, &this->img_depth, &this->img_color_type, &interlace_type, NULL, NULL);
#ifdef VSIMAGE_DEBUG
	cerr<<"2. Loading a PNG file : width="<<sizeX<<", height="<<sizeY<<", depth="<<img_depth<<", img_color="<<img_color_type<<", interlace="<<interlace_type<<endl;
#endif
	row_pointers = (unsigned char **)malloc (sizeof (unsigned char *) *this->sizeY);
	int numchan=1;
	if (this->img_color_type&PNG_COLOR_MASK_COLOR)
		numchan =3;
	if (this->img_color_type &PNG_COLOR_MASK_PALETTE)
		numchan =1;
	if (this->img_color_type&PNG_COLOR_MASK_ALPHA)
		numchan++;
	unsigned long stride = numchan*sizeof (unsigned char)*this->img_depth/8;
#ifdef VSIMAGE_DEBUG
	cerr<<"3. Allocating image buffer of size="<<(stride*sizeX*sizeY)<<endl;
#endif
	unsigned char * image = (unsigned char *) malloc (stride*this->sizeX*this->sizeY);
	for (unsigned int i=0;i<this->sizeY;i++)
	{
		row_pointers[i] = &image[i*stride*this->sizeX];
	}
	png_read_image (png_ptr,row_pointers);
	unsigned char * result;
	if (tt) {
#ifdef VSIMAGE_DEBUG
		cerr<<"4. Doing a tranformation"<<endl;
#endif
		result = (*tt) (this->img_depth,this->img_color_type,this->sizeX,this->sizeY,row_pointers);
		free (image);
	}
	else
	{
		result = image;
	}
	free (row_pointers);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
#ifdef VSIMAGE_DEBUG
	   VSFileSystem::vs_fprintf (stderr,"Decompressing Done.\n");
#endif

	if( result)
		this->AllocatePalette();
	return result;
}

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

unsigned char *	VSImage::ReadJPEG()
{
	this->img_depth = 8;
	jpeg_decompress_struct cinfo;

	my_error_mgr jerr;
	JSAMPARRAY row_pointers=NULL;// Output row buffer

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer))
	{
		// If we get here, the JPEG code has signaled an error.
		// We need to clean up the JPEG object, and return.
		jpeg_destroy_decompress(&cinfo);
#ifdef VSIMAGE_DEBUG
		cerr<<"VSImage ERROR : error reading jpg file"<<endl;
#endif
		VSExit(1);
		return NULL;
	}

	jpeg_create_decompress(&cinfo);

	if( !img_file->UseVolume())
		jpeg_stdio_src((j_decompress_ptr)&cinfo, img_file->GetFP());
	else
		jpeg_memory_src(&cinfo, (unsigned char *)img_file->pk3_extracted_file, img_file->Size());

	(void) jpeg_read_header(&cinfo, TRUE);
	this->sizeX = cinfo.image_width;
	this->sizeY = cinfo.image_height;

	(void) jpeg_start_decompress(&cinfo);

	this->img_color_type = PNG_COLOR_TYPE_RGB;
	if (cinfo.output_components == 1)
		this->img_color_type = PNG_COLOR_TYPE_GRAY;
	else if (cinfo.output_components==4)
		this->img_color_type = PNG_COLOR_TYPE_RGB_ALPHA;
	else if (cinfo.output_components== 2)
		this->img_color_type = PNG_COLOR_TYPE_GRAY_ALPHA;

#ifdef VSIMAGE_DEBUG
	cerr<<"1. Loading a JPEG file : width="<<sizeX<<", height="<<sizeY<<", img_color="<<img_color_type<<endl;
#endif
	row_pointers = (unsigned char **)malloc (sizeof (unsigned char *) * cinfo.image_height);

	this->img_depth=8;
	int numchan =cinfo.output_components;

	unsigned long stride = numchan*sizeof (unsigned char)*this->img_depth/8;
	unsigned char * image = (unsigned char *) malloc (stride*cinfo.image_width*cinfo.image_height);

	for (unsigned int i=0;i<cinfo.image_height;i++)
	{
		row_pointers[i] = &image[i*stride*cinfo.image_width];
	}
	unsigned int count=0;
	while (count<this->sizeY)
	{
		count+= jpeg_read_scanlines(&cinfo,&( row_pointers[count]), this->sizeY-count);
	}

	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	unsigned char * result=image;
	if (tt) {
		result = (*tt) (this->img_depth,this->img_color_type,this->sizeX,this->sizeY,row_pointers);
		free (image);
	}
	free (row_pointers);
	if( result)
		this->AllocatePalette();
	return result;
}

unsigned char *	VSImage::ReadBMP()
{
  	unsigned char *data;
	if( CheckBMPSignature( img_file)!=Ok)
	{
		cerr<<"VSImage ERROR : BMP signature check failed : this should not happen !!!"<<endl;
		VSExit(1);
	}
	//seek back to beginning
	img_file->GoTo(SIZEOF_BITMAPFILEHEADER);
	//long temp;
	BITMAPINFOHEADER info;
	img_file->Read(&info, SIZEOF_BITMAPINFOHEADER);
	this->sizeX = le32_to_cpu(info.biWidth);	
	this->sizeY = le32_to_cpu(info.biHeight);

	/*
	if (img_file2!=NULL)
	{
		img_file2->GoTo(SIZEOF_BITMAPFILEHEADER);
		  
		  img_file2->Read(&info1,SIZEOF_BITMAPINFOHEADER);
		  if (tex->sizeX != (unsigned int) le32_to_cpu(info1.biWidth)||tex->sizeY!=(unsigned int)le32_to_cpu(info1.biHeight))
		  {
		      return NULL;
		  }
		  RGBQUAD ptemp1;	
		  if (le16_to_cpu(info1.biBitCount) == 8)
		  {
		      for (int i=0; i<256; i++)
				img_file2->Read(&ptemp1, sizeof(RGBQUAD)); //get rid of the palette for a b&w greyscale alphamap
		      
		  }
	}
	*/
	if(le16_to_cpu(info.biBitCount) == 24)
	{
		mode = _24BITRGBA;
		if(img_file2 && img_file2->Valid())
			mode = _24BITRGBA;
		data = NULL;
		data= (unsigned char *)malloc (sizeof(unsigned char)*4*this->sizeY*this->sizeX); // all bitmap data needs to be 32 bits
		if (!data)
			return NULL;
		for (int i=sizeY-1; i>=0;i--)
		{
			int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
			for (unsigned int j=0; j<sizeX;j++)
			{
				if(img_file2 && img_file2->Valid())
					img_file2->Read(data+3, sizeof(unsigned char));
				else
					*(data+3) = 0xff; // default alpha = 1
				for (int k=2; k>=0;k--)
				{
					img_file->Read (data+k+4*j+itimes4width,sizeof (unsigned char));
				}
			}
		}
	}
	else if(le16_to_cpu(info.biBitCount) == 8)
	{
		mode = _8BIT;
		data = NULL;
		data= (unsigned char *)malloc(sizeof(unsigned char)*sizeY*sizeX);
		this->palette = (unsigned char *)malloc(sizeof(unsigned char)* (256*4+1));
		memset (this->palette,0,(256*4+1)*sizeof(unsigned char));
		unsigned char *paltemp = this->palette;
		unsigned char ctemp;
		for(int palcount = 0; palcount < 256; palcount++)
		{
			img_file->Read(paltemp, SIZEOF_RGBQUAD);
			ctemp = paltemp[0];
			paltemp[0] = paltemp[2];
			paltemp[2] = ctemp;
			paltemp+=4; // pal size
		}
		if (!data)
			return NULL;
		for (int i=sizeY-1; i>=0;i--)
		{
			for (unsigned int j=0; j<sizeX;j++)
			{
				img_file->Read(data+ j + i * sizeX,sizeof (unsigned char));
			}
		}
	}
	return data;
}

void	VSImage::AllocatePalette()
{
	  //FIXME deal with palettes and grayscale with alpha
	  if (!(img_color_type&PNG_HAS_COLOR)||(img_color_type&PNG_HAS_PALETTE)) {
	    mode=_8BIT;
	    if (!(img_color_type&PNG_HAS_COLOR)){
	      palette = (unsigned char *) malloc(sizeof(unsigned char)*(256*4+1));
	      for (unsigned int i =0;i<256;i++) {
			palette[i*4]=i;
			palette[i*4+1]=i;
			palette[i*4+2]=i;
			palette[i*4+3]=255;
	      }
	    }
	  } 
	  if (img_color_type&PNG_HAS_COLOR) {
	    if (img_color_type&PNG_HAS_ALPHA) {
	      mode=_24BITRGBA;
	    } else {
	      mode=_24BIT;
	    }
	  }
}

VSError	VSImage::WriteImage( char * filename, unsigned char * data, VSImageType type, unsigned int width, unsigned int height, bool alpha, char bpp, VSFileType ft)
{
	this->img_type = type;
	VSFile f;
	VSError err = f.OpenCreateWrite( filename, ft);
	if( err>Ok)
	{
		cerr<<"VSImage ERROR : failed to open "<<filename<<" for writing"<<endl;
		VSExit(1);
	}

	VSError ret = this->WriteImage( &f, data, type, width, height, alpha, bpp);
	f.Close();
	return ret;
}

VSError	VSImage::WriteImage( VSFile * pf, unsigned char * data, VSImageType type, unsigned int width, unsigned int height, bool alpha, char bpp)
{
	VSError ret = BadFormat;

	this->img_file = pf;
	this->img_depth = bpp;
	this->sizeX = width;
	this->sizeY = height;
	this->img_alpha = alpha;

	switch( type)
	{
		case PngImage :
			ret = this->WritePNG( data);
		break;
		case JpegImage :
			ret = this->WriteJPEG( data);
		break;
		case BmpImage :
			ret = this->WriteBMP( data);
		break;
		default :
			cerr<<"VSImage ERROR : Unknown image format"<<endl;
			VSExit(1);
	}
	this->img_file = NULL;
	this->img_depth = 0;
	this->sizeX = 0;
	this->sizeY = 0;
	this->img_alpha = false;
	return ret;
}

VSError	VSImage::WritePNG( unsigned char * data)
{
  png_structp png_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,NULL,NULL);
  if (!png_ptr)
    return BadFormat;
  
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    return BadFormat;
  }
  if (setjmp(png_ptr->jmpbuf)) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return BadFormat;
  }

  //if( !img_file->UseVolume())
  // For now we always write to standard files
  png_init_io(png_ptr, img_file->GetFP());

  png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

  /* set other zlib parameters */
  png_set_compression_mem_level(png_ptr, 8);
  png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
  png_set_compression_window_bits(png_ptr, 15);
  png_set_compression_method(png_ptr, 8);
  
  png_set_IHDR(png_ptr, 
	       info_ptr, 
	       this->sizeX,
	       this->sizeY,
	       this->img_depth, 
	       this->img_alpha?PNG_COLOR_TYPE_RGB_ALPHA:PNG_COLOR_TYPE_RGB, 
	       PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_DEFAULT, 
	       PNG_FILTER_TYPE_DEFAULT);
  
  png_write_info(png_ptr, info_ptr);
# if __BYTE_ORDER != __BIG_ENDIAN  
  if (this->img_depth==16) {
    png_set_swap(png_ptr);
  }
#endif
  int stride = (this->img_depth/8)*(this->img_alpha?4:3);
  png_byte **row_pointers = new png_byte*[this->sizeY];
  for (unsigned int i=0;i<this->sizeY;i++) {
    row_pointers[i]= (png_byte *)&data[stride*i*sizeX];
  }
  png_write_image (png_ptr,row_pointers);
  png_write_end(png_ptr, info_ptr);
  png_write_flush(png_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  free (data);
  delete [] row_pointers;
  return Ok;
}

VSError	VSImage::WriteJPEG( unsigned char * data)
{
	return BadFormat;
}

VSError	VSImage::WriteBMP( unsigned char * data)
{
	return BadFormat;
}

