/*
 * gl_init.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
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
#include "vs_logging.h"

#if !defined (_WIN32) && !defined (__CYGWIN__)

#include <stdlib.h>

#else
#ifndef NOMINMAX
#define NOMINMAX
#endif //tells VCC not to generate min/max macros
// #if defined(__APPLE__) && defined(__MACH__)
// #include <GL/glew.h>
// #else
#define GL_TEXTURE_CUBE_MAP_SEAMLESS_ARB 0x884F
// #endif
#include <windows.h>
#endif
#define GL_GLEXT_PROTOTYPES 1
// See https://github.com/vegastrike/Vega-Strike-Engine-Source/pull/851#discussion_r1589254766
#if defined(__APPLE__) && defined(__MACH__)
#   include <gl.h>
#   include <glext.h>
#   include <dlfcn.h>
#else
#   include <gl.h>
#   include <glext.h>
#endif
#ifdef GL_EXT_compiled_vertex_array
# ifndef PFNGLLOCKARRAYSEXTPROC
#  undef GL_EXT_compiled_vertex_array
# endif
#endif
#if !defined (IRIX)
//typedef void (APIENTRY * PFNGLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
//typedef void (APIENTRY * PFNGLUNLOCKARRAYSEXTPROC) (void);

#if !defined (WIN32) && !defined (__HAIKU__) && !defined (__APPLE__) && !defined (MACOSX)
    # define GLX_GLXEXT_PROTOTYPES 1
    # define GLX_GLXEXT_LEGACY 1
    # include <GL/glx.h>
    # include <glext.h>
#endif
#endif

#include <stdio.h>
#include "gl_init.h"
#define WINDOW_TITLE "Vega Strike " VERSION

#if defined (CG_SUPPORT)
#include "cg_global.h"
#endif

PFNGLBINDBUFFERARBPROC glBindBufferARB_p = nullptr;
PFNGLGENBUFFERSPROC glGenBuffersARB_p = nullptr;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB_p = nullptr;
PFNGLBUFFERDATAARBPROC glBufferDataARB_p = nullptr;
PFNGLMAPBUFFERARBPROC glMapBufferARB_p = nullptr;
PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB_p = nullptr;

PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB_p = nullptr;
PFNGLCLIENTACTIVETEXTUREARBPROC glActiveTextureARB_p = nullptr;
PFNGLCOLORTABLEEXTPROC glColorTable_p = nullptr;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB_p = nullptr;
PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB_p = nullptr;

PFNGLLOCKARRAYSEXTPROC glLockArraysEXT_p = nullptr;
PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT_p = nullptr;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D_p = nullptr;
PFNGLMULTIDRAWARRAYSEXTPROC glMultiDrawArrays_p = nullptr;
PFNGLMULTIDRAWELEMENTSEXTPROC glMultiDrawElements_p = nullptr;

PFNGLGETSHADERIVPROC glGetShaderiv_p = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv_p = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_p = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_p = nullptr;
PFNGLCREATESHADERPROC glCreateShader_p = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource_p = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader_p = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram_p = nullptr;
PFNGLATTACHSHADERPROC glAttachShader_p = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram_p = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram_p = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_p = nullptr;
PFNGLUNIFORM1FPROC glUniform1f_p = nullptr;
PFNGLUNIFORM2FPROC glUniform2f_p = nullptr;
PFNGLUNIFORM3FPROC glUniform3f_p = nullptr;
PFNGLUNIFORM4FPROC glUniform4f_p = nullptr;

PFNGLUNIFORM1IPROC glUniform1i_p = nullptr;
PFNGLUNIFORM2IPROC glUniform2i_p = nullptr;
PFNGLUNIFORM3IPROC glUniform3i_p = nullptr;
PFNGLUNIFORM4IPROC glUniform4i_p = nullptr;

PFNGLUNIFORM1FVPROC glUniform1fv_p = nullptr;
PFNGLUNIFORM2FVPROC glUniform2fv_p = nullptr;
PFNGLUNIFORM3FVPROC glUniform3fv_p = nullptr;
PFNGLUNIFORM4FVPROC glUniform4fv_p = nullptr;

PFNGLUNIFORM1IVPROC glUniform1iv_p = nullptr;
PFNGLUNIFORM2IVPROC glUniform2iv_p = nullptr;
PFNGLUNIFORM3IVPROC glUniform3iv_p = nullptr;
PFNGLUNIFORM4IVPROC glUniform4iv_p = nullptr;

PFNGLDELETESHADERPROC glDeleteShader_p = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram_p = nullptr;

typedef void ( *(*get_gl_proc_fptr_t)(const GLubyte *))();
#ifdef _WIN32
    typedef char * GET_GL_PTR_TYP;
    #define GET_GL_PROC wglGetProcAddress
#else
    #if defined(__HAIKU__)
        typedef char * GET_GL_PTR_TYP;
        #define GET_GL_PROC glutGetProcAddress
    #elif defined (__APPLE__) && defined (__MACH__)
        typedef const char * GET_GL_PTR_TYP;
        void * vega_dlsym(GET_GL_PTR_TYP function_name) {
            return dlsym(RTLD_SELF, function_name);
        }
        #define GET_GL_PROC vega_dlsym
    #else
        typedef GLubyte *GET_GL_PTR_TYP;
        #define GET_GL_PROC glXGetProcAddressARB
    #endif
#endif

#if defined (CG_SUPPORT)
CG_Cloak *cloak_cg = new CG_Cloak();
#endif

/* CENTRY */
int vsExtensionSupported(const char *extension) {
    static const GLubyte *extensions = nullptr;
    const GLubyte *start = nullptr;
    GLubyte *where = nullptr;
    GLubyte *terminator = nullptr;

    /* Extension names should not have spaces. */
    where = (GLubyte *) strchr(extension, ' ');
    if (where || *extension == '\0') {
        return 0;
    }
    if (!extensions) {
        extensions = glGetString(GL_EXTENSIONS);
        if (!extensions) {
            VS_LOG_AND_FLUSH(serious_warning, "glGetString(GL_EXTENSIONS) returned NULL!");
            return false;
        }
    }
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
        where = (GLubyte *) strstr((const char *) start, extension);
        if (!where) {
            break;
        }
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

bool vsVendorMatch(const char *vendor) {
    static const GLubyte *_glvendor = NULL;

    if (_glvendor == NULL) {
        _glvendor = glGetString(GL_VENDOR);
        if (_glvendor != NULL) {
            VS_LOG(info, (boost::format("OpenGL Vendor: %1%") % (const char *) _glvendor));
        }
    }

    if (_glvendor != NULL) {
        // NOTE: Don't be fooled by substrings within words
        //      Should match whole-words ONLY

        static const std::string glvendor = strtolower(std::string((const char *) _glvendor));
        static const std::string::size_type glvendor_sz = glvendor.length();
        std::string svendor = strtolower(vendor);
        std::string::size_type svendor_sz = svendor.length();
        std::string::size_type pos = glvendor.find(svendor);

        if (pos == std::string::npos) {
            return false;
        }

        if ((pos > 0) && isalnum(glvendor[pos - 1])) {
            return false;
        }

        if ((pos + svendor_sz) < glvendor_sz && isalnum(glvendor[pos + svendor_sz])) {
            return false;
        }

        return true;
    } else {
        return false;
    }
}

void init_opengl_extensions() {
    const unsigned char *extensions = glGetString(GL_EXTENSIONS);

    if (extensions) {
        VS_LOG(trace, (boost::format("OpenGL Extensions supported: %1%") % extensions));
    } else {
        VS_LOG(serious_warning, "OpenGL Extensions supported: none!");
    }

#ifndef NO_COMPILEDVERTEXARRAY_SUPPORT
    if (vsExtensionSupported("GL_EXT_compiled_vertex_array")
            && vs_options::instance().LockVertexArrays) {
#if defined(__APPLE__) && defined(__MACH__)
#ifndef __APPLE_PANTHER_GCC33_CLI__
#if defined (glLockArraysEXT) && defined (glUnlockArraysEXT)
        glLockArraysEXT_p   = &glLockArraysEXT;
        glUnlockArraysEXT_p = &glUnlockArraysEXT;
#else
        glLockArraysEXT_p   = nullptr;
        glUnlockArraysEXT_p = nullptr;
#endif
#else
        glLockArraysEXT_p = (PFNGLLOCKARRAYSEXTPROC)(dlsym(RTLD_SELF, "glLockArraysEXT"));
        glUnlockArraysEXT_p = (PFNGLUNLOCKARRAYSEXTPROC)(dlsym(RTLD_SELF, "glUnlockArraysEXT"));
#endif /*__APPLE_PANTHER_GCC33_CLI__*/
#else
        glLockArraysEXT_p = (PFNGLLOCKARRAYSEXTPROC)
                GET_GL_PROC((GET_GL_PTR_TYP) "glLockArraysEXT");
        glUnlockArraysEXT_p = (PFNGLUNLOCKARRAYSEXTPROC)
                GET_GL_PROC((GET_GL_PTR_TYP) "glUnlockArraysEXT");
#endif
        VS_LOG(trace, "OpenGL::GL_EXT_compiled_vertex_array supported");
    } else {
#if defined(__APPLE__) && defined(__MACH__)
        glLockArraysEXT_p = nullptr;
        glUnlockArraysEXT_p = nullptr;
#endif
        VS_LOG(debug, "OpenGL::GL_EXT_compiled_vertex_array unsupported");
    }
#endif
#if defined (__MACOSX__)
    if (vsExtensionSupported("GL_EXT_multi_draw_arrays")) {
        glMultiDrawArrays_p = (PFNGLMULTIDRAWARRAYSEXTPROC)
                GET_GL_PROC((GET_GL_PTR_TYP) "glMultiDrawArraysEXT");
        glMultiDrawElements_p = (PFNGLMULTIDRAWELEMENTSEXTPROC)
                GET_GL_PROC((GET_GL_PTR_TYP) "glMultiDrawElementsEXT");
        VS_LOG(trace, "OpenGL::GL_EXT_multi_draw_arrays supported");
    } else {
        glMultiDrawArrays_p = nullptr;
        glMultiDrawElements_p = nullptr;
        VS_LOG(debug, "OpenGL::GL_EXT_multi_draw_arrays unsupported");
    }
#else
    if (vsExtensionSupported("GL_EXT_multi_draw_arrays")) {
        glMultiDrawArrays_p = (PFNGLMULTIDRAWARRAYSEXTPROC)
                GET_GL_PROC((GET_GL_PTR_TYP) "glMultiDrawArraysEXT");
        glMultiDrawElements_p = (PFNGLMULTIDRAWELEMENTSEXTPROC)
                GET_GL_PROC((GET_GL_PTR_TYP) "glMultiDrawElementsEXT");
        VS_LOG(trace, "OpenGL::GL_EXT_multi_draw_arrays supported");
    } else {
        glMultiDrawArrays_p = nullptr;
        glMultiDrawElements_p = nullptr;
        VS_LOG(debug, "OpenGL::GL_EXT_multi_draw_arrays unsupported");
    }
#endif

#ifndef NO_VBO_SUPPORT
    glBindBufferARB_p = (PFNGLBINDBUFFERARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glBindBuffer");
    glGenBuffersARB_p = (PFNGLGENBUFFERSPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glGenBuffers");
    glDeleteBuffersARB_p = (PFNGLDELETEBUFFERSARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glDeleteBuffers");
    glBufferDataARB_p = (PFNGLBUFFERDATAARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glBufferData");
    glMapBufferARB_p = (PFNGLMAPBUFFERARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glMapBuffer");
    glUnmapBufferARB_p = (PFNGLUNMAPBUFFERARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUnmapBuffer");
#endif

    glColorTable_p = (PFNGLCOLORTABLEEXTPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glColorTableEXT");
    glMultiTexCoord2fARB_p = (PFNGLMULTITEXCOORD2FARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glMultiTexCoord2fARB");
    glMultiTexCoord4fARB_p = (PFNGLMULTITEXCOORD4FARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glMultiTexCoord4fARB");
    glClientActiveTextureARB_p =
            (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glClientActiveTextureARB");
    glActiveTextureARB_p = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glActiveTextureARB");
    if (!glMultiTexCoord2fARB_p) {
        glMultiTexCoord2fARB_p = (PFNGLMULTITEXCOORD2FARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glMultiTexCoord2fEXT");
    }
    if (!glMultiTexCoord4fARB_p) {
        glMultiTexCoord4fARB_p = (PFNGLMULTITEXCOORD4FARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glMultiTexCoord4fEXT");
    }
    if (!glClientActiveTextureARB_p) {
        glClientActiveTextureARB_p =
                (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glClientActiveTextureEXT");
    }
    if (!glActiveTextureARB_p) {
        glActiveTextureARB_p = (PFNGLCLIENTACTIVETEXTUREARBPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glActiveTextureEXT");
    }
    if (!glCompressedTexImage2D_p) {
        glCompressedTexImage2D_p =
                (PFNGLCOMPRESSEDTEXIMAGE2DPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glCompressedTexImage2D");
    }
    if (!glGetShaderiv_p) {
        glGetShaderiv_p = (PFNGLGETSHADERIVPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glGetShaderiv");
    }
    if (!glGetProgramiv_p) {
        glGetProgramiv_p = (PFNGLGETPROGRAMIVPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glGetProgramiv");
    }
    if (!glGetShaderInfoLog_p) {
        glGetShaderInfoLog_p = (PFNGLGETSHADERINFOLOGPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glGetShaderInfoLog");
    }
    if (!glGetProgramInfoLog_p) {
        glGetProgramInfoLog_p = (PFNGLGETPROGRAMINFOLOGPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glGetProgramInfoLog");
    }
    if (!glCreateShader_p) {
        glCreateShader_p = (PFNGLCREATESHADERPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glCreateShader");
    }
    if (!glCreateProgram_p) {
        glCreateProgram_p = (PFNGLCREATEPROGRAMPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glCreateProgram");
    }
    if (!glShaderSource_p) {
        glShaderSource_p = (PFNGLSHADERSOURCEPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glShaderSource");
    }
    if (!glCompileShader_p) {
        glCompileShader_p = (PFNGLCOMPILESHADERPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glCompileShader");
    }
    if (!glAttachShader_p) {
        glAttachShader_p = (PFNGLATTACHSHADERPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glAttachShader");
    }
    if (!glLinkProgram_p) {
        glLinkProgram_p = (PFNGLLINKPROGRAMPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glLinkProgram");
    }
    if (!glUseProgram_p) {
        glUseProgram_p = (PFNGLUSEPROGRAMPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUseProgram");
    }
    if (!glGetUniformLocation_p) {
        glGetUniformLocation_p = (PFNGLGETUNIFORMLOCATIONPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glGetUniformLocation");
    }
    if (!glUniform1f_p) {
        glUniform1f_p = (PFNGLUNIFORM1FPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform1f");
    }
    if (!glUniform2f_p) {
        glUniform2f_p = (PFNGLUNIFORM2FPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform2f");
    }
    if (!glUniform3f_p) {
        glUniform3f_p = (PFNGLUNIFORM3FPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform3f");
    }
    if (!glUniform4f_p) {
        glUniform4f_p = (PFNGLUNIFORM4FPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform4f");
    }
    if (!glUniform1i_p) {
        glUniform1i_p = (PFNGLUNIFORM1IPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform1i");
    }
    if (!glUniform2i_p) {
        glUniform2i_p = (PFNGLUNIFORM2IPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform2i");
    }
    if (!glUniform3i_p) {
        glUniform3i_p = (PFNGLUNIFORM3IPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform3i");
    }
    if (!glUniform4i_p) {
        glUniform4i_p = (PFNGLUNIFORM4IPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform4i");
    }
    if (!glUniform1fv_p) {
        glUniform1fv_p = (PFNGLUNIFORM1FVPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform1fv");
    }
    if (!glUniform2fv_p) {
        glUniform2fv_p = (PFNGLUNIFORM2FVPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform2fv");
    }
    if (!glUniform3fv_p) {
        glUniform3fv_p = (PFNGLUNIFORM3FVPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform3fv");
    }
    if (!glUniform4fv_p) {
        glUniform4fv_p = (PFNGLUNIFORM4FVPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform4fv");
    }
    if (!glUniform1iv_p) {
        glUniform1iv_p = (PFNGLUNIFORM1IVPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform1iv");
    }
    if (!glUniform2iv_p) {
        glUniform2iv_p = (PFNGLUNIFORM2IVPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform2iv");
    }
    if (!glUniform3iv_p) {
        glUniform3iv_p = (PFNGLUNIFORM3IVPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform3iv");
    }
    if (!glUniform4iv_p) {
        glUniform4iv_p = (PFNGLUNIFORM4IVPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glUniform4iv");
    }
    if (!glDeleteShader_p) {
        glDeleteShader_p = (PFNGLDELETESHADERPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glDeleteShader");
    }
    if (!glDeleteProgram_p) {
        glDeleteProgram_p = (PFNGLDELETEPROGRAMPROC) GET_GL_PROC((GET_GL_PTR_TYP) "glDeleteProgram");
    }
    //fixme

#ifdef GL_FOG_DISTANCE_MODE_NV
    if (vsExtensionSupported("GL_NV_fog_distance")) {
        VS_LOG(trace, "OpenGL::Accurate Fog Distance supported");
        switch (vs_options::instance().fogdetail) {
            case 0:
                glFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_PLANE_ABSOLUTE_NV);
                break;
            case 1:
                glFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_PLANE);
                break;
            case 2:
                glFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
                break;
        }
    } else {
#endif
        VS_LOG(debug, "OpenGL::Accurate Fog Distance unsupported");
#ifdef GL_FOG_DISTANCE_MODE_NV
    }
#endif
    if (vsExtensionSupported("GL_ARB_texture_compression")) {
        VS_LOG(trace, "OpenGL::Generic Texture Compression supported");
    } else {
        VS_LOG(info, "OpenGL::Generic Texture Compression unsupported");
        gl_options.compression = 0;
    }
    if (vsExtensionSupported("GL_EXT_texture_compression_s3tc")) {
        VS_LOG(trace, "OpenGL::S3TC Texture Compression supported");
        //should be true;
    } else {
        gl_options.s3tc = false;
        VS_LOG(info, "OpenGL::S3TC Texture Compression unsupported");
    }
    if ((glMultiTexCoord2fARB_p && glMultiTexCoord4fARB_p && glClientActiveTextureARB_p && glActiveTextureARB_p)
            && (vsExtensionSupported("GL_ARB_multitexture") || vsExtensionSupported("GL_EXT_multitexture"))) {
        GLint multitex = gl_options.Multitexture;
        glGetIntegerv(GL_MAX_TEXTURE_UNITS, &multitex);
        if (multitex > 1) {
            gl_options.Multitexture = multitex;
        } else {
            gl_options.Multitexture = 0;
        }
        //gl_options.Multitexture = 1*gl_options.Multitexture;//might be zero by input
        if (gl_options.Multitexture) {
            VS_LOG(trace, (boost::format("OpenGL::Multitexture supported (%1% units)") % gl_options.Multitexture));
        }
    } else {
        gl_options.Multitexture = 0;
        VS_LOG(debug, "OpenGL::Multitexture unsupported");
    }
    if (vsExtensionSupported("GL_ARB_texture_cube_map") || vsExtensionSupported("GL_EXT_texture_cube_map")) {
        gl_options.cubemap = 1;
        VS_LOG(trace, "OpenGL::TextureCubeMapExt supported");
    } else {
        gl_options.cubemap = 0;
        VS_LOG(info, "OpenGL::TextureCubeMapExt unsupported");
    }
    if (vsExtensionSupported("GL_EXT_texture_edge_clamp") || vsExtensionSupported("GL_SGIS_texture_edge_clamp")) {
        VS_LOG(trace, "OpenGL::S3TC Texture Clamp-to-Edge supported");
        // should be true
    } else {
        gl_options.ext_clamp_to_edge = false;
        VS_LOG(info, "OpenGL::S3TC Texture Clamp-to-Edge unsupported");
    }
    if (vsExtensionSupported("GL_ARB_texture_border_clamp") || vsExtensionSupported("GL_SGIS_texture_border_clamp")) {
        VS_LOG(trace, "OpenGL::S3TC Texture Clamp-to-Border supported");
        // should be true
    } else {
        gl_options.ext_clamp_to_border = false;
        VS_LOG(info, "OpenGL::S3TC Texture Clamp-to-Border unsupported");
    }
    if (vsExtensionSupported("GL_ARB_framebuffer_sRGB") || vsExtensionSupported("GL_EXT_framebuffer_sRGB")) {
        VS_LOG(trace, "OpenGL::sRGB Framebuffer supported");
        GLboolean srgbCapable = true;

        if (vsExtensionSupported("GL_EXT_framebuffer_sRGB")) {
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
            VS_LOG(info, "OpenGL::sRGB Framebuffer unsupported by visual");
            gl_options.ext_srgb_framebuffer = false;
        }
    } else {
        gl_options.ext_srgb_framebuffer = false;
        VS_LOG(info, "OpenGL::sRGB Framebuffer unsupported");
    }
    if (vsExtensionSupported("GL_NV_fragment_program2")) {
        gl_options.nv_fp2 = true;
        VS_LOG(trace, "OpenGL::NV_fragment_program2 supported");
    } else {
        gl_options.nv_fp2 = false;
        VS_LOG(info, "OpenGL::NV_fragment_program2 unsupported");
    }
    if (GFXDefaultShaderSupported()) {
        if (gl_options.Multitexture < 16) {
            gl_options.Multitexture = 16;
        }
    }
    //only support shaders on ps_2_0 cards

    // some useful generic limits
    GLint max_vertices = -1, max_indices = -1;
    glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &max_indices);
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &max_vertices);
    gl_options.max_array_indices = max_indices;
    gl_options.max_array_vertices = max_vertices;

    VS_LOG(info, (boost::format("Max vertex array indices: %1%") % gl_options.max_array_indices));
    VS_LOG(info, (boost::format("Max vertex array vertices: %1%") % gl_options.max_array_vertices));
}

static void initfov() {
    g_game.fov = vs_options::instance().fov;
    g_game.aspect = vs_options::instance().aspect;
    g_game.znear = vs_options::instance().znear;
    g_game.zfar = vs_options::instance().zfar;
    g_game.detaillevel = vs_options::instance().ModelDetail;
    g_game.use_textures = vs_options::instance().UseTextures;
    g_game.use_ship_textures = vs_options::instance().UseShipTextures;
    g_game.use_planet_textures = vs_options::instance().UsePlanetTextures;
    g_game.use_logos = vs_options::instance().UseLogos;
    g_game.use_sprites = vs_options::instance().UseVSSprites;
    g_game.use_animations = vs_options::instance().UseAnimations;
    g_game.use_videos = vs_options::instance().UseVideos;

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

static void Reshape(int x, int y) {
    g_game.x_resolution = x;
    g_game.y_resolution = y;
    VS_LOG(trace, (boost::format("Reshaping %1% %2%") % x % y));
}

extern void GFXInitTextureManager();

void GFXInit(int argc, char **argv) {
    char vsname[12] = "Vega Strike";
    char vsicon[9] = "vega.ico";
    winsys_init(&argc, argv, &vsname[0], &vsicon[0]);

    glViewport(0, 0, g_game.x_resolution, g_game.y_resolution);
    static GFXColor clearcol = vs_config->getColor("space_background");;
    gl_options.wireframe = vs_options::instance().use_wireframe;
    gl_options.max_texture_dimension = vs_options::instance().max_texture_dimension;
    gl_options.max_movie_dimension = vs_options::instance().max_movie_dimension;
    bool textsupported =
            (vsExtensionSupported("GL_ARB_texture_non_power_of_two") || vsExtensionSupported("GL_ARB_texture_rectangle")
                    || vsExtensionSupported("GL_NV_texture_rectangle")) ? "true" : "false";

    gl_options.rect_textures = vs_options::instance().rect_textures ? true : textsupported;

    if (gl_options.rect_textures) {
        VS_LOG(trace, "RECT textures supported");

        // Fetch max rect textue dimension
        GLint max_rect_dimension = 65535;
        glGetIntegerv(
                GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB,
                &max_rect_dimension);

        gl_options.max_rect_dimension = max_rect_dimension;
        VS_LOG(trace, (boost::format("RECT max texture dimension: %1%") % max_rect_dimension));
    }

    bool vidsupported = (gl_options.rect_textures
                         || (vsExtensionSupported("GL_ARB_texture_non_power_of_two") && vsVendorMatch("nvidia")));

    gl_options.pot_video_textures = vs_options::instance().pot_video_textures ? true : vidsupported;

    if (!gl_options.pot_video_textures && gl_options.rect_textures) {
        // Enforce max rect texture for movies, which use them
        if (gl_options.max_movie_dimension > gl_options.max_rect_dimension) {
            gl_options.max_movie_dimension = gl_options.max_rect_dimension;
        }
    }

    /*if (gl_options.pot_video_textures) {
        VS_LOG(info, "Forcing POT video textures");
    } else {
        VS_LOG(trace, "Using NPOT video textures");
    }*/
    // Removing gl_options soon
    gl_options.smooth_shade = vs_options::instance().SmoothShade;
    gl_options.mipmap = vs_options::instance().mipmapdetail;
    gl_options.compression = vs_options::instance().texture_compression;
    gl_options.Multitexture = vs_options::instance().reflection;
    gl_options.smooth_lines = vs_options::instance().smooth_lines;
    gl_options.smooth_points = vs_options::instance().smooth_points;

    gl_options.display_lists = vs_options::instance().displaylists;
    gl_options.s3tc = vs_options::instance().s3tc;
    gl_options.ext_clamp_to_edge = vs_options::instance().ext_clamp_to_edge;
    gl_options.ext_clamp_to_border = vs_options::instance().ext_clamp_to_border;

    glClearColor(clearcol.r, clearcol.g, clearcol.b, clearcol.a);
    winsys_set_reshape_func(Reshape);
    initfov();
    glShadeModel(GL_SMOOTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    if (gl_options.wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (gl_options.smooth_shade == 0) {
        glShadeModel(GL_FLAT);
    }
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0);

    init_opengl_extensions();
    GFXInitTextureManager();
    if (gl_options.Multitexture) {
        GFXActiveTexture(0);
    }
    glEnable(GL_TEXTURE_2D);          //use two-dimensional texturing
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

#if defined (IRIX)
    glEnable( GL_SHARED_TEXTURE_PALETTE_EXT );
#endif
    if (vsExtensionSupported("GL_EXT_color_table") || vsExtensionSupported("GL_EXT_shared_texture_palette")) {
        gl_options.PaletteExt = 1;
        VS_LOG(trace, "OpenGL::EXTColorTable supported");
    } else {
        gl_options.PaletteExt = 0;
        VS_LOG(debug, "OpenGL::EXTColorTable unsupported");
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    if (gl_options.Multitexture) {
        for (int i = 1; i < 4; ++i) {
            GFXActiveTexture(i);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            GFXTextureEnv(i, GFXADDTEXTURE);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
            // Choose cubemap or spheremap coordinates.
            if (i == 1) {
#ifdef NV_CUBE_MAP
                GFXToggleTexture(true, 1, CUBEMAP);
                GFXTextureCoordGenMode(1, CUBE_MAP_GEN, NULL, NULL);
#else
                const float tempo[4] = {1, 0, 0, 0};
                GFXToggleTexture( true, 1, TEXTURE2D );
                GFXTextureCoordGenMode( 1, SPHERE_MAP_GEN, tempo, tempo );
#endif
            }
        }
    }
    GFXActiveTexture(0);
    glClearDepth(1);
    glEnable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    GFXBlendMode(ONE, ZERO);

    glColor3f(0, 0, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();     //set all matricies to identity
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glEnable(GL_LIGHTING);

    glDisable(GL_NORMALIZE);

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);

    int con;
    GFXCreateLightContext(con);
    //glutSetCursor(GLUT_CURSOR_NONE);
    /* Avoid scrambled screen on startup - Twice, for triple buffering */
    if (vs_options::instance().ClearOnStartup) {
        glClear(GL_COLOR_BUFFER_BIT);
        winsys_swap_buffers();
        glClear(GL_COLOR_BUFFER_BIT);
        winsys_swap_buffers();
    }
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    winsys_show_cursor(false);
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

void GFXLoop(void main_loop()) {
    winsys_set_display_func(main_loop);
    winsys_set_idle_func(main_loop);

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

void GFXShutdown() {
    extern void GFXDestroyAllLights();

    GFXDestroyAllTextures();
    GFXDestroyAllLights();
    if (gl_options.fullscreen) {
        winsys_shutdown();
    }
}
