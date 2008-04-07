; Kinnear's NSIS SuperPiMP VS Install Script


Page components
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage components
UninstPage instfiles
SetDatablockOptimize off ;fark.. 50 minutes to compile down to 2

  Name "VegaStrike 0.4.3"        ; caption in titlebar
  OutFile "vegastrike-043.exe"	; installer file to create
  Icon "C:\i\data4.x\vegastrike.ico"		
  BrandingText " "		; removes default 'nullsoft' branding at
				; bottom of window
  CRCCheck on			; YAY!
  LicenseText "This will install Vega Strike 0.4.3 onto your PC."
  LicenseData vega-license.txt

;default install dir, and registry entry
  AutoCloseWindow true
  InstallDir "C:\Program Files\Vegastrike"
  InstallDirRegKey HKLM SOFTWARE\Vegastrike\0.4.3\ "Install_Dir"

  ComponentText "This will install Vega Strike 0.4.3 onto your PC."
  DirText "Choose a directory to install in to:"
;  EnabledBitmap "yes.bmp"
;  DisabledBitmap "no.bmp"
  ShowInstDetails show

;first option section - install the program and write uninstall registry
;entries
  Section "Vega Strike Files (Required)"
  SectionIn RO
  SetOutPath $INSTDIR\Vegastrike-0.4.3
  File /r "C:\i\data4.x\*.*"
  WriteRegStr HKLM SOFTWARE\VegaStrike\0.4.3\ "Install_Dir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VegaStrike-0.4.3\" "DisplayName" "VegaStrike 4.3"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VegaStrike-0.4.3\" "UninstallString" '"$INSTDIR\Vegastrike-0.4.3\uninstall.exe"'
  WriteUninstaller "Vegastrike-0.4.3\uninstall.exe"
;  Rename "$INSTDIR\Vegastrike.exe" "$INSTDIR\Vegastrike.exe.oldrelease"
;  Rename "$INSTDIR\Launcher.exe" "$INSTDIR\Launcher.exe.oldrelease"
;  Rename "$INSTDIR\Setup.exe" "$INSTDIR\Setup.exe.oldrelease"
  SectionEnd

;second install option - adds the shortcuts to the start menu. optional.
  Section "Vega Strike Music (Recommended)"
  SetOutPath $INSTDIR\Vegastrike-0.4.3\music
  File /r "c:\i\music\*.*"
  SectionEnd

;third install option - adds the shortcuts to the start menu. optional.
  Section "Start Menu Shortcuts"
    CreateDirectory "$SMPROGRAMS\Vega Strike\0.4.3"
    CreateShortCut "$SMPROGRAMS\Vega Strike\0.4.3\Vega Strike Story.lnk" "$INSTDIR\Vegastrike-0.4.3\documentation\IntroMonologue.txt" "" "$INSTDIR\Vegastrike-0.4.3\documentation\IntroMonologue.txt" 0
    CreateShortCut "$SMPROGRAMS\Vega Strike\0.4.3\Vega Strike Manual.lnk" "$INSTDIR\Vegastrike-0.4.3\documentation\Vega_Strike_Players_Guide.pdf" "" "$INSTDIR\Vegastrike-0.4.3\documentation\Vega_Strike_Players_Guide.pdf" 0
;    CreateShortCut "$SMPROGRAMS\Vega Strike\0.4.3\Vega Strike Readme.lnk" "$INSTDIR\Vegastrike-0.4.3\documentation\readme.txt" "" "$INSTDIR\Vegastrike-0.4.3\documentation\readme.txt" 0
    CreateShortCut "$SMPROGRAMS\Vega Strike\0.4.3\Screenshots.lnk" "explorer.exe" "$INSTDIR\Vegastrike-0.4.3\.vegastrike\textures" "c:\windows\explorer.exe" 0
    CreateShortCut "$SMPROGRAMS\Vega Strike\0.4.3\Vega Strike.lnk" "$INSTDIR\Vegastrike-0.4.3\bin\vegastrike.exe" "" "$INSTDIR\Vegastrike-0.4.3\bin\vegastrike.exe" 0
    CreateShortCut "$SMPROGRAMS\Vega Strike\0.4.3\Vega Strike Setup.lnk" "$INSTDIR\Vegastrike-0.4.3\bin\setup.exe" "" "$INSTDIR\Vegastrike-0.4.3\bin\setup.exe" 0
    CreateShortCut "$SMPROGRAMS\Vega Strike\0.4.3\Vega Strike Uninstall.lnk" "$INSTDIR\Vegastrike-0.4.3\uninstall.exe" "" "$INSTDIR\Vegastrike-0.4.3\uninstall.exe" 0
