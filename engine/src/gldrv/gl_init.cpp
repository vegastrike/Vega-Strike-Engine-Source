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
#include <assert.h>
#include <string>

#define GL_INIT_CPP
#include "gl_globals.h"
#undef GL_INIT_CPP
#include "gl_include.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "config_xml.h"
#include "winsys.h"
#include "gfxlib.h"
#include "options.h"



#if !defined (_WIN32) && !defined (__CYGWIN__)

//#if !(defined(__APPLE__) || defined(MACOSX))
//#define GL_GLEXT_PROTOTYPES 1
//#define GLX_GLXEXT_PROTOTYPES 1
//#define GLX_GLXEXT_LEGACY 1

//#   include <GL/glxext.h>
//#   include <GL/glx.h>
//#   include <GL/glxext.h>
//#endif

#include <stdlib.h>

#else
#ifndef NOMINMAX
#define NOMINMAX
#endif //tells VCC not to generate min/max macros
#define GL_TEXTURE_CUBE_MAP_SEAMLESS_ARB 0x884F // FIXME May need to actually include the GLEW package for this to work.
#include <windows.h>
#endif
#define GL_GLEXT_PROTOTYPES 1
#if defined (__APPLE__) || defined (MACOSX)
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
#if !defined (IRIX)
//typedef void (APIENTRY * PFNGLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
//typedef void (APIENTRY * PFNGLUNLOCKARRAYSEXTPROC) (void);

#if !defined (__APPLE__) && !defined (MACOSX) && !defined (WIN32)  && !defined (__HAIKU__)
    # define GLX_GLXEXT_PROTOTYPES 1
    # define GLX_GLXEXT_LEGACY 1
    # include <GL/glx.h>
    # include <GL/glext.h>
#endif
#endif

#include <stdio.h>
#include "gl_init.h"
#define WINDOW_TITLE "Vega Strike " VERSION

#if defined (CG_SUPPORT)
#include "cg_global.h"
#endif

#if !defined (__APPLE__)

PFNGLBINDBUFFERARBPROC    glBindBufferARB_p    = 0;
PFNGLGENBUFFERSARBPROC    glGenBuffersARB_p    = 0;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB_p = 0;
PFNGLBUFFERDATAARBPROC    glBufferDataARB_p    = 0;
PFNGLMAPBUFFERARBPROC     glMapBufferARB_p     = 0;
PFNGLUNMAPBUFFERARBPROC   glUnmapBufferARB_p   = 0;

PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB_p = 0;
PFNGLCLIENTACTIVETEXTUREARBPROC glActiveTextureARB_p = 0;
PFNGLCOLORTABLEEXTPROC glColorTable_p = 0;
PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2fARB_p     = 0;
PFNGLMULTITEXCOORD4FARBPROC     glMultiTexCoord4fARB_p     = 0;

PFNGLLOCKARRAYSEXTPROC        glLockArraysEXT_p        = 0;
PFNGLUNLOCKARRAYSEXTPROC      glUnlockArraysEXT_p      = 0;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D_p = 0;
PFNGLMULTIDRAWARRAYSEXTPROC   glMultiDrawArrays_p      = 0;
PFNGLMULTIDRAWELEMENTSEXTPROC glMultiDrawElements_p    = 0;

PFNGLGETSHADERIVPROC        glGetShaderiv_p        = 0;
PFNGLGETPROGRAMIVPROC       glGetProgramiv_p       = 0;
PFNGLGETSHADERINFOLOGPROC   glGetShaderInfoLog_p   = 0;
PFNGLGETPROGRAMINFOLOGPROC  glGetProgramInfoLog_p  = 0;
PFNGLCREATESHADERPROC       glCreateShader_p       = 0;
PFNGLSHADERSOURCEPROC       glShaderSource_p       = 0;
PFNGLCOMPILESHADERPROC      glCompileShader_p      = 0;
PFNGLCREATEPROGRAMPROC      glCreateProgram_p      = 0;
PFNGLATTACHSHADERPROC       glAttachShader_p       = 0;
PFNGLLINKPROGRAMPROC        glLinkProgram_p        = 0;
PFNGLUSEPROGRAMPROC glUseProgram_p = 0;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_p = 0;
PFNGLUNIFORM1FPROC     glUniform1f_p     = 0;
PFNGLUNIFORM2FPROC     glUniform2f_p     = 0;
PFNGLUNIFORM3FPROC     glUniform3f_p     = 0;
PFNGLUNIFORM4FPROC     glUniform4f_p     = 0;

