import sys
import os
def mymangle(s):
	
	if (len(s)>3):
		return s[0:2]+s[2].upper()+s[3].upper()+s[4:]
	else:
		return s[0:len(s)-1]+s[len(s)-1].upper()
for i in sys.argv[1:]:
	j=mymangle(i);
	os.system ("./replace "+i+" "+j+" "+sys.argv[1]+" "+sys.argv[1]);
