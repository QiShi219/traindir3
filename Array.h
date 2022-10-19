/*

Copyright (c) 2007-2008 Giampiero Caprino,
Backer Street Software, Sunnyvale, CA, USA.

This Original Source Code, including accompanying documents, or other related
items, is being provided by the copyright holder(s) subject to the terms of
this License. By obtaining, using and/or copying this Original Source Code,
you agree that you have read, understand, and will comply with the following
terms and conditions of this License:

Permission to use, copy, modify, merge, distribute, and sublicense derivative
work ("Derivative Work") based on the Original Source Code and its
documentation, for any purpose, and without fee or royalty to the copyright
holder(s) is hereby granted, provided that you include the following on ALL
copies of the Derivative Work that you make:

Any pre-existing intellectual property disclaimers, notices, or terms and
conditions.

Notice of any changes or modifications to the Original Source Code,
including the date the changes were made.

NO REPRESENTATION IS MADE ABOUT THE SUITABILITY OF THIS SOURCE CODE FOR ANY
PURPOSE. IT IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

The name and trademarks of copyright holder(s) may NOT be used in advertising
or publicity pertaining to the Original Source Code or Derivative Works
without specific, written prior permission. Title to copyright in the
Original Source Code and any associated documentation will at all times
remain with the copyright holders.

*/

#ifndef _COLLECTION_H
#define _COLLECTION_H

#ifdef _MSC_VER
#if _MSC_VER > 1000
#pragma once
#endif
#pragma warning(disable: 4514) // unreferenced inline/local function has been removed
#endif // _MSC_VER


/*
 * Array
 *
 * A Array template that doesn't require the contained classes
 * to implement all operations required by std::Array.
 * This is only used to store sets of pointers to objects.
 * Memory management then becomes very easy since there is no
 * useless instance copying, and another Array can be used to store
 * a pool of allocated objects, making easy to track memory allocation.
 * Objects can then be treated as references.
 * It's also easy to debug and to dump the content of the Array.
 * It may not be very efficient for lots of objects that are frequently
 * created and destroyed. In such cases, use a linked list.
 */

#include <stdio.h>
#include <stdlib.h>		// malloc et al. on Mac
#if !defined(__unix__) && !defined(__WXMAC__)
#include <malloc.h>
#endif
#include <memory.h>

#define Array_INCREMENT	128

extern	void	Panic(const void *msg);

template <class T>
class Array
{
	template <class TT> friend class ArrayIterator;

public:
	Array()
        {
	    _nInstances = 0;
	    _maxInstances = 0;
	    _instances = NULL;
	}

	~Array()
	{
	    Release();
	}


	void	Release()
	{
	    if(_instances)
		delete[] _instances; // free(_instances);
	    _nInstances = 0;
	    _maxInstances = 0;
	    _instances = NULL;
	}

	void	Clear()
	{
	    _nInstances = 0;
	}

	//	Add an item to the set

	void	Add(const T& item)
	{
	    if(_nInstances >= _maxInstances) {
		_maxInstances += Array_INCREMENT;
		/*
		if(!_instances)
		    _instances = (T *)calloc(sizeof(T), _maxInstances);
		else
		    _instances = (T *)realloc(_instances, sizeof(T) * _maxInstances);
		*/
		if(!_instances)
		    _instances = new T[_maxInstances];
		else {
		    T *insts = new T[_maxInstances];
		    for(int xx = 0; xx < _nInstances; ++xx)
			insts[xx] = _instances[xx];
		    delete[] _instances;
		    _instances = insts;
		}
		if(!_instances)
		    ::Panic("out of memory");

		memset(_instances + _maxInstances - Array_INCREMENT,
				0, Array_INCREMENT * sizeof(T));
	    }
	    _instances[_nInstances++] = item;
	}

	void	Push(const T& item)
	{
	    Add(item);
	}

	T	Pop()
	{
	    if(_nInstances > 0) {
		--_nInstances;
		return _instances[_nInstances];
	    }
	    ::Panic("pop from an empty stack");
	    return _instances[0];
	}

	//	Return item at position 'i'

	T	At(int i)
	{
	    if(i >= _nInstances)
		::Panic("out of bounds access");
	    return _instances[i];
	}

	T	operator[](int i)
	{
	    if(i >= _nInstances)
		::Panic("out of bounds access");
	    return _instances[i];
	}

	T	Last()
	{
	    return At(_nInstances - 1);
	}

	//	Find if 'item' is present in the set

	int	Find(const T& item) const
	{
	    int		i;

	    for(i = _nInstances; --i >= 0; )
		if(item == _instances[i])
		    return i;
	    return -1;
	}

