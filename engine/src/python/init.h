/*
 * init.h
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

#ifdef HAVE_PYTHON

#ifndef VEGA_STRIKE_ENGINE_PYTHON_INIT_H
#define VEGA_STRIKE_ENGINE_PYTHON_INIT_H

#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include "gfx/vec.h"
#include <boost/version.hpp>

#if BOOST_VERSION != 102800

#if defined (_MSC_VER) && _MSC_VER <= 1200
#define Vector Vactor
#endif //defined (_MSC_VER) && _MSC_VER <= 1200

#if defined (_MSC_VER) && _MSC_VER <= 1200
#undef Vector
#endif // defined (_MSC_VER) && _MSC_VER <= 1200

#else //defined (_MSC_VER) && _MSC_VER <= 1200
#include <boost/python/detail/extension_class.hpp>
#endif // defined (_MSC_VER) && _MSC_VER <= 1200

class Python {
public:
    static void init();
    static void initpaths();
    static void test();
    static void reseterrors();
};

BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE
#if BOOST_VERSION != 102800
/*
 *  typedef boost::tuples::tuple<double,double,double> python_wector;
 *  struct to_python <Vector> {
 *       inline PyObject *operator () (const Vector &vec) const{
 *               boost::python::to_python<boost::python::tuple> a();
 *               return a.convert(boost::python::make_tuple((double)vec.i,(double)vec.j,(double)vec.k));
 *       }
 *  };
 */
struct my_builtin_to_python {
    BOOST_STATIC_CONSTANT(bool, uses_registry = false);
};

#ifndef BOOST_PYTHON_TO_PYTHON_BY_VALUE

# define BOOST_PYTHON_RETURN_TO_PYTHON_BY_VALUE(T, expr) \
    template < class MYTYPE >                              \
    struct to_python_value;                                \
    template < >                                           \
    struct to_python_value< T& >:                          \
        my_builtin_to_python                               \
    {                                                      \
        inline PyObject* operator()( T const &x ) const    \
        {                                                  \
            return expr;                                   \
        }                                                  \
    };                                                     \
    template < >                                           \
    struct to_python_value< T const& >:                    \
        my_builtin_to_python                               \
    {                                                      \
        inline PyObject* operator()( T const &x ) const    \
        {                                                  \
            return expr;                                   \
        }                                                  \
    };

# define BOOST_PYTHON_ARG_TO_PYTHON_BY_VALUE(T, expr) \
    namespace converter                                 \
    {                                                   \
    template < class MYTYPE >                           \
    struct arg_to_python;                               \
    template < >                                        \
    struct arg_to_python< T >:                          \
        ::boost::python::handle< >                      \
    {                                                   \
        arg_to_python( T const&x ) :                    \
            ::boost::python::handle< > ( expr ) {}      \
    };                                                  \
    }

//Specialize argument and return value converters for T using expr
# define BOOST_PYTHON_TO_PYTHON_BY_VALUE(T, expr)   \
    BOOST_PYTHON_RETURN_TO_PYTHON_BY_VALUE( T, expr ) \
    BOOST_PYTHON_ARG_TO_PYTHON_BY_VALUE( T, expr )

#endif //BOOST_PYTHON_TO_PYTHON_BY_VALUE

BOOST_PYTHON_TO_PYTHON_BY_VALUE(Vector,
        boost::python::to_python_value<boost::python::tuple>()(boost::python::make_tuple((double)
                        x.i,
                (double)
                        x.j,
                (double)
                        x.k)));
BOOST_PYTHON_TO_PYTHON_BY_VALUE(QVector,
        boost::python::to_python_value<boost::python::tuple>()(boost::python::make_tuple((double)
                        x.i,
                (double)
                        x.j,
                (double)
                        x.k)));
#else //BOOST_VERSION != 102800
inline PyObject * to_python( Vector vec )
{
    return to_python( boost::python::tuple( (double) vec.i, (double) vec.j, (double) vec.k ) );
}
inline PyObject * to_python( QVector vec )
{
    return to_python( boost::python::tuple( (double) vec.i, (double) vec.j, (double) vec.k ) );
}

inline Vector from_python( PyObject *p, boost::python::type< Vector >)
{
    Vector vec( 0, 0, 0 );
    PyArg_ParseTuple( p, "fff", &vec.i, &vec.j, &vec.k );
    return vec;
}
inline QVector from_python( PyObject *p, boost::python::type< QVector >)
{
    QVector vec( 0, 0, 0 );
    PyArg_ParseTuple( p, "ddd", &vec.i, &vec.j, &vec.k );
    return vec;
}
#endif //BOOST_VERSION != 102800

BOOST_PYTHON_END_CONVERSION_NAMESPACE

void InitBriefing();
void InitBriefing2();
void InitDirector();
void InitDirector2();
void InitVS();
void InitVS2();
void InitBase();
void InitBase2();

#endif //VEGA_STRIKE_ENGINE_PYTHON_INIT_H

#endif //HAVE_PYTHON
