# create a build directory somewhere outside of this source tree
# run cmake <path to source tree> inside it
# run ccmake and hand tune any config variables/paths you need to, save 
# run make ...or build your project files
# valid targets are vegastrike, vssetup, and vegaserver (though it's broken right now)
# ccmake will let you set the Build type (debug,release,basic)
# the CMAKE_CXX_FLAGS line directly below is a hack. 

cmake_minimum_required(VERSION 2.4)

project (vsUTCS)

SET(CMAKE_CXX_FLAGS "-include config.h -pipe -O2")

include_directories(${vsUTCS_SOURCE_DIR}/src 
					${vsUTCS_SOURCE_DIR}/src/cmd
					${vsUTCS_BINARY_DIR} ) 
					
add_library(netlowlevel    src/networking/lowlevel/netbuffer.cpp
						   src/networking/lowlevel/netui.cpp
						   src/networking/lowlevel/packet.cpp
						   src/networking/lowlevel/packetmem.cpp
						   src/networking/lowlevel/vsnet_address.cpp
						   src/networking/lowlevel/vsnet_clientstate.cpp
						   src/networking/lowlevel/vsnet_cmd.cpp
						   src/networking/lowlevel/vsnet_debug.cpp
						   src/networking/lowlevel/vsnet_dloadenum.cpp
						   src/networking/lowlevel/vsnet_dloadmgr.cpp
						   src/networking/lowlevel/vsnet_notify.cpp
						   src/networking/lowlevel/vsnet_oss.cpp
						   src/networking/lowlevel/vsnet_parsecmd.cpp
						   src/networking/lowlevel/vsnet_pipe.cpp
						   src/networking/lowlevel/vsnet_serversocket.cpp
						   src/networking/lowlevel/vsnet_socket.cpp
						   src/networking/lowlevel/vsnet_socketbase.cpp
						   src/networking/lowlevel/vsnet_socketflags.cpp
						   src/networking/lowlevel/vsnet_sockethttp.cpp
						   src/networking/lowlevel/vsnet_socketset.cpp
						   src/networking/lowlevel/vsnet_sockettcp.cpp
						   src/networking/lowlevel/vsnet_socketudp.cpp
						   src/networking/lowlevel/vsnet_thread_non.cpp
						   src/networking/lowlevel/vsnet_thread_p.cpp 
						   src/networking/lowlevel/vsnet_thread_sdl.cpp )

SET(LIBNET_SOURCES src/networking/inet_file.cpp 
				   src/networking/inet.cpp )

add_library(netgeneric    src/networking/accountsxml.cpp
						  src/networking/client.cpp
						  src/networking/fileutil.cpp
						  src/networking/savenet_util.cpp
						  src/networking/cubicsplines.cpp
						  src/networking/mangle.cpp
						  src/networking/netclient_clients.cpp
						  src/networking/netclient_devices.cpp 
						  src/networking/netclient_login.cpp
						  src/networking/netclient.cpp
						  src/networking/netserver_acct.cpp
						  src/networking/netserver_clients.cpp
						  src/networking/netserver_devices.cpp
						  src/networking/netserver_login.cpp
						  src/networking/netserver_net.cpp
						  src/networking/netserver.cpp 
						  src/networking/prediction.cpp
						  src/networking/zonemgr.cpp )

SET(LIBNETCLIENT_SOURCES src/networking/networkcomm.cpp
						 src/networking/webcam_support.cpp )

SET(LIBNETSERVER_SOURCES src/networking/networkcomm_server.cpp )

SET(LIBGUI_SOURCES src/gui/button.cpp
				   src/gui/control.cpp
				   src/gui/eventmanager.cpp
				   src/gui/eventresponder.cpp 
				   src/gui/font.cpp
				   src/gui/general.cpp
				   src/gui/glut_support.cpp
				   src/gui/groupcontrol.cpp
				   src/gui/guidefs.cpp
				   src/gui/guitexture.cpp
				   src/gui/modaldialog.cpp
				   src/gui/newbutton.cpp
				   src/gui/painttext.cpp
				   src/gui/picker.cpp
				   src/gui/scroller.cpp
				   src/gui/simplepicker.cpp
				   src/gui/slider.cpp
				   src/gui/staticdisplay.cpp
				   src/gui/text_area.cpp
				   src/gui/textinputdisplay.cpp
				   src/gui/window.cpp
				   src/gui/windowcontroller.cpp )

