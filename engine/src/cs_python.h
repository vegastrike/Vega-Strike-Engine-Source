#ifndef __CS_PYTHON_H__
#define __CS_PYTHON_H__

// *** This file added by chuck_starchaser (dan_w) ***
// Purpose is to get rid of redefinition warnings issuing
// from /usr/include/python2.4/pyconfig.h when
// boost/python has been included already
// Files that had #include <Python.h> now have #include "cs_python.h"
// which undefines the troublesome symbols and then includes Python.h

#ifndef Py_PYTHON_H

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings" 
#endif
#include <boost/python/detail/wrap_python.hpp>
//#include <Python.h>
#ifdef __GNUC__
#pragma GCC diagnostic error "-Wwrite-strings" 
#endif

#endif

#endif

