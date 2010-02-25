#ifndef __CS_BOOSTPYTHON_H__
#define __CS_BOOSTPYTHON_H__

// *** This file added by chuck_starchaser (dan_w) ***
// Purpose is to get rid of redefinition warnings issuing
// from boost/python.hpp when  /usr/include/python2.4/pyconfig.h
// has been included already (by #including <Python.h>.
// Files that had #include <boost/python.hpp> now have
// #include "cs_boostpython.h", which undefines the troublesome
// symbols and then #includes <boost/python.hpp>.

#ifndef PYTHON_DWA2002810_HPP

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#include <boost/python.hpp>

#endif

#endif

