# Microsoft Developer Studio Project File - Name="jrtplib2_7" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=jrtplib2_7 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "jrtplib2_7.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "jrtplib2_7.mak" CFG="jrtplib2_7 - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "jrtplib2_7 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "jrtplib2_7 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "jrtplib2_7 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x80c /d "NDEBUG"
# ADD RSC /l 0x80c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\jrtplib-2.7.lib"

!ELSEIF  "$(CFG)" == "jrtplib2_7 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x80c /d "_DEBUG"
# ADD RSC /l 0x80c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\jrtplib-2.7.lib"

!ENDIF

# Begin Target

# Name "jrtplib2_7 - Win32 Release"
# Name "jrtplib2_7 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\rtpconnection.cpp
# End Source File
# Begin Source File

SOURCE=.\rtpcontributingsources.cpp
# End Source File
# Begin Source File

SOURCE=.\rtpdebug.cpp
# End Source File
# Begin Source File

SOURCE=.\rtpdestlist.cpp
# End Source File
# Begin Source File

SOURCE=.\rtperror.cpp
# End Source File
# Begin Source File

SOURCE=.\rtphandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\rtpiptable.cpp
# End Source File
# Begin Source File

SOURCE=.\rtplocalinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\rtpmcasttable.cpp
# End Source File
# Begin Source File

SOURCE=.\rtppacket.cpp
# End Source File
# Begin Source File

SOURCE=.\rtppacketprocessor.cpp
# End Source File
# Begin Source File

SOURCE=.\rtprandom.cpp
# End Source File
# Begin Source File

SOURCE=.\rtprtcpmodule.cpp
# End Source File
# Begin Source File

SOURCE=.\rtpsession.cpp
# End Source File
# Begin Source File

SOURCE=.\rtpsourcedata.cpp
# End Source File
# Begin Source File

SOURCE=.\rtpsourcedescription.cpp
# End Source File
# Begin Source File

SOURCE=.\rtpsources.cpp
# End Source File
# Begin Source File

SOURCE=.\rtpsrclist.cpp
# End Source File
# Begin Source File

SOURCE=.\rtptimeutil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\rtpconfig.h
# End Source File
# Begin Source File

SOURCE=.\rtpconfig_win.h
# End Source File
# Begin Source File

SOURCE=.\rtpconnection.h
# End Source File
# Begin Source File

SOURCE=.\rtpcontributingsources.h
# End Source File
# Begin Source File

SOURCE=.\rtpdebug.h
# End Source File
# Begin Source File

SOURCE=.\rtpdefines.h
# End Source File
# Begin Source File

SOURCE=.\rtpdestlist.h
# End Source File
# Begin Source File

SOURCE=.\rtperror.h
# End Source File
# Begin Source File

SOURCE=.\rtpexceptionstructs.h
# End Source File
# Begin Source File

SOURCE=.\rtphandlers.h
# End Source File
# Begin Source File

SOURCE=.\rtpiptable.h
# End Source File
# Begin Source File

SOURCE=.\rtplocalinfo.h
# End Source File
# Begin Source File

SOURCE=.\rtpmcasttable.h
# End Source File
# Begin Source File

SOURCE=.\rtppacket.h
# End Source File
# Begin Source File

SOURCE=.\rtppacketprocessor.h
# End Source File
# Begin Source File

SOURCE=.\rtprandom.h
# End Source File
# Begin Source File

SOURCE=.\rtprtcpmodule.h
# End Source File
# Begin Source File

SOURCE=.\rtpsession.h
# End Source File
# Begin Source File

SOURCE=.\rtpsourcedata.h
# End Source File
# Begin Source File

SOURCE=.\rtpsourcedescription.h
# End Source File
# Begin Source File

SOURCE=.\rtpsources.h
# End Source File
# Begin Source File

SOURCE=.\rtpsrclist.h
# End Source File
# Begin Source File

SOURCE=.\rtpstructs.h
# End Source File
# Begin Source File

SOURCE=.\rtptimeutil.h
# End Source File
# End Group
# End Target
# End Project
