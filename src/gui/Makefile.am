if USEBOOST128
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost
else
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost129
endif

noinst_LIBRARIES = libgui.a

libgui_a_SOURCES =	button.cpp control.cpp eventmanager.cpp eventresponder.cpp font.cpp general.cpp glut_support.cpp groupcontrol.cpp guidefs.cpp modaldialog.cpp newbutton.cpp painttext.cpp picker.cpp scroller.cpp simplepicker.cpp slider.cpp staticdisplay.cpp text_area.cpp window.cpp windowcontroller.cpp