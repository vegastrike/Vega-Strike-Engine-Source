#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <utility>

#ifdef _WIN32
	#include <hash_map>
#else
	#if __GNUC__ != 2
		#include <ext/hash_map>
	#endif
#endif

#include <boost/smart_ptr.hpp>

#if !defined(_WIN32) && !defined(__CYGWIN__)
	#include <stdlib.h>
#else
	#include <windows.h>
#endif

#define GL_GLEXT_PROTOTYPES 1

#if defined(__APPLE__) || defined(MACOSX)
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
#else
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif
