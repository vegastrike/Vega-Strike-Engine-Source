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

#ifndef _FONTS_H_
#define _FONTS_H_

typedef struct font_ font_t;

void init_fonts();

bool_t load_font( char *fontname, char *filename, char *texname );

bool_t bind_font( char *binding, char *fontname, scalar_t size, 
		  colour_t colour );
bool_t get_font_binding( char *binding, font_t **font );
bool_t unbind_font( char *binding );

bool_t flush_fonts(void);

void bind_font_texture( font_t *font );
void draw_character( font_t *font, char c);
void draw_string( font_t *font, char *string);
void draw_string_with_markup( font_t *font, char *string);
void get_font_metrics( font_t *font, char *string,
		       int *width, int *max_ascent, int *max_descent);

  //void register_font_callbacks(Tcl_Interp *ip);

#endif /* _FONTS_H_ */

#ifdef __cplusplus
} /* extern "C" */
#endif
