#include <cstring>
#include <string>
#include "background.h"
Background::Background(const char *, int, float, const std::string&, const GFXColor &, bool) {

}
void Background::EnableBG(bool){}
void Background::Draw(){}
void Background::BackgroundClone::FreeClone(){}
Background::BackgroundClone Background::Cache() {
  Background::BackgroundClone ret;
  memset(&ret,0,sizeof(ret));
  return ret;
}
