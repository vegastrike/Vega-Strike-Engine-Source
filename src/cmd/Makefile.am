
noinst_LIBRARIES = libcmd.a libcmdserver.a
SUBDIRS = ai script collide

libcmdgeneric =			\
	collide_map.cpp	\
	unit_bsp.cpp	\
	csv.cpp		\
	csv.h		\
	unit_generic.h					\
	unit_generic.cpp				\
	unit_csv.cpp	\
	atmosphere_generic.cpp			\
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
	mount.cpp				\
	container.h						\
	container.cpp	\
	unit_const_cache.cpp \
	unit_const_cache.h	\
	alphacurve.cpp	\
	alphacurve.h	\
	unit_collide.h				\
	unit_collide.cpp			\
	collide.h		\
	collide.cpp		\
	bolt.h			\
	bolt_generic.cpp	\
	beam.h			\
	beam_generic.cpp	\
	weapon_xml.cpp			\
	weapon_xml.h			\
	faction_xml.cpp			\
	role_bitmask.cpp	\
	role_bitmask.h

libcmdserver_a_SOURCES =$(libcmdgeneric)			\
	atmosphere_server.cpp			\
	unit_util_server.cpp			\
	unit_functions_server.cpp			\
	bolt_server.cpp			\
	beam_server.cpp			\
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
	basecomputer.cpp		\
	beam.cpp				\
	briefing.h			\
	briefing.cpp			\
	bolt.cpp				\
	building.h			\
	building.cpp			\
	cont_terrain.cpp		\
	cont_terrain.h		\
	click_list.cpp			\
	click_list.h			\
	unit_util.h			\
	unit_util.cpp			\
	music.cpp			\
	music.h				\
	enhancement.h			\
	nebula.cpp			\
	nebula.h			\
	terrain.cpp			\
	terrain.h			\
	unit.h				\
	unit_interface.cpp		\
	unit_factory.cpp		\
	planet.h 				\
	planet.cpp 				\
	missile.h			\
	unit_functions.cpp

#	unit_jump.cpp				\
#	unit_customize.cpp			\
#	unit_damage.cpp				\
#	unit_physics.cpp			\
#	unit.cpp
