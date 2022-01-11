##
# CMakeLists.txt
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

INCLUDE(GNUInstallDirs)
# There is no good method for this. The preferred method of using the `TYPE` argument doesn't exist until CMake 3.14
# and there is no CMake Policy to detect the change or force only the older style. `TYPE` is preferred because the
# destination is held by CMake instead of being hard coded here. So once CMake 3.14 is the base version this can
# be dropped to just the one version.
IF (CMAKE_VERSION VERSION_LESS "3.14")
	# Because paths are hard coded this is limited to Unix platforms.
	IF (UNIX)
		MESSAGE("Hard coded doc destination")
		INSTALL(FILES AUTHORS DESTINATION "${CMAKE_INSTALL_DOCDIR}"  RENAME AUTHORS.SF)
		INSTALL(FILES "${PROJECT_SOURCE_DIR}/../AUTHORS" DESTINATION "${CMAKE_INSTALL_DOCDIR}" RENAME AUTHORS.GITHUB)
		INSTALL(FILES ChangeLog DESTINATION "${CMAKE_INSTALL_DOCDIR}")
	ENDIF (UNIX)
ELSE (CMAKE_VERSION VERSION_LESS "3.14")
	# This is the preferred instruction, but it's not available until cmake 3.14
	MESSAGE("Newer CMake version, using preferred method")
	INSTALL(FILES AUTHORS TYPE DOC RENAME AUTHORS.SF)
	INSTALL(FILES "${PROJECT_SOURCE_DIR}/../AUTHORS" TYPE DOC RENAME AUTHORS.GITHUB)
	INSTALL(FILES ChangeLog TYPE DOC)
ENDIF (CMAKE_VERSION VERSION_LESS "3.14")

ADD_SUBDIRECTORY(man)