        void    PushIfNotIn(const T& item)
        {
	    int		i;

	    for(i = _nInstances; --i >= 0; )
		if(item == _instances[i])
		    return;
            Add(item);
        }

	//	Remove 'item' from the set (without destroying it)

	void	Remove(T item)
	{
	    RemoveAt(Find(item));
	}

	//	Remove item at position 'i' from the set (without destroying it)

	void	RemoveAt(int i)
	{
	    if(i < 0 || i >= _nInstances)
		return;
	    while(i < _nInstances - 1) {
		_instances[i] = _instances[i + 1];
		++i;
	    }
	    --_nInstances;
	}

	void	RemoveLast()
	{
	    if(_nInstances > 0)
		--_nInstances;
	}

	//	Insert an item at position 'i', moving all items
	//	from position 'i' to the end.

	void	InsertAt(int i, T& item)
	{
	    int	    x;

	    Add(item);	    // make space for one item at the end of the Array
	    for(x = Length() - 1; x > i; --x)
		_instances[x] = _instances[x - 1];
	    _instances[i] = item;
	}

	//
	//

	void	SetAt(int i, T item)
	{
	    if(i < _nInstances)
		_instances[i] = item;
	}

	//	Delete all objects contained in the Array,
	//	effectively releasing all memory used by the Array.
	//	This can only be called when the Array "owns"
	//	the objects stored in it.

	void	DeleteAll()
	{
	    int		i;

	    for(i = _nInstances - 1; i >= 0; --i)
		delete _instances[i];
            Release();
	}

	//	Return how many items are currently in the set

	int	Length() const
	{
	    return _nInstances;
	}

	//	Allocate enough entries to store 'maxInstances' items.
	//	This is useful when a Array must be copied into another
	//	Array, to avoid unnecessary realloc() calls.

	void	Reserve(int maxInstances)
	{
	    if(_maxInstances >= maxInstances)
		return;
	    if(!_instances)
		_instances = new T[maxInstances]; //(T *)malloc(maxInstances * sizeof(T));
	    else {
                T *insts = new T[maxInstances];
                for(int xx = 0; xx < _nInstances; ++xx)
                    insts[xx] = _instances[xx];
                delete[] _instances;
                _instances = insts;
//		_instances = (T *)realloc(_instances, maxInstances * sizeof(T));
            }
	    if(!_instances)
		::Panic("out of memory");
	    _maxInstances = maxInstances;
	}

	//	Sort items according to the policy specified by 'sorter'

	void	Sort(int (*sorter)(const void *, const void *))
	{
	    qsort(_instances, _nInstances, sizeof(T), sorter);
	}

///private:
	T	*_instances;		// array of (pointer) instances
	int	_nInstances;		// how many instances are stored in the Array
	int	_maxInstances;		// how many entries can m_instances[] hold (expanded on demand)
};


/*
 *  An iterator to hide sequential access to Array elements.
 */

template <class T>
class ArrayIterator
{
public:
	ArrayIterator(Array<T>& Array)
	{
	    _index = 0;
	    _Array = &Array;
	};

	ArrayIterator(Array<T> *Array)
	{
	    _index = 0;
	    _Array = Array;
	};

	~ArrayIterator()
	{
	};

	T   First()
	{
	    _index = 0;
	    if(!_Array->_nInstances)
		return 0;
	    return _Array->_instances[0];
	};

	T   Last()
	{
	    if(!_Array->_nInstances)
		return 0;
	    _index = _Array->_nInstances - 1;
	    return _Array->_instances[_index];
	};

	T   Next()
	{
	    if(_index + 1 < _Array->_nInstances)
		return _Array->_instances[++_index];
	    return 0;
	}

	T   Prev()
	{
	    if(_index > 0)
		return _Array->_instances[--_index];
	    return 0;
	}

private:
	Array<T>  *_Array;
	int	    _index;
};


/*
 *  ManagedArray
 *
 *  Automatically destroy objects contained in a Array
 *  when the Array is destroyed.
 */


template <class T>
class ManagedArray : public Array<T>
{
public:
	ManagedArray()
	{
	}

	~ManagedArray()
	{
	    Clean();
	}


	void	DeleteAt(int pos)
	{
	    if(pos >= Array<T>::Length())
		return;

	    T	    item = Array<T>::At(pos);
	    Array<T>::RemoveAt(pos);
	    delete item;
	}



	void	Clean()
	{
	    Array<T>::DeleteAll();
	}
};

#endif // _COLLECTION_H
