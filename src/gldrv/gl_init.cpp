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
#define GL_INIT_CPP
#include "gl_globals.h"
#undef GL_INIT_CPP
#include "gl_include.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "config_xml.h"
#include "winsys.h"
#include <assert.h>
#include "gfxlib.h"


#if !defined(_WIN32) && !defined(__CYGWIN__)

//#if !(defined(__APPLE__) || defined(MACOSX))
//#define GL_GLEXT_PROTOTYPES 1
//#define GLX_GLXEXT_PROTOTYPES 1
//#define GLX_GLXEXT_LEGACY 1

//    #   include <GL/glxext.h>
  //  #   include <GL/glx.h>
    //#   include <GL/glxext.h>
//#endif

#include <stdlib.h>

#else
#include <windows.h>
#endif
#define GL_GLEXT_PROTOTYPES 1
#if defined(__APPLE__) || defined(MACOSX)
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
#else
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif
#ifdef GL_EXT_compiled_vertex_array
# ifndef PFNGLLOCKARRAYSEXTPROC
#  undef GL_EXT_compiled_vertex_array
# endif
#endif
#if !defined(IRIX)
//typedef void (APIENTRY * PFNGLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
//typedef void (APIENTRY * PFNGLUNLOCKARRAYSEXTPROC) (void);

#if !defined(__APPLE__) && !defined(MACOSX) && !defined(WIN32)
    # define GLX_GLXEXT_PROTOTYPES 1
    # define GLX_GLXEXT_LEGACY 1
    # include <GL/glx.h>
    # include <GL/glext.h>
#endif
#endif

#include <stdio.h>
#include "gl_init.h"
#define WINDOW_TITLE "Vega Strike "VERSION

#if defined(CG_SUPPORT)
#include "cg_global.h"
#endif

#if !defined(__APPLE__)

PFNGLBINDBUFFERARBPROC glBindBufferARB_p=0;
PFNGLGENBUFFERSARBPROC glGenBuffersARB_p=0;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB_p=0;
PFNGLBUFFERDATAARBPROC glBufferDataARB_p=0;
PFNGLMAPBUFFERARBPROC  glMapBufferARB_p=0;
PFNGLUNMAPBUFFERARBPROC  glUnmapBufferARB_p=0;

PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB_p=0;
PFNGLCLIENTACTIVETEXTUREARBPROC glActiveTextureARB_p=0;
PFNGLCOLORTABLEEXTPROC glColorTable_p=0;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB_p = 0;

PFNGLLOCKARRAYSEXTPROC glLockArraysEXT_p = 0;
PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT_p = 0;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D_p = 0;
#endif /* __APPLE_PANTHER_GCC33_CLI__ */

typedef void (*(*get_gl_proc_fptr_t)(const GLubyte *))(); 
#ifdef _WIN32
    typedef char * GET_GL_PTR_TYP;
#define GET_GL_PROC wglGetProcAddress

#else
    typedef GLubyte * GET_GL_PTR_TYP;
#define GET_GL_PROC glXGetProcAddressARB
#endif

