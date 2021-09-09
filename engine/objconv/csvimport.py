#!/usr/bin/env python

##
# csvimport.py
#
# Copyright (c) 2001-2002 Daniel Horn
# Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
# Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
#
# https://github.com/vegastrike/Vega-Strike-Engine-Source
#
# This file is part of Vega Strike.
#
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
#

# csvimport.py
# Rewritten by: geoscope
# This file opens units.csv (or another similar file), and a <unitname>.csv file (created most likely with csvexport.py
# it looks within units.csv for a line with the <unitname> key, and if found, updates that line to whatever information
# is is the <unitname>.csv file.
# Caveats: This script ONLY changes existing records, it does NOT add records.
#		This makes sense, since placing the new unit in a specific record location would be problematic.
#		New units could only be appended to the end of units.csv, which is not a good way to keep it organized.
#		TODO: Can this be fixed, thus making adding and inserting new units feasible?
#	   If units.csv does not already contain a key == <unitname>, then the information is not changed in units.csv
#		This uses the commandline <unitname> not the key within the file...
#		This makes changing the key itself possible, if the key in <unitfile>.csv is different.
#	   But if <unitname>.csv has a key that is different than the name of <unitname>, the key in units.csv
#		is changed and <unitname>.csv will no longer work to update units.csv, unless it is renamed as well.
#		Which it now is. 16 Aug 2006
# TODO: Command-line option -all, to import all <filename>.csv files in a directory into units.csv.
#	Turn this file's funtions into class methods
#	Gradually moving towards combining csv.py, csvexport.py and csvimport.py into one file
#		of 3 related classes, possibly with inheritance. With a wrapper python script to access
#		the classes and their methods.
#	I am doing this in preparation for next step in a csv editor, which in turn will lead to a
#		units IDE.
#_____________________________________________________________________________________________________
from __future__ import print_function
USAGE = """USAGE: csvimport.py <filename> <unitname(s)>
     <filename> = <path/><units.csv>
     <unitname> = llama goddard		to import changed llama and goddard"""
#_____________________________________________________________________________________________________
import sys, os, csv # the Vegastrike csv.py module, not the standard python csv module.
# It should be in the same directory as csvexport.py and csvimport.py

def CsvImport(units, unitList):
#        if len(sys.argv) < 3:			# if errorchecking is not done here, the errors aren't
#            return usageError(sys.argv[0])	#	handled when it is used as a module
#        if (sys.argv[1].find('.csv') != -1):	# find returns -1 if string NOT found, -1 == bool True
#            units = sys.argv[1]			# main csv file to modify
#            unitList = sys.argv[2:]		# List of csv record file(s) used to update main csv
#        else:
#            return usageError(sys.argv[0])

        ModFile = file(units, "r")		# open file for read, units.csv
        Lines = ModFile.readlines()		# Read List of all Lines into memory
        ModFile.close()
        key = csv.semiColonSeparatedList(Lines[0],',')	# string into list for the first line Keys

        for unit in unitList:
            try:				# Catch file IOError, if the <unitname>.csv doesn't exist
                Unit = unit+".csv"
                UnitFile=file(Unit, "r")	# open csv <unitname> for read
            except IOError:
                fileError(Unit)
                break

            for numline in range(0,len(Lines)):	# have to index the line to change Lines[numline], not line in Lines that exists only in this block
                line = Lines[numline]
                if (csv.earlyStrCmp(line,unit)):		# look for unit key in the line and if found
                    line = csv.writeList(reorderList(key,ProcessUnit(UnitFile)))
                    UnitFile.close()
                    Lines[numline] = line
                    # We also need to check that the unit record being modified didn't change the key. If it did,
                    # We will need to rename the <unitname>.csv to reflect this, so it remains usable by csvimport later.
                    if not csv.earlyStrCmp(line, unit):			# key for record was changed
                        NewName = line[:line.find(",")]+".csv"		# save the new name.
                        os.rename(Unit, NewName)
        OutFile = file(units, "w")		# All changes have been made, time to write out the file
        for line in Lines:
            OutFile.write(line)
        OutFile.close()
        return

def usageError(Module):
	print()
	print(Module, USAGE)
	print()
	return

def fileError(File):
        print()
        print(File, ":File does not exit")
        print()
        return

def CollapseStruct(s):
        l=""
        for i in range(0,len(s),2):
            if i!=0:
                l+=";"
            l+=s[i];
        return l
		
def removeBraces(s):
        r=""
        s=s.split("{");
        for k in s:
            r+=k
        r=r.split("}")
        s=""
        for k in r:
            s+=k
        return s
	
def removeParen(s):
        o=s.find("(");
        c=s.find(")")
        if (o!=-1 and c!=-1):
            return s[0:o]
        return s

def CollapseList(lis):
        s=""
        r=[]
        i=0
        l=[]
        for i in lis:
            op=0
            clo=0
            guide=0
            if (i.find("{")!=-1):
                op=1
            if (i.find("}")!=-1):
                clo=1
            if (i.find("=")!=-1):
                guide=1
            if (op+clo+guide>1):
                if (clo):
                    l.append("}")
                if (op):
                    l.append("{")
                if(guide):
                    l.append(removeBraces(i))
            else:
                l.append(i)
        for i in range(0,len(l),2):
            if l[i]=='{':
                r.append([])
            else:
                r[len(r)-1].append(l[i])
        found=0
        for i in r[len(r)-1]:
            if i!="":
                found=1
        if (0==found):
            r=r[0:len(r)-1]
        for i in r:
            s+='{'
            for j in range(len(i)):
                if (j!=0):
                    s+=";"
                s+=i[j]
            s+='}'
        return s;

def ProcessUnit(f):
        l=[]
        m=[]
        line=f.readline()
        while (line!=""):
            x=csv.semiColonSeparatedList(line,',')
            if (len(x)>=1):
                m.append(removeParen(x[0]))
                if (len(x)>=2):
                    if (len(x)==2):
                        l.append(x[1])
                    elif (x[1].find('{')!=-1):
                        l.append(CollapseList(x[1:]))
                    else:
                        l.append(CollapseStruct(x[2:]))
                else:
                    l.append('')
            line=f.readline()
        return (m,l)

def reorderList(keys,keylist):
        h={}
        ret=[]
        for i in range(len(keylist[0])):
            h[keylist[0][i]]=keylist[1][i]
        for i in keys:
            if i in h:
                ret.append(h[i])
            else:
                ret.append("");
        return ret

if __name__ == "__main__":
    if len(sys.argv) < 3:			# if errorchecking is not done here, the errors aren't
        usageError(sys.argv[0])			#    handled properly when it is used stand-alone
    elif (sys.argv[1].find('.csv') != -1):	# find returns -1 if string NOT found, -1 == bool True
        units=sys.argv[1]			# main csv file to modify
        unitList=sys.argv[2:]			# List of csv record file(s) used to update main csv
        CsvImport(units, unitList)
    else:
        usageError(sys.argv[0])
    
