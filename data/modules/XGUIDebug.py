TRACE_BIGDEBUG = 10
TRACE_DEBUG = 5
TRACE_VERBOSE = 3
TRACE_INFO = 1
TRACE_NORMAL = 0
TRACE_WARNING = -1
TRACE_ERROR = -5

__xgui_trace_level = TRACE_NORMAL

def trace(level,msg):
	global __xgui_trace_level
	if __xgui_trace_level>=level:
		print msg

