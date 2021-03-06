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
#include <stdlib.h>
#include <string.h>
#include <graphics.h>
#include <iglu/screen.h>

/* Rutinas de bajo nivel para tapar la basura que es las BGI...
 *
 */

// Si no funca el modo 640x480x4bpp en WindowsXP activar esto que
// inicia el modo 640x350x4bpp pero este no es flat (el otro era planar)
// porque los ingenieros de mocochot no dan mas soporte a DOS...
// (En Linux siiiii!!!!)
//#define WindowsXP

// Metricas de la pantalla y demas...
int Ht, Wt, MaxX, MaxY;

// Area de recorte de dibujo actual (realmente deverian haverle puesto mas
// huebo los muchachos de Borland al hacer esta parte... loco te pagan!!!)
static TRect actualclip;

extern void _outtextxy (int, int, char*); // Vease src/fontm.cpp

/* Como los subnormales que hicieron la BGI se les ocurrio que cada
 * clip que se hiciera (ver setviewport/getviewport) redefinia el
 * sistema de coordenadas a la esquina superior derecha del viewport
 * (hay que ser muy entreverados!!!) yo tengo que transformal las
 * coordenadas absolutas que me mandan los componentes a virtuales
 * (habrase visto semejante taradez) que es lo que espera encontrar
 * las BGI (que despues hacen el trabajo inverso al mio!!!)
 */
void virtoabs (int& x, int& y) {
  x-=actualclip.a.x;
  y-=actualclip.a.y;
}

void virtoabs (int& ax, int& ay, int& bx, int& by) {
  ax-=actualclip.a.x;
  ay-=actualclip.a.y;
  bx-=actualclip.a.x;
  by-=actualclip.a.y;
}

void initGUI (void)
{
#ifndef WindowsXP
  int gd=DETECT, gm;
#else
  int gd=VGA, gm=VGAMED;
#endif
  initgraph(&gd, &gm, NULL);
  Ht = gui_texth("$");
  Wt = gui_textw("_");
  MaxX = getmaxx();
  MaxY = getmaxy();

  initMouseHandle();
  gui_setclip(Rect(0,0,getmaxx(),getmaxy()));
}

void doneGUI ()
{
  doneMouseHandle();
  closegraph();
}

//////////////////////////////////////////////////////////////////////

void gui_drawpixel(int x, int y, int color) {
  MouseHide();
  virtoabs(x,y);
  putpixel(x, y, color);
  MouseShow();
}

int gui_getpixel(int x, int y) {
  virtoabs(x,y);
  return getpixel(x,y);
}

void gui_drawline(int xa, int ya, int xb, int yb, int cl) {
  MouseHide();
  virtoabs(xa,ya,xb,yb);
  setcolor(cl);
  line(xa,ya,xb,yb);
  MouseShow();
}

void gui_drawvline(int x, int y, int dx, int cl) {
  MouseHide();
  virtoabs(x,y);
  dx-=actualclip.a.y;
  setcolor(cl);
  line(x,y,x,dx);
  MouseShow();
}

void gui_drawhline(int x, int y, int dx, int cl) {
  MouseHide();
  virtoabs(x,y);
  dx-=actualclip.a.x;
  setcolor(cl);
  line(x,y,dx,y);
  MouseShow();
}

void gui_drawpolyline(int n, int *points, int color) {
  int i;
  // VirToAbs para todos los puntos!!!
  for (i=0; i<(n<<1); i+=2) {
    points[i]   -= actualclip.a.x;
    points[i+1] -= actualclip.a.y;
  }
  MouseHide();
  setcolor(color);
  drawpoly(n, points);
  MouseShow();
  // Pero como los estamos modificando, tenemos que debilverlos a
  // su estado original (por eso no sarparse con la cantidad de
  // puntos a renderizar)
  for (i=0; i<(n<<1); i+=2) {
    points[i]   += actualclip.a.x;
    points[i+1] += actualclip.a.y;
  }
}

void gui_drawrect(TRect& r2, int color) {
  TRect r=r2;
  virtoabs (r.a.x,r.a.y,r.b.x,r.b.y);
  MouseHide();
  setcolor(color);
  rectangle(r.a.x, r.a.y, r.b.x, r.b.y);
  MouseShow();
}

/* Si, si, si!!!! Ahora y por unica vez, la famosa funcion que los Mac
 * traia por ahi en su BIOS (junto con un monton mas) y que los PC no
 * conocieron hasta la llegada del guindos exispe!!!!!
 */
