/* 
 * Vega Strike 
 * Copyright (C) 2001-2002 Daniel Horn
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
#include "textures.h"
#include "image.h"
#include "hash.h"
#include "list.h"

static bool_t initialized = False;
static hash_table_t texture_table;
static hash_table_t binding_table;


bool_t get_texture_binding( char *binding, GLuint *texid )
{
    texture_node_t *texnode;
    if (get_hash_entry(binding_table, binding, (hash_entry_t*)(&texnode))) {
	*texid = texnode->texture_id;
	return True;
    }
    return False;  
}

bool_t load_and_bind_texture( char *binding, char *filename )
{
    return load_texture( binding, filename, 1 ) &&
	bind_texture( binding, binding );
}

void init_textures() 
{
    if (!initialized) {
	texture_table = create_hash_table();
	binding_table = create_hash_table();
    }
    initialized = True;
} 

int get_min_filter()
{
    switch( getparam_mipmap_type() ) {
    case 0: 
	return GL_NEAREST;
    case 1:
	return GL_LINEAR;
    case 2: 
	return GL_NEAREST_MIPMAP_NEAREST;
    case 3: 
	return GL_LINEAR_MIPMAP_NEAREST;
    case 4: 
	return GL_NEAREST_MIPMAP_LINEAR;
    case 5: 
	return GL_LINEAR_MIPMAP_LINEAR;
    default:
	return GL_LINEAR_MIPMAP_NEAREST;
    }
}

bool_t load_texture( char *texname, char *filename, int repeatable )
{
    IMAGE *texImage;
    texture_node_t *tex;
    int max_texture_size;


    print_debug(DEBUG_TEXTURE, "Loading texture %s from file: %s", 
		texname, filename);
    if ( initialized == False ) {
        check_assertion( 0, "texture module not initialized" );
    } 

    texImage = ImageLoad( filename );

    if ( texImage == NULL ) {
    	print_warning( IMPORTANT_WARNING, 
		       "couldn't load image %s", filename );
    	return False;
    }

    if (get_hash_entry( texture_table, texname, (hash_entry_t*)&tex )) { 
	print_debug(DEBUG_TEXTURE, "Found texture %s with id: %d", 
		    texname, tex->texture_id);
        glDeleteTextures( 1, &(tex->texture_id) );
    } else {
        tex = (texture_node_t*)malloc(sizeof(texture_node_t));

	check_assertion( tex != NULL, "out of memory" );

	tex->ref_count = 0;
	add_hash_entry( texture_table, texname, (hash_entry_t)tex ); 
    }
 
    tex->repeatable = repeatable;
    glGenTextures( 1, &(tex->texture_id) );
    glBindTexture( GL_TEXTURE_2D, tex->texture_id );

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);


    if ( repeatable ) {
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    } else {
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    }
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                     get_min_filter() );

    /* Check if we need to scale image */
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &max_texture_size );
    if ( texImage->sizeX > max_texture_size ||
	 texImage->sizeY > max_texture_size ) 
    {
	char *newdata = (char*)malloc( texImage->sizeZ *
				       max_texture_size *
				       max_texture_size );

	check_assertion( newdata != NULL, "out of memory" );

	print_debug( DEBUG_TEXTURE, "Texture `%s' too large -- scaling to "
		     "maximum allowed size",
		     filename );

	/* In the case of large- or small-aspect ratio textures, this
           could end up using *more* space... oh well. */
	gluScaleImage( texImage->sizeZ == 3 ? GL_RGB : GL_RGBA,
		       texImage->sizeX, texImage->sizeY, 
		       GL_UNSIGNED_BYTE,
		       texImage->data,
		       max_texture_size, max_texture_size, 
		       GL_UNSIGNED_BYTE,
		       newdata );

	free( texImage->data );
	texImage->data = (unsigned char*) newdata;
	texImage->sizeX = max_texture_size;
	texImage->sizeY = max_texture_size;
    }

    gluBuild2DMipmaps( GL_TEXTURE_2D, texImage->sizeZ, texImage->sizeX,
		       texImage->sizeY, texImage->sizeZ == 3 ? GL_RGB : GL_RGBA, 
		       GL_UNSIGNED_BYTE, texImage->data );

    free( texImage->data );
    free( texImage );

    return True;
} 