SET(LIBPYTHON_SOURCES src/python/init.cpp
					  src/python/python_compile.cpp
					  src/python/unit_exports.cpp
					  src/python/unit_exports1.cpp
					  src/python/unit_exports2.cpp
					  src/python/unit_exports3.cpp 
					  src/python/unit_method_defs.cpp
					  src/python/unit_wrapper.cpp
					  src/python/universe_util_export.cpp )
					  				   
SET(LIBSCRIPT_SOURCES src/cmd/script/director_generic.cpp
					  src/cmd/script/mission_script.cpp
					  src/cmd/script/mission.cpp
					  src/cmd/script/msgcenter.cpp
					  src/cmd/script/pythonmission.cpp
					  src/cmd/script/script_call_olist.cpp
					  src/cmd/script/script_call_omap.cpp
					  src/cmd/script/script_call_order.cpp
					  src/cmd/script/script_call_string.cpp
					  src/cmd/script/script_call_unit_generic.cpp
					  src/cmd/script/script_callbacks.cpp
					  src/cmd/script/script_expression.cpp
					  src/cmd/script/script_generic.cpp
					  src/cmd/script/script_statement.cpp
					  src/cmd/script/script_util.cpp
					  src/cmd/script/script_variables.cpp )
					  
SET(LIBCMDSCRIPTSERVER_SOURCES src/cmd/script/flightgroup_server.cpp
							   src/cmd/script/script_call_unit_server.cpp
							   src/cmd/script/c_alike/c_alike.tab.cpp
							   src/cmd/script/c_alike/lex.yy.cpp )


SET(LIBAI_SOURCES src/cmd/ai/aggressive.cpp
				  src/cmd/ai/comm_ai.cpp
				  src/cmd/ai/communication_xml.cpp
				  src/cmd/ai/communication.cpp
				  src/cmd/ai/docking.cpp
				  src/cmd/ai/event_xml.cpp
				  src/cmd/ai/fire.cpp
				  src/cmd/ai/fireall.cpp
				  src/cmd/ai/flybywire.cpp
				  src/cmd/ai/hard_coded_scripts.cpp
				  src/cmd/ai/ikarus.cpp
				  src/cmd/ai/missionscript.cpp
				  src/cmd/ai/navigation.cpp
				  src/cmd/ai/order_comm.cpp
				  src/cmd/ai/order.cpp
				  src/cmd/ai/script.cpp
				  src/cmd/ai/tactics.cpp
				  src/cmd/ai/turretai.cpp
				  src/cmd/ai/warpto.cpp )

add_library(OPcollide  src/cmd/collide2/Ice/IceAABB.cpp
			   		   src/cmd/collide2/Ice/IceContainer.cpp
			   		   src/cmd/collide2/Ice/IceHPoint.cpp
			   		   src/cmd/collide2/Ice/IceIndexedTriangle.cpp
			   		   src/cmd/collide2/Ice/IceMatrix3x3.cpp
			   		   src/cmd/collide2/Ice/IceMatrix4x4.cpp
			   		   src/cmd/collide2/Ice/IceOBB.cpp
			   		   src/cmd/collide2/Ice/IcePlane.cpp
			   		   src/cmd/collide2/Ice/IcePoint.cpp
			   		   src/cmd/collide2/Ice/IceRandom.cpp
			   		   src/cmd/collide2/Ice/IceRay.cpp
			   		   src/cmd/collide2/Ice/IceRevisitedRadix.cpp
			   		   src/cmd/collide2/Ice/IceSegment.cpp 
			   		   src/cmd/collide2/Ice/IceTriangle.cpp
			   		   src/cmd/collide2/Ice/IceUtils.cpp
			   		   src/cmd/collide2/csgeom2/opbox.cpp
			   		   src/cmd/collide2/csgeom2/opmatrix3.cpp
			   		   src/cmd/collide2/csgeom2/opvector3.cpp
			   		   src/cmd/collide2/OPC_AABBCollider.cpp
			   		   src/cmd/collide2/OPC_AABBTree.cpp
			   		   src/cmd/collide2/OPC_BaseModel.cpp
			   		   src/cmd/collide2/OPC_BoxPruning.cpp
			   		   src/cmd/collide2/OPC_Collider.cpp 
			   		   src/cmd/collide2/OPC_Common.cpp
			   		   src/cmd/collide2/OPC_HybridModel.cpp
			   		   src/cmd/collide2/OPC_LSSCollider.cpp
			   		   src/cmd/collide2/OPC_MeshInterface.cpp 
			   		   src/cmd/collide2/OPC_Model.cpp
			   		   src/cmd/collide2/OPC_OBBCollider.cpp 
			   		   src/cmd/collide2/OPC_OptimizedTree.cpp 
			   		   src/cmd/collide2/OPC_Picking.cpp 
			   		   src/cmd/collide2/OPC_PlanesCollider.cpp 
			   		   src/cmd/collide2/OPC_RayCollider.cpp 
			   		   src/cmd/collide2/OPC_SphereCollider.cpp 
			   		   src/cmd/collide2/OPC_SweepAndPrune.cpp 
			   		   src/cmd/collide2/OPC_TreeBuilders.cpp 
			   		   src/cmd/collide2/OPC_TreeCollider.cpp 
			   		   src/cmd/collide2/OPC_VolumeCollider.cpp 
			   		   src/cmd/collide2/CSopcodecollider.cpp )

