#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include "python/script_errors.h"

namespace PythonScripts {

bool StrictScriptExceptions () {
	static int strict=-1;
	if (strict<0) {
		const char *s=getenv("VS_STRICT_PYTHON");
		strict=(s&&s[0]&&std::string(s)!="0")?1:0;
	}
	return strict==1;
}

static std::string ObjectText (PyObject *o) {
	std::string ret;
	if (!o)
		return ret;
	PyObject *s=PyObject_Str(o);
	if (s) {
		const char *c=PyUnicode_AsUTF8(s);
		if (c)
			ret=c;
		Py_DECREF(s);
	}
	PyErr_Clear();
	return ret;
}

bool ReportScriptException (const char *where) {
	if (!PyErr_Occurred())
		return StrictScriptExceptions();
	static std::map<std::string, unsigned long> seen;
	PyObject *type=NULL, *value=NULL, *tb=NULL;
	PyErr_Fetch(&type,&value,&tb);
	std::string key=std::string(where)+": "+ObjectText(type)+": "+ObjectText(value);
	unsigned long n=++seen[key];
	bool strict=StrictScriptExceptions();
	if (n<=3||strict) {
		fprintf(stderr,"CRITICAL: Python exception in %s%s\n",where,
			strict?"":" (the script was skipped; set VS_STRICT_PYTHON=1 to stop on errors)");
		PyErr_Restore(type,value,tb);
		PyErr_Print();
	} else {
		if ((n&(n-1))==0) // 4, 8, 16... occurrences
			fprintf(stderr,"CRITICAL: Python exception in %s repeated %lu times: %s\n",where,n,key.c_str());
		Py_XDECREF(type);
		Py_XDECREF(value);
		Py_XDECREF(tb);
	}
	PyErr_Clear();
	fflush(stderr);
	fflush(stdout);
	return strict;
}

}
