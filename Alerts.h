/*	Alerts.h - Created by Giampiero Caprino

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

#ifndef _ALERTS_H
#define _ALERTS_H

#include "LinkedList.h"

class   AlertLine : public LinkItem<AlertLine>
{
public:
        AlertLine() {  _text = 0; };
        virtual ~AlertLine() { free(_text); }
        Char        *_text;
        int         _modTime;
};
extern  AlertLine *firstAlert, *lastAlert;

class Alerts : public SynchronizedList<AlertLine>
{
public:
        Alerts();
        ~Alerts();

        AlertLine   *AddLine(Char *text);
};
extern  Alerts  alerts;

#endif // _ALERTS_H
