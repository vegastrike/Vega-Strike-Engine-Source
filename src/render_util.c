/* 
 * Vega Strike 
 * Copyright (C) 1999-2000 Daniel Horn
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

#include "tuxracer.h"
#include "gl_util.h"
#include "render_util.h"
#include "textures.h"
#include "ui_theme.h"

/*
 * Constants 
 */

/* Distance by which to push back far clip plane, to ensure that the
   fogging plane is drawn (m) */
#define FAR_CLIP_FUDGE_AMOUNT 5

static const colour_t text_colour = { 0.0, 0.0, 0.0, 1.0 };

const colour_t white = { 1.0, 1.0, 1.0, 1.0 };
const colour_t grey  = { 0.7, 0.7, 0.7, 1.0 };
const colour_t red   = { 1.0, 0. , 0., 1.0  };
const colour_t green = { 0. , 1.0, 0., 1.0  };
const colour_t blue  = { 0. , 0. , 1.0, 1.0 };
const colour_t light_blue = { 0.5, 0.5, 0.8, 1.0 };
const colour_t black = { 0., 0., 0., 1.0 };
const colour_t sky   = { 0.82, 0.86, 0.88, 1.0 };

/* XXX: this will eventually replaced with nicer texture-mapped chars */
/* This routine taken from Mesa Demos */
void print_string( void *font, char *string )
{
    int len, i;

    len = (int) strlen(string);
    for (i=0; i<len; i++) 
        glutBitmapCharacter( font, string[i] );
} 

void print_string_centered( scalar_t y, void *font, char *string )
{
    scalar_t width;

    width = glutBitmapLength( font, (unsigned char*) string );
    width = width / getparam_x_resolution() * 640.;
    glRasterPos2i( (int)(640. / 2. - width / 2.), (int) y );
    print_string( font, string );
}

void reshape( int w, int h )
{
    scalar_t far_clip_dist;

    setparam_x_resolution( w );
    setparam_y_resolution( h );
    glViewport( 0, 0, (GLint) w, (GLint) h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    far_clip_dist = getparam_forward_clip_distance() + FAR_CLIP_FUDGE_AMOUNT;

    gluPerspective( getparam_fov(), (scalar_t)w/h, NEAR_CLIP_DIST, 
		    far_clip_dist );

    glMatrixMode( GL_MODELVIEW );
} 

void flat_mode()
{
    set_gl_options( TEXT );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( -0.5, 639.5, -0.5, 479.5, -1.0, 1.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}

void draw_overlay() {
    glColor4f( 0.0, 0.0, 1.0, 0.1 );
    glRecti( 0, 0, 640, 480 );
} 

void print_health(scalar_t health_pct)
{
    char buff[30];

    flat_mode();
    sprintf( buff, "Health: %3d%%", ROUND_TO_NEAREST(health_pct) );

    glColor3f( text_colour.r, text_colour.g, text_colour.b );
    glRasterPos2i( 5, 20 ); 
    print_string( GLUT_BITMAP_HELVETICA_10, buff );
}

void clear_rendering_context()
{
    glDepthMask( GL_TRUE );
    glClearColor( ui_background_colour.r,
		  ui_background_colour.g,
		  ui_background_colour.b,
		  ui_background_colour.a );
    glClearStencil( 0 );
    glClear( GL_COLOR_BUFFER_BIT 
	     | GL_DEPTH_BUFFER_BIT 
	     | GL_STENCIL_BUFFER_BIT );
}

/* 
 * Sets the material properties
 */
void set_material( colour_t diffuse_colour, colour_t specular_colour,
			 scalar_t specular_exp )
{
  GLfloat mat_amb_diff[4];
  GLfloat mat_specular[4];

  /* Set material colour (used when lighting is on) */
  mat_amb_diff[0] = diffuse_colour.r;
  mat_amb_diff[1] = diffuse_colour.g;
  mat_amb_diff[2] = diffuse_colour.b;
  mat_amb_diff[3] = diffuse_colour.a; 
  glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff );

  mat_specular[0] = specular_colour.r;
  mat_specular[1] = specular_colour.g;
  mat_specular[2] = specular_colour.b;
  mat_specular[3] = specular_colour.a;
  glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular );

  glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, specular_exp );

  /* Set standard colour */
  glColor4f( diffuse_colour.r, diffuse_colour.g, diffuse_colour.b, 
	     diffuse_colour.a);
} 

/**
void draw_billboard( 
		     point_t center_pt, scalar_t width, scalar_t height, 
		     bool_t use_world_y_axis, 
		     point2d_t min_tex_coord, point2d_t max_tex_coord )
{
    point_t pt;
    vector_t x_vec;
    vector_t y_vec;
    vector_t z_vec;

    x_vec.x = plyr->view.inv_view_mat[0][0];
    x_vec.y = plyr->view.inv_view_mat[0][1];
    x_vec.z = plyr->view.inv_view_mat[0][2];

    if ( use_world_y_axis ) {
	y_vec = make_vector( 0, 1, 0 );
	x_vec = project_into_plane( y_vec, x_vec );
	normalize_vector( &x_vec );
	z_vec = cross_product( x_vec, y_vec );
    } else {
	y_vec.x = plyr->view.inv_view_mat[1][0];
	y_vec.y = plyr->view.inv_view_mat[1][1];
	y_vec.z = plyr->view.inv_view_mat[1][2];
	z_vec.x = plyr->view.inv_view_mat[2][0];
	z_vec.y = plyr->view.inv_view_mat[2][1];
	z_vec.z = plyr->view.inv_view_mat[2][2];
    }

    glBegin( GL_QUADS );
    {
	pt = move_point( center_pt, scale_vector( -width/2.0, x_vec ) );
	pt = move_point( pt, scale_vector( -height/2.0, y_vec ) );
	glNormal3f( z_vec.x, z_vec.y, z_vec.z );
	glTexCoord2f( min_tex_coord.x, min_tex_coord.y );
	glVertex3f( pt.x, pt.y, pt.z );

	pt = move_point( pt, scale_vector( width, x_vec ) );
	glTexCoord2f( max_tex_coord.x, min_tex_coord.y );
	glVertex3f( pt.x, pt.y, pt.z );

	pt = move_point( pt, scale_vector( height, y_vec ) );
	glTexCoord2f( max_tex_coord.x, max_tex_coord.y );
	glVertex3f( pt.x, pt.y, pt.z );

	pt = move_point( pt, scale_vector( -width, x_vec ) );
	glTexCoord2f( min_tex_coord.x, max_tex_coord.y );
	glVertex3f( pt.x, pt.y, pt.z );
    }
    glEnd();
}
*/
