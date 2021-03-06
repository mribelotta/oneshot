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

#include <iglu/tlist.h>
#include <stdlib.h>
#include <string.h>

TANode::TANode (char *s, TObject *o)
{
  next = prev = NULL;
  object = o;
  name = strdup (s);
}

TANode::~TANode ()
{
  delete object;
  free (name);
}

/*******************************************************************/

TList::TList (): base(NULL), head (NULL), count (0)
{
}

TList::~TList ()
{
  while (base) {
    TANode *t = base;
    base = base->next;
    delete t;
  }
}


void TList::addObject (char *name, TObject *obj)
{
  TANode *ptr = new TANode (name, obj);
  if (!base && !head)
    base = ptr;
  else {
    head->next = ptr;
    ptr->prev = head;
  }
  head = ptr;
  count++;
}


int TList::indexOf (char *name)
{
  TANode *t;  int i;
  for (i=0, t=base; t; t=t->next, i++)
    if (strcmp(t->getname(), name)==0)
      return i;
  return -1;
}


int TList::indexOf (TObject *obj)
{
  TANode *t;  int i;
  for (i=0, t=base; t; t=t->next, i++)
    if (t->getobject()==obj)
      return i;
  return -1;
}

TANode *TList::getnode (int idx)
{
  TANode *tmp = base;  int i=0;
  while (tmp) {
    if (i==idx)
      return tmp;
    tmp = tmp->next;
    i++;
  }
  return NULL;
}

void TList::delObject (int idx)
{
  if ((idx>=0) && (idx<count)) {
    TANode *ptr = getnode (idx);
    if (ptr) {
      if (ptr->prev)
	ptr->prev->next = ptr->next;
      if (ptr->prev)
	ptr->next->prev = ptr->prev;
      if (ptr==base)
	base = base->next;
      if (ptr==head)
	head = head->prev;
      delete ptr;
      count--;
    }
  }
}

char *TList::getname (int idx)
{
  TANode *t = getnode (idx);
  if (t) return t->getname();
  return NULL;
}

TObject *TList::getobject (int idx)
{
  TANode *t = getnode (idx);
  if (t) return t->getobject();
  return NULL;
}
