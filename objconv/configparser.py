#!/usr/bin/python
import sys
import os
import fnmatch
import string

global varlines
varlines = list()

def add(filelist, dirname, filenames):
    """Adds any .cpp files in the directory to filelist."""
    filenames = fnmatch.filter(filenames, '*.cpp')
    filelist.extend([os.path.join(dirname, filename) for filename in filenames])

def parseLine(line, filename):
    while 1:
        ind = line.find("getVariable")
        if ind == -1:
            break
        line = line[ind:]
        start = line.find("(")
        if start == -1:
            print "Error: bracket not found"+filename
            break
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
        if not finish:
            return
        parsed = line[start+1:finish]
        parsed = makeList(parsed)
        if parsed:
            global varlines
            parsed.append(filename)
            varlines.append(parsed)
        line=line[finish+1:]
    return

def makeList(parsed):
    parsed = cleanSection(parsed,True).split("\",\"")
    if len(parsed) < 2:
        print "Unparsable: "+filename+" : "+str(parsed)
        return False
    if parsed[0][0] != "\"":
        print 'a'
        return False
    else:
        parsed[0] = parsed[0][1:]
    last = parsed[len(parsed)-1]
    last = last[len(last)-1]
    if last != "\"":
        print 'b'
        return False
    parsed[len(parsed)-1] = parsed[len(parsed)-1][:len(parsed[len(parsed)-1])-1]
    default = parsed.pop()
    var = parsed.pop()
    section=list()
    for sec in parsed:
        if sec == '' or var == '':
            print 'c'
            return False
        section.append(sec)
    return [section,var,default]

def cleanSection(word,onlywhitespace=False):
    """Cleans the arguments of the config function call for parsing."""
    newword=str()
    quoted=str()
    for char in word:
        if quoted != str():
            if char == quoted[len(quoted)-1]:
#                print "Changing quoted from : "+quoted,
                quoted = quoted[:len(quoted)-1]
#                print "To : "+quoted+" Char : "+char
                if newword[len(newword)-1] == char:
#                    print "Changing newword from : "+newword,
                    newword = newword[:len(newword)-1]
#                    print "To : "+newword
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

#                print "\tCannot clean section:\n\t"+word
                pass
#                (etype, eobj, etb) = sys.exc_info()
                # eobj is the SIDL exception object
#                print eobj  # show the exception comment
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
#    print 'Parsing '+parsable
    while 1:
        closeloc = parsable.find("*/")#Lets search for a single block quote
        openloc = parsable.find("/*")
        lineloc = parsable.find("//")

        opened = openloc != -1
        closed = closeloc != -1
        linedout = lineloc != -1
#        print 'Comment '+str(comment)
#        print closeloc
#        print openloc
#        print lineloc

#        print closed
#        print opened
#        print linedout
#        print parsable

        if comment:
            if closed:
#                print 'Is closed : ',
                parsable = parsable[closeloc+2:]
#                print parsable
                comment = False
                continue
            else:
#                print 'Given up'
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
                    print "\nNon-Identical Duplicate:\n"+formatDuplicate(varlines[i],varlines[i+1])+"\n"


#----------------
# The start of the script
#----------------

arg = sys.argv
print arg

srcpath = os.curdir
if len(arg) > 1:
    srcpath=arg[1]

srcpath=srcpath+os.sep

print os.getcwd()
print srcpath

print 'Grepping files'

filelist = list()

os.path.walk(srcpath, add, filelist)

print 'Parsing list'

for filename in filelist:
    a = file(filename)
    comment = False
    while 1:
        line = a.readline()
        if not line:
            break
#        print comment
        parsable, comment = removeComments(line,comment)
#        print filename
        parseLine(parsable, filename)
    a.close()

varlines.sort()

printDuplicates()

print 'Creating XML'

def createSection(currnode,section):
    for sec in section:
        currnode = constructTraverse(currnode, sec)
    global config
    return currnode

def constructTraverse(currnode, sec):
    add = True
    for element in currnode.getElementsByTagName('section'):
        if element.attributes.has_key('name'):
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
        if element.attributes.has_key('name'):
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

def shouldOverwrite(element,new):
    current = element.attributes['value'].value
    if current == str() and new != str():
        print 'Overwriting %s with %s'%(current,new)
        return True

from xml.dom.minidom import *
global config
config = Document()
config.appendChild(config.createElement('vegaconfig'))
for [section, var, default, cpp] in varlines:
    node = createSection(config.firstChild,section)
    createVar(node, var, default)

a = open('pygrep','w')
for line in varlines:
    a.write(str(line)+"\n")
a.close()

a = open('pyconfig','w')
a.write(config.toprettyxml('    '))
a.close()
