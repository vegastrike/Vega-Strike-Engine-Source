#include "gauge.h"


Gauge::Gauge (const char *file, DIRECTION up): Sprite(file) {
  dir = up;
  float sx,sy;
  Sprite::GetSize(sx,sy);
  SetSize (-sx,-sy);
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
  Sprite::GetSize (sx,sy);
  Sprite::GetPosition(px,py);
  switch (dir) {
  case GAUGE_RIGHT:
  case GAUGE_LEFT:
    SetST ((1-percentage),0);
    Sprite::SetSize (sx*percentage,sy);
    Sprite::SetPosition (px+sx*(1-percentage)*.5,py);
    break;
  case GAUGE_DOWN:
  case GAUGE_UP:
    SetST (0,(1-percentage));
    Sprite::SetSize (sx,sy*percentage);
    Sprite::SetPosition (px,py+sy*(1-percentage)*.5);
    break;
  }
  Sprite::Draw();
  Sprite::SetSize (sx,sy);
  Sprite::SetPosition (px,py);
}
