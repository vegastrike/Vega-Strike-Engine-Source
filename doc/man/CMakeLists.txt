IF (UNIX)

	# The method of using the `TYPE` argument will not allow to install the manpages into the
	# correct subdirectory, so as man pages are only installed on "UNIX" we can use GNUInstallDirs as recommended by cmake
	# This also allows distro packagers to override CMAKE_INSTALL_MANDIR if needed
	INSTALL(FILES vegastrike-engine.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")
	INSTALL(FILES vegastrike-engine.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1" RENAME vegastrike.1)
	INSTALL(FILES vsinstall.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")
	INSTALL(FILES vslauncher.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")
	INSTALL(FILES vegasettings.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

ENDIF (UNIX)
