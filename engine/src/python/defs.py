# Standard definitions for Vegastrike python scripts

# Callable wrapper for class methods
class Callable:
	def __init__(self,anycallable):
		self.__call__ = anycallable


