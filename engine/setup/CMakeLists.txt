IF (NOT BEOS)
    #SET_PROPERTY(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
    SET(VEGASETTINGS_SOURCES
        src/c/setup.cpp
        src/include/central.cpp
        src/include/display_gtk.cpp
        src/include/display_dialog.cpp
        src/include/file.cpp
        src/include/general.cpp
        ../src/common/common.cpp
    )

    ADD_EXECUTABLE(vegasettings ${VEGASETTINGS_SOURCES})

    #find GTK2
    FIND_PACKAGE(GTK2 REQUIRED)
    IF (GTK2_FOUND)
        SET(GTK_LIBS ${GTK2_LIBRARIES})
        SET(GTK_CFLAGS ${GTK2_DEFINITIONS} -DGTK)
    ELSE (GTK2_FOUND)
        MESSAGE("Cant get GTK2 found")
    ENDIF (GTK2_FOUND)

    ADD_DEFINITIONS(${GTK_CFLAGS})
    INCLUDE_DIRECTORIES(
        ${vsUTCS_SOURCE_DIR}/setup/src/include
        ${vsUTCS_SOURCE_DIR}/src/common
        ${vsUTCS_BINARY_DIR}
        ${GTK2_INCLUDE_DIRS}
    )
    TARGET_LINK_LIBRARIES(vegasettings ${GTK_LIBS})
ENDIF (NOT BEOS)
