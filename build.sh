#!/bin/sh
#make $*
c++  -ggdb3 -Wall -I/usr/include/SDL -D_REENTRANT -pipe  -L/usr/X11R6/lib -L/usr/X11R6/lib  -o vegastrike \
\
../vegastrike/src/faction_util.o		\
../vegastrike/src/faction_util_generic.o		\
../vegastrike/src/config_xml.o		\
../vegastrike/src/configxml.o		\
../vegastrike/src/debug_vs.o		\
../vegastrike/src/easydom.o		\
../vegastrike/src/endianness.o		\
../vegastrike/src/faction_generic.o		\
../vegastrike/src/force_feedback.o		\
../vegastrike/src/galaxy.o		\
../vegastrike/src/galaxy_gen.o		\
../vegastrike/src/galaxy_xml.o		\
../vegastrike/src/gfxlib_struct.o		\
../vegastrike/src/hashtable.o		\
../vegastrike/src/in_handler.o		\
../vegastrike/src/in_joystick.o		\
../vegastrike/src/in_kb.o		\
../vegastrike/src/in_main.o		\
../vegastrike/src/in_mouse.o		\
../vegastrike/src/in_sdl.o		\
../vegastrike/src/lin_time.o		\
../vegastrike/src/load_mission.o		\
../vegastrike/src/macosx_math.o		\
../vegastrike/src/main.o		\
../vegastrike/src/main_loop.o		\
../vegastrike/src/md5.o		\
../vegastrike/src/physics.o		\
../vegastrike/src/savegame.o		\
../vegastrike/src/star_system.o		\
../vegastrike/src/star_system_generic.o		\
../vegastrike/src/star_system_jump.o		\
../vegastrike/src/star_system_xml.o		\
../vegastrike/src/stardate.o		\
../vegastrike/src/universe.o		\
../vegastrike/src/universe_generic.o		\
../vegastrike/src/universe_util.o		\
../vegastrike/src/universe_util_generic.o		\
../vegastrike/src/vs_globals.o		\
../vegastrike/src/vs_path.o		\
../vegastrike/src/xml_serializer.o		\
../vegastrike/src/xml_support.o		\
\
\
\
\
\
../vegastrike/src/cmd/ai/aggressive.o		\
\
../vegastrike/src/cmd/ai/comm_ai.o		\
\
../vegastrike/src/cmd/ai/communication.o		\
\
../vegastrike/src/cmd/ai/communication_xml.o		\
../vegastrike/src/cmd/ai/docking.o		\
\
../vegastrike/src/cmd/ai/event_xml.o		\
\
../vegastrike/src/cmd/ai/fire.o		\
\
../vegastrike/src/cmd/ai/fireall.o		\
\
../vegastrike/src/cmd/ai/firekeyboard.o		\
\
../vegastrike/src/cmd/ai/flybywire.o		\
\
../vegastrike/src/cmd/ai/flyjoystick.o		\
\
../vegastrike/src/cmd/ai/flykeyboard.o		\
\
../vegastrike/src/cmd/ai/hard_coded_scripts.o		\
\
../vegastrike/src/cmd/ai/ikarus.o		\
\
../vegastrike/src/cmd/ai/input_dfa.o		\
\
../vegastrike/src/cmd/ai/missionscript.o		\
../vegastrike/src/cmd/ai/navigation.o		\
\
../vegastrike/src/cmd/ai/order.o		\
\
../vegastrike/src/cmd/ai/order_comm.o		\
../vegastrike/src/cmd/ai/script.o		\
\
../vegastrike/src/cmd/ai/tactics.o		\
\
../vegastrike/src/cmd/ai/turretai.o		\
\
../vegastrike/src/cmd/ai/warpto.o		\
\
\
\
\
\
\
\
../vegastrike/src/cmd/script/c_alike/c_alike.tab.o		\
\
../vegastrike/src/cmd/script/c_alike/lex.yy.o		\
\
../vegastrike/src/cmd/script/director.o		\
../vegastrike/src/cmd/script/director_generic.o		\
../vegastrike/src/cmd/script/flightgroup.o		\
\
../vegastrike/src/cmd/script/mission.o		\
\
../vegastrike/src/cmd/script/mission_script.o		\
\
../vegastrike/src/cmd/script/msgcenter.o		\
\
../vegastrike/src/cmd/script/pythonmission.o		\
\
../vegastrike/src/cmd/script/script_call_briefing.o		\
../vegastrike/src/cmd/script/script_call_olist.o		\
../vegastrike/src/cmd/script/script_call_omap.o		\
../vegastrike/src/cmd/script/script_call_order.o		\
../vegastrike/src/cmd/script/script_call_string.o		\
../vegastrike/src/cmd/script/script_call_unit.o		\
../vegastrike/src/cmd/script/script_call_unit_generic.o		\
../vegastrike/src/cmd/script/script_callbacks.o		\
../vegastrike/src/cmd/script/script_expression.o		\
../vegastrike/src/cmd/script/script_generic.o		\
../vegastrike/src/cmd/script/script_statement.o		\
../vegastrike/src/cmd/script/script_util.o		\
../vegastrike/src/cmd/script/script_variables.o		\
\
\
\
\
\
\
\
\
\
\
../vegastrike/src/cmd/collide/box.o		\
\
\
\
\
../vegastrike/src/cmd/collide/matrix3.o		\
../vegastrike/src/cmd/collide/pbuild.o		\
../vegastrike/src/cmd/collide/peigen.o		\
../vegastrike/src/cmd/collide/prapid.o		\
\
\
\
\
../vegastrike/src/cmd/collide/vector3.o		\
\
../vegastrike/src/cmd/alphacurve.o		\
\
../vegastrike/src/cmd/asteroid.o		\
\
../vegastrike/src/cmd/asteroid_generic.o		\
\
../vegastrike/src/cmd/atmosphere.o		\
\
\
../vegastrike/src/cmd/base_init.o		\
../vegastrike/src/cmd/base_interface.o		\
../vegastrike/src/cmd/base_util.o		\
\
../vegastrike/src/cmd/base_write_python.o		\
../vegastrike/src/cmd/base_xml.o		\
../vegastrike/src/cmd/basecomputer.o		\
\
../vegastrike/src/cmd/beam.o		\
\
../vegastrike/src/cmd/beam_generic.o		\
../vegastrike/src/cmd/bolt.o		\
\
../vegastrike/src/cmd/bolt_generic.o		\
../vegastrike/src/cmd/briefing.o		\
\
../vegastrike/src/cmd/building.o		\
\
../vegastrike/src/cmd/building_generic.o		\
\
../vegastrike/src/cmd/click_list.o		\
\
../vegastrike/src/cmd/collection.o		\
\
../vegastrike/src/cmd/collide.o		\
\
../vegastrike/src/cmd/cont_terrain.o		\
\
../vegastrike/src/cmd/container.o		\
\
\
\
../vegastrike/src/cmd/faction_xml.o		\
\
\
\
../vegastrike/src/cmd/missile_generic.o		\
\
../vegastrike/src/cmd/mount.o		\
../vegastrike/src/cmd/music.o		\
\
../vegastrike/src/cmd/nebula.o		\
\
../vegastrike/src/cmd/nebula_generic.o		\
\
../vegastrike/src/cmd/planet.o		\
\
../vegastrike/src/cmd/planet_generic.o		\
\
../vegastrike/src/cmd/role_bitmask.o		\
\
../vegastrike/src/cmd/terrain.o		\
\
\
../vegastrike/src/cmd/unit_bsp.o		\
\
../vegastrike/src/cmd/unit_collide.o		\
\
\
../vegastrike/src/cmd/unit_const_cache.o		\
\
../vegastrike/src/cmd/unit_factory.o		\
\
../vegastrike/src/cmd/unit_factory_generic.o		\
\
../vegastrike/src/cmd/unit_functions.o		\
../vegastrike/src/cmd/unit_functions_generic.o		\
../vegastrike/src/cmd/unit_generic.o		\
\
../vegastrike/src/cmd/unit_interface.o		\
\
../vegastrike/src/cmd/unit_util.o		\
\
../vegastrike/src/cmd/unit_util_generic.o		\
../vegastrike/src/cmd/unit_xml.o		\
\
../vegastrike/src/cmd/weapon_xml.o		\
\
\
\
../vegastrike/src/aldrv/al_globals.o		\
\
../vegastrike/src/aldrv/al_init.o		\
../vegastrike/src/aldrv/al_listen.o		\
../vegastrike/src/aldrv/al_sound.o		\
\
\
\
../vegastrike/src/python/briefing_wrapper.o		\
../vegastrike/src/python/init.o		\
\
\
../vegastrike/src/python/python_compile.o		\
\
\
\
../vegastrike/src/python/unit_exports.o		\
\
../vegastrike/src/python/unit_exports1.o		\
../vegastrike/src/python/unit_exports2.o		\
../vegastrike/src/python/unit_exports3.o		\
\
../vegastrike/src/python/unit_method_defs.o		\
../vegastrike/src/python/unit_wrapper.o		\
\
../vegastrike/src/python/universe_util_export.o		\
\
\
\
../vegastrike/src/gldrv/gl_clip.o		\
../vegastrike/src/gldrv/gl_fog.o		\
../vegastrike/src/gldrv/gl_globals.o		\
\
\
../vegastrike/src/gldrv/gl_init.o		\
\
../vegastrike/src/gldrv/gl_light.o		\
\
../vegastrike/src/gldrv/gl_light_pick.o		\
../vegastrike/src/gldrv/gl_light_state.o		\
../vegastrike/src/gldrv/gl_material.o		\
../vegastrike/src/gldrv/gl_matrix.o		\
\
../vegastrike/src/gldrv/gl_misc.o		\
../vegastrike/src/gldrv/gl_quad_list.o		\
../vegastrike/src/gldrv/gl_state.o		\
../vegastrike/src/gldrv/gl_texture.o		\
../vegastrike/src/gldrv/gl_vertex_list.o		\
\
../vegastrike/src/gldrv/winsys.o		\
\
\
\
\
\
\
../vegastrike/src/gfx/nav/drawgalaxy.o		\
../vegastrike/src/gfx/nav/drawlist.o		\
\
../vegastrike/src/gfx/nav/navgetxmldata.o		\
\
\
\
\
../vegastrike/src/gfx/nav/navscreen.o		\
\
../vegastrike/src/gfx/nav/navscreenoccupied.o		\
\
\
../vegastrike/src/gfx/ani_texture.o		\
\
../vegastrike/src/gfx/animation.o		\
\
../vegastrike/src/gfx/aux_logo.o		\
\
../vegastrike/src/gfx/aux_palette.o		\
\
../vegastrike/src/gfx/aux_texture.o		\
\
../vegastrike/src/gfx/background.o		\
\
../vegastrike/src/gfx/bounding_box.o		\
\
\
../vegastrike/src/gfx/bsp.o		\
\
../vegastrike/src/gfx/camera.o		\
\
\
../vegastrike/src/gfx/cockpit.o		\
\
../vegastrike/src/gfx/cockpit_generic.o		\
\
../vegastrike/src/gfx/cockpit_xml.o		\
../vegastrike/src/gfx/coord_select.o		\
\
\
../vegastrike/src/gfx/env_map_gent.o		\
\
../vegastrike/src/gfx/gauge.o		\
\
../vegastrike/src/gfx/halo.o		\
\
../vegastrike/src/gfx/halo_system.o		\
\
../vegastrike/src/gfx/hud.o		\
\
../vegastrike/src/gfx/lerp.o		\
\
../vegastrike/src/gfx/loc_select.o		\
\
../vegastrike/src/gfx/masks.o		\
\
../vegastrike/src/gfx/matrix.o		\
\
../vegastrike/src/gfx/mesh.o		\
\
../vegastrike/src/gfx/mesh_bin.o		\
../vegastrike/src/gfx/mesh_fx.o		\
../vegastrike/src/gfx/mesh_gfx.o		\
../vegastrike/src/gfx/mesh_poly.o		\
../vegastrike/src/gfx/mesh_xml.o		\
\
../vegastrike/src/gfx/particle.o		\
\
../vegastrike/src/gfx/pipelined_texture.o		\
\
\
../vegastrike/src/gfx/png_texture.o		\
\
../vegastrike/src/gfx/png_write.o		\
\
../vegastrike/src/gfx/quadsquare.o		\
\
../vegastrike/src/gfx/quadsquare_cull.o		\
../vegastrike/src/gfx/quadsquare_render.o		\
../vegastrike/src/gfx/quadsquare_update.o		\
../vegastrike/src/gfx/quadtree.o		\
\
../vegastrike/src/gfx/quadtree_xml.o		\
../vegastrike/src/gfx/quaternion.o		\
\
../vegastrike/src/gfx/ring.o		\
\
../vegastrike/src/gfx/sphere.o		\
\
../vegastrike/src/gfx/sphere_generic.o		\
../vegastrike/src/gfx/sprite.o		\
\
../vegastrike/src/gfx/star.o		\
\
../vegastrike/src/gfx/vdu.o		\
\
../vegastrike/src/gfx/vec.o		\
\
\
../vegastrike/src/gfx/vsbox.o		\
\
../vegastrike/src/gfx/warptrail.o		\
\
\
\
\
../vegastrike/src/gui/guitexture.o \
../vegastrike/src/gui/button.o		\
\
../vegastrike/src/gui/control.o		\
\
../vegastrike/src/gui/eventmanager.o		\
\
../vegastrike/src/gui/eventresponder.o		\
\
../vegastrike/src/gui/font.o		\
\
../vegastrike/src/gui/general.o		\
\
../vegastrike/src/gui/glut_support.o		\
\
../vegastrike/src/gui/groupcontrol.o		\
\
../vegastrike/src/gui/guidefs.o		\
\
../vegastrike/src/gui/modaldialog.o		\
\
../vegastrike/src/gui/newbutton.o		\
\
../vegastrike/src/gui/painttext.o		\
\
../vegastrike/src/gui/picker.o		\
\
../vegastrike/src/gui/scroller.o		\
\
../vegastrike/src/gui/simplepicker.o		\
\
../vegastrike/src/gui/slider.o		\
\
../vegastrike/src/gui/staticdisplay.o		\
\
../vegastrike/src/gui/text_area.o		\
\
../vegastrike/src/gui/window.o		\
\
../vegastrike/src/gui/windowcontroller.o		\
\
\
\
\
../vegastrike/src/networking/accountsxml.o		\
../vegastrike/src/networking/vsnet_dloadenum.o		\
../vegastrike/src/networking/vsnet_notify.o		\
\
\
\
../vegastrike/src/networking/clientstructs.o		\
\
../vegastrike/src/networking/cubicsplines.o		\
\
\
../vegastrike/src/networking/inet.o		\
\
../vegastrike/src/networking/inet_file.o		\
\
../vegastrike/src/networking/mangle.o		\
\
../vegastrike/src/networking/netbuffer.o		\
\
\
../vegastrike/src/networking/netclient.o		\
\
../vegastrike/src/networking/netserver.o		\
\
../vegastrike/src/networking/netui.o		\
\
../vegastrike/src/networking/packet.o		\
\
../vegastrike/src/networking/packetmem.o		\
\
../vegastrike/src/networking/savenet_util.o		\
\
../vegastrike/src/networking/vsnet_address.o		\
\
\
../vegastrike/src/networking/vsnet_cmd.o		\
\
../vegastrike/src/networking/vsnet_dloadmgr.o		\
\
\
../vegastrike/src/networking/vsnet_pipe.o		\
\
\
../vegastrike/src/networking/vsnet_serversocket.o		\
\
../vegastrike/src/networking/vsnet_socket.o		\
\
../vegastrike/src/networking/vsnet_socketset.o		\
\
../vegastrike/src/networking/vsnet_sockettcp.o		\
\
../vegastrike/src/networking/vsnet_socketudp.o		\
\
\
../vegastrike/src/networking/vsnet_thread_non.o		\
../vegastrike/src/networking/vsnet_thread_p.o		\
../vegastrike/src/networking/vsnet_thread_sdl.o		\
../vegastrike/src/networking/zonemgr.o		\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
\
../vegastrike/src/boost/classes.o		\
../vegastrike/src/boost/conversions.o		\
../vegastrike/src/boost/cross_module.o		\
../vegastrike/src/boost/errors.o		\
../vegastrike/src/boost/extension_class.o		\
../vegastrike/src/boost/functions.o		\
../vegastrike/src/boost/init_function.o		\
../vegastrike/src/boost/module_builder.o		\
../vegastrike/src/boost/objects.o		\
../vegastrike/src/boost/types.o		\
\
\
\
../vegastrike/src/common/common.o		\
\
 src/jthread/libjthread.a -lpthread  -lutil -L/usr/lib -lSDL -lpthread -lSDL_mixer  -lGL  -lGLU  -lglut  -lXi -lXmu  -lexpat  -lpng  -ljpeg  -lopenal -lpython2.2 -Xlinker -export-dynamic


