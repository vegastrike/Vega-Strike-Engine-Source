#ifndef _IN_KB_DATA_H_
#define _IN_KB_DATA_H_
#include <string>
class KBData{
public:
  std::string data;
  KBData(){}
  KBData(const std::string &s){this->data=s;}
};
#endif
