#!BPY

"""
Name: 'VegaStrike (.xmesh)...'
Blender: 232
Group: 'Import'
Tooltip: 'Import VegaStrike Models (.xmesh)'
"""

__author__ = "Alex 'CubOfJudahsLion' Feterman"
__url__ = ("blender", "http://www.blender.org",
"Author's homepage, http://geocities.com/cubofjudahslion")
__version__ = "0.1.1"

__bpydoc__ = """\
xmesh_import.py | Python Script for Blender3D | imports a VegaStrike .xmesh
Copyright (C)2005 Alex Feterman

<p>This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

<p>This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

<p>You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

<p>Usage:<br>
	Execute this script from the "File->Import" menu and choose a
	Xmesh file to open.

<p>Notes:<br>
	Generates UV mappings, but for the texture to be activated, go to
	the texture buttons and 
"""

import Blender
from Blender import Image, Texture, Material, Object, NMesh, Types
import xml.sax
import mod_meshtools
from string import rfind, lower


class XMeshHandler(xml.sax.handler.ContentHandler):
	"""
		Created to handle XML contexts in XMESH objects
	"""

	def __init__(self, filename):
		# find where the directory path ends
		# (both un*x and win accounted for)
		backSlashPos	= rfind(filename, '\\')
		slashPos		= rfind(filename, '/')
		if backSlashPos > slashPos:
			slashPos	= backSlashPos
		slashPos += 1
		# store the path
		self.path		= filename[:slashPos]
		dotPos			= rfind(filename, '.', slashPos)
		# object name is the filename minus path and extension
		if dotPos == -1:
			self.objName = filename[slashPos:]
		else:
			self.objName = filename[slashPos:dotPos]
		# material values (to be checked later)
		self.faces			= []
		self.verts			= []
		self.uvs			= []
		self.faceuvs		= []
		self.alpha			=\
		self.rgbCol	 		=\
		self.amb			=\
		self.emit			=\
		self.colorTexture	=\
		self.specTexture	=\
		self.spec			=\
		self.specCol		= None

	def startDocument(self):
		"""
			Callback. Invoked when the parsing starts. Used to
			display notification of process initiation.
		"""
		print "Loading file..."
		Blender.Window.DrawProgressBar(0.0, "Loading file...")
		
	def endDocument(self):
		"""
			Invoked when mesh processing is done. Used for realizing
			the mesh from collected vertex/faces and texturizing info.
		"""
		# report
		print "Finished loading file, constructing mesh..."
		Blender.Window.DrawProgressBar(0.9, "Building mesh...")
		# build object
		mod_meshtools.create_mesh(self.verts, self.faces, self.objName, self.faceuvs, self.uvs)
		print "Done, object built"
		# load corresponding images and set texture
		Blender.Window.DrawProgressBar(0.95, "Texture...")
		colorTex, specTex = None, None
		# convert images into textures
		if self.colorTexture:
			colorImg		= Image.Load(self.path + self.colorTexture)
			colorTex		= Texture.New(self.objName + ".col.tx")
			colorTex.type	= Texture.Types.IMAGE
			colorTex.image	= colorImg
		if self.specTexture:
			specImg			= Image.Load(self.path + self.specTexture)
			specTex			= Texture.New(self.objName + ".spe.tx")
			specTex.type	= Texture.Types.IMAGE
			specTex.image	= specImg
		# make material with them and all other previously collected data
		mat = Material.New(self.objName + ".mat")
		mat.mode		|= Material.Modes.TEXFACE | Material.Modes.SHADOW | Material.Modes.TRACEABLE | Material.Modes.ZTRANSP
		mat.specTransp	= 1.0
		if self.alpha	: mat.alpha		= self.alpha
		if self.rgbCol	: mat.rgbCol	= self.rgbCol
		if self.amb		: mat.amb		= self.amb
		if self.emit	: mat.emit		= self.emit
		if self.spec	: mat.spec		= self.spec
		if self.specCol	: mat.specCol	= self.specCol
		if colorTex:
			mat.setTexture(0, colorTex, Texture.TexCo.UV, Texture.MapTo.COL)
		if specTex:
			mat.setTexture(1, specTex, Texture.TexCo.UV, Texture.MapTo.SPEC)
		# apply to mesh
		obj = Object.Get(self.objName)
		mesh = obj.data
		mesh.mode = NMesh.Modes.NOVNORMALSFLIP
		mesh.addMaterial(mat)
		mesh.update()
		# Done, notify user
		Blender.Window.DrawProgressBar(1.0, "Done.")

	def startElement(self, pname, attr):
		"""
			Receives pre-parsed data for every geometry/texture
			datum in the mesh
		"""
		# we ignore case
		name = lower(pname)
		# like blender, wings3d and vegastrike are also opengl apps.
		# the internal format described by the xml tags is similar to
		# that of blender. see the xmesh format description and the opengl
		# red/blue books for structure and mapping details
		if name == "mesh":
			if "texture" in attr:
				self.colorTexture = attr["texture"]
				print "* color tex:", self.colorTexture
			if "texture1" in attr:
				self.specTexture = attr["texture1"]
				print "* spec tex:", self.specTexture
		elif name == "points":
			print "Reading point coordinates..."
			Blender.Window.DrawProgressBar(0.1, "Reading coords...")
		elif name == "location":
			self.verts.append( (float(attr["x"]), float(attr["y"]), float(attr["z"])) )
		elif name == "polygons":
			print "Reading faces..."
			Blender.Window.DrawProgressBar(0.25, "Reading faces...")
		elif name == "tri" or name == "quad" or name == "trifan":
			self.faceVerts	= []
			self.facevUVs	= []
		elif name == "vertex":
			self.faceVerts.append(int(attr["point"]))
			self.facevUVs.append( (float(attr["s"]), 1-float(attr["t"])) )
		elif name == "diffuse":
			self.rgbCol	= [float(attr['red']), float(attr['green']), float(attr['blue'])]
			self.alpha	= float(attr['alpha'])
		elif name == "ambient":
			self.amb = (float(attr['red']) + float(attr['green']) + float(attr['blue'])) / 3.0 * float(attr['alpha'])
		elif name == "specular":
			specIn			= float(attr['alpha'])
			self.specCol	= [specIn*float(attr['red']), specIn*float(attr['green']), specIn*float(attr['blue'])]
			self.spec		= 0.01 # float(attr['alpha'])
		elif name == "emissive":
			# sorry, no emissive color, only emission index
			self.emit = (float(attr['red']) + float(attr['green']) + float(attr['blue'])) / 3.0 * float(attr['alpha'])

	def endElement(self, pname):
		"""
			Serves to assemble gathered
			data from inner subelements
		"""
		name = lower(pname)
		# these are handled directly
		if name == "tri" or name == "quad":
			# the faces are an array, each element an array of
			# vertex indexes as collected in self.verts
			# to get (x,y,z) tuples for the jth vertex of the ith face:
			# self.verts[self.faces[i][j]]
			self.faces.append(self.faceVerts)
			# similarly, the UV mapping coordinats for the same vertex
			# are expected to be retrievable as
			# self.uvs[self.faceuvs[i][j]]
			insertPos = len(self.uvs)
			self.faceuvs.append(range(insertPos, insertPos+len(self.facevUVs)))
			self.uvs.extend(self.facevUVs)
		# yes, opengl handles triangle fans naturally, but not blender
		elif name == "trifan":
			fanIdx = 2
			while fanIdx < len(self.faceVerts):
				# so we make triangles out of them instead
				self.faces.append( [self.faceVerts[0], self.faceVerts[fanIdx-1], self.faceVerts[fanIdx]] )
				insertPos = len(self.uvs)
				self.faceuvs.append(range(insertPos, insertPos+3))
				self.uvs.extend( [self.facevUVs[0], self.facevUVs[fanIdx-1], self.facevUVs[fanIdx]] )
				fanIdx += 1

def read(filename):
	"""
		Invokes the xml parser on the file upon being
		called by the file selector with a file name
	"""
	xml.sax.parse(filename, XMeshHandler(filename))

Blender.Window.FileSelector(read, "VegaStrike .XMesh")