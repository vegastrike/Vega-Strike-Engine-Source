#include <cstring>
#include <string>
#include "background.h"
Background::Background(const char *, int, float, std::string) {

}
void Background::EnableBG(bool){}
void Background::Draw(){}
void Background::BackgroundClone::FreeClone(){}
Background::BackgroundClone Background::Cache() {
  Background::BackgroundClone ret;
  memset(&ret,0,sizeof(ret));
  return ret;
}
