import traceback
import sys

debugnum=0

class VSException(Exception):
	pass


def _devnull(msg, *fmtargs): # == /dev/null
	pass

def prettyfile(fil):
	lasttwo=str(fil).split('/')[-2:]
	if len(lasttwo)<2: return fil
	return '%s/%s' % (lasttwo[0][0],lasttwo[1])

def _withlineno(msg, *fmtargs): # Simple line number
	laststack = traceback.extract_stack()[-2]
	print ' +++ %s:%s %s' % (prettyfile(laststack[0]),laststack[1],str(msg) % fmtargs)


def _warn(msg, *fmtargs): # Traceback without killing the script
	global debugnum
	debugnum+=1
	print " *** Python Warning %s!" % (debugnum,)
	sys.stderr.write('Warning Traceback %s:\n' % (debugnum,))
	for frame in traceback.extract_stack()[:-1]:
		sys.stderr.write('  File "%s", line %s' % (prettyfile(frame[0]), frame[1]))
		sys.stderr.write(', in %s\n    %s\n' % (frame[2],frame[3]))
	sys.stderr.write('Message: %s\n\n' % (msg % fmtargs,))

def _fatal(msg, *fmtargs): # Kill the script!
	global debugnum
	debugnum+=1
	print "Python VSException %s!" % (debugnum,)
	raise VSException(msg)

fatal = _fatal # Really bad error... Kill the script.  Same as a call to raise()

warn = _warn   # Anything that shouldn't happen, but shouldn't cause a crash either.
error = _warn  # Different name for the same thing.

# Less important messages that happen a lot.
debug = _withlineno # Useful messages for hunting down bugs, or loading status.
info = _warn

# For release, we can disable unimportant messages:
info = _devnull
debug = _devnull


