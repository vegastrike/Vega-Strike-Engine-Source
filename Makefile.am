ACLOCAL_AMFLAGS = -I m4scripts



#BOOST_BEGIN -- Data between the BOOST_AUTO and BOOST_END tags auto generated.
if !BOOSTSYSTEM
BOOSTDIR=boost
if BOOST1_33
BOOSTSOURCES = \
  boost/1_33/src/list.cpp \
  boost/1_33/src/module.cpp \
  boost/1_33/src/aix_init_module.cpp \
  boost/1_33/src/object_protocol.cpp \
  boost/1_33/src/dict.cpp \
  boost/1_33/src/numeric.cpp \
  boost/1_33/src/errors.cpp \
  boost/1_33/src/str.cpp \
  boost/1_33/src/object_operators.cpp \
  boost/1_33/src/long.cpp \
  boost/1_33/src/object/pickle_support.cpp \
  boost/1_33/src/object/life_support.cpp \
  boost/1_33/src/object/inheritance.cpp \
  boost/1_33/src/object/class.cpp \
  boost/1_33/src/object/enum.cpp \
  boost/1_33/src/object/function.cpp \
  boost/1_33/src/object/iterator.cpp \
  boost/1_33/src/converter/from_python.cpp \
  boost/1_33/src/converter/registry.cpp \
  boost/1_33/src/converter/arg_to_python_base.cpp \
  boost/1_33/src/converter/builtin_converters.cpp \
  boost/1_33/src/converter/type_id.cpp \
  boost/1_33/src/tuple.cpp 

else !BOOST1_33
if BOOST1_31
BOOSTSOURCES = \
  boost/1_31/src/list.cpp \
  boost/1_31/src/module.cpp \
  boost/1_31/src/aix_init_module.cpp \
  boost/1_31/src/object_protocol.cpp \
  boost/1_31/src/dict.cpp \
  boost/1_31/src/numeric.cpp \
  boost/1_31/src/errors.cpp \
  boost/1_31/src/str.cpp \
  boost/1_31/src/object_operators.cpp \
  boost/1_31/src/long.cpp \
  boost/1_31/src/object/pickle_support.cpp \
  boost/1_31/src/object/life_support.cpp \
  boost/1_31/src/object/inheritance.cpp \
  boost/1_31/src/object/class.cpp \
  boost/1_31/src/object/enum.cpp \
  boost/1_31/src/object/function.cpp \
  boost/1_31/src/object/iterator.cpp \
  boost/1_31/src/converter/from_python.cpp \
  boost/1_31/src/converter/registry.cpp \
  boost/1_31/src/converter/arg_to_python_base.cpp \
  boost/1_31/src/converter/builtin_converters.cpp \
  boost/1_31/src/converter/type_id.cpp \
  boost/1_31/src/tuple.cpp 

else !BOOST1_31
if BOOST1_28
BOOSTSOURCES = \
  boost/1_28/src/module.cpp \
  boost/1_28/src/cross_module.cpp \
  boost/1_28/src/types.cpp \
  boost/1_28/src/extension_class.cpp \
  boost/1_28/src/errors.cpp \
  boost/1_28/src/functions.cpp \
  boost/1_28/src/init_function.cpp \
  boost/1_28/src/module_builder.cpp \
  boost/1_28/src/objects.cpp \
  boost/1_28/src/object/life_support.cpp \
  boost/1_28/src/object/function.cpp \
  boost/1_28/src/converter/callback.cpp \
  boost/1_28/src/classes.cpp \
  boost/1_28/src/conversions.cpp 
else !BOOST1_28
BOOSTSOURCES=
endif !BOOST1_28
endif !BOOST1_31
endif !BOOST1_33
#BOOST_END -- Data between the BOOST_AUTO and BOOST_END tags auto generated.

#This was in the acct server and networking library stuff
#if MACOSX
#incl2 = \
#	 -I/System/Library/Frameworks/QuickTime.framework/Headers \
#	 -I/System/Library/Frameworks/Carbon.framework/Versions/A/Headers
#endif

endif 

EXTRA_DIST = \
  src/cmd/unit.cpp \
  src/cmd/unit_click.cpp \
  src/cmd/unit_customize.cpp \
  src/cmd/unit_damage.cpp \
  src/cmd/unit_jump.cpp \
  src/cmd/unit_physics.cpp \
  vc98/vegastrike.dsp \
  vc98/vegastrike.dsw \
  README \
  README.MACOSX.rtf \
  README.sgi \
  NEWS \
  ToDo.txt \
  Resources.rc \
  resource.h \
  ts.ico \
  ChangeLog \
  AUTHORS \
  vegastrike.spec \
  bootstrap-sh \
  launcher/launcher.rc \
  launcher/resource.h \
  launcher/icon1.ico \
  launcher/icon2.ico \
  src/cmd/script/c_alike/c_alike.lex \
  src/cmd/script/c_alike/c_alike.yacc

#include src/Makefile

LIBNET = \
  src/networking/inet_file.cpp \
  src/networking/inet_file.h \
  src/networking/inet.cpp \
  src/networking/inet.h

