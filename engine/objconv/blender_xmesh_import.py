# blender_xmesh_import.py
#
# xmesh_import.py | Python Script for Blender | imports a VegaStrike .xmesh
# Copyright (C) 2005 Alex 'CubOfJudahsLion' Feterman, dandandaman
# Copyright (C) 2007 ace123
# Copyright (C) 2013 cellsafemode, klaussfreire
# Copyright (C) 2020 pyramid3d
# Copyright (C) 2025, 2026 Stephen G. Tuggy, Danny Gehl
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see
# <https://www.gnu.org/licenses/>.
#

bl_info = {
    "name": "Vega Strike (.xmesh) Import",
    "author": "Danny Gehl",
    "version": (0, 3, 0),
    "blender": (4, 2, 0),
    "location": "File > Import > Vega Strike (.xmesh)",
    "description": "Import Vega Strike .xmesh models with Custom Properties retained",
    "warning": "",
    "doc_url": "",
    "category": "Import-Export",
}

import os
import xml.sax
import bpy
from bpy_extras.io_utils import ImportHelper
from bpy.props import StringProperty
from bpy.types import Operator


def find_texture(path, file_name):
    """Locate a texture file beside the mesh or under ../../textures."""
    if not file_name:
        return None
        
    source_path = os.path.join(path, file_name)
    if os.path.isfile(source_path):
        return source_path

    search_base_dir = os.path.normpath(os.path.join(path, "..", "..", "textures"))
    if not os.path.isdir(search_base_dir):
        # vessels have an extra sub directory
        search_base_dir = os.path.normpath(os.path.join(path, "..", "..", "..", "textures"))
        if not os.path.isdir(search_base_dir):
            return None

    for root, _dirs, files in os.walk(search_base_dir):
        if file_name in files:
            return os.path.join(root, file_name)
    return None


def _load_image(filepath):
    if not filepath:
        return None
    image_name = os.path.basename(filepath)
    if image_name in bpy.data.images:
        image = bpy.data.images[image_name]
        if os.path.abspath(image.filepath) == os.path.abspath(filepath):
            return image
    return bpy.data.images.load(filepath, check_existing=True)


def _make_material(obj_name, handler):
    mat = bpy.data.materials.new(name=obj_name + ".mat")
    mat.use_nodes = True

    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    bsdf = nodes.new("ShaderNodeBsdfPrincipled")
    output.location = (400, 0)
    bsdf.location = (0, 0)
    links.new(bsdf.outputs["BSDF"], output.inputs["Surface"])

    # Base Color setup
    diffuse = handler.rgb_col or [1.0, 1.0, 1.0]
    bsdf.inputs["Base Color"].default_value = (diffuse[0], diffuse[1], diffuse[2], 1.0)
    
    if handler.alpha < 1.0:
        bsdf.inputs["Alpha"].default_value = handler.alpha
        mat.blend_method = "BLEND"
            
    if handler.emit > 0.0:
        bsdf.inputs["Emission Strength"].default_value = handler.emit
        
    if handler.spec is not None:
        if "Specular IOR Level" in bsdf.inputs:
            bsdf.inputs["Specular IOR Level"].default_value = handler.spec
        elif "Specular" in bsdf.inputs:
            bsdf.inputs["Specular"].default_value = handler.spec
        
    if handler.spec_col:
        bsdf.inputs["Specular Tint"].default_value = (handler.spec_col[0], handler.spec_col[1], handler.spec_col[2], 1.0)

    # 1. Color Texture (Texture)
    color_socket = None
    if handler.color_texture:
        color_path = find_texture(handler.path, handler.color_texture)
        if color_path:
            color_image = _load_image(color_path)
            if color_image:
                tex_node = nodes.new("ShaderNodeTexImage")
                tex_node.image = color_image
                tex_node.location = (-300, 200)
                color_socket = tex_node.outputs["Color"]
                if handler.alpha < 1.0:
                    links.new(tex_node.outputs["Alpha"], bsdf.inputs["Alpha"])

    if color_socket and handler.rgb_col != [1.0, 1.0, 1.0]:
        mix = nodes.new("ShaderNodeMix")
        mix.data_type = "RGBA"
        mix.blend_type = "MULTIPLY"
        mix.inputs["Factor"].default_value = 1.0
        mix.inputs["B"].default_value = (handler.rgb_col[0], handler.rgb_col[1], handler.rgb_col[2], 1.0)
        mix.location = (-50, 200)
        links.new(color_socket, mix.inputs["A"])
        links.new(mix.outputs["Result"], bsdf.inputs["Base Color"])
    elif color_socket:
        links.new(color_socket, bsdf.inputs["Base Color"])

    # 2. Specular Texture (Texture1)
    if handler.texture1:
        spec_path = find_texture(handler.path, handler.texture1)
        if spec_path:
            spec_image = _load_image(spec_path)
            if spec_image:
                if spec_image.colorspace_settings.name != 'Non-Color':
                    spec_image.colorspace_settings.name = 'Non-Color'
                spec_node = nodes.new("ShaderNodeTexImage")
                spec_node.image = spec_image
                spec_node.location = (-300, -100)
                links.new(spec_node.outputs["Color"], bsdf.inputs["Specular Tint"])

    # 3. Emissive / Damage Texture (Texture2)
    if handler.texture2:
        emis_path = find_texture(handler.path, handler.texture2)
        if emis_path:
            emis_image = _load_image(emis_path)
            if emis_image:
                emis_node = nodes.new("ShaderNodeTexImage")
                emis_node.image = emis_image
                emis_node.location = (-300, -400)
                
                links.new(emis_node.outputs["Color"], bsdf.inputs["Emission Color"])
                bsdf.inputs["Emission Strength"].default_value = 1.0

    # 4. Normal Map Texture (Texture4)
    if handler.texture4:
        normal_path = find_texture(handler.path, handler.texture4)
        if normal_path:
            normal_image = _load_image(normal_path)
            if normal_image:
                if normal_image.colorspace_settings.name != 'Non-Color':
                    normal_image.colorspace_settings.name = 'Non-Color'
                
                normal_tex_node = nodes.new("ShaderNodeTexImage")
                normal_tex_node.image = normal_image
                normal_tex_node.location = (-600, -600)
                
                normal_map_node = nodes.new("ShaderNodeNormalMap")
                normal_map_node.location = (-250, -600)
                
                links.new(normal_tex_node.outputs["Color"], normal_map_node.inputs["Color"])
                links.new(normal_map_node.outputs["Normal"], bsdf.inputs["Normal"])

    return mat


