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
#include <iglu/tless.h>
#include <iglu/screen.h>
#include <string.h>

TLessBase::TLessBase (TRect& r): TView(r)
{
  TPoint p1, p2;  int lx, ly;
  p1 = TPoint (r.a.x, r.b.y-16);  lx = r.b.x-16;
  p2 = TPoint (r.b.x-16, r.a.y);  ly = r.b.y-16;
  sh = new TScroll (p1, lx, HScroll);
  sv = new TScroll (p2, ly, VScroll);
  sh->next = sv;
  sv->prev = sh;
  sh->min = sv->min =
  sh->pos = sv->pos =
  sh->max = sv->max =
  linestart = lineend = 0;
  sh->inc = sv->inc = 1;
  sv->Align = vaRight+vaTop+vaPreservW+vaPropH;
  sh->Align = vaLeft+vaBottom+vaPreservH+vaPropW;
}

void TLessBase::Draw ()
{
  MouseHide ();
  TView::Draw ();
  sh->Draw ();
  sv->Draw ();
  DrawText ();
  MouseShow ();
}

void TLessBase::HandleEvent (TEvent& e)
{
  sh->Owner = sv->Owner = Owner;
  sh->HandleEvent (e);
  sv->HandleEvent (e);
  TView::HandleEvent (e);
  if ((e.type&evMessage) && (e.msg.message==cmChangeScroll)) {
      if (e.msg.sender==sv) {
	int paso = (e.msg.getlong()&0x80000000)? 1:-1;
	if (lineend<=linemax) {
	  linestart += paso;
	  lineend += paso;
	  DrawText ();
	}
      } else if (e.msg.sender==sh)
	DrawText ();
    }
}

void TLessBase::moverel (int dx, int dy)
{
  TView::moverel (dx,dy);
  sh->moverel (dx,dy);
  sv->moverel (dx,dy);
}

void TLessBase::RecuestAlign (TRect& bo, TRect& bn)
{
  TView::RecuestAlign (bo, bn);
  sh->RecuestAlign (bo, bn);
  sv->RecuestAlign (bo, bn);
}

int TLessBase::calclend ()
{
  int deltatext, line;
  deltatext = Bounds.deltay()/Ht;
  line = linestart+deltatext;
  return line;
}

void TLessBase::DrawText ()
{
  int lin;
  TRect oldClip, clip;
  TPoint p;
  int paso = Ht+2;
  Bounds.b.x -= 17; Bounds.b.y -= 17;
  gui_getclip(oldClip);
  clip = Intersect (Bounds, oldClip);
  gui_setclip(clip);
  p = TPoint (Bounds.a.x+2-sh->pos, Bounds.a.y+(paso>>1));
  MouseHide ();
  gui_drawrectfill (Bounds, EGA_WHITE);
  for (lin=linestart; lin<lineend; lin++, p.y+=paso)
    gui_outtextxy (p.x, p.y, getline (lin), 0, EGA_BLACK, -1);
  MouseShow ();
  gui_setclip(oldClip);
  Bounds.b.x += 17; Bounds.b.y += 17;
}

/****************************************************************/

TLessFile::TLessFile (TRect& r, char *filename): TLessBase(r)
{
  char fm[3];
  fm[0]='r', fm[1]='t', fm[2]=0;
  buffer = new char[256];
  f = fopen(filename, fm);
  if (f) {
    int wmax=0, lmax=0, i;
    while (!feof(f)) {
      fgets (buffer, 255, f);
      i = strlen(buffer);
      if (i>wmax)
	wmax=i;
      lmax++;
    }
    sh->max = wmax*Wt;
    sh->inc = 10;
    linemax = lmax;
    sv->max = lmax-calclend();
    if (sv->max<0) sv->max=0;
    eolseek = new int[lmax];
    fseek (f, 0l, SEEK_SET);
    i=0;
    while (!feof(f)) {
      eolseek[i++] = ftell(f);
      fgets (buffer, 255, f);
    }
  }
  lineend = calclend ();
  if (linemax<lineend)
    lineend=linemax;
}

TLessFile::~TLessFile ()
{
  fclose(f);
  delete eolseek;
  delete buffer;
}

char *TLessFile::getline (int l)
{
  buffer[0]=0;
  if (f && (l>=0) && (l<linemax)) {
    int i;
    fseek (f, eolseek[l], SEEK_SET);
    fgets(buffer, 255, f);
    i = 0;
    while (buffer[i]>=' ') i++;
    buffer[i]=0;
  }
  return buffer;
}
