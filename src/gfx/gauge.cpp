#include "gauge.h"


Gauge::Gauge (const char *file, DIRECTION up): Sprite(file) {
  dir = up;
  float sx,sy;
  if (dir==GAUGE_UP) {
    GetSize(sx,sy);
    SetSize (sx,-sy);
    dir = GAUGE_DOWN;
  }
  if (dir==GAUGE_LEFT) {
    GetSize (sx,sy);
    SetSize (-sx,sy);
    dir = GAUGE_RIGHT;
  }
}
void Gauge::Draw (float percentage) {
  float sx,sy,px,py;
  GetSize (sx,sy);
  GetPosition(px,py);
  switch (dir) {
  case GAUGE_RIGHT:
    SetST (percentage,1);
    SetSize (sx*percentage,sy);
    SetPosition (px-sx*(1-percentage)*.5,py);
    break;
  case GAUGE_DOWN:
    SetST (1,percentage);
    SetSize (sx,sy*percentage);
    SetPosition (px,py-sy*(1-percentage)*.5);
    break;
  }
  Sprite::Draw();
  SetSize (sx,sy);
  SetPosition (px,py);
}
