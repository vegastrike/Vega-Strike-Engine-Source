#ifndef __PYTHON_SCRIPT_ERRORS_H__
#define __PYTHON_SCRIPT_ERRORS_H__

// Python exceptions raised by scripts the engine calls (mission Execute,
// Python AIs...).  By default a broken script is reported as a CRITICAL
// error in the log and the game goes on; with VS_STRICT_PYTHON=1 in the
// environment the exception propagates and ends the game (the old
// behaviour, handy when debugging the data).
namespace PythonScripts {
	// Call from a catch (boost::python::error_already_set) block.  Prints
	// the pending Python exception (the full traceback the first times a
	// given error happens, then only occasional counts) and clears it.
	// Returns true when the caller should rethrow (strict mode).
	bool ReportScriptException (const char *where);
	bool StrictScriptExceptions ();
}

// try { stmt } and report/rethrow a Python exception raised by it
#define VS_PYTHON_GUARDED(where, stmt) \
	try { stmt; } \
	catch (const boost::python::error_already_set &) { \
		if (::PythonScripts::ReportScriptException(where)) throw; \
	}

#endif
