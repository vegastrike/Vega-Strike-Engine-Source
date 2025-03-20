# Copyright (C) 2025 Stephen G. Tuggy and other Vega Strike contributors.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.

MESSAGE("PROCESSING DIRECTORY: ${CMAKE_CURRENT_SOURCE_DIR}")

ADD_LIBRARY(vegastrike_gfx_generic SHARED
        texture_manager.cpp
        boltdrawmanager.cpp
        cockpit_generic.cpp
        lerp.cpp
        matrix.cpp
        mesh_bxm.cpp
        mesh_poly.cpp
        mesh_xml.cpp
        mesh.cpp
        quaternion.cpp
        soundcontainer_generic.cpp
        sphere_generic.cpp
        tvector.cpp
        macosx_math.cpp
)
