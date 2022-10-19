/*	run.h - Created by Giampiero Caprino

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
#include "Traindir3.h"
#include "Itinerary.h"

class Segment {
public:
        Segment() { _id = 0; _start = _end = 0; _path = 0; }
        ~Segment() { }
        Train   *GetTrain();
        int     GetTravelDistance(Train *trn);

        int     _id;
        Track   *_start;
        Track   *_end;
        Vector  *_path;
};

class   Ai {
public:
        Ai() { count = 0; };
        ~Ai() { };

        int     FindSegments(Track *layout);
        int     FindSegmentPath(Segment *segment, Track *start, int dir);
        Signal  *FindLinkedSignal(Track *trk) const;
        int     WalkSegment(Track *trk, int dir);
        void    DumpColumns();

        void    CollectSensors();

        Array<Signal *> signals;
        Array<Track *> switches;
        Array<Track *> blocks; // poor man's segment
        Array<Track *> stations;
        Array<Segment *> segments;

        int     FindStation(Char *name);
        int     FindStation(Track *trk) const;

        Array<SignalAspect *>aspects;
        Array<Char *> actions;

        int     FindAspect(const Char *name);
        int     FindAction(const Char *name);

        int count;
};

extern  Ai  gAi;