libacctserver = \
  src/networking/accountsxml.cpp \
  src/networking/accountsxml.h \
  src/networking/acctserver.h \
  src/networking/client.cpp \
  src/networking/client.h \
  src/networking/const.h \
  src/networking/fileutil.cpp \
  src/networking/fileutil.h \
  src/networking/savenet_util.cpp \
  src/networking/savenet_util.h

libnetgeneric = \
  $(libacctserver) \
  src/networking/cubicsplines.cpp \
  src/networking/cubicsplines.h \
  src/networking/mangle.cpp \
  src/networking/mangle.h \
  src/networking/netclient_clients.cpp \
  src/networking/netclient_devices.cpp \
  src/networking/netclient_login.cpp \
  src/networking/netclient.cpp \
  src/networking/netclient.h \
  src/networking/netserver_acct.cpp \
  src/networking/netserver_clients.cpp \
  src/networking/netserver_devices.cpp \
  src/networking/netserver_login.cpp \
  src/networking/netserver_net.cpp \
  src/networking/netserver.cpp \
  src/networking/netserver.h \
  src/networking/prediction.cpp \
  src/networking/prediction.h \
  src/networking/zonemgr.cpp \
  src/networking/zonemgr.h

libnetclientserver = \
  $(libnetgeneric)

LIBNETCLIENT = \
  $(libnetclientserver) \
  src/networking/networkcomm.cpp \
  src/networking/networkcomm.h \
  src/networking/webcam_support.h \
  src/networking/webcam_support.cpp

LIBNETSERVER = \
  $(libnetclientserver) \
  src/networking/networkcomm_server.cpp

LIBGUI = \
  src/gui/button.cpp \
  src/gui/button.h \
  src/gui/control.cpp \
  src/gui/control.h \
  src/gui/eventmanager.cpp \
  src/gui/eventmanager.h \
  src/gui/eventresponder.cpp \
  src/gui/eventresponder.h \
  src/gui/font.cpp \
  src/gui/font.h \
  src/gui/general.cpp \
  src/gui/general.h \
  src/gui/glut_support.cpp \
  src/gui/glut_support.h \
  src/gui/groupcontrol.cpp \
  src/gui/groupcontrol.h \
  src/gui/guidefs.cpp \
  src/gui/guidefs.h \
  src/gui/guitexture.cpp \
  src/gui/guitexture.h  \
  src/gui/modaldialog.cpp \
  src/gui/modaldialog.h \
  src/gui/newbutton.cpp \
  src/gui/newbutton.h \
  src/gui/painttext.cpp \
  src/gui/painttext.h \
  src/gui/picker.cpp \
  src/gui/picker.h \
  src/gui/scroller.cpp \
  src/gui/scroller.h \
  src/gui/simplepicker.cpp \
  src/gui/simplepicker.h \
  src/gui/slider.cpp \
  src/gui/slider.h \
  src/gui/staticdisplay.cpp \
  src/gui/staticdisplay.h \
  src/gui/text_area.cpp \
  src/gui/text_area.h \
  src/gui/textinputdisplay.cpp \
  src/gui/textinputdisplay.h \
  src/gui/window.cpp \
  src/gui/window.h \
  src/gui/windowcontroller.cpp \
  src/gui/windowcontroller.h

LIBPYTHON = \
  src/python/define_odd_unit_functions.h \
  src/python/init.cpp \
  src/python/init.h \
  src/python/python_class.h \
  src/python/python_compile.cpp \
  src/python/python_compile.h \
  src/python/python_unit_wrap.h \
  src/python/star_system_exports.h \
  src/python/unit_exports.cpp \
  src/python/unit_exports.h \
  src/python/unit_exports1.cpp \
  src/python/unit_exports2.cpp \
  src/python/unit_exports3.cpp \
  src/python/unit_from_to_python.h \
  src/python/unit_method_defs.cpp \
  src/python/unit_wrapper_class.h \
  src/python/unit_wrapper.cpp \
  src/python/universe_util_export.cpp

LIBSCRIPT = \
  src/cmd/script/director_generic.cpp \
  src/cmd/script/mission_script.cpp \
  src/cmd/script/mission_types.h \
  src/cmd/script/mission.cpp \
  src/cmd/script/mission.h \
  src/cmd/script/msgcenter.cpp \
  src/cmd/script/msgcenter.h \
  src/cmd/script/pythonmission.cpp \
  src/cmd/script/pythonmission.h \
  src/cmd/script/script_call_olist.cpp \
  src/cmd/script/script_call_omap.cpp \
  src/cmd/script/script_call_order.cpp \
  src/cmd/script/script_call_string.cpp \
  src/cmd/script/script_call_unit_generic.cpp \
  src/cmd/script/script_callbacks.cpp \
  src/cmd/script/script_expression.cpp \
  src/cmd/script/script_generic.cpp \
  src/cmd/script/script_statement.cpp \
  src/cmd/script/script_util.cpp \
  src/cmd/script/script_variables.cpp

