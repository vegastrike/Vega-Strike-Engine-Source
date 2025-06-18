#!/usr/bin/python
##
# configparser.py
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Project creator: Daniel Horn
# Original development team: As listed in the AUTHORS file. Specifically: Daniel Aleksandrow
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

from __future__ import print_function
import sys
import os
import fnmatch
import string
from xml.dom.minidom import *

global varlines
varlines = list()

global bindlines
bindlines = list()

global templine
templine = tuple()

global config
config = Document()


#----------------
# Parsing functions
#----------------

def add(filelist, dirname, filenames):
    """Adds any .cpp files in the directory to filelist."""
    filenames = fnmatch.filter(filenames, '*.cpp')
    filelist.extend([os.path.join(dirname, filename) for filename in filenames])

def hasPreProcessor(line, filename):
    commands = ["ifdef", "if", "else", "endif"]
    for command in commands:
        if line.find("#"+command) >= 0:
            return True
    return False

def parseLine(line, filename):
    global templine
    if templine.__len__() == 2:#if this line continues on a getVariable
        if filename != templine[1]:
            print("Incomplete reference in file "+filename[1])
            print(filename[0])
        else:
            line = templine[0]+line
        templine = tuple()
    while 1:
        ind = line.find("getVariable")
        if ind == -1:
            break
        if hasPreProcessor(line, filename):#Don't want to include mixed stuff
            return
        line = line[ind:]
        start = line.find("(")
        if start == -1:
            print("No opening bracket found: "+line +"("+filename+")")
            return
        finish = False
        count = 0
        for i in range(len(line[start:])):
            ch = line[start:][i]
            if ch is '(':
                count+=1
            if ch is ')':
                count-=1
            if count == 0:
                finish = start+i
                break
        if not finish:#no closed set of brackets found
            global templine
            templine = (line, filename)#Try multiline statement
            return
        parsed = line[start+1:finish]
        parsed = makeList(parsed)
        if parsed:
            global varlines
            parsed.append(filename)
            varlines.append(parsed)
        line=line[finish+1:]
    return

def parseColorLine(line, filename):
    global templine
    if templine.__len__() == 2:#if this line continues on a getVariable
        if filename != templine[1]:
            print("Incomplete color in file "+filename[1])
            print(filename[0])
        else:
            line = templine[0]+line
        templine = tuple()
    while 1:
        ind = line.find("getColor")
        if ind == -1:
            break
        if hasPreProcessor(line, filename):#Don't want to include mixed stuff
            return
        line = line[ind:]
        start = line.find("(")
        if start == -1:
            print("No opening bracket found: "+line +"("+filename+")")
            return
        finish = False
        count = 0
        for i in range(len(line[start:])):
            ch = line[start:][i]
            if ch is '(':
                count+=1
            if ch is ')':
                count-=1
            if count == 0:
                finish = start+i
                break
        if not finish:#no closed set of brackets found
            global templine
            templine = (line, filename)#Try multiline statement
            return
        parsed = line[start+1:finish]
        parsed = makeColorList(parsed)
        print(parsed)
        if parsed:
            global varlines
            parsed.append(filename)
            varlines.append(parsed)
        line=line[finish+1:]
    return

def parseKeyBindingLine(line, filename):
    global templine
    if templine.__len__() == 2:#if this line continues on a getVariable
        if filename != templine[1]:
            print("Incomplete keybinding in file "+filename[1])
            print(filename[0])
        else:
            line = templine[0]+line
        templine = tuple()
    while 1:
        ind = line.find("commandMap")
        if ind == -1:
            break
        if hasPreProcessor(line, filename):#Don't want to include mixed stuff
            return
        line = line[ind:]
        start = line.find("[")
        if start == -1:
            print("No opening bracket found: "+line +"("+filename+")")
            return
        finish = False
        count = 0
        for i in range(len(line[start:])):
            ch = line[start:][i]
            if ch is '[':
                count+=1
            if ch is ']':
                count-=1
            if count == 0:
                finish = start+i
                break
        if not finish:#no closed set of brackets found
            global templine
            templine = (line, filename)#Try multiline statement
            return
        parsed = line[start+1:finish]
        parsed = makeKeyBinding(parsed)
        if parsed:
            global bindlines
            bindlines.append(parsed)
        line=line[finish+1:]
    return

def makeList(parsed):
    parsed = cleanSection(parsed,True).split("\",\"")
    if len(parsed) < 2:#This will happen when all the arguments to getVariable are variables themselves
