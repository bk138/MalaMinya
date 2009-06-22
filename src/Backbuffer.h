
#ifndef BACKBUFFER_H
#define BACKBUFFER_H

#include "XConn.h"


class Backbuffer
{
  XConn* x11;

 public:
  GC gc;
  Pixmap buf;

 public:
  Backbuffer(XConn* x11, Drawable d, int w, int h);
  ~Backbuffer();


};


#endif
