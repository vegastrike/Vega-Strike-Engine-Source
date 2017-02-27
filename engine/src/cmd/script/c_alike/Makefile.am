noinst_LIBRARIES = libc_alike.a


libc_alike_a_SOURCES =				\
	c_alike.tab.cpp		\
	lex.yy.cpp

AM_CXXFLAGS = -DBISON

c_alike.tab.cpp: c_alike.yacc
#	bison -d -o c_alike.tab.cpp c_alike.yacc

lex.yy.cpp: c_alike.lex
	flex -o lex.yy.cpp c_alike.lex 
