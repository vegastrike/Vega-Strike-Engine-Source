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
#include "gl_globals.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "config_xml.h"
//#include "gl_globals.h"

#ifndef WIN32
#   include <GL/glx.h>
#include <stdlib.h>
#include "gfxlib.h"
#else
#include <windows.h>
#endif
#include <GL/gl.h>
#ifdef GL_EXT_compiled_vertex_array
# ifndef PFNGLLOCKARRAYSEXTPROC
#  undef GL_EXT_compiled_vertex_array
# endif
#endif
#if !defined(IRIX)
//typedef void (APIENTRY * PFNGLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
//typedef void (APIENTRY * PFNGLUNLOCKARRAYSEXTPROC) (void);

# include <GL/glext.h>
#endif

#include <stdio.h>
#include "gl_init.h"
#define WINDOW_TITLE "Vega Strike "VERSION
static int glutWindow;
#ifdef _WIN32
static HWND hWnd;
static HINSTANCE hInst;								// current instance
LRESULT CALLBACK DLOG_start(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return TRUE;
	case WM_DESTROY:
		return FALSE;
	}
    return FALSE;
}
#include "../resource.h"
#endif

#ifdef WIN32
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB=0;
PFNGLCLIENTACTIVETEXTUREARBPROC glActiveTextureARB=0;
PFNGLCOLORTABLEEXTPROC glColorTable=0;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB = 0;
// the following two lines were not defined for WIN32... any particular reason?
PFNGLLOCKARRAYSEXTPROC glLockArraysEXT_p;
PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT_p;
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

	(void) fprintf(stderr, "OpenGL Extensions supported: %s\n", extensions);

    if (glutExtensionSupported( "GL_EXT_compiled_vertex_array")) {
	glLockArraysEXT_p = (PFNGLLOCKARRAYSEXTPROC) 
	    GET_GL_PROC( (GET_GL_PTR_TYP) "glLockArraysEXT" );
	glUnlockArraysEXT_p = (PFNGLUNLOCKARRAYSEXTPROC) 
	    GET_GL_PROC( (GET_GL_PTR_TYP) "glUnlockArraysEXT" );
	(void) fprintf(stderr, "OpenGL::GL_EXT_compiled_vertex_array supported\n");
    } else
	{
	glLockArraysEXT_p = NULL;
	glUnlockArraysEXT_p = NULL;
	(void) fprintf(stderr, "OpenGL::GL_EXT_compiled_vertex_array unsupported\n");
    }
#ifdef WIN32
    glColorTable = (PFNGLCOLORTABLEEXTPROC ) GET_GL_PROC((GET_GL_PTR_TYP)"glColorTableEXT");
    glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glMultiTexCoord2fARB");
    glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glClientActiveTextureARB");
    glActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glActiveTextureARB");
    if (!glMultiTexCoord2fARB) {
      glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glMultiTexCoord2fEXT");
      glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glClientActiveTextureEXT");
      glActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glActiveTextureEXT");
    }
#endif
    if (glutExtensionSupported ("GL_EXT_texture_compression_s3tc")) {
      (void) fprintf(stderr, "OpenGL::Texture Compression supported\n");
    } else {
      gl_options.compression=0;
      (void) fprintf(stderr, "OpenGL::Texture Compression unsupported\n");
    }
    if (glutExtensionSupported ("GL_ARB_multitexture")||glutExtensionSupported ("GL_EXT_multitexture")) {
      gl_options.Multitexture = 1*gl_options.Multitexture;//might be zero by input
      (void) fprintf(stderr, "OpenGL::Multitexture supported\n");
    } else {
      gl_options.Multitexture = 0;
      (void) fprintf(stderr, "OpenGL::Multitexture unsupported\n");
    }
    if ( glutExtensionSupported( "GL_ARB_texture_cube_map" ) || glutExtensionSupported( "GL_EXT_texture_cube_map" ) ) {
      gl_options.cubemap = 1;
      (void) fprintf(stderr, "OpenGL::TextureCubeMapExt supported\n");
    } else {
      gl_options.cubemap = 0;
      (void) fprintf(stderr, "OpenGL::TextureCubeMapExt unsupported\n");
    }
}

 static void initfov () {

    g_game.fov = XMLSupport::parse_float (vs_config->getVariable ("graphics","fov","78"));
    g_game.aspect = XMLSupport::parse_float (vs_config->getVariable ("graphics","aspect","1.33"));
    g_game.znear = XMLSupport::parse_float (vs_config->getVariable ("graphics","znear","1"));
    g_game.zfar = XMLSupport::parse_float (vs_config->getVariable ("graphics","zfar","100000"));
    g_game.detaillevel = XMLSupport::parse_float (vs_config->getVariable ("graphics","ModelDetail","1"));
    /*
    FILE * fp = fopen ("glsetup.txt","r");
    if (fp) {
      fscanf (fp,"fov %f\n",&g_game.fov);
      fscanf (fp,"aspect %f\n",&g_game.aspect);
      fscanf (fp,"znear %f\n",&g_game.znear);
      fscanf (fp,"zfar %f\n",&g_game.zfar);
      fclose (fp);
    }
    */
 }
