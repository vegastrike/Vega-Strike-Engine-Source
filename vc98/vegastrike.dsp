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
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "HAVE_SDL" /YX /FD /c
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

SOURCE=\src\cmd_ai.cpp
# End Source File
# Begin Source File

SOURCE=\src\cmd_gun.cpp
# End Source File
# Begin Source File

SOURCE=\src\cmd_physics.cpp
# End Source File
# Begin Source File

SOURCE=\src\cmd_unit.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_animation.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_aux_logo.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_aux_palette.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_aux_texture.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_background.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_bsp.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_camera.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_hud.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_light.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_mesh.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_primitive.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_sprite.cpp
# End Source File
# Begin Source File

SOURCE=\src\gfx_transform_vector.cpp
# End Source File
# Begin Source File

SOURCE=\src\gl_globals.cpp
# End Source File
# Begin Source File

SOURCE=\src\gl_init.cpp
# End Source File
# Begin Source File

SOURCE=\src\gl_light.cpp
# End Source File
# Begin Source File

SOURCE=\src\gl_material.cpp
# End Source File
# Begin Source File

SOURCE=\src\gl_matrix.cpp
# End Source File
# Begin Source File

SOURCE=\src\gl_misc.cpp
# End Source File
# Begin Source File

SOURCE=\src\gl_state.cpp
# End Source File
# Begin Source File

SOURCE=\src\gl_texture.cpp
# End Source File
# Begin Source File

SOURCE=\src\gl_vertex_list.cpp
# End Source File
# Begin Source File

SOURCE=\src\hashtable.cpp
# End Source File
# Begin Source File

SOURCE=\src\in_handler.cpp
# End Source File
# Begin Source File

SOURCE=\src\in_kb.cpp
# End Source File
# Begin Source File

SOURCE=\src\in_main.cpp
# End Source File
# Begin Source File

SOURCE=\src\in_mouse.cpp
# End Source File
# Begin Source File

SOURCE=\src\lin_time.cpp
# End Source File
# Begin Source File

SOURCE=\src\main.cpp
# End Source File
# Begin Source File

SOURCE=\src\main_loop.cpp
# End Source File
# Begin Source File

SOURCE=\src\physics.cpp
# End Source File
# Begin Source File

SOURCE=\src\quadgeom.cpp
# End Source File
# Begin Source File

SOURCE=\src\wrapgfx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=\src\alglib.h
# End Source File
# Begin Source File

SOURCE=\src\cmd.h
# End Source File
# Begin Source File

SOURCE=\src\cmd_ai.h
# End Source File
# Begin Source File

SOURCE=\src\cmd_gun.h
# End Source File
# Begin Source File

SOURCE=\src\cmd_node.h
# End Source File
# Begin Source File

SOURCE=\src\cmd_unit.h
# End Source File
# Begin Source File

SOURCE=\src\cmd_unitenum.h
# End Source File
# Begin Source File

SOURCE=\src\debug.h
# End Source File
# Begin Source File

SOURCE=\src\error_util.h
# End Source File
# Begin Source File

SOURCE=\src\file.h
# End Source File
# Begin Source File

SOURCE=\src\file_main.h
# End Source File
# Begin Source File

SOURCE=\src\file_util.h
# End Source File
# Begin Source File

SOURCE=\src\fog.h
# End Source File
# Begin Source File

SOURCE=\src\fonts.h
# End Source File
# Begin Source File

SOURCE=\src\gauss.h
# End Source File
# Begin Source File

SOURCE=\src\gfx.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_animation.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_aux.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_aux_logo.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_aux_palette.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_aux_texture.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_background.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_bsp.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_camera.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_hud.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_mesh.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_primitive.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_sprite.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_transform.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_transform_matrix.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_transform_vector.h
# End Source File
# Begin Source File

SOURCE=\src\gfx_vertex.h
# End Source File
# Begin Source File

SOURCE=\src\gfxlib.h
# End Source File
# Begin Source File

SOURCE=\src\gfxlib_struct.h
# End Source File
# Begin Source File

SOURCE=\src\gl_globals.h
# End Source File
# Begin Source File

SOURCE=\src\gl_init.h
# End Source File
# Begin Source File

SOURCE=\src\GL_util.h
# End Source File
# Begin Source File

SOURCE=\src\hash.h
# End Source File
# Begin Source File

SOURCE=\src\hashtable.h
# End Source File
# Begin Source File

SOURCE=\src\image.h
# End Source File
# Begin Source File

SOURCE=\src\in.h
# End Source File
# Begin Source File

SOURCE=\src\in_handler.h
# End Source File
# Begin Source File

SOURCE=\src\in_kb.h
# End Source File
# Begin Source File

SOURCE=\src\in_main.h
# End Source File
# Begin Source File

SOURCE=\src\in_mouse.h
# End Source File
# Begin Source File

SOURCE=\src\joystick.h
# End Source File
# Begin Source File

SOURCE=\src\lin_time.h
# End Source File
# Begin Source File

SOURCE=\src\list.h
# End Source File
# Begin Source File

SOURCE=\src\main_loop.h
# End Source File
# Begin Source File

SOURCE=\src\nmrcl.h
# End Source File
# Begin Source File

SOURCE=\src\physics.h
# End Source File
# Begin Source File

SOURCE=\src\pic.h
# End Source File
# Begin Source File

SOURCE=\src\quadgeom.hpp
# End Source File
# Begin Source File

SOURCE=\src\quanternion.h
# End Source File
# Begin Source File

SOURCE=\src\render_util.h
# End Source File
# Begin Source File

SOURCE=\src\screenshot.h
# End Source File
# Begin Source File

SOURCE=\src\sdl_audio.h
# End Source File
# Begin Source File

SOURCE=\src\sdl_audio_data.h
# End Source File
# Begin Source File

SOURCE=\src\sdl_init.h
# End Source File
# Begin Source File

SOURCE=\src\string_util.h
# End Source File
# Begin Source File

SOURCE=\src\tex_font_metrics.h
# End Source File
# Begin Source File

SOURCE=\src\textures.h
# End Source File
# Begin Source File

SOURCE=\src\vegastrike.h
# End Source File
# Begin Source File

SOURCE=\src\vs_types.h
# End Source File
# Begin Source File

SOURCE=\src\wrapgfx.h
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
