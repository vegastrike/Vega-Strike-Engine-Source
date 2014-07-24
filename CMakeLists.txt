# create a build directory somewhere outside of this source tree
# run cmake <path to source tree> inside it
# run ccmake and hand tune any config variables/paths you need to, save 
# run make ...or build your project files
# valid targets are vegastrike, vssetup, and vegaserver (though it's broken right now)
# ccmake will let you set the Build type (debug,release,basic)


cmake_minimum_required(VERSION 2.6)

project (vsUTCS)

UNSET(PYTHONLIBS_FOUND)
UNSET(Boost_FOUND)
UNSET(OPENGL_FOUND)
UNSET(OPENGL_GLU_FOUND)
UNSET(GLUT_FOUND)
UNSET(OPENAL_FOUND)
UNSET(SDL_FOUND)
UNSET(VORBIS_FOUND)
UNSET(JPEG_FOUND)
UNSET(PNG_FOUND)
UNSET(ZLIB_FOUND)
UNSET(EXPAT_FOUND)
UNSET(MATH_FOUND)
UNSET(FFMPEG_FOUND)
UNSET(OGRE_FOUND)
UNSET(Boost_DIR)

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
				  src/cmd/ai/autodocking.cpp
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
				  src/cmd/ai/warpto.cpp
				  src/cmd/ai/flykeyboard_generic.cpp)

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
				   src/cmd/missile_generic.cpp
				   src/cmd/mount.cpp
				   src/cmd/nebula_generic.cpp
				   src/cmd/planet_generic.cpp
				   src/cmd/role_bitmask.cpp
				   src/cmd/unit_collide.cpp
				   src/cmd/unit_const_cache.cpp
				   src/cmd/unit_csv.cpp
				   src/cmd/unit_factory_generic.cpp 
				   src/cmd/unit_functions_generic.cpp 
				   src/cmd/unit_generic.cpp 
				   src/cmd/fg_util.cpp
				   src/cmd/unit_util_generic.cpp
				   src/cmd/unit_xml.cpp 
				   src/cmd/weapon_xml.cpp )

SET(LIBCMDSERVER_SOURCES src/cmd/atmosphere_server.cpp
						 src/cmd/base_util_server.cpp
						 src/cmd/beam_server.cpp
						 src/cmd/bolt_server.cpp
						 src/cmd/unit_factory_server.cpp
						 src/cmd/unit_functions_server.cpp
						 src/cmd/unit_util_server.cpp
                         src/cmd/unit_server.cpp 
                         src/cmd/pilot_server.cpp 
                         src/cmd/images_server.cpp 
                         src/cmd/faction_xml_server.cpp )


SET(LIBGFXGENERIC_SOURCES src/gfx/cockpit_generic.cpp
						  src/gfx/lerp.cpp
						  src/gfx/matrix.cpp
						  src/gfx/mesh_bxm.cpp
						  src/gfx/mesh_poly.cpp
						  src/gfx/mesh_xml.cpp
						  src/gfx/mesh.cpp
						  src/gfx/quaternion.cpp
                                                  src/gfx/soundcontainer_generic.cpp
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
                                                 src/gfx/mesh_bin.cpp
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
						 src/gfx/radar/bubble_display.cpp 
						 src/gfx/radar/bubble_display.h 
						 src/gfx/radar/dual_display.cpp 
						 src/gfx/radar/dual_display.h 
						 src/gfx/radar/null_display.h 
						 src/gfx/radar/plane_display.cpp 
						 src/gfx/radar/plane_display.h 
						 src/gfx/radar/radar.cpp 
						 src/gfx/radar/radar.h 
						 src/gfx/radar/sensor.cpp 
						 src/gfx/radar/sensor.h 
						 src/gfx/radar/sphere_display.cpp 
						 src/gfx/radar/sphere_display.h 
						 src/gfx/radar/track.cpp 
						 src/gfx/radar/track.h 
						 src/gfx/radar/viewarea.cpp 
						 src/gfx/radar/viewarea.h 
						 src/gfx/particle.cpp
						 src/gfx/pipelined_texture.cpp
						 src/gfx/quadsquare_cull.cpp
						 src/gfx/quadsquare_render.cpp
						 src/gfx/quadsquare_update.cpp
						 src/gfx/quadsquare.cpp
						 src/gfx/quadtree_xml.cpp
						 src/gfx/quadtree.cpp
						 src/gfx/ring.cpp
                                                 src/gfx/occlusion.cpp
						 src/gfx/screenshot.cpp
                                                 src/gfx/soundcontainer.cpp
                                                 src/gfx/soundcontainer_aldrv.cpp
						 src/gfx/sphere.cpp
						 src/gfx/sprite.cpp 
						 src/gfx/star.cpp
						 src/gfx/stream_texture.cpp
						 src/gfx/technique.cpp
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
						 src/gfx/background_server.cpp
                        src/gfx/mesh_bin_server.cpp )

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
						   src/VSFileXMLSerializer.cpp
						   src/xml_serializer.cpp
						   src/xml_support.cpp
						   src/XMLDocument.cpp
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
						  
