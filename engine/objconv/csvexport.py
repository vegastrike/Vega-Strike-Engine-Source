#!/usr/bin/env python
##
# csvexport.py
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Project creator: Daniel Horn
# Original development team: As listed in the AUTHORS file. Specifically: geoscope
# Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
#
#
# https://github.com/vegastrike/Vega-Strike-Engine-Source
#
# This file is part of Vega Strike.
#
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
#

# csvexport.py
# Rewritten by: geoscope
# This file reads units.csv and prints out a more easily human readable entry for units.
# Program should work standalone, or as a module.
#	csvexport.CsvExport
# Files generated: now use the key field for file name generation, not the command line arguement.
# TODO: commandline option -all, to export all entries to <key>.csv files.
#	Turn this file's funtions into class methods
#	Gradually moving towards combining csv.py, csvexport.py and csvimport.py into one file
#		of 3 related classes, possibly with inheritance. With a wrapper python script to access
#		the classes and their methods.
#	I am doing this in preparation for next step in a csv editor, which in turn will lead to a
#		units IDE.
#_____________________________________________________________________________________________________
from __future__ import print_function
USAGE = """USAGE: csvexport.py <filename> OPTIONS <unitname(s)>
     <filename> = <path/><units.csv>
        OPTIONS = -units		to display units of measurment in the output, default does not
                  -toscreen		writes output to screen, default prints to "<unitname>.csv"
     <unitname> = llama goddard		to print information on llama and goddard"""
#_____________________________________________________________________________________________________
import sys, os, csv # the Vegastrike csv.py module, not the standard python csv module.
# It should be in the same directory as csvexport.py and csvimport.py


def CsvExport(path, args):
    global showunits, ToScreen
    showunits=0
    ToScreen=0
#    if len(sys.argv) < 3:
#	return usageError(sys.argv[0])
#    if (sys.argv[1].find('.csv') != -1): # find returns -1 if string NOT found, -1 == bool True
#        path=sys.argv[1]
#        args=sys.argv[2:]
#    else:
#        return usageError(sys.argv[0])

    for i in args:	# easy to add OPTIONS, if you do, please add to USAGE information as well
        if (i=="-units"):
	    showunits=1
	if (i=="-toscreen"):
	    ToScreen=1

    workfile = file(path, "r")
    KeyList = csv.semiColonSeparatedList(workfile.readline().strip())
    GuideList = csv.semiColonSeparatedList(workfile.readline().strip())
    Lines = workfile.readlines()
    workfile.close()

    KeyLL = len(KeyList)
    GuideLL = len(GuideList)
    numLine = 2		# we already read 2 lines from the file
    for line in Lines:
	numLine += 1
	for arg in args:
	    if (csv.earlyStrCmp(line,arg)): # Used to look up unique key entry.
		# It isn't strictly necessary for looking up Ex, llama.blank
		# But makes looking up llama, find only the milspec version
                line = line.strip()
                row = csv.semiColonSeparatedList(line)
                rowLL = len(row)
	        Entry = row[0]
	        #if (rowLL!=KeyLL or rowLL!=GuideLL):
	        #    print "Mismatch in row length", rowLL, "for", row[0], "with key len =", KeyLL, "and guide len =", GuideLL
	        #    print "Contains only", rowLL, "fields"
	        #    break
	        # The above block should work, rarely, to catch errors... and next line should be elif rowLL:
	        # It fails miserably because most lines in units.csv do not have the same number of fields.
                #if rowLL:	# bool False, if row list length is zero... an empty line in file, this suppresses blank line results.
	        print("Line:", numLine, "Columns:", rowLL, "Entry:", Entry)
		if ToScreen:
		    for i in range(rowLL):
	                if GuideList[i].find('{')!=-1:
	                    print(csv.writeList([KeyList[i]]+ProcessList(GuideList[i],row[i])))
	                elif GuideList[i].find(';')!=-1:
	                    print(csv.writeList([KeyList[i]]+ProcessStruct(GuideList[i],row[i])))
	                else:
	                    print(csv.writeList([makeName(KeyList[i],GuideList[i]), row[i]]))
                else:
		    outfile = file(Entry+".csv", "w")
		    for i in range(rowLL):
	                if GuideList[i].find('{')!=-1:
	                    outfile.write(csv.writeList([KeyList[i]]+ProcessList(GuideList[i],row[i])))
	                elif GuideList[i].find(';')!=-1:
	                    outfile.write(csv.writeList([KeyList[i]]+ProcessStruct(GuideList[i],row[i])))
	                else:
	                    outfile.write(csv.writeList([makeName(KeyList[i],GuideList[i]), row[i]]))
    #print
    #print "Number of Keys:", KeyLL, "\tNumber of Guides:", GuideLL
    #print """By strickest definition of CSV, Every record should have the same number
    #of Columns (commas) as above. units.csv does not. The old csvexport.py would have
    #failed most of these records."""

def usageError(Module):
	print()
	print(Module, USAGE)
	print()
	return

def filterParen(l):
	global showunits
	if (showunits):
		return l
	where=l.find("(");
	if (where!=-1):
		return l[0:where]
	return l;

def interleave (l1,l2,add1,add2):
	ret=[]
	for i in range(len(l1)):
		ret.append(add1+filterParen(l1[i])+add2);
		ret.append(l2[i]);
	return ret

def makeName(nam,guide):
	global showunits
	if (showunits):
		return nam+'('+guide+')'
	return nam

def ProcessStruct (guide,struc):
	if (len(struc)==0):
		for i in range(guide.count(";")):
			struc+=";"
	l=struc.split(';')
	g=guide.split(';')
	return interleave(g,l," ",'=');

def ProcessList(guide,row):
	og=guide.find('{');
	cg=guide.find('}');
	if (og==-1 or cg== -1):
		print("error in "+str(row)+" "+str(guide))
		return ""
	guide=guide[og+1:cg]
	ret=[]
	while(1):
		_or=row.find('{');
		_cr=row.find('}');
		if (_or==-1 or _cr==-1):
			break;
		ret+=['{']
		ret+=ProcessStruct(guide,row[_or+1:_cr])
		row=row[_cr+1:]
		ret+=['}']
	ret+=['{']
	ret+=ProcessStruct(guide,"")
	ret+=['}']
	return ret

if __name__ == "__main__":
    if len(sys.argv) < 3:			# if errorchecking is not done here, the errors aren't
        usageError(sys.argv[0])			#    handled properly when it is used stand-alone
    elif (sys.argv[1].find('.csv') != -1):	# find returns -1 if string NOT found, -1 == bool True
        units=sys.argv[1]			# main csv file to modify
        unitList=sys.argv[2:]			# List of csv record file(s) used to update main csv
        CsvExport(units, unitList)
    else:
        usageError(sys.argv[0])

