/*	TVector.cpp - Created by Giampiero Caprino

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

#include <stdio.h>
#include <string.h>

#if !defined(__unix__) && !defined(__WXMAC__)
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include <math.h>
#include "wx/ffile.h"
#include "wx/textfile.h"
#include "Traindir3.h"

/*	Vector handling routines.
 *
 *	Vectors are used to store path information during the simulation.
 *	Note that these are not the paths that automatically compute
 *	the entry/exit times from a .pth file.
 *	The simulation paths are the list of track elements that the
 *	train will have to travel trough.
 *	A path always ends at the next signal or at the end of the track.
 */

#define USE_VECTOR_POOL 1

Vector::Vector(int hint)
{
        _size = 0;
        _maxelem = hint;
        if(hint) {
            _ptr = (TrackBase **)malloc(hint * sizeof(TrackBase *));
            _flags = (char *)malloc(hint * sizeof(char));
        } else {
            _ptr = 0;
            _flags = 0;
        }
        _pathlen = 0;
        _nextFree = 0;
}

Vector::~Vector()
{
        if(_ptr)
            free(_ptr);
        _ptr = 0;
        if(_flags)
            free(_flags);
        _flags = 0;
}

Track	*Vector::TrackAt(int index)
{
	if(index >= _size) {
	    wxSnprintf(alert_msg, sizeof(alert_msg)/sizeof(wxChar),
		wxT("Bad index %d: only %d elements in vector!\n"), index, _size);
	    Traindir::Panic();
	}
	return (Track *)_ptr[index];
}

Track	*Vector::FirstTrack()
{
	if(_size < 1) {
	    wxSnprintf(alert_msg, sizeof(alert_msg)/sizeof(wxChar), wxT("No tracks in vector!\n"));
	    Traindir::Panic();
	}
	return (Track *)_ptr[0];
}

Track	*Vector::LastTrack()
{
	if(_size < 1) {
	    wxSnprintf(alert_msg, sizeof(alert_msg)/sizeof(wxChar), wxT("No tracks in vector!\n"));
	    Traindir::Panic();
	}
	return (Track *)_ptr[_size - 1];
}

int	Vector::FlagAt(int index)
{
	if(index >= _size) {
	    wxSnprintf(alert_msg, sizeof(alert_msg)/sizeof(wxChar),
		wxT("Bad index %d: only %d elements in vector!\n"), index, _size);
	    Traindir::Panic();
	}
	return _flags[index];
}


void	Vector::Add(TrackBase *e, int flag)
{
	if(_size >= _maxelem) {
	    if(_ptr) {
                _maxelem *= 2;
		_ptr = (TrackBase **)realloc(_ptr, _maxelem * sizeof(TrackBase *));
		_flags = (char *)realloc(_flags, _maxelem * sizeof(char));
	    } else {
	        _maxelem = 20;
		_ptr = (TrackBase **)malloc(_maxelem * sizeof(TrackBase *));
		_flags = (char *)malloc(_maxelem * sizeof(char));
	    }
	}
	_flags[_size] = flag;
	_ptr[_size++] = e;
}

void	Vector::Empty()
{
	_size = 0;
}

void	Vector::DeleteAt(int del)
{
	while(del + 1 < _size) {
	    _flags[del] = _flags[del + 1];
	    _ptr[del] = _ptr[del + 1];
	    ++del;
	}
	--_size;
	ComputeLength();
}

int	Vector::Find(TrackBase *trk)
{
	int	i;

	for(i = 0; i < _size; ++i)
	    if(_ptr[i] == trk)
		return i;
	return -1;
}

void	Vector::ComputeLength()
{
	int	cx;
	TrackBase *trk;
	_pathlen = 0;
	
	for(cx = 0; cx < _size; ++cx) {
	    trk = _ptr[cx];
	    _pathlen += trk->length;
	}
}

