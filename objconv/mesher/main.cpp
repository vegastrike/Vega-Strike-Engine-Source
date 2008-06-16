#include "PrecompiledHeaders/Converter.h"
#include "Converter.h"

#if defined(_WIN32)&&!defined(__CYGWIN__)
	#define PATH_SEPARATORS "/\\"
#else
	#define PATH_SEPARATORS "/"
#endif

int main (int argc, char **argv) {
  // executable's path
  std::string rootpath=argv[0];
  std::string::size_type seppos = rootpath.find_last_of(PATH_SEPARATORS);
  if (seppos==std::string::npos)
	  rootpath = "."; else
	  rootpath.erase(seppos);

  Converter::getRootPath()=rootpath;
  return Converter::parseParams(argc-1,argv+1);
}