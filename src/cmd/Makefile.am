noinst_LIBRARIES = libcmd.a libcmdserver.a
SUBDIRS = ai script collide
INCLUDES = -I ../

libcmdgeneric =			\
	unit_generic.h					\
	unit_generic.cpp				\
	asteroid_generic.cpp			\
	asteroid_generic.h			\
	nebula_generic.cpp			\
	nebula_generic.h			\
	building_generic.h			\
	building_generic.cpp			\
	planet_generic.h 				\
	planet_generic.cpp 				\
	enhancement_generic.h			\
	missile_generic.h			\
	missile_generic.cpp			\
	unit_util.h						\
	unit_xml.cpp				\
	unit_util_generic.cpp			\
	unit_util_generic.cpp			\
	unit_factory_generic.h			\
	unit_factory_generic.cpp		\
	unit_functions_generic.cpp		\
	iterator.h						\
	images.h						\
	collection.h					\
	collection.cpp					\
	container.h						\
	container.cpp	\
	unit_const_cache.cpp \
	unit_const_cache.h	\
	role_bitmask.cpp	\
	role_bitmask.h

libcmdserver_a_SOURCES =$(libcmdgeneric)			\
	unit_util_server.cpp			\
	unit_functions_server.cpp			\
	unit_factory_server.cpp


libcmd_a_SOURCES =$(libcmdgeneric) 				\
	atmosphere.h			\
	atmosphere.cpp			\
	asteroid.cpp			\
	asteroid.h			\
	base.h				\
	base_interface.cpp		\
	base_init.cpp			\
	base_xml.cpp			\
	base_util.cpp			\
	base_util.h			\
	base_write_xml.cpp		\
	base_write_python.cpp		\
	beam.h				\
	beam.cpp				\
	briefing.h			\
	briefing.cpp			\
	bolt.h				\
	bolt.cpp				\
	building.h			\
	building.cpp			\
	cont_terrain.cpp		\
	cont_terrain.h		\
	click_list.cpp			\
	click_list.h			\
	unit_collide.h				\
	unit_util.h			\
	unit_util.cpp			\
	faction_xml.cpp			\
	music.cpp			\
	music.h				\
	enhancement.h			\
	nebula.cpp			\
	nebula.h			\
	terrain.cpp			\
	terrain.h			\
	unit.h				\
	unit_interface.cpp		\
	unit_bsp.cpp		\
	unit_factory.cpp		\
	weapon_xml.cpp			\
	weapon_xml.h			\
	planet.h 				\
	planet.cpp 				\
	missile.h			\
	unit_functions.cpp	\
	mount.cpp		\
	mount.h			\
	collide.h		\
	collide.cpp

