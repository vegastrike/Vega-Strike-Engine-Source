/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
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
#ifndef _GL_GLOBALS_H_
#define _GL_GLOBALS_H_
#include <queue>
const static bool GFX_BUFFER_MAP_UNMAP = false;
/* Hack for multitexture on Mac, here and in gl_init, ifdefined - griff */
//Moved this because defining GL_EXT_texture... doesn't work under Jaguar
#ifdef __APPLE_PANTHER_GCC33_CLI__
    #define GL_EXT_texture_env_combine 1
#endif /* __APPLE_PANTHER_GCC33_CLI__ */
#undef __APPLE_PANTHER_GCC33_CLI__
#if defined (__GNUC__) && defined (__APPLE__)
    #if (__GNUC__ == 3 && __GNUC_MINOR__ > 2) || (__GNUC__ > 3)
        #define __APPLE_PANTHER_GCC33_CLI__
    #endif
#endif

#ifndef GFXSTAT
#define GFXSTAT
#ifdef STATS_QUEUE
#include <time.h>

struct GFXStats
{
    int    drawnTris;
    int    drawnQuads;
    int    drawnPoints;
    time_t ztime;
    GFXStats()
    {
        drawnTris = drawnQuads = drawnPoints = 0;
        time( &ztime );
    }
    GFXStats( int tri, int quad, int point )
    {
        drawnTris   = tri;
        drawnQuads  = quad;
        drawnPoints = point;
    }
    GFXStats&operator+=( const GFXStats &rval )
    {
        drawnTris   += rval.drawnTris;
        drawnQuads  += rval.drawnQuads;
        drawnPoints += rval.drawnPoints;
        return *this;
    }
    int total()
    {
        return drawnTris*3+drawnQuads*4+drawnPoints;
    }
    int elapsedTime()
    {
        time_t t;
        time( &t );
        return (int) (t-ztime);
    }
};
#endif
#endif

#define MAX_NUM_LIGHTS 4
#define MAX_NUM_MATERIAL 4
#define TEXTURE_CUBE_MAP_ARB 0x8513

//extern Matrix model;
//extern Matrix view;
#if defined (__CYGWIN__)
#define GL_EXT_color_subtable 1
#endif

#ifndef _WIN32
//#define GL_GLEXT_PROTOTYPES

#endif
#if defined (_WIN32) || defined (__CYGWIN__)
#ifndef NOMINMAX
#define NOMINMAX
#endif //tells VCC not to generate min/max macros
#include <windows.h>
#include <GL/gl.h>
#endif
#if defined (__APPLE__) || defined (MACOSX)
    #include <GLUT/glut.h>
//#if defined( GL_INIT_CPP) || defined( GL_MISC_CPP) || defined( GL_STATE_CPP)
#if defined (GL_ARB_vertex_program) && defined (GL_ARB_fragment_program)
#define OSX_AT_LEAST_10_4
#else
#define OSX_LOWER_THAN_10_4
#endif
#define GL_GLEXT_PROTOTYPES
//#endif
    #include <OpenGL/glext.h>
#else
#define __glext_h_
    #include <GL/glut.h>
#include "gl_undefined_extensions.h"
#undef __glext_h_

    #include <GL/glext.h>
#endif
#ifdef _WIN32
#define GL_TEXTURE0_ARB 0x84C0
#define GL_TEXTURE1_ARB 0x84C1
#define GL_TEXTURE_CUBE_MAP_ARB 0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB 0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB 0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB 0x851C
#define GL_TEXTURE_CUBE_MAP_EXT 0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT 0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_EXT 0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT 0x851C
#endif

#if defined (__APPLE__)
#define glMultiDrawElements_p glMultiDrawElements
#define glMultiDrawArrays_p glMultiDrawArrays
#define glMultiTexCoord4fARB_p glMultiTexCoord4fARB
#define glMultiTexCoord2fARB_p glMultiTexCoord2fARB
#define glClientActiveTextureARB_p glClientActiveTextureARB
#define glActiveTextureARB_p glActiveTextureARB
#define glColorTable_p glColorTable
#define glCompressedTexImage2D_p glCompressedTexImage2D
#ifndef OSX_LOWER_THAN_10_4
#define glGetShaderiv_p glGetShaderiv
#define glGetProgramiv_p glGetProgramiv
#define glGetShaderInfoLog_p glGetShaderInfoLog
#define glGetProgramInfoLog_p glGetProgramInfoLog
#define glCreateShader_p glCreateShader
#define glShaderSource_p glShaderSource
#define glCompileShader_p glCompileShader
#define glCreateProgram_p glCreateProgram
#define glAttachShader_p glAttachShader
#define glLinkProgram_p glLinkProgram
#define glUseProgram_p glUseProgram
#define glGetUniformLocation_p glGetUniformLocation
#define glUniform1f_p glUniform1f
#define glUniform2f_p glUniform2f
#define glUniform3f_p glUniform3f
#define glUniform4f_p glUniform4f
#define glUniform1i_p glUniform1i
#define glUniform2i_p glUniform2i
#define glUniform3i_p glUniform3i
#define glUniform4i_p glUniform4i
#define glUniform1fv_p glUniform1fv
#define glUniform2fv_p glUniform2fv
#define glUniform3fv_p glUniform3fv
#define glUniform4fv_p glUniform4fv
#define glUniform1iv_p glUniform1iv
#define glUniform2iv_p glUniform2iv
#define glUniform3iv_p glUniform3iv
#define glUniform4iv_p glUniform4iv

