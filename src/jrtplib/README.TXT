------------------------------------------------------------------------------
                                                            10th December 2002


                             Notes about JRTPLIB (v2.7)



LIBRARY LOCATION AND CONTACT
============================
Normally, you should be able to download the latest version of the library
from this url:
	http://lumumba.luc.ac.be/jori/jrtplib/jrtplib.html

If you have questions about the library, you can mail me at:
	jori@lumumba.luc.ac.be


DISCLAIMER & COPYRIGHT
======================

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

INSTALLATION NOTES
==================

* To compile the library on a unix-like platform, just start the configure
  script. This should generate a makefile which you can use to start
  building the library using 'make'.

* If you intend to use the library on a VxWorks system, you should run the
  configure script with option '--target=vxworks'.

* For a manual about the library, please refer to doc/manual.txt.
  A tutorial can also be found in the 'doc' directory.

* If you should want to make some manual adjustments, it's probably
  useful to know that these are the defines in use:
	WIN32
		If defined, the library will compile for use on a MS-Windows
		system, using Winsock socket functions. Otherwise, a unix-like
		environment is assumed and those socket functions will be
		used. This define used to be WINVERSION, but I changed it
		because Visual C++ automatically defines 'WIN32' in its
		projects.
	RTP_BIG_ENDIAN
		If defined, the library will compile for use on a big endian
		system, if not it will compile for a little endian system.
	RTP_SOCKLENTYPE_UINT
		If defined, the last parameter in functions 'getsockname' and
		'recvfrom' will be of type 'unsigned int *', otherwise it's an
		'int *'.
	RTP_HAVE_SYS_FILIO
		On some systems (on Solaris anyway) an extra file needs
		to be included ( sys/filio.h ). This define make sure this
		file gets included.
	RTP_MULTICAST
		This define specifies if the library should de built with
		support for multicasting.
	RTP_VXWORKS
		This define is used to compile the library on a VxWorks 
		system.
	RTPDEBUG
		If you enable this, the library will use some memory tracking
		routines to check for memory leaks. Note that this will 
		slightly alter some class definitions, so you must also enable
		this flag when you're compiling a program with this debug
		version of the library. You probably don't need this define,
		unless you're making some serious changes to the library. To
		enable it, you should just uncomment the appropriate line in
		'rtpconfig.h'
------------------------------------------------------------------------------

