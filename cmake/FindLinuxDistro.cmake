# - FindLinuxDistro.cmake
# This module detects the Linux Distro and package build tooling, and provides
# some variables for the primary CMake script to utilize to decide what to build.
#
# ========================
#  Example Usage:
#
#    FIND_PACKAGE(LinuxDistro, REQUIRED)
#
# Output Variables:
#    LINUX_ETC_OS_RELEASE_EXISTS
#       Does /etc/os-release exist?
#
#    LINUX_ID
#       Linux Distro Name (semi-authoritative)
#       Only valid if LINUX_ETC_OS_RELEASE_EXISTS is TRUE
#
#    LINUX_CODENAME
#       Linux Distro Code Name (semi-authoritative)
#       Only valid if LINUX_ETC_OS_RELEASE_EXISTS is TRUE
#
#    LINUX_VERSION_ID
#       Linux Distro Version (semi-authoritative)
#       Only valid if LINUX_ETC_OS_RELEASE_EXISTS is TRUE
#
#    LSB_EXISTS
#       Is the distro an LSB Compliant Distro?
#
#    LSB_LINUX_DISTRIBUTION
#       Using LSB Tooling, what is the host distro family? (authoritative)
#       Only valid if LSB_EXISTS is TRUE
#
#    LSB_LINUX_DISTRIBUTION_CODENAME
#       Using LSB Tooling, what is the host distro code name? (authoritative)
#       Only valid if LSB_EXISTS is TRUE
#
#    VS_CAN_BUILD_DEB
#       Does the host have tooling installed to build Debian (DEB) Packages?
#
#    VS_CAN_BUILD_RPM
#       Does the host have the tooling installed to build RPM Packages?
#

IF (CMAKE_SYSTEM_NAME STREQUAL Linux)

    SET(LINUX_ETC_OS_RELEASE_EXISTS FALSE)
    SET(LINUX_ID "Unknown")
    SET(LINUX_CODENAME "Unknown")
    SET(LINUX_VERSION_ID "Unknown")
    IF (EXISTS "/etc/os-release")
        SET(LINUX_ETC_OS_RELEASE_EXISTS TRUE)
        # Gather info about Linux distro and release (if applicable) for later use down below.
        EXECUTE_PROCESS(
            COMMAND sh -c "cat /etc/os-release | grep ^ID= | sed 's/^ID=//' | tr -d '\"\n'"
            OUTPUT_VARIABLE LINUX_ID)
        EXECUTE_PROCESS(
            COMMAND sh -c "cat /etc/os-release | grep ^VERSION_CODENAME= | sed 's/^VERSION_CODENAME=//' | tr -d '\"\n'"
            OUTPUT_VARIABLE LINUX_CODENAME)
        EXECUTE_PROCESS(
            COMMAND sh -c "cat /etc/os-release | grep ^VERSION_ID= | sed 's/^VERSION_ID=//' | tr -d '\"\n'"
            OUTPUT_VARIABLE LINUX_VERSION_ID)
    ENDIF (EXISTS "/etc/os-release")

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

ENDIF (CMAKE_SYSTEM_NAME STREQUAL Linux)
