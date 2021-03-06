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
#include <iglu/tlistbox.h>
#include <iglu/tshape.h>
#include <iglu/mousehdl.h>

//#define height_item  32

#define d_items    (1<<2)
#define d_decorate (1<<3)
#define d_oters    (1<<4)
#define d_all      (d_items+d_decorate+d_oters)

TListBox::TListBox (TRect& r): TView(r)
{
  height_item = 16;
  onSelect = NULL;
  flags=d_all;
  list = new TList ();
  TPoint p (r.a);
  p.x += r.deltax()-16-3;
  p.y += 3;
  bar = new TScroll (p, r.b.y-3, VScroll);
  bar->Align = vaPreservW+vaRight+vaPropH;
  bar->min = bar->pos = pos = 0;
  bar->inc = 1;
  bar->max = 0;
  calculateparams ();
}

TListBox::~TListBox ()
{
  delete bar;
  delete list;
}

void TListBox::clearlist ()
{
  delete list;
  list = new TList ();
  calculateparams ();
}

void TListBox::RecuestAlign (TRect& o, TRect& n)
{
  TRect b (Bounds);
  TView::RecuestAlign (o, n);
  bar->RecuestAlign (b, Bounds);
}

void TListBox::GotFocus ()
{
  if (State&stFocus) return;
  TView::GotFocus ();
  drawdecorate ();
}

void TListBox::LostFocus ()
{
  if (State&stFocus) {
    TView::LostFocus ();
    drawdecorate ();
  }
}

void TListBox::moverel (int dx, int dy)
{
  TView::moverel(dx, dy);
  bar->moverel (dx, dy);
}

int TListBox::finditem (char *s)
{
  return list->indexOf (s);
}

void TListBox::additem (char *name, TObject *obj)
{
  list->addObject (name, obj);
  bar->min = bar->pos = pos = 0;
  bar->inc = 1;
  bar->max = 0;
  calculateparams ();
}

void TListBox::drawdecorate ()
{
  int old = flags;
  flags = d_decorate;
  doDraw ();
  flags = old;
}

void TListBox::dodrawdecorate ()
{
  if (State&stFocus)
    gui_drawrect (Bounds, EGA_LIGHTBLUE);
  else {
    gui_drawvline (Bounds.a.x, Bounds.a.y, Bounds.b.y, EGA_BLACK);
    gui_drawvline (Bounds.b.x, Bounds.a.y, Bounds.b.y, EGA_WHITE);
    gui_drawhline (Bounds.a.x, Bounds.a.y, Bounds.b.x-1, EGA_BLACK);
    gui_drawhline (Bounds.a.x, Bounds.b.y, Bounds.b.x, EGA_WHITE);
  }
}

void TListBox::drawitems ()
{
  int old=flags;
  flags = d_items;
  doDraw ();
  flags = old;
}

void TListBox::calculateparams ()
{
  int l, n;
  TRect r;
  getClientArea (r, 1);
  n = r.deltay ()/height_item;
  bottomitem = bar->pos;
  l = bottomitem+n;
  topitem = (nitems()<l)? nitems():l;
  l = nitems()-n;
  bar->max = (l>bar->min)? l:bar->min;
}

void TListBox::dodrawitems ()
{
  int i, bg, fg;
  TRect g1, g3, g4;  gui_getclip(g1);  TRect g2(g1);
  getClientArea (g3, 1);
  g1 = Intersect (g1, g3);
  getClipArea (g1);
  if (!g1.isvalid()) return;
  gui_setclip (g1);
  g4 = g3;
  g4.b.y = g4.a.y + height_item;
  calculateparams ();
  char *s;
  for (i=bottomitem; i<topitem; i++) {
    fg = EGA_BLACK;
    bg = EGA_LIGHTGRAY;
    if (i==pos) { fg=EGA_WHITE;  bg=EGA_BLACK; }
    gui_drawrectfill (g4, bg);
    s = getn (i);
    gui_outtextxy (g4.a.x, g4.a.y+g4.deltay()/2-Ht/2, s, 0, fg, -1);
    g4.moverel (0, g4.deltay());
  }
  gui_setclip (g2);
}

void TListBox::Draw ()
{
  if (flags&d_oters) {
    TShape shp (Bounds, shpDownBox);
    shp.Bounds.Grow (1,1);
    shp.Draw ();
    bar->Draw ();
  }
  if (flags&d_items)
    dodrawitems ();
  if (flags&d_decorate)
    dodrawdecorate ();
}

void TListBox::selectn (int n)
{
  if (n!=pos) {
    if (n==-3)
      { n=pos+1; if (n>nitems()) n=nitems(); }
    if (n==-2)
      { n=pos-1; if (n<0) n=0; }
    pos = n;
    if (pos>bottomitem)
      topitem++;
    if (pos<topitem)
      topitem--;
    drawitems ();
  }
}

void TListBox::getClientArea (TRect& r, int i)
{
  TView::getClientArea (r, i);
  r.Grow (3,3);
  r.b.x = bar->Bounds.a.x-2;
}

void TListBox::HandleEvent (TEvent& e)
{
  bar->Owner = Owner;
  flags |= d_all;
  TView::HandleEvent(e);
  if (State&stFocus) {
    TRect a;
    getClientArea (a, 1);
    calculateparams ();
    bar->HandleEvent (e);
    if (e.type==evMessage)
      if ((e.msg.message == cmChangeScroll) && (e.msg.sender == bar))
        drawitems ();
    if (e.type&evMouseDown)
      SetState (stSelect, 1);
    if ((e.type&(evMouseMove|evMouseDown)) && (State&stSelect)) {
      char *s;  int i;
      TRect g = a;
      g.b.y = g.a.y+height_item;
      for (i=bottomitem; i<topitem; i++) {
	  if (g.contain (e.pos)) {
	     selectn (i);
	     if (onSelect)
	       onSelect (e.pos, this);
	  }
	  g.moverel (0, g.deltay());
      }
    }
    if (e.type&evMouseUp)
      SetState (stSelect, 0);
  }
}

