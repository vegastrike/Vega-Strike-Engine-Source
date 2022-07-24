##
# CMakeLists.txt
#
# Copyright (c) 2001-2002 Daniel Horn
# Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
# Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

IF (UNIX)

	# There is no good method for this. The preferred method of using the `TYPE` argument doesn't exist until CMake 3.14
	# and there is no CMake Policy to detect the change or force only the older style. `TYPE` is preferred because the
	# destination is held by CMake instead of being hard coded here. So once CMake 3.14 is the base version this can
	# be dropped to just the one version.
	IF (CMAKE_VERSION VERSION_LESS "3.14")
		MESSAGE("Hard coded man page destinations")
		INSTALL(FILES vegastrike-engine.1 DESTINATION "${CMAKE_INSTALL_MANDIR}")
		INSTALL(FILES vegastrike-engine.1 DESTINATION "${CMAKE_INSTALL_MANDIR}" RENAME vegastrike.1)
		INSTALL(FILES vsinstall.1 DESTINATION "${CMAKE_INSTALL_MANDIR}")
		INSTALL(FILES vslauncher.1 DESTINATION "${CMAKE_INSTALL_MANDIR}")
		INSTALL(FILES vegasettings.1 DESTINATION "${CMAKE_INSTALL_MANDIR}")
	ELSE (CMAKE_VERSION VERSION_LESS "3.14")
		# This is the preferred instruction, but it's not available until cmake 3.14
		INSTALL(FILES vegastrike-engine.1 TYPE MAN)
		INSTALL(FILES vegastrike-engine.1 TYPE MAN RENAME vegastrike.1)
		INSTALL(FILES vsinstall.1 TYPE MAN)
		INSTALL(FILES vslauncher.1 TYPE MAN)
		INSTALL(FILES vegasettings.1 TYPE MAN)
	ENDIF (CMAKE_VERSION VERSION_LESS "3.14")

ENDIF (UNIX)