SET(LIBROOTSERVER_SOURCES  src/force_feedback_server.cpp
						   src/faction_util_server.cpp
						   src/gfxlib_struct_server.cpp
						   src/libaudioserver.cpp
						   src/libserver.cpp
						   src/universe_util_server.cpp )
						   
SET(LIBAUDIO_SOURCES src/audio/CodecRegistry.cpp
                     src/audio/Listener.cpp
                     src/audio/RenderableListener.cpp
                     src/audio/RenderableSource.cpp
                     src/audio/Renderer.cpp
                     src/audio/Scene.cpp
                     src/audio/SceneManager.cpp
                     src/audio/SimpleScene.cpp
                     src/audio/SimpleSound.cpp
                     src/audio/SimpleSource.cpp
                     src/audio/SoundBuffer.cpp
                     src/audio/Sound.cpp
                     src/audio/Source.cpp
                     src/audio/SourceTemplate.cpp
                     src/audio/Stream.cpp
                     src/audio/TemplateManager.cpp
                     src/audio/test.cpp
                     src/audio/utils.cpp
                     src/audio/codecs/Codec.cpp
                     src/audio/codecs/FFCodec.cpp
                     src/audio/codecs/FFStream.cpp
                     src/audio/codecs/OggCodec.cpp
                     src/audio/codecs/OggData.cpp
                     src/audio/codecs/OggStream.cpp
                     src/audio/renderers/OpenAL/OpenALHelpers.cpp
                     src/audio/renderers/OpenAL/OpenALRenderableListener.cpp
                     src/audio/renderers/OpenAL/OpenALRenderableSource.cpp
                     src/audio/renderers/OpenAL/OpenALRenderableStreamingSource.cpp
                     src/audio/renderers/OpenAL/OpenALRenderer.cpp
                     src/audio/renderers/OpenAL/OpenALSimpleSound.cpp
                     src/audio/renderers/OpenAL/OpenALStreamingSound.cpp
                     )


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
					   src/cmd/unit_util.cpp
                       src/cmd/faction_xml.cpp
                       src/cmd/pilot.cpp
                       src/cmd/images.cpp
                       src/cmd/unit.cpp
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
					   src/cmd/ai/flykeyboard_server.cpp
					   src/gldrv/gl_vertex_list.cpp
					   src/gldrv/gl_sphere_list_server.cpp)
					   
if(COMMAND cmake_policy)					   
	cmake_policy(SET CMP0003 OLD) 
endif(COMMAND cmake_policy)

# Set the default build type to Release
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Release, Debug, Profiler"
      FORCE )
ENDIF(NOT CMAKE_BUILD_TYPE)

IF(NOT BUILD_OPT)
	SET(BUILD_OPT -O2 CACHE STRING
      "-O0, -O1, -O2, -O3, -Os, -Ofast"
      FORCE )
ENDIF(NOT BUILD_OPT)



# Provide boolean options for enabling various cpu optimizations.
OPTION(CPUAMD_k8  "Enable AMD K8 optimizations (Athlon through athlon64)" 
    OFF )
OPTION(CPUAMD_k9  "Enable AMD K9 (sse3) optimizations (Athlon64 AM3) "
    OFF )
OPTION(CPUAMD_k10  "Enable AMD K10 optimizations (Athlon2)"
    OFF )
