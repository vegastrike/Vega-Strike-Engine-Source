import Base
import VS
import GUI
import XGUITypes
import XGUIDebug


XGUIRootSingleton = None
XGUIPythonScriptAPISingleton = None

"""----------------------------------------------------------------"""
"""                                                                """
""" XGUIRoot - root management interface for the XML-GUI framework."""
"""                                                                """
"""----------------------------------------------------------------"""


class XGUIRoot:
	def __init__(self):
		self.templates = {}

	def getTemplate(self,type,name):
		if type in self.templates and name in sefl.templates[type]:
			return self.templates[type][name]
		else:
			return None

	def addTemplate(self,tpl):
		type = tpl.getType()
		name = tpl.getName()
		if not type in self.templates:
			XGUIDebug.trace(1,"XGUI: Initializing template category \"" + str(type) + "\"\n")
			self.templates[type] = {}
		XGUIDebug.trace(2,"XGUI: Loading template \"" + str(name) + "\" into category \"" + str(type) + "\"\n")
		self.templates[type][name] = tpl

class XGUIPythonScript:
	def __init__(self,code,filename):
		code = code.replace("\r\n","\n")
		code += "\n"
		self.code = compile(code,filename,'exec')

	def execute(self,context):
		exec self.code in context
		return context


"""----------------------------------------------------------------"""
"""                                                                """
"""  XGUIPythonScriptAPI - through this class, all PythonScript    """
"""     API calls are routed.                                      """
"""                                                                """
"""----------------------------------------------------------------"""

class XGUIPythonScriptAPI:
	def __init__(self,layout,room):
		self.layout = layout
		self.room = room



"""----------------------------------------------------------------"""
"""                                                                """
"""  XGUI global initialization                                    """
"""                                                                """
"""----------------------------------------------------------------"""

def XGUIInit():
	XGUIRootSingleton = XGUIRoot()

