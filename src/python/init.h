//#define HAVE_PYTHON
#ifdef HAVE_PYTHON
#ifndef PY_INIT_H_
#define PY_INIT_H_

class Python {
public:
	static void init();
	static void initpaths();
	static void test();
	static void reseterrors();
};
#endif

#endif

