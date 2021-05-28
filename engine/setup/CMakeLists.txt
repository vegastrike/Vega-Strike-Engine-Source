IF ((NOT BEOS) AND (NOT WIN32))
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

	INSTALL(TARGETS vegasettings DESTINATION bin)

	#find GTK3
	FIND_PACKAGE(GTK3 REQUIRED)
	IF(GTK3_FOUND)
		SET(GTK_LIBS ${GTK3_LIBRARIES})
		SET(GTK_CFLAGS ${GTK3_DEFINITIONS} -DGTK)
	ELSE(GTK3_FOUND)
		MESSAGE("Did not find GTK3")
	ENDIF(GTK3_FOUND)

	ADD_DEFINITIONS(${GTK_CFLAGS})
	INCLUDE_DIRECTORIES(
        ${Vega_Strike_SOURCE_DIR}/setup/src/include
        ${Vega_Strike_SOURCE_DIR}/src/common
        ${Vega_Strike_BINARY_DIR}
        ${GTK3_INCLUDE_DIRS}
    )
	TARGET_LINK_LIBRARIES(vegasettings ${GTK_LIBS})
ENDIF ((NOT BEOS) AND (NOT WIN32))
