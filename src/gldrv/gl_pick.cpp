/* GL selection buffer code
 */

//#include <GL/gl.h>
#include "gl_include.h"
#include "gfxlib.h"
#include "gl_matrix.h"

#include  <assert.h>
#define SELECTBUF_SIZE MAX_PICK*4
const float epsilon = 0.001;

static vector<PickData> *picked_objects = new vector<PickData>();
static unsigned selectbuf[SELECTBUF_SIZE];

using namespace GFXMatrices;

void GFXBeginPick(int x, int y, int xsize, int ysize) {
  // save and change gl projection matrix
  GLint viewport[4];

  float left, right, bottom, top, fnear, ffar;
  GFXGetFrustumVars (true, &left, &right, &bottom, &top, &fnear, &ffar);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glGetIntegerv(GL_VIEWPORT, viewport);
  gluPickMatrix(x,viewport[3]-y,xsize, ysize, viewport);
  glMultMatrixf(projection);

  glSelectBuffer(SELECTBUF_SIZE, selectbuf);
  glRenderMode(GL_SELECT);
  glGetError();

  delete picked_objects;
  picked_objects = new vector<PickData>();
}

void GFXSetPickName(int name) {
  glInitNames();
  glPushName(name);
  glLoadName(name);
  glGetError();
}

void
drawRects(GLenum mode)
{
    glLoadName(1);
  glBegin(GL_QUADS);
  glColor3f(1.0, 1.0, 0.0);
  glVertex3i(2, 0, 0);
  glVertex3i(2, 6, 0);
  glVertex3i(6, 6, 0);
  glVertex3i(6, 0, 0);
  glEnd();
    glLoadName(2);
  glBegin(GL_QUADS);
  glColor3f(0.0, 1.0, 1.0);
  glVertex3i(3, 2, -1);
  glVertex3i(3, 8, -1);
  glVertex3i(8, 8, -1);
  glVertex3i(8, 2, -1);
  glEnd();
    glLoadName(3);
  glBegin(GL_QUADS);
  glColor3f(1.0, 0.0, 1.0);
  glVertex3i(0, 2, -2);
  glVertex3i(0, 7, -2);
  glVertex3i(5, 7, -2);
  glVertex3i(5, 2, -2);
  glEnd();
}

// Don't call this function
bool GFXCheckPicked() {  
  assert(0);
  glFlush();
  int num_hits = glRenderMode(GL_RENDER);
  glRenderMode(GL_SELECT);
  glGetError();
  assert(num_hits>=0&&num_hits<=1);
  return num_hits>0;
}

vector<PickData> *GFXEndPick() {
  int num_hits = glRenderMode(GL_RENDER);
  assert(num_hits!=-1);
  //num_hits could == -1, check for this case
  unsigned *ptr = selectbuf;
  for(int a=0; a<num_hits; a++) {
    picked_objects->push_back(PickData(ptr[3],ptr[1],ptr[2]));
    ptr+=ptr[0]+3;
  }
  // restore gl projection matrix
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  return picked_objects;
}
