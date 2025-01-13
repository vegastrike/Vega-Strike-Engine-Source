#!BPY
##
# blender_xmesh_export.py
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Creator: Daniel Horn
# Original development team: As listed in the AUTHORS file
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


"""
Name: 'VegaStrike (selected) (.xmesh)...'
Blender: 228
Group: 'Export'
Tooltip: 'Export VegaStrike (selected) (.xmesh)'
"""
from __future__ import print_function
try:
    range = xrange
except:
    pass

__author__	= "Dandandaman"
__url__		= ("blender", "http://www.blender.org", "Author's homepage, http://black-paralysis.sf.net/")
__version__	= "0.1"

#
# XMESH exporter for Blender v2.28+ (by dandandaman <dandandaman@users.sourceforge.net>
# Written from the ground up for XMESH and for the new api.  Reference was taken of
# Volker [vmx] Mische's OBJ script for v2.11+
#
#
#==================================================
#               Things to do
#==================================================
# Get texture filename
# Fix index problem better for tex coords
# Add support for specifying directories
# Add GUI support
# Add input support
#
#
#
#==================================================
#               Usage Instructions
#==================================================
# Select the objects you wish to export.
# Run this script.
#
# Remember that texture coordinates must be UV mapped!
# And remember to use only one UV image per object!
#
#
#
#
#
#
# ------------------------------
# IMPORT NESCESSARY LIBRARIES
# ------------------------------
import Blender
from Blender import Types, Object, NMesh
import os

# ------------------------------
# SETUP RUNTIME VARIABLES
# ------------------------------
global outputdir
outputpath = '.'	# If this is invalid, I use your current working dir (printed out at end)
if 'HOMEDRIVE' in os.environ and 'HOMEPATH' in os.environ:
	outputpath=os.environ['HOMEDRIVE'].os.environ['HOMEPATH']
if 'HOME' in os.environ:
	outputpath=os.environ['HOME']

#outputdir, Name = os.path.split(outputpath)
#print outputdir
#print Name
outputdir=outputpath

objtransforms = False									# FIXME:  This is not used yet!!!

def processSelected():
	global outputdir
	if not os.path.isdir(outputdir):
		outputdir = os.curdir

	os.chdir(outputdir)

	objs = Object.GetSelected()

	if len(objs) == 0:
		print("Error:  No Objects Selected")
	else:
		for obj in objs:
			if obj.getType() != "Mesh":
				print(("Error: Object " + obj.name + " is a " + obj.getType() + ", not a mesh!"))
			else:
				print(("Object is a " + obj.getType()))
				exportObject(obj)

	print("Finished: xmesh files written to %s" % os.getcwd())


def exportObject(obj):
	objmesh = NMesh.GetRawFromObject(obj.getName())
	objfaces = objmesh.faces
	objfacelists = list()
	for fac in objfaces:
		objfacelists.append(fac.v)
	for i in range(len(objfacelists)):
		for j in range(len(objfacelists[i])):
			objfacelists[i][j] = objfacelists[i][j].index
#	print objfacelists
	objmaterials = objmesh.materials
	objvertices = objmesh.verts
	vertcoords = list()
	for vert in objvertices:
		vertcoords.append(vert.co)
#	print vertcoords
	vertnormals = list()
	for vert in objvertices:
		vertnormals.append(vert.no)
#	print vertnormals
#	texcoords = list()
#	for vert in objvertices:
#		texcoords.append(vert.uvco)
#	print texcoords
	texcoords = list()
	for fac in objfaces:
		texcoords.append(fac.uv)
