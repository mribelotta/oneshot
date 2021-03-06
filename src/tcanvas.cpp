/*
 * IGLU Iterfaz Grafica Libre del Usuario. (libre de usuarios) (Graphics user interface "free of users")
 * Copyright (C) 2005 Martin Alejandro Ribelotta
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <iglu/tcanvas.h>

TScrollCanvas::TScrollCanvas (TRect& r, int vw, int vh):
  TContainer(r), virtualw(vw), virtualh(vh),
  h(TPoint(r.a.x, r.b.y-16), r.b.x-16, HScroll),
  v(TPoint(r.b.x-16, r.a.y), r.b.y-16, VScroll)
{
  h.min=v.min=0;
  h.max=vh;
  v.max=vw;
  h.inc = r.deltay()/8;
  v.inc = r.deltax()/8;
  h.Align = vaLeft+vaBottom+vaPreservH+vaPropW;
  v.Align = vaRight+vaTop+vaPreservW+vaPropH;
  oldposx=oldposy=h.pos=v.pos=0;
  h.Owner = v.Owner = this;
}

void TScrollCanvas::moveall (int newx, int newy)
{
  TView *curr;
  for (curr=base; curr; curr=curr->next)
      curr->moverel(newx,newy);
}

void TScrollCanvas::getClientArea (TRect& r, int b)
{
  TContainer::getClientArea( r, b );
  TPoint p (v.Bounds.deltax(), h.Bounds.deltay());
  r.b -= p;
}

void TScrollCanvas::moverel (int dx, int dy)
{
  TContainer::moverel(dx,dy);
  h.moverel(dx,dy);
  v.moverel(dx,dy);
}

void TScrollCanvas::Draw ()
{
//  TPoint p (v.Bounds.deltax(), h.Bounds.deltay());
//  Bounds.b -= p;
  TContainer::Draw ();
//  Bounds.b += p;
  h.Draw ();
  v.Draw ();
  gui_drawrect (Bounds, EGA_BLACK);
}

void TScrollCanvas::RecuestAlign (TRect& o, TRect& n)
{
  TRect b (Bounds);
  TContainer::RecuestAlign (o,n);
  v.RecuestAlign (b, Bounds);
  h.RecuestAlign (b, Bounds);
}

void TScrollCanvas::HandleEvent (TEvent& e)
{
  h.Owner = v.Owner = this;
  if ((e.type&evMessage) && ((e.msg.sender==&h) || (e.msg.sender==&v))) {
    int incx, incy, posx = oldposx, posy = oldposy;
    if (e.msg.sender==&v) posy = v.pos;
    else posx = h.pos;
    incx = oldposx-posx;
    incy = oldposy-posy;
    oldposx = posx;
    oldposy = posy;
    moveall (incx, incy);
    doDraw();
    e.type&=~evMessage;
  }
  h.HandleEvent (e);
  v.HandleEvent (e);
  TContainer::HandleEvent (e);
}

