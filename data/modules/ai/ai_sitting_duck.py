import VS
import sys
class MyAI(VS.PythonAI):
    def init(self,un):
#        self.XMLScript ("++turntowards.xml")
        self.AddReplaceLastOrder(1)
    def Execute(self):
        VS.PythonAI.Execute(self);
#        sys.stdout.write('h')
        return ''
hi1 = MyAI()
print 'AI creation successful'
hi1 = 0
#: 1.7; previous revision: 1.6
