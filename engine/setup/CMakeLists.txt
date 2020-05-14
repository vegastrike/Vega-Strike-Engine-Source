IF (NOT BEOS)
#set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
SET(VEGASETTINGS_SOURCES src/c/setup.cpp
                    src/include/central.cpp
                    src/include/display_gtk.cpp
                    src/include/display_dialog.cpp
                    src/include/file.cpp
                    src/include/general.cpp
                    ../src/common/common.cpp )                                                                                                        


add_executable(vegasettings ${VEGASETTINGS_SOURCES})

#find GTK3
find_package(GTK3 REQUIRED)
IF(GTK3_FOUND)
    SET(GTK_LIBS ${GTK3_LIBRARIES})
    SET(GTK_CFLAGS ${GTK3_DEFINITIONS} -DGTK)
ELSE(GTK3_FOUND)
     message("Cant get GTK3 found")
ENDIF(GTK3_FOUND)

add_definitions(${GTK_CFLAGS})
include_directories(${vsUTCS_SOURCE_DIR}/setup/src/include ${vsUTCS_SOURCE_DIR}/src/common ${vsUTCS_BINARY_DIR} ${GTK3_INCLUDE_DIRS})
target_link_libraries(vegasettings ${GTK_LIBS})
ENDIF (NOT BEOS)
