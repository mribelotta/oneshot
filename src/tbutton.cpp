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
#include <iglu/tbutton.h>
#include <iglu/screen.h>
#include <iglu/mousehdl.h>
#include <iglu/tshape.h>

void TAbstButton::GotFocus()
{
  if (State&stFocus) return; // Si ya tiene el foco... nada
  // Tomar el foco y dibujarse.
  TView::GotFocus();
  doDraw();
}

void TAbstButton::LostFocus()
{
  if (State&stFocus) { // Si tiene el foco...
    TView::LostFocus(); // Perderlo y redibujarse
    doDraw();
  }
}

void TAbstButton::KeyEvent(TEvent& e)
{
  // Si se preciono ENTER llamar a Assert.
  if ((State&stFocus) && ((e.keycode&0xff)=='\r'))
    Assert(e);
}

void TAbstButton::MouseEvent(TEvent& e)
{
  TRect r (Bounds); // Copia de la geometria
  getClipArea (r); // Ajustar al area recortada visualmente
  int contain = r.contain (e.pos); // Vandera de "cursor contenido"

  // Si se preciono el boton sobre el cuerpo del boton, seleccionarse
  // y pulsarlo.
  if ((e.type&evMouseDown) && contain)
      SetState (stDown+stSelect,1);
  // Si el boton esta seleccionado (esto es, se pulso antes)
  if (State&stSelect) {
    if (e.type&evMouseMove) // Si se movio ponerlo pulsado o no segun
	SetState (stDown, contain); // este o no el cursor sobre el cuerpo.
    if (e.type&evMouseUp) { // Si se solto el raton
      // Deseleccionarlo y mostrarlo sin pulsar (si es necesario)
      SetState (stDown+stSelect, 0);
      if (contain) // Si se solto dentro del cuerpo del boton
	   Assert (e); // ejecutar las acciones correspondientes.
    }
  }
  if (contain) // Si el cursor esta contenido en el cuerpo
    e.type &=~ evMouse; // del boton bloquear sus eventos
}

void TAbstButton::SetState(int Stat, int active)
{
  int oldSt = State; // Guardar el estado previo
  TView::SetState(Stat, active); // modificar los estados
  if (oldSt!=State) // Si hay cambios redibujar.
    doDraw();
}

void TPushButton::DrawState(int st)
{
  TRect r(Bounds);
#define bOldStyle
#ifdef bOldStyle
  gui_drawrect (Bounds, (State&stFocus)? EGA_LIGHTBLUE:EGA_BLACK);
  r.Grow(1,1);
  TShape shp (r, shpUpBox, 1, EGA_DARKGRAY);
  if (st) shp.type=shpDownBox;
  shp.Draw ();
#else
  int fg, bg, d=2;
  fg = EGA_BLACK;
  bg = EGA_LIGHTGRAY;
  gui_drawrectfill (r, bg);
  if (st) {
    r.a.x += d;
    r.a.y += d;
  } else {
    gui_drawvline (r.b.x-1, r.a.y+2, r.b.y, fg);
    gui_drawvline (r.b.x, r.a.y+2, r.b.y, fg);
    gui_drawhline (r.a.x+2, r.b.y, r.b.x, fg);
    gui_drawhline (r.a.x+2, r.b.y-1, r.b.x, fg);
    r.b.x -= d;   r.b.y -= d;
  }
  gui_drawrect (r, fg);
  r.Grow (2,2);
  gui_drawrect (r, (State&stFocus)? fg:bg);
#endif
}

void TPushButton::Assert (TEvent&) {
  TMessage m(sig, this, 0L);
  EventManager.SendMessage (m);
  if (onClick) onClick (this);
}

void TTextButton::DrawState(int st)
{
  int d=(st)? 2:0;
  TPushButton::DrawState(st);
  gui_outtextxy (Bounds.a.x+Bounds.deltax()/2+d,
		 Bounds.a.y+Bounds.deltay()/2+d,
		 caption, 1, EGA_BLACK, -1);
}

void TCharButton::DrawState(int st)
{
  char s[2] = { caption, 0 };
  int d=(st)? 2:0;
  TPushButton::DrawState(st);
  gui_outtextxy (Bounds.a.x+Bounds.deltax()/2+d,
		 Bounds.a.y+Bounds.deltay()/2+d,
		 s, 1, EGA_BLACK, -1);
}
