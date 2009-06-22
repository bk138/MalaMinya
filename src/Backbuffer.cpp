
#include "Backbuffer.h"


Backbuffer::Backbuffer(XConn* x11, Drawable d, int w, int h)
{
  this->x11 = x11;

  XGCValues gcvalues;
  gcvalues.foreground = x11->white;

  buf = XCreatePixmap(x11->dpy, d, w, h, x11->depth);
  gc = XCreateGC(x11->dpy, buf, GCForeground, &gcvalues);

  XFillRectangle(x11->dpy, buf, gc, 0, 0, w, h);

  XFlush(x11->dpy);
}



Backbuffer::~Backbuffer()
{
  XFreeGC(x11->dpy, gc);
  XFreePixmap(x11->dpy, buf);
}

