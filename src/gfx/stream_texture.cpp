#include "stream_texture.h"

StreamTexture* StreamTexture::Clone();{
  unsigned char * x = Map();
  StreamTexture * ret = new StreamTexture(width,height,filtertype,x);
  UnMap(false);
  return ret;
}

StreamTexture::StreamTexture(int width, int height, enum FILTER filtertype, unsigned char * origdata) {
  this->width=width;
  this->height=height;
  this->filtertype=filtertype;
  this->data = malloc(sizeof(unsigned char )*width*height*4);
  if (origdata)
    memcpy(this->data,origdata,sizeof(unsigned char)*width*height*4);
  GFXTransferTexture(data,handle,RGBA32);
}
unsigned char * StreamTexture::Map() {
  return data;
}
void StreamTexture::UnMap(bool changed){
  if (changed) {
    GFXTransferTexture(data,handle,RGBA32);    
  }
}
StreamTexture::~StreamTexture() {
  GFXDeleteTexture(handle);
  if (this->data)
    free(this->data);
  this->data=NULL;
}
void StreamTexture::MakeActive(int stage) {
  GFXSelectTexture(handle,stage);
}