#        print "Unparsable: "+filename+" : "+str(parsed)
        return False
    if parsed[0][0] != "\"":
#        print 'a'
        return False
    else:
        parsed[0] = parsed[0][1:]
    last = parsed[len(parsed)-1]
    last = last[len(last)-1]
    if last != "\"":
#        print 'b'
        return False
    parsed[len(parsed)-1] = parsed[len(parsed)-1][:len(parsed[len(parsed)-1])-1]
    default = parsed.pop()
    var = parsed.pop()
    section=list()
    for sec in parsed:#This will happen when one or more of the arguments to getVariable are variables themselves
        if sec == '' or var == '':
#            print 'c'
            return False
        section.append(sec)
    return [section,var,default]

def makeColorList(parsed):
    print("-----------------")
    print(parsed)
    parsed = cleanSection(parsed,True).split("\"")
    if len(parsed) < 1:
        return False
    print(parsed)
    if parsed[0] != "":
#        print 'a'
        return False
    parsed = parsed[1:]
    newparsed = list()
    print(newparsed)
    for item in parsed:
        if item == "":
            break
        if item == ",":
            continue
        newparsed.append(item)
    parsed = newparsed
    if len(parsed) < 1:#ie there is no string in the method call, unparsable
#        print "Unparsable: "+filename+" : "+str(parsed)
        return False
    if len(parsed) != 1:#if not a single string
        if len(parsed) != 2:#if not two strings
            print("More than the expected number of color strings!")
            return False
    else:
        if parsed[0] == "default":
            return False
        parsed = ["default",parsed[0]]
    name = parsed.pop()
    section = parsed.pop()
    return [section,name]

def makeKeyBinding(parsed):
    parsed = parsed.split("\"")
    if len(parsed) != 3:
        print("Bad keybinding ... this should *never* happen if the code compiles")
        return False
    return parsed[1]

def cleanSection(word,onlywhitespace=False):
    """Cleans the arguments of the config function call for parsing."""
    newword=str()
    quoted=str()
    for char in word:
        if quoted != str():
            if char == quoted[len(quoted)-1]:
                quoted = quoted[:len(quoted)-1]
                if newword[len(newword)-1] == char:
                    newword = newword[:len(newword)-1]
                else:
                    newword+=char
            else:
                if char in "\"\'":
                    quoted+=char
                newword+=char
        elif char in "\"\'":
            quoted+=char
            newword+=char
        elif char == ",":
            try:
                if newword[len(newword)-1] == char:
                    newword+="\"\""
                newword+=char
            except:
                pass
    if len(newword) > 1 and newword[len(newword)-1] == ",":
        newword+="\"\""
    return newword

def formatDuplicate(varone,vartwo):
    finalform=str()
    finalform+=os.path.split(varone[3])[1]+" :: "+os.path.split(vartwo[3])[1]
    finalform+="\n"
    for val in varone[0]:
        finalform+=val+'/'
    finalform+=varone[1]+" ("+varone[2]+")"
    finalform+=" :: "
    for val in vartwo[0]:
        finalform+=val+'/'
    finalform+=vartwo[1]+" ("+vartwo[2]+")"
    return finalform

def removeComments(parsable,comment):
    while 1:
        closeloc = parsable.find("*/")
        openloc = parsable.find("/*")
        lineloc = parsable.find("//")

        opened = openloc != -1
        closed = closeloc != -1
        linedout = lineloc != -1

        if comment:
            if closed:
                parsable = parsable[closeloc+2:]
                comment = False
                continue
            else:
                break

        elif (not opened) and (not linedout):
            if closed:
                raise RuntimeError("Shit code, an extra closed symbol")
            break

        elif not comment:
            if linedout:
                if opened:
                    if lineloc < openloc:
                        parsable = parsable[:lineloc]
                        continue
                elif closed:
                    if lineloc < closeloc:
                        parsable = parsable[:lineloc]
                        continue
                else:
                    parsable = parsable[:lineloc]
                    continue
            if opened:
                if closed:
                    parsable = parsable[:openloc]+parsable[closeloc+2:]
                    continue
                else:
                    parsable = parsable[:openloc]
                    comment = True
                    continue
    return parsable, comment


def printDuplicates():
    for i in range(len(varlines)-1):
        if len(varlines[i][0]) == len(varlines[i+1][0]):
            duplicate = True
            for j in range(len(varlines[i][0])):
                if varlines[i][0][j] != varlines[i+1][0][j]:
                    duplicate = False
                    break
            if varlines[i][1] == varlines[i+1][1] and duplicate:
                if varlines[i][2] != varlines[i+1][2]:
                    print("\nNon-Identical Duplicate:\n"+formatDuplicate(varlines[i],varlines[i+1])+"\n")

