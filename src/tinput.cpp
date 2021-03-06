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
#include <iglu/tinput.h>
#include <iglu/tshape.h>
#include <ctype.h>
#include <string.h>

#ifdef __TURBOC__
#define kleft  19200
#define kdown  20480
#define kright 19712
#define kup    18432
#define khome  18176
#define kend   20224
#define kdel   21248
#elif defined (ONGRX)
#include <grxkeys.h>
#define kleft  GrKey_Left
#define kdown  GrKey_Down
#define kright GrKey_Right
#define kup    GrKey_Up
#define khome  GrKey_Home
#define kend   GrKey_End
#define kdel   GrKey_Delete
#elif defined (ONX11)
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#undef XK_MISCELLANY
#define kleft  XK_Left
#define kdown  XK_Down
#define kright XK_Right
#define kup    XK_Up
#define khome  XK_Home
#define kend   XK_End
#define kdel   XK_Delete
#else
#error Not target recognized
#endif

TInputLine::TInputLine (TRect& r, int l): TView(r),
  text(new char[l]), maxlen(l), pos(0), len(0)
{
  text[0]=0;
}

void TInputLine::GotFocus ()
{
  if (State&stFocus) return;
  TView::GotFocus();
  doDraw();
}

void TInputLine::LostFocus ()
{
  if (State&stFocus) {
    TView::LostFocus();
    doDraw();
  }
}

void TInputLine::setText (char *s)
{
  int l = strlen (s);
  strncpy (text, s, maxlen);
  len = (l<maxlen)? l:maxlen;
  doDraw ();
}

void TInputLine::Draw ()
{
  gui_drawrectfill (Bounds, EGA_WHITE);
  gui_drawrect (Bounds, (State&stFocus)? EGA_LIGHTBLUE:EGA_BLACK);
  gui_outtextxy (Bounds.a.x+8,Bounds.a.y+Bounds.deltay()/2-Ht/2,
		 text,0, EGA_BLACK,-1);
  if (State&stFocus)
    gui_drawvline(Bounds.a.x+8+(pos*Wt), Bounds.a.y+Bounds.deltay()/2-5,
      Bounds.a.y+Bounds.deltay()/2+5, EGA_BLACK);
}

void TInputLine::insertChar(int p, int c)
{
  if (len<maxlen-1) {
    int i;
    for (i=++len; i>p; i--)
      text[i]=text[i-1];
    text[p]=c;
  }
}

void TInputLine::deleteChar(int p)
{
  if (len && p<maxlen) {
    int i;
    for (i=p; (i<len) && text[i]; i++)
      text[i]=text[i+1];
    text[--len]=0;
  }
}

void TInputLine::setPos (int p) {
  if (p!=pos) {
    if (p>len) p=len;
    if (p<0) p=0;
    pos=p;
    text[len+1]=0;
    doDraw ();
  }
}

void TInputLine::HandleEvent (TEvent& e)
{
  TView::HandleEvent(e);
  if (State&stFocus) {
    TRect r(Bounds);
    getClipArea (r);
    if ((e.type&evMouseDown) && r.contain(e.pos))
      setPos ((e.pos.x-Bounds.a.x-8)/Wt);
    if (e.type&evKeyDown)
      switch (e.keycode) {
	     case kleft:  setPos (pos-1);  break;
	     case kright: setPos (pos+1);  break;
	     case khome:  setPos (0);      break;
	     case kend:   setPos (len);    break;
	     case kdel:
	       if (pos<len) {
		 deleteChar(pos);
		 doDraw ();
	       }
	       break;
	     default:
		     int c=e.keycode&0xFF, p=pos;
		     if (isprint(c))
			insertChar(p++, c);
		     else if ((c=='\b') && p)
			  deleteChar(--p);
		     setPos (p);
      }
  }
}

