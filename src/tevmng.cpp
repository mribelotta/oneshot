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

#include <iglu/tevent.h>

// Definir un maximo en la cola de eventos para no sobrecargar la memoria.
#define MaxQueueLen 10

TEventManager EventManager;

void TEventManager::putevent (TEvent& e)
{
  if ((++cnt)<MaxQueueLen) {
    TEvent *ptr = new TEvent(e);
    if (!base && !head)
      base=head=ptr;
    else {
      head->next=ptr;
      head=ptr;
    }
  }
}

void TEventManager::SendMessage (TMessage& m)
{
  TEvent e;
  e.type = evMessage;
  e.msg = m;
  putevent(e);
}

void TEventManager::getevent (TEvent& e)
{
  e.GetEvent();
  if (!isempty()) {
    e.type |= evMessage;
    e.msg = base->msg;
    base=base->next;
    if (!base) head=base;
    cnt--;
  }
}

void TEventManager::clearqueue ()
{
  while (base) {
    TEvent *tmp = base;
    base=base->next;
    delete tmp;
  }
  base=head=NULL;
  cnt=0;
}
