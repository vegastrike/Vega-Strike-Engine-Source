# Microsoft Developer Studio Project File - Name="vegastrike" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=vegastrike - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vegastrike.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vegastrike.mak" CFG="vegastrike - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vegastrike - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "vegastrike - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vegastrike - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "vc98" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "HAVE_AL" /D "HAVE_SDL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib expat.lib SDL.lib glut32.lib openal32.lib alut.lib alu.lib /nologo /subsystem:windows /machine:I386 /libpath:"vc98"

!ELSEIF  "$(CFG)" == "vegastrike - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "vegastrike___Win32_Debug"
# PROP BASE Intermediate_Dir "vegastrike___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "vegastrike___Win32_Debug"
# PROP Intermediate_Dir "vegastrike___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /w /W0 /Gm /GX /ZI /Od /I "vc98" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "HAVE_AL" /D "HAVE_SDL" /Fp"Debug/vegastrike.pch" /YX /Fo"Debug/" /Fd"Debug/" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib expat.lib SDL.lib glut32.lib openal32.lib alut.lib alu.lib SDLmain.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /out:"Debug/vegastrike.exe" /pdbtype:sept /libpath:"vc98"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "vegastrike - Win32 Release"
# Name "vegastrike - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "cmd"

# PROP Default_Filter "*.*"
# Begin Group "ai"

# PROP Default_Filter "*.*"
# Begin Source File

SOURCE=.\src\cmd\ai\aggressive.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\aggressive.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\event_xml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\event_xml.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\fire.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\fire.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\firekeyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\firekeyboard.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\flybywire.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\flybywire.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\flyjoystick.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\flyjoystick.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\flykeyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\flykeyboard.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\input_dfa.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\input_dfa.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\navigation.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\navigation.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\order.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\order.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\script.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\script.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\tactics.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\ai\tactics.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\cmd\beam.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\beam.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\bolt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\bolt.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\click_list.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\click_list.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\collection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\collection.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\container.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\container.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\faction_xml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\iterator.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\music.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\music.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\planet.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\planet.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\unit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\unit.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\unit_bsp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\unit_click.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\unit_collide.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\unit_collide.h
# End Source File
# Begin Source File

SOURCE=.\src\cmd\unit_damage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\unit_physics.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\unit_weapon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\unit_xml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\weapon_xml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmd\weapon_xml.h
# End Source File
# End Group
# Begin Group "gldrv"

# PROP Default_Filter "*.*"
# Begin Source File

SOURCE=.\src\gldrv\gl_globals.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_globals.h
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_init.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_init.h
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_light.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_light.h
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_light_pick.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_light_state.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_material.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_matrix.h
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_misc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_pick.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_quad_list.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_state.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_texture.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gldrv\gl_vertex_list.cpp
# End Source File
# End Group
# Begin Group "aldrv"

# PROP Default_Filter "*.*"
# Begin Source File

SOURCE=.\src\aldrv\al_globals.cpp
# End Source File
# Begin Source File

SOURCE=.\src\aldrv\al_globals.h
# End Source File
# Begin Source File

SOURCE=.\src\aldrv\al_init.cpp
# End Source File
# Begin Source File

SOURCE=.\src\aldrv\al_listen.cpp
# End Source File
# Begin Source File

SOURCE=.\src\aldrv\al_sound.cpp
# End Source File
# End Group
# Begin Group "gfx"

# PROP Default_Filter "*.*"
# Begin Source File

SOURCE=.\src\gfx\animation.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\animation.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\aux_logo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\aux_logo.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\aux_palette.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\aux_palette.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\aux_texture.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\aux_texture.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\background.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\background.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\bounding_box.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\bounding_box.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\box.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\box.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\bsp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\bsp.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\camera.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\camera.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\cockpit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\cockpit.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\cockpit_xml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\coord_select.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\coord_select.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\decalqueue.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\env_map_gent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\env_map_gent.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\gauge.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\gauge.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\halo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\halo.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\hud.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\hud.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\lerp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\lerp.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\loc_select.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\loc_select.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\matrix.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\mesh.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\mesh.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\mesh_bin.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\mesh_fx.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\mesh_poly.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\mesh_xml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\quaternion.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\quaternion.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\sphere.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\sphere.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\sprite.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\star.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\star.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\vdu.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\vdu.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\vec.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gfx\vec.h
# End Source File
# Begin Source File

SOURCE=.\src\gfx\vertex.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\config_xml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\easydom.cpp
# End Source File
# Begin Source File

SOURCE=.\src\hashtable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\in_handler.cpp
# End Source File
# Begin Source File

SOURCE=.\src\in_joystick.cpp
# End Source File
# Begin Source File

SOURCE=.\src\in_kb.cpp
# End Source File
# Begin Source File

SOURCE=.\src\in_main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\in_mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\src\in_sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lin_time.cpp
# End Source File
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\main_loop.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mission.cpp
# End Source File
# Begin Source File

SOURCE=.\src\physics.cpp
# End Source File
# Begin Source File

SOURCE=.\src\star_system.cpp
# End Source File
# Begin Source File

SOURCE=.\src\star_system_xml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\universe.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vs_path.cpp
# End Source File
# Begin Source File

SOURCE=.\src\xml_support.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\audiolib.h
# End Source File
# Begin Source File

SOURCE=.\src\config_xml.h
# End Source File
# Begin Source File

SOURCE=.\src\easydom.h
# End Source File
# Begin Source File

SOURCE=.\src\endianness.h
# End Source File
# Begin Source File

SOURCE=.\src\file_main.h
# End Source File
# Begin Source File

SOURCE=.\src\gfxlib.h
# End Source File
# Begin Source File

SOURCE=.\src\gfxlib_struct.h
# End Source File
# Begin Source File

SOURCE=.\src\hashtable.h
# End Source File
# Begin Source File

SOURCE=.\src\hashtable_3d.h
# End Source File
# Begin Source File

SOURCE=.\src\in.h
# End Source File
# Begin Source File

SOURCE=.\src\in_handler.h
# End Source File
# Begin Source File

SOURCE=.\src\in_joystick.h
# End Source File
# Begin Source File

SOURCE=.\src\in_kb.h
# End Source File
# Begin Source File

SOURCE=.\src\in_main.h
# End Source File
# Begin Source File

SOURCE=.\src\in_mouse.h
# End Source File
# Begin Source File

SOURCE=.\src\lin_time.h
# End Source File
# Begin Source File

SOURCE=.\src\linecollide.h
# End Source File
# Begin Source File

SOURCE=.\src\main_loop.h
# End Source File
# Begin Source File

SOURCE=.\src\mission.h
# End Source File
# Begin Source File

SOURCE=.\src\physics.h
# End Source File
# Begin Source File

SOURCE=.\src\profile.h
# End Source File
# Begin Source File

SOURCE=.\src\star_system.h
# End Source File
# Begin Source File

SOURCE=.\src\universe.h
# End Source File
# Begin Source File

SOURCE=.\src\vegastrike.h
# End Source File
# Begin Source File

SOURCE=.\src\vs_globals.h
# End Source File
# Begin Source File

SOURCE=.\src\vs_path.h
# End Source File
# Begin Source File

SOURCE=.\src\xml_support.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\vc98\SDL.lib
# End Source File
# Begin Source File

SOURCE=.\vc98\SDLmain.lib
# End Source File
# End Target
# End Project
