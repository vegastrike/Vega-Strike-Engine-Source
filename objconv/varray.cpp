/*
 * Copyright (c) 1999  Silicon Graphics, Inc.  All rights reserved.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF THE
 * POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*  vertexArrays.c  - vertex array example. uses the same shaded
 *                    polygon seen in ~/rendering/shadeModel.c
 *
 *	SPACE key	- toggle between flat/smooth shading
 *	ESCAPE key	- exit program
 */

#include <GL/glut.h>	/* includes gl.h, glu.h */

#undef GL_ARB_vertex_buffer_object
#include <GL/glext.h>
#include <stdio.h>
PFNGLBINDBUFFERARBPROC glBindBufferARB_p=0;
PFNGLGENBUFFERSARBPROC glGenBuffersARB_p=0;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB_p=0;
PFNGLBUFFERDATAARBPROC glBufferDataARB_p=0;
PFNGLMAPBUFFERARBPROC  glMapBufferARB_p=0;
PFNGLUNMAPBUFFERARBPROC  glUnmapBufferARB_p=0;


#ifndef _WIN32
#include <GL/glx.h>
#endif

/*  Function Prototypes  */

GLvoid  drawScene( GLvoid );
static GLvoid redisplay (GLvoid) {
  glutPostRedisplay();
  
  
}

/* Global Definitions */

#define KEY_ESC	27	/* ascii value for the escape key */
static GLvoid 
keyboard( GLubyte key, GLint x, GLint y )
{
	switch (key) {
	case KEY_ESC:	/* Exit whenever the Escape key is pressed */
		exit(0);
	}
}
typedef void (*(*get_gl_proc_fptr_t)(const GLubyte *))(); 
#ifdef _WIN32
    typedef char * GET_GL_PTR_TYP;
#define GET_GL_PROC wglGetProcAddress

#else
    typedef GLubyte * GET_GL_PTR_TYP;
#define GET_GL_PROC glXGetProcAddressARB
#endif

void
main( int argc, char *argv[] )
{
	GLsizei width, height;

	glutInit( &argc, argv );

	width = glutGet( GLUT_SCREEN_WIDTH ); 
	height = glutGet( GLUT_SCREEN_HEIGHT );
	glutInitWindowPosition( width / 4, height / 4 );
	glutInitWindowSize( width / 2, height / 2 );
	glutInitDisplayMode( GLUT_RGBA );
	glutCreateWindow( argv[0] );

	glClearColor( 0.0, 0.0, 1.0, 1.0 );
	glOrtho( 0.0, 2.1, 0.0, 2.1, -2.0, 2.0 );
        glBindBufferARB_p=(PFNGLBINDBUFFERARBPROC)GET_GL_PROC((GET_GL_PTR_TYP)"glBindBuffer");	;
        glGenBuffersARB_p=(PFNGLGENBUFFERSARBPROC)GET_GL_PROC((GET_GL_PTR_TYP)"glGenBuffers");	;
        glDeleteBuffersARB_p=(PFNGLDELETEBUFFERSARBPROC)GET_GL_PROC((GET_GL_PTR_TYP)"glDeleteBuffers");	;
        glBufferDataARB_p=(PFNGLBUFFERDATAARBPROC)GET_GL_PROC((GET_GL_PTR_TYP)"glBufferData");	;
        glMapBufferARB_p=(PFNGLMAPBUFFERARBPROC)GET_GL_PROC((GET_GL_PTR_TYP)"glMapBuffer");	;
        glUnmapBufferARB_p=(PFNGLUNMAPBUFFERARBPROC)GET_GL_PROC((GET_GL_PTR_TYP)"glUnmapBuffer");	

	glutKeyboardFunc( keyboard );
	glutDisplayFunc( drawScene ); 
        glutIdleFunc( redisplay);

	glutMainLoop();
}



GLvoid
drawArrays( GLvoid )
{
	static GLfloat	colorArray[] = {
		1.0f, 0.0f, 1.0f, /* purple */
		1.0f, 0.0f, 0.0f, /* red */
		0.0f, 0.0f, 0.0f, /* black */
		0.0f, 0.5f, 0.0f, /* dark green */
		1.0f, 1.0f, 1.0f  /* white */
	};

	static GLfloat  vertexArray[] = {
		0.5f, 0.1f,
		1.0f, 0.4f,
		0.9f, 1.0f,
		0.3f, 0.8f,
		0.1f, 0.5f
	};

	static GLuint	indices[] = { 2, 3, 4, 0, 1 };
	

	/* set up the array data */
	glColorPointer( 3, GL_FLOAT, 3*sizeof(GLfloat), colorArray );
	glVertexPointer( 2, GL_FLOAT, 2*sizeof(GLfloat), vertexArray );

	/* enable vertex arrays */
	glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_VERTEX_ARRAY );

	/* draw a polygon using the arrays sequentially */
	glDrawArrays( GL_POLYGON, 0, 5 );

	/* draw a triangle fan using the arrays in a different order */
			glDrawElements ( GL_TRIANGLE_FAN, 5, GL_UNSIGNED_INT, indices );

	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );

}

GLvoid drawScene( GLvoid )
{
	glClear( GL_COLOR_BUFFER_BIT );

	/* draw lower left polygon without vertex arrays */
	drawArrays();


	glFlush();
}
