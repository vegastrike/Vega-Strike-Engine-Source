import XGUIDebug


XGUITypesRootSingleton = None

"""---------------------------------------------------------------------------"""
"""                                                                           """
""" XGUITypesRoot - root management interface for the XML-GUI-Types framework."""
"""                                                                           """
"""---------------------------------------------------------------------------"""


class XGUITypesRoot:
	def __init__(self):
		self.factories = {}

	def _parseArrayTypeDef(self,type):
		type = type.strip()
		(s,e) = (type.find("["),type.rfind("]"))
		if (s==-1) or (e==-1) or (e<s):
			return (type,0)
		elif (e==s+1):
			return (type,-1)
		elif type[s+1:e].isdigit():
			return (type[:s],int(type[s+1:e]))
		else:
			return (type,0)
			

	def getFactory(self,type):
		(type,arraysize) = self._parseArrayTypeDef(type)
		if type in self.factories:
			return self.factories[type]
		else:
			return None

	def addFactory(self,type,fac):
		self.factories[type]=fac

	def enumFactories(self):
		ids = {}
		rv = []
		for t in self.factories:
			if not id(self.factories[t]) in ids:
				ids[id(self.factories[t])] = 1
				rv.append(self.factories[t])
		return rv


"""-----------------------------"""
"""                             """
"""  Standard factories         """
"""                             """
"""-----------------------------"""

class XGUITypesFactory:
	def __init__(self,types,desc):
		self.types = types
		self.desc = desc

	def registerSelf(self):
		for type in self.types:
			XGUITypesRootSingleton.addFactory(type,self)

	def getTypes(self):
		return self.types

	def getDesc(self):
		return self.desc


class XGUITypesBuiltinFactory(XGUITypesFactory):
	def __init__(self):
		types = ( 
			"bool", "bool1", "bool2", "bool3", "bool4",
			"int", "int1", "int2", "int3", "int4",
			"float", "float1", "float2", "float3", "float4",
			"number", "number1", "number2", "number3", "number4",
			"string" 
			)
		desc = "Built-in type factory"
		XGUITypesFactory.__init__(self,types,desc)
		self.boolconv = { "TRUE":1, "1":1, "FALSE":0, "0":0, "ENABLE":1, "DISABLE":0, "ON":1, "OFF":0 }

	def parse(self,type,value):
		_type = type
		(type,arraysize) = XGUITypesRootSingleton._parseArrayTypeDef(type)
		if type in self.types:
			if arraysize == 0:	

				""" Extract vector length - "vector" defaults to 3 """
				if type[-1:].isdigit():
					(type,vlen) = (type[:-1],int(type[-1:]))
				else:
					if type == "vector":
						vlen = 3
					else:
						vlen = 0
				if type == "vector":
					type = "float"
		
				""" Now, if we have a vector, vectors use array sintax, so parse as if it were a fixed-size array """
				""" Otherwise, do a builtin conversion """
				if vlen>0:
					return tuple(self.parse("%s[%d]" % (type,vlen), value))
				else:
					if type == "float" or (type == "number" and value.find(".")>0):
						return float(value)
					elif type == "int" or type == "number":
						return int(float(value))
					elif type == "string":
						return value
					elif type == "bool":
						if value.upper() in self.boolconv:
							return self.boolconv[value.upper()]
						else:
							XGUIDebug.trace(0,"XGUITypes: ERROR! String unrecognizable as a boolean expression: \"%s\"\n" % (value))
							return None
					else:
						XGUIDebug.trace(-1,"XGUITypes: ERROR! supposedly unreachable code reached - veeery bad.\n")
						return None

			else:

				""" Recursive parsing """
				(s,e) = (value.find("("),value.rfind(")"))
				if (s!=-1) and (e!=-1) and (e>s):
					value = value[s+1:e].split(',')
					rv = []
					for val in value:
						rv.append( self.parse(type,val) )
					if arraysize==-1 or len(rv) == arraysize:
						return rv
					else:
						XGUIDebug.trace(0,"XGUITypes: ERROR! Type mismatch. Expected %s[%d], got %s[%d]\n\n" % (type,arraysize,type,len(rv)))
				else:
					XGUIDebug.trace(0,"XGUITypes: ERROR! Unable to parse data string\n\tType: %s\n\tValue: \"%s\"\n" % (_type,value))
					return None
				
		else:
			XGUIDebug.trace(0,"XGUITypes: ERROR! Unrecognized type " + str(type))
			return None


