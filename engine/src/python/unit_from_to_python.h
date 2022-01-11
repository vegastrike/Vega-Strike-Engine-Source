/**
* unit_from_to_python.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _UNIT_FROM_TO_PYTHON_H_
#define _UNIT_FROM_TO_PYTHON_H_
#include <boost/version.hpp>
#if BOOST_VERSION != 102800
//#include <boost/python/converter/arg_from_python.hpp>

#include "init.h"
BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE BOOST_PYTHON_TO_PYTHON_BY_VALUE( Unit*, ::boost::python::to_python_value< UnitWrapper > () (
                                                                            UnitWrapper( x ) ) );
template < >
struct default_result_converter::apply< Unit* >
{
//typedef boost::python::to_python_value<Unit *> type;
    typedef::boost::python::default_result_converter::apply< UnitWrapper >::type type;
};

/*namespace converter{
 *
 *  template <>
 *  struct arg_rvalue_from_python <Vector>
 *  {
 *  //	typedef typename boost::add_reference<
 *  //		typename boost::add_const<Vector>::type
 *  //	>::type result_type;
 *       typedef Vector result_type;
 *
 *       arg_rvalue_from_python(PyObject*p);
 *   bool convertible() const {return true;}
 *
 *       result_type operator()(PyObject*p) {
 *               Vector vec(0,0,0);
 *               PyArg_ParseTuple(p,"fff",&vec.i,&vec.j,&vec.k);
 *               return vec;
 *       }
 *
 *  private:
 *       rvalue_from_python_data<result_type> m_data;
 *  };
 *
 *  template <>
 *  struct arg_rvalue_from_python <QVector>
 *  {
 *  //	typedef typename boost::add_reference<
 *  //		typename boost::add_const<QVector>::type
 *  //	>::type result_type;
 *       typedef QVector result_type;
 *
 *   arg_rvalue_from_python(PyObject*p);
 *   bool convertible() const {return true;}
 *
 *       result_type operator()(PyObject*p) {
 *               QVector vec(0,0,0);
 *               PyArg_ParseTuple(p,"ddd",&vec.i,&vec.j,&vec.k);
 *               return vec;
 *       }
 *
 *  private:
 *       rvalue_from_python_data<result_type> m_data;
 *  };
 *
 *  }
 */

BOOST_PYTHON_END_CONVERSION_NAMESPACE
#else
TO_PYTHON_SMART_POINTER( UnitWrapper );
TO_PYTHON_SMART_POINTER( Cargo );

PYTHON_INIT_GLOBALS( Unit, UnitWrapper )
BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE
inline PyObject*to_python( Unit*un )
{
    return to_python( UnitWrapper( un ) );
}
inline Unit * from_python( PyObject *p, boost::python::type< Unit* >)
{
    UnitWrapper uw = ( from_python( p, boost::python::type< UnitWrapper& > () ) );
    return uw.GetUnit();
}
BOOST_PYTHON_END_CONVERSION_NAMESPACE
#endif
#endif