#----------------
# XML creation functions
#----------------

def createSection(currnode,section):
    for sec in section:
        currnode = constructTraverse(currnode, sec)
    global config
    return currnode

def constructTraverse(currnode, sec):
    add = True
    for element in currnode.getElementsByTagName('section'):
        if 'name' in element.attributes:
            if element.attributes['name'].value == sec:
                add = False
                newchild = element
                break
    if add:
        global config
        newchild = config.createElement('section')
        newchild.setAttribute('name', sec)
        currnode.appendChild(newchild)
    return newchild

def createVar(currnode, var, default):
    add = True
    for element in currnode.getElementsByTagName('var'):
        if 'name' in element.attributes:
            if element.attributes['name'].value == var:
                if shouldOverwrite(element,default):
                    element.attributes['name'].value = var
                add = False
                break
    if add:
        global config
        newchild = config.createElement('var')
        newchild.setAttribute('name', var)
        newchild.setAttribute('value', default)
        currnode.appendChild(newchild)

def createColor(currnode, name):#<color name="engine" r="1" g="1" b="1" a="1"/>	    <section name="absolute">
    print("color %s added"%name)
    add = True
    for element in currnode.getElementsByTagName('color'):
        if 'name' in element.attributes:
            if element.attributes['name'].value == name:
#                if shouldOverwrite(element,default):
#                    element.attributes['name'].value = var
                add = False
                break
    if add:
        global config
        newchild = config.createElement('color')
        newchild.setAttribute('name', name)
        newchild.setAttribute('r', '1')
        newchild.setAttribute('g', '1')
        newchild.setAttribute('b', '1')
        newchild.setAttribute('a', '1')
        currnode.appendChild(newchild)

def createBinding(currnode, binding):
    add = True
    for element in currnode.getElementsByTagName('bind'):
        if 'command' in element.attributes:
            if element.attributes['command'].value == binding:
                print("Not adding keybinding %s.  Already present."%binding)
                add = False
                break
    if add:
        global config
        newchild = config.createElement('bind')
        newchild.setAttribute('key', '')
        newchild.setAttribute('modifier', '')
        newchild.setAttribute('command', binding)
        currnode.appendChild(newchild)

def shouldOverwrite(element,new):
    current = element.attributes['value'].value
    if current == str() and new != str():
        print('Overwriting %s with %s'%(current,new))
        return True


#----------------
# The start of the script
#----------------

arg = sys.argv

srcpath = os.curdir
if len(arg) > 1:
    srcpath=arg[1]

print('Using source in '+srcpath)

srcpath=srcpath+os.sep

print('Grepping files')

filelist = list()

os.path.walk(srcpath, add, filelist)

print('Parsing list')

#Browse through for normal variables.
for filename in filelist:
    a = file(filename)
    comment = False
    while 1:
        line = a.readline()
        if not line:
            break
        parsable, comment = removeComments(line,comment)
        parseLine(parsable, filename)
    a.close()

varlines.sort()

#Extract keybindings.
a = file(srcpath+os.sep+"config_xml.cpp")
comment = False
while 1:
    line = a.readline()
    if not line:
        break
    parsable, comment = removeComments(line,comment)
    parseKeyBindingLine(parsable, filename)
a.close()


printDuplicates()

print('Creating XML')

config.appendChild(config.createElement('vegaconfig'))

node = config.createElement('bindings')
config.firstChild.appendChild(node)
for binding in bindlines:
    createBinding(node, binding)

for [section, var, default, cpp] in varlines:
    node = createSection(config.firstChild,section)
    createVar(node, var, default)

varlines = list()
#Doing it again for colors
for filename in filelist:
    a = file(filename)
    comment = False
    while 1:
        line = a.readline()
        if not line:
            break
        parsable, comment = removeComments(line,comment)
        parseColorLine(parsable, filename)
    a.close()

varlines.sort()
colorsnode = config.createElement('colors')
config.firstChild.appendChild(colorsnode)
for [section, name, cpp] in varlines:
    node = createSection(colorsnode,[section])#as our section is a string rather than the list like for vars
    createColor(node, name)


#a = open('pygrep','w')
#for line in varlines:
#    a.write(str(line)+"\n")
#a.close()

a = open('pygrep.config','w')
a.write(config.toprettyxml('    '))
a.close()

# TODO
# "color" support
# "doc" tag support, and the support of merging into an existing file (with prompt)
