#set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
include_directories("..")
SET(TRISORT_SOURCES trisort.cpp)
add_executable(trisort ${TRISORT_SOURCES})

SET(ASTEROID_SOURCES asteroidgen.cpp)
add_executable(asteroidgen ${ASTEROID_SOURCES})

SET(REPLACE_SOURCES replace.cpp)
add_executable(replace ${REPLACE_SOURCES})

#find Expat
find_package(EXPAT REQUIRED)
IF(EXPAT_FOUND)
	SET(MSH_INCLUDES ${MSH_INCLUDES} ${EXPAT_INCLUDE_DIRS})
	SET(MSH_LIBS ${MSH_LIBS} ${EXPAT_LIBRARIES})
ELSE(EXPAT_FOUND)
	message("!! We can't find expat for XML crap")
ENDIF(EXPAT_FOUND)

#ogre already looked for by parent
IF(OGRE_FOUND)
    SET(MSH_INCLUDES ${MSH_INCLUDES} ${OGRE_INCLUDE_DIRS})
    SET(MSH_LIBS ${MSH_LIBS} ${OGRE_LIBRARIES})
    message("++ Compiling mesh_tool with OgreMesh support")
ELSE(OGRE_FOUND)
    message("-- Compiling mesh_tool without OgreMesh support: Ogre not found")
ENDIF(OGRE_FOUND)

SET(MESHER_SOURCES mesher/Converter.cpp
                   mesher/from_BFXM.cpp
                   mesher/from_obj.cpp
                   mesher/main.cpp
                   mesher/to_obj.cpp
                   mesher/to_OgreMesh.cpp
                   mesher/to_BFXM.cpp
                   mesher/Modules/Convert.cpp
                   mesher/Modules/OldSyntax.cpp
                   mesher/Modules/Dims.cpp
                   mesher/Modules/BFXM_to_Wavefront.cpp
                   mesher/Modules/BFXM_to_XMesh.cpp
                   mesher/Modules/XMesh_to_BFXM.cpp
                   mesher/Modules/XMesh_to_Ogre.cpp
                   mesher/Modules/Wavefront_to_BFXM.cpp
                   ${vsUTCS_SOURCE_DIR}/src/hashtable.cpp
                   ${vsUTCS_SOURCE_DIR}/src/xml_support.cpp)

include_directories(${MSH_INCLUDES} mesher)
add_executable(mesh_tool ${MESHER_SOURCES})

# OGRE already looked for by parent
# USE_SYSTEM_BOOST already set by main script
IF(OGRE_FOUND AND NOT USE_SYSTEM_BOOST)
    # Ogre usage in mesher requires boost::system
    add_dependencies(mesh_tool boost_system)
    SET(MSH_LIBS ${MSH_LIBS} boost_system)
ENDIF(OGRE_FOUND AND NOT USE_SYSTEM_BOOST)

target_link_libraries(mesh_tool ${MSH_LIBS})


