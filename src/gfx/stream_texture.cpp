#include "stream_texture.h"

StreamTexture* StreamTexture::Clone(){
  unsigned char * x = Map();
  StreamTexture * ret = new StreamTexture(sizeX,sizeY,filtertype,x);
  UnMap(false);
  return ret;
}

StreamTexture::StreamTexture(int width, int height, enum FILTER filtertype, unsigned char * origdata) {
  /*
  img_type=Unrecognized;
  img_depth=8;
  img_color_type=(PNG_HAS_COLOR&PNG_HAS_ALPHA);
  img_alpha=true;
  strip_16=true;
  */
  mode=_24BITRGBA;
  this->stage=0;
  this->sizeX=width;
  this->sizeY=height;
  this->original=NULL;
  this->palette=NULL;
  this->refcount=0;
  texture_target=TEXTURE2D;
  image_target=TEXTURE_2D;
  ismipmapped=filtertype;
  GFXCreateTexture(width,height,RGBA32,&name,0,0,filtertype);
  this->mutabledata = (unsigned char*)malloc(sizeof(unsigned char )*width*height*4);
  if (origdata) {
    memcpy(this->mutabledata,origdata,sizeof(unsigned char)*width*height*4);
    GFXTransferTexture(mutabledata,name,RGBA32);
  }
}
unsigned char * StreamTexture::Map() {
  return mutabledata;
}
void StreamTexture::UnMap(bool changed){
  if (changed) {
    MakeActive(0);
    GFXTransferTexture(mutabledata,name,RGBA32);    
  }
}
StreamTexture::~StreamTexture() {
  GFXDeleteTexture(name);
  name=-1;
  if (this->mutabledata)
    free(this->mutabledata);
  this->mutabledata=NULL;
}
void StreamTexture::MakeActive(int stage) {
  GFXSelectTexture(name,stage);
}

