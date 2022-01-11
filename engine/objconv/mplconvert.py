##
# mplconvert.py
#
# Copyright (C) 2001-2002 Daniel Horn
# Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
# Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
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

from __future__ import print_function
import sys
import xml
import xml.dom
import xml.dom.minidom
import math

masterpartfile=file("master_part_list.csv","w")

def printncommas(n):
    retstring=""
    for k in range(n):
        retstring+=','
    return retstring
def printn1(n,delim=","):
    retstring=""
    for k in range(n):
        retstring+='1'
        retstring+=delim
    return retstring
def printn0(n,delim=','):
    retstring=""
    for k in range(n):
        retstring+='0'
        retstring+=delim
    return retstring

def usage():
    print("Usage:\n\tCall with name of Xunit file to convert to MasterPartList file")
    return

def xmltocaps(dom):
    if(dom.nodeType==1):
        dom.tagName=dom.tagName.capitalize()
    curchild=dom.firstChild
    while (curchild):
        if(curchild.nodeType==1):
            xmltocaps(curchild)
        curchild=curchild.nextSibling
    return

def getAttributeValueCIS(node,attributename):
    for key in node.attributes.keys():
        if node.getAttributeNode(key).name.lower()==attributename.lower():
            return node.getAttribute(key);
    return ""
def defaultFloat (s,f=0):
    if (len(s)):
        return float(s)
    return f

def printcargo(category):
    retstring=""
    categoryname=getAttributeValueCIS(category,"file")
    cargo=category.getElementsByTagName("Cargo")
    for acargo in cargo:
        retstring+=getAttributeValueCIS(acargo,"file")+','+categoryname+','+getAttributeValueCIS(acargo,"price")+','+getAttributeValueCIS(acargo,"mass")+','+getAttributeValueCIS(acargo,"volume")+',"'+getAttributeValueCIS(acargo,"description")+'"\n'
    return retstring

##
##  START HERE
##

for inputunit in sys.argv:
    isblank = (inputunit.find(".blank")!=-1)
    istemplate = (inputunit.find(".template")!=-1)
    ismilspec = isblank==0 and istemplate==0
    print("Adding "+inputunit)
    inpxml = open(inputunit,"r")
    try:
      intermed = xml.dom.minidom.parseString(inpxml.read())
    except:
      print(inputunit+" failed")
      continue
    inpxml.close()
    xmltocaps(intermed)
    retstring=""
#key
    retstring+='file,categoryname,price,mass,volume,description\n'
#cargo
    categories=intermed.getElementsByTagName('Category')
    for category in categories:
        retstring+=printcargo(category)

masterpartfile.write(retstring)
masterpartfile.close()

