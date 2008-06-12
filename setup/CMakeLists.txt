#set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
SET(VSSETUP_SOURCES src/c/setup.cpp
                    src/include/central.cpp
                    src/include/display_gtk.cpp
                    src/include/display_dialog.cpp
                    src/include/file.cpp
                    src/include/general.cpp
                    ../src/common/common.cpp )                                                                                                        


add_executable(vssetup ${VSSETUP_SOURCES})

#find GTK2
find_package(GTK2 REQUIRED)
IF(GTK2_FOUND)
    SET(GTK_LIBS ${GTK2_LIBRARIES})
    SET(GTK_CFLAGS ${GTK2_DEFINITIONS} -DGTK)
ELSE(GTK2_FOUND)
     message("Cant get GTK2 found")
ENDIF(GTK2_FOUND)

add_definitions(${GTK_CFLAGS})
include_directories(${vsUTCS_SOURCE_DIR}/setup/src/include ${vsUTCS_SOURCE_DIR}/src/common ${vsUTCS_BINARY_DIR} ${GTK2_INCLUDE_DIRS})
target_link_libraries(vssetup ${GTK_LIBS})