def create_xmesh_object(handler):
    mesh = bpy.data.meshes.new(handler.obj_name)
    mesh.from_pydata(handler.verts, [], handler.faces)
    mesh.update()

    # Assign UV Coordinates
    if handler.faceuvs and handler.uvs:
        uv_layer = mesh.uv_layers.new(name="UVMap")
        for face_idx, poly in enumerate(mesh.polygons):
            for corner_idx, loop_idx in enumerate(range(poly.loop_start, poly.loop_start + poly.loop_total)):
                uv_index = handler.faceuvs[face_idx][corner_idx]
                uv_layer.data[loop_idx].uv = handler.uvs[uv_index]

    # Assign Custom Normals if present
    if len(handler.normals) == len(handler.verts):
        mesh.normals_split_custom_set_from_vertices(handler.normals)

    obj = bpy.data.objects.new(handler.obj_name, mesh)
    handler.context.collection.objects.link(obj)

    # Store Vega Strike properties with standardized keys matching the Exporter
    obj["xmesh_scale"] = handler.scale
    obj["xmesh_reverse"] = handler.reverse
    obj["xmesh_forcetexture"] = handler.forcetexture
    obj["xmesh_sharevert"] = handler.sharevert
    obj["xmesh_polygonoffset"] = handler.polygonoffset
    obj["xmesh_blend"] = handler.blend
    obj["xmesh_alphatest"] = handler.alphatest
    
    obj["mat_power"] = handler.mat_power
    obj["mat_cullface"] = handler.mat_cullface
    obj["mat_reflect"] = handler.mat_reflect
    obj["mat_lighting"] = handler.mat_lighting
    obj["mat_usenormals"] = handler.mat_usenormals
    
    if handler.color_texture: obj["xmesh_texture"] = handler.color_texture
    if handler.texture1: obj["xmesh_texture1"] = handler.texture1
    if handler.texture2: obj["xmesh_texture2"] = handler.texture2
    if handler.texture4: obj["xmesh_texture4"] = handler.texture4

    mat = _make_material(handler.obj_name, handler)
    mesh.materials.append(mat)

    return obj