OPTION(CPUAMD_vishera  "Enable AMD Vishera optimizations"
    OFF )

OPTION(CPUINTEL_p4 "Enable Intel P4 optimizations (1st gen P4's)"
    OFF )
OPTION(CPUINTEL_prescott  "Enable Intel P4 (sse3) optimizations"
    OFF )
OPTION(CPUINTEL_nocona "Enable Intel P4 (64bit) optimizations"
    OFF )
OPTION(CPUINTEL_core2 "Enable Intel Core2 optimizations"
    OFF )
OPTION(CPUINTEL_native "Enable host-specific optimizations for this machine"
    OFF )

SET(DATADIR "" CACHE STRING "Default vegastrike data directory")

# Hide our temporary variable and initialize it to empty
SET(CPU_OPTS  )
SET(DEFINES " -DNV_CUBE_MAP -DBOOST_PYTHON_NO_PY_SIGNATURES ")
MARK_AS_ADVANCED( CPU_OPTS )
MARK_AS_ADVANCED( DEFINES )
SET(CPU_SMP "1" CACHE STRING 
    "Number of CPU's in system")
# Depending on the boolean chosen above, set CPU_OPTS to the correct cpu opts

IF(CPUAMD_k8)
    SET(CPU_OPTS "-mtune=athlon64 -mfpmath=sse -msse2 -mmmx -m3dnow -ftree-vectorize -ffast-math -fassociative-math -funsafe-math-optimizations -fprefetch-loop-arrays ")
ELSEIF(CPUAMD_K9)
    SET(CPU_OPTS "-mtune=k8-sse3 -mfpmath=sse -msse3 -mmmx -m3dnow -ftree-vectorize -ffast-math -fassociative-math -funsafe-math-optimizations -funroll-loops ")
ELSEIF(CPUINTEL_p4)
    SET(CPU_OPTS "-mtune=pentium4 -mfpmath=sse -msse2 -mmmx -ftree-vectorize -ffast-math -fassociative-math -funsafe-math-optimizations -funroll-loops ")
ELSEIF(CPUINTEL_prescott)
    SET(CPU_OPTS "-mtune=prescott -mfpmath=sse -msse3 -mmmx -ftree-vectorize -ffast-math -fassociative-math -funsafe-math-optimizations -funroll-loops " )
ELSEIF(CPUINTEL_nocona)
    SET(CPU_OPTS "-mtune=nocona -mfpmath=sse -msse3 -mmmx -ftree-vectorize -ffast-math -fassociative-math -funsafe-math-optimizations -funroll-loops ")
ELSEIF(CPUINTEL_core2)
    SET(CPU_OPTS "-mtune=core2 -mfpmath=sse -msse3 -mmmx -ftree-vectorize -ffast-math -fassociative-math -funsafe-math-optimizations -funroll-loops ")
ELSEIF(CPUINTEL_native)
    SET(CPU_OPTS "-mtune=native -march=native -mfpmath=sse -msse3 -mmmx -ftree-vectorize -ffast-math -fassociative-math -funsafe-math-optimizations -funroll-loops ")
ELSEIF(CPUAMD_vishera)
    SET(CPU_OPTS "-march=bdver2 -mfpmath=both -mmmx -msse4a -msse4.2 -maes -mpclmul -mfsgsbase -mrdrnd -mf16c -mfma -mfma4 -m3dnow -mxop -mlwp -mtbm -mavx2 -mvzeroupper -msahf -mcx16 -funsafe-loop-optimizations ") 
ENDIF(CPUAMD_k8)

IF(CPU_SMP GREATER 1)
    SET(CPU_OPTS " ${CPU_OPTS} -ftree-parallelize-loops=${CPU_SMP} ")
ENDIF(CPU_SMP GREATER 1)

IF(DATADIR)
    SET(DEFINES "${DEFINES} -DDATA_DIR=\"${DATADIR}\"")
ENDIF(DATADIR)

# Debug target block. 
SET(CMAKE_CXX_FLAGS_DEBUG " ${BUILD_OPT} ${CPU_OPTS} ${DEFINES} -include config.h -pipe -g2 -Wall -fvisibility=hidden" CACHE STRING
    "Flags used by the C++ compiler during debug builds."
     FORCE )
