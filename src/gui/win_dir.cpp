#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>

DIR * _g_win32_opendir (const char * c) {
	return opendir (c);
}
dirent * _g_win32_readdir (DIR * d) {
	return readdir  (d);
}

void _g_win32_closedir (DIR * d) {
	closedir (d);
}
#endif