//#define HAVE_PYTHON
#ifdef HAVE_PYTHON
#ifndef PY_INIT_H_
#define PY_INIT_H_
#include "gfx/vec.h"
#ifdef USE_BOOST_129
#include <boost/python.hpp>
#else
#include <boost/python/detail/extension_class.hpp>
#endif
class Python {
public:
	static void init();
	static void initpaths();
	static void test();
	static void reseterrors();
};

BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE
#ifdef USE_BOOST_129
/*
typedef boost::tuples::tuple<double,double,double> python_wector;
struct to_python <Vector> {
	inline PyObject *operator () (const Vector &vec) const{
		boost::python::to_python<boost::python::tuple> a();
		return a.convert(boost::python::make_tuple((double)vec.i,(double)vec.j,(double)vec.k)); 
	}
};
*/
#ifndef BOOST_PYTHON_TO_PYTHON_BY_VALUE
# define BOOST_PYTHON_RETURN_TO_PYTHON_BY_VALUE(T, expr)        \
    template <> struct to_python_value<T&>                      \
        : detail::builtin_to_python                             \
    {                                                           \
        inline PyObject* operator()(T const& x) const           \
        {                                                       \
            return (expr);                                      \
        }                                                       \
    };                                                          \
    template <> struct to_python_value<T const&>                \
        : detail::builtin_to_python                             \
    {                                                           \
        inline PyObject* operator()(T const& x) const           \
        {                                                       \
            return (expr);                                      \
        }                                                       \
    };

# define BOOST_PYTHON_ARG_TO_PYTHON_BY_VALUE(T, expr)   \
    namespace converter                                 \
    {                                                   \
      template <> struct arg_to_python< T >             \
        : handle<>                                      \
      {                                                 \
          arg_to_python(T const& x)                     \
            : python::handle<>(expr) {}                 \
      };                                                \
    } 

// Specialize argument and return value converters for T using expr
# define BOOST_PYTHON_TO_PYTHON_BY_VALUE(T, expr)       \
        BOOST_PYTHON_RETURN_TO_PYTHON_BY_VALUE(T,expr)  \
        BOOST_PYTHON_ARG_TO_PYTHON_BY_VALUE(T,expr)
#endif
BOOST_PYTHON_TO_PYTHON_BY_VALUE(Vector, boost::python::to_python_value<boost::python::tuple> ()(boost::python::make_tuple((double)x.i,(double)x.j,(double)x.k)));
BOOST_PYTHON_TO_PYTHON_BY_VALUE(QVector, boost::python::to_python_value<boost::python::tuple> ()(boost::python::make_tuple((double)x.i,(double)x.j,(double)x.k)));
#else
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
#endif
BOOST_PYTHON_END_CONVERSION_NAMESPACE

void InitBriefing ();
void InitDirector ();
void InitVS ();
void InitBase ();

#endif

#endif

