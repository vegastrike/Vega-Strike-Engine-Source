# Microsoft Developer Studio Project File - Name="vegastrike" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

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
!MESSAGE "vegastrike - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "vegastrike - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W1 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "HAVE_SDL" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "HAVE_SDL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "vegastrike - Win32 Release"
# Name "vegastrike - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Src\Alglib.c
# End Source File
# Begin Source File

SOURCE=..\Src\Cmd_gun.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\cmd_input_dfa.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\cmd_order.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\cmd_physics.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Cmd_unit.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\cmd_unit_xml.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Debug.c
# End Source File
# Begin Source File

SOURCE=..\Src\error_util.c
# End Source File
# Begin Source File

SOURCE=..\Src\file_util.c
# End Source File
# Begin Source File

SOURCE=..\Src\Fog.c
# End Source File
# Begin Source File

SOURCE=..\Src\Gauss.c
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_animation.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_aux_logo.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_aux_palette.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_aux_texture.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_background.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_bounding_box.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Gfx_box.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Gfx_bsp.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_camera.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_click_list.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_coordinate_select.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Gfx_hud.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Gfx_lerp.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_location_select.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Gfx_mesh.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_mesh_xml.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_primitive.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_sphere.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_sprite.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_transform_matrix.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_transform_vector.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gl_globals.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Gl_init.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Gl_light.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gl_material.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gl_matrix.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Gl_misc.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Gl_pick.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gl_quadstrip.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Gl_state.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\gl_texture.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\GL_util.c
# End Source File
# Begin Source File

SOURCE=..\Src\gl_vertex_list.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\hashtable.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Image.c
# End Source File
# Begin Source File

SOURCE=..\Src\in_handler.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\In_kb.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\In_main.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\In_mouse.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Joystick.c
# End Source File
# Begin Source File

SOURCE=..\Src\Lin_time.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\List.c
# End Source File
# Begin Source File

SOURCE=..\Src\Main.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\main_loop.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Nmrcl.c
# End Source File
# Begin Source File

SOURCE=..\Src\Physics.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Pic.c
# End Source File
# Begin Source File

SOURCE=..\Src\Planet.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Quadgeom.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\quaternion.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\screenshot.c
# End Source File
# Begin Source File

SOURCE=..\Src\sdl_audio.c
# End Source File
# Begin Source File

SOURCE=..\Src\sdl_audio_data.c
# End Source File
# Begin Source File

SOURCE=..\Src\Sdl_init.c
# End Source File
# Begin Source File

SOURCE=..\Src\star_system.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\string_util.c
# End Source File
# Begin Source File

SOURCE=..\Src\tex_font_metrics.c
# End Source File
# Begin Source File

SOURCE=..\Src\UnitCollection.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Wrapgfx.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\xml_support.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Src\Alglib.h
# End Source File
# Begin Source File

SOURCE=..\Src\Cmd.h
# End Source File
# Begin Source File

SOURCE=..\Src\Cmd_ai.h
# End Source File
# Begin Source File

SOURCE=..\Src\Cmd_gun.h
# End Source File
# Begin Source File

SOURCE=..\Src\cmd_input_dfa.h
# End Source File
# Begin Source File

SOURCE=..\Src\cmd_navigation_orders.h
# End Source File
# Begin Source File

SOURCE=..\Src\Cmd_node.h
# End Source File
# Begin Source File

SOURCE=..\Src\cmd_order.h
# End Source File
# Begin Source File

SOURCE=..\Src\Cmd_unit.h
# End Source File
# Begin Source File

SOURCE=..\Src\cmd_unitenum.h
# End Source File
# Begin Source File

SOURCE=..\Src\Debug.h
# End Source File
# Begin Source File

SOURCE=..\Src\error_util.h
# End Source File
# Begin Source File

SOURCE=..\Src\File.h
# End Source File
# Begin Source File

SOURCE=..\Src\file_main.h
# End Source File
# Begin Source File

SOURCE=..\Src\file_util.h
# End Source File
# Begin Source File

SOURCE=..\Src\Fog.h
# End Source File
# Begin Source File

SOURCE=..\Src\Fonts.h
# End Source File
# Begin Source File

SOURCE=..\Src\Gauss.h
# End Source File
# Begin Source File

SOURCE=..\Src\Gfx.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_animation.h
# End Source File
# Begin Source File

SOURCE=..\Src\Gfx_aux.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_aux_logo.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_aux_palette.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_aux_texture.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_background.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_bounding_box.h
# End Source File
# Begin Source File