;    CreateShortCut "$SMPROGRAMS\Vega Strike\0.4.3\Vega Strike Updater.lnk" "$INSTDIR\Vegastrike-0.4.3\bin\AutoUpdate.bat" "" "$INSTDIR\Vegastrike-0.4.3\bin\AutoUpdate.bat" 0
  SectionEnd

;Other Functions - this one is what to do once install is completed

Function GetWindowsVersion
 
   Push $R0
   Push $R1
 
   ReadRegStr $R0 HKLM \
   "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion

   IfErrors 0 lbl_winnt
   
   ; we are not NT
   ReadRegStr $R0 HKLM \
   "SOFTWARE\Microsoft\Windows\CurrentVersion" VersionNumber
 
   StrCpy $R1 $R0 1
   StrCmp $R1 '4' 0 lbl_error
 
   StrCpy $R1 $R0 3
 
   StrCmp $R1 '4.0' lbl_win32_95
   StrCmp $R1 '4.9' lbl_win32_ME lbl_win32_98
 
   lbl_win32_95:
     StrCpy $R0 '95'
   Goto lbl_done
 
   lbl_win32_98:
     StrCpy $R0 '98'
   Goto lbl_done
 
   lbl_win32_ME:
     StrCpy $R0 'ME'
   Goto lbl_done
 
   lbl_winnt:
 
   StrCpy $R1 $R0 1
 
   StrCmp $R1 '3' lbl_winnt_x
   StrCmp $R1 '4' lbl_winnt_x
 
   StrCpy $R1 $R0 3
 
   StrCmp $R1 '5.0' lbl_winnt_2000
   StrCmp $R1 '5.1' lbl_winnt_XP
   StrCmp $R1 '5.2' lbl_winnt_2003 lbl_error
 
   lbl_winnt_x:
     StrCpy $R0 "NT $R0" 6
   Goto lbl_done
 
   lbl_winnt_2000:
     Strcpy $R0 '2000'
   Goto lbl_done
 
   lbl_winnt_XP:
     Strcpy $R0 'XP'
   Goto lbl_done
 
   lbl_winnt_2003:
     Strcpy $R0 '2003'
   Goto lbl_done
 
   lbl_error:
     Strcpy $R0 ''
   lbl_done:
 
   Pop $R1
   Exch $R0
 
FunctionEnd

Function .onInstSuccess
  Call GetWindowsVersion
  Pop $R0
  StrCmp $R0 '98' lbl_copy
  StrCmp $R0 '95' lbl_copy
  StrCmp $R0 'ME' lbl_copy lbl_avoid_copy
lbl_copy:
  Rename /REBOOTOK $INSTDIR\bin\SDL.dll $INSTDIR\bin\SDL2k.dll
  Rename /REBOOTOK $INSTDIR\bin\SDL98.dll $INSTDIR\bin\SDL.dll
lbl_avoid_copy:
;        ExecWait $INSTDIR/Vegastrike-0.4.3/bin/OpenALwEAX.exe
      ExecWait $INSTDIR/Vegastrike-0.4.3/bin/SETUP.EXE 
      MessageBox MB_YESNO "Installation Successful. View Manual?" IDNO NoReadme
         ExecShell "open" $INSTDIR\Vegastrike-0.4.3\documentation\Vega_Strike_Players_Guide.pdf
         NoReadme:
      MessageBox MB_YESNO "Would you like to view the story behind Vega Strike?" IDNO NoStory
         ExecShell "open" "$INSTDIR\Vegastrike-0.4.3\history\a brief history in time and space.pdf"
         NoStory:
  FunctionEnd

 Function .onInstFailed
        MessageBox MB_OK "Installation Cancelled or Data corrupt."  
 FunctionEnd

