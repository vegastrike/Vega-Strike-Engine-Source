#ifndef IN_H
#define IN_H
enum KBSTATE {
	UP,
	DOWN,
	PRESS,
	RELEASE,
	RESET
};

typedef void (*KBHandler)(int, KBSTATE);

typedef void (*MouseHandler)(KBSTATE,int x, int y, int delx, int dely, int mod);

#endif

