FIND_PACKAGE(EXPAT REQUIRED)
IF (EXPAT_FOUND)
    SET(MESHER_SOURCES
        Converter.cpp
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
        ${vsUTCS_SOURCE_DIR}/src/xml_support.cpp
    )

    # Still need to add CEGUI and OGRE find packages
    #if have OGRE to_OgreMesh.cpp

    ADD_EXECUTABLE(mesher ${MESHER_SOURCES})

    # OGRE already looked for by parent
    # USE_SYSTEM_BOOST already set by main script
    MESSAGE("-- ${OGRE_FOUND} ${USE_SYSTEM_BOOST}"
    IF (OGRE_FOUND AND NOT USE_SYSTEM_BOOST)
        # Ogre usage in mesher requires boost::system
        ADD_DEPENDENCIES(mesher boost_system)
        SET(MESHER_LIBRARIES ${MESHER_LIBRARIES} boost_system)
    ENDIF (OGRE_FOUND AND NOT USE_SYSTEM_BOOST)

    TARGET_LINK_LIBRARIES(mesher ${MESHER_LIBRARIES} ${EXPAT_LIBRARIES} ${OGRE_LIBRARIES})
    SET_TARGET_PROPERTIES(mesher PROPERTIES LINK_FLAGS "-L/usr/lib")
ELSE (EXPAT_FOUND)
    MESSAGE("!! Not going to be able to compile mesher, no expat found")
ENDIF(EXPAT_FOUND)
