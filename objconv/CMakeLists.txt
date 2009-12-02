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
	message("We can't find expat for XML crap")
ENDIF(EXPAT_FOUND)

SET(MESHER_SOURCES 
	mesher/Converter.cpp
	mesher/from_BFXM.cpp
	mesher/from_obj.cpp
	mesher/main.cpp
	mesher/to_BFXM.cpp
#	mesher/to_OgreMesh.cpp
	mesher/to_obj.cpp
	../src/xml_support.cpp
	mesher/Modules/BFXM_to_Wavefront.cpp
	mesher/Modules/BFXM_to_XMesh.cpp
	mesher/Modules/Convert.cpp
	mesher/Modules/Dims.cpp
	mesher/Modules/OldSyntax.cpp
	mesher/Modules/Wavefront_to_BFXM.cpp
	mesher/Modules/XMesh_to_BFXM.cpp
)

include_directories(${MSH_INCLUDES} mesher)
add_executable(mesh_tool ${MESHER_SOURCES})
target_link_libraries(mesh_tool ${MSH_LIBS})


