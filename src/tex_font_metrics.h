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

#ifndef TEX_FONT_METRICS_H
#define TEX_FONT_METRICS_H

typedef struct tex_font_metrics_ tex_font_metrics_t; /* Opaque */

tex_font_metrics_t* load_tex_font_metrics( char *filename );
void delete_tex_font_metrics( tex_font_metrics_t *tfm );
void get_tex_font_string_bbox( tex_font_metrics_t *tfm, 
			       char *string, 
			       int *width, int *max_ascent, int *max_descent );
void draw_tex_font_char( tex_font_metrics_t *tfm, char c );
void draw_tex_font_string( tex_font_metrics_t *tfm, char *string );
bool_t is_character_in_tex_font( tex_font_metrics_t *tfm, char c );

#endif /* TEX_FONT_METRICS_H */

#ifdef __cplusplus
} /* extern "C" */
#endif

/* EOF */