#define glDeleteShader_p glDeleteShader
#define glDeleteProgram_p glDeleteProgram
#else
#define glGetShaderiv_p( a, b, c )
#define glGetProgramiv_p( a, b, c )
#define glGetShaderInfoLog_p( a, b, c, d )
#define glGetProgramInfoLog_p( a, b, c, d )
#define glCreateShader_p( a ) 0
#define glShaderSource_p( a, b, c, d )
#define glCompileShader_p( a )
#define glCreateProgram_p() 0
#define glAttachShader_p( a, b )
#define glLinkProgram_p( a )
#define glUseProgram_p( a )
#define glGetUniformLocation_p( a, b ) 0
#define glUniform1f_p( a, b )
#define glUniform2f_p( a, b, c )
#define glUniform3f_p( a, b, c, d )
#define glUniform4f_p( a, b, c, d, e )
#define glUniform1i_p( a, b )
#define glUniform2i_p( a, b, c )
#define glUniform3i_p( a, b, c, d )
#define glUniform4i_p( a, b, c, d, e )
#define glUniform1fv_p( a, b, c )
#define glUniform2fv_p( a, b, c )
#define glUniform3fv_p( a, b, c )
#define glUniform4fv_p( a, b, c )
#define glUniform1iv_p( a, b, c )
#define glUniform2iv_p( a, b, c )
#define glUniform3iv_p( a, b, c )
#define glUniform4iv_p( a, b, c )

#define glDeleteProgram_p glIsTexture

#endif
#if !defined (glLockArraysEXT) || !defined (glUnlockArraysEXT)
#define NO_COMPILEDVERTEXARRAY_SUPPORT
#endif

#ifndef NO_COMPILEDVERTEXARRAY_SUPPORT
#define glLockArraysEXT_p glLockArraysEXT
#define glUnlockArraysEXT_p glUnlockArraysEXT
#endif

#if !defined (glDeleteBuffersARB) || !defined (glGenBuffersARB) || !defined (glBindBuffersARB) || !defined (glMapBufferARB) \
    || !defined (glUnmapBufferARB)
#define NO_VBO_SUPPORT
#endif

#ifndef NO_VBO_SUPPORT
#define glDeleteBuffersARB_p glDeleteBuffersARB
#define glGenBuffersARB_p glGenBuffersARB
#define glBindBufferARB_p glBindBufferARB
#define glBufferDataARB_p glBufferDataARB
#define glMapBufferARB_p glMapBufferARB
#define glUnmapBufferARB_p glUnmapBufferARB
#endif

#else
extern PFNGLBINDBUFFERARBPROC          glBindBufferARB_p;
extern PFNGLGENBUFFERSARBPROC          glGenBuffersARB_p;
extern PFNGLDELETEBUFFERSARBPROC       glDeleteBuffersARB_p;
extern PFNGLBUFFERDATAARBPROC          glBufferDataARB_p;
extern PFNGLMAPBUFFERARBPROC           glMapBufferARB_p;
extern PFNGLUNMAPBUFFERARBPROC         glUnmapBufferARB_p;
extern PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2fARB_p;
extern PFNGLMULTITEXCOORD4FARBPROC     glMultiTexCoord4fARB_p;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB_p;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glActiveTextureARB_p;
extern PFNGLCOLORTABLEEXTPROC          glColorTable_p;
extern PFNGLLOCKARRAYSEXTPROC          glLockArraysEXT_p;
extern PFNGLMULTIDRAWARRAYSEXTPROC     glMultiDrawArrays_p;
extern PFNGLMULTIDRAWELEMENTSEXTPROC   glMultiDrawElements_p;
extern PFNGLUNLOCKARRAYSEXTPROC        glUnlockArraysEXT_p;
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC   glCompressedTexImage2D_p;
extern PFNGLGETSHADERIVPROC glGetShaderiv_p;
extern PFNGLGETPROGRAMIVPROC           glGetProgramiv_p;
extern PFNGLGETSHADERINFOLOGPROC       glGetShaderInfoLog_p;
extern PFNGLGETPROGRAMINFOLOGPROC      glGetProgramInfoLog_p;
extern PFNGLCREATESHADERPROC           glCreateShader_p;
extern PFNGLSHADERSOURCEPROC           glShaderSource_p;
extern PFNGLCOMPILESHADERPROC          glCompileShader_p;
extern PFNGLCREATEPROGRAMPROC          glCreateProgram_p;
extern PFNGLATTACHSHADERPROC           glAttachShader_p;
extern PFNGLLINKPROGRAMPROC glLinkProgram_p;
extern PFNGLUSEPROGRAMPROC glUseProgram_p;
extern PFNGLGETUNIFORMLOCATIONPROC     glGetUniformLocation_p;
extern PFNGLUNIFORM1FPROC     glUniform1f_p;
extern PFNGLUNIFORM2FPROC     glUniform2f_p;
extern PFNGLUNIFORM3FPROC     glUniform3f_p;
extern PFNGLUNIFORM4FPROC     glUniform4f_p;

