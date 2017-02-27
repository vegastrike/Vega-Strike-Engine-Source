import VS
import sys
class MyAI(VS.CommAI):
    def Execute(self):
        sys.stdout.write('MyAI\\n')
        return ''