PFNGLUNIFORM1IPROC     glUniform1i_p     = 0;
PFNGLUNIFORM2IPROC     glUniform2i_p     = 0;
PFNGLUNIFORM3IPROC     glUniform3i_p     = 0;
PFNGLUNIFORM4IPROC     glUniform4i_p     = 0;

PFNGLUNIFORM1FVPROC    glUniform1fv_p    = 0;
PFNGLUNIFORM2FVPROC    glUniform2fv_p    = 0;
PFNGLUNIFORM3FVPROC    glUniform3fv_p    = 0;
PFNGLUNIFORM4FVPROC    glUniform4fv_p    = 0;

PFNGLUNIFORM1IVPROC    glUniform1iv_p    = 0;
PFNGLUNIFORM2IVPROC    glUniform2iv_p    = 0;
PFNGLUNIFORM3IVPROC    glUniform3iv_p    = 0;
PFNGLUNIFORM4IVPROC    glUniform4iv_p    = 0;

PFNGLDELETESHADERPROC  glDeleteShader_p  = 0;
PFNGLDELETEPROGRAMPROC glDeleteProgram_p = 0;

#endif /* __APPLE_PANTHER_GCC33_CLI__ */

typedef void ( *(*get_gl_proc_fptr_t)(const GLubyte*) )();
#ifdef _WIN32
typedef char*GET_GL_PTR_TYP;
#define GET_GL_PROC wglGetProcAddress
#else
	#if defined(__HAIKU__)
	    typedef char * GET_GL_PTR_TYP;
		#define GET_GL_PROC glutGetProcAddress
	#else
		typedef GLubyte*GET_GL_PTR_TYP;
		#define GET_GL_PROC glXGetProcAddressARB
	#endif
#endif

#if defined (CG_SUPPORT)
CG_Cloak *cloak_cg = new CG_Cloak();
#endif

/* CENTRY */
int vsExtensionSupported( const char *extension )
{
    static const GLubyte *extensions = NULL;
    const GLubyte *start;
    GLubyte *where, *terminator;

    /* Extension names should not have spaces. */
    where = (GLubyte*) strchr( extension, ' ' );
    if (where || *extension == '\0')
        return 0;
    if (!extensions)
        extensions = glGetString( GL_EXTENSIONS );
    /* It takes a bit of care to be fool-proof about parsing the
     *  OpenGL extensions string.  Don't be fooled by sub-strings,
     *  etc. */
    start = extensions;
    for (;;) {
        /* If your application crashes in the strstr routine below,
         *  you are probably calling vsExtensionSupported without
         *  having a current window.  Calling glGetString without
         *  a current OpenGL context has unpredictable results.
         *  Please fix your program. */
        where = (GLubyte*) strstr( (const char*) start, extension );
        if (!where)
            break;
        terminator = where+strlen( extension );
        if (where == start || *(where-1) == ' ')
            if (*terminator == ' ' || *terminator == '\0')
                return 1;
        start = terminator;
    }
    return 0;
}

bool vsVendorMatch( const char *vendor )
{
    static const GLubyte *_glvendor = NULL;

    if (_glvendor == NULL) {
        _glvendor = glGetString( GL_VENDOR );
        //if (_glvendor != NULL) {
        //    BOOST_LOG_TRIVIAL(info) << boost::format("OpenGL Vendor: %1%") % (const char *)_glvendor;
        //}
    }

    if (_glvendor != NULL) {
        // NOTE: Don't be fooled by substrings within words
        //      Should match whole-words ONLY

        static const std::string glvendor = strtolower(std::string((const char*)_glvendor));
        static const std::string::size_type glvendor_sz = glvendor.length();
        std::string svendor = strtolower(vendor);
        std::string::size_type svendor_sz = svendor.length();
        std::string::size_type pos = glvendor.find(svendor);

        if (pos == std::string::npos)
            return false;

        if ((pos > 0) && isalnum(glvendor[pos-1]))
            return false;

        if ((pos+svendor_sz) < glvendor_sz && isalnum(glvendor[pos+svendor_sz]))
            return false;

        return true;
    } else {
        return false;
    }
}

