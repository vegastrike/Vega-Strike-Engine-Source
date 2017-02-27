#!/usr/bin/python
import os
import sys
index=0
fp=0
def makeWebPage(fp,index):
	if (fp):
		fp.write("\n</body>\n</html>\n");
		fp.close();
	fp = open (str(index/10)+".html","w")
	fp.write('<html>\n<head>\n<title>Pictures from Hong Kong</title>\n</head>\n<body vlink="#001e73" link="#001e73" bgcolor="#ffffff" text="#000000">\n');
	return fp

for args in sys.argv:
	print (args)	
	if (index%20==0):
		fp = makeWebPage(fp,index);
	index+=1
	if (index==1):
		continue
	os.system("xview -zoom 50 "+args+" &")
	first = sys.stdin.readline()
	if (first=="x\n"):
		continue
	second= sys.stdin.readline()
	fp.write('<br><br><b>\n'+first+'</b>')
	fp.write('<br><img src="'+args+'" width="1024"/>')
	fp.write("<br>"+second+"<br>")

