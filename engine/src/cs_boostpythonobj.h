#ifndef __CS_BOOSTPYTHONOBJ_H__
#define __CS_BOOSTPYTHONOBJ_H__

// *** This file added by chuck_starchaser (dan_w) ***
// Purpose is to get rid of redefinition warnings issuing
// from boost/python.hpp when  /usr/include/python2.4/pyconfig.h
// has been included already (by #including <Python.h>.
// Files that had #include <boost/python/object.hpp> now have
// #include "cs_boostpythonobj.h", which undefines the troublesome
// symbols and then #includes <boost/python/object.hpp>.

#ifndef OBJECT_DWA2002612_HPP

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#include <boost/python/object.hpp>

#endif

#endif