SET( CMAKE_C_FLAGS_DEBUG "  ${BUILD_OPT} ${CPU_OPTS} ${DEFINES} -include config.h -pipe -g2 -Wall -Wno-unused-function -Wno-unused-variable -fvisibility=hidden" CACHE STRING
    "Flags used by the C compiler during debug builds."
     FORCE )
SET( CMAKE_EXE_LINKER_FLAGS_DEBUG
    "-Bsymbolic " CACHE STRING
    "Flags used for linking binaries during debug builds."
     FORCE )
MARK_AS_ADVANCED(
	CMAKE_CXX_FLAGS_DEBUG
	CMAKE_C_FLAGS_DEBUG)
IF("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	MARK_AS_ADVANCED( CLEAR
    	CMAKE_CXX_FLAGS_DEBUG )
ELSE("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    MARK_AS_ADVANCED( FORCE 
    	CMAKE_CXX_FLAGS_DEBUG )
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")



# Release Target block 
SET(CMAKE_CXX_FLAGS_RELEASE  " ${BUILD_OPT} ${CPU_OPTS} -DNV_CUBE_MAP -DBOOST_PYTHON_NO_PY_SIGNATURES -include config.h -pipe -Wall -fvisibility=hidden" CACHE STRING
    "Flags used by the C++ compiler during release builds."
    FORCE)
SET( CMAKE_C_FLAGS_RELEASE " ${BUILD_OPT} ${CPU_OPTS} -DNV_CUBE_MAP -DBOOST_PYTHON_NO_PY_SIGNATURES -include config.h -pipe -Wall -Wno-unused-function -Wno-unused-variable -fvisibility=hidden" CACHE STRING
    "Flags used by the C compiler during release builds." 
    FORCE )
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Bsymbolic" CACHE STRING "Release linker flags"
     FORCE )
MARK_AS_ADVANCED(
    CMAKE_C_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_EXE_LINKER_FLAGS_RELEASE)
IF("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	MARK_AS_ADVANCED( CLEAR
    	CMAKE_CXX_FLAGS_RELEASE )
ELSE("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	MARK_AS_ADVANCED( FORCE
		CMAKE_CXX_FLAGS_RELEASE )
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "Release")

# Profiler Target block
SET(CMAKE_CXX_FLAGS_PROFILER " ${BUILD_OPT} ${CPU_OPTS} -pg -g2 -DNV_CUBE_MAP -DBOOST_PYTHON_NO_PY_SIGNATURES -include config.h -pipe -Wall -Wno-unused-function -Wno-unused-variable" CACHE STRING
    "Flags used by the C++ compiler during profiler builds."
     FORCE )
SET( CMAKE_C_FLAGS_PROFILER " ${BUILD_OPT} ${CPU_OPTS} -pg -g2 -DNV_CUBE_MAP -DBOOST_PYTHON_NO_PY_SIGNATURES -include config.h -pipe -Wall -Wno-unused-function -Wno-unused-variable" CACHE STRING
    "Flags used by the C compiler during profiler builds."
     FORCE )
SET( CMAKE_EXE_LINKER_FLAGS_PROFILER
    "-Bsymbolic " CACHE STRING
    "Flags used for linking binaries during profiler builds."
     FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_PROFILER
    CMAKE_C_FLAGS_PROFILER
    CMAKE_EXE_LINKER_FLAGS_PROFILER
    CMAKE_SHARED_LINKER_FLAGS_PROFILER )
IF("${CMAKE_BUILD_TYPE}" STREQUAL "Profiler")
	MARK_AS_ADVANCED( CLEAR
    	CMAKE_CXX_FLAGS_PROFILER )
ELSE("${CMAKE_BUILD_TYPE}" STREQUAL "Profiler")   
	MARK_AS_ADVANCED( FORCE
		 CMAKE_CXX_FLAGS_PROFILER )
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "Profiler")


# Turn off compiling vegastrike bin
OPTION(DISABLE_CLIENT "Disable building the vegastrike bin"
		OFF )
IF(NOT DISABLE_CLIENT)
	add_executable(vegastrike ${VEGASTRIKE_SOURCES})
ENDIF(NOT DISABLE_CLIENT)

OPTION(DISABLE_SERVER "Disable building the vegaserver bin"
        OFF )
