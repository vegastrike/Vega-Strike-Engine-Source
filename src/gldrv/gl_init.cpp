/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
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
 */

#include "vs_globals.h"
//#include "gl_globals.h"

#ifndef WIN32
#   include <GL/glx.h>
#include <stdlib.h>
#include "gfxlib.h"
#else
#include <windows.h>
#endif
#include <GL/gl.h>
#   include <GL/glext.h>
#include <stdio.h>
#include "gl_init.h"
#define WINDOW_TITLE "Vega Strike "VERSION
static int glutWindow;




#ifdef WIN32
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB=0;
PFNGLCLIENTACTIVETEXTUREARBPROC glActiveTextureARB=0;
PFNGLCOLORTABLEEXTPROC glColorTable=0;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB = 0;
//PFNGLSELECTTEXTURESGISPROC glSelectTextureSGIS ;
//PFNGLMULTITEXCOORD2FSGISPROC glMultiTexCoord2fSGIS ;
//PFNGLMTEXCOORDPOINTERSGISPROC glMTexCoordPointerSGIS ;
#include "gfxlib.h"
#else
PFNGLLOCKARRAYSEXTPROC glLockArraysEXT_p;
PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT_p;

#endif
typedef void (*(*get_gl_proc_fptr_t)(const GLubyte *))(); 
#ifdef WIN32
    typedef char * GET_GL_PTR_TYP;
#define GET_GL_PROC wglGetProcAddress

#else
    typedef GLubyte * GET_GL_PTR_TYP;
#define GET_GL_PROC glXGetProcAddressARB

#endif
#include <GL/glut.h>
void init_opengl_extensions()
{
	const unsigned char * extensions = glGetString(GL_EXTENSIONS);
	fprintf (stderr,"OpenGL Extensions supported: %s\n",extensions);

    if ( glutExtensionSupported( "GL_EXT_compiled_vertex_array" ) ) {

	printf( "GL_EXT_compiled_vertex_array extension "
		     "supported\n" );

	glLockArraysEXT_p = (PFNGLLOCKARRAYSEXTPROC) 
	    GET_GL_PROC( (GET_GL_PTR_TYP) "glLockArraysEXT" );
	glUnlockArraysEXT_p = (PFNGLUNLOCKARRAYSEXTPROC) 
	    GET_GL_PROC( (GET_GL_PTR_TYP) "glUnlockArraysEXT" );

    } else {
	printf(  "GL_EXT_compiled_vertex_array extension "
		     "NOT supported\n" );
	glLockArraysEXT_p = NULL;
	glUnlockArraysEXT_p = NULL;

    }
    g_game.mipmap = 2;
#ifdef WIN32
    glColorTable = (PFNGLCOLORTABLEEXTPROC ) GET_GL_PROC((GET_GL_PTR_TYP)"glColorTableEXT");
    glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glMultiTexCoord2fARB");
    glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glClientActiveTextureARB");
    glActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glActiveTextureARB");
#endif
    if (glutExtensionSupported ("GL_ARB_multitexture")) {
      g_game.Multitexture = 1;
      printf ("OpenGL::Multitexture supported\n");
    } else {
      g_game.Multitexture =0;
      printf ("OpenGL::Multitexture unsupported\n");
    }
    if ( glutExtensionSupported( "GL_ARB_texture_cube_map" ) ) {
      printf ("OpenGL::Texture Cube Map Ext Supported\n");
      g_game.cubemap =1;
    }
}

 static void initfov () {

    g_game.fov = 78;

    g_game.aspect = 1.33F;

    g_game.znear = 1.00F;

    g_game.zfar = 100000.00F;
    g_game.display_lists=1;
    FILE * fp = fopen ("glsetup.txt","r");
    if (fp) {
      fscanf (fp,"fov %f\n",&g_game.fov);
      fscanf (fp,"aspect %f\n",&g_game.aspect);
      fscanf (fp,"znear %f\n",&g_game.znear);
      fscanf (fp,"zfar %f\n",&g_game.zfar);
      fclose (fp);
    }
 }
static void Reshape (int x, int y) {
  g_game.x_resolution = x;
  g_game.y_resolution = y;

  
}
extern void GFXInitTextureManager();
void GFXInit (int argc, char ** argv){
    glutInit( &argc, argv );
     
#ifdef USE_STENCIL_BUFFER
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL );
#else
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );
#endif

      char str [15];
      sprintf (str, "%dx%d:%d",g_game.x_resolution,g_game.y_resolution,g_game.color_depth); 
      glutGameModeString(str);

    /* Create a window */
      if (g_game.fullscreen &&glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
	glutInitWindowPosition( 0, 0 );
	glutEnterGameMode();
    } else {
	/* Set the initial window size */
	glutInitWindowSize(g_game.x_resolution, g_game.y_resolution );
	glutInitWindowPosition( 0, 0 );
	glutWindow = glutCreateWindow( "Vegastrike " );
	if ( glutWindow == 0 ) {
	    fprintf( stderr, "Couldn't create a window.\n" );
	    exit(1);
	} 
    }
    /* Ingore key-repeat messages */
    glutIgnoreKeyRepeat(1);
    glViewport (0, 0, g_game.x_resolution,g_game.y_resolution);
    glClearColor ((float)0.0, (float)0.0, (float)0.0, (float)0);
    glutReshapeFunc (Reshape);
    initfov();
    glShadeModel (GL_SMOOTH);
    glEnable (GL_CULL_FACE);
    glCullFace (GL_BACK);
    glShadeModel (GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
    
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0);

    init_opengl_extensions();
    GFXInitTextureManager();
    
    if (g_game.Multitexture)
      GFXActiveTexture(0);
    glEnable(GL_TEXTURE_2D);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
    //int retval= glGetError();
    if ( !glutExtensionSupported( "GL_EXT_color_table" ) ) {
      g_game.PaletteExt = 0;
      printf ("Color Table Not Supported\n");
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    if (g_game.Multitexture){
      GFXActiveTexture(1);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#ifdef NV_CUBE_MAP

#else
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif

      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
      glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
      glPixelStorei(GL_PACK_ROW_LENGTH, 256);

      // Spherical texture coordinate generation
#ifdef NV_CUBE_MAP
      glEnable(GL_TEXTURE_CUBE_MAP_EXT);
      glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP_NV);
      glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP_NV);
      glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP_NV);
      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);
      glEnable(GL_TEXTURE_GEN_R);

#else
      glEnable(GL_TEXTURE_2D);
      glTexGenf(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
      glTexGenf(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);
#endif
    }
    glClearDepth(1);
    glEnable (GL_BLEND);
    glDisable (GL_ALPHA_TEST);
    GFXBlendMode (ONE, ZERO);
    
    glColor3f(0,0,0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode (GL_TEXTURE);
    glLoadIdentity(); //set all matricies to identity
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    glEnable(GL_LIGHTING);
    
    glDisable(GL_NORMALIZE);
    int con;
    GFXCreateLightContext (con);


    glutSetCursor(GLUT_CURSOR_NONE );

}

void GFXLoop(void main_loop()) {
  glutDisplayFunc(main_loop);
  glutIdleFunc (main_loop);
  glutMainLoop();
  //never make it here;

}
extern void GFXDestroyAllLights();

void GFXShutdown () {
  GFXDestroyAllTextures();
  GFXDestroyAllLights();
  if ( g_game.fullscreen ) {
    glutLeaveGameMode();
  }
}