LIBAI = \
  src/cmd/ai/aggressive.cpp \
  src/cmd/ai/aggressive.h \
  src/cmd/ai/comm_ai.cpp \
  src/cmd/ai/comm_ai.h \
  src/cmd/ai/communication_xml.cpp \
  src/cmd/ai/communication.cpp \
  src/cmd/ai/communication.h \
  src/cmd/ai/docking.cpp \
  src/cmd/ai/docking.h \
  src/cmd/ai/event_xml.cpp \
  src/cmd/ai/event_xml.h \
  src/cmd/ai/fire.cpp \
  src/cmd/ai/fire.h \
  src/cmd/ai/fireall.cpp \
  src/cmd/ai/fireall.h \
  src/cmd/ai/firekeyboard.h \
  src/cmd/ai/flybywire.cpp \
  src/cmd/ai/flybywire.h \
  src/cmd/ai/flykeyboard.h \
  src/cmd/ai/hard_coded_scripts.cpp \
  src/cmd/ai/hard_coded_scripts.h \
  src/cmd/ai/ikarus.cpp \
  src/cmd/ai/ikarus.h \
  src/cmd/ai/missionscript.cpp \
  src/cmd/ai/missionscript.h \
  src/cmd/ai/navigation.cpp \
  src/cmd/ai/navigation.h \
  src/cmd/ai/order_comm.cpp \
  src/cmd/ai/order.cpp \
  src/cmd/ai/order.h \
  src/cmd/ai/script.cpp \
  src/cmd/ai/script.h \
  src/cmd/ai/script.h \
  src/cmd/ai/tactics.cpp \
  src/cmd/ai/tactics.h \
  src/cmd/ai/turretai.cpp \
  src/cmd/ai/turretai.h \
  src/cmd/ai/warpto.cpp \
  src/cmd/ai/warpto.h

LIBCOLLIDE = \
  src/cmd/collide/box.cpp \
  src/cmd/collide/collider.h \
  src/cmd/collide/cs_compat.h \
  src/cmd/collide/garray.h \
  src/cmd/collide/matrix3.cpp \
  src/cmd/collide/pbuild.cpp \
  src/cmd/collide/peigen.cpp \
  src/cmd/collide/prapid.cpp \
  src/cmd/collide/prapid.h \
  src/cmd/collide/qint.h \
  src/cmd/collide/qsqrt.h \
  src/cmd/collide/rapcol.h \
  src/cmd/collide/vector3.cpp \
  src/cmd/collide/csgeom/matrix3.h \
  src/cmd/collide/csgeom/vector3.h \
  src/cmd/collide/csgeom/box.h \
  src/cmd/collide/csgeom/transfrm.h


LIBCMD = \
  src/cmd/alphacurve.cpp \
  src/cmd/alphacurve.h \
  src/cmd/asteroid_generic.cpp \
  src/cmd/asteroid_generic.h \
  src/cmd/beam_generic.cpp \
  src/cmd/beam.h \
  src/cmd/click_list.h \
  src/cmd/bolt_generic.cpp \
  src/cmd/bolt.h \
  src/cmd/building_generic.cpp \
  src/cmd/building_generic.h \
  src/cmd/collection.cpp \
  src/cmd/collection.h \
  src/cmd/collide_map.cpp \
  src/cmd/collide_map.h \
  src/cmd/collide.cpp \
  src/cmd/collide.h \
  src/cmd/container.cpp \
  src/cmd/container.h \
  src/cmd/csv.cpp \
  src/cmd/csv.h \
  src/cmd/enhancement_generic.h \
  src/cmd/faction_xml.cpp \
  src/cmd/images.h \
  src/cmd/iterator.h \
  src/cmd/key_mutable_set.h \
  src/cmd/missile_generic.cpp \
  src/cmd/missile_generic.h \
  src/cmd/mount.cpp \
  src/cmd/nebula_generic.cpp \
  src/cmd/nebula_generic.h \
  src/cmd/planet_generic.cpp \
  src/cmd/planet_generic.h \
  src/cmd/role_bitmask.cpp \
  src/cmd/role_bitmask.h \
  src/cmd/unit_bsp.cpp \
  src/cmd/unit_bsp.h \
  src/cmd/unit_collide.cpp \
  src/cmd/unit_collide.h \
  src/cmd/unit_const_cache.cpp \
  src/cmd/unit_const_cache.h \
  src/cmd/unit_csv.cpp \
  src/cmd/unit_factory_generic.cpp \
  src/cmd/unit_factory.h \
  src/cmd/unit_functions_generic.cpp \
  src/cmd/unit_generic.cpp \
  src/cmd/unit_generic.h \
  src/cmd/pilot.cpp \
  src/cmd/pilot.h \
  src/cmd/unit_util_generic.cpp \
  src/cmd/unit_util.h \
  src/cmd/unit_xml.cpp \
  src/cmd/unit_xml.h \
  src/cmd/weapon_xml.cpp \
  src/cmd/weapon_xml.h

if VS_MAKE_GTKUTILS
  GTKUTILS_PROG=vssetup vslauncher
else
  GTKUTILS_PROG=
endif

if VS_MAKE_NO_CLIENT
  VS_MAKE_SOUNDSERVER=
  CLIENT_PROG=
else
  CLIENT_PROG= \
    vegastrike \
    base_maker
endif

if VS_MAKE_SOUNDSERVER
  MUSIC_PROG= soundserver
else
  MUSIC_PROG=
endif

