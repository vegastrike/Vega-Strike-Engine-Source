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


#include "vegastrike.h"
#include "fonts.h"
#include "image.h"
#include "hash.h"
#include "list.h"
#include "tex_font_metrics.h"
#include "textures.h"

#include "render_util.h"

typedef struct {
    tex_font_metrics_t *tfm;
    texture_node_t *tex;
    int ref_count;
} font_node_t;

struct font_ {
    font_node_t *node;
    scalar_t size;
    colour_t colour;
};

static bool_t initialized = False;
static hash_table_t font_table;
static hash_table_t binding_table;

void init_fonts() 
{
    if (!initialized) {
	font_table = create_hash_table();
	binding_table = create_hash_table();
    }
    initialized = True;
} 

static bool_t get_font( char *fontname, font_node_t **fontnode )
{
    return get_hash_entry(font_table, fontname, (hash_entry_t*)fontnode);		
}

bool_t get_font_binding( char *binding, font_t **font )
{
    if (get_hash_entry(binding_table, binding, (hash_entry_t*)(font))) {
	return True;
    }
    return False;  
}

bool_t load_font( char *fontname, char *filename, char *texname )
{
    font_node_t *fontnode;
    tex_font_metrics_t *tfm;
    texture_node_t *tex;

    print_debug(DEBUG_FONT, "Loading font %s from file: %s", 
		fontname, filename);
    if ( initialized == False ) {
        check_assertion( 0, "font module not initialized" );
    } 

    check_assertion( tex != NULL, "out of memory" );

    if ( ! get_texture( texname, &tex ) ) {
	print_warning( IMPORTANT_WARNING, 
		       "Texture `%s' does not exist", texname );
	return False;
    }

    tfm = load_tex_font_metrics( filename );

    if ( tfm == NULL ) {
    	print_warning( IMPORTANT_WARNING, 
		       "couldn't load font file %s", filename );
    	return False;
    }


    if (get_hash_entry( font_table, fontname, (hash_entry_t*)&fontnode )) { 
	print_debug( DEBUG_FONT, "Font %s already exists, deleting...", 
		     fontname );
	delete_tex_font_metrics( fontnode->tfm );
	fontnode->tex->ref_count -= 1;
    } else {
	fontnode = (font_node_t*)malloc(sizeof(font_node_t));
	fontnode->ref_count = 0;
	add_hash_entry( font_table, fontname, (hash_entry_t)fontnode ); 
    }

    fontnode->tfm = tfm;
    fontnode->tex = tex;
    tex->ref_count += 1;

    return True;
} 

static bool_t del_font( char *fontname )
{
    font_node_t *fontnode;
    if ( del_hash_entry(font_table, fontname, (hash_entry_t*)(&fontnode)) ) {
	check_assertion( fontnode->ref_count == 0,
			 "Trying to delete font with non-zero reference "
			 "count" );
	delete_tex_font_metrics( fontnode->tfm );
	fontnode->tex->ref_count -= 1;
	free(fontnode);
	return True;
    }

    return False;
}

bool_t bind_font( char *binding, char *fontname, scalar_t size, 
		  colour_t colour )
{
    font_node_t *fontnode;
    font_t *font;

    print_debug(DEBUG_FONT, "Binding %s to font name: %s", 
		binding, fontname);

    if (!get_font( fontname, &fontnode)) {
	check_assertion(0, "Attempt to bind to unloaded font");
	return False;
    }

    if (get_hash_entry(binding_table, binding, (hash_entry_t*)(&font))) {
	font->node->ref_count--;
    } else {
	font = (font_t*)malloc(sizeof(font_t));
	add_hash_entry(binding_table, binding, (hash_entry_t)font);
    }

    font->node = fontnode;
    font->size = size;
    font->colour = colour;

    font->node->ref_count += 1;

    return True;
}

bool_t unbind_font( char *binding )
{
    font_t *font;

    if (get_hash_entry( binding_table, binding, (hash_entry_t*)(&font))) {
	font->node->ref_count -= 1;
	if (!del_hash_entry( binding_table, binding, NULL )) {
	    check_assertion(0, "Cannot delete known font binding");
	    return False;
	}
	free(font);
	return True;
    }

    return False;
}

bool_t flush_fonts(void)
{
    font_node_t *fontnode;
    hash_search_t  sptr;
    list_t delete_list;
    list_elem_t elem;
    char *key;
    bool_t result;

    delete_list = create_list();
  
    begin_hash_scan(font_table, &sptr);
    while ( next_hash_entry(sptr, &key, (hash_entry_t*)(&fontnode)) ) {
	if (fontnode->ref_count == 0) {
	    elem = insert_list_elem(delete_list, NULL, (list_elem_data_t)key);
	}
    }
    end_hash_scan(sptr);

    elem = get_list_head(delete_list);
    while (elem != NULL) {
	key = (char*)get_list_elem_data(elem);

	result = del_font( key );
	check_assertion(result, "Attempt to flush non-existant font");

	elem = get_next_list_elem(delete_list, elem);
    }

    del_list(delete_list);

    return True;

}

static scalar_t get_scale_factor( font_t *font ) 
{
    char *empty_string = "";
    int dummy1, dummy2, max_ascent;

    get_tex_font_string_bbox( font->node->tfm, empty_string, &dummy1, 
			      &max_ascent, &dummy2 );

    return font->size / max_ascent;
}

static void start_font_draw( font_t *font )
{
    scalar_t scale_fact = get_scale_factor( font );
    glPushMatrix();
    glScalef( scale_fact,
	      scale_fact,
	      scale_fact );

    glColor4dv( (scalar_t*) &font->colour );
}

static void end_font_draw( font_t *font )
{
    glPopMatrix();
}

void bind_font_texture( font_t *font )
{
    glBindTexture( GL_TEXTURE_2D, font->node->tex->texture_id );
}

static void advance( font_t *font, char *string )
{
    int w, a, d;
    get_font_metrics( font, string, &w, &a, &d );
    glTranslatef( w, 0, 0 );
}

void draw_character( font_t *font, char c)
{
    char buff[2];

    start_font_draw( font );
    draw_tex_font_char( font->node->tfm, c );
    end_font_draw( font );

    buff[0] = c;
    buff[1] = '\0';

    advance( font, buff );
}

void draw_string( font_t *font, char *string )
{
    start_font_draw( font );
    draw_tex_font_string( font->node->tfm, string );
    end_font_draw( font );

    advance( font, string );
}

void get_font_metrics( font_t *font, char *string,
		       int *width, int *max_ascent, int *max_descent)
{
    scalar_t scale_fact = get_scale_factor( font );
    get_tex_font_string_bbox( font->node->tfm, string, width, 
			      max_ascent, max_descent );

    *width *= scale_fact;
    *max_ascent *= scale_fact;
    *max_descent *= scale_fact;
}



