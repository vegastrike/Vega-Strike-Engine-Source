/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * This code taken from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _RENDER_UTIL_H_
#define _RENDER_UTIL_H_

#define NEAR_CLIP_DIST 0.1

extern const colour_t white;
extern const colour_t grey;
extern const colour_t red;
extern const colour_t green;
extern const colour_t light_blue;
extern const colour_t blue;
extern const colour_t black;
extern const colour_t sky;

void print_string( void *font, char *string );
void print_string_centered( scalar_t y, void *font, char *string );
void reshape( int w, int h );
void flat_mode();
void draw_overlay();
void clear_rendering_context();
void configure_fog();
void set_material( colour_t diffuse_colour, colour_t specular_colour,
		   scalar_t specular_exp );
void print_health(scalar_t health_pct);
  /**
void draw_billboard(  
		     point_t center_pt, scalar_t width, scalar_t height, 
		     bool_t use_world_y_axis, 
		     point2d_t min_tex_coord, point2d_t max_tex_coord );
  */
#endif /* _RENDER_UTIL_H_ */

#ifdef __cplusplus
} /* extern "C" */
#endif
