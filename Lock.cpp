/*

Copyright (c) 2007-2011 Giampiero Caprino,
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


#include "Lock.h"

#ifdef WIN32

#include "windows.h"

Lock::Lock()
{
	_theLock = (void *)calloc(sizeof(CRITICAL_SECTION), 1);
	InitializeCriticalSection((CRITICAL_SECTION *)_theLock);
}


Lock::~Lock()
{
	DeleteCriticalSection((CRITICAL_SECTION *)_theLock);
	free(_theLock);
	_theLock = 0;
}


void	Lock::Set()
{
	EnterCriticalSection((CRITICAL_SECTION *)_theLock);
}

void	Lock::Clear()
{
	LeaveCriticalSection((CRITICAL_SECTION *)_theLock);
}

#else // non-WIN32 below

#include <stdlib.h>		// malloc et al. on Mac
#ifndef MACOS
#include <malloc.h>
#endif
#include <memory.h>
#include <string.h>
#include <pthread.h>

static	pthread_mutex_t		mutex_initializer = PTHREAD_MUTEX_INITIALIZER;

Lock::Lock()
{
	_theLock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	memcpy(_theLock, &mutex_initializer, sizeof(pthread_mutex_t));
}

Lock::~Lock()
{
	if(_theLock)
	{
	    free(_theLock);
	    _theLock = 0;
	}
}


void	Lock::Set()
{
	pthread_mutex_lock((pthread_mutex_t *)_theLock);
}


void	Lock::Clear()
{
	pthread_mutex_unlock((pthread_mutex_t *)_theLock);
}

#endif // WIN32