SET(LIBCMD_SOURCES src/cmd/alphacurve.cpp
				   src/cmd/asteroid_generic.cpp
				   src/cmd/beam_generic.cpp
				   src/cmd/bolt_generic.cpp
				   src/cmd/building_generic.cpp
				   src/cmd/collection.cpp
				   src/cmd/collide_map.cpp
				   src/cmd/collide.cpp
				   src/cmd/container.cpp
				   src/cmd/csv.cpp
				   src/cmd/faction_xml.cpp
				   src/cmd/missile_generic.cpp
				   src/cmd/mount.cpp
				   src/cmd/nebula_generic.cpp
				   src/cmd/planet_generic.cpp
				   src/cmd/role_bitmask.cpp
				   src/cmd/unit_bsp.cpp
				   src/cmd/unit_collide.cpp
				   src/cmd/unit_const_cache.cpp
				   src/cmd/unit_csv.cpp
				   src/cmd/unit_factory_generic.cpp 
				   src/cmd/unit_functions_generic.cpp 
				   src/cmd/unit_generic.cpp 
				   src/cmd/fg_util.cpp
				   src/cmd/pilot.cpp
				   src/cmd/unit_util_generic.cpp
				   src/cmd/unit_xml.cpp 
				   src/cmd/weapon_xml.cpp )

SET(LIBCMDSERVER_SOURCES src/cmd/atmosphere_server.cpp
						 src/cmd/base_util_server.cpp
						 src/cmd/beam_server.cpp
						 src/cmd/bolt_server.cpp
						 src/cmd/unit_factory_server.cpp
						 src/cmd/unit_functions_server.cpp
						 src/cmd/unit_util_server.cpp )


SET(LIBGFXGENERIC_SOURCES src/gfx/bounding_box.cpp
						  src/gfx/bsp.cpp
						  src/gfx/cockpit_generic.cpp
						  src/gfx/lerp.cpp
						  src/gfx/matrix.cpp
						  src/gfx/mesh_bxm.cpp
						  src/gfx/mesh_bin.cpp 
						  src/gfx/mesh_poly.cpp
						  src/gfx/mesh_xml.cpp
						  src/gfx/mesh.cpp
						  src/gfx/quaternion.cpp
						  src/gfx/sphere_generic.cpp
						  src/gfx/vec.cpp )

