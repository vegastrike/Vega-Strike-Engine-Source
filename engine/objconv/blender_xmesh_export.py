# blender_xmesh_export.py
#
# Copyright (C) dandandaman
# Copyright (C) 2007 ace123
# Copyright (C) 2013 cellsafemode, klaussfreire
# Copyright (C) 2020 pyramid3d
# Copyright (C) 2021 daviewales
# Copyright (C) 2025, 2026 Stephen G. Tuggy
# Copyright (C) 2026 Danny Gehl
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#


#
# XMESH exporter for Blender v5.2+ by Danny Gehl
# Written from the ground up for XMESH and for the latest Blender API.
# This is an evolution of the XMESH exporter for Blender v2.28+ (by dandandaman <dandandaman@users.sourceforge.net>
# Reference was taken of Volker [vmx] Mische's OBJ script for v2.11+
bl_info = {
    "name": "Vega Strike (.xmesh) Export",
    "author": "Danny Gehl",
    "version": (0, 3, 0),
    "blender": (4, 2, 0),
    "location": "File > Export > Vega Strike (.xmesh)",
    "description": "Export selected objects to Vega Strike .xmesh format structural custom properties.",
    "category": "Import-Export",
}

import bpy
import bmesh
from xml.sax.saxutils import quoteattr
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty
from bpy.types import Operator


def escape_attr(val):
    """Safely format and quote string values for XML attributes."""
    return quoteattr(str(val))


