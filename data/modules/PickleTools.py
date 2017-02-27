# -*- coding: utf-8 -*-
from types import *

def _bytehex2(num, ord=ord, chr=chr, ord_0=ord('0'), ord_a=ord('a')):
	if num<10:
		return chr(num+ord_0)
	else:
		return chr(num+ord_a-10)

def _bytehex(num, int=int, divmod=divmod, bytehex2=_bytehex2):
	q,r=divmod(int(num),16)		
	return bytehex2(q%16)+bytehex2(r)		

bytehex = map(_bytehex, xrange(256))

# '\\' is always forbidden
def addSlashes(m,forbidden="#!|\\?\"\'\r\n",extended_forbidden=1, bytehex=bytehex):
	rv = []
	rva = rv.append
	for i, m_i in enumerate(m):
		quote =    (extended_forbidden and ord(m_i)>=128) \
		        or (m_i == '\\') \
		        or (m_i in forbidden)
		if quote:
			if not rv and i: rva( m[:i] )
			rva( "\\" ) 
			rva( bytehex[ord(m_i)%256] )
		elif rv:
			rva( m_i )
	return rv and "".join(rv) or m

def _hexbyte2(c, ord=ord, ord_0=ord('0'), ord_9=ord('9'), ord_a=ord('a')):
	ord_c = ord(c)
	if ( ord_c>=ord_0 and ord_c<=ord_9 ):
		return ord_c - ord_0
	else:
		return 10 + ord_c - ord_a

def _hexbyte(s, hexbyte2=_hexbyte2):
	return ( hexbyte2(s[0])*16+hexbyte2(s[1]) ) % 256

def stripSlashes(m, hexbyte=_hexbyte, chr=chr):
	if '\\' not in m:
		return m
	rv = []
	rva = rv.append
	i = 0
	l = len(m)
	while (i<l):
		if (m[i]=='\\') and (i+2<l):
			rva( chr( hexbyte(m[i+1:i+3]) ) )
			i += 3
		else:
			rva( m[i] )
			i += 1
	return "".join(rv)

def encodeMap(m, str=str):
	if type(m) is DictionaryType:
		rv = []
		rva = rv.append
		_addSlashes = addSlashes
		_encodeMap = encodeMap
		for k,v in m.iteritems():
			#recursive, in case there are nested maps
			rva( _addSlashes(str(k)) + "#" + _encodeMap(v) )
		rv = "|".join(rv)
		del rva
	else:
		rv = addSlashes(str(m))
	return addSlashes(rv)

def decodeMap(m, len=len):
	m = stripSlashes(m)
	ilist = m.split('|')
	if len(ilist)==1:
		ipair = ilist[0].split('#')
		if len(ipair)==1:
			return stripSlashes(ipair[0])
		elif len(ipair)>=2:
			return { stripSlashes(ipair[0]) : decodeMap(ipair[1]) } 
		else:
			return ''
	else:
		rv = {}
		_stripSlashes = stripSlashes
		_decodeMap = decodeMap
		for ipair in ilist:
			ipair = ipair.split('#')
			if len(ipair)>=2:
				rv[_stripSlashes(ipair[0])] = decodeMap(ipair[1])
		return rv