class XGUITypesTupleFactory(XGUITypesFactory):
	def __init__(self,form,types,desc):
		XGUITypesFactory.__init__(self,types,desc)

		form = form.strip()
		self.form = form

		""" compile form """
		(s,e) = (form.find("("),form.rfind(")"))
		if (s!=0) or (e!=len(form)-1):
			XGUIDebug.trace(-1,"XGUITypes: ERROR! TupleFactory initialized with invalid form: \"%s\""%(form))
			return

		cform = form[s+1:e]
		self.cform = form[s+1:e].split(",")

	def parse(self,type,value):
		_type = type
		(type,arraysize) = XGUITypesRootSingleton._parseArrayTypeDef(type)
		if type in self.types:
			factoryList = []
			for t in self.cform:
				factoryList.append(XGUITypesRootSingleton.getFactory(t))

			rva = []
			rvv = []
			recurse = 0
			pos = value.find("(")
			s = pos+1
			while pos>=0:
				if recurse>0:
					pos = value.find(")",pos+1)
				elif recurse<0:
					pos = value.find("(",pos+1)
					s = pos+1
				else:
					(np,nc,nc2) = ( value.find("(",pos+1) , value.find(",",pos+1) , value.find(")",pos+1) )
					if nc<0 or (nc2>=0 and nc2<nc): nc=nc2
					if np>=0 and np<nc:
						pos = np
						recurse += 1
					elif nc>=0:
						if len(rvv)>=len(self.cform):
							XGUIDebug.trace(0,"XGUITypes: ERROR! Type mistmatch. Too many arguments for expected form %s in \"%s\""%(self.form,value))
						else:
							if factoryList[len(rvv)]:
								rvv.append( factoryList[len(rvv)].parse(self.cform[len(rvv)],value[s:nc]) )
							else:
								XGUIDebug.trace(-1,"XGUITypes: ERROR! TupleFactory, element type has no associated factory, in form %s"%(self.form))
							if value[nc]==")": 
								rva.append(tuple(rvv))
								recurse -= 1
						pos = nc
						s = pos+1
					else:
						print "Premature EOS"
						pos = -1

			if arraysize == 0:
				if len(rva)==1:
					return rva[0]
				else:
					XGUIDebug.trace(0,"XGUITypes: ERROR! Type mistmatch. Expected %s, got %s[%d]\n"%(type,type,len(rva)))
			elif arraysize<0 or arraysize == len(rva):
				return rva
			else:
				XGUIDebug.trace(0,"XGUITypes: ERROR! Type mistmatch. Expected %s[%d], got %s[%d]\n"%(type,arraysize,type,len(rva)))
				
		else:
			XGUIDebug.trace(0,"XGUITypes: ERROR! Unrecognized type " + str(type))
			return None




"""----------------------------------------------------------------"""
"""                                                                """
"""  XGUITypes global initialization                               """
"""                                                                """
"""----------------------------------------------------------------"""

def XGUITypesInit():
	global XGUITypesRootSingleton

	XGUITypesRootSingleton = XGUITypesRoot()
	builtinFactory = XGUITypesBuiltinFactory()
	builtinFactory.registerSelf()

def XGUITypesTest():
	XGUITypesInit()
	XGUITypesTupleFactory("(int,string)",["int-string-tuple"],"int-string Tuple Factory").registerSelf()
	XGUITypesTupleFactory("(int,int,float,float)",["i2f2"],"i2f2 Tuple Factory").registerSelf()

	for t in XGUITypesRootSingleton.enumFactories():
		print t.getDesc()
	
	print XGUITypesRootSingleton.getFactory("int4").parse("int4","(3.3,3.4,4,4.6)")
	print XGUITypesRootSingleton.getFactory("float4").parse("float4","(3.3,3.4,4,4.6)")
	print XGUITypesRootSingleton.getFactory("int[4]").parse("int[4]","(3.3,3.4,4,4.6)")
	print XGUITypesRootSingleton.getFactory("float[4]").parse("float[4]","(3.3,3.4,4,4.6)")
	print XGUITypesRootSingleton.getFactory("i2f2").parse("i2f2","(3.3,3.4,4,4.6)")