static void Reshape (int x, int y) {
  g_game.x_resolution = x;
  g_game.y_resolution = y;

  
}
extern void GFXInitTextureManager();
void GFXInit (int argc, char ** argv){
#ifdef _WIN32
	int dumbi;
	MSG msg;
	hWnd=CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_START),NULL, (DLGPROC)DLOG_start, 0);
	ShowWindow(hWnd, SW_SHOW);
	for (dumbi=0;dumbi<3;dumbi++) {
		if(GetMessage(&msg, NULL, 0, 0)) {
			DispatchMessage(&msg);
		} else {
			break;
		}
	}
//	DialogBox (hInst,(LPCTSTR)IDD_START,hWnd,(DLGPROC)DLOG_start);
#endif
    glutInit( &argc, argv );
    g_game.x_resolution = XMLSupport::parse_int (vs_config->getVariable ("graphics","x_resolution","1024"));     
    g_game.y_resolution = XMLSupport::parse_int (vs_config->getVariable ("graphics","y_resolution","768"));     
    gl_options.mipmap = XMLSupport::parse_int (vs_config->getVariable ("graphics","mipmapdetail","2"));     
    gl_options.compression = XMLSupport::parse_bool (vs_config->getVariable ("graphics","texture_compression","false"));
    gl_options.Multitexture = XMLSupport::parse_bool (vs_config->getVariable ("graphics","reflection","true"));
    gl_options.fullscreen = XMLSupport::parse_bool (vs_config->getVariable ("graphics","fullscreen","false"));
    gl_options.color_depth = XMLSupport::parse_int (vs_config->getVariable ("graphics","colordepth","16"));
    gl_options.display_lists = XMLSupport::parse_bool (vs_config->getVariable ("graphics","displaylists","false"));
#ifdef USE_STENCIL_BUFFER
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL );
#else
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );
#endif

      char str [15];
      sprintf (str, "%dx%d:%d",g_game.x_resolution,g_game.y_resolution,gl_options.color_depth); 
      glutGameModeString(str);

    /* Create a window */
      if (gl_options.fullscreen) {
	glutInitWindowPosition( 0, 0 );
	glutEnterGameMode();
    } else {
	/* Set the initial window size */
	glutInitWindowSize(g_game.x_resolution, g_game.y_resolution);
	glutInitWindowPosition(0, 0);
	glutWindow = glutCreateWindow("Vegastrike");
	if (glutWindow == 0) {
	    (void) fprintf(stderr, "Couldn't create a window.\n");
	    exit(1);
	} 
    }
    /* Ingore key-repeat messages */
    glutIgnoreKeyRepeat(1);
    glViewport (0, 0, g_game.x_resolution,g_game.y_resolution);
    float clearcol[4];
    vs_config->getColor ("space_background",clearcol);
    glClearColor (clearcol[0],clearcol[1],clearcol[2],clearcol[3]);
    glutReshapeFunc (Reshape);
    initfov();
    glShadeModel (GL_SMOOTH);
    glEnable (GL_CULL_FACE);
    glCullFace (GL_BACK);
    glShadeModel (GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);
    
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0);

    init_opengl_extensions();
    GFXInitTextureManager();
    if (gl_options.Multitexture)
      GFXActiveTexture(0);

    glEnable(GL_TEXTURE_2D);		// use two-dimensional texturing
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


#if defined(IRIX)
    glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
#endif
    if (glutExtensionSupported("GL_EXT_color_table")||glutExtensionSupported ("GL_EXT_shared_texture_palette")) {
      gl_options.PaletteExt = 1;
      //(void) fprintf(stderr, "OpenGL::EXTColorTable supported\n");
    } else {
      gl_options.PaletteExt = 0;
      (void) fprintf(stderr, "OpenGL::EXTColorTable unsupported\n");
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    if (gl_options.Multitexture){
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
    GFXCreateLightContext(con);
    glutSetCursor(GLUT_CURSOR_NONE);
}

#if defined(IRIX)
#include "lin_time.h"

/*
** Update the game counter, generate a redisplay request.
*/
void idle_loop(void) {
	UpdateTime();
	glutPostRedisplay();
}

/*
** Install idle loop only if window is visible.
*/
void visible(int vis) {
	glutIdleFunc(vis == GLUT_VISIBLE ? idle_loop : NULL);
}

/*
** Install the display and visibility callback functions,
** start the main loop.
*/
void GFXLoop(void (*main_loop)(void)) {
	glutDisplayFunc(main_loop);
	glutVisibilityFunc(visible);
	glutMainLoop(); /*NOTREACHED*/
}
#else

void GFXLoop(void main_loop()) {
  glutDisplayFunc(main_loop);
  glutIdleFunc (main_loop);
#ifdef _WIN32
  DestroyWindow(hWnd);
#endif
  glutMainLoop();
  //never make it here;

}
#endif

void GFXShutdown () {
  extern void GFXDestroyAllLights();

  GFXDestroyAllTextures();
  GFXDestroyAllLights();
  if ( gl_options.fullscreen ) {
    glutLeaveGameMode();
  }
}