SET(LIBGFXCLIENT_SOURCES src/gfx/ani_texture.cpp
						 src/gfx/animation.cpp
						 src/gfx/aux_logo.cpp
						 src/gfx/aux_palette.cpp
						 src/gfx/aux_texture.cpp
						 src/gfx/background.cpp
						 src/gfx/camera.cpp
						 src/gfx/cockpit_xml.cpp
						 src/gfx/cockpit.cpp
						 src/gfx/coord_select.cpp
						 src/gfx/env_map_gent.cpp
						 src/gfx/gauge.cpp
						 src/gfx/halo_system.cpp
						 src/gfx/halo.cpp
						 src/gfx/hud.cpp
						 src/gfx/jpeg_memory.cpp
						 src/gfx/loc_select.cpp
						 src/gfx/masks.cpp
						 src/gfx/mesh_fx.cpp
						 src/gfx/mesh_gfx.cpp
						 src/gfx/nav/criteria_xml.cpp
						 src/gfx/nav/criteria.cpp
						 src/gfx/nav/drawgalaxy.cpp
						 src/gfx/nav/drawlist.cpp
						 src/gfx/nav/drawsystem.cpp
						 src/gfx/nav/navcomputer.cpp
						 src/gfx/nav/navgetxmldata.cpp
						 src/gfx/nav/navpath.cpp
						 src/gfx/nav/navscreen.cpp
						 src/gfx/nav/navscreenoccupied.cpp
						 src/gfx/particle.cpp
						 src/gfx/pipelined_texture.cpp
						 src/gfx/quadsquare_cull.cpp
						 src/gfx/quadsquare_render.cpp
						 src/gfx/quadsquare_update.cpp
						 src/gfx/quadsquare.cpp
						 src/gfx/quadtree_xml.cpp
						 src/gfx/quadtree.cpp
						 src/gfx/ring.cpp
						 src/gfx/screenshot.cpp
						 src/gfx/sphere.cpp
						 src/gfx/sprite.cpp 
						 src/gfx/star.cpp
						 src/gfx/stream_texture.cpp
						 src/gfx/tex_transform.cpp
						 src/gfx/vdu.cpp
						 src/gfx/vid_file.cpp
						 src/ffmpeg_init.cpp 
						 src/gfx/vsbox.cpp
						 src/gfx/vsimage.cpp 
						 src/gfx/warptrail.cpp )

SET(LIBGFXSERVER_SOURCES src/gfx/sphere_server.cpp
						 src/gfx/vsbox_server.cpp
						 src/gfx/mesh_server.cpp 
						 src/gfx/background_server.cpp )


#add_library(posh  src/posh.cpp )

SET(LIBROOTGENERIC_SOURCES src/configxml.cpp
						   src/easydom.cpp
						   src/endianness.cpp
						   src/macosx_math.cpp 
						   src/faction_generic.cpp
						   src/faction_util_generic.cpp
						   src/galaxy_gen.cpp
						   src/galaxy_xml.cpp
						   src/galaxy.cpp
						   src/hashtable.cpp
						   src/lin_time.cpp
						   src/load_mission.cpp
						   src/pk3.cpp 
						   src/posh.cpp
						   src/savegame.cpp
						   src/star_system_generic.cpp
						   src/star_system_xml.cpp
						   src/stardate.cpp 
						   src/universe_generic.cpp
						   src/universe_util_generic.cpp
						   src/vs_globals.cpp
						   src/vsfilesystem.cpp
						   src/xml_serializer.cpp
						   src/xml_support.cpp
						   src/options.cpp )

SET(LIBROOTCLIENT_SOURCES src/cg_global.cpp
						  src/command.cpp
						  src/config_xml.cpp
						  src/debug_vs.cpp
						  src/faction_util.cpp
						  src/force_feedback.cpp
						  src/gamemenu.cpp
						  src/gfxlib_struct.cpp
						  src/in_joystick.cpp
						  src/in_kb.cpp
						  src/in_main.cpp
						  src/in_mouse.cpp
						  src/in_sdl.cpp 
						  src/main_loop.cpp
						  src/physics.cpp
						  src/rendertext.cpp
						  src/ship_commands.cpp
						  src/star_system_jump.cpp
						  src/star_system.cpp
						  src/universe_util.cpp
						  src/universe.cpp )
						  
SET(LIBROOTSERVER_SOURCES  src/faction_util_server.cpp
						   src/force_feedback_server.cpp
						   src/gfxlib_struct_server.cpp
						   src/libaudioserver.cpp
						   src/libserver.cpp
						   src/universe_util_server.cpp )
						   
SET(LIBAUDIO_SOURCES src/audio/CodecRegistry.cpp
					 src/audio/Stream.cpp
					 src/audio/codecs/Codec.cpp
					 src/audio/codecs/FFStream.cpp
					 src/audio/codecs/OggStream.cpp
					 src/audio/codecs/OggData.cpp
					 src/audio/codecs/OggCodec.cpp
					 src/audio/codecs/FFCodec.cpp )


