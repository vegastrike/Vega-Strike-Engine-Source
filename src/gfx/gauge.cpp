#include "gauge.h"


Gauge::Gauge (const char *file, DIRECTION up): Sprite(file) {
  dir = up;
  float sx,sy;
  if (dir==GAUGE_UP) {
    Sprite::GetSize(sx,sy);
    Sprite::SetSize (sx,-sy);
  }
  if (dir==GAUGE_LEFT) {
    Sprite::GetSize (sx,sy);
    Sprite::SetSize (-sx,sy);
  }
}
void Gauge::SetSize (float x, float y) {
  if (dir==GAUGE_UP) {
    y= -y;
  } else if (dir==GAUGE_LEFT) {
    x = -x;
  }
  Sprite::SetSize (x,y);

}
void Gauge::GetSize (float &x, float &y) {
  Sprite::GetSize (x,y);
  if (dir==GAUGE_UP) {
    y= -y;
  } else if (dir==GAUGE_LEFT) {
    x = -x;
  }
}


void Gauge::Draw (float percentage) {
  float sx,sy,px,py;
  GetSize (sx,sy);
  GetPosition(px,py);
  switch (dir) {
  case GAUGE_RIGHT:
  case GAUGE_LEFT:
    SetST (percentage,1);
    SetSize (sx*percentage,sy);
    SetPosition (px-sx*(1-percentage)*.5,py);
    break;
  case GAUGE_DOWN:
  case GAUGE_UP:
    SetST (1,percentage);
    SetSize (sx,sy*percentage);
    SetPosition (px,py-sy*(1-percentage)*.5);
    break;
  }
  Sprite::Draw();
  SetSize (sx,sy);
  SetPosition (px,py);
}
