##
# CMakeLists.txt
#
# Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
# Benjamen R. Meyer, and other Vega Strike contributors
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
#


INCLUDE(GNUInstallDirs)
MESSAGE("Always using preferred method now to install doc files")
INSTALL(FILES AUTHORS TYPE DOC RENAME AUTHORS.SF)
INSTALL(FILES "${PROJECT_SOURCE_DIR}/AUTHORS" TYPE DOC RENAME AUTHORS.GITHUB)
INSTALL(FILES ChangeLog TYPE DOC)

ADD_SUBDIRECTORY(man)
