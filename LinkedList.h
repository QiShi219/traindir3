/*	LinkedList.h - Created by Giampiero Caprino

This file is part of Train Director 3

Train Director is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; using exclusively version 2.
It is expressly forbidden the use of higher versions of the GNU
General Public License.

Train Director is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Train Director; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H

#ifdef _MSC_VER
#if _MSC_VER > 1000
#pragma once
#endif
#pragma warning(disable: 4514) // unreferenced inline/local function has been removed
#endif // _MSC_VER

/*
 * LinkedList
 */

#include <stdio.h>
#include <stdlib.h>		// malloc et al. on Mac
#ifndef __WXMAC__
#include <malloc.h>
#endif
#include <memory.h>

#include "HostLock.h"
#include "Array.h"

extern	void	Panic(const void *msg);

template <class T>
class LinkItem
{
public:
        LinkItem() {
            _next = _prev = 0;
        };

        virtual ~LinkItem() { };

        T   *_next;
        T   *_prev;
};


template <class T>
class LinkedList
{
public:
        LinkedList() {
            _firstItem = _lastItem = 0;
            _nItems = 0;
        }
        virtual ~LinkedList() {
            Clear();
        }

        virtual void Clear() {
            while(_firstItem) {
                T *obj = _firstItem;
                _firstItem = obj->_next;
                delete obj;
            }
            _firstItem = _lastItem = 0;
            _nItems =0;
        }

        T   *AppendNewItem() {
            T   *obj = new T();
            if(!_firstItem)
                _firstItem = obj;
            else
                _lastItem->_next = obj;
            obj->_prev = _lastItem;
            _lastItem = obj;
            return obj;
        }

        T   *Remove(T *item) {
            T *next = item->_next;
            if(item->_prev)
                item->_prev->_next = item->_next;
            else
                _firstItem = item->_next;
            if(item->_next)
                item->_next->prev = item->_prev;
            else
                _lastItem = item->_prev;
            return next;
        }

        T   *_firstItem;
        T   *_lastItem;
        int _nItems;
};

class   EventListener : public LinkItem<EventListener>
{
public:
        EventListener() { };
        virtual ~EventListener() { };
        virtual void OnEvent(void *list) { };
};


template <class T>
class SynchronizedList : public LinkedList<T>
{
public:
        SynchronizedList() { };
        virtual ~SynchronizedList() { Unlock(); };

        void    AddListener(EventListener *l) {
            _listenersLock.Lock();
            _listeners.Add(l);
            _listenersLock.Unlock();
        }

        void    RemoveListener(EventListener *l) {
            _listenersLock.Lock();
            _listeners.Remove(l);
            _listenersLock.Unlock();
        }

        void    NotifyListeners() {
            _listenersLock.Lock();
            int     i;
            for(i = 0; i < _listeners.Length(); ++i) {
                EventListener *l = _listeners.At(i);
                l->OnEvent(this);
            }
            _listenersLock.Unlock();
        }

        void    Lock() { _lock.Lock(); };
        void    Unlock() { _lock.Unlock(); };
        HostLock    _lock;
        HostLock    _listenersLock;
        Array<EventListener *> _listeners;
};


#endif // _LINKEDLIST_H