bin_PROGRAMS = \
  $(CLIENT_PROG) \
  $(MUSIC_PROG) \
  accountserver \
  asteroidgen \
  mesher \
  mesh_xml \
  tempgen \
  replace \
  trisort \
  vegaserver \
  vsrextract \
  vsrmake \
  $(GTKUTILS_PROG)


if !BOOSTSYSTEM
noinst_LIBRARIES=\
  libboost_python.a \
  libnetlowlevel.a
else
noinst_LIBRARIES= \
  libnetlowlevel.a
endif 

LIBCMDSERVER = \
  src/cmd/atmosphere_server.cpp \
  src/cmd/beam_server.cpp \
  src/cmd/bolt_server.cpp \
  src/cmd/unit_factory_server.cpp \
  src/cmd/unit_functions_server.cpp \
  src/cmd/unit_util_server.cpp

LIBSCRIPTSERVER = \
  src/cmd/script/flightgroup_server.cpp \
  src/cmd/script/flightgroup.h \
  src/cmd/script/script_call_unit_server.cpp \
  src/cmd/script/c_alike/c_alike.tab.cpp \
  src/cmd/script/c_alike/lex.yy.cpp

if !BOOSTSYSTEM
libboost_python_a_SOURCES = \
  $(BOOSTSOURCES)
endif


LIBGFX = \
  src/gfx/aux_texture.h \
  src/gfx/bounding_box.cpp \
  src/gfx/bounding_box.h \
  src/gfx/bsp.cpp \
  src/gfx/bsp.h \
  src/gfx/cockpit_generic.cpp \
  src/gfx/cockpit_generic.h \
  src/gfx/halo_system.h \
  src/gfx/lerp.cpp \
  src/gfx/lerp.h \
  src/gfx/matrix.cpp \
  src/gfx/matrix.h \
  src/gfx/mesh_bxm.cpp \
  src/gfx/mesh_bxm.h \
  src/gfx/mesh_bin.cpp \
  src/gfx/mesh_io.h \
  src/gfx/mesh_poly.cpp \
  src/gfx/mesh_xml.cpp \
  src/gfx/mesh_xml.h \
  src/gfx/mesh.cpp \
  src/gfx/mesh.h \
  src/gfx/quaternion.cpp \
  src/gfx/quaternion.h \
  src/gfx/sphere_generic.cpp \
  src/gfx/vec.cpp \
  src/gfx/vec.h \
  src/gfx/vertex.h


LIBGFXCLIENT = \
  src/gfx/ani_texture.cpp \
  src/gfx/ani_texture.h \
  src/gfx/animation.cpp \
  src/gfx/animation.h \
  src/gfx/aux_logo.cpp \
  src/gfx/aux_logo.h \
  src/gfx/aux_palette.cpp \
  src/gfx/aux_palette.h \
  src/gfx/aux_texture.cpp \
  src/gfx/aux_texture.h \
  src/gfx/background.cpp \
  src/gfx/background.h \
  src/gfx/camera.cpp \
  src/gfx/camera.h \
  src/gfx/car_assist.h \
  src/gfx/cockpit_xml.cpp \
  src/gfx/cockpit.cpp \
  src/gfx/cockpit.h \
  src/gfx/coord_select.cpp \
  src/gfx/coord_select.h \
  src/gfx/decalqueue.h \
  src/gfx/env_map_gent.cpp \
  src/gfx/env_map_gent.h \
  src/gfx/gauge.cpp \
  src/gfx/gauge.h \
  src/gfx/halo_system.cpp \
  src/gfx/halo.cpp \
  src/gfx/halo.h \
  src/gfx/hud.cpp \
  src/gfx/hud.h \
  src/gfx/jpeg_memory.cpp \
  src/gfx/jpeg_memory.h \
  src/gfx/loc_select.cpp \
  src/gfx/loc_select.h \
  src/gfx/masks.cpp \
  src/gfx/masks.h \
  src/gfx/mesh_fx.cpp \
  src/gfx/mesh_fx.h \
  src/gfx/mesh_gfx.cpp \
  src/gfx/nav/criteria_xml.cpp \
  src/gfx/nav/criteria.cpp \
  src/gfx/nav/criteria.h \
  src/gfx/nav/drawgalaxy.cpp \
  src/gfx/nav/drawlist.cpp \
  src/gfx/nav/drawlist.h \
  src/gfx/nav/drawsystem.cpp \
  src/gfx/nav/navcomputer.cpp \
  src/gfx/nav/navcomputer.h \
  src/gfx/nav/navgetxmldata.cpp \
  src/gfx/nav/navgetxmldata.h \
  src/gfx/nav/navitemstodraw.h \
  src/gfx/nav/navitemtypes.h \
  src/gfx/nav/navparse.h \
  src/gfx/nav/navpath.cpp \
  src/gfx/nav/navpath.h \
  src/gfx/nav/navscreen.cpp \
  src/gfx/nav/navscreen.h \
  src/gfx/nav/navscreenoccupied.cpp \
  src/gfx/nav/navscreenoccupied.h \
  src/gfx/nonlinear_transform.h \
  src/gfx/particle.cpp \
  src/gfx/particle.h \
  src/gfx/pipelined_texture.cpp \
  src/gfx/pipelined_texture.h \
  src/gfx/planetary_transform.h \
  src/gfx/point_to_cam.h \
  src/gfx/quadsquare_cull.cpp \
  src/gfx/quadsquare_render.cpp \
  src/gfx/quadsquare_update.cpp \
  src/gfx/quadsquare.cpp \
  src/gfx/quadsquare.h \
  src/gfx/quadtree_xml.cpp \
  src/gfx/quadtree.cpp \
  src/gfx/quadtree.h \
  src/gfx/ring.cpp \
  src/gfx/ring.h \
  src/gfx/screenshot.cpp \
  src/gfx/screenshot.h \
  src/gfx/sphere.cpp \
  src/gfx/sphere.h \
  src/gfx/sprite.cpp \
  src/gfx/sprite.h \
  src/gfx/star.cpp \
  src/gfx/star.h \
  src/gfx/stream_texture.cpp \
  src/gfx/stream_texture.h \
  src/gfx/tex_transform.cpp \
  src/gfx/vdu.cpp \
  src/gfx/vdu.h \
  src/gfx/vsbox.cpp \
  src/gfx/vsbox.h \
  src/gfx/vsimage.cpp \
  src/gfx/vsimage.h \
  src/gfx/warptrail.cpp \
  src/gfx/warptrail.h \
  src/gfx/xvector.h


