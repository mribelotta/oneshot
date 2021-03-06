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
#include <iglu/tview.h>
#include <iglu/screen.h>
#include <iglu/mousehdl.h>
#include <stddef.h>

TView::TView (const TRect& r): TObject(),
  Bounds(r), next(NULL), prev(NULL), Owner(NULL),
  State(stVisible+stExpose+stFocusheable),
  Align(vaNone)
{
}

TView::~TView ()
{
}

void TView::doDraw ()
{
  TRect clip;  gui_getclip (clip);  TRect r (clip);
  getClipArea (r);
  if (r.isvalid ()) {
    gui_setclip (r);
    MouseHide ();
    Draw ();
    MouseShow ();
    gui_setclip (clip);
  }
}

void TView::Draw()
{
}

void TView::HandleEvent(TEvent& e)
{
  if (e.type&evMouse) {
    if (State&stFocusheable) {
      TRect r (Bounds);
      getClipArea (r);
      if (r.isvalid() && (e.type&evMouseDown) && r.contain(e.pos))
	GotFocus ();
    }
    MouseEvent(e);
  }
  if (e.type&evKeyDown)
    KeyEvent(e);
  if (e.type&evMessage)
    MessageEvent(e);
  if (e.type&evTimerOut)
    TimerEvent(e);
  if (e.type==evNoting)
    idle();
}

void TView::RecuestAlign (TRect& o, TRect& n)
{
  TRect nb (Bounds);
  int w=nb.deltax(), h=nb.deltay();
  int gapleft=0, gaptop=0, gapbottom=0, gapright=0;

  if (Align&vaPreservGapR)
    gapright = o.b.x-nb.b.x;
  if (Align&vaPreservGapL)
    gapleft = nb.a.x-o.a.x;
  if (Align&vaPreservGapB)
    gapbottom = o.b.y-nb.b.y;
  if (Align&vaPreservGapT)
    gaptop = nb.a.y-o.a.y;

  if (Align&vaLeft)
    nb.a.x = n.a.x;
  if (Align&vaRight)
    nb.b.x = n.b.x;
  if (Align&vaTop)
    nb.a.y = n.a.y;
  if (Align&vaBottom)
    nb.b.y = n.b.y;

  if (gapright)
    nb.b.x = n.b.x-gapright;
  if (gapbottom)
    nb.b.y = n.b.y-gapbottom;
  if (gapleft)
    nb.a.x = n.a.x+gapleft;
  if (gaptop)
    nb.a.y = n.a.y+gaptop;


  if (Align&vaPreservW)
    if (Align&vaLeft)
      nb.b.x = nb.a.x + w;
    else if (Align&vaRight)
      nb.a.x = nb.b.x - w;
  if (Align&vaPreservH)
    if (Align&vaTop)
      nb.b.y = nb.a.y + h;
    else if (Align&vaBottom)
      nb.a.y = nb.b.y - h;

  move (nb.a.x, nb.a.y);
  Bounds.b = nb.b;
}

void TView::GotFocus()
{
  if (State&stFocusheable) {
    TView *tmp;
    for (tmp=next; tmp; tmp=tmp->next)
      tmp->LostFocus();
    for (tmp=prev; tmp; tmp=tmp->prev)
      tmp->LostFocus();
    State|=stFocus;
  }
}

void TView::getClipArea(TRect& r)
{
  TView *tOwner=Owner;
  TRect area, newclip (r);
  if (!(State&stNoClipped)) // Only for views whit clipping (default)
    while (tOwner) {
      if ( State&stAbsoluteClipped )
	area = tOwner->Bounds;
      else
	tOwner->getClientArea (area, 0);
      newclip = Intersect (newclip, area);
      tOwner = tOwner->Owner;
    }
  r = newclip;
}

void TView::LostFocus()
{
  SetState(stFocus, 0);
}

void TView::SetState(int Stat, int active)
{
  if (active)
    State|=Stat;
  else
    State&=~Stat;
}

void TView::resize (int w, int h)
{
  Bounds.b.x = Bounds.a.x + w;
  Bounds.b.y = Bounds.a.y + h;
}

