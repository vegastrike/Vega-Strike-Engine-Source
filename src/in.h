#ifndef IN_H
#define IN_H
enum KBSTATE {
	UP,
	DOWN,
	PRESS,
	RELEASE,
	RESET
};

class KBData;
typedef void (*KBHandler)(const KBData&, KBSTATE);

typedef void (*MouseHandler)(KBSTATE,int x, int y, int delx, int dely, int mod);

#endif