SOURCE=..\Src\Gfx_box.h
# End Source File
# Begin Source File

SOURCE=..\Src\Gfx_bsp.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_camera.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_click_list.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_coordinate_select.h
# End Source File
# Begin Source File

SOURCE=..\Src\Gfx_hud.h
# End Source File
# Begin Source File

SOURCE=..\Src\Gfx_lerp.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_location_select.h
# End Source File
# Begin Source File

SOURCE=..\Src\Gfx_mesh.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_primitive.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_sphere.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_sprite.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_transform.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_transform_matrix.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_transform_vector.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfx_vertex.h
# End Source File
# Begin Source File

SOURCE=..\Src\Gfxlib.h
# End Source File
# Begin Source File

SOURCE=..\Src\gfxlib_struct.h
# End Source File
# Begin Source File

SOURCE=..\Src\gl_globals.h
# End Source File
# Begin Source File

SOURCE=..\Src\Gl_init.h
# End Source File
# Begin Source File

SOURCE=..\Src\gl_matrix.h
# End Source File
# Begin Source File

SOURCE=..\Src\GL_util.h
# End Source File
# Begin Source File

SOURCE=..\Src\Hash.h
# End Source File
# Begin Source File

SOURCE=..\Src\hashtable.h
# End Source File
# Begin Source File

SOURCE=..\Src\Image.h
# End Source File
# Begin Source File

SOURCE=..\Src\In.h
# End Source File
# Begin Source File

SOURCE=..\Src\in_handler.h
# End Source File
# Begin Source File

SOURCE=..\Src\In_kb.h
# End Source File
# Begin Source File

SOURCE=..\Src\In_main.h
# End Source File
# Begin Source File

SOURCE=..\Src\In_mouse.h
# End Source File
# Begin Source File

SOURCE=..\Src\Iterator.h
# End Source File
# Begin Source File

SOURCE=..\Src\Joystick.h
# End Source File
# Begin Source File

SOURCE=..\Src\Lin_time.h
# End Source File
# Begin Source File

SOURCE=..\Src\List.h
# End Source File
# Begin Source File

SOURCE=..\Src\main_loop.h
# End Source File
# Begin Source File

SOURCE=..\Src\Nmrcl.h
# End Source File
# Begin Source File

SOURCE=..\Src\Physics.h
# End Source File
# Begin Source File

SOURCE=..\Src\Pic.h
# End Source File
# Begin Source File

SOURCE=..\Src\Planet.h
# End Source File
# Begin Source File

SOURCE=..\Src\Profile.h
# End Source File
# Begin Source File

SOURCE=..\Src\Quadgeom.hpp
# End Source File
# Begin Source File

SOURCE=..\Src\quaternion.h
# End Source File
# Begin Source File

SOURCE=..\Src\render_util.h
# End Source File
# Begin Source File

SOURCE=..\Src\screenshot.h
# End Source File
# Begin Source File

SOURCE=..\Src\sdl_audio.h
# End Source File
# Begin Source File

SOURCE=..\Src\sdl_audio_data.h
# End Source File
# Begin Source File

SOURCE=..\Src\Sdl_init.h
# End Source File
# Begin Source File

SOURCE=..\Src\star_system.h
# End Source File
# Begin Source File

SOURCE=..\Src\string_util.h
# End Source File
# Begin Source File

SOURCE=..\Src\tex_font_metrics.h
# End Source File
# Begin Source File

SOURCE=..\Src\Textures.h
# End Source File
# Begin Source File

SOURCE=..\Src\UnitCollection.h
# End Source File
# Begin Source File

SOURCE=..\Src\vegastrike.h
# End Source File
# Begin Source File

SOURCE=..\Src\vs_globals.h
# End Source File
# Begin Source File

SOURCE=..\Src\Vs_types.h
# End Source File
# Begin Source File

SOURCE=..\Src\Wrapgfx.h
# End Source File
# Begin Source File

SOURCE=..\Src\xml_support.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "SDL"

# PROP Default_Filter ".lib,.sdl"
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\begin_code.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\close_code.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_active.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_audio.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_byteorder.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_cdrom.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_copying.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_endian.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_error.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_events.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_joystick.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_keyboard.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_keysym.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_main.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_mouse.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_mutex.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_quit.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_rwops.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_syswm.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_thread.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_timer.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_types.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_version.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\include\SDL_video.h"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\lib\SDLmain.lib"
# End Source File
# Begin Source File

SOURCE=".\sdl\SDL-1.1.7\lib\SDL.lib"
# End Source File
# End Group
# End Target
# End Project
