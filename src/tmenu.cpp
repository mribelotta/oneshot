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
#include <iglu/tmenu.h>
#include <iglu/screen.h>
#include <iglu/tprogram.h>
#include <iglu/tshape.h>
#include <stdlib.h>

TMenuPopUp::TMenuPopUp(int x, int y, TSubMenu *submenu):
  TView(Rect(x,y,x,y)), menu(submenu), actual(NULL), onpop(1)
{
  int i, l;
  TMenuItem *tmp;
  for (l=i=0, tmp=menu->items; tmp; i++, tmp=tmp->next) {
    if (tmp->caption) {
      int l2 = gui_textw(tmp->caption);
      if (l<l2) l=l2;
    }
  }
  Bounds.b.x+=l+(Wt*6);
  Bounds.b.y+=i*(Ht<<1);
}

void TMenuPopUp::Draw()
{
  MouseHide ();
  TRect r(Bounds.a.x+1, Bounds.a.y+1, Bounds.b.x-1, Bounds.a.y+(Ht+Ht)-1);
  TShape shp (Bounds, shpUpBox, 1);
  shp.doDraw ();
  TMenuItem *tmp = menu->items;
  while (tmp) {
    shp.Bounds = r;
    shp.bg = EGA_LIGHTGRAY;
    if (tmp->caption==mnuSeparatorLine)
      shp.type = shpHLine;
    else if (tmp == actual)
      shp.type = shpDownBox,  shp.bg=EGA_LIGHTBLUE;
    else
      shp.type = shpNone;
    shp.doDraw ();
    if (tmp->caption)
      gui_outtextxy(r.a.x+Wt+Wt, r.a.y+(Ht>>1), tmp->caption, 0, EGA_BLACK, -1);
    r.moverel(0, (Ht+Ht));
    tmp=tmp->next;
  }
  MouseShow ();
}

void TMenuPopUp::MouseEvent(TEvent& e)
{
  TRect r2(Bounds);
  getClipArea (r2);
  if (!r2.isvalid()) return;
  TRect r(r2.a.x, r2.a.y, r2.b.x, r2.a.y+(Ht+Ht));
  TMenuItem *tmp=menu->items;
  while (tmp) {
    if (r2.contain(e.pos)) {
      getClipArea (r);
      if (!r.isvalid()) return;
      if ((e.type&evMouseMove) && r.contain(e.pos) && (actual!=tmp)) {
	 actual=tmp;
	 e.type&=~evMouseMove;
	 doDraw();
      }
      if (e.type&evMouseUp) {
	 TMessage m (actual->sig, actual, 1);
	 EventManager.SendMessage(m);
	 onpop=0;
      }
    } else if (e.type&(evMouseUp|evMouseDown))
      onpop=0;
    tmp=tmp->next;
    r.moverel(0,Ht+Ht);
  }
}

//////////////////////////////////////////////////////////////////////

TMenuBar::TMenuBar(TRect& r, TSubMenu *sub_menu):
  TView(r), menu(sub_menu), pop(NULL), actual(NULL)
{
}

TMenuBar::~TMenuBar()
{
  while (menu) {
    TSubMenu *tmp = menu;
    TMenuItem *it= menu->items;
    while (it) {
      TMenuItem *tmp2 = it;
      it=it->next;
      delete tmp2;
    }
    menu=menu->next;
    delete tmp;
  }
}

void TMenuBar::Draw()
{
  int l=Bounds.a.x;
  TSubMenu *tmp=menu;
  TShape shp (Bounds, shpUpBox, 1);
  MouseHide ();
  shp.doDraw ();
  shp.Bounds = Rect(l+1, Bounds.a.y+1, 0, Bounds.b.y-1);
  int y = shp.Bounds.a.y+(Ht>>1);
  while (tmp) {
    int x = shp.Bounds.a.x;
    shp.Bounds.b.x = x+gui_textw(tmp->caption)+Wt+Wt;
    shp.type = shpNone;
    shp.bg = EGA_LIGHTGRAY;
    if (tmp == actual)
      shp.type = shpDownBox,  shp.bg = EGA_LIGHTBLUE;
    shp.doDraw ();
    gui_outtextxy(x+Wt, y, tmp->caption, 0, EGA_BLACK, -1);
    shp.moverel (shp.Bounds.deltax()+1, 0);
    tmp=tmp->next;
  }
  if (pop && pop->onpop)
    pop->doDraw();
  MouseShow ();
}

void TMenuBar::MouseEvent(TEvent& e)
{
 if (State&stFocus) {
  int l=Bounds.a.x;
  TSubMenu *tmp=menu;
  while (tmp) {
    TRect r(l, Bounds.a.y, l+gui_textw(tmp->caption)+Wt+Wt, Bounds.b.y);
    getClipArea (r);
    if (r.contain(e.pos)) {
      if ((e.type&evMouseMove) && (State&stSelect))
	 goto do_pop;
      if (e.type&evMouseUp)
	 e.type&=~evMouseUp;
    }
    if ((e.type&evMouseDown) && r.contain(e.pos))
       goto do_pop;
    l+= r.deltax();
    tmp=tmp->next;
  }
  if (pop && pop->onpop) {
    pop->MouseEvent(e);
    if (pop->Bounds.contain(e.pos))
      e.type&=~evMouse;
    if (!pop->onpop)
      PopDown();
  }
  return;
do_pop:
  if (actual!=tmp) {
    actual=tmp;  lactual=l;
    PopUp();
  }
 }
}

void TMenuBar::PopUp()
{
  TSubMenu *tmp = actual;
  int l=lactual;
  if (pop)
    PopDown();
  if (tmp) {
    pop = new TMenuPopUp (lactual=l, Bounds.b.y, actual = tmp);
    pop->Owner = Owner;
    SetState (stSelect, 1);
    doDraw();
  }
}

void TMenuBar::PopDown()
{
  if (pop) {
    TRect g;
    actual=NULL;
    lactual=-1;
    gui_getclip(g);
    SetState (stSelect, 0);
    pop->Bounds.a.y = Bounds.a.y;
    gui_setclip(pop->Bounds);
    delete pop;
    pop=NULL;
    MouseHide ();
    if (Owner)
      Owner->doDraw ();
    doDraw();
    MouseShow ();
    gui_setclip (g);
  }
}

