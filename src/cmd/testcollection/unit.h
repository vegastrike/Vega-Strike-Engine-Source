#ifndef __UNIT_TEST_H_
#define __UNIT_TEST_H_
#include <stdio.h>

class Unit
{

	public:
		bool killed;
		bool zapped;
		int ucref;
		Unit (bool kill):killed(kill) {
			ucref=0;
			zapped=false;
		}
		void Kill() {
			if (zapped==true) {
				printf ("segfault");
			}
			killed=true;
		}
		bool Killed() {
			if (zapped==true) {
				printf ("segfault");
			}
			return killed;
		}

		void Ref () {
			if (zapped==true) {
				printf ("segfault");
			}
			ucref+=1;
		}
		void UnRef () {
			if (zapped==true) {
				printf ("segfault");
			}
			ucref-=1;
			if (ucref==0&&killed) {
				zapped=true;
			}
		}
};
#endif
