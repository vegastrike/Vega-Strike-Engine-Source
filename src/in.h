#ifndef IN_H
#define IN_H
#include <string>
enum KBSTATE {
	UP,
	DOWN,
	PRESS,
	RELEASE,
	RESET
};

typedef void (*KBHandler)(const std::string&, KBSTATE);

typedef void (*MouseHandler)(KBSTATE,int x, int y, int delx, int dely, int mod);

#endif