//////////////////////////////////////////////////////////////////////

TGroup::TGroup(const TRect& r): TView(r), base(NULL), head(NULL)
{
}

TGroup::~TGroup()
{
  while (base) {
    TView *tmp = base;
    base=base->next;
    delete tmp;
  }
}

void TGroup::Draw()
{
  MouseHide ();
  TView::Draw();
  DrawSubViews(base);
  MouseShow ();
}

void TGroup::HandleEvent(TEvent& e)
{
  TView::HandleEvent(e);
  if (State&stFocus) {
    HandleEventSubViews(e, head);
    if (Bounds.contain(e.pos))
      e.type&=~evMouse;
  }
}

void TGroup::Execute()
{
  TEvent e;
  while (State&stModal) {
    EventManager.getevent(e);
    HandleEvent(e);
    gui_flush();
  }
}

void TGroup::ExecView(TView *q)
{
  if (q) {
    int old_Status = q->State;
    TView *old_prev = q->prev;
    TView *old_next = q->next;
    TGroup *old_Owner = q->Owner;
    insert(q);
    q->GotFocus();
    q->SetState(stModal,1);
    q->Execute();
    TRect qb(q->Bounds);
    detach(q);
    q->State = old_Status;
    q->prev = old_prev;
    q->next = old_next;
    q->Owner = old_Owner;
    RedrawRect(qb);
  }
}

void TGroup::moverel (int dx, int dy)
{
  TView *tmp=base;
  TView::moverel(dx,dy);
  while (tmp) {
    tmp->moverel(dx,dy);
    tmp=tmp->next;
  }
}

void TGroup::insert(TView *q)
{
  if (q) {
    q->Owner = this;
    q->moverel(Bounds.a.x, Bounds.a.y);
    if (!base && !head) {
      base=head=q;
      q->prev=NULL;
    } else {
      head->next=q;
      q->prev=head;
      head=q;
    }
    q->next=NULL;
  }
}

void TGroup::detach(TView *q)
{
  if (q) {
    if (q->prev)
      q->prev->next=q->next;
    if (q->next)
      q->next->prev=q->prev;
    if (q==base)
      base=q->next;
    if (q==head)
      head=q->prev;
    q->prev=q->next=NULL;
    q->moverel(-Bounds.a.x, -Bounds.a.y);
    q->Owner = NULL;
  }
}

int TGroup::indexOf(TView *q)
{
  int i=0;  TView *tmp=base;
  while (tmp) {
    if (q==tmp)
      return (i);
    i++;  tmp=tmp->next;
  }
  return (-1);
}

void TGroup::RedrawRect(TRect& r)
{
  TRect g, g2;
  gui_getclip(g);
  g2 = Intersect (g, r);
  gui_setclip(g2);
  doDraw ();
  gui_setclip(g);
}

void TGroup::moveTop(TView *q)
{
  detach(q);
  insert(q);
}

void TGroup::DrawSubViews(TView *initial)
{
  TView *tmp;
  for (tmp=initial; tmp; tmp=tmp->next)
    tmp->doDraw();
}

void TGroup::HandleEventSubViews(TEvent& e, TView *initial)
{
  TView *tmp;
  for (tmp=initial; tmp; tmp=tmp->prev)
    tmp->HandleEvent(e);
}

void TGroup::SendToChild (TMessage& m)
{
  TEvent e(m);
  HandleEventSubViews (e, head);
}

void TGroup::RecuestAlign (TRect& o, TRect& n)
{
  TRect oldBounds (Bounds);
  TView::RecuestAlign (o, n);
  TView *t;
  for (t=base; t; t=t->next)
    t->RecuestAlign (oldBounds, Bounds);
}

void TGroup::resize (int w, int h)
{
  TRect r1, r2;  TView *t;
  getClientArea (r1, 0);
  TView::resize (w, h);
  getClientArea (r2, 0);
  for (t=base; t; t=t->next)
    t->RecuestAlign (r1, r2);
}