void gui_drawrrect(TRect& r2, int color) {
  TRect r=r2;
  int l = 4;
  virtoabs (r.a.x,r.a.y,r.b.x,r.b.y);
  MouseHide();
  setcolor(color);
  line (r.a.x+l, r.a.y, r.b.x-l, r.a.y);
  line (r.a.x+l, r.b.y, r.b.x-l, r.b.y);
  line (r.a.x, r.a.y+l, r.a.x, r.b.y-l);
  line (r.b.x, r.a.y+l, r.b.x, r.b.y-l);

  line (r.a.x+l, r.a.y, r.a.x, r.a.y+l);
  line (r.b.x-l, r.a.y, r.b.x, r.a.y+l);

  line (r.a.x, r.b.y-l, r.a.x+l, r.b.y);
  line (r.b.x, r.b.y-l, r.b.x-l, r.b.y);

  MouseShow();
}

void gui_drawrectfill(TRect& r2, int color) {
  TRect r=r2;
  virtoabs (r.a.x,r.a.y,r.b.x,r.b.y);
  MouseHide();
  setfillstyle(SOLID_FILL, color);
  bar(r.a.x, r.a.y, r.b.x, r.b.y);
  MouseShow();
}

void gui_drawcircle(int x, int y, int radius, int color) {
  virtoabs(x,y);
  MouseHide();
  setcolor(color);
  circle(x, y, radius);
  MouseShow();
}

void gui_drawcirclefill(int x, int y, int radius, int color) {
  virtoabs(x,y);
  MouseHide();
  setfillstyle(SOLID_FILL, color);
  setcolor(color);
  pieslice(x, y, 0, 360, radius);
  MouseShow();
}

void gui_outtextxy(int x, int y, char *s, int center, int fg, int bg)
{
  if (center)
    (y-=Ht/2, x-=gui_textw(s)>>1);
  if (bg>-1)
    gui_drawrectfill(TRect(x, y, x+gui_textw(s), y+Ht), bg);
  setcolor(fg);
  MouseHide();
  virtoabs(x,y);
  _outtextxy(x,y,s);
  MouseShow();
}

int gui_textw(char *s)
{
  return textwidth(s);
}

int gui_texth(char *s)
{
  return textheight(s);
}

void gui_setclip(TRect& r)
{
  TRect t(r);
  // De yapa, si nos safamos con el tema de las coordenadas el
  // setviewport guacho, directamente ignora las llamadas, tengo
  // que hacer todo yoooooooooooooooooooooooooo buaaaaaaaaaaaaaa
  if (t.a.x<0) t.a.x=0;
  if (t.a.y<0) t.a.y=0;
  if (t.b.x>getmaxx()) t.b.x = getmaxx();
  if (t.b.y>getmaxy()) t.b.y = getmaxy();
  actualclip = t;
  setviewport(t.a.x, t.a.y, t.b.x, t.b.y, 1);
}

void gui_getclip(TRect& r)
{
  viewporttype vp;
  getviewsettings(&vp);
  r = TRect(vp.left, vp.top, vp.right, vp.bottom);
}

void gui_wmode(int mode)
{
  setwritemode(mode);
}

void gui_drawelipse (int ax, int ay, int bx, int by, int cl)
{
  virtoabs(ax,ay,bx,by);
  int rx=(bx-ax)>>1, ry=(by-ay)>>1, x=ax+rx, y=ay+ry;
  MouseHide ();
  setcolor (cl);
  // Solamente a los de Borland se les ocurre como parametros
  // apra una elipse el angulo de inicio y fin (evidentemente
  // lo havian pensado para el asunto de hacer graficos de torta
  // pero las tortas se las comieron las ratas X-P)
  ellipse (x,y,0,360,rx,ry);
  MouseShow ();
}

void gui_drawfillelipse (int ax, int ay, int bx, int by, int cl)
{
  virtoabs(ax,ay,bx,by);
  int rx=(bx-ax)>>1, ry=(by-ay)>>1, x=ax+rx, y=ay+ry;
  MouseHide ();
  setcolor (cl);
  setfillstyle(SOLID_FILL, cl);
  fillellipse (x,y,rx,ry);
  MouseShow ();
}

/* Realmente, esto es algo que si lo hubieran implementado los de
 * Borland podrian haverce ganado mi simpatia. Por eso pienso que
 * son unos subnormales que no tiene un joraca que hacer y se dedican
 * a joder con el TurboPascal en ves de laburar pa lo que se lespaga
 * (Solo mirar el mal compilador de C++ que tienen, ni siquiera soporta
 * los nombres de espacio!!!)
 */
void gui_drawpixmap (int x, int y, int w, int h, char *b, char *p)
{
  int i,j,k;
  virtoabs(x,y);
  MouseHide ();
  for (j=0, k=0; j<h; j++)
    for (i=0; i<w; i++, k++)
      if (p[b[k]-'0']>=0)
	putpixel(x+i, y+j, p[b[k]-'0']);
  MouseShow ();
}

void gui_flush ()
{
  // Esto es por si la arquitectura permite hacer Double buffer
  // Cosa que si me pusiera a implementar con las BGI quedaria mas
  // tarado de lo que ya estoy!!!!!!! Huuuuuuuuuuueeeeeeeeeeeeee
}
