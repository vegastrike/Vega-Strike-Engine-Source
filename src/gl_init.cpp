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

#include "vegastrike.h"
#include "gl_globals.h"
#if defined( HAVE_GL_GLX_H )
#   include <GL/glx.h>
#   include <GL/glext.h>
#endif /* defined( HAVE_GL_GLX_H ) */

#include "gl_init.h"
#include "gfxlib.h"
#define WINDOW_TITLE "Vega Strike " VERSION
static int glutWindow;



PFNGLLOCKARRAYSEXTPROC glLockArraysEXT_p;
PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT_p;
//PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB=0;
//PFNGLACTIVETEXTUREARBPROC glActiveTextureARB=0;

//PFNGLSELECTTEXTURESGISPROC glSelectTextureSGIS ;
//PFNGLMULTITEXCOORD2FSGISPROC glMultiTexCoord2fSGIS ;
//PFNGLMTEXCOORDPOINTERSGISPROC glMTexCoordPointerSGIS ;
//PFNGLCOLORTABLEEXTPROC glColorTable;

typedef void (*(*get_gl_proc_fptr_t)(const GLubyte *))(); 

void init_opengl_extensions()
{
    get_gl_proc_fptr_t get_gl_proc;
#ifdef WIN32
    get_gl_proc = (get_gl_proc_fptr_t) wglGetProcAddress;
#else
    get_gl_proc = (get_gl_proc_fptr_t) glXGetProcAddressARB;
#endif

    if ( glutExtensionSupported( "GL_EXT_compiled_vertex_array" ) ) {
	print_debug( DEBUG_GL_EXT, "GL_EXT_compiled_vertex_array extension "
		     "supported" );
	glLockArraysEXT_p = (PFNGLLOCKARRAYSEXTPROC) 
	    get_gl_proc( (GLubyte*) "glLockArraysEXT" );
	glUnlockArraysEXT_p = (PFNGLUNLOCKARRAYSEXTPROC) 
	    get_gl_proc( (GLubyte*) "glUnlockArraysEXT" );
    } else {
	print_debug( DEBUG_GL_EXT, "GL_EXT_compiled_vertex_array extension "
		     "NOT supported" );
	glLockArraysEXT_p = NULL;
	glUnlockArraysEXT_p = NULL;
    }
    if (glutExtensionSupported ("GL_SGIS_multitexture")) {
      g_game.Multitexture =1;
      //glMTexCoordPointerSGIS=(PFNGLMTEXCOORDPOINTERSGISPROC)glSelectTextureSGIS=get_gl_proc ((GLubyte*) "glMTexCoordPointerSGIS");
      //glSelectTextureSGIS = (PFNGLSELECTTEXTURESGISPROC) get_gl_proc ((GLubyte*) "glSelectTextureSGIS")
      //glMultiTexCoord2fSGIS =  (PFNGLMULTITEXCOORD2FSGISPROC)get_gl_proc ((GLubyte*) "glMTexCoord2fSGIS")
    } else {
      g_game.Multitexture = 0;
    }

    //glColorTable = (PFNGLCOLORTABLEEXTPROC ) get_gl_proc((GLubyte*)"glColorTableEXT");
    //glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) get_gl_proc((GLubyte*)"glMultiTexCoord2fARB");
    //glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) get_gl_proc((GLubyte*)"glClientActiveTextureARB");
    //glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) get_gl_proc((GLubyte*)"glActiveTextureARB");
    if (glMultiTexCoord2fARB!=0) {
      g_game.Multitexture = 1;
    } else {
      g_game.Multitexture =0;
    }
    g_game.Multitexture=1;
}
BOOL GFXInit (int argc, char ** argv){
    glutInit( &argc, argv );
    
#ifdef USE_STENCIL_BUFFER
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE/* | GLUT_STENCIL*/ );
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
	glutWindow = glutCreateWindow( WINDOW_TITLE );
	if ( glutWindow == 0 ) {
	    fprintf( stderr, "Couldn't create a window.\n" );
	    exit(1);
	} 
    }
    /* Ingore key-repeat messages */
    glutIgnoreKeyRepeat(1);
    glViewport (0, 0, g_game.x_resolution,g_game.y_resolution);
    glClearColor ((float)1.0, (float)0, (float)0, (float)0);
    glShadeModel (GL_SMOOTH);
    glEnable (GL_CULL_FACE);
	//glDisable (GL_CULL_FACE);
    glCullFace (GL_BACK);
	//glCullFace (GL_FRONT);
    glShadeModel (GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
	//glDepthFunc (GL_LESS);

    //glEnable(TEXTURE0_SGIS);
    //glEnable(TEXTURE1_SGIS);

    
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0);


    init_opengl_extensions();

    
    if (g_game.Multitexture)
      glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
    int retval= glGetError();
    if (retval == 0x0500) {
      g_game.PaletteExt = 0;
      printf ("Palette Not Supported");
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    if (g_game.Multitexture){
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glEnable(GL_TEXTURE_2D);
      //glDisable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
      glPixelStorei(GL_PACK_ROW_LENGTH, 256);
    }
    glClearDepth(1);
    glEnable (GL_BLEND);
    glDisable (GL_ALPHA_TEST);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(0,0,0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode (GL_TEXTURE);
    glLoadIdentity(); //set all matricies to identity
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
//	gluPerspective (78,1.33,0.5,20); //set perspective to 78 degree FOV
//	glPushMatrix();

    glEnable(GL_LIGHTING);
	//glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE);
    for(int count = 0; count<MAX_NUM_LIGHTS; count++) {
      GFXDisableLight(count); //turn all lights off
    }

	
    GFXEnableLight(0);
    {
      float params[4] = {0.2F, 0.2F, 0.2F, 1.0F};
      glLightfv(GL_LIGHT0, GL_AMBIENT, params);
    }
    {
      float params[4] = {1.0F, 1.0F, 1.0F, 1.0F};
      glLightfv(GL_LIGHT0, GL_DIFFUSE, params);
    }
    {
      float params[4] = {1.0F, 1.0F, 1.0F, 1.0F};
      glLightfv(GL_LIGHT0, GL_SPECULAR, params);
    }
    //FIXME VEGASTRIKE //GFXLoadIdentity(MODEL);
    //FIXME VEGASTRIKE //GFXLoadIdentity(VIEW);
    //FIXME VEGASTRIKE //GFXLoadIdentity(PROJECTION);



    glutSetCursor(GLUT_CURSOR_INHERIT );
    //GFXPerspective(78,1.33,0.5,20);
    return TRUE;
}

BOOL GFXLoop(void main_loop()) {
  glutDisplayFunc(main_loop);
  glutIdleFunc (main_loop);
  glutMainLoop();
  //never make it here;
  return TRUE;
}


BOOL GFXShutdown () {
  if ( g_game.fullscreen ) {
    glutLeaveGameMode();
  }
  return TRUE;
}
