find_package(EXPAT REQUIRED)
IF(EXPAT_FOUND)
SET(MESHER_SOURCES Converter.cpp
                   from_obj.cpp
                   to_BFXM.cpp
                   from_BFXM.cpp
                   main.cpp
                   to_obj.cpp
                   to_OgreMesh.cpp
                   Modules/Convert.cpp
                   Modules/OldSyntax.cpp
                   Modules/Dims.cpp
                   Modules/BFXM_to_Wavefront.cpp
                   Modules/BFXM_to_XMesh.cpp
                   Modules/XMesh_to_BFXM.cpp
                   Modules/XMesh_to_Ogre.cpp
                   Modules/Wavefront_to_BFXM.cpp
                   PrecompiledHeaders/Converter.cpp
                   ${vsUTCS_SOURCE_DIR}/src/hashtable.cpp
                   ${vsUTCS_SOURCE_DIR}/src/xml_support.cpp)

# Still need to add CEGUI and OGRE find packages
#if have OGRE
#                   to_OgreMesh.cpp

add_executable(mesher ${MESHER_SOURCES})

target_link_libraries(mesher ${MESHER_LIBRARIES} ${EXPAT_LIBRARIES} ${OGRE_LIBRARIES})
set_target_properties(mesher PROPERTIES LINK_FLAGS "-L/usr/lib")
ELSE(EXPAT_FOUND)
    message("!! Not going to be able to compile mesher, no expat found")
ENDIF(EXPAT_FOUND)
