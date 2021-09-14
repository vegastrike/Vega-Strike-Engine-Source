# - FindLinuxDistro.cmake
# This module detects the Linux Distro and package build tooling, and provides
# some variables for the primary CMake script to utilize to decide what to build.
#
# ========================
#  Example Usage:
#
#	FIND_PACKAGE(LinuxDistro, REQUIRED)
#
# Output Variables:
#	LSB_EXISTS
#		Is the distro an LSB Compliant Distro?
#
#	LSB_LINUX_DISTRIBUTION
#		Using LSB Tooling, what is the host distro family?
#
#	LSB_LINUX_DISTRIBUTION_CODENAME
#		Using LSB Tooling, what is the host distro code name?
#
#	VS_CAN_BUILD_DEB
#		Does the host have tooling installed to build Debian Packages?
#

# lsb_release -i --> but then it'll find `Ubuntu` instead of Debian so will have to map more

# we can probably more reliably detect via checking for different packager commands
# `lsb_release` doesn't always work correctly, so it's easier to read the underlying file
# if it exists. It doesn't always exist. Debian doesn't use it, but Ubuntu does.
SET(LSB_EXISTS FALSE)
SET(LSB_LINUX_DISTRIBUTION "Unknown")
SET(LSB_LINUX_DISTRIBUTION_CODENAME "Unknown")

# Lookup the LSB Data - this is known for Ubuntu, but also used by other distros too.
# and may help with distro-sub-selection
FIND_PROGRAM(LSB_RELEASE_EXEC lsb_release)
IF (LSB_RELEASE_EXEC)
	EXECUTE_PROCESS(COMMAND ${LSB_RELEASE_EXEC} --codename --short
		OUTPUT_VARIABLE LSB_LINUX_DISTRIBUTION_CODENAME
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	EXECUTE_PROCESS(COMMAND ${LSB_RELEASE_EXEC} --id --short
		OUTPUT_VARIABLE LSB_LINUX_DISTRIBUTION
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	SET(LSB_EXISTS TRUE)
	MESSAGE("Found Linux Distribution Release Name: ${LSB_LINUX_DISTRIBUTION_CODENAME}")
ENDIF (LSB_RELEASE_EXEC)

# Definitively detect Debian Package Build tools
FIND_PROGRAM(VS_CAN_BUILD_DEB NAMES dpkg-buildpackage)

# Definitively detect RPM Package Build Tools
FIND_PROGRAM(VS_CAN_BUILD_RPM rpmbuild)