LIBGFXSERVER = \
  src/gfx/sphere_server.cpp \
  src/gfx/vsbox_server.cpp \
  src/gfx/mesh_server.cpp \
  src/gfx/background_server.cpp



LIBNETACCT = \
  src/networking/clientptr.h \
  src/networking/const.h \
  src/networking/accountsxml.cpp \
  src/networking/accountsxml.h \
  src/networking/acctserver.h \
  src/networking/fileutil.h \
  src/networking/fileutil.cpp \
  src/networking/acctserver.cpp \
  src/networking/acctserver.h

libnetlowlevel_a_SOURCES = \
  src/networking/lowlevel/GCPtr.h \
  src/networking/lowlevel/netbuffer.cpp \
  src/networking/lowlevel/netbuffer.h \
  src/networking/lowlevel/netui.cpp \
  src/networking/lowlevel/netui.h \
  src/networking/lowlevel/packet.cpp \
  src/networking/lowlevel/packet.h \
  src/networking/lowlevel/packetmem.cpp \
  src/networking/lowlevel/packetmem.h \
  src/networking/lowlevel/vsnet_address.cpp \
  src/networking/lowlevel/vsnet_address.h \
  src/networking/lowlevel/vsnet_clientstate.cpp \
  src/networking/lowlevel/vsnet_clientstate.h \
  src/networking/lowlevel/vsnet_cmd.cpp \
  src/networking/lowlevel/vsnet_cmd.h \
  src/networking/lowlevel/vsnet_debug.cpp \
  src/networking/lowlevel/vsnet_debug.h \
  src/networking/lowlevel/vsnet_dloadenum.cpp \
  src/networking/lowlevel/vsnet_dloadenum.h \
  src/networking/lowlevel/vsnet_dloadmgr.cpp \
  src/networking/lowlevel/vsnet_dloadmgr.h \
  src/networking/lowlevel/vsnet_headers.h \
  src/networking/lowlevel/vsnet_err.h \
  src/networking/lowlevel/vsnet_notify.cpp \
  src/networking/lowlevel/vsnet_notify.h \
  src/networking/lowlevel/vsnet_oss.cpp \
  src/networking/lowlevel/vsnet_oss.h \
  src/networking/lowlevel/vsnet_parsecmd.cpp \
  src/networking/lowlevel/vsnet_parsecmd.h \
  src/networking/lowlevel/vsnet_pipe.cpp \
  src/networking/lowlevel/vsnet_pipe.h \
  src/networking/lowlevel/vsnet_queue.h \
  src/networking/lowlevel/vsnet_serversocket.cpp \
  src/networking/lowlevel/vsnet_serversocket.h \
  src/networking/lowlevel/vsnet_socket.cpp \
  src/networking/lowlevel/vsnet_socket.h \
  src/networking/lowlevel/vsnet_socketbase.cpp \
  src/networking/lowlevel/vsnet_socketbase.h \
  src/networking/lowlevel/vsnet_socketflags.cpp \
  src/networking/lowlevel/vsnet_socketflags.h \
  src/networking/lowlevel/vsnet_sockethttp.cpp \
  src/networking/lowlevel/vsnet_socketset.cpp \
  src/networking/lowlevel/vsnet_socketset.h \
  src/networking/lowlevel/vsnet_sockettcp.cpp \
  src/networking/lowlevel/vsnet_sockettcp.h \
  src/networking/lowlevel/vsnet_socketudp.cpp \
  src/networking/lowlevel/vsnet_socketudp.h \
  src/networking/lowlevel/vsnet_thread_non.cpp \
  src/networking/lowlevel/vsnet_thread_p.cpp \
  src/networking/lowlevel/vsnet_thread_sdl.cpp \
  src/networking/lowlevel/vsnet_thread.h


soundserver_SOURCES = \
  $(LIBNET) \
  src/networking/softvolume.cpp \
  src/networking/softvolume.h \
  src/networking/soundserver.cpp

soundserver_LDADD = \
  $(SDL_LIBS) \
  $(SDL_MIXER_LIBS)

