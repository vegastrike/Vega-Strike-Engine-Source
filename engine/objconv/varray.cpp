/**
* varray.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <GL/glut.h>
#include <vector>
#undef GL_ARB_vertex_buffer_object
#include <GL/glext.h>
#include <stdio.h>
PFNGLBINDBUFFERARBPROC    glBindBufferARB_p    = 0;
PFNGLGENBUFFERSARBPROC    glGenBuffersARB_p    = 0;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB_p = 0;
PFNGLBUFFERDATAARBPROC    glBufferDataARB_p    = 0;
PFNGLMAPBUFFERARBPROC     glMapBufferARB_p     = 0;
PFNGLUNMAPBUFFERARBPROC   glUnmapBufferARB_p   = 0;

#ifndef _WIN32
#define glXGetProcAddress glXGetProcAddressARB
#include <GL/glx.h>
#endif

void DrawScene();
static void ReDisplay()
{
    glutPostRedisplay();
}
using namespace std;
static GLfloat colorArray[] = {
    1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, //Lancelot's favorite color
    0.0f, 0.0f, 0.0f,
    0.0f, 0.5f, 0.0f,
    1.0f, 1.0f, 1.0f
};

static GLfloat vertexArray[] = {
    0.5f, 0.1f, 0.0f,
    1.0f, 0.4f, 0.0f,
    0.9f, 1.0f, 0.0f,
    0.3f, 0.8f, 0.0f,
    0.1f, 0.5f, 0.0f
};
static GLuint  indices[] = {2, 3, 4, 0, 1};
#define shapesize 5
unsigned int   myrandmax = 1880881;

unsigned int myrand()
{
    static unsigned int seed = 31337;
    seed += 345676543;
    seed %= myrandmax;
    return seed;
}

static bool isPowTwo( unsigned int n )
{
    return ( n&(n-1) ) == 0;
}
template < class T >
vector< T >GetIndices( T size, size_t isize, size_t *memsize )
{
    vector< T >index( isize );
    *memsize = sizeof (T);
    for (size_t i = 0; i < isize; ++i)
        index[i] = ( ( (i/shapesize)*shapesize )+indices[i%shapesize] )%(size_t) size;
    return index;
}
float readbytes = 0;
class vbo
{
    GLuint vert;
    GLuint ind;
    size_t size;
    size_t isize;
    void BindBuf()
    {
        static GLuint cur_buffer = 0;
        if (1) {
            (*glBindBufferARB_p)(GL_ARRAY_BUFFER_ARB, vert);
            cur_buffer = vert;
        }
    }
    void BindInd()
    {
        static GLuint cur_buffer = 0;
        if (1) {
            (*glBindBufferARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB, ind);
            cur_buffer = ind;
        }
    }
public: vbo( size_t s, bool indexed, bool mutate )
    {
        vector< GLfloat >varray( s*3 );
        vert = ind = 0;
        size = s;
        if (indexed)
            isize = s+(size_t) ( s*( (float) myrand()/myrandmax ) );
        for (size_t i = 0; i < s*3; ++i)
            varray[i] = vertexArray[i%(shapesize*3)];
        (*glGenBuffersARB_p)(1, &vert);
        if (indexed)
            (*glGenBuffersARB_p)(1, &ind);
        BindBuf();
        (*glBufferDataARB_p)(GL_ARRAY_BUFFER_ARB,
                             size*3*sizeof (GLfloat),
                             &varray[0],
                             (mutate) ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB);
        if (indexed) {
            BindInd();
            size_t memsize = sizeof (GLubyte);
            if (s < 256) {
                vector< GLubyte >indices = GetIndices( (GLubyte) size, isize, &memsize );
                (*glBufferDataARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB,
                                     isize*memsize,
                                     &indices[0],
                                     (mutate) ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB);
            } else if (s < 65536) {
                GLushort LSize = s;
                vector< GLushort >indices = GetIndices( (GLushort) size, isize, &memsize );

                (*glBufferDataARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB,
                                     isize*memsize,
                                     &indices[0],
                                     (mutate) ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB);
            } else {
                GLuint LSize = s;
                vector< GLuint >indices = GetIndices( (GLuint) size, isize, &memsize );
                (*glBufferDataARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB,
                                     isize*memsize,
                                     &indices[0],
                                     (mutate) ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB);
            }
        }
    }
    void Draw()
    {
        BindBuf();
        if (ind)
            BindInd();
        glInterleavedArrays( GL_V3F, sizeof (GLfloat)*3, 0 );
        if (ind) {
            BindInd();
            char stride = ( size < 256 ? sizeof (GLubyte) : ( size < 65536 ? sizeof (GLushort) : sizeof (GLuint) ) );
            glDrawElements( GL_TRIANGLE_FAN, size, size
                            < 256 ? GL_UNSIGNED_BYTE : (size < 65536 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT), 0 );
        } else {
            glDrawArrays( GL_TRIANGLE_FAN, 0, size );
        }
    }
    void Write()
    {
        if (ind) {
            BindInd();
            (*glMapBufferARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_READ_ONLY_ARB);         //ignored
        }
        BindBuf();
        GLfloat *varray = (GLfloat*) (*glMapBufferARB_p)(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
        for (size_t i = 0; i < size*3; ++i)
            varray[i] = vertexArray[i%(shapesize*3)];
        if (ind) {
            BindInd();
            (*glUnmapBufferARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB);
        }
        BindBuf();
        (*glUnmapBufferARB_p)(GL_ARRAY_BUFFER_ARB);
    }
    void Read()
    {
        char const *tmpind;
        if (ind) {
            BindInd();
            tmpind = (char*) (*glMapBufferARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_READ_ONLY_ARB);
        }
        BindBuf();
        GLfloat const*const tmp = (GLfloat*) (*glMapBufferARB_p)(GL_ARRAY_BUFFER_ARB, GL_READ_ONLY_ARB);
        for (unsigned int i = 0; i < size; ++i) {
            readbytes += tmp[i*3];
            readbytes += tmp[i*3+1];
            readbytes += tmp[i*3+2];
            if (ind)
                readbytes += tmpind[i];
        }
        if (ind) {
            BindInd();
            (*glUnmapBufferARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB);
        }
        BindBuf();
        (*glUnmapBufferARB_p)(GL_ARRAY_BUFFER_ARB);
    }
    ~vbo()
    {
        (*glDeleteBuffersARB_p)(1, &vert);
        if (ind)
            (*glDeleteBuffersARB_p)(1, &ind);
    }
};
vector< vbo* >varrays( 33 );
static void keyboard( unsigned char key, int x, int y )
{
    switch (key)
    {
    case 27:
        exit( 0 );
    }
}
typedef void ( *(*get_gl_proc_fptr_t)(const GLubyte*) )();
#ifdef _WIN32
typedef char*GET_GL_PTR_TYP;
#define GET_GL_PROC wglGetProcAddress

#else
typedef GLubyte*GET_GL_PTR_TYP;
#define GET_GL_PROC glXGetProcAddress
#endif

int main( int argc, char *argv[] )
{
    GLsizei width, height;

    glutInit( &argc, argv );

    width  = glutGet( GLUT_SCREEN_WIDTH );
    height = glutGet( GLUT_SCREEN_HEIGHT );
    glutInitWindowPosition( width/4, height/4 );
    glutInitWindowSize( width/2, height/2 );
    glutInitDisplayMode( GLUT_RGBA );
    glutCreateWindow( argv[0] );

    glClearColor( 0.0, 0, 0.2, 1.0 );
    glOrtho( 0.0, 2.1, 0.0, 2.1, -2.0, 2.0 );
    glBindBufferARB_p    = (PFNGLBINDBUFFERARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glBindBuffer" );
    glGenBuffersARB_p    = (PFNGLGENBUFFERSARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glGenBuffers" );
    glDeleteBuffersARB_p = (PFNGLDELETEBUFFERSARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glDeleteBuffers" );
    glBufferDataARB_p    = (PFNGLBUFFERDATAARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glBufferData" );
    glMapBufferARB_p     = (PFNGLMAPBUFFERARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glMapBuffer" );
    glUnmapBufferARB_p   = (PFNGLUNMAPBUFFERARBPROC) GET_GL_PROC( (GET_GL_PTR_TYP) "glUnmapBuffer" );

    glutKeyboardFunc( keyboard );
    glutDisplayFunc( DrawScene );
    glutIdleFunc( ReDisplay );

    glutMainLoop();
    return 0;     //unreachage
}

void DrawArrays()
{
    for (size_t i = 0; i < varrays.size(); ++i)
        if (varrays[i] != NULL) {
            fprintf( stderr, "Drawing %d\n", i );
            varrays[i]->Draw();
            fprintf( stderr, "done\n", i );
        }
}

void DrawScene()
{
    glClear( GL_COLOR_BUFFER_BIT );

    /* draw lower left polygon without vertex arrays */
    DrawArrays();
    for (int LC = 0; LC < myrand()%47; ++LC) {
        size_t i = myrand()%varrays.size();
        vbo   *v = varrays[i];
        bool   didsomething = false;
        switch (myrand()%7)           //CHANGE 5 to 3 if you wish for simpler behavior (takes longer to crash, like 30 sec)
        {
        case 0:
            if (v) {
                fprintf( stderr, "Drawing %d\n", i );
                v->Draw();
                didsomething = true;
            }
            break;
        case 1:
            if (v) {
                fprintf( stderr, "Deleting %d\n", i );
                delete v;
                varrays[i]   = NULL;
                didsomething = true;
            }
            break;
        case 2:
            if (!v) {
                size_t size = myrand()%256;
                if (myrand() < myrandmax/3)
                    size = myrand()%65536;
                else if (myrand() < myrandmax/4)
                    size = myrand()%65536+65536;
                bool useindex = myrand() < myrandmax/2 ? true : false;                 //still happens (takes about 3 minutes) if useindex is false
                bool muticle  = myrand() < myrandmax/2 ? true : false;
                fprintf( stderr,
                         "Allocating %d with size %d using index %d using mutable %d \n",
                         i,
                         size,
                         (int) useindex,
                         (int) muticle );
                varrays[i]   = new vbo( size,
                                        useindex,
                                        muticle );
                didsomething = true;
            }
            break;
        case 3:
        case 5:
            if (v) {
                fprintf( stderr, "Writing %d\n", i );

                v->Write();
                didsomething = true;
            }
            break;
        case 4:
        case 6:
            if (v) {
                fprintf( stderr, "Reading %d\n", i );
                v->Read();
                didsomething = true;
            }
            break;
        }
        if (didsomething)
            fprintf( stderr, "Done\n" );
    }
    glFlush();
}