add_library(engine_com ${LIBAI_SOURCES}
					   ${LIBCMD_SOURCES}
					   ${LIBNET_SOURCES}
					   ${LIBPYTHON_SOURCES}
					   ${LIBROOTGENERIC_SOURCES}
					   ${LIBSCRIPT_SOURCES}
					   ${LIBGFXGENERIC_SOURCES} )
				   
SET(VEGASTRIKE_SOURCES ${LIBGUI_SOURCES}
					   ${LIBNETCLIENT_SOURCES}
					   ${LIBROOTCLIENT_SOURCES}
					   ${LIBGFXCLIENT_SOURCES}
					   ${LIBAUDIO_SOURCES}
					   src/aldrv/al_globals.cpp
					   src/aldrv/al_init.cpp
					   src/aldrv/al_listen.cpp 
					   src/aldrv/al_sound.cpp
					   src/cmd/ai/firekeyboard.cpp
					   src/cmd/ai/flyjoystick.cpp
					   src/cmd/ai/flykeyboard.cpp
					   src/cmd/ai/input_dfa.cpp
					   src/cmd/asteroid.cpp
					   src/cmd/atmosphere.cpp
					   src/cmd/base_init.cpp
					   src/cmd/base_interface.cpp
					   src/cmd/base_util.cpp
					   src/cmd/base_write_python.cpp
					   src/cmd/base_write_xml.cpp
					   src/cmd/base_xml.cpp
					   src/cmd/basecomputer.cpp
					   src/cmd/beam.cpp
					   src/cmd/bolt.cpp
					   src/cmd/briefing.cpp
					   src/cmd/building.cpp
					   src/cmd/click_list.cpp
					   src/cmd/cont_terrain.cpp
					   src/cmd/music.cpp
					   src/cmd/nebula.cpp
					   src/cmd/planet.cpp 
					   src/cmd/script/c_alike/c_alike.tab.cpp
					   src/cmd/script/c_alike/lex.yy.cpp 
					   src/cmd/script/director.cpp 
					   src/cmd/script/flightgroup.cpp
					   src/cmd/script/script_call_briefing.cpp
					   src/cmd/script/script_call_unit.cpp
					   src/cmd/terrain.cpp
					   src/cmd/unit_factory.cpp 
					   src/cmd/unit_functions.cpp 
					   src/cmd/unit_interface.cpp
					   src/cmd/unit_util.cpp
					   src/gldrv/gl_program.cpp
					   src/gldrv/gl_clip.cpp
					   src/gldrv/gl_fog.cpp
					   src/gldrv/gl_globals.cpp
					   src/gldrv/gl_init.cpp
					   src/gldrv/gl_light_pick.cpp
					   src/gldrv/gl_light_state.cpp
					   src/gldrv/gl_light.cpp
					   src/gldrv/gl_material.cpp
					   src/gldrv/gl_matrix.cpp
					   src/gldrv/gl_misc.cpp
					   src/gldrv/gl_quad_list.cpp
					   src/gldrv/gl_sphere_list.cpp
					   src/gldrv/gl_state.cpp
					   src/gldrv/sdds.cpp
					   src/gldrv/gl_texture.cpp
					   src/gldrv/gl_vertex_list.cpp
					   src/gldrv/winsys.cpp
					   src/main.cpp 
					   src/python/briefing_wrapper.cpp )

SET(VEGASERVER_SOURCES ${LIBCMDSERVER_SOURCES}
					   ${LIBNETSERVER_SOURCES}
					   ${LIBROOTSERVER_SOURCES}
					   ${LIBCMDSCRIPTSERVER_SOURCES}
					   ${LIBGFXSERVER_SOURCES}
					   src/vegaserver.cpp
					   src/cmd/script/director_server.cpp
					   src/gldrv/gl_vertex_list.cpp
					   src/gldrv/gl_sphere_list_server.cpp)
					   


add_executable(vegastrike ${VEGASTRIKE_SOURCES})
add_executable(vegaserver ${VEGASERVER_SOURCES})



include (CheckIncludeFiles)

# Let cmake find our in-tree modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${vsUTCS_SOURCE_DIR})