class XMeshHandler(xml.sax.handler.ContentHandler):
    """Parse Vega Strike .xmesh XML into mesh geometry and material data."""

    def __init__(self, filename, context):
        self.context = context
        self.path, simple_file = os.path.split(os.path.abspath(filename))
        self.obj_name = os.path.splitext(simple_file)[0]
        self._wm = bpy.context.window_manager

        self.faces = []
        self.verts = []
        self.normals = []
        self.uvs = []
        self.faceuvs = []

        # Material defaults
        self.rgb_col = [1.0, 1.0, 1.0]
        self.alpha = 1.0
        self.emit = 0.0
        self.spec = 0.5
        self.spec_col = None
        self.amb = 0.0

        # Header defaults
        self.scale = 1.0
        self.reverse = 0
        self.forcetexture = 0
        self.sharevert = 0
        self.polygonoffset = 0.0
        self.blend = "ONE ZERO"
        self.alphatest = 0.0
        
        self.color_texture = ""
        self.texture1 = ""
        self.texture2 = ""
        self.texture4 = ""

        self.mat_power = 60.0
        self.mat_cullface = 1
        self.mat_reflect = 0.0
        self.mat_lighting = 1
        self.mat_usenormals = 1

    def _progress(self, factor):
        try:
            if self._wm:
                self._wm.progress_update(int(factor * 100))
        except Exception:
            pass

    def startDocument(self):
        try:
            if self._wm:
                self._wm.progress_begin(0, 100)
        except Exception:
            pass

    def endDocument(self):
        create_xmesh_object(self)
        try:
            if self._wm:
                self._wm.progress_end()
        except Exception:
            pass

    def startElement(self, pname, attr_mixed):
        name = pname.lower()
        attr = {key.lower(): value for key, value in attr_mixed.items()}

        if name == "mesh":
            self.color_texture = attr.get("texture", "")
            self.texture1 = attr.get("texture1", "")
            self.texture2 = attr.get("texture2", "")
            self.texture4 = attr.get("texture4", "")

            if "scale" in attr:
                try: self.scale = float(attr["scale"])
                except ValueError: pass
            if "reverse" in attr: self.reverse = int(attr["reverse"])
            if "forcetexture" in attr: self.forcetexture = int(attr["forcetexture"])
            if "sharevert" in attr: self.sharevert = int(attr["sharevert"])
            if "polygonoffset" in attr: self.polygonoffset = float(attr["polygonoffset"])
            if "blend" in attr: self.blend = attr["blend"]
            if "alphatest" in attr: self.alphatest = float(attr["alphatest"])

        elif name == "location":
            self.verts.append((float(attr["x"]), float(attr["y"]), float(attr["z"])))
            
        elif name == "normal":
            self.normals.append((float(attr["i"]), float(attr["j"]), float(attr["k"])))

        elif name in ("tri", "quad", "trifan"):
            self._face_verts = []
            self._face_uvs = []

        elif name == "vertex":
            self._face_verts.append(int(attr["point"]))
            self._face_uvs.append((float(attr["s"]), 1.0 - float(attr["t"])))

        elif name == "diffuse":
            self.rgb_col = [float(attr["red"]), float(attr["green"]), float(attr["blue"])]
            self.alpha = float(attr["alpha"])

        elif name == "ambient":
            self.amb = (float(attr["red"]) + float(attr["green"]) + float(attr["blue"])) / 3.0 * float(attr["alpha"])

        elif name == "specular":
            self.spec_col = [float(attr["red"]), float(attr["green"]), float(attr["blue"])]
            self.spec = float(attr.get("alpha", 0.5))

        elif name == "emissive":
            self.emit = (float(attr["red"]) + float(attr["green"]) + float(attr["blue"])) / 3.0 * float(attr["alpha"])

        elif name == "material":
            if "power" in attr: self.mat_power = float(attr["power"])
            if "cullface" in attr: self.mat_cullface = int(attr["cullface"])
            if "reflect" in attr: self.mat_reflect = float(attr["reflect"])
            if "lighting" in attr: self.mat_lighting = int(attr["lighting"])
            if "usenormals" in attr: self.mat_usenormals = int(attr["usenormals"])

    def endElement(self, pname):
        name = pname.lower()

        if name in ("tri", "quad"):
            self.faces.append(self._face_verts)
            insert_pos = len(self.uvs)
            self.faceuvs.append(list(range(insert_pos, insert_pos + len(self._face_uvs))))
            self.uvs.extend(self._face_uvs)

        elif name == "trifan":
            fan_idx = 2
            while fan_idx < len(self._face_verts):
                self.faces.append([self._face_verts[0], self._face_verts[fan_idx - 1], self._face_verts[fan_idx]])
                insert_pos = len(self.uvs)
                self.faceuvs.append(list(range(insert_pos, insert_pos + 3)))
                self.uvs.extend([self._face_uvs[0], self._face_uvs[fan_idx - 1], self._face_uvs[fan_idx]])
                fan_idx += 1


def import_xmesh(context, filepath):
    xml.sax.parse(filepath, XMeshHandler(filepath, context))
    return {"FINISHED"}


class ImportXMesh(Operator, ImportHelper):
    """Import a Vega Strike .xmesh file"""

    bl_idname = "import_scene.xmesh"
    bl_label = "Vega Strike (.xmesh)"
    bl_options = {"PRESET", "UNDO"}

    filename_ext = ".xmesh"
    filter_glob: StringProperty(
        default="*.xmesh",
        options={"HIDDEN"},
        maxlen=255,
    )

    def execute(self, context):
        return import_xmesh(context, self.filepath)


def menu_func_import(self, context):
    self.layout.operator(ImportXMesh.bl_idname, text="Vega Strike (.xmesh)")


def register():
    bpy.utils.register_class(ImportXMesh)
    bpy.types.TOPBAR_MT_file_import.append(menu_func_import)


def unregister():
    bpy.utils.unregister_class(ImportXMesh)
    bpy.types.TOPBAR_MT_file_import.remove(menu_func_import)


if __name__ == "__main__":
    register()