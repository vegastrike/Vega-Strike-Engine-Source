//#define HAVE_PYTHON
#ifdef HAVE_PYTHON
#ifndef PY_INIT_H_
#define PY_INIT_H_
#include "gfx/vec.h"
#include <boost/python/detail/extension_class.hpp>

class Python {
public:
	static void init();
	static void initpaths();
	static void test();
	static void reseterrors();
};

BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE
inline PyObject *to_python (Vector vec) {
	return to_python(boost::python::tuple((double)vec.i,(double)vec.j,(double)vec.k));
}
inline PyObject *to_python (QVector vec) {
	return to_python(boost::python::tuple((double)vec.i,(double)vec.j,(double)vec.k));
}

inline Vector from_python(PyObject *p,boost::python::type<Vector>) {
	Vector vec(0,0,0);
	PyArg_ParseTuple(p,"fff",&vec.i,&vec.j,&vec.k);
	return vec;
}
inline QVector from_python(PyObject *p,boost::python::type<QVector>) {
	QVector vec(0,0,0);
 	PyArg_ParseTuple(p,"ddd",&vec.i,&vec.j,&vec.k);
	return vec;
}
BOOST_PYTHON_END_CONVERSION_NAMESPACE

void InitBriefing ();
void InitVS ();

#endif

#endif

