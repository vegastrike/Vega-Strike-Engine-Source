noinst_LIBRARIES = libgfx.a libgfxserver.a
INCLUDES = -I ../ 
SUBDIRS = nav
libgfxgeneric =  \
	cockpit_generic.h	\
	cockpit_generic.cpp	\
	matrix.cpp	\
	matrix.h	\
	vec.h	\
	vec.cpp	\
	quaternion.cpp	\
	quaternion.h	\
	bounding_box.cpp	\
	bounding_box.h	\
	mesh.cpp	\
	mesh.h		\
	mesh_bin.cpp	\
	mesh_fx.cpp	\
	mesh_poly.cpp	\
	mesh_xml.cpp	\
	sphere_generic.cpp	\
	bsp.cpp		\
	bsp.h		\
	vertex.h		

libgfxserver_a_SOURCES =	$(libgfxgeneric)	\
	sphere_server.cpp	\
	vsbox_server.cpp	\
	mesh_server.cpp

libgfx_a_SOURCES =	$(libgfxgeneric)\
	mesh_gfx.cpp	\
	ani_texture.h	\
	ani_texture.cpp	\
	animation.cpp	\
	animation.h	\
	aux_logo.cpp	\
	aux_logo.h	\
	masks.h		\
	masks.cpp	\
	aux_palette.cpp	\
	aux_palette.h	\
	aux_texture.cpp	\
	aux_texture.h	\
	png_texture.cpp	\
	png_texture.h	\
	background.cpp	\
	background.h	\
	car_assist.h 	\
	vsbox.cpp		\
	vsbox.h		\
	camera.cpp	\
	camera.h	\
	cockpit.h	\
	cockpit.cpp	\
	cockpit_xml.cpp	\
	coord_select.cpp	\
	coord_select.h	\
	decalqueue.h	\
	env_map_gent.cpp	\
	env_map_gent.h	\
	gauge.cpp	\
	gauge.h		\
	halo.cpp	\
	halo_system.cpp	\
	halo.h		\
	hud.cpp		\
	hud.h		\
	lerp.cpp	\
	lerp.h		\
	loc_select.cpp	\
	loc_select.h	\
	nonlinear_transform.h \
	planetary_transform.h	\
	pipelined_texture.cpp	\
	pipelined_texture.h	\
	png_write.cpp	\
	point_to_cam.h	\
	particle.cpp	\
	particle.h	\
	quadtree.cpp	\
	quadtree.h	\
	quadsquare.cpp	\
	quadsquare.h	\
	quadsquare_render.cpp	\
	quadsquare_update.cpp	\
	quadsquare_cull.cpp	\
	quadtree_xml.cpp \
	star.h		\
	star.cpp	\
	sphere.cpp	\
	ring.cpp	\
	ring.h		\
	sphere.h	\
	sprite.cpp	\
	sprite.h	\
	vdu.cpp		\
	warptrail.h	\
	warptrail.cpp	\
	vdu.h

