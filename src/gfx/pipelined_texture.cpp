#include "pipelined_texture.h"

PipelinedTexture::PipelinedTexture () :Texture() {
}
unsigned char * PipelinedTexture::beginMutate() {
  return data;
}

Texture * PipelinedTexture::Original ()
{
	// Had to duplicate the Texture::Original() function otherwise VC++ 6 would not compile anymore
	// reporting an undefined reference to Texture::Original()
  if (original) {
    return original->Original();
  }else {
    return this;
  }

}

void PipelinedTexture::endMutate (int xoffset,int yoffset, int width, int height) {
  GFXTransferSubTexture (data,name,xoffset,yoffset,width,height,TEXTURE_2D);
}

PipelinedTexture::PipelinedTexture (unsigned int width, unsigned int height, unsigned char * current, unsigned char * last):Texture() {
  this->sizeX = width;
  this->sizeY = height;
  this->clone = 0;
  ismipmapped=BILINEAR;
  mode = _24BITRGBA;
  data = new unsigned char [4*width*height];
  stage=0;
  original=NULL;
  refcount=0;
  texture_target=TEXTURE2D;
  image_target = TEXTURE_2D;
  GFXCreateTexture (sizeX,sizeY,(mode==_24BITRGBA)?RGBA32:((mode==_8BIT)?PALETTE8:RGB32),&name,NULL,(stage==1)?0:1);
  GFXCreateTexture (sizeX,sizeY,(mode==_24BITRGBA)?RGBA32:((mode==_8BIT)?PALETTE8:RGB32),&this->current,NULL,(stage==1)?0:1);
  GFXCreateTexture (sizeX,sizeY,(mode==_24BITRGBA)?RGBA32:((mode==_8BIT)?PALETTE8:RGB32),&this->last,NULL,(stage==1)?0:1);
  if (current)
    GFXTransferTexture (current,this->current,(mode==_24BITRGBA)?RGBA32:((mode==_8BIT)?PALETTE8:RGB32),TEXTURE_2D);
  if (last)
    GFXTransferTexture (last,this->last,(mode==_24BITRGBA)?RGBA32:((mode==_8BIT)?PALETTE8:RGB32),TEXTURE_2D);	
}

void PipelinedTexture::MakeActive () {
  GFXSelectTexture (current,stage);
}
void PipelinedTexture::MakeLastActive () {
  GFXSelectTexture (last,0);
}
void PipelinedTexture::MakeBothActive() {
  GFXSelectTexture (current,stage);
  GFXSelectTexture (last,(stage==1)?0:1);
}
void PipelinedTexture::Swap() {
  stage= (stage+1)%2;
  int tmp=name;
  name=  last;
  last = current;
  current = tmp;
  if (clone) {
    clone--;
    GFXCreateTexture (sizeX,sizeY,(mode==_24BITRGBA)?RGBA32:((mode==_8BIT)?PALETTE8:RGB32),&name,NULL,(stage==1)?0:1);
  }
}

Texture * PipelinedTexture::Clone () {
  PipelinedTexture * retval = new PipelinedTexture ();
  *retval = *this;
  retval->last = last;
  retval->current= current;
  retval->clone = 2;
  int bpp = (mode==_24BITRGBA)?4:((mode==_8BIT)?1:3);
  retval->data = new unsigned char [sizeX*sizeY*bpp];
  memcpy (retval->data,data,bpp*sizeof(unsigned char)*sizeX*sizeY);
  GFXCreateTexture (sizeX,sizeY,(mode==_24BITRGBA)?RGBA32:((mode==_8BIT)?PALETTE8:RGB32),&name,NULL,(stage==1)?0:1,BILINEAR,TEXTURE2D);
  GFXTransferTexture (retval->data,name,(mode==_24BITRGBA)?RGBA32:((mode==_8BIT)?PALETTE8:RGB32),TEXTURE_2D);
  return retval;
}

PipelinedTexture::~PipelinedTexture () {
  if (clone<2) {
    GFXDeleteTexture (current);
  }
  if (!clone) {
    GFXDeleteTexture (last);
  }
  GFXDeleteTexture(name);
}