extern PFNGLUNIFORM1IPROC     glUniform1i_p;
extern PFNGLUNIFORM2IPROC     glUniform2i_p;
extern PFNGLUNIFORM3IPROC     glUniform3i_p;
extern PFNGLUNIFORM4IPROC     glUniform4i_p;

extern PFNGLUNIFORM1FVPROC    glUniform1fv_p;
extern PFNGLUNIFORM2FVPROC    glUniform2fv_p;
extern PFNGLUNIFORM3FVPROC    glUniform3fv_p;
extern PFNGLUNIFORM4FVPROC    glUniform4fv_p;

extern PFNGLUNIFORM1IVPROC    glUniform1iv_p;
extern PFNGLUNIFORM2IVPROC    glUniform2iv_p;
extern PFNGLUNIFORM3IVPROC    glUniform3iv_p;
extern PFNGLUNIFORM4IVPROC    glUniform4iv_p;

extern PFNGLDELETESHADERPROC  glDeleteShader_p;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram_p;

#endif /* __APPLE_PANTHER_GCC33_CLI__ */

//extern int sharedcolortable;
#ifdef STATS_QUEUE
extern queue< GFXStats >statsqueue;
#endif

// Most of these will likely be folded into vs_options

typedef struct
{
    int  fullscreen;
    size_t  Multitexture;
    int  PaletteExt;
    int  display_lists;
    int  mipmap;    //0 = nearest 1 = linear 2 = mipmap
    int  color_depth;
    int  cubemap;
    int  compression;
    char wireframe;
    char smooth_shade;
    int  max_texture_dimension;
    int  max_movie_dimension;
    int  max_rect_dimension;
    unsigned int  max_array_indices;
    unsigned int  max_array_vertices;
    bool rect_textures;
    bool pot_video_textures;
    bool s3tc;
    bool ext_clamp_to_edge;
    bool ext_clamp_to_border;
    bool ext_srgb_framebuffer;
    bool nv_fp2; // NV_fragment_program2 signals the presence of texture2DLod on plain 1.10 GLSL
    bool smooth_lines;
    bool smooth_points;
} gl_options_t;
extern gl_options_t gl_options;

// rendering stats
extern int gl_vertices_this_frame;
extern int gl_batches_this_frame;

//Maximum number of things that can be returned in a pick operation
#define MAX_PICK 2048
#define GFX_SCALE 1./1024.
#endif

// Not all platforms define GL_TEXTURE_CUBE_MAP_EXT and friends
// Some platforms define _ARB variants,
// Some plafrorms define suffixless variants
#ifndef GL_TEXTURE_CUBE_MAP_EXT
    #ifdef GL_TEXTURE_CUBE_MAP
        #define GL_TEXTURE_CUBE_MAP_EXT GL_TEXTURE_CUBE_MAP
        #define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_X
        #define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT GL_TEXTURE_CUBE_MAP_NEGATIVE_X
        #define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_Y
        #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
        #define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_Z
        #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    #else 
        #ifdef GL_TEXTURE_CUBE_MAP_ARB
            #define GL_TEXTURE_CUBE_MAP_EXT GL_TEXTURE_CUBE_MAP_ARB
            #define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB
            #define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB
            #define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB
            #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB
            #define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB
            #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
        #endif
    #endif
#endif

#ifndef GL_TEXTURE_RECTANGLE_ARB
    #define GL_TEXTURE_RECTANGLE_ARB          0x84F5
    #define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB 0x84F8
#endif


// Not all platforms define GL_FRAMEBUFFER_SRGB stuff
#ifndef GL_FRAMEBUFFER_SRGB_EXT
    #define GL_FRAMEBUFFER_SRGB_EXT 0x8DB9
    #define GL_FRAMEBUFFER_SRGB_CAPABLE_EXT 0x8DBA
#endif