bool_t 
get_texture( char *texname, texture_node_t **tex )
{
    return get_hash_entry(texture_table, texname, (hash_entry_t*)tex);		
}


bool_t 
del_texture( char *texname )
{
    texture_node_t *tex;

    print_debug( DEBUG_TEXTURE, "Deleting texture %s", texname );

    if (del_hash_entry(texture_table, texname, (hash_entry_t*)(&tex))) {
	check_assertion( tex->ref_count == 0,
			 "Trying to delete texture with non-zero reference "
			 "count" );
	glDeleteTextures( 1, &(tex->texture_id) );
	free(tex);
	return True;
    }

    return False;
}


bool_t bind_texture( char *binding, char *texname )
{
    texture_node_t *tex, *oldtex;

    print_debug(DEBUG_TEXTURE, "Binding %s to texture name: %s", 
		binding, texname);
    if (!get_texture( texname, &tex)) {
	check_assertion(0, "Attempt to bind to unloaded texture");
	return False;
    }

    if (get_hash_entry(binding_table, binding, (hash_entry_t*)(&oldtex))) {
	oldtex->ref_count--;
	if (!del_hash_entry(binding_table, binding, NULL)) {
	    check_assertion(0, "Cannot delete known texture");
	    return False;
	}
    }

    add_hash_entry(binding_table, binding, (hash_entry_t)tex);
    tex->ref_count++;

    return True;
}

bool_t unbind_texture( char *binding )
{
    texture_node_t *tex;

    if (get_hash_entry( binding_table, binding, (hash_entry_t*)(&tex))) {
	tex->ref_count--;
	if (!del_hash_entry( binding_table, binding, NULL )) {
	    check_assertion(0, "Cannot delete known texture binding");
	    return False;
	}
	return True;
    }

    return False;
}

void get_current_texture_dimensions( int *width, int *height )
{
    glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, width );
    glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, height );
}

bool_t flush_textures(void)
{
    texture_node_t *tex;
    hash_search_t  sptr;
    list_t delete_list;
    list_elem_t elem;
    char *key;
    bool_t result;

    delete_list = create_list();
  
    begin_hash_scan(texture_table, &sptr);
    while ( next_hash_entry(sptr, &key, (hash_entry_t*)(&tex)) ) {
	if (tex->ref_count == 0) {
	    elem = insert_list_elem(delete_list, NULL, (list_elem_data_t)key);
	}
    }
    end_hash_scan(sptr);

    elem = get_list_head(delete_list);
    while (elem != NULL) {
	key = (char*)get_list_elem_data(elem);

	result = del_texture( key );
	check_assertion(result, "Attempt to flush non-existant texture");

	elem = get_next_list_elem(delete_list, elem);
    }

    del_list(delete_list);

    return True;

}

static int load_texture_cb ( ClientData cd, Tcl_Interp *ip, int argc, 
			     char *argv[]) 
{
    int repeatable = 1;

    if ( ( argc != 3 ) && (argc != 4) ) {
	Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], "<texture name> <image file>",
			 " [repeatable]", (char *)0 );
	return TCL_ERROR;
    } 

    if ( ( argc == 4 ) && ( Tcl_GetInt( ip, argv[3], &repeatable ) != TCL_OK ) ) {
        Tcl_AppendResult(ip, argv[0], ": invalid repeatable flag",
			 (char *)0 );
        return TCL_ERROR;
    } 
    
    if (!load_texture(argv[1], argv[2], repeatable)) {
	Tcl_AppendResult(ip, argv[0], ": Could not load texture ", 
			 argv[2], (char*)0);
	return TCL_ERROR;
    }

    return TCL_OK;
}

static int bind_texture_cb ( ClientData cd, Tcl_Interp *ip, int argc, 
			     char *argv[])
{
    if ( argc != 3 ) {
	Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], "<object name> <texture name>",
			 (char *)0 );
	return TCL_ERROR;
    } 

    if (!bind_texture(argv[1], argv[2])) {
	Tcl_AppendResult(ip, argv[0], ": Could not bind texture ", 
			 argv[2], (char*)0);
	return TCL_ERROR;
    }

    return TCL_OK;
}


void register_texture_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_load_texture",   load_texture_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_bind_texture",   bind_texture_cb,   0,0);
}