if !BOOSTSYSTEM
  vegastrike_LDADD = \
  libboost_python.a \
  libnetlowlevel.a \
  -lz  
else
  vegastrike_LDADD = \
  libnetlowlevel.a \
  -lboost_python-st \
  -lz
endif  

if !BOOSTSYSTEM 
  vegastrike_DEPENDENCIES = \
  libboost_python.a \
  libnetlowlevel.a
else
  vegastrike_DEPENDENCIES = \
    libnetlowlevel.a
endif

if !BOOSTSYSTEM
vegaserver_LDADD = \
  libboost_python.a \
  libnetlowlevel.a \
  -lz
else
vegaserver_LDADD = \
  libnetlowlevel.a \
  -lboost_python-st \
  -lz
endif

if !BOOSTSYSTEM
vegaserver_DEPENDENCIES = \
  libboost_python.a \
  libnetlowlevel.a
else
vegaserver_DEPENDENCIES = \
  libnetlowlevel.a
endif


accountserver_LDADD = \
  libnetlowlevel.a \
  -lz

accountserver_DEPENDENCIES = \
  libnetlowlevel.a



librootclient = \
  src/audiolib.h \
  src/cg_global.cpp \
  src/command.cpp \
  src/command.h \
  src/config_xml.cpp \
  src/config_xml.h \
  src/debug_vs.cpp \
  src/debug_vs.h \
  src/faction_util.cpp \
  src/force_feedback.cpp \
  src/force_feedback.h \
  src/functors.h \
  src/fmod.h \
  src/gfxlib_struct.cpp \
  src/gfxlib_struct.h \
  src/gfxlib.h \
  src/in_handler.h \
  src/in_joystick.cpp \
  src/in_joystick.h \
  src/in_kb.cpp \
  src/in_kb.h \
  src/in_kb_data.h \
  src/in_main.cpp \
  src/in_main.h \
  src/in_mouse.cpp \
  src/in_mouse.h \
  src/in_sdl.cpp \
  src/linecollide.h \
  src/main_loop.cpp \
  src/main_loop.h \
  src/physics.cpp \
  src/physics.h \
  src/profile.h \
  src/rendertext.cpp \
  src/rendertext.h \
  src/resizable.h \
  src/ship_commands.cpp \
  src/ship_commands.h \
  src/star_system_jump.cpp \
  src/star_system.cpp \
  src/star_system.h \
  src/universe_util.cpp \
  src/universe_util.h \
  src/universe.cpp \
  src/universe.h

# Disabled from librootclient since MMORPG depends on SDL which is not
# easily available on Macs. -Patrick
#  src/mmorpgclient.cpp
#  src/mmorpgclient.h

librootgeneric = \
  src/cg_global.h \
  src/configxml.cpp \
  src/configxml.h \
  src/easydom.cpp \
  src/easydom.h \
  src/endianness.cpp \
  src/endianness.h \
  src/macosx_math.cpp \
  src/macosx_math.h \
  src/faction_generic.cpp \
  src/faction_generic.h \
  src/faction_util_generic.cpp \
  src/file_main.h \
  src/galaxy_gen.cpp \
  src/galaxy_gen.h \
  src/galaxy_xml.cpp \
  src/galaxy_xml.h \
  src/galaxy.cpp \
  src/hashtable.cpp \
  src/hashtable.h \
  src/in.h \
  src/lin_time.cpp \
  src/lin_time.h \
  src/load_mission.cpp \
  src/load_mission.h \
  src/pk3.cpp \
  src/pk3.h \
  src/posh.cpp \
  src/posh.h \
  src/savegame.cpp \
  src/savegame.h \
  src/SharedPool.h \
  src/star_system_generic.cpp \
  src/star_system_generic.h \
  src/star_system_xml.cpp \
  src/save_util.h \
  src/stardate.cpp \
  src/stardate.h \
  src/universe_generic.cpp \
  src/universe_generic.h \
  src/universe_util_generic.cpp \
  src/universe_util.h \
  src/vegastrike.h \
  src/vs_globals.cpp \
  src/vs_globals.h \
  src/vs_random.h \
  src/vsfilesystem.cpp \
  src/vsfilesystem.h \
  src/xml_serializer.cpp \
  src/xml_serializer.h \
  src/xml_support.cpp \
  src/xml_support.h

librootserver = \
  src/faction_util_server.cpp \
  src/force_feedback_server.cpp \
  src/gfxlib_struct_server.cpp \
  src/libaudioserver.cpp \
  src/libserver.cpp \
  src/universe_util_server.cpp

LIBENGINE_COMMON = \
  $(LIBAI) \
  $(LIBCMD) \
  $(LIBCOLLIDE) \
  $(LIBNET) \
  $(LIBPYTHON) \
  $(librootgeneric) \
  $(LIBSCRIPT) \
  $(LIBGFX)

vegaserver_SOURCES = \
  $(LIBENGINE_COMMON) \
  $(LIBCMDSERVER) \
  $(LIBNETSERVER) \
  $(librootserver) \
  $(LIBSCRIPTSERVER) \
  $(LIBGFXSERVER) \
  src/vegaserver.cpp \
  src/cmd/script/director_server.cpp \
  src/gldrv/gl_vertex_list.cpp \
  src/gldrv/gl_sphere_list_server.cpp

