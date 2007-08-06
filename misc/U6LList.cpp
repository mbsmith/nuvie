/*
 *  U6LList.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Sat Mar 15 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "nuvieDefs.h"

#include "U6LList.h"

 U6LList::U6LList()
{
 head = NULL;
 tail = NULL;
 cur = NULL;
}

 U6LList::~U6LList()
{
 removeAll();
}

bool U6LList::add(void *data)
{
 U6Link *link;

 link = new U6Link;
 if(link == NULL)
   return false;

 if(tail == NULL)
   head = tail = link;
 else
   {
    link->prev = tail;
    tail->next = link;
    tail = link;
   }

 link->data = data;

 return true;
}

 bool U6LList::addAtPos(uint32 pos, void *data)
{
 uint32 i;
 U6Link *link, *prev, *new_link;

 new_link = new U6Link;
 if(new_link == NULL)
   return false;

 new_link->data = data;

 if(pos == 0 || head == NULL) // pos at head or list empty
   {
    if(head != NULL)
      head->prev = new_link;
    new_link->next = head;
    head = new_link;
    if(tail == NULL)
      tail = head;
   }
 else
   {
    prev=NULL;
    for(link=head,i=0;link != NULL && i < pos;i++)
      {
       prev=link;
       link=link->next;
      }

    if(prev == tail)
      {
       new_link->prev = tail;
       tail->next = new_link;
       tail = new_link;
      }
    else
      {
       new_link->prev = prev;
       new_link->next = prev->next;
       prev->next = new_link;
      }
   }

 return true;
}

uint32 U6LList::findPos(void *data)
{
 U6Link *link;
 uint32 pos;
 
 for(pos=0,link = start(); link != NULL; link = link->next,pos++)
  {
   if(link->data == data)
     return pos;
  }
  
 return 0;
}

//replace the first instance of old with new.
bool U6LList::replace(void *old_data, void *new_data)
{
  U6Link *link;
  
  for(link = start(); link != NULL; link = link->next)
  {
    if(link->data == old_data)
    {
      link->data = new_data;
      return true;
    }
  }
  
  return false;
}

bool U6LList::remove(void *data)
{
 U6Link *link;
 U6Link *prev;

 if(head == NULL)
   return false;

 if(head->data == data) // remove head
   {
    link = head;
    head = head->next;
    if(head == NULL) // empty list
      tail = NULL;
    else
      head->prev = NULL;

    delete link;

    return true;
   }

 for(link = prev = head;link != NULL;)
  {
   if(link->data == data)
    {
     prev->next = link->next;

     if(link == tail)
         tail = prev;

     delete link;

     if(prev->next)
      {
       link = prev->next;
       link->prev = prev;
      }

     return true;
    }

   prev = link;
   link = link->next;
  }

 return false;
}

bool U6LList::removeAll()
{
 U6Link *tmp_link, *link;

 for(link=head;link != NULL;)
  {
   tmp_link = link;
   link=link->next;

   delete tmp_link;
  }

 head = NULL;
 tail = NULL;
 cur = NULL;

 return true;
}

uint32 U6LList::count()
{
 uint32 i;
 U6Link *link;

 for(i=0,link=head; link != NULL; link=link->next)
  {
   i++;
  }

 return i;
}

 U6Link *U6LList::start()
{
 cur = head;

 return cur;
}

U6Link *U6LList::end()
{
 cur = tail;

 return cur;
}

U6Link *U6LList::next()
{
 if(cur == tail)
   return NULL;

 cur = cur->next;

 return cur;
}

U6Link *U6LList::prev()
{
 if(cur == head)
   return NULL;

 cur = cur->prev;

 return cur;
}

U6Link *U6LList::gotoPos(uint32 pos)
{
 U6Link *link;
 uint32 i;

 for(link=head,i=0;link != NULL && i < pos;i++)
   link = link->next;

 return link;
}