IF(NOT DISABLE_SERVER)
    add_executable(vegaserver ${VEGASERVER_SOURCES})
ENDIF(NOT DISABLE_SERVER)


include (CheckIncludeFile)
include (CheckIncludeFileCXX)
INCLUDE(CheckTypeSize)
check_type_size("void*" SIZEOF_VOID_P BUILTIN_TYPES_ONLY)
# Let cmake find our in-tree modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${vsUTCS_SOURCE_DIR})

# Configurable python version (some data sets might need 3.x, some 2.x)
SET(VS_PYTHON_VERSION 2 CACHE STRING "Python version to build against: 2, 3, 2.x, 3.x")
SET(Python_ADDITIONAL_VERSIONS 3.3mu 3.3m 3.2mu 3.2m)

find_package(PythonLibs ${VS_PYTHON_VERSION} REQUIRED)
IF(PYTHONLIBS_FOUND OR PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
    message("Found Python " ${PYTHONLIBS_VERSION_STRING})
    # Multi-arch systems with python 3.3 need to check 64bit include dir
    # and the regular generic include dir, cmake's python search doesn't do that
    # This is a hack block to workaround it until we decide to include our own 
    # python search and fix it.  
    # If the additional include dir doesn't match everyone's setup, add other versions (mu)
    string(REGEX MATCH "3\\.3\\..*" FOUND_33 "${PYTHONLIBS_VERSION_STRING}")
    IF(FOUND_33 AND SIZEOF_VOID_P EQUAL 8)
       #  THIS IS 64 bit and version 3.3 ####################
       SET(TST_INCLUDES ${PYTHON_INCLUDE_PATH} "/usr/include/x86_64-linux-gnu/python3.3m")
    ELSE(FOUND_33 AND SIZEOF_VOID_P EQUAL 8)
       IF(FOUND_33)
          #  THIS IS 32bit and 3.3       #######################
          SET(TST_INCLUDES ${PYTHON_INCLUDE_PATH} "/usr/include/i386-linux-gnu/python3.3m")
       ELSE(FOUND_33)
          #  THIS IS NOT 3.3 so who cares   ####################
          SET(TST_INCLUDES ${PYTHON_INCLUDE_PATH})
       ENDIF(FOUND_33)
   ENDIF(FOUND_33 AND SIZEOF_VOID_P EQUAL 8)
   #  What the above block says is if we're 3.3 then our distro may be putting
   #  pyconfig.h in a arch specific dir. This is not included by default yet
   #  If we're not 3.3 then do the normal thing
       
   SET(TST_LIBS ${PYTHON_LIBRARIES})
   SET(HAVE_PYTHON 1)
ELSE(PYTHONLIBS_FOUND OR PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
    message(FATAL_ERROR "Can't find python3")
ENDIF(PYTHONLIBS_FOUND OR PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)

# Find Boost with boost_python library
OPTION(USE_SYSTEM_BOOST "Use system boost" OFF)
IF(USE_SYSTEM_BOOST)
  find_package( Boost 1.40.0 COMPONENTS python )
  IF(Boost_FOUND)
    message("++ Found System Boost : ${Boost_VERSION}")
    SET(BOOST_PYTHON_NO_PY_SIGNATURES 1)
    SET(TST_INCLUDES ${TST_INCLUDES} ${Boost_INCLUDE_DIRS})
    SET(TST_LIBS ${TST_LIBS} ${Boost_LIBRARIES})
    include_directories(${TST_INCLUDES})
  ELSE(Boost_FOUND)
    SET(USE_SYSTEM_BOOST OFF)
  ENDIF(Boost_FOUND)
ENDIF(USE_SYSTEM_BOOST)

IF(NOT USE_SYSTEM_BOOST)
    message("++ Using Internal Boost")
# This is just to cleanup advanced variable viewing, since they're not used anyway
    UNSET(Boost_INCLUDE_DIRS)
    UNSET(Boost_LIBRARIES)
    UNSET(Boost_INCLUDE_DIR)
    UNSET(Boost_LIBRARY_DIRS)
    UNSET(Boost_PYTHON_LIBRARY)
    UNSET(Boost_PYTHON_LIBRARY_DEBUG)
    UNSET(Boost_PYTHON_LIBRARY_RELEASE)
    SET(Boost_DIR 1_53_0)
    SET(BOOST_PYTHON_NO_PY_SIGNATURES 1)
    SET(TST_INCLUDES ${TST_INCLUDES} ${vsUTCS_SOURCE_DIR}/boost/1_53)
    SET(TST_LIBS ${TST_LIBS} boost_python)
    include_directories(${TST_INCLUDES})
    add_subdirectory(boost/1_53)
    IF(NOT DISABLE_CLIENT)
	add_dependencies(vegastrike boost_python)
    ENDIF(NOT DISABLE_CLIENT)
    IF(NOT DISABLE_SERVER)
	add_dependencies(vegaserver boost_python)
    ENDIF(NOT DISABLE_SERVER)
ENDIF(NOT USE_SYSTEM_BOOST)

IF(NOT DISABLE_CLIENT) ##########

#Find GL
find_package(OpenGL REQUIRED)
IF(OPENGL_FOUND AND OPENGL_GLU_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${OPENL_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${OPENGL_LIBRARIES})
	message("++ OpenGL found : ${OPENGL_LIBRARIES}")
ELSE(OPENGL_FOUND AND OPENGL_GLU_FOUND)
	message("!! Why you no have GL?")
ENDIF(OPENGL_FOUND AND OPENGL_GLU_FOUND)

#Find GLUT
find_package(GLUT REQUIRED)
IF(GLUT_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${GLUT_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${GLUT_LIBRARIES})
	message("++ GLUT found : ${GLUT_LIBRARIES}")
ELSE(GLUT_FOUND)
	message("!! I can't build this, missing GLUT")
ENDIF(GLUT_FOUND)


#Find OpenAL
find_package(OpenAL REQUIRED)
IF(OPENAL_FOUND)
	message("++ Found OpenAL")
	SET(TST_INCLUDES ${TST_INCLUDES} ${OPENAL_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${OPENAL_LIBRARY})
	SET(HAVE_AL 1)
ELSE(OPENAL_FOUND)
	message("!! We aint got no sound")
ENDIF(OPENAL_FOUND)

IF (NOT BEOS)
#Find SDL
find_package(SDL)
IF(SDL_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${SDL_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${SDL_LIBRARY})
	message("++ SDL Found")
	SET(HAVE_SDL 1)
	SET(SDL_WINDOWING 1)
ELSE(SDL_FOUND)
	message("!! How will we render to OpenGL without SDL?")
ENDIF(SDL_FOUND)
ENDIF (NOT BEOS)

#find Vorbis
find_package(Vorbis REQUIRED)
IF(Vorbis_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${Vorbis_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${Vorbis_LIBRARIES})
	SET(HAVE_OGG 1)
ELSE(Vorbis_FOUND)
	message("!! Can't find Vorbis libs")
ENDIF(Vorbis_FOUND)

#Find JPEG
find_package(JPEG REQUIRED)
IF(JPEG_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${JPEG_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${JPEG_LIBRARIES})
ELSE(JPEG_FOUND)
	message("!! How are we gonna open jpegs?")
ENDIF(JPEG_FOUND)

#Find PNG
find_package(PNG REQUIRED)
IF(PNG_FOUND)
	SET(TEST_INCLUDES ${TST_INCLUDES} ${PNG_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${PNG_LIBRARIES})
ELSE(PNG_FOUND)
	message("!! Can't find PNG lib")
ENDIF(PNG_FOUND)

ENDIF(NOT DISABLE_CLIENT) ##########

#find Zlib
find_package(ZLIB REQUIRED)
IF(ZLIB_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${ZLIB_INCLUDE_DIR})
	SET(TST_LIBS ${TST_LIBS} ${ZLIB_LIBRARIES})
ELSE(ZLIB_FOUND)
	message("!! For the love of god, no zlib here")
ENDIF(ZLIB_FOUND)

#find Expat
find_package(EXPAT REQUIRED)
IF(EXPAT_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${EXPAT_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${EXPAT_LIBRARIES})
ELSE(EXPAT_FOUND)
	message("!! We can't find expat for XML crap")
ENDIF(EXPAT_FOUND)

#find Math
IF (NOT BEOS)
find_package(MATH REQUIRED)
IF(MATH_FOUND)
	SET(TST_INCLUDES ${TST_INCLUDES} ${MATH_INCLUDE_DIRS})
	SET(TST_LIBS ${TST_LIBS} ${MATH_LIBRARIES})
ELSE(MATH_FOUND)
	message("!! Without math we have nothing")
ENDIF(MATH_FOUND)
ENDIF (NOT BEOS)

#Find FFMpeg

OPTION(DISABLE_FFMPEG "Disable ffmpeg codecs" OFF)

IF(NOT DISABLE_FFMPEG)
  find_package(FFMPEG)
  IF(FFMPEG_FOUND)
	  SET(TST_INCLUDES ${TST_INCLUDES} ${FFMPEG_INCLUDE_DIRS})
	  SET(TST_LIBS ${TST_LIBS} ${FFMPEG_LIBRARIES})
	  add_definitions(${FFMPEG_DEFINITIONS})
	  SET(HAVE_FFMPEG 1 )
    message("++ FFmpeg found.")
	  IF(swscale1_FOUND)
          message("++ FFmpeg's libswscale found.")
	  ELSE(swscale1_FOUND)
          message("-- FFMpeg's libswscale not found... depending on your ffmpeg version, VS might not build.")
	  ENDIF(swscale1_FOUND)
  ELSE(FFMPEG_FOUND)
	  message("-- FFMPEG Not Found")
  ENDIF(FFMPEG_FOUND)
ELSE(NOT DISABLE_FFMPEG)
  message("-- FFMPEG disabled")
ENDIF(NOT DISABLE_FFMPEG)

#Find Ogre

find_package(OGRE)
IF(OGRE_FOUND)
    SET(TST_INCLUDES ${TST_INCLUDES} ${OGRE_INCLUDE_DIR})
    SET(TST_LIBS ${TST_LIBS} ${OGRE_LIBRARY})
    add_definitions(${OGRE_DEFINITIONS})
    SET(HAVE_OGRE 1)
    message("++ Found Ogre: ${OGRE_VERSION}")
ELSE(OGRE_FOUND)
    message("-- Ogre Not Found: compiling without")
ENDIF(OGRE_FOUND)

IF(NOT BEOS)
	FIND_LIBRARY(UTIL_LIB util)
ELSE (NOT BEOS)
	FIND_LIBRARY(UTIL_LIB network)
ENDIF(NOT BEOS)

SET(TST_LIBS ${TST_LIBS} ${UTIL_LIB})

# Still need to add CEGUI and OGRE find packages


# Multiplayer networking option 
OPTION(USE_NET_THREAD_POSIX "Enable Posix net threads" OFF)
if(NOT USE_NET_THREAD_POSIX)
	SET(USE_NET_THREAD_NONE 1)
endif(NOT USE_NET_THREAD_POSIX)



#config.h generation
INCLUDE(CheckFunctionExists)
INCLUDE (CheckSymbolExists)
CHECK_INCLUDE_FILE(wchar.h HAVE_WCHAR_H)
CHECK_INCLUDE_FILE(float.h HAVE_FLOAT_H)
CHECK_INCLUDE_FILE(wctype.h HAVE_WCTYPE_H)
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(zlib.h HAVE_ZLIB_H)
CHECK_INCLUDE_FILE(math.h HAVE_MATH_H)
CHECK_INCLUDE_FILE_CXX(tr1/unordered_map HAVE_TR1_UNORDERED_MAP)

CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)
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
CHECK_INCLUDE_FILE(sys/stat.h HAVE_STAT_H)
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
elseif(BEOS)
	SET(HOSTOS "HAIKU")
else(UNIX)
	SET(HOSTOS "APPLE")
endif(UNIX)

CONFIGURE_FILE(${vsUTCS_SOURCE_DIR}/cmake-config.h.in ${vsUTCS_BINARY_DIR}/config.h)

#end config.h generation 
#SET(CMAKE_CXX_FLAGS "-include config.h;-pipe;"${CMAKE_CXX_FLAGS})

#Setup shared lib and include listings for vegastrike and vegaserver.
SET(TST_LIBS engine_com OPcollide netgeneric netlowlevel ${TST_LIBS})
SET(TST_SERVER_LIBS netgeneric engine_com netlowlevel OPcollide ${TST_LIBS})
include_directories(${TST_INCLUDES})	

IF(NOT DISABLE_CLIENT)
	target_link_libraries(vegastrike ${TST_LIBS})
	set_target_properties(vegastrike PROPERTIES LINK_FLAGS "-L/usr/lib ${TST_LFLAGS}")
ENDIF(NOT DISABLE_CLIENT)

# Vssetup Sub build file  
add_subdirectory(setup)

#ADD_CUSTOM_TARGET(vssetup DEPENDS setup/vssetup COMMAND ln -fs ${vsUTCS_BINARY_DIR}/setup/vssetup vssetup || true)

# Add other utilies here
add_subdirectory(objconv)

#ADD_CUSTOM_TARGET(mesher DEPENDS objconv/mesher COMMAND ln -fs ${vsUTCS_BINARY_DIR}/objconv/mesher mesher || true)


IF(NOT DISABLE_SERVER)
target_link_libraries(vegaserver ${TST_SERVER_LIBS})
set_target_properties(vegaserver PROPERTIES LINK_FLAGS "-L/usr/lib -L${vsUTCS_BINARY_DIR}"
                                            COMPILE_FLAGS "-DNO_GFX")
add_dependencies(vegaserver mesh_tool)
add_dependencies(vegaserver vssetup)
ENDIF(NOT DISABLE_SERVER)

# Debug crap 
get_directory_property(TEMP_DIRECTORY INCLUDE_DIRECTORIES)
message("Default build type is Release, no cpu opts enabled. ")
#message("Building with CFLAGS: ${CMAKE_CXX_FLAGS}")
#message("Linking with : ${TST_LIBS}")
#message("including : ${TEMP_DIRECTORY}")
# End Debug Crap 

MARK_AS_ADVANCED(FORCE
	Boost_DIR
	AVCODEC_FOUND
	AVFORMAT_FOUND
	AVUTIL_FOUND
	AVCODEC_LIBRARY
	AVCODEC_INCLUDE_DIR
	AVUTIL_INCLUDE_DIR
	AVFORMAT_LIBRARY
	AVUTIL_LIBRARY
	SWSCALE_INCLUDE_DIR
	SWSCALE_LIBRARY
	AVFORMAT_INCLUDE_DIR
	HAVE_AVFORMAT_H
	HAVE_AVCODEC_H
	HAVE_AVIO_H
	HAVE_FFMPEG_SWSCALE_H
	CMAKE_BACKWARDS_COMPATIBILITY
	FFMPEG_INCLUDE_DIR
	FFMPEG_FOUND
	FFMPEG_LIBRARIES
	FFMPEG_DEFINITIONS
	GTK2_ATK_INCLUDE_DIR
	GTK2_ATK_LIBRARY
	GTK2_CAIRO_INCLUDE_DIR
	GTK2_CAIRO_LIBRARY
	GTK2_GDK_INCLUDE_DIR
	GTK2_GDK_LIBRARY
	GTK2_GDK_PIXBUF_LIBRARY
	GTK2_GDK_PIXBUF_INCLUDE_DIR
	GTK2_GLIBCONFIG_INCLUDE_DIR
	GTK2_GLIB_INCLUDE_DIR
	GTK2_GLIB_LIBRARY
	GTK2_GMODULE_LIBRARY
	GTK2_GOBJECT_LIBRARY
	GTK2_PANGO_INCLUDE_DIR
	GTK2_PANGO_LIBRARY
	GTK2_GTHREAD_LIBRARY
	GTK2_GTKGL_INCLUDE_DIR
	GTK2_GTKGL_LIBRARY
	GTK2_GTK_INCLUDE_DIR
	GTK2_GTK_LIBRARY
	LIBRARY_OUTPUT_PATH
	OPENAL_INCLUDE_DIR
	OPENAL_LIBRARY
	SDLMAIN_LIBRARY
	SDL_INCLUDE_DIR
	SDL_LIBRARY
	UTIL_LIB
	ogg_INCLUDE_DIR
	ogg_LIBRARY
	vorbis_INCLUDE_DIR
	vorbis_LIBRARY
	vorbisfile_LIBRARY )