accountserver_SOURCES = \
  $(LIBNETACCT) \
  src/accountserver.cpp \
  src/configxml.cpp \
  src/configxml.h \
  src/easydom.cpp \
  src/easydom.h \
  src/endianness.cpp \
  src/endianness.h \
  src/libserver.cpp \
  src/lin_time.cpp \
  src/lin_time.h \
  src/pk3.cpp \
  src/pk3.h \
  src/posh.cpp \
  src/vsfilesystem.cpp \
  src/vsfilesystem.h \
  src/xml_support.cpp \
  src/xml_support.h \
  src/SharedPool.h

vegastrike_SOURCES = \
  $(LIBENGINE_COMMON) \
  $(LIBGUI) \
  $(LIBNETCLIENT) \
  $(librootclient) \
  $(LIBGFXCLIENT) \
  src/aldrv/al_globals.cpp \
  src/aldrv/al_globals.h \
  src/aldrv/al_init.cpp \
  src/aldrv/al_listen.cpp \
  src/aldrv/al_sound.cpp \
  src/cmd/ai/firekeyboard.cpp \
  src/cmd/ai/firekeyboard.h \
  src/cmd/ai/flyjoystick.cpp \
  src/cmd/ai/flyjoystick.h \
  src/cmd/ai/flykeyboard.cpp \
  src/cmd/ai/flykeyboard.h \
  src/cmd/ai/input_dfa.cpp \
  src/cmd/ai/input_dfa.h \
  src/cmd/asteroid.cpp \
  src/cmd/asteroid.h \
  src/cmd/atmosphere.cpp \
  src/cmd/atmosphere.h \
  src/cmd/base_init.cpp \
  src/cmd/base_interface.cpp \
  src/cmd/base_util.cpp \
  src/cmd/base_util.h \
  src/cmd/base_write_python.cpp \
  src/cmd/base_write_xml.cpp \
  src/cmd/base_xml.cpp \
  src/cmd/base.h \
  src/cmd/basecomputer.cpp \
  src/cmd/basecomputer.h \
  src/cmd/beam.cpp \
  src/cmd/beam.h \
  src/cmd/bolt.cpp \
  src/cmd/briefing.cpp \
  src/cmd/briefing.h \
  src/cmd/building.cpp \
  src/cmd/building.h \
  src/cmd/click_list.cpp \
  src/cmd/click_list.h \
  src/cmd/cont_terrain.cpp \
  src/cmd/cont_terrain.h \
  src/cmd/enhancement.h \
  src/cmd/missile.h \
  src/cmd/music.cpp \
  src/cmd/music.h \
  src/cmd/nebula.cpp \
  src/cmd/nebula.h \
  src/cmd/planet.cpp \
  src/cmd/planet.h \
  src/cmd/script/c_alike/c_alike.tab.cpp \
  src/cmd/script/c_alike/lex.yy.cpp \
  src/cmd/script/director.cpp \
  src/cmd/script/flightgroup.cpp \
  src/cmd/script/flightgroup.h \
  src/cmd/script/script_call_briefing.cpp \
  src/cmd/script/script_call_unit.cpp \
  src/cmd/terrain.cpp \
  src/cmd/terrain.h \
  src/cmd/unit_armorshield.h \
  src/cmd/unit_csv.h \
  src/cmd/unit_factory.cpp \
  src/cmd/unit_find.h \
  src/cmd/unit_functions.cpp \
  src/cmd/unit_interface.cpp \
  src/cmd/unit_interface.h \
  src/cmd/unit_template.h \
  src/cmd/unit_util.cpp \
  src/cmd/unit_util.h \
  src/cmd/unit.h \
  src/gldrv/gl_clip.cpp \
  src/gldrv/gl_fog.cpp \
  src/gldrv/gl_globals.cpp \
  src/gldrv/gl_globals.h \
  src/gldrv/gl_include.h \
  src/gldrv/gl_init.cpp \
  src/gldrv/gl_init.h \
  src/gldrv/gl_light_pick.cpp \
  src/gldrv/gl_light_state.cpp \
  src/gldrv/gl_light.cpp \
  src/gldrv/gl_light.h \
  src/gldrv/gl_material.cpp \
  src/gldrv/gl_matrix.cpp \
  src/gldrv/gl_matrix.h \
  src/gldrv/gl_misc.cpp \
  src/gldrv/gl_quad_list.cpp \
  src/gldrv/gl_sphere_list.cpp \
  src/gldrv/gl_state.cpp \
  src/gldrv/gl_texture.cpp \
  src/gldrv/gl_vertex_list.cpp \
  src/gldrv/gl_undefined_extensions.h \
  src/gldrv/glx_undefined_extensions.h \
  src/gldrv/gl_include.h \
  src/gldrv/hashtable_3d.h \
  src/gldrv/winsys.cpp \
  src/gldrv/winsys.h \
  src/main.cpp \
  src/python/briefing_wrapper.cpp


pkgdatadir = $(prefix)/objconv
pkgdata_DATA = objconv/3ds2xml objconv/obj2xml objconv/wcp2xml
EXTRA_DIST += $(pkgdata_DATA)