#if defined(CG_SUPPORT)
CG_Cloak *cloak_cg = new CG_Cloak();
#endif
/* CENTRY */
int 
vsExtensionSupported(const char *extension)
{
  static const GLubyte *extensions = NULL;
  const GLubyte *start;
  GLubyte *where, *terminator;

  /* Extension names should not have spaces. */
  where = (GLubyte *) strchr(extension, ' ');
  if (where || *extension == '\0')
    return 0;

  if (!extensions) {
    extensions = glGetString(GL_EXTENSIONS);
  }
  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string.  Don't be fooled by sub-strings,
     etc. */
  start = extensions;
  for (;;) {
    /* If your application crashes in the strstr routine below,
       you are probably calling vsExtensionSupported without
       having a current window.  Calling glGetString without
       a current OpenGL context has unpredictable results.
       Please fix your program. */
    where = (GLubyte *) strstr((const char *) start, extension);
    if (!where)
      break;
    terminator = where + strlen(extension);
    if (where == start || *(where - 1) == ' ') {
      if (*terminator == ' ' || *terminator == '\0') {
        return 1;
      }
    }
    start = terminator;
  }
  return 0;
}
void init_opengl_extensions()
{
	const unsigned char * extensions = glGetString(GL_EXTENSIONS);

(void) VSFileSystem::vs_fprintf(stderr, "OpenGL Extensions supported: %s\n", extensions);

#ifndef NO_COMPILEDVERTEXARRAY_SUPPORT
    if (vsExtensionSupported( "GL_EXT_compiled_vertex_array")&&XMLSupport::parse_bool (vs_config->getVariable ("graphics","LockVertexArrays","true"))) {
#ifdef __APPLE__
#ifndef __APPLE_PANTHER_GCC33_CLI__
#if defined(glLockArraysEXT)&&defined(glUnlockArraysEXT)
        glLockArraysEXT_p = &glLockArraysEXT;
        glUnlockArraysEXT_p = &glUnlockArraysEXT;
#else
		glLockArraysEXT_p = 0;
		glUnlockArraysEXT_p = 0;
#endif
#endif /*__APPLE_PANTHER_GCC33_CLI__*/
#else
	glLockArraysEXT_p = (PFNGLLOCKARRAYSEXTPROC) 
	    GET_GL_PROC( (GET_GL_PTR_TYP) "glLockArraysEXT" );
	glUnlockArraysEXT_p = (PFNGLUNLOCKARRAYSEXTPROC) 
	    GET_GL_PROC( (GET_GL_PTR_TYP) "glUnlockArraysEXT" );	
#endif
	(void) VSFileSystem::vs_fprintf(stderr, "OpenGL::GL_EXT_compiled_vertex_array supported\n");
    } else {    
#ifdef __APPLE__
#ifndef __APPLE_PANTHER_GCC33_CLI__
		glLockArraysEXT_p = 0;
		glUnlockArraysEXT_p = 0;
#endif /*__APPLE_PANTHER_GCC33_CLI__*/
#endif
		(void) VSFileSystem::vs_fprintf(stderr, "OpenGL::GL_EXT_compiled_vertex_array unsupported\n");
    }
#endif

#ifdef __APPLE__
#ifndef __APPLE__
	glColorTable_p = glColorTableEXT;
	glMultiTexCoord2fARB_p = glMultiTexCoord2fARB;
	glClientActiveTextureARB_p = glClientActiveTextureARB;
	glActiveTextureARB_p = glActiveTextureARB;		
#endif /*__APPLE_PANTHER_GCC33_CLI__*/
#else
#ifndef NO_VBO_SUPPORT
    glBindBufferARB_p=(PFNGLBINDBUFFERARBPROC)GET_GL_PROC((GET_GL_PTR_TYP)"glBindBuffer");	;
    glGenBuffersARB_p=(PFNGLGENBUFFERSARBPROC)GET_GL_PROC((GET_GL_PTR_TYP)"glGenBuffers");	;
    glDeleteBuffersARB_p=(PFNGLDELETEBUFFERSARBPROC)GET_GL_PROC((GET_GL_PTR_TYP)"glDeleteBuffers");	;
    glBufferDataARB_p=(PFNGLBUFFERDATAARBPROC)GET_GL_PROC((GET_GL_PTR_TYP)"glBufferData");	;
    glMapBufferARB_p=(PFNGLMAPBUFFERARBPROC)GET_GL_PROC((GET_GL_PTR_TYP)"glMapBuffer");	;
    glUnmapBufferARB_p=(PFNGLUNMAPBUFFERARBPROC)GET_GL_PROC((GET_GL_PTR_TYP)"glUnmapBuffer");	
#endif

    glColorTable_p = (PFNGLCOLORTABLEEXTPROC ) GET_GL_PROC((GET_GL_PTR_TYP)"glColorTableEXT");
    glMultiTexCoord2fARB_p = (PFNGLMULTITEXCOORD2FARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glMultiTexCoord2fARB");
    glClientActiveTextureARB_p = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glClientActiveTextureARB");
    glActiveTextureARB_p = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glActiveTextureARB");
    if(!glMultiTexCoord2fARB_p) 
        glMultiTexCoord2fARB_p = (PFNGLMULTITEXCOORD2FARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glMultiTexCoord2fEXT");
    if(!glClientActiveTextureARB_p)
        glClientActiveTextureARB_p = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glClientActiveTextureEXT");
    if(!glActiveTextureARB_p)
        glActiveTextureARB_p = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glActiveTextureEXT");	
	if(!glCompressedTexImage2D_p)
		glCompressedTexImage2D_p = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) GET_GL_PROC((GET_GL_PTR_TYP)"glCompressedTexImage2D");
#endif

#ifdef GL_FOG_DISTANCE_MODE_NV
    if (vsExtensionSupported ("GL_NV_fog_distance")) {
      VSFileSystem::vs_fprintf (stderr,"OpenGL::Accurate Fog Distance supported\n");
      int foglev=XMLSupport::parse_int (vs_config->getVariable ("graphics","fogdetail","0"));
      switch (foglev) {
      case 0:
	glFogi (GL_FOG_DISTANCE_MODE_NV,GL_EYE_PLANE_ABSOLUTE_NV);
	break;
      case 1:
	glFogi (GL_FOG_DISTANCE_MODE_NV,GL_EYE_PLANE);
	break;
      case 2:
	glFogi (GL_FOG_DISTANCE_MODE_NV,GL_EYE_RADIAL_NV);
	break;
      }
    }else {
#endif    
      VSFileSystem::vs_fprintf (stderr,"OpenGL::Accurate Fog Distance unsupported\n");
#ifdef GL_FOG_DISTANCE_MODE_NV
    }
#endif

    if (vsExtensionSupported ("GL_ARB_texture_compression")) {
      VSFileSystem::vs_fprintf (stderr,"OpenGL::Generic Texture Compression supported\n");
    }else {
      VSFileSystem::vs_fprintf (stderr,"OpenGL::Generic Texture Compression unsupported\n");
      gl_options.compression=0;
    }
    if (vsExtensionSupported ("GL_EXT_texture_compression_s3tc")) {
      (void) VSFileSystem::vs_fprintf(stderr, "OpenGL::S3TC Texture Compression supported\n");
      //should be true;
    } else {
      gl_options.s3tc=false;
      (void) VSFileSystem::vs_fprintf(stderr, "OpenGL::S3TC Texture Compression unsupported\n");
    }
    if (  (glMultiTexCoord2fARB_p&&glClientActiveTextureARB_p&&glActiveTextureARB_p)
        &&(vsExtensionSupported ("GL_ARB_multitexture")||vsExtensionSupported ("GL_EXT_multitexture"))  ) {
      GLint multitex=gl_options.Multitexture;
      glGetIntegerv(GL_MAX_TEXTURE_UNITS,&multitex);
      if (multitex>1)
          gl_options.Multitexture = multitex; else
          gl_options.Multitexture = 0;
      //gl_options.Multitexture = 1*gl_options.Multitexture;//might be zero by input
      if (gl_options.Multitexture)
          (void) VSFileSystem::vs_fprintf(stderr, "OpenGL::Multitexture supported (%d units)\n", gl_options.Multitexture);
    } else {
      gl_options.Multitexture = 0;
      (void) VSFileSystem::vs_fprintf(stderr, "OpenGL::Multitexture unsupported\n");
    }
    if ( vsExtensionSupported( "GL_ARB_texture_cube_map" ) || vsExtensionSupported( "GL_EXT_texture_cube_map" ) ) {
      gl_options.cubemap = 1;
      (void) VSFileSystem::vs_fprintf(stderr, "OpenGL::TextureCubeMapExt supported\n");
    } else {
      gl_options.cubemap = 0;
      (void) VSFileSystem::vs_fprintf(stderr, "OpenGL::TextureCubeMapExt unsupported\n"); 
    }
    if (vsExtensionSupported ("GL_EXT_texture_edge_clamp")||vsExtensionSupported ("GL_SGIS_texture_edge_clamp")) {
      (void) VSFileSystem::vs_fprintf(stderr, "OpenGL::S3TC Texture Clamp-to-Edge supported\n");
      //should be true;
    } else {
      gl_options.ext_clamp_to_edge=false;
      (void) VSFileSystem::vs_fprintf(stderr, "OpenGL::S3TC Texture Clamp-to-Edge unsupported\n");
    }
    if (vsExtensionSupported ("GL_ARB_texture_border_clamp")||vsExtensionSupported ("GL_SGIS_texture_border_clamp")) {
      (void) VSFileSystem::vs_fprintf(stderr, "OpenGL::S3TC Texture Clamp-to-Border supported\n");
      //should be true;
    } else {
      gl_options.ext_clamp_to_border=false;
      (void) VSFileSystem::vs_fprintf(stderr, "OpenGL::S3TC Texture Clamp-to-Border unsupported\n");
    }

#if defined(CG_SUPPORT)
	if(! glh_init_extensions(CG_REQUIRED_EXTENSIONS))
	{
		cerr << "Necessary OpenGL extensions for Cg were not supported:" << endl
			 << glh_get_unsupported_extensions() << endl << endl
			 << "Press <enter> to quit." << endl;
		char buff[10];
		cin.getline(buff, 10);
		winsys_exit(0);
	}

cloak_cg->vertexProfile = CG_PROFILE_ARBVP1;
cloak_cg->shaderContext = cgCreateContext();
cloak_cg->cgLoadMedia("programs/cloaking_effect", "vertex.cg");
#endif


    
}

 static void initfov () {

    g_game.fov = XMLSupport::parse_float (vs_config->getVariable ("graphics","fov","78"));
    g_game.aspect = XMLSupport::parse_float (vs_config->getVariable ("graphics","aspect","1.33"));
    g_game.znear = XMLSupport::parse_float (vs_config->getVariable ("graphics","znear","1"));
    g_game.zfar = XMLSupport::parse_float (vs_config->getVariable ("graphics","zfar","100000"));
    g_game.detaillevel = XMLSupport::parse_float (vs_config->getVariable ("graphics","ModelDetail","1"));
    g_game.use_textures = XMLSupport::parse_bool (vs_config->getVariable ("graphics","UseTextures","true"));
    g_game.use_ship_textures = XMLSupport::parse_bool (vs_config->getVariable ("graphics","UseShipTextures","false"));
    g_game.use_planet_textures = XMLSupport::parse_bool (vs_config->getVariable ("graphics","UsePlanetTextures","false"));
    g_game.use_logos = XMLSupport::parse_bool (vs_config->getVariable ("graphics","UseLogos","true"));
    g_game.use_sprites = XMLSupport::parse_bool (vs_config->getVariable ("graphics","UseVSSprites","true"));
    g_game.use_animations = XMLSupport::parse_bool (vs_config->getVariable ("graphics","UseAnimations","true"));
    g_game.use_videos = XMLSupport::parse_bool (vs_config->getVariable ("graphics","UseVideos","true"));


    /*
    FILE * fp = fopen ("glsetup.txt","r");
    if (fp) {
      VSFileSystem::Fscanf (fp,"fov %f\n",&g_game.fov);
      VSFileSystem::Fscanf (fp,"aspect %f\n",&g_game.aspect);
      VSFileSystem::Fscanf (fp,"znear %f\n",&g_game.znear);
      VSFileSystem::Fscanf (fp,"zfar %f\n",&g_game.zfar);
      VSFileSystem::Close (fp);
    }
    */
 }
static void Reshape (int x, int y) {
  g_game.x_resolution = x;
  g_game.y_resolution = y;
  VSFileSystem::vs_fprintf (stderr,"Reshaping %d %d", x,y);
  
}
extern void GFXInitTextureManager();
void GFXInit (int argc, char ** argv){
  winsys_init (&argc,argv,"Vega Strike","vega.ico");
    /* Ingore key-repeat messages */
  winsys_enable_key_repeat(false);

    glViewport (0, 0, g_game.x_resolution,g_game.y_resolution);
    float clearcol[4];
    gl_options.wireframe = XMLSupport::parse_bool (vs_config->getVariable ("graphics","use_wireframe","0"));     
    gl_options.max_texture_dimension= XMLSupport::parse_int (vs_config->getVariable ("graphics","max_texture_dimension","65536"));     
    gl_options.smooth_shade = XMLSupport::parse_bool (vs_config->getVariable ("graphics","SmoothShade","true"));     
    gl_options.mipmap = XMLSupport::parse_int (vs_config->getVariable ("graphics","mipmapdetail","2"));     
    gl_options.compression = XMLSupport::parse_int (vs_config->getVariable ("graphics","texture_compression","0"));
    gl_options.Multitexture = XMLSupport::parse_bool (vs_config->getVariable ("graphics","reflection","true"));
    gl_options.display_lists = XMLSupport::parse_bool (vs_config->getVariable ("graphics","displaylists","false"));
    gl_options.s3tc = XMLSupport::parse_bool (vs_config->getVariable ("graphics","s3tc","true"));
    gl_options.ext_clamp_to_edge = XMLSupport::parse_bool (vs_config->getVariable ("graphics","ext_clamp_to_edge","true"));
    gl_options.ext_clamp_to_border = XMLSupport::parse_bool (vs_config->getVariable ("graphics","ext_clamp_to_border","true"));

    vs_config->getColor ("space_background",clearcol);
    glClearColor (clearcol[0],clearcol[1],clearcol[2],clearcol[3]);
    winsys_set_reshape_func (Reshape);
    initfov();
    glShadeModel (GL_SMOOTH);
    glEnable (GL_CULL_FACE);
    glCullFace (GL_BACK);
    glShadeModel (GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);
    if (gl_options.wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    }
    if (gl_options.smooth_shade==0) {
      glShadeModel (GL_FLAT);
    }

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
    if (vsExtensionSupported("GL_EXT_color_table")||vsExtensionSupported ("GL_EXT_shared_texture_palette")) {
      gl_options.PaletteExt = 1;
      //(void) VSFileSystem::vs_fprintf(stderr, "OpenGL::EXTColorTable supported\n");
    } else {
      gl_options.PaletteExt = 0;
      (void) VSFileSystem::vs_fprintf(stderr, "OpenGL::EXTColorTable unsupported\n");
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    if (gl_options.Multitexture){
		for (int i=1;i<4;++i) {
			GFXActiveTexture(i);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			GFXTextureEnv (i,GFXADDTEXTURE);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);


			// Spherical texture coordinate generation
			if (i==1) {			
#ifdef NV_CUBE_MAP
                GFXToggleTexture(true,1,CUBEMAP);
                GFXTextureCoordGenMode(1,CUBE_MAP_GEN,NULL,NULL);
#else
				const float tempo[4]={1,0,0,0};
                GFXToggleTexture(true,1,TEXTURE2D);
				GFXTextureCoordGenMode(1,SPHERE_MAP_GEN,tempo,tempo);
#endif
			}
		}
    }
	GFXActiveTexture(0);
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
    //    glutSetCursor(GLUT_CURSOR_NONE);

    /* Avoid scrambled screen on startup - Twice, for triple buffering */
    if (XMLSupport::parse_bool (vs_config->getVariable ("graphics","ClearOnStartup","true"))) {
      glClear(GL_COLOR_BUFFER_BIT);
      winsys_swap_buffers();
      glClear(GL_COLOR_BUFFER_BIT);
      winsys_swap_buffers();
    }

    winsys_show_cursor(false);
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
	static bool are_we_looping=false;
	///so we can call this function multiple times
	if (!are_we_looping) {
	  are_we_looping=true;
	  glutMainLoop();
	}
}
#else

void GFXLoop(void main_loop()) {
  winsys_set_display_func (main_loop);
  winsys_set_idle_func (main_loop);

  //  glutDisplayFunc(main_loop);
  //  glutIdleFunc (main_loop);
  static bool are_we_looping=false;
  /// so we can call this function multiple times to change the display and idle functions
  if (!are_we_looping) {
    are_we_looping=true;
    winsys_process_events();
#define CODE_NOT_REACHED 0
  assert(CODE_NOT_REACHED);
  //never make it here;
  }


}
#endif

void GFXShutdown () {
  extern void GFXDestroyAllLights();

  GFXDestroyAllTextures();
  GFXDestroyAllLights();
  if ( gl_options.fullscreen ) {
    winsys_shutdown();
  }
}

