IF (NOT BEOS)
#set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
SET(VSSETUP_SOURCES src/c/setup.cpp
                    src/include/central.cpp
                    src/include/display_gtk.cpp
                    src/include/display_dialog.cpp
                    src/include/file.cpp
                    src/include/general.cpp
                    ../src/common/common.cpp )                                                                                                        


add_executable(vssetup ${VSSETUP_SOURCES})

#find GTK3 (replaces removed GTK2) via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
SET(GTK_CFLAGS -DGTK)
add_definitions(${GTK_CFLAGS})
include_directories(${vsUTCS_SOURCE_DIR}/setup/src/include ${vsUTCS_SOURCE_DIR}/src/common ${vsUTCS_BINARY_DIR} ${GTK3_INCLUDE_DIRS})
target_link_libraries(vssetup ${GTK3_LIBRARIES})
ENDIF (NOT BEOS)