#	print texcoords

	filename = "%s_%s.xmesh" % (obj.getName(),objmesh.name)
	FILE = open(filename, "w")
	FILE.write("<!--\n")
	FILE.write("Vegastrike XMESH <" + filename + "> from Blender (by dandandaman's script)\n")


	# Polycount info
	FILE.write("Total number of Faces:   \t%s\n" % len(objfaces))
	FILE.write("Total number of Vertices:\t%s\n" % len(objvertices))
	FILE.write("-->\n")


	quads = list()
	tris = list()

	global teximg
	teximg = [objfaces[0].image]

	for fac in objfaces:
		if len(fac.v) == 4:
			quads.append(fac)
		elif len(fac.v) == 3:
			tris.append(fac)
		if fac.image not in teximg and (fac.image):
			teximg.append(fac.image)

	texname = ""
	tex_ind = 0
	while (tex_ind < len(teximg)):
		if not teximg[tex_ind]:
			tex_ind+=1
		else:
			texname = teximg[tex_ind].getName()
			tex_ind+=1

	# FIXME: add texture header!!!
	FILE.write("<Mesh ")
	FILE.write("texture = \"%s\">\n" % texname)

	# Print all vertices and vertice normals to file
	FILE.write("<Points>\n")
	for i in range(len(objvertices)):
		FILE.write("\t<Point>\n")
		FILE.write("\t\t<Location x=\"%s\" y=\"%s\" z=\"%s\"/> <!-- %s -->\n" % (objvertices[i][0], objvertices[i][1], objvertices[i][2], i))
		FILE.write("\t\t<Normal i=\"%s\" j=\"%s\" k=\"%s\"/>\n" % (vertnormals[i][0], vertnormals[i][1], vertnormals[i][2]))
		FILE.write("\t</Point>\n")
	FILE.write("</Points>\n")

	FILE.write("<Polygons>\n")

	makePolys(tris,FILE)
	makePolys(quads,FILE)

#	if (quads):					#FIXME: Add helper function for polygons to make this easier!!!
#		for quad in quads:
#			FILE.write("\t<Quad>\n")
#			for j in range(len(quad.v)):
#				if has_uv:
#					FILE.write("\t\t<Vertex point=\"%s\" s=\"%s\" t=\"%s\"/>\n" % ((quad.v[j] + 1), quad.uv[j][0], quad.uv[j][1]))
#				else:
#					FILE.write("\t\t<Vertex point=\"%s\" s=\"\" t=\"\"/>\n" % (quad.v[j] + 1))
#			FILE.write("\t</Quad>\n")
#
#	if (tris):
#		for tri in tris:
#			FILE.write("\t<Tri>\n")
#			for j in range(len(tri.v)):
#				if has_uv:
#					FILE.write("\t\t<Vertex point=\"%s\" s=\"%s\" t=\"%s\"/>\n" % ((tri.v[j] + 1), tri.uv[j][0], tri.uv[j][1]))
#				else:
#					FILE.write("\t\t<Vertex point=\"%s\" s=\"\" t=\"\"/>\n" % (tri.v[j] + 1))
#			FILE.write("\t</Tri>\n")
#
#

	FILE.write("</Polygons>\n<Material>\n</Material>\n</Mesh>")
	FILE.close()
 	print(("Written " + filename))
	print("\t%s contains %s faces (%s quads, %s tris) and %s vertices" % (filename, len(objfaces), len(quads), len(tris), len(objvertices)))
	printErrors()

def printErrors():
	global teximg
	if teximg == [""]:
		print("\tError (non critical): your object has no textures!")
		print("\t\tIf this is a surprise to you, remember that your object")
		print("\t\tcan only use UV mapping to export the texture info properly.")

	elif len(teximg) != 1:
		print("\tError (non critical): your object uses %s images as textures." % len(teximg))
		print("\t\tAlthough valid in blender...VS doesn't support this.")
		print("\t\tIf you hassle me enough, I may make the script automatically")
		print("\t\tbreak the object into seperate meshes to accomodate that.")
	print("\n")

def makePolys(polylist,FILE):
	has_uv = False
	for poly in polylist:
		FILE.write("\t<%s>\n" % (getLabel(poly)))
		for j in range(len(poly.v)):
			FILE.write("\t\t<Vertex point=\"%s\" %s/>\n" % (poly.v[j], formatUV(poly.uv,j)))
#			if has_uv:
#				FILE.write("\t\t<Vertex point=\"%s\" %s/>\n" % (poly.v[j] + 1, formatUV(poly.uv[j])))
#			else:
#				FILE.write("\t\t<Vertex point=\"%s\" s=\"\" t=\"\"/>\n" % (poly.v[j] + 1))
		FILE.write("\t</%s>\n" % (getLabel(poly)))

def getLabel(poly):
	num = len(poly.v)
	if num == 3:
		return "Tri"
	if num == 4:
		return "Quad"

def formatUV(uv,i):
	str = "s=\"\" t=\"\""
	if (uv):
		str = "s=\"%s\" t=\"%s\"" % (uv[i][0], uv[i][1])
	return str

# ------------------------------
# Runs the script
# ------------------------------
#Window.FileSelector(processSelected, 'Export VegaStrike (selected)', newFName('xmesh'))

processSelected()
