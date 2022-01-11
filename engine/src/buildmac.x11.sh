##
# buildmac.x11.sh
#
# Copyright (C) 2001-2002 Daniel Horn
# Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
# Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
#
# https://github.com/vegastrike/Vega-Strike-Engine-Source
#
# This file is part of Vega Strike.
#
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
#

g++    -pipe -O2  -Wall  -ffast-math -fomit-frame-pointer -fexpensive-optimizations    -o vegastrike star_system_generic.o universe_generic.o universe_util_generic.o galaxy.o galaxy_xml.o galaxy_gen.o faction_generic.o hashtable.o configxml.o easydom.o xml_serializer.o xml_support.o lin_time.o endianness.o faction_util.o load_mission.o savegame.o vs_path.o debug_vs.o gfxlib_struct.o in_joystick.o faction.o force_feedback.o in_kb.o in_sdl.o in_mouse.o in_main.o in_handler.o main_loop.o physics.o star_system_jump.o star_system_xml.o star_system.o universe.o universe_util.o config_xml.o macosx_math.o vs_globals.o main.o aldrv/libaldrv.a common/libvscommon.a networking/libnetclient.a cmd/script/script_call_briefing.o cmd/script/libscript.a cmd/script/c_alike/libc_alike.a python/briefing_wrapper.o cmd/libcmd.a cmd/base_init.o python/libpython.a gfx/libgfx.a cmd/ai/libai.a gldrv/libgldrv.a gui/libgui.a networking/libnet.a cmd/collide/libcollide.a boost129/libboost_python.a boost/libboost_python.a -lpthread  -lobjc /Users/daniel/Install/glut-3.7/lib/glut/glut_8x13.o /Users/daniel/Install/glut-3.7/lib/glut/glut_9x15.o /Users/daniel/Install/glut-3.7/lib/glut/glut_bitmap.o /Users/daniel/Install/glut-3.7/lib/glut/glut_bwidth.o /Users/daniel/Install/glut-3.7/lib/glut/glut_cindex.o /Users/daniel/Install/glut-3.7/lib/glut/glut_cmap.o /Users/daniel/Install/glut-3.7/lib/glut/glut_cursor.o /Users/daniel/Install/glut-3.7/lib/glut/glut_dials.o /Users/daniel/Install/glut-3.7/lib/glut/glut_dstr.o /Users/daniel/Install/glut-3.7/lib/glut/glut_event.o /Users/daniel/Install/glut-3.7/lib/glut/glut_ext.o /Users/daniel/Install/glut-3.7/lib/glut/glut_fullscrn.o /Users/daniel/Install/glut-3.7/lib/glut/glut_gamemode.o /Users/daniel/Install/glut-3.7/lib/glut/glut_get.o /Users/daniel/Install/glut-3.7/lib/glut/glut_glxext.o /Users/daniel/Install/glut-3.7/lib/glut/glut_hel10.o /Users/daniel/Install/glut-3.7/lib/glut/glut_hel12.o /Users/daniel/Install/glut-3.7/lib/glut/glut_hel18.o /Users/daniel/Install/glut-3.7/lib/glut/glut_init.o /Users/daniel/Install/glut-3.7/lib/glut/glut_input.o /Users/daniel/Install/glut-3.7/lib/glut/glut_joy.o /Users/daniel/Install/glut-3.7/lib/glut/glut_key.o /Users/daniel/Install/glut-3.7/lib/glut/glut_keyctrl.o /Users/daniel/Install/glut-3.7/lib/glut/glut_keyup.o /Users/daniel/Install/glut-3.7/lib/glut/glut_menu.o /Users/daniel/Install/glut-3.7/lib/glut/glut_menu2.o /Users/daniel/Install/glut-3.7/lib/glut/glut_mesa.o /Users/daniel/Install/glut-3.7/lib/glut/glut_modifier.o /Users/daniel/Install/glut-3.7/lib/glut/glut_mroman.o /Users/daniel/Install/glut-3.7/lib/glut/glut_overlay.o /Users/daniel/Install/glut-3.7/lib/glut/glut_roman.o /Users/daniel/Install/glut-3.7/lib/glut/glut_shapes.o /Users/daniel/Install/glut-3.7/lib/glut/glut_space.o /Users/daniel/Install/glut-3.7/lib/glut/glut_stroke.o /Users/daniel/Install/glut-3.7/lib/glut/glut_swap.o /Users/daniel/Install/glut-3.7/lib/glut/glut_swidth.o /Users/daniel/Install/glut-3.7/lib/glut/glut_tablet.o /Users/daniel/Install/glut-3.7/lib/glut/glut_teapot.o /Users/daniel/Install/glut-3.7/lib/glut/glut_tr10.o /Users/daniel/Install/glut-3.7/lib/glut/glut_tr24.o /Users/daniel/Install/glut-3.7/lib/glut/glut_util.o /Users/daniel/Install/glut-3.7/lib/glut/glut_vidresize.o /Users/daniel/Install/glut-3.7/lib/glut/glut_warp.o /Users/daniel/Install/glut-3.7/lib/glut/glut_win.o /Users/daniel/Install/glut-3.7/lib/glut/glut_winmisc.o /Users/daniel/Install/glut-3.7/lib/glut/layerutil.o  -L/usr/X11R6/lib/ -l GL -l GLU -lX11 -lXmu -lXi  /sw/lib/libexpat.a /sw/lib/libjpeg.a /sw/lib/python2.2/config/libpython2.2.a   /Users/daniel/Install/libpng-1.2.5/libpng.a /users/daniel/install/zlib/libz.a