;uninstaller stuff
UninstallText "This will delete the following directory and remove ALL of its contents, including your saved games. Hit the uninstall button to continue."
UninstallIcon "C:\i\data4.x\uninstall.ico"


Section "un.Uninstall Data Files"
  SectionIn RO
RMDir /r "$INSTDIR\accounts"

Delete "$INSTDIR\021.nsi"
Delete "$INSTDIR\021.nsi.~1.5.~"
Delete "$INSTDIR\BFXMupdate.pl"
Delete "$INSTDIR\CVS"
Delete "$INSTDIR\Makefile.am"
Delete "$INSTDIR\Makefile.in"
Delete "$INSTDIR\vega-license.txt"
Delete "$INSTDIR\uninstall.ico"
Delete "$INSTDIR\README"
Delete "$INSTDIR\Vega.icns"
Delete "$INSTDIR\Version.txt"
Delete "$INSTDIR\accounts"
Delete "$INSTDIR\accountserver.config"
RMDir /r "$INSTDIR\ai"
RMDir /r "$INSTDIR\animations"
RMDir /r "$INSTDIR\bases"
RMDir /r "$INSTDIR\bin"
RMDir /r "$INSTDIR\cockpits"
RMDir /r "$INSTDIR\communications"
Delete "$INSTDIR\configure.ac"
Delete "$INSTDIR\convertbfxm.sh"
Delete "$INSTDIR\cursor1.cur"
RMDir /r "$INSTDIR\documentation"
Delete "$INSTDIR\dynaverse.dat"
Delete "$INSTDIR\factions.xml"
Delete "$INSTDIR\favicon.ico"
Delete "$INSTDIR\fixmusic.sh"
RMDir /r "$INSTDIR\history"
Delete "$INSTDIR\m3uloki_add.sh"
Delete "$INSTDIR\m3uloki_remove.sh"
Delete "$INSTDIR\makeloki.sh"
Delete "$INSTDIR\master_part_list.csv"
Delete "$INSTDIR\master_part_list.xml"
RMDir /r "$INSTDIR\meshes"
RMDir /r "$INSTDIR\mission"
RMDir /r "$INSTDIR\mods"
RMDir /r "$INSTDIR\modules"
RMDir /r "$INSTDIR\nav"
Delete "$INSTDIR\newfs_changes.txt"
Delete "$INSTDIR\play_vs"
RMDir /r "$INSTDIR\sectors"
Delete "$INSTDIR\setup.base.xml"
Delete "$INSTDIR\setup.config"
Delete "$INSTDIR\setup.music.xml"
Delete "$INSTDIR\setup.xml"
RMDir /r "$INSTDIR\sounds"
RMDir /r "$INSTDIR\sprites"
RMDir /r "$INSTDIR\stats"
RMDir /r "$INSTDIR\textures"
RMDir /r "$INSTDIR\units"
RMDir /r "$INSTDIR\universe"
Delete "$INSTDIR\vega.ico"
Delete "$INSTDIR\vegaserver.config"
Delete "$INSTDIR\vegastrike-data.spec"
Delete "$INSTDIR\vegastrike.config"
Delete "$INSTDIR\vegastrike.config.2player"
Delete "$INSTDIR\vegastrike.ico"
Delete "$INSTDIR\vegastrike.sh"
Delete "$INSTDIR\vegastrike.xpm"
Delete "$INSTDIR\vsinstall.sh"
Delete "$INSTDIR\vslogo.xpm"
Delete "$INSTDIR\weapon_list.xml"
  Delete "$SMPROGRAMS\Vega Strike\0.4.3\*.*"
  RMDir /r "$SMPROGRAMS\Vega Strike\0.4.3\"
  RMDir "$INSTDIR\"
  DeleteRegKey HKLM  "Software\Microsoft\Windows\CurrentVersion\Uninstall\VegaStrike-0.4.3\"
  DeleteRegKey HKLM  "SOFTWARE\VegaStrike\0.4.3\" 

SectionEnd
Section "un.Remove Savegame/Universe Data directory (optional)" 
  RMDir /r "$INSTDIR\.vegastrike"
  RMDir "$INSTDIR\"
SectionEnd
; note - i haven't made the uninstaller remove VegaStrike folder. This is
; because we don't want it to delete EVERY copy of VS.. just the one
; they're uninstalling. :)

; eof
