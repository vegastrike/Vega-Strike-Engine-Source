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
 * This Code adapted from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */
#include "gl_util.h"
void init_glfloat_array( int num, GLfloat arr[], ... )
{
    int i;
    va_list args;

    va_start( args, arr );

    for (i=0; i<num; i++) {
	arr[i] = va_arg(args, double);
    }

    va_end( args );
}

void set_gl_options( RenderMode mode ) 
{
    /* Must set the following options:
         Enable/Disable:
	   GL_TEXTURE_2D
	   GL_DEPTH_TEST
	   GL_CULL_FACE
	   GL_LIGHTING
	   GL_NORMALIZE
	   GL_ALPHA_TEST
	   GL_BLEND
	   GL_STENCIL_TEST
	   GL_TEXTURE_GEN_S
	   GL_TEXTURE_GEN_T
	   GL_COLOR_MATERIAL
           
	 Other Functions:
	   glDepthMask
	   glShadeModel
	   glDepthFunc
    */

    /*
     * Modify defaults based on rendering mode
     * 
     * This could could be improved if it stored state and avoided
     * redundant state changes, which are costly (or so I've heard)...  
     */
    switch( mode ) {
    case GUI:
        glEnable( GL_TEXTURE_2D );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
        glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );
	glDisable( GL_FOG );
        break;
    case GAUGE_BARS:
        glEnable( GL_TEXTURE_2D );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
        glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glEnable( GL_TEXTURE_GEN_S );
	glEnable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );

	glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
	glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
        break;

    case TEXFONT:
        glEnable( GL_TEXTURE_2D );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
        glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );
        break;

    case TEXT:
        glDisable( GL_TEXTURE_2D );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
        glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );
        break;

    case SPLASH_SCREEN:
        glDisable( GL_TEXTURE_2D );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
        glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );
        break;

    case COURSE:
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glEnable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glEnable( GL_TEXTURE_GEN_S );
	glEnable( GL_TEXTURE_GEN_T );
	glEnable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LEQUAL );

	glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
	glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
	break;

    case TREES:
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
        glEnable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );

        glAlphaFunc( GL_GEQUAL, 0.5 );
        break;
        
    case PARTICLES:
        glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glEnable( GL_ALPHA_TEST );
        glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );

        glAlphaFunc( GL_GEQUAL, 0.5 );
        break;
        
    case PARTICLE_SHADOWS:
        glDisable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
        glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );

        break;

    case BACKGROUND:
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE ); 
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );
        break;

    case SKY:
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE ); 
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_FALSE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );
	break;
 	
    case FOG_PLANE:
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE ); 
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );
	break;

    case TUX:
        glDisable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
        glEnable( GL_LIGHTING );
	glEnable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );

        break;

    case TUX_SHADOW:
#ifdef USE_STENCIL_BUFFER
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glEnable( GL_STENCIL_TEST );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_FALSE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );

	glStencilFunc( GL_EQUAL, 0, ~0 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );
#else
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );
#endif
	break;

    case TRACK_MARKS:
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_COLOR_MATERIAL );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDepthMask( GL_FALSE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LEQUAL );
	break;

    case OVERLAYS:
        glEnable( GL_TEXTURE_2D );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glEnable( GL_ALPHA_TEST );
        glEnable( GL_BLEND );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_COLOR_MATERIAL );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LESS );

        glAlphaFunc( GL_GEQUAL, 0.5 );
        break;

    default:
	code_not_reached();
    } 
} 

void check_gl_error()
{
    GLenum error;
    error = glGetError();
    if ( error != GL_NO_ERROR ) {
	print_warning( CRITICAL_WARNING, 
		       "OpenGL Error: %s", gluErrorString( error ) );
	fflush( stderr );
    }
}

void copy_to_glfloat_array( GLfloat dest[], scalar_t src[], int n )
{
    int i;
    for (i=0; i<n; i++) {
	dest[i] = src[i];
    }
}
