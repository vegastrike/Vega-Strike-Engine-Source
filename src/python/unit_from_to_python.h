#ifndef _UNIT_FROM_TO_PYTHON_H_
#define _UNIT_FROM_TO_PYTHON_H_
#ifdef USE_BOOST_129
#include <boost/python/converter/arg_from_python.hpp>
BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE


BOOST_PYTHON_TO_PYTHON_BY_VALUE(Unit*,to_python_value <UnitWrapper> ()(UnitWrapper(x)));

template <>
struct default_result_converter::apply<Unit *>
{
//	typedef boost::python::to_python_value<Unit *> type;
	typedef default_result_converter::apply<UnitWrapper>::type type;
};

/*namespace converter{

template <>
struct arg_rvalue_from_python <Vector>
{
//	typedef typename boost::add_reference<
//		typename boost::add_const<Vector>::type
//	>::type result_type;
	typedef Vector result_type;

	arg_rvalue_from_python(PyObject*p);
    bool convertible() const {return true;}

	result_type operator()(PyObject*p) {
		Vector vec(0,0,0);
		PyArg_ParseTuple(p,"fff",&vec.i,&vec.j,&vec.k);
		return vec;
	}

private:
	rvalue_from_python_data<result_type> m_data;
};

template <>
struct arg_rvalue_from_python <QVector>
{
//	typedef typename boost::add_reference<
//		typename boost::add_const<QVector>::type
//	>::type result_type;
	typedef QVector result_type;

    arg_rvalue_from_python(PyObject*p);
    bool convertible() const {return true;}

	result_type operator()(PyObject*p) {
		QVector vec(0,0,0);
		PyArg_ParseTuple(p,"ddd",&vec.i,&vec.j,&vec.k);
		return vec;
	}

private:
	rvalue_from_python_data<result_type> m_data;
};

}
*/


BOOST_PYTHON_END_CONVERSION_NAMESPACE
#else
 TO_PYTHON_SMART_POINTER(UnitWrapper);
 TO_PYTHON_SMART_POINTER(Cargo);

PYTHON_INIT_GLOBALS(Unit,UnitWrapper)
BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE
inline PyObject *to_python(Unit * un) {
  return to_python (UnitWrapper(un));
}
inline Unit * from_python(PyObject *p,boost::python::type<Unit *>) {
  UnitWrapper uw =(from_python (p,boost::python::type<UnitWrapper&>()));
  return uw.GetUnit();
}
BOOST_PYTHON_END_CONVERSION_NAMESPACE
#endif
#endif