def export_object_to_xmesh(obj, filepath, triangulate=True):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    obj_eval = obj.evaluated_get(depsgraph)
    
    # Use BMesh to optionally triangulate N-Gons cleanly
    bm = bmesh.new()
    bm.from_mesh(obj_eval.to_mesh())
    
    if triangulate:
        bmesh.ops.triangulate(bm, faces=bm.faces[:])
        
    mesh = bpy.data.meshes.new(name="temp_xmesh")
    bm.to_mesh(mesh)
    bm.free()

    # Re-fetch fallback custom property values
    scale = obj.get("xmesh_scale", 1.0)
    reverse = obj.get("xmesh_reverse", 0)
    forcetexture = obj.get("xmesh_forcetexture", 0)
    sharevert = obj.get("xmesh_sharevert", 0)
    polygonoffset = obj.get("xmesh_polygonoffset", 0.0)
    blend = obj.get("xmesh_blend", "ONE ZERO")
    alphatest = obj.get("xmesh_alphatest", 0.0)

    texture = obj.get("xmesh_texture", "")
    texture1 = obj.get("xmesh_texture1", "")
    texture2 = obj.get("xmesh_texture2", "")
    texture4 = obj.get("xmesh_texture4", "")

    # Material settings extraction
    mat_power = obj.get("mat_power", 60.0)
    mat_cullface = obj.get("mat_cullface", 1)
    mat_reflect = obj.get("mat_reflect", 0)
    mat_lighting = obj.get("mat_lighting", 1)
    mat_usenormals = obj.get("mat_usenormals", 1)

    # Fallback to Shader Nodes for texture mapping
    if not texture and obj.data.materials:
        mat = obj.data.materials[0]
        if mat and mat.use_nodes:
            for node in mat.node_tree.nodes:
                if node.type == 'TEX_IMAGE' and node.image:
                    img_name = node.image.name
                    if "NORMAL" in img_name.upper() or node.image.colorspace_settings.name == 'Non-Color':
                        texture4 = img_name
                    else:
                        texture = img_name

    try:
        with open(filepath, "w", encoding="utf-8") as f:
            # Header writing with attribute escaping
            f.write(f'<Mesh scale="{scale:.6f}" ')
            f.write(f'reverse="{reverse}" forcetexture="{forcetexture}" sharevert="{sharevert}" ')
            f.write(f'polygonoffset="{polygonoffset:.6f}" blend={escape_attr(blend)} alphatest="{alphatest:.6f}"')
            if texture: f.write(f' texture={escape_attr(texture)}')
            if texture1: f.write(f' texture1={escape_attr(texture1)}')
            if texture2: f.write(f' texture2={escape_attr(texture2)}')
            if texture4: f.write(f' texture4={escape_attr(texture4)}')
            f.write(">\n")

            # Material block
            f.write(f'<Material power="{mat_power:.6f}" cullface="{mat_cullface}" reflect="{mat_reflect}" ')
            f.write(f'lighting="{mat_lighting}" usenormals="{mat_usenormals}">\n')
            f.write('\t<Ambient Red="1.000000" Green="1.000000" Blue="1.000000" Alpha="1.000000"/>\n')
            f.write('\t<Diffuse Red="1.000000" Green="1.000000" Blue="1.000000" Alpha="1.000000"/>\n')
            f.write('\t<Emissive Red="0.000000" Green="0.000000" Blue="0.000000" Alpha="1.000000"/>\n')
            f.write('\t<Specular Red="1.000000" Green="1.000000" Blue="1.000000" Alpha="1.000000"/>\n')
            f.write("</Material>\n")

            # Write Points (3D Coordinates & Normals)
            f.write("<Points>\n")
            for vert in mesh.vertices:
                norm = vert.normal
                f.write("\t<Point>\n")
                f.write(f'\t\t<Location x="{vert.co.x:.6f}" y="{vert.co.y:.6f}" z="{vert.co.z:.6f}"/>\n')
                f.write(f'\t\t<Normal i="{norm.x:.6f}" j="{norm.y:.6f}" k="{norm.z:.6f}"/>\n')
                f.write("\t</Point>\n")
            f.write("</Points>\n")

            # Write Polygons with Per-Loop UVs
            f.write("<Polygons>\n")
            uv_layer = mesh.uv_layers.active

            for poly in mesh.polygons:
                p_type = "Tri" if len(poly.loop_indices) == 3 else "Quad" if len(poly.loop_indices) == 4 else "Trifan"
                f.write(f"\t<{p_type}>\n")
                
                for loop_idx in poly.loop_indices:
                    vert_idx = mesh.loops[loop_idx].vertex_index
                    s_coord, t_coord = 0.0, 0.0
                    if uv_layer:
                        uv = uv_layer.data[loop_idx].uv
                        s_coord = uv.x
                        t_coord = 1.0 - uv.y  # Invert V-axis for DirectX/VegaStrike spec
                    
                    f.write(f'\t\t<Vertex point="{vert_idx}" s="{s_coord:.6f}" t="{t_coord:.6f}"/>\n')
                
                f.write(f"\t</{p_type}>\n")
            f.write("</Polygons>\n")
            f.write("</Mesh>\n")

    finally:
        bpy.data.meshes.remove(mesh)
        obj_eval.to_mesh_clear()


class ExportXMesh(Operator, ExportHelper):
    """Export selection to Vega Strike .xmesh"""
    bl_idname = "export_scene.xmesh"
    bl_label = "Export Vega Strike (.xmesh)"
    bl_options = {'PRESET'}

    filename_ext = ".xmesh"
    filter_glob: StringProperty(
        default="*.xmesh",
        options={'HIDDEN'},
        maxlen=255,
    )

    triangulate: BoolProperty(
        name="Triangulate Mesh",
        description="Convert all n-gons and quads to triangles before exporting",
        default=True,
    )

    def execute(self, context):
        selected_objs = context.selected_objects
        active_obj = context.active_object
        
        target_obj = active_obj if (active_obj and active_obj.type == 'MESH') else None
        if not target_obj:
            for obj in selected_objs:
                if obj.type == 'MESH':
                    target_obj = obj
                    break
                    
        if not target_obj:
            self.report({'ERROR'}, "No valid Mesh object selected for export.")
            return {'CANCELLED'}

        export_object_to_xmesh(target_obj, self.filepath, triangulate=self.triangulate)
        self.report({'INFO'}, f"Successfully exported xmesh file to {self.filepath}")
        return {'FINISHED'}


def menu_func_export(self, context):
    self.layout.operator(ExportXMesh.bl_idname, text="Vega Strike (.xmesh)")


def register():
    bpy.utils.register_class(ExportXMesh)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportXMesh)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()