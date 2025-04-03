/*
 * unit_from_to_python.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Benjamen R. Meyer, and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VEGA_STRIKE_ENGINE_PYTHON_UNIT_FROM_TO_PYTHON_H
#define VEGA_STRIKE_ENGINE_PYTHON_UNIT_FROM_TO_PYTHON_H

#define PY_SSIZE_T_CLEAN
#include <boost/version.hpp>
#include <boost/python.hpp>

//#include <boost/python/converter/arg_from_python.hpp>

#include "init.h"
BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE BOOST_PYTHON_TO_PYTHON_BY_VALUE(Unit*,
        ::boost::python::to_python_value<UnitWrapper>()(
                UnitWrapper(x)));
template<>
struct default_result_converter::apply<Unit *> {
//typedef boost::python::to_python_value<Unit *> type;
    typedef ::boost::python::default_result_converter::apply<UnitWrapper>::type type;
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

#endif //VEGA_STRIKE_ENGINE_PYTHON_UNIT_FROM_TO_PYTHON_H
