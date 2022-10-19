/*	ai.cpp - Created by Giampiero Caprino

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
#include "MotivePower.h"
#include "ai.h"

Ai      gAi;

bool    loggingEvents = false;
FILE    *logEventsFile;

static  int     get_all_signals(Track *layout, Array<Signal *>& signals)
{
        signals.Clear();
        for (Track *t = layout; t; t = t->next) {
            if (t->type == TSIGNAL)
                signals.Add((Signal *)t);
        }
        return signals.Length();
}

static  int     get_all_switches(Track *layout, Array<Track *>& switches)
{
        switches.Clear();
        for (Track *t = layout; t; t = t->next) {
            if (t->type == SWITCH)
                switches.Add(t);
        }
        return switches.Length();
}

static  int     get_all_blocks(Track *layout, Array<Track *>& blocks)
{
        blocks.Clear();
        for (Track *t = layout; t; t = t->next) {
            if (t->type == TEXT && t->station && t->station[0] == '<') {
                if (t->wlinkx && t->wlinky) {
                    Track *t1 = find_track(layout, t->wlinkx, t->wlinky);
                    blocks.Add(t1);
                } else if (t->elinkx && t->elinky) {
                    Track *t1 = find_track(layout, t->elinkx, t->elinky);
                    blocks.Add(t1);
                }
            }
        }
        return blocks.Length();
}

static  int     get_all_stations(Track *layout, Array<Track *>& stations)
{
        stations.Clear();
        for (Track *t = layout; t; t = t->next) {
            if (t->type == TRACK && t->station)
                stations.Add(t);
        }
        return stations.Length();
}

static  void    get_all_aspects(Track *layout, Ai& ai)
{
	SignalAspect *asp;
        int i;

        ai.aspects.Clear();
        ai.actions.Clear();

        // add built-in aspects and actions
        // TODO: green+green, orange+orange

        asp = new SignalAspect();
        asp->_name = "red";
        asp->_action = "stop";
        ai.aspects.Add(asp);
        ai.actions.Add(asp->_action);
        asp = new SignalAspect();
        asp->_name = "green";
        asp->_action = "proceed";
        ai.aspects.Add(asp);
        ai.actions.Add(asp->_action);

        for (i = ai.signals.Length(); --i >= 0; ) {
            Signal *sig = ai.signals.At(i);
	    SignalInterpreterData *interp = (SignalInterpreterData *)sig->_interpreterData;
            if (!interp)
                continue;
            for (SignalAspect *asp = interp->_aspects; asp; asp = asp->_next) {
                if (ai.FindAspect(asp->_name) < 0)
                    continue;
                ai.aspects.Add(asp);
                if (ai.FindAction(asp->_action) < 0)
                    continue;
                ai.actions.Add(asp->_action);
            }
        }
}

int     Ai::FindAspect(const Char *name)
{
        int i;
        for (i = aspects.Length(); --i >= 0; ) {
            SignalAspect *asp = aspects[i];
            if (!strcmp(name, asp->_name))
                return i;
        }
        return -1;
}

int     Ai::FindAction(const Char *name)
{
        int i;
        for (i = actions.Length(); --i >= 0; ) {
            Char *act = actions[i];
            if (!strcmp(name, act))
                return i;
        }
        return -1;
}

int     Ai::FindStation(Char *name)
{
        int i;
        for (i = stations.Length(); --i >= 0; ) {
            Track *trk = (Track *)stations.At(i);
            if (!strcmp(name, trk->station))
                return i;
        }
        return -1;
}

int     Ai::FindStation(Track *trk) const
{
        int i;
        i = stations.Find(trk);
        return i;
}

Signal  *Ai::FindLinkedSignal(Track *trk) const
{
        if (trk->wsignal)
            return trk->wsignal;
        return trk->esignal;
}

int     Ai::FindSegmentPath(Segment *seg, Track *trk, int dir)
{
        trkdir  ndir = (trkdir)dir;
        int     dx = 1;
        int     dy = 0;
        Track   *tTrack;
        Track   *nxtTrack;
        Signal *sig;

	if(!seg->_path)
	    seg->_path = new_Vector(0);
	else
	    seg->_path->Empty();
        if(!trk)
            return 0;
        seg->_start = trk;
agn:
 	seg->_path->Add(trk, dir);
        if (dir == E_W) {
            dx = -1;
            tTrack = track_walkwest(trk, &ndir);
            if (!tTrack)
                goto done;
            nxtTrack = find_track(layout, tTrack->x, tTrack->y);
            if (!nxtTrack)
                goto done;
            sig = FindLinkedSignal(nxtTrack);
            if (sig)
                goto done;
            if (nxtTrack->type == SWITCH) {
                seg->_end = nxtTrack;
                goto done;
            }
            if (nxtTrack->type == TEXT) {
                seg->_end = nxtTrack;
                goto done;
            }
            dir = ndir;
            trk = nxtTrack;
            goto agn;
        } else {
            tTrack = track_walkeast(trk, &ndir);
            if (!tTrack)
                goto done;
            nxtTrack = find_track(layout, tTrack->x, tTrack->y);
            if (!nxtTrack)
                goto done;
            sig = FindLinkedSignal(nxtTrack);
            if (sig)
                goto done;
            if (nxtTrack->type == SWITCH) {
                seg->_end = nxtTrack;
                goto done;
            }
            if (nxtTrack->type == TEXT) {
                seg->_end = nxtTrack;
                goto done;
            }
            dir = ndir;
            trk = nxtTrack;
            goto agn;
        }
done:
        return 0;
}

int     Ai::WalkSegment(Track *trk, int dir)
{
        Segment *segment = new Segment();
        segment->_id = ++count;
        segment->_start = trk;
        segments.Add(segment);
        FindSegmentPath(segment, trk, dir);
        return 0;
}

void    Ai::DumpColumns()
{
}

int     Ai::FindSegments(Track *layout)
{
        segments.Clear();

        get_all_signals(layout, signals);
        get_all_switches(layout, switches);
        Track *t;
        for (t = layout; t; t = t->next) {
            if (t->type == TRACK && t->wsignal) {
                WalkSegment(t, E_W);
            }
            if (t->type == TRACK && t->esignal) {
                WalkSegment(t, W_E);
            }
        }
        return 0;
}

static  Char objname[256];

static  Char    *GenObjName(Track *trk)
{
        if(trk->station)
            return trk->station;
        wxSnprintf(objname, sizeof(objname)/sizeof(Char), "%d_%d", trk->x, trk->y);
        return objname;
}

Train   *Segment::GetTrain()
{
        Track *trk;
        int i;
        Train *trn;

        for(i = 0; i < _path->_size; ++i) {
            trk = _path->TrackAt(i);
            for (trn = schedule; trn; trn = trn->next) {
                if (trn->status == train_READY)
                    continue;
                if (!trn->position)
                    continue;
                if (trk != trn->position)
                    continue;
                return trn;
            }
        }
        return 0;
}

int     Segment::GetTravelDistance(Train *trn)
{
        Track *trk;
        int dist = 0;
        int sign = 1;
        int j;

        trk = _path->FirstTrack();
        if(trk->fgcolor == conf.fgcolor) // traveling west, since entry is not cleared
            sign = -1; // TODO: check tail position
        for (j = 0; j < _path->_size; ++j) {
            trk = _path->TrackAt(j);
            if (trk->fgcolor == conf.fgcolor)
                continue;
            dist += trk->length;
        }
        return sign * (dist - trn->trackpos); // trackpos is the distance already traveled in the current track
}


static  Array<void *>   sensors;

//      Write list of blocks, signals, switches, trains.
//      These are the sources of our sensor data (inputs).

static  void    write_sensors()
{
        FILE *fp;
        int i;
        Track   *trk;
        Train   *trn;

        sensors.Clear();
        if (!(fp = fopen("/tmp/sensors.csv", "w")))
            return;
        for (i = gAi.blocks.Length(); --i >= 0; ) {
            trk = gAi.blocks.At(i);
            fprintf(fp, "%d,%s\n", sensors.Length(), GenObjName(trk));
            sensors.Add(trk);
        }
        for (i = gAi.switches.Length(); --i >= 0; ) {
            trk = gAi.switches.At(i);
            fprintf(fp, "%d,%s\n", sensors.Length(), GenObjName(trk));
            sensors.Add(trk);
        }
        for (i = gAi.signals.Length(); --i >= 0; ) {
            trk = gAi.signals.At(i);
            fprintf(fp, "%d,%s\n", sensors.Length(), GenObjName(trk));
            sensors.Add(trk);
        }
        for (trn = schedule; trn; trn = trn->next) {
            fprintf(fp, "%d,%s\n", sensors.Length(), trn->name);
            sensors.Add(trn);
        }
        fclose(fp);
}

static  void    write_stations_aspects_actions()
{
        FILE *fp;
        int i;
        Track   *trk;

        if (!(fp = fopen("/tmp/stations.csv", "w")))
            return;
        for (i = 0; i < gAi.stations.Length(); ++i) {
            trk = gAi.stations.At(i);
            fprintf(fp, "%d,%s\n", i, GenObjName(trk));
        }
        fclose(fp);

        if (!(fp = fopen("/tmp/aspects.csv", "w")))
            return;
        for (i = 0; i < gAi.aspects.Length(); ++i) {
            SignalAspect *asp = gAi.aspects.At(i);
            fprintf(fp, "%d,%s\n", i, asp->_name);
        }
        fclose(fp);

        if (!(fp = fopen("/tmp/actions.csv", "w")))
            return;
        for (i = 0; i < gAi.actions.Length(); ++i) {
            Char *asp = gAi.actions.At(i);
            fprintf(fp, "%d,%s\n", i, asp);
        }
        fclose(fp);
}

static  int find_sensor(void *s)
{
        int i = sensors.Find(s);
        return i;
}

static  void get_segments(Track *layout, Ai& ai)
{
        int i;
        gAi.segments.Clear();
        for (i = ai.blocks.Length(); --i >= 0; ) {
            Track *trk = ai.blocks.At(i);
            ai.WalkSegment(trk, W_E);
        }
}

static  int getOccupancy(Track *trk)
{
        Segment *seg;
        int i, j;
        int occ = 0;
        for (i = gAi.segments.Length(); --i; ) {
            seg = gAi.segments.At(i);
            if (seg->_start == trk) {
                for (j = 0; j < seg->_path->_size; ++j) {
                    trk = seg->_path->TrackAt(j);
                    if (trk->fgcolor != conf.fgcolor) {
                        occ += trk->length;
                    }
                }
            }
        }
        return occ;
}

// return next stop for train traveling the segment pointed by trk

static  int getNextStop(Track *trk)
{
        Segment *seg;
        int i, j;
        int occ = 0;
        Train *trn;
        // find the segment pointed by trk
        for (i = gAi.segments.Length(); --i; ) {
            seg = gAi.segments.At(i);
            if (seg->_start != trk)
                continue;
            // find if there is a train traveling on that segment
            for (j = 0; j < seg->_path->_size; ++j) {
                trk = seg->_path->TrackAt(j);
                if (trk->fgcolor == conf.fgcolor) // ???
                    continue;
                for (trn = schedule; trn; trn = trn->next) {
                    if (!trn->position)
                        continue;
                    if (trk != trn->position)
                        continue;
                    // we found the train running in this segment
                    // now find the next stop for the train,
                    // whether it is in the segment or not.
                    TrainStop *stop = trn->stops;
                    while(stop && stop->stopped)
                        stop = stop->next;
                    if(stop) {
                        // convert the stop into a stations[] index
                        int x = 0;
                        for (x = gAi.stations.Length(); --x >= 0; ) {
                            Track *station = gAi.stations.At(x);
                            if (!strcmp(station->station, stop->station))
                                return x;
                        }
                    }
                    return -1;
                }
                return -1;
            }
            return -1;
        }
        return -1;
}

static  int getBlockSpeed(Track *trk)
{
        Segment *seg;
        int i, j;
        int occ = 0;
        for (i = gAi.segments.Length(); --i; ) {
            seg = gAi.segments.At(i);
            if (seg->_start == trk) {
                for (j = 0; j < seg->_path->_size; ++j) {
                    trk = seg->_path->TrackAt(j);
                    if (trk->fgcolor != conf.fgcolor) {
                        occ += trk->length;
                    }
                }
            }
        }
        return occ;
}

void    LogEvent(const Char *eventName, Coord& pos)
{
        Track   *trk;
        Train   *trn;
        int     i;

        if(!loggingEvents)
            return;
        if(!logEventsFile) {
            logEventsFile = fopen("/tmp/tdlog.csv", "w");
            if(!logEventsFile)
                return;
            // collect list of sensors (signals aspects, blocks occupancy, trains statuses, etc.)
            gAi.CollectSensors();
            write_sensors();
            write_stations_aspects_actions();

            // define columns

            // write column names
            /*
            fprintf(logEventsFile, "time,event,objtype,objname");
            for (i = gAi.switches.Length(); --i >= 0; ) {
                Track *trk = gAi.switches.At(i);
                fprintf(logEventsFile, ",%s", GenObjName(trk));
            }
            Train *trn;
            for (trn = schedule; trn; trn->next) {
                wxSnprintf(objname, sizeof(objname)/sizeof(Char), ",%s_name", trn->name);
                fprintf(logEventsFile, ",%s", objname);
            }
            fprintf(logEventsFile, "\n");
            */
        }

        // STATE at current_time is defined as
        //    TRACKS: { (blockIndex,) occupancy, nextStopIndex, speed }[blocks.length]

        fprintf(logEventsFile, "%ld", current_time);
        for (i = 0; i < gAi.blocks.Length(); ++i) {
            Track *trk = gAi.blocks.At(i);
            int occ = getOccupancy(trk);
            int nxtStop = getNextStop(trk);
            int speed = getBlockSpeed(trk);
            fprintf(logEventsFile, ",%d,%d,%d,%d", i, occ, nxtStop, speed);
        }

        //    SIGNALS: { (signalIndex,) aspectIndex, actionIndex }[signals.length]

        for (i = 0; i < gAi.signals.Length(); ++i) {
            Signal *sig = gAi.signals.At(i);
            int aspect = gAi.FindAspect(sig->GetAspect());
            int action = gAi.FindAction(sig->GetAction());
            fprintf(logEventsFile, ",%d,%d,%d", i, aspect, action);
        }

        //    SWITCHES: { (switchIndex,) isThrown, occupancy[0,1] }[switches.length]

        for (i = 0; i < gAi.switches.Length(); ++i) {
            Track *trk = gAi.switches.At(i);
            int thrown = trk->switched;
            int busy = trk->IsBusy();
            fprintf(logEventsFile, ",%d,%d,%d", i, thrown, busy);
        }

        //    TRAINS: { (trainIndex,) nextStopIndex, speed, stockIndex }[trains.length]

        for (i = 0, trn = schedule; trn; trn = trn->next, ++i) {
            int nxtStop = 0;
            int speed = trn->curspeed;
            int stock = 0; // trn->stock;
            fprintf(logEventsFile, ",%d,%d,%d,%d", i, nxtStop, speed, stock);
        }

        // EVENT to move from t to t+1
        //    { time, eventNameIndex[CLICK,RCLICK,...] , sensorTypeIndex[TRK,SIG,TRN,SWTCH], sensorIndex }

        trn = findTrain(pos.x, pos.y);
        if (trn) {
            fprintf(logEventsFile, ",%s,TRN,%s\n", eventName, trn->name);
            fflush(logEventsFile);
            return;
        }
        Signal *sig = findSignal(pos.x, pos.y);
        if (sig) {
            fprintf(logEventsFile, ",%s,SIG,%s\n", eventName, sig->station);
            fflush(logEventsFile);
            return;
        }
        trk = findTrack(pos.x, pos.y);
        if (trk) {
            fprintf(logEventsFile, ",%s,TRK,%s\n", eventName, trk->station);
            fflush(logEventsFile);
            return;
        }
        fprintf(logEventsFile, ",%s,OBJ,%d_%d\n", eventName, pos.x, pos.y);
        fflush(logEventsFile);
}



void    Ai::CollectSensors()
{
        // collect list of sensors (signals aspects, blocks occupancy, trains statuses, etc.)
        get_all_signals(layout, signals);
        get_all_switches(layout, switches);
        get_all_blocks(layout, blocks);
        get_all_stations(layout, stations);
        get_all_aspects(layout, *this);

        get_segments(layout, *this);
}