void init_opengl_extensions()
{
    //const unsigned char *extensions = glGetString( GL_EXTENSIONS );

    //BOOST_LOG_TRIVIAL(trace) << boost::format("OpenGL Extensions supported: %1%") % extensions;

#ifndef NO_COMPILEDVERTEXARRAY_SUPPORT
    if ( vsExtensionSupported( "GL_EXT_compiled_vertex_array" )
        && game_options.LockVertexArrays ) {
#ifdef __APPLE__
#ifndef __APPLE_PANTHER_GCC33_CLI__
#if defined (glLockArraysEXT) && defined (glUnlockArraysEXT)
        glLockArraysEXT_p   = &glLockArraysEXT;
        glUnlockArraysEXT_p = &glUnlockArraysEXT;
#else
        glLockArraysEXT_p   = 0;
        glUnlockArraysEXT_p = 0;
#endif
#endif /*__APPLE_PANTHER_GCC33_CLI__*/
#else
        glLockArraysEXT_p   = (PFNGLLOCKARRAYSEXTPROC)
                              GET_GL_PROC( (GET_GL_PTR_TYP) "glLockArraysEXT" );
        glUnlockArraysEXT_p = (PFNGLUNLOCKARRAYSEXTPROC)
                              GET_GL_PROC( (GET_GL_PTR_TYP) "glUnlockArraysEXT" );
#endif
        //BOOST_LOG_TRIVIAL(trace) << "OpenGL::GL_EXT_compiled_vertex_array supported";
    } else {
#ifdef __APPLE__
#ifndef __APPLE_PANTHER_GCC33_CLI__
        glLockArraysEXT_p   = 0;
        glUnlockArraysEXT_p = 0;
#endif /*__APPLE_PANTHER_GCC33_CLI__*/
#endif
        //BOOST_LOG_TRIVIAL(debug) << "OpenGL::GL_EXT_compiled_vertex_array unsupported";
    }
#endif
#ifndef __APPLE__
    if ( vsExtensionSupported( "GL_EXT_multi_draw_arrays" ) ) {
        glMultiDrawArrays_p   = (PFNGLMULTIDRAWARRAYSEXTPROC)
                                GET_GL_PROC( (GET_GL_PTR_TYP) "glMultiDrawArraysEXT" );
        glMultiDrawElements_p = (PFNGLMULTIDRAWELEMENTSEXTPROC)
                                GET_GL_PROC( (GET_GL_PTR_TYP) "glMultiDrawElementsEXT" );
        //BOOST_LOG_TRIVIAL(trace) << "OpenGL::GL_EXT_multi_draw_arrays supported";
    } else {
        glMultiDrawArrays_p   = 0;
        glMultiDrawElements_p = 0;
        //BOOST_LOG_TRIVIAL(debug) << "OpenGL::GL_EXT_multi_draw_arrays unsupported";
    }
#endif

#ifdef __APPLE__
#ifndef __APPLE__
    glColorTable_p = glColorTableEXT;
    glMultiTexCoord2fARB_p     = glMultiTexCoord2fARB;
    glMultiTexCoord4fARB_p     = glMultiTexCoord4fARB;
    glClientActiveTextureARB_p = glClientActiveTextureARB;
    glActiveTextureARB_p = glActiveTextureARB;
#endif /*__APPLE_PANTHER_GCC33_CLI__*/
#else
#ifndef NO_VBO_SUPPORT
    glBindBufferARB_p    = (PFNGLBINDBUFFERARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glBindBuffer" );
    glGenBuffersARB_p    = (PFNGLGENBUFFERSARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glGenBuffers" );
    glDeleteBuffersARB_p = (PFNGLDELETEBUFFERSARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glDeleteBuffers" );
    glBufferDataARB_p    = (PFNGLBUFFERDATAARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glBufferData" );
    glMapBufferARB_p     = (PFNGLMAPBUFFERARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glMapBuffer" );
    glUnmapBufferARB_p   = (PFNGLUNMAPBUFFERARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUnmapBuffer" );
#endif

    glColorTable_p = (PFNGLCOLORTABLEEXTPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glColorTableEXT" );
    glMultiTexCoord2fARB_p     = (PFNGLMULTITEXCOORD2FARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glMultiTexCoord2fARB" );
    glMultiTexCoord4fARB_p     = (PFNGLMULTITEXCOORD4FARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glMultiTexCoord4fARB" );
    glClientActiveTextureARB_p = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glClientActiveTextureARB" );
    glActiveTextureARB_p = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glActiveTextureARB" );
    if (!glMultiTexCoord2fARB_p)
        glMultiTexCoord2fARB_p = (PFNGLMULTITEXCOORD2FARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glMultiTexCoord2fEXT" );
    if (!glMultiTexCoord4fARB_p)
        glMultiTexCoord4fARB_p = (PFNGLMULTITEXCOORD4FARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glMultiTexCoord4fEXT" );
    if (!glClientActiveTextureARB_p)
        glClientActiveTextureARB_p = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glClientActiveTextureEXT" );
    if (!glActiveTextureARB_p)
        glActiveTextureARB_p = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glActiveTextureEXT" );
    if (!glCompressedTexImage2D_p)
        glCompressedTexImage2D_p = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glCompressedTexImage2D" );
    if (!glGetShaderiv_p)
        glGetShaderiv_p = (PFNGLGETSHADERIVPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glGetShaderiv" );
    if (!glGetProgramiv_p)
        glGetProgramiv_p = (PFNGLGETPROGRAMIVPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glGetProgramiv" );
    if (!glGetShaderInfoLog_p)
        glGetShaderInfoLog_p = (PFNGLGETSHADERINFOLOGPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glGetShaderInfoLog" );
    if (!glGetProgramInfoLog_p)
        glGetProgramInfoLog_p = (PFNGLGETPROGRAMINFOLOGPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glGetProgramInfoLog" );
    if (!glCreateShader_p)
        glCreateShader_p = (PFNGLCREATESHADERPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glCreateShader" );
    if (!glCreateProgram_p)
        glCreateProgram_p = (PFNGLCREATEPROGRAMPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glCreateProgram" );
    if (!glShaderSource_p)
        glShaderSource_p = (PFNGLSHADERSOURCEPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glShaderSource" );
    if (!glCompileShader_p)
        glCompileShader_p = (PFNGLCOMPILESHADERPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glCompileShader" );
    if (!glAttachShader_p)
        glAttachShader_p = (PFNGLATTACHSHADERPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glAttachShader" );
    if (!glLinkProgram_p)
        glLinkProgram_p = (PFNGLLINKPROGRAMPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glLinkProgram" );
    if (!glUseProgram_p)
        glUseProgram_p = (PFNGLUSEPROGRAMPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUseProgram" );
    if (!glGetUniformLocation_p)
        glGetUniformLocation_p = (PFNGLGETUNIFORMLOCATIONPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glGetUniformLocation" );
    if (!glUniform1f_p)
        glUniform1f_p = (PFNGLUNIFORM1FPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform1f" );
    if (!glUniform2f_p)
        glUniform2f_p = (PFNGLUNIFORM2FPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform2f" );
    if (!glUniform3f_p)
        glUniform3f_p = (PFNGLUNIFORM3FPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform3f" );
    if (!glUniform4f_p)
        glUniform4f_p = (PFNGLUNIFORM4FPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform4f" );
    if (!glUniform1i_p)
        glUniform1i_p = (PFNGLUNIFORM1IPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform1i" );
    if (!glUniform2i_p)
        glUniform2i_p = (PFNGLUNIFORM2IPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform2i" );
    if (!glUniform3i_p)
        glUniform3i_p = (PFNGLUNIFORM3IPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform3i" );
    if (!glUniform4i_p)
        glUniform4i_p = (PFNGLUNIFORM4IPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform4i" );
    if (!glUniform1fv_p)
        glUniform1fv_p = (PFNGLUNIFORM1FVPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform1fv" );
    if (!glUniform2fv_p)
        glUniform2fv_p = (PFNGLUNIFORM2FVPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform2fv" );
    if (!glUniform3fv_p)
        glUniform3fv_p = (PFNGLUNIFORM3FVPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform3fv" );
    if (!glUniform4fv_p)
        glUniform4fv_p = (PFNGLUNIFORM4FVPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform4fv" );
    if (!glUniform1iv_p)
        glUniform1iv_p = (PFNGLUNIFORM1IVPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform1iv" );
    if (!glUniform2iv_p)
        glUniform2iv_p = (PFNGLUNIFORM2IVPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform2iv" );
    if (!glUniform3iv_p)
        glUniform3iv_p = (PFNGLUNIFORM3IVPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform3iv" );
    if (!glUniform4iv_p)
        glUniform4iv_p = (PFNGLUNIFORM4IVPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUniform4iv" );
    if (!glDeleteShader_p)
        glDeleteShader_p = (PFNGLDELETESHADERPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glDeleteShader" );
    if (!glDeleteProgram_p)
        glDeleteProgram_p = (PFNGLDELETEPROGRAMPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glDeleteProgram" );
    //fixme
#endif

#ifdef GL_FOG_DISTANCE_MODE_NV
    if ( vsExtensionSupported( "GL_NV_fog_distance" ) ) {
        //BOOST_LOG_TRIVIAL(trace) << "OpenGL::Accurate Fog Distance supported";
        switch (game_options.fogdetail)
        {
        case 0:
            glFogi( GL_FOG_DISTANCE_MODE_NV, GL_EYE_PLANE_ABSOLUTE_NV );
            break;
        case 1:
            glFogi( GL_FOG_DISTANCE_MODE_NV, GL_EYE_PLANE );
            break;
        case 2:
            glFogi( GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV );
            break;
        }
    } else {
#endif
        //BOOST_LOG_TRIVIAL(debug) << "OpenGL::Accurate Fog Distance unsupported";
#ifdef GL_FOG_DISTANCE_MODE_NV
    }
#endif
    if ( vsExtensionSupported( "GL_ARB_texture_compression" ) ) {
        //BOOST_LOG_TRIVIAL(trace) << "OpenGL::Generic Texture Compression supported";
    } else {
        //BOOST_LOG_TRIVIAL(info) << "OpenGL::Generic Texture Compression unsupported";
        gl_options.compression = 0;
    }
    if ( vsExtensionSupported( "GL_EXT_texture_compression_s3tc" ) ) {
        //BOOST_LOG_TRIVIAL(trace) << "OpenGL::S3TC Texture Compression supported";
        //should be true;
    } else {
        gl_options.s3tc = false;
        //BOOST_LOG_TRIVIAL(info) << "OpenGL::S3TC Texture Compression unsupported";
    }
    if ( (glMultiTexCoord2fARB_p && glMultiTexCoord4fARB_p && glClientActiveTextureARB_p && glActiveTextureARB_p)
        && ( vsExtensionSupported( "GL_ARB_multitexture" ) || vsExtensionSupported( "GL_EXT_multitexture" ) ) ) {
        GLint multitex = gl_options.Multitexture;
        glGetIntegerv( GL_MAX_TEXTURE_UNITS, &multitex );
        if (multitex > 1)
            gl_options.Multitexture = multitex;

        else
            gl_options.Multitexture = 0;
        //gl_options.Multitexture = 1*gl_options.Multitexture;//might be zero by input
        if (gl_options.Multitexture) {
            //BOOST_LOG_TRIVIAL(trace) << boost::format("OpenGL::Multitexture supported (%1% units)") % gl_options.Multitexture;
        }
    } else {
        gl_options.Multitexture = 0;
        //BOOST_LOG_TRIVIAL(debug) << "OpenGL::Multitexture unsupported";
    }
    if ( vsExtensionSupported( "GL_ARB_texture_cube_map" ) || vsExtensionSupported( "GL_EXT_texture_cube_map" ) ) {
        gl_options.cubemap = 1;
        //BOOST_LOG_TRIVIAL(trace) << "OpenGL::TextureCubeMapExt supported";
    } else {
        gl_options.cubemap = 0;
        //BOOST_LOG_TRIVIAL(info) << "OpenGL::TextureCubeMapExt unsupported";
    }
    if ( vsExtensionSupported( "GL_EXT_texture_edge_clamp" ) || vsExtensionSupported( "GL_SGIS_texture_edge_clamp" ) ) {
        //BOOST_LOG_TRIVIAL(trace) << "OpenGL::S3TC Texture Clamp-to-Edge supported";
        // should be true
    } else {
        gl_options.ext_clamp_to_edge = false;
        //BOOST_LOG_TRIVIAL(info) << "OpenGL::S3TC Texture Clamp-to-Edge unsupported";
    }
    if ( vsExtensionSupported( "GL_ARB_texture_border_clamp" ) || vsExtensionSupported( "GL_SGIS_texture_border_clamp" ) ) {
        //BOOST_LOG_TRIVIAL(trace) << "OpenGL::S3TC Texture Clamp-to-Border supported";
        // should be true
    } else {
        gl_options.ext_clamp_to_border = false;
        //BOOST_LOG_TRIVIAL(info) << "OpenGL::S3TC Texture Clamp-to-Border unsupported";
    }
    if ( vsExtensionSupported( "GL_ARB_framebuffer_sRGB" ) || vsExtensionSupported( "GL_EXT_framebuffer_sRGB" ) ) {
        //BOOST_LOG_TRIVIAL(trace) << "OpenGL::sRGB Framebuffer supported";
        GLboolean srgbCapable = true;

        if ( vsExtensionSupported( "GL_EXT_framebuffer_sRGB" ) ) {
            // TODO: GL_EXT_framebuffer_sRGB allows us to query sRGB capability easily.
            // The ARB variant requires us to use GLX and GLW stuff, which would be harder to do portably
            // For now, we'll just assume the ARB variant supports it, since it's quite probable that it
            // does for most framebuffer visuals.
            srgbCapable = false;
            glGetBooleanv(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &srgbCapable);
        }

        if (srgbCapable) {
            gl_options.ext_srgb_framebuffer = true;
        } else {
            //BOOST_LOG_TRIVIAL(info) << "OpenGL::sRGB Framebuffer unsupported by visual";
            gl_options.ext_srgb_framebuffer = false;
        }
    } else {
        gl_options.ext_srgb_framebuffer = false;
        //BOOST_LOG_TRIVIAL(info) << "OpenGL::sRGB Framebuffer unsupported";
    }
    if ( vsExtensionSupported( "GL_NV_fragment_program2" ) ) {
        gl_options.nv_fp2 = true;
        //BOOST_LOG_TRIVIAL(trace) << "OpenGL::NV_fragment_program2 supported";
    } else {
        gl_options.nv_fp2 = false;
        //BOOST_LOG_TRIVIAL(info) << "OpenGL::NV_fragment_program2 unsupported";
    }
    if ( GFXDefaultShaderSupported() )
        if (gl_options.Multitexture < 16)
            gl_options.Multitexture = 16;
    //only support shaders on ps_2_0 cards

    // some useful generic limits
    GLint max_vertices=-1, max_indices=-1;
    glGetIntegerv( GL_MAX_ELEMENTS_INDICES, &max_indices );
    glGetIntegerv( GL_MAX_ELEMENTS_VERTICES, &max_vertices );
    gl_options.max_array_indices = max_indices;
    gl_options.max_array_vertices = max_vertices;

    //BOOST_LOG_TRIVIAL(info) << boost::format("Max vertex array indices: %1%") % gl_options.max_array_indices;
    //BOOST_LOG_TRIVIAL(info) << boost::format("Max vertex array vertices: %1%") % gl_options.max_array_vertices;
}

static void initfov()
{
    g_game.fov    = game_options.fov;
    g_game.aspect = game_options.aspect;
    g_game.znear  = game_options.znear;
    g_game.zfar   = game_options.zfar;
    g_game.detaillevel         = game_options.ModelDetail;
    g_game.use_textures        = game_options.UseTextures;
    g_game.use_ship_textures   = game_options.UseShipTextures;
    g_game.use_planet_textures = game_options.UsePlanetTextures;
    g_game.use_logos           = game_options.UseLogos;
    g_game.use_sprites         = game_options.UseVSSprites;
    g_game.use_animations      = game_options.UseAnimations;
    g_game.use_videos          = game_options.UseVideos;

    /*
     *  FILE * fp = fopen ("glsetup.txt","r");
     *  if (fp) {
     *  VSFileSystem::Fscanf (fp,"fov %f\n",&g_game.fov);
     *  VSFileSystem::Fscanf (fp,"aspect %f\n",&g_game.aspect);
     *  VSFileSystem::Fscanf (fp,"znear %f\n",&g_game.znear);
     *  VSFileSystem::Fscanf (fp,"zfar %f\n",&g_game.zfar);
     *  VSFileSystem::Close (fp);
     *  }
     */
}

static void Reshape( int x, int y )
{
    g_game.x_resolution = x;
    g_game.y_resolution = y;
    //BOOST_LOG_TRIVIAL(trace) << boost::format("Reshaping %1% %2%") % x % y;
}

extern void GFXInitTextureManager();

void GFXInit( int argc, char **argv )
{
    char vsname[12] = "Vega Strike";
    char vsicon[9] = "vega.ico";
    winsys_init (&argc,argv,&vsname[0],&vsicon[0]);
    /* Ingore key-repeat messages */
    winsys_enable_key_repeat( false );

    glViewport( 0, 0, g_game.x_resolution, g_game.y_resolution );
    static GFXColor clearcol = vs_config->getColor( "space_background" );;
    gl_options.wireframe = game_options.use_wireframe;
    gl_options.max_texture_dimension = game_options.max_texture_dimension;
    gl_options.max_movie_dimension   = game_options.max_movie_dimension;
    bool textsupported = (vsExtensionSupported( "GL_ARB_texture_non_power_of_two" ) ||  vsExtensionSupported( "GL_ARB_texture_rectangle" ) || vsExtensionSupported( "GL_NV_texture_rectangle" )) ? "true" : "false" ;

    gl_options.rect_textures    = game_options.rect_textures ? true : textsupported;

    if (gl_options.rect_textures) {
        //BOOST_LOG_TRIVIAL(trace) << "RECT textures supported";

        // Fetch max rect textue dimension
        GLint max_rect_dimension = 65535;
        glGetIntegerv(
            GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB,
            &max_rect_dimension);

        gl_options.max_rect_dimension = max_rect_dimension;
        //BOOST_LOG_TRIVIAL(trace) << boost::format("RECT max texture dimension: %1%") % max_rect_dimension;
    }

    bool vidsupported = (   gl_options.rect_textures || (   vsExtensionSupported( "GL_ARB_texture_non_power_of_two" ) && vsVendorMatch("nvidia")));

    gl_options.pot_video_textures    = game_options.pot_video_textures ? true : vidsupported;

    if (!gl_options.pot_video_textures && gl_options.rect_textures) {
        // Enforce max rect texture for movies, which use them
        if (gl_options.max_movie_dimension > gl_options.max_rect_dimension)
            gl_options.max_movie_dimension = gl_options.max_rect_dimension;
    }

    /*if (gl_options.pot_video_textures) {
        BOOST_LOG_TRIVIAL(info) << "Forcing POT video textures";
    } else {
        BOOST_LOG_TRIVIAL(trace) << "Using NPOT video textures";
    }*/
    // Removing gl_options soon
    gl_options.smooth_shade        = game_options.SmoothShade;
    gl_options.mipmap 		   = game_options.mipmapdetail;
    gl_options.compression         = game_options.texture_compression;
    gl_options.Multitexture        = game_options.reflection;
    gl_options.smooth_lines        = game_options.smooth_lines;
    gl_options.smooth_points       = game_options.smooth_points;

    gl_options.display_lists       = game_options.displaylists;
    gl_options.s3tc 		   = game_options.s3tc;
    gl_options.ext_clamp_to_edge   = game_options.ext_clamp_to_edge;
    gl_options.ext_clamp_to_border = game_options.ext_clamp_to_border;


    glClearColor( clearcol.r, clearcol.g, clearcol.b, clearcol.a );
    winsys_set_reshape_func( Reshape );
    initfov();
    glShadeModel( GL_SMOOTH );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    glShadeModel( GL_SMOOTH );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );
    if (gl_options.wireframe)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    if (gl_options.smooth_shade == 0)
        glShadeModel( GL_FLAT );
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0.0 );

    init_opengl_extensions();
    GFXInitTextureManager();
    if (gl_options.Multitexture)
        GFXActiveTexture( 0 );
    glEnable( GL_TEXTURE_2D );          //use two-dimensional texturing
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

#if defined (IRIX)
    glEnable( GL_SHARED_TEXTURE_PALETTE_EXT );
#endif
    if ( vsExtensionSupported( "GL_EXT_color_table" ) || vsExtensionSupported( "GL_EXT_shared_texture_palette" ) ) {
        gl_options.PaletteExt = 1;
        //BOOST_LOG_TRIVIAL(trace) << "OpenGL::EXTColorTable supported";
    } else {
        gl_options.PaletteExt = 0;
        //BOOST_LOG_TRIVIAL(debug) << "OpenGL::EXTColorTable unsupported";
    }
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    if (gl_options.Multitexture) {
        for (int i = 1; i < 4; ++i) {
            GFXActiveTexture( i );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
            GFXTextureEnv( i, GFXADDTEXTURE );
            glPixelStorei( GL_UNPACK_SKIP_ROWS, 0 );
            glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0 );
            glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
            glPixelStorei( GL_UNPACK_SWAP_BYTES, 0 );
            // Choose cubemap or spheremap coordinates.
            if (i == 1) {
#ifdef NV_CUBE_MAP
                GFXToggleTexture( true, 1, CUBEMAP );
                GFXTextureCoordGenMode( 1, CUBE_MAP_GEN, NULL, NULL );
#else
                const float tempo[4] = {1, 0, 0, 0};
                GFXToggleTexture( true, 1, TEXTURE2D );
                GFXTextureCoordGenMode( 1, SPHERE_MAP_GEN, tempo, tempo );
#endif
            }
        }
    }
    GFXActiveTexture( 0 );
    glClearDepth( 1 );
    glEnable( GL_BLEND );
    glDisable( GL_ALPHA_TEST );
    GFXBlendMode( ONE, ZERO );

    glColor3f( 0, 0, 0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();     //set all matricies to identity
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    glEnable( GL_LIGHTING );

    glDisable( GL_NORMALIZE );

    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_POINT_SMOOTH );

    int con;
    GFXCreateLightContext( con );
    //glutSetCursor(GLUT_CURSOR_NONE);
    /* Avoid scrambled screen on startup - Twice, for triple buffering */
    if ( game_options.ClearOnStartup ) {
        glClear( GL_COLOR_BUFFER_BIT );
        winsys_swap_buffers();
        glClear( GL_COLOR_BUFFER_BIT );
        winsys_swap_buffers();
    }
    glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
    winsys_show_cursor( false );
}

#if defined (IRIX)
#include "lin_time.h"

/*
** Update the game counter, generate a redisplay request.
*/
void idle_loop( void )
{
    UpdateTime();
    glutPostRedisplay();
}

/*
** Install idle loop only if window is visible.
*/
void visible( int vis )
{
    glutIdleFunc( vis == GLUT_VISIBLE ? idle_loop : NULL );
}

/*
** Install the display and visibility callback functions,
** start the main loop.
*/
void GFXLoop( void (*main_loop)( void ) )
{
    glutDisplayFunc( main_loop );
    glutVisibilityFunc( visible );
    static bool are_we_looping = false;
    ///so we can call this function multiple times
    if (!are_we_looping) {
        are_we_looping = true;
        glutMainLoop();
    }
}
#else

void GFXLoop( void main_loop() )
{
    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );

    //glutDisplayFunc(main_loop);
    //glutIdleFunc (main_loop);
    static bool are_we_looping = false;
    /// so we can call this function multiple times to change the display and idle functions
    if (!are_we_looping) {
        are_we_looping = true;
        winsys_process_events();
    }
}
#endif

void GFXShutdown()
{
    extern void GFXDestroyAllLights();

    GFXDestroyAllTextures();
    GFXDestroyAllLights();
    if (gl_options.fullscreen)
        winsys_shutdown();
}