base_maker_CXXFLAGS = -I../.. -I../../src $(BOOST_CPPFLAGS) -g 
# ToDo: Fix so -L/usr/... is found in configure  
#base_maker_LDFLAGS = -lGL -lGLU -lSDL -lglut -ljpeg -lpng \
base_maker_LDFLAGS = -lGL -lGLU -lglut -ljpeg -lpng \
  -lpthread -L/usr/X11R6/lib -lXi -lXmu
base_maker_SOURCES = \
  objconv/basemaker/base_maker.cpp \
  objconv/basemaker/base_maker_texture.cpp \
  objconv/basemaker/base_maker_texture.h

mesher_CXXFLAGS = $(OGRE_CFLAGS) -I../../src -I../..
mesher_LDFLAGS = $(OGRE_LIBS)
mesher_SRC = \
  objconv/mesher/converter.cpp \
  objconv/mesher/expat.h  \
  objconv/mesher/from_BFXM.cpp \
  objconv/mesher/from_BFXM.h \
  objconv/mesher/from_obj.cpp \
  objconv/mesher/from_obj.h \
  objconv/mesher/mesh_io.h  \
  objconv/mesher/mesh.h  \
  objconv/mesher/to_BFXM.cpp \
  objconv/mesher/to_BFXM.h \
  objconv/mesher/to_obj.cpp \
  objconv/mesher/to_obj.h \
  src/endianness.h \
  src/hashtable.cpp \
  src/hashtable.h \
  src/xml_support.cpp \
  src/xml_support.h
if HAVE_OGRE
mesher_SOURCES = $(mesher_SRC) \
  objconv/mesher/to_OgreMesh.cpp \
  objconv/mesher/to_OgreMesh.h 
else
mesher_SOURCES = $(mesher_SRC)
endif
tempgen_LDFLAGS = -lexpat
tempgen_CPPFLAGS = -I../../src
tempgen_SOURCES = \
  objconv/MeshParse/template_generator.cpp \
  src/xml_support.cpp \
  src/xml_support.h \
  src/hashtable.cpp \
  src/hashtable.h

mesh_xml_LDFLAGS = -lexpat
mesh_xml_CPPFLAGS = -I../../src
mesh_xml_SOURCES = \
  objconv/MeshParse/template_generator.cpp \
  src/xml_support.cpp \
  src/xml_support.h

trisort_SOURCES = \
  objconv/trisort.cpp \
  objconv/trisort.h

replace_SOURCES = \
  objconv/replace.cpp

asteroidgen_SOURCES = \
  objconv/asteroidgen.cpp

vsrmake_SOURCES= \
  tools/common.cpp \
  tools/common.h \
  tools/vsrmake.cpp \
  tools/vsrtools.h

vsrextract_SOURCES= \
  tools/common.cpp \
  tools/common.h \
  tools/vsrextract.cpp \
  tools/vsrtools.h

GTK_LIBS=`gtk-config --libs`
GTK_CFLAGS=`gtk-config --cflags`

vssetup_LDFLAGS = $(GTK_LIBS)
vssetup_CXXFLAGS= $(GTK_CFLAGS) -DGTK
vssetup_SOURCES = \
    setup/src/c/setup.cpp \
    setup/src/include/central.cpp \
    setup/src/include/central.h \
    setup/src/include/display_gtk.cpp \
    setup/src/include/display.h \
    setup/src/include/file.cpp \
    setup/src/include/file.h \
    setup/src/include/general.cpp \
    setup/src/include/general.h \
    src/common/common.cpp \
    src/common/common.h

vslauncher_LDFLAGS = $(GTK_LIBS)
vslauncher_CXXFLAGS= $(GTK_CFLAGS)
vslauncher_SOURCES= \
    launcher/saveinterface.cpp \
    launcher/general.cpp \
    launcher/general.h \
    src/common/common.cpp \
    src/common/common.h

#src/cmd/script/c_alike/c_alike.tab.cpp: src/cmd/script/c_alike/c_alike.yacc

#src/cmd/script/c_alike/lex.yy.cpp: src/cmd/script/c_alike/c_alike.lex
#	flex -o src/cmd/script/c_alike/lex.yy.cpp src/cmd/script/c_alike/c_alike.lex 

#The following is a way to get around the HUGE number of files that boost contains.
if !BOOSTSYSTEM 
dist-hook:
	$(mkdir_p) $(distdir)/$(BOOSTDIR)
	for x in `find $(BOOSTDIR) -maxdepth 1 ! -wholename $(BOOSTDIR) -type d -printf '%f\n' |sort -r`; do \
	  find $(BOOSTDIR)/$$x/boost "(" -name CVS -prune ")" , "(" ! -name CVS -type d -printf "D %p\n" ")" , "(" -type f -printf "F %p\n" ")" \
	    | $(AWK) '$$1=="D"{print("'"$(mkdir_p) $(distdir)/"'"$$2)}; $$1=="F"{print("cp -p "$$2"'" $(distdir)/"'/"$$2)};'  | $(SHELL); \
done
endif

%.h.gch: %.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: headers
headers:
	find . -name '*.h' -exec make '{}.gch' ';'

.PHONY: clean-local
clean-local:
	rm -f `find . -name '*.h.gch'`