find_package(PythonLibs REQUIRED)
IF(PYTHONLIBS_FOUND OR PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
    SET(TST_INCLUDES ${PYTHON_INCLUDE_PATH})
    SET(TST_LIBS ${PYTHON_LIBRARIES})
    SET(HAVE_PYTHON 1)
ELSE(PYTHONLIBS_FOUND OR PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
    message(FATAL_ERROR "Can't find python")
ENDIF(PYTHONLIBS_FOUND OR PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)

# Find Boost with boost_python library
find_package(Boost COMPONENTS python)
IF(Boost_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${Boost_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${Boost_LIBRARIES})
	include_directories(${TST_INCLUDES})
ELSE(Boost_FOUND)
	message("-- Using Internal Boost !")
	IF(APPLE)
		SET(TST_INCLUDES ${TST_INCLUDES} ${vsUTCS_SOURCE_DIR}/boost/1_28)
		include_directories(${TST_INCLUDES})
		add_subdirectory(boost/1_28)
		SET(TST_LIBS ${TST_LIBS} boost_python)
	ELSE(APPLE)
		SET(TST_INCLUDES ${TST_INCLUDES} ${vsUTCS_SOURCE_DIR}/boost/1_35)
		include_directories(${TST_INCLUDES})	
		add_subdirectory(boost/1_35)
		SET(TST_LIBS ${TST_LIBS} boost_python)
	ENDIF(APPLE)
	add_dependencies(vegastrike boost_python)
	add_dependencies(vegaserver boost_python)
ENDIF(Boost_FOUND)

#Find FFMpeg

find_package(FFMPEG)
IF(FFMPEG_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${FFMPEG_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${FFMPEG_LIBRARIES})
	add_definitions(${FFMPEG_DEFINITIONS})
	SET(HAVE_FFMPEG 1 )
	IF(swscale1_FOUND)
		SET(HAVE_FFMPEG_SWSCALE_H 1)
	ENDIF(swscale1_FOUND)
ELSE(FFMPEG_FOUND)
	message("What the hell, no FFMpeg")
ENDIF(FFMPEG_FOUND)


#Find GL
find_package(OpenGL REQUIRED)
IF(OPENGL_FOUND AND OPENGL_GLU_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${OPENL_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${OPENGL_LIBRARIES})
	message("-- OpenGL found : ${OPENGL_LIBRARIES}")
ELSE(OPENGL_FOUND AND OPENGL_GLU_FOUND)
	message("Why you no have GL?")
ENDIF(OPENGL_FOUND AND OPENGL_GLU_FOUND)

#Find GLUT
find_package(GLUT REQUIRED)
IF(GLUT_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${GLUT_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${GLUT_LIBRARIES})
	message("-- GLUT found : ${GLUT_LIBRARIES}")
ELSE(GLUT_FOUND)
	message("I can't build this, missing GLUT")
ENDIF(GLUT_FOUND)


#Find OpenAL
find_package(OpenAL REQUIRED)
IF(OPENAL_FOUND)
	message("-- Found OpenAL")
	SET(TST_INCLUDES ${TST_INCLUDES} ${OPENAL_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${OPENAL_LIBRARY})
	SET(HAVE_AL 1)
ELSE(OPENAL_FOUND)
	message("We aint got no sound")
ENDIF(OPENAL_FOUND)

#Find JPEG
find_package(JPEG REQUIRED)
IF(JPEG_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${JPEG_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${JPEG_LIBRARIES})
ELSE(JPEG_FOUND)
	message("How are we gonna open jpegs?")
ENDIF(JPEG_FOUND)

#Find PNG
find_package(PNG REQUIRED)
IF(PNG_FOUND)
	SET(TEST_INCLUDES ${TST_INCLUDES} ${PNG_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${PNG_LIBRARIES})
ELSE(PNG_FOUND)
	message("Can't find PNG lib")
ENDIF(PNG_FOUND)

#Find SDL
find_package(SDL)
IF(SDL_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${SDL_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${SDL_LIBRARY})
	message("-- SDL Found")
	SET(HAVE_SDL 1)
	SET(SDL_WINDOWING 1)
ELSE(SDL_FOUND)
	message("How will we render to OpenGL without SDL?")
ENDIF(SDL_FOUND)

#find Vorbis
find_package(Vorbis REQUIRED)
IF(Vorbis_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${Vorbis_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${Vorbis_LIBRARIES})
	SET(HAVE_OGG 1)
ELSE(Vorbis_FOUND)
	message("Can't find Vorbis libs")
ENDIF(Vorbis_FOUND)

#find Zlib
find_package(ZLIB REQUIRED)
IF(ZLIB_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${ZLIB_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${ZLIB_LIBRARIES})
ELSE(ZLIB_FOUND)
	message("For the love of god, no zlib here")
ENDIF(ZLIB_FOUND)

#find Expat
find_package(EXPAT REQUIRED)
IF(EXPAT_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${EXPAT_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${EXPAT_LIBRARIES})
ELSE(EXPAT_FOUND)
	message("We can't find expat for XML crap")
ENDIF(EXPAT_FOUND)

#find Math
find_package(MATH REQUIRED)
IF(MATH_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${MATH_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${MATH_LIBRARIES})
ELSE(MATH_FOUND)
	message("Without math we have nothing")
ENDIF(MATH_FOUND)



# Still need to add CEGUI and OGRE find packages


#These are boolean options the user can enable/disable via ccmake
OPTION(USE_NET_THREAD_POSIX "Enable Posix net threads" OFF)
if(NOT USE_NET_THREAD_POSIX)
	SET(USE_NET_THREAD_NONE 1)
endif(NOT USE_NET_THREAD_POSIX)



#config.h generation
INCLUDE(CheckFunctionExists)
INCLUDE (CheckSymbolExists)
CHECK_INCLUDE_FILES(wchar.h HAVE_WCHAR_H)
CHECK_INCLUDE_FILES(float.h HAVE_FLOAT_H)
CHECK_INCLUDE_FILES(wctype.h HAVE_WCTYPE_H)
CHECK_INCLUDE_FILES(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES(zlib.h HAVE_ZLIB_H)
CHECK_INCLUDE_FILES(math.h HAVE_MATH_H)
if(HAVE_MATH_H)
	CHECK_FUNCTION_EXISTS(isnan HAVE_ISNAN)
	if(NOT HAVE_ISNAN)	
		CHECK_FUNCTION_EXISTS(_isnan HAVE__ISNAN)
	endif(NOT HAVE_ISNAN)
	CHECK_FUNCTION_EXISTS(finite HAVE_FINITE)
	if(NOT HAVE_FINITE)
		CHECK_FUNCTION_EXISTS(_finite HAVE__finite)
	endif(NOT HAVE_FINITE)
endif(HAVE_MATH_H)
CHECK_INCLUDE_FILES(sys/stat.h HAVE_STAT_H)
if(HAVE_STAT_H)
	CHECK_FUNCTION_EXISTS(lstat HAVE_LSTAT)
endif(HAVE_STAT_H)

CHECK_SYMBOL_EXISTS(GLX_ARB_get_proc_address GL/glx.h HAVE_GLXGETPROCADDRESSARB)

if(UNIX)
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		SET(HOSTOS "LINUX")
		SET(_GNU_SOURCE 1)
	else(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		SET(HOSTOS "BSD")
	endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	
elseif(WIN32)
	SET(HOSTOS "WIN32")
else(UNIX)
	SET(HOSTOS "APPLE")
endif(UNIX)

CONFIGURE_FILE(${vsUTCS_SOURCE_DIR}/cmake-config.h.in ${vsUTCS_BINARY_DIR}/config.h)

#end config.h generation 

#Setup shared lib and include listings for vegastrike and vegaserver.
SET(TST_LIBS ${TST_LIBS} engine_com OPcollide netgeneric netlowlevel)
SET(TST_SERVER_LIBS ${TST_LIBS} netgeneric engine_com netlowlevel OPcollide)
include_directories(${TST_INCLUDES})	


target_link_libraries(vegastrike ${TST_LIBS})
set_target_properties(vegastrike PROPERTIES LINK_FLAGS "-L/usr/lib")

target_link_libraries(vegaserver ${TST_SERVER_LIBS})
set_target_properties(vegaserver PROPERTIES LINK_FLAGS "-L/usr/lib -L${vsUTCS_BINARY_DIR}")

# Debug crap 
get_directory_property(TEMP_DIRECTORY INCLUDE_DIRECTORIES)
#message("Building with CFLAGS: ${CMAKE_CXX_FLAGS}")
#message("Linking with : ${TST_LIBS}")
#message("including : ${TEMP_DIRECTORY}")
# End Debug Crap 

# Vssetup Sub build file  
add_subdirectory(setup)

# Add other utilies here
add_subdirectory(objconv)