void	Vector::ContainBusy(int len){

	int	cx;
	bool flag = true;
	TrackBase *trk;
	_distostp = _ptr[0]->length;
	Train  *trn;

	for(cx = 1; _distostp < _pathlen; ++cx) {
	    trk = _ptr[cx];

		//if(flag&&((trk->fgcolor == conf.fgcolor)||(trk->fgcolor == color_bise)))
		if(trk->fgcolor == color_bise||(trn = findTrain(trk->x, trk->y)) != 0)
			return;
		_distostp += trk->length;
	}
	if(_distostp<6000)
		int ii = 1;

}


void	Vector::DeleteTrack(TrackBase *trk)
{
	int	i = Find(trk);
	if(i < 0)
	    return;
	DeleteAt(i);
}


void	Vector::Reverse()
{
	int	i, j;
	TrackBase *trk;
	int	f;

	j = _size - 1;
	i = 0;
	while(i < j) {
	    trk = _ptr[i];
	    _ptr[i] = _ptr[j];
	    _ptr[j] = trk;
	    f = _flags[i];
	    _flags[i] = _flags[j];
	    _flags[j] = f;
	    ++i;
	    --j;
	}
}

void	Vector::Insert(TrackBase *trk, int f)
{
	int	i;

	Add(trk, f);	    // just to make space
	for(i = _size - 1; i > 0; --i) {
	    _ptr[i] = _ptr[i - 1];
	    _flags[i] = _flags[i - 1];
	}
	_ptr[0] = trk;
	_flags[0] = f;
	ComputeLength();
}


void	Vector::Insert(Vector *newPath)
{
	int	i;
	TrackBase *trk;
	int	f;

	if(!newPath)
	    return;
	for(i = newPath->_size; --i >= 0; ) {
	    trk = newPath->TrackAt(i);
	    f = newPath->FlagAt(i);
	    Insert(trk, f);
	}
}


void	Vector_merge(Vector *dest, Vector *src)
{
	int	i;
	TrackBase *trk;

	for(i = 0; i < dest->_size; ++i) {
	    trk = src->TrackAt(i);
	    if(dest->Find(trk) < 0)
		dest->Add(trk, src->FlagAt(i));
	}
	dest->ComputeLength();
}

void Vector_Print(Vector *v, wxChar * str, short pos)
{
	TrackBase *trk;
	int i;

	/* For debugging purposes */

	wxPrintf(wxT("%s = ["), str);
	for(i = 0; i < v->_size; i++) {
	trk = v->_ptr[i];
	if(trk) {
	    if(pos == i) 
		wxPrintf(wxT("[*%hd,%hd*]"), trk->x, trk->y);
	    else
		wxPrintf(wxT("[%hd,%hd]"), trk->x, trk->y);
	    }
	}
	wxPrintf(wxT("]\n"));
}

class VectorPool {
public:
        VectorPool();
        ~VectorPool();
        Vector  *New(int hint);
        void    Delete(Vector *p);

        Vector  *_pool;
} vector_pool;

VectorPool::VectorPool()
{
        _pool = 0;
}

VectorPool::~VectorPool()
{
        Vector  *v;

        while(_pool) {
            v = _pool->_nextFree;
            delete _pool;
            _pool = v;
        }
}

Vector  *VectorPool::New(int hint)
{
        if(!_pool)
            return new Vector(hint);
        Vector *p = _pool;
        _pool = p->_nextFree;
        p->_nextFree = 0;
        return p;
}

void    VectorPool::Delete(Vector *v)
{
        v->Empty();
        v->_nextFree = _pool;
        _pool = v;
}

Vector  *new_Vector(int hint)
{
#if USE_VECTOR_POOL
        return vector_pool.New(hint);
#else
        return new Vector(hint);
#endif
}

void    Vector_delete(Vector *v)
{
#if USE_VECTOR_POOL
        vector_pool.Delete(v);
#else
        delete v;
#endif
}
