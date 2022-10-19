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


#include "HostThread.h"

#ifdef WIN32

HostThread::HostThread(ThreadFunction entryPoint, void *obj)
{
	_hThread = CreateThread(NULL,		// no security attributes
		0,          			// use default stack size
		(LPTHREAD_START_ROUTINE)entryPoint,// thread function
		obj, // (LPVOID)ProcessID,      // argument to thread function
		0,				// use default creation flags
		&_threadId);
}


HostThread::~HostThread()
{
    TerminateThread(&_hThread, 0);
}


void    HostThread::Sleep(int msec)
{
    ::Sleep(msec);
}

#else // Linux code below

#include <unistd.h>

HostThread::HostThread(ThreadFunction entryPoint, void *obj)
{
    pthread_create(&_thread, NULL, entryPoint, obj);
}

HostThread::~HostThread()
{
    pthread_kill(_thread, SIGKILL);
}

void    HostThread::Sleep(int msec)
{
        usleep(msec);
}

#endif

