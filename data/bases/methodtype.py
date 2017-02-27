import sys
def methodtype(a,b,c):
	if sys.version[0:3]=='2.2':
		import new
		return new.instancemethod(a,b,c)
	else:
		import types
		return types.MethodType(a,b,c)
