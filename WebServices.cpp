/*	WebServices.cpp - Created by Giampiero Caprino

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

#include <wx/thread.h>
#include "Traindir3.h"
#include "trsim.h"
#include "Train.h"
#include "Itinerary.h"
#include "Array.h"
#include "TDFile.h"
#include "Server.h"
#include "Options.h"
#include "GraphView.h"
#include "PlatformGraphView.h"
#include "SwitchBoard.h"
#include "HostLock.h"
#include "Ai.h"

Train	*find_train(Train *sched, const Char *name);
bool    ignore_train(Train *tr);
void    printRunInfo(Char *timeMsg, Char *totalPointsMsg);
int     getcolor_rgb(int);
int     translate_track_color(Track *trk);
extern  Shape   *get_train_shape(Train *trn);

extern	TDSkin	*curSkin;
extern  Option  *intopt_list[];
extern  FileOption  alert_sound;
extern  FileOption  entry_sound;
extern  FileOption  searchPath;
extern  int	time_mults[];
extern  const Char *locale_name;
extern  bool    quitting;

static  void    ToHex(Char *dest, int size, Char *src)
{
        int     x;

        for(x = 0; x + 2 < size && src[x]; ++x) {
            wxSprintf(&dest[x * 2], wxT("%02X"), src[x] & 0xff);
        }
        dest[x * 2] = 0;
}

static  void    FromHex(Char *dest, int size, Char *src)
{
        Char *xp = src;
        int x = 0;
        while(*xp && x + 1 < size) {
            char c = *xp++;
            char c1 = *xp++;
            if(c >= '0' && c <= '9')
                c -= '0';
            else if(c >= 'A' && c <= 'F')
                c = c - 'A' + 10;
            else if(c >= 'a' && c <= 'f')
                c = c - 'a' + 10;
            if(c1 >= '0' && c1 <= '9')
                c1 -= '0';
            else if(c1 >= 'A' && c1 <= 'F')
                c1 = c1 - 'A' + 10;
            else if(c1 >= 'a' && c1 <= 'f')
                c1 = c1 - 'a' + 10;
            dest[x++] = (c << 4) | c1;
        }
        dest[x] = 0;
}

static  void    getLayout(Servlet& s, wxString& out)
{
        wxString    shapeStr(wxT(""));
        Char    buff[512];
        Track   *trk;
        const Char    *sep = wxT("\n");

        out.Append(wxT("\"layout\": ["));
        for(trk = layout; trk; trk = trk->next) {
            if(trk->invisible)
                continue;
            shapeStr.Clear();
            int col = translate_track_color(trk);
            wxSprintf(buff, wxT("%s{ \"x\": %d,\n  \"y\": %d,\n  \"type\": %d,\n  \"color\": %d,\n"), sep, trk->x, trk->y, trk->type, col);
            if(trk->type == SWITCH)
                wxSprintf(buff + wxStrlen(buff), wxT("  \"switched\": %d,\n"), trk->switched);
            if(trk->_shape)
                Servlet::json(shapeStr, wxT("shape"), trk->_shape->_id->c_str(), false);
            if(trk->type == TSIGNAL) {
                Signal *sig = (Signal *)trk;
                Shape *shape = sig->FindShape();
                if(shape)
                    Servlet::json(shapeStr, wxT("shape"), shape->_id->c_str(), false);
            } else if(trk->type == IMAGE && trk->station) {
                Servlet::json(shapeStr, wxT("shape"), trk->station, false);
            } else if((trk->type == TEXT || trk->type == ITIN) && trk->station) {
                Servlet::json(shapeStr, wxT("text"), trk->station, false);
            }
            if(shapeStr.Len() > 0)
                wxStrcat(buff, shapeStr.c_str());
            wxSprintf(buff + wxStrlen(buff), wxT("\n  \"dir\": %d\n}\n"), trk->direction);
            out.Append(buff);
            sep = wxT(",\n");
        }
        out.Append(wxT("]\n"));
}


static  void    getAllTrains(Servlet& s, wxString& out)
{
        Train   *trn;
        const Char *sep = wxT("{\n");
        TrainInfo info;
        Char    row[8192];
        Char    link[1024];
        wxString str;

        out.append(wxT("\"trains\":\n["));
        for(trn = schedule; trn; trn = trn->next) {
            //str.clear();
            //if(trn->days && !(trn->days & run_day))
            //    continue;
            trn->Get(info);

            ToHex(link, sizeof(link), trn->name);
            row[0] = 0;
            Servlet::json(row, sizeof(row)/sizeof(row[0]), wxT("name"), trn->name, sep);
            wxSprintf(row + wxStrlen(row), wxT("\"link\": \"%s\",\n"), link);
            wxSprintf(row + wxStrlen(row), wxT("\"entry\": \"%s\",\n"), trn->entrance);
            wxSprintf(row + wxStrlen(row), wxT("\"exit\": \"%s\",\n"), trn->exit);
            wxSprintf(row + wxStrlen(row), wxT("\"timeIn\": \"%s\",\n"), info.entering_time);
            wxSprintf(row + wxStrlen(row), wxT("\"timeOut\": \"%s\",\n"), info.leaving_time);
            wxSprintf(row + wxStrlen(row), wxT("\"speed\": \"%s\",\n"), info.current_speed);
            wxSprintf(row + wxStrlen(row), wxT("\"delay\": \"%s\",\n"), info.current_delay);
            wxSprintf(row + wxStrlen(row), wxT("\"late\": \"%s\",\n"), info.current_late);
            wxSprintf(row + wxStrlen(row), wxT("\"state\": \"class%d\",\n"), (int)trn->status);
            wxSprintf(row + wxStrlen(row), wxT("\"status\": \"%s\"\n"), info.current_status);
            out.append(row);
            switch(trn->status) {
            case train_RUNNING:
            case train_STOPPED:
            case train_ARRIVED:
            case train_WAITING:
            case train_STARTING:
                if(trn->position) {
                    out.Append(wxT(", \"loco\": { "));
                    Shape *shape = trn->direction == W_E ? trn->epix : trn->wpix;
                    if(shape) {
                        Servlet::json(out, wxT("shape"), shape->_id->c_str(), false);
                    }
                    Servlet::json(out, wxT("x"), trn->position->x, false);
                    Servlet::json(out, wxT("y"), trn->position->y, true);
                    out.Append(wxT(" }\n"));
                }
                if(trn->tail && trn->tail->position && trn->tail->position != trn->position) {
                    out.Append(wxT(", \"car\": { "));
                    Shape *shape = trn->direction == W_E ? trn->ecarpix : trn->wcarpix;
                    if(shape) {
                        Servlet::json(out, wxT("shape"), shape->_id->c_str(), false);
                    }
                    Servlet::json(out, wxT("x"), trn->tail->position->x, false);
                    Servlet::json(out, wxT("y"), trn->tail->position->y, true);
                    out.Append(wxT(" }\n"));
                }
            }
            sep = wxT("},\n{");
        }
        if(sep[0] == '}') // we emitted data about at least one train
            out.append(wxT("}\n]\n"));
        else
            out.append(wxT("]\n"));
}

static  void    getStatus(wxString& out)
{
        Train *trn;

        RunInfoStr info;
        printRunInfo(info);
        out.append(wxT("\"status\": {\n"));
        Servlet::json(out, wxT("mult"), time_mults[cur_time_mult], false);
        Servlet::json(out, wxT("time"), format_time(current_time), false);
        Servlet::json(out, wxT("running"), running ? 1 : 0, false);
        Servlet::json(out, wxT("timemsg"), wxStrchr(info.time_msg, 'R'), false);
        Servlet::json(out, wxT("quitting"), quitting ? 1 : 0, false);
        Servlet::json(out, wxT("points"), info.points_msg, false);
        Servlet::json(out, wxT("scenario"), current_project.c_str(), false);
        Servlet::json(out, wxT("editing"), editing ? 1 : 0, false);
        Servlet::json(out, wxT("runnning"), ntrains_running);
        Servlet::json(out, wxT("ready"), ntrains_ready);
        Servlet::json(out, wxT("starting"), ntrains_starting);
        Servlet::json(out, wxT("waiting"), ntrains_waiting);
        Servlet::json(out, wxT("stopped"), ntrains_stopped);
        Servlet::json(out, wxT("arrived"), ntrains_arrived);
        Servlet::json(out, wxT("delayed"), total_delay);
        Servlet::json(out, wxT("late"), total_late);
        for(trn = schedule; trn; trn = trn->next) {
            if(trn->status != train_ARRIVED)
                break;
        }
        Servlet::json(out, wxT("finished"), trn != 0 ? 0 : 1, true); // no more trains scheduled
        out.append(wxT("\n}\n"));
}

static  void    getAlerts(Servlet& s, wxString& out)
{
        const Char *sep = wxT("\n");
        AlertLine *alert;
        wxString    str;

        out.append(wxT("\"alerts\" :\n["));
        alerts.Lock();
        for(alert = alerts._firstItem; alert; alert = alert->_next) {
            str.Printf(wxT("%s\"%s\""), sep, alert->_text);
            out.append(str);
            sep = wxT(",\n");
        }
        alerts.Unlock();
        out.append(wxT("\n]\n"));
}


class WS_Trains : public Servlet
{
public:
        WS_Trains() : Servlet("/war/trains.json") { };
        ~WS_Trains() { };
        const char *getMimeType() const { return MIME_JSON; };

        bool    get(wxString& out, Char *args);
} ws_trains;

bool    WS_Trains::get(wxString& out, Char *args)
{
        out.append(wxT("{\n"));
        getAllTrains(*this, out);
        out.append(wxT("}\n"));
        return true;
}




class WS_Stations : public Servlet
{
public:
        WS_Stations() : Servlet("/war/stations.json") { };
        ~WS_Stations() { };
       const char *getMimeType() const { return MIME_JSON; };

        bool    get(wxString& out, Char *args);
} ws_stations;

bool    WS_Stations::get(wxString& out, Char *args)
{
        Track   *trk;
        StationList stations;
        int     i;
        Char    buff[512];
        const Char *sep = wxT("{");

        out.append(wxT("{ \"stations\" :\n["));
        stations.Build(false);
        if(stations._nItems) {
            for(i = 0; stations._items[i]; ++i) {
                trk = stations._items[i];
                out.append(sep);
                ToHex(buff, sizeof(buff), trk->station);
                json(out, wxT("link"), buff, false);
                json(out, wxT("name"), trk->station, true);
                sep = wxT("},\n{");
            }
            out.append(wxT("}"));
        }
        out.append(wxT("\n]\n}\n"));
        return true;
}


class WS_TrainStops : public Servlet
{
public:
        WS_TrainStops() : Servlet("/war/stops.json") { };
        ~WS_TrainStops() { };
        const char *getMimeType() const { return MIME_JSON; };

        bool    get(wxString& out, Char *args);
} ws_train_stops;

bool    WS_TrainStops::get(wxString& out, Char *args)
{
        Train   *trn;
        TrainStop *stp;
        Char    buff[256];
        const Char *sep = wxT("{");

        out.append(wxT("{\n"));
        if(!wxStrncmp(args, wxT("t="), 2)) {
            //FromUrl(buff, args + 2);
            FromHex(buff, sizeof(buff), args + 2);
            trn = find_train(schedule, buff);
            if(trn) {
                json(out, wxT("name"), trn->name);
                out.append(wxT("\"stops\" :\n["));
                for(stp = trn->stops; stp; stp = stp->next) {
                    out.append(sep);
                    json(out, wxT("station"), stp->station);
                    ToHex(buff, sizeof(buff), stp->station);
                    json(out, wxT("stationlink"), buff);
                    json(out, wxT("arrival"), format_time(stp->arrival));
                    json(out, wxT("departure"), format_time(stp->departure));
                    json(out, wxT("minstop"), stp->minstop);
                    json(out, wxT("stopped"), stp->stopped);
                    json(out, wxT("late"), stp->late);
                    json(out, wxT("delay"), stp->delay, true);
                    sep = wxT("},\n{");
                }
                if(trn->stops)
                    out.append(wxT("}"));
                out.append(wxT("\n]\n"));
            }
        }
        out.append(wxT("}\n"));
        return true;
}


class WS_Schedule : public Servlet
{
public:
        WS_Schedule() : Servlet("/war/sched.json") { };
        ~WS_Schedule() { };
        const char *getMimeType() const { return MIME_JSON; };

        bool    get(wxString& out, Char *args);
} ws_schedule;

bool    WS_Schedule::get(wxString& out, Char *args)
{
        const Char *sep = wxT("\n{ ");
        Char    buff[256];
        Char    *p;
        int     r;
        out.append(wxT("{ \"schedule\" :\n["));

        if(!wxStrncmp(args, wxT("s="), 2)) {
//            FromUrl(buff, args + 2);
            FromHex(buff, sizeof(buff), args + 2);
            StationSchedule sched;
            sched.Build(buff);
            struct station_sched *sc;

            for(sc = sched._items; sc; sc = sc->next) {
                out.append(sep);
                json(out, wxT("train"), sc->tr->name);
                ToHex(buff, sizeof(buff), sc->tr->name);
                json(out, wxT("trainlink"), buff);
                json(out, wxT("arrival"), sc->arrival != -1 ? format_time(sc->arrival) : wxT(""));
                json(out, wxT("entrance"), sc->tr->entrance);
                json(out, wxT("departure"), sc->departure != -1 ? format_time(sc->departure) : wxT(""));
                json(out, wxT("exit"), sc->tr->exit);
	        buff[0] = 0;
	        if(sc->stopname && (p = wxStrchr(sc->stopname, '@')))
		    wxStrcpy(buff, p + 1);
                json(out, wxT("platform"), buff);
	        int x = 0;
	        if(sc->tr->days) {
		    for(r = 0; r < 7; ++r)
		        if(sc->tr->days & (1 << r))
			    buff[x++] = r + '1';
	        }
	        buff[x] = 0;
                json(out, wxT("days"), buff, true);
                sep = wxT("},\n{ ");
            }
            if(sched._items)
                out.append(wxT("}"));
        }
        out.append(wxT("\n]\n}\n"));
        return true;
}



class WS_Alerts : public Servlet
{
public:
        WS_Alerts() : Servlet("/war/alerts.json") { };
        ~WS_Alerts() { };
        const char *getMimeType() const { return MIME_JSON; };

        bool    get(wxString& out, Char *args);
} ws_alerts;

bool    WS_Alerts::get(wxString& out, Char *args)
{
        out.append(wxT("{\n"));
        getAlerts(*this, out);
        out.append(wxT("}\n"));
        return true;
}


class EventListListener : public EventListener
{
public:
        EventListListener() { _sema = new wxSemaphore(0, 0); }
        ~EventListListener() { };

        wxSemaError Wait();
        void    OnEvent(void *list);

        wxSemaphore *_sema;
} events_listener;

wxSemaError EventListListener::Wait()
{
        return _sema->WaitTimeout(20000);
}

void    EventListListener::OnEvent(void *list)
{
         _sema->Post();
}

class WS_Events : public Servlet
{
public:
        WS_Events() : Servlet("/war/events.json") { };
        ~WS_Events() { };
       const char *getMimeType() const { return MIME_JSON; };

        bool    get(wxString& out, Char *args);
} ws_events;

bool    WS_Events::get(wxString& out, Char *args)
{
        Char    buff[64];
        const Char *sep = wxT("{");

        int     lastEventId = 0;
        if(args && !wxStrncmp(args, wxT("l="), 2)) {
            lastEventId = wxStrtoul(args + 2, &args, 10);
        }
        out.append(wxT("{\n\"events\" : {\n"));
        if(lastEventId >= lastModTime) {
            // wait here until some event is available (i.e. lastEventId < lastModTime)
            alerts.AddListener(&events_listener);
            timetable.AddListener(&events_listener);
            wxSemaError error = events_listener.Wait();
            timetable.RemoveListener(&events_listener);
            alerts.RemoveListener(&events_listener);
            if(error != wxSEMA_NO_ERROR) {
                wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("  \"lastMod\" : %d,\n"), lastModTime);
                out.append(buff);
                out.append(wxT("  \"reloaded\": "));
                if(timetable._lastReloaded > lastEventId) {
                    out.append(wxT("\"true\""));
                } else {
                    out.append(wxT("\"false\""));
                }
                out.append(wxT("\n  }\n}\n"));   // end events
                return true;
            }
        }
        wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("  \"lastMod\" : %d,\n"), lastModTime);
        out.append(buff);

        // return alerts

        out.append(wxT("  \"alerts\": [\n"));
        AlertLine *line;
        sep = wxT("    {");
        for(line = alerts._firstItem; line; line = line->_next) {
            if(line->_modTime < lastEventId) {
                continue;
            }
            out.append(sep);
            out.append(wxT(" \"msg\":\""));
            out.append(line->_text);
            out.append(wxT("\" }"));
            sep = wxT(",\n    {");
        }
        out.append(wxT("\n],\n"));

        out.append(wxT("  \"timetable\": [\n"));
        Train *tr;
        TrainInfo info;
        sep = wxT("    {");
        for(tr = schedule; tr; tr = tr->next) {
            if(tr->_lastUpdate < lastEventId)
                continue;
            if(!tr->entrance || tr->isExternal)
                continue;
            if(!show_canceled._iValue && is_canceled(tr))
                continue;

            const Char *internalStatus = 0; // non-localized version of tr->status
            if(tr->status == train_ARRIVED) {
                if(tr->stock) {
	            Train *t1 = findTrainNamed(tr->stock);
                    if(t1 && t1->status != train_READY) {
                        internalStatus = wxT("to-be-assigned");
                        break;
                    }
                }
            }
            if(internalStatus == 0) {
                switch(tr->status) {
                case train_ARRIVED:
                    internalStatus = wxT("arrived");
                    break;
                case train_DELAY:
                    internalStatus = wxT("delayed");
                    break;
                case train_DERAILED:
                    internalStatus = wxT("derailed");
                    break;
                case train_READY:
                    internalStatus = wxT("ready");
                    break;
                case train_RUNNING:
                    internalStatus = wxT("running");
                    break;
                case train_STOPPED:
                    internalStatus = wxT("stopped");
                    break;
                case train_WAITING:
                    internalStatus = wxT("waiting");
                    break;
                default:
                    internalStatus = wxT("?");
                }
            }
            tr->Get(info);

            out.append(sep);
            out.append(wxT(" \"name\":\""));
            out.append(tr->name);
            out.append(wxT("\",\n  \"entry\":\""));
            out.append(tr->entrance);
            out.append(wxT("\",\n  \"exit\":\""));
            out.append(tr->exit);
            out.append(wxT("\",\n  \"timeIn\":\""));
            out.append(info.entering_time);
            out.append(wxT("\",\n  \"timeOut\":\""));
            out.append(info.leaving_time);
            out.append(wxT("\",\n  \"speed\":\""));
            out.append(info.current_speed);
            out.append(wxT("\",\n  \"status\":\""));
            out.append(info.current_status);
            out.append(wxT("\",\n  \"internalstatus\":\""));
            out.append(internalStatus);
            out.append(wxT("\",\n  \"late\":\""));
            out.append(info.current_late);
            out.append(wxT("\"\n  }"));
            sep = wxT(",\n    {");
        }
  //      SetItem(i, 0, info.entering_time);
//	SetItem(i, 3, info.leaving_time);
//	SetItem(i, 5, info.current_speed);
//	SetItem(i, 6, info.current_delay);
//	SetItem(i, 7, info.current_late);
//	SetItem(i, 8, info.current_status);
//	for(n = 0; n < MAXNOTES; ++n) {
//	    notes += t->notes[n] ? t->notes[n] : wxT("");
//	    notes += wxT(" ");
//	}
//	SetItem(i, 9, notes);
  //      }
        out.append(wxT("\n],\n"));

        out.append(wxT("  \"reloaded\": "));
        if(timetable._lastReloaded > lastEventId) {
            out.append(wxT("\"true\"\n"));
        } else {
            out.append(wxT("\"false\"\n"));
        }
        out.append(wxT("}\n}\n"));   // end events
        return true;
}

extern  int saveCanvasToMemory(void **out, const Char *path);
extern  int readCanvasSnapshot(void **out);

class WS_Canvas : public Servlet
{
public:
        WS_Canvas() : Servlet("/war/canvas") { };
        ~WS_Canvas() { };
        const char *getMimeType() const { return "image/png"; };

        bool    get(void **out, int& len, Char *args);
} ws_canvas;

bool    WS_Canvas::get(void **out, int& len, Char *args)
{
//        wxString fileName = wxT("C:/Temp/canvas.png");

        void *outputBytes;
//        len = saveCanvasToMemory(&outputBytes, wxT("*.png"));
        len = readCanvasSnapshot(&outputBytes);
        if(len < 0)
            return false;
        *out = outputBytes;
        /*
        FILE *fimage = fopen(fileName.c_str(), "rb");
        fseek(fimage, 0, 2);
        len = (int)ftell(fimage);
        *out = malloc(len);
        fseek(fimage, 0, 0);
        int cnt = fread(*out, 1, len, fimage);
        fclose(fimage);
        if(cnt != len)
            return false;
        */
        return true;
}

class WS_Do : public Servlet
{
public:
        WS_Do() : Servlet("/war/do") { };
        ~WS_Do() { };

        bool    get(wxString& out, Char *args);
} ws_do;

void	post_command(wxChar *cmd);
void	do_command(const wxChar *cmd, bool sendToClients);

bool    WS_Do::get(wxString& out, Char *args)
{
        Char    buff[1024];
        Char    buff1[1024];
        Char    buff2[512];
        Char    *d, *p;
        bool    sync = false;

        FromUrl(buff, args);
        p = buff;
        if(*p == '!') {
            ++p;
            sync = true;
        }
        for(d = buff1; *p; ) {
            if(*p == '\b') {
                Char *d2 = buff2;
                while(*++p && *p != '\b')
                    *d2++ = *p;
                if(*p)
                    ++p;
                *d2 = 0;
                FromHex(d, sizeof(buff1) - (d - buff1), buff2);
                d += wxStrlen(d);
            } else {
                *d++ = *p++;
            }
        }
        *d = 0;
        if(sync) {
            do_command(buff1, false);
        } else {
            // post_command queues the command into the UI pending cmd list - see Main.cpp and Server.cpp
            post_command(buff1);
        }
        out.append(wxT("{ \"status\": \"ok\" }\n"));
        return true;
}

extern  BoolOption zooming;

class WS_Click : public Servlet
{
public:
        WS_Click() : Servlet("/war/click") { };
        ~WS_Click() { };

        bool    get(wxString& out, Char *args);
} ws_click;

bool    WS_Click::get(wxString& out, Char *args)
{
        Char    buff[512];
        Char    arg[128];
        Char    value[128];
        Char    *p;
        Char    *d;
        int     x = -1, y = -1;
        bool    withCoords = false;
        bool    withZoom = false;
        bool    shift = false;
        bool    alt = false;
        bool    ctrl = false;
        bool    right = false;
        Char    *cmd = wxT("click");

        FromUrl(buff, args);
        p = buff;
        while(*p) {
            d = arg;
            while(*p && *p != '=' && *p != '&')
                *d++ = *p++;
            *d = 0;
            if(*p == '=') {
                ++p;
                d = value;
                while(*p && *p != '&')
                    *d++ = *p++;
                *d = 0;
                if(*p)
                    ++p;
            } else
                value[0] = 0;
            if(!wxStrcmp(arg, wxT("x"))) {
                x = wxAtoi(value);
            } else if(!wxStrcmp(arg, wxT("y"))) {\
                y = wxAtoi(value);
            } else if(!wxStrcmp(arg, wxT("shift"))) {
                shift = value[0] != '0';
            } else if(!wxStrcmp(arg, wxT("alt"))) {
                alt = value[0] != '0';
            } else if(!wxStrcmp(arg, wxT("ctrl"))) {
                ctrl = value[0] != '0';
                if(ctrl)
                    cmd = wxT("ctrlclick");
            } else if(!wxStrcmp(arg, wxT("btn"))) {
                right = wxAtoi(value) > 1;
                if(right)
                    cmd = wxT("rclick");
            }
        }
        if(withZoom != zooming._iValue || withCoords != bShowCoord) {
            out.append(wxT("{ \"status\": \"bad\" }\n"));
            return true;
        }
        if(alt && !right) {
            right = true;
            cmd = wxT("rclick");
        }
        wxSprintf(buff, wxT("%s %d,%d"), cmd, x, y);
        post_command(buff);
        out.append(wxT("{ \"status\": \"ok\" }\n"));
        return true;
}


class WS_FullState : public Servlet
{
public:
        WS_FullState() : Servlet("/war/fullstate.json") { };
        ~WS_FullState() { };
        const char *getMimeType() const { return MIME_JSON; };

        bool    get(wxString& out, Char *args);
        HostLock _lock;
        wxString _content;
} ws_fullState;

bool    WS_FullState::get(wxString& out, Char *args)
{
        _lock.Lock();
        _content.clear();
        _content.append(wxT("{\n\"zoom\": "));
        _content.append(zooming._iValue ? wxT("1") : wxT("0"));
        _content.append(wxT(",\n\"coords\": "));
        _content.append(bShowCoord ? wxT("1"): wxT("0"));
        _content.append(wxT(",\n\"time\": \""));
        _content.append(format_time(current_time));
        _content.append(wxT("\",\n"));
        getAllTrains(*this, _content);
        _content.append(wxT(",\n"));
        getAlerts(*this, _content);
        _content.append(wxT(",\n"));
        getStatus(_content);
        _content.append(wxT("}\n"));
        out = _content;
        _lock.Unlock();
        return true;
}

class WS_Itineraries : public Servlet
{
public:
        WS_Itineraries() : Servlet("/war/itineraries.yaml") { };
        ~WS_Itineraries() { };

        bool    get(wxString& out, Char *args);
} ws_itineraries;

bool    WS_Itineraries::get(wxString& out, Char *args)
{
        Itinerary *it;

        out.append(wxT("itineraries:\n"));
	for(it = itineraries; it; it = it->next) {
            out.append(wxT("- name: "));
            out.append(it->name);
            out.append(wxT("\n  from: "));
            out.append(it->signame);
            out.append(wxT("\n  to: "));
            out.append(it->endsig);
            out.append(wxT("\n  next: "));
            out.append(it->nextitin);
            out.append(wxT("\n"));
	}
        return true;
}

class WS_InfoScenario : public Servlet
{
public:
        WS_InfoScenario() : Servlet("/war/infoscenario") { };
        ~WS_InfoScenario() { };
        const char *getMimeType() const { return MIME_HTML; };

        bool    get(wxString& out, Char *args);
} ws_infoscenario;

extern  wxString    info_page;

bool    WS_InfoScenario::get(wxString& out, Char *args)
{
        TDFile	infoFile(info_page);

        infoFile.SetExt(wxT(".htm"));
        if(infoFile.Load()) {
            out.append(infoFile.content);
        } else {
            out.append(wxT("No info file found ('"));
            out.append(info_page);
            out.append(wxT("'.\n"));
        }
        return true;
}

class WS_SimScenario : public Servlet
{
public:
        WS_SimScenario() : Servlet("/sim/info/") { };
        ~WS_SimScenario() { };
        const char *getMimeType() const { return MIME_HTML; };

        bool    get(wxString& out, Char *args);
} ws_simInfo;

extern  wxString    info_page;

bool    WS_SimScenario::get(wxString& out, Char *args)
{
        if(*args) {
            TDFile	infoFile(args); // TODO: check if png/jpg

            if(infoFile.Load()) {
                out.append(infoFile.content);
            } else {
                out.append(wxT("No info file found ('"));
                out.append(info_page);
                out.append(wxT("'.\n"));
            }
        }
        return true;
}

class WS_AssignInfo : public Servlet
{
public:
        WS_AssignInfo() : Servlet("/war/assigninfo.yaml") { };
        ~WS_AssignInfo() { };

        bool    get(wxString& out, Char *args);
} ws_assigninfo;

bool    WS_AssignInfo::get(wxString& out, Char *args)
{
        AssignInfo info;
        Train   *trn;
        int     i;
        Char    *p;
        Char    buff[1024];

        try {
            if(!wxStrncmp(args, wxT("t="), 2)) {
                FromHex(buff, sizeof(buff), args + 2);
                trn = find_train(schedule, buff);
                if(trn) {
                    if(trn->Get(info)) {
                        if(info.assign_to >= 0 && info.assign_to < info.candidates.Length()) {
                            out.append(wxT("dest: "));
                            out.append(info.candidates[info.assign_to]->name);
                            out.append(wxT("\n"));
                            out.append(wxT("link: "));
                            ToHex(buff, sizeof(buff), info.candidates[info.assign_to]->name);
                            out.append(buff);
                            out.append(wxT("\n"));
                        }
                        out.append(wxT("candidates:\n"));
	                for(i = 0; i < info.candidates.Length(); ++i) {
	                    Train   *t1 = info.candidates[i];
                            out.append(wxT("- train: "));
                            out.append(t1->name);
                            ToHex(buff, sizeof(buff), t1->name);
                            out.append(wxT("\n  link: "));
                            out.append(buff);
                            out.append(wxT("\n  arr: "));
                            p = format_time(t1->timein);
                            while(*p == ' ') ++p;
                            out.append(p);
                            out.append(wxT("\n  dep: "));
                            p = format_time(t1->timeout);
                            while(*p == ' ') ++p;
                            out.append(p);
	                    wxStrcpy(buff, t1->entrance);
	                    if((p = wxStrchr(buff, '@'))) {
		                *p = 0;
                                out.append(wxT("\n  platform: "));
                                out.append(p + 1);
	                    }
                            out.append(wxT("\n  from: "));
                            out.append(buff);
                            out.append(wxT("\n  to: "));
                            out.append(t1->exit);

	                    int		n;

                            out.append(wxT("\n  notes: "));
	                    for(n = 0; n < MAXNOTES; ++n) {
                                if (t1->notes[n]) {
		                    out.append(t1->notes[n]);
		                    out.append(wxT(" "));
                                }
	                    }
                            out.append(wxT("\n"));
                        }
                    }
                }
            }
        } catch (...) {
            return true;
        }
        return true;
}


class WS_Options : public Servlet
{
public:
        WS_Options() : Servlet("/war/options.yaml") { };
        ~WS_Options() { };

        void    AddFileOption(wxString& out, FileOption& opt);
        void    AddColorOption(wxString& out, const Char *id, const Char *descr, int value);
        bool    get(wxString& out, Char *args);
} ws_options;

bool    WS_Options::get(wxString& out, Char *args)
{
        int     i;

        out.append(wxT("options:\n"));
	for(i = 0; intopt_list[i]; ++i) {
            Option *opt = intopt_list[i];
            out.append(wxT("- id: "));
            out.append(opt->_name.c_str());
            out.append(wxT("\n  desc: "));
            out.append(LV(opt->_descr.c_str()));
            const Char *type = wxT("");
            switch(opt->_type) {
            case OPTION_STRING: type = wxT("string"); break;
            case OPTION_INT:    type = wxT("int"); break;
            case OPTION_BOOL:   type = wxT("bool"); break;
            case OPTION_FILE:   type = wxT("file"); break;
            }
            out.append(wxT("\n  type: "));
            out.append(type);
            out.append(wxT("\n  value: "));
            out.append(opt->_sValue.c_str());
            out.append(wxT("\n"));
	}

        AddFileOption(out, entry_sound);
        AddFileOption(out, alert_sound);
        AddFileOption(out, searchPath);

        AddColorOption(out, wxT("free_track"), L("Free Track"), curSkin->free_track);
	AddColorOption(out, wxT("reserved_track"), L("Reserved Track"), curSkin->reserved_track);
	AddColorOption(out, wxT("reserved_shunting"), L("Reserved for Shunting"), curSkin->reserved_shunting);
	AddColorOption(out, wxT("occupied_track"), L("Occupied"), curSkin->occupied_track);
	AddColorOption(out, wxT("working_track"), L("Reserved for Working"), curSkin->working_track);
        AddColorOption(out, wxT("background"), L("Background"), curSkin->background);
	AddColorOption(out, wxT("outline"), L("Switch Outline"), curSkin->outline);
	AddColorOption(out, wxT("text"), L("Text"), curSkin->text);
        return true;
}

void    WS_Options::AddFileOption(wxString& out, FileOption& opt)
{
        Char    buff[256];
        
        out.append(wxT("- id: "));
        out.append(opt._name.c_str());
        out.append(wxT("\n  desc: "));
        out.append(LV(opt._descr.c_str()));
        wxSprintf(buff, wxT("\n  type: file\n  value: %s\n"), opt._sValue.c_str());
        out.append(buff);
}


void    WS_Options::AddColorOption(wxString& out, const Char *id, const Char *descr, int value)
{
        Char    buff[256];
        
        out.append(wxT("- id: "));
        out.append(id);
        out.append(wxT("\n  desc: "));
        out.append(descr);
        wxSprintf(buff, wxT("\n  type: color\n  value: \"%d,%d,%d\"\n"), (value >> 16) & 0xff, (value >> 8) & 0xff, (value & 0xff));
        out.append(buff);
}

class WS_Play : public Servlet
{
public:
        WS_Play() : Servlet("/war/play.yaml") { };
        ~WS_Play() { };

        bool    get(wxString& out, Char *args) {
            out.append(wxT("status:\n"));
            if (running) {
                out.append(wxT("  prev: running\n"));
            } else {
                out.append(wxT("  prev: stopped\n"));
                start_stop();
            }
            if(running)
                out.append(wxT("  current: running\n"));
            else
                out.append(wxT("  current: stopped\n"));
            return true;
        }
} ws_play;


class WS_Pause : public Servlet
{
public:
        WS_Pause() : Servlet("/war/pause.yaml") { };
        ~WS_Pause() { };
        const char *getMimeType() const { return MIME_TEXT; };

        bool    get(wxString& out, Char *args) {
            out.append(wxT("status:\n"));
            if (running) {
                out.append(wxT("  prev: running\n"));
                start_stop();
            } else {
                out.append(wxT("  prev: stopped\n"));
            }
            if(running)
                out.append(wxT("  current: running\n"));
            else
                out.append(wxT("  current: stopped\n"));
            return true;
        }
} ws_pause;

extern  void    simulate_one_second(void);

class WS_Step : public Servlet
{
public:
        WS_Step() : Servlet("/war/step.json") { };
        ~WS_Step() { };

        bool    get(wxString& out, Char *args) {
            if (running)
                start_stop();
            simulate_one_second();
            update_labels();
            wxString tmp;
            getStatus(tmp);
            const Char *p = tmp.c_str();
            p = wxStrchr(p, '{');
            out.append(p);
            //out.append(wxT("\n}\n"));
            return true;
        }
} ws_step;


class WS_Sensors : public Servlet
{
public:
        WS_Sensors() : Servlet("/war/sensors.json") { };
        ~WS_Sensors() { };

        bool    get(wxString& out, Char *args);
} ws_sensors;

bool    WS_Sensors::get(wxString& out, Char *args)
{
        gAi.CollectSensors();
        int i;
        wxString tmp;
        Char    sep = ' ';
        Char    sep1 = ' ';
        TrackBase *trk;

        out.append(wxT("{ \"segs\": [\n"));
        for (i = 0; i < gAi.segments.Length(); ++i) {
            Segment *seg = gAi.segments.At(i);
            tmp.Printf(wxT("%c {  \"id\": %d,\n  \"path\": [\n"), sep, seg->_id);
            out.append(tmp);
            sep1 = ' ';
            int p;
            int clearedLength = 0;
            int seglength = 0;
            for (p = 0; p < seg->_path->_size; ++p) {
                TrackBase *trk = seg->_path->_ptr[p];
                tmp.Printf(wxT("%c    \"%d_%d\"\n"), sep1, trk->x, trk->y);
                out.append(tmp);
                if (trk->fgcolor != conf.fgcolor)
                    clearedLength += trk->length;
                seglength += trk->length;
                sep1 = ',';
            }
            out.append(wxT("  ]\n"));
            // add block status: which train, if any, occupies it
            // and the next stop of the train
            Train *trn = seg->GetTrain();
            if (trn) {
                int toTravel = seg->GetTravelDistance(trn);
                ExprValue nextStop(String);
                trn->GetPropertyValue("nextStation", nextStop);
                out.append(wxT(",  \"train\": {\n     \"name\": \""));
                out.append(trn->name);
                out.append(wxT("\",\n    \"dest\": \""));
                out.append(nextStop._txt);
                tmp.Printf(wxT("\",\n    \"speed\": %d,\n  \"traveled\": %d\n  }\n"), (int)trn->curspeed, toTravel);
                out.append(tmp);
                clearedLength += toTravel;
            }
            tmp.Printf(wxT("  ,  \"seglength\": %d, \"busylength\" : %d\n  }\n"), seglength, clearedLength);
            out.append(tmp);
            sep = ',';
        }
        out.append(wxT("  ],\n  \"sigs\": [\n"));
        sep1 = ' ';
        for (i = 0; i < gAi.signals.Length(); ++i) {
            Signal *sig = gAi.signals.At(i);
            double limit;
            int ilimit;
            if (sig->controls && sig->controls->fgcolor == color_white)
                limit = 0.3;
            else if(!sig->IsClear())
                limit = 0;
            else if (!sig->GetSpeedLimit(&ilimit))
                limit = 1;
            else
                limit = ilimit;
            tmp.Printf(wxT("%c  { \"name\": \"_sig%d\", \"x\": %d, \"y\": %d, \"limit\": %f }\n"), sep1, i, sig->x, sig->y, limit);
            out.append(tmp);
            sep1 = ',';
        }
        out.append(wxT("  ],\n  \"switches\": [\n"));  // switches statuses
        sep1 = ' ';
        for (i = 0; i < gAi.switches.Length(); ++i) {
            trk = gAi.switches.At(i);
            tmp.Printf(wxT("%c    { \"name\": \"%d\", \"x\": %d, \"y\": %d, \"thrown\": %d }\n"), sep1, i, trk->x, trk->y, trk->switched);
            out.append(tmp);
            sep1 = ',';
        }
        out.append(wxT("  ],\n  \"destinations\": [\n"));  // entry/exit points + stations
        sep1 = ' ';
        for (i = 0; i < gAi.stations.Length(); ++i) {
            trk = gAi.stations.At(i);
            tmp.Printf(wxT("%c    { \"name\": \"%s\", \"x\": %d, \"y\": %d, \"isstation\": %d }\n"), sep1, trk->station, trk->x, trk->y, trk->type == TRACK);
            out.append(tmp);
            sep1 = ',';
        }
        for (trk = layout; trk; trk = trk->next) {
	    if(trk->type == TEXT && trk->station &&
                ((trk->wlinkx && trk->wlinky) || (trk->elinkx && trk->elinky))) {
                    if (trk->station[0] == '<') // block marker - remove
                        continue;
                    tmp.Printf(wxT("%c    { \"name\": \"%s\", \"x\": %d, \"y\": %d }\n"), sep1, trk->station, trk->x, trk->y);
                    out.append(tmp);
                    sep1 = ',';
            }
        }
        out.append(wxT("]\n}\n"));
        return true;
}


extern  int late_data[60 * 24]; // 60 minutes * 24 hours

class WS_LateData : public Servlet
{
public:
        WS_LateData() : Servlet("/war/latedata.json") { };
        ~WS_LateData() { };
        const char *getMimeType() const { return MIME_JSON; };

        bool    get(wxString& out, Char *args);
} ws_latedata;

bool    WS_LateData::get(wxString& out, Char *args)
{
        int     h, m;
        Char    buff[256];
        Char    *sep = wxT("");
        int     last = 0;

        out.append(wxT("{ \"latedata\": [\n"));
        for(h = 0; h < 24; ++h) {
            for(m = 0; m < 64; ++m) {
                if(late_data[h * 60 + m] != last) {
                    last = late_data[h * 60 + m];
                    wxSprintf(buff, wxT("%s{ \"hour\": %d,\n  \"min\": %d,\n  \"late\": %d }\n"), sep, h, m, last);
                    out.append(buff);
                    sep = wxT(",\n");
                }
            }
        }
        out.append(wxT("  ]\n}\n"));
        return true;
}

class WS_TimeDistance : public Servlet
{
public:
        WS_TimeDistance() : Servlet("/war/timedist.yaml") { };
        ~WS_TimeDistance() { };
        const char *getMimeType() const { return MIME_TEXT; };

        bool    get(wxString& out, Char *args);
} ws_timedistance;

bool    WS_TimeDistance::get(wxString& out, Char *args)
{
        int     i;
        Char    buff[256];
        GraphViewData gd;

        gd.ComputeStationsPositions();
        gd.DrawTrains();
        out.append(wxT("stations:\n"));
        for(i = 0; i < gd.nStations; ++i) {
            wxSprintf(buff, wxT("- name: %s\n  y: %d\n"), gd.stationNames[i], gd.yStations[i]);
            out.append(buff);
        }
        out.append(wxT("trains:\n"));
        GraphLine *gl;
        for(gl = gd.lines; gl; gl = gl->next) {
            wxSprintf(buff, wxT("- x0: %d\n  y0: %d\n  x1: %d\n  y1: %d\n  color: %d\n"),
                gl->x0, gl->y0, gl->x1, gl->y1, gl->color);
            out.append(buff);
        }
        return true;
}

class WS_PlatformsGraph : public Servlet
{
public:
        WS_PlatformsGraph() : Servlet("/war/platforms.yaml") { };
        ~WS_PlatformsGraph() { };
        const char *getMimeType() const { return MIME_TEXT; };

        bool    get(wxString& out, Char *args);
} ws_platformsGraph;

bool    WS_PlatformsGraph::get(wxString& out, Char *args)
{
        int     i;
        Char    buff[1024];
        PlatformGraphData pd;
        PlatformSegment *seg;

        pd.Clear();
        pd.ComputeStations();
        pd.ComputeTrains();

        out.append(wxT("stations:\n"));
        for(i = 0; i < pd.nStations; ++i) {
            wxSprintf(buff, wxT("- name: %s\n"), pd.stations[i]->station);
            out.append(buff);
        }
        out.append(wxT("trains:\n"));
        for(seg = pd.segments; seg; seg = seg->next) {
            wxSprintf(buff, wxT("- train: %s\n  x0: %d\n  x1: %d\n  y: %d\n  in: %s\n"),
                seg->train->name, seg->x0, seg->x1, seg->y, format_time(seg->timein));
            out.append(buff);
            wxSprintf(buff, wxT("  out: %s\n  color: %d\n"), format_time(seg->timeout), getcolor_rgb(seg->train->type));
            out.append(buff);
            if(seg->parent) {
                wxSprintf(buff, wxT("  from: %s\n"), seg->parent->name);
                out.append(buff);
            }
        }
        return true;
}


class WS_Switchboards : public Servlet
{
public:
        WS_Switchboards() : Servlet("/war/switchboards.yaml") { };
        ~WS_Switchboards() { };

        bool    get(wxString& out, Char *args);
} ws_switchboards;

bool    WS_Switchboards::get(wxString& out, Char *args)
{
	SwitchBoard *sb;

	for(sb = switchBoards; sb; sb = sb->_next) {
            sb->toYAML(out);
	}
        return true;
}



class WS_Element : public Servlet
{
public:
        WS_Element() : Servlet("/war/element.yaml") { };
        ~WS_Element() { };

        bool    get(wxString& out, Char *args);
} ws_element;

bool    WS_Element::get(wxString& out, Char *args)
{
        int x = -1, y = -1;
        Char    *p, *d;
        Char    value[128];
        Char    arg[128];
        Char    buff[512];
        Char    link[512];

        FromUrl(buff, args);
        p = buff;
        while(*p) {
            d = arg;
            while(*p && *p != '=' && *p != '&')
                *d++ = *p++;
            *d = 0;
            if(*p == '=') {
                ++p;
                d = value;
                while(*p && *p != '&')
                    *d++ = *p++;
                *d = 0;
                if(*p)
                    ++p;
            } else
                value[0] = 0;
            if(!wxStrcmp(arg, wxT("x"))) {
                x = wxAtoi(value);
            } else if(!wxStrcmp(arg, wxT("y"))) {\
                y = wxAtoi(value);
            }
        }
        if(x < 0 || y < 0) {
            out.append(wxT("error: Missing coordinate\n"));
            return true;
        }
        Track *t;
       	for(t = layout; t; t = t->next)
	    if(t->x == x && t->y == y)
		break;
        if(t) {
            out.append(wxT("element:\n"));
            
            wxSprintf(buff, wxT("  type: %s\n"), t->GetTypeName());
            out.append(buff);
            wxSprintf(buff, wxT("  name: %s\n"), t->station ? t->station : wxT(""));
            out.append(buff);
            wxSprintf(buff, wxT("  length: %d\n"), t->length);
            out.append(buff);
            if(t->type == TSIGNAL) {
                Signal *sig = (Signal *)t;
                wxSprintf(buff, wxT("  aspect: %s\n"), sig->GetAspect());
                out.append(buff);
                if(sig->controls) {
                    wxSprintf(buff, wxT("  controls: (%d,%d)\n"), sig->controls->x, sig->controls->y);
                    out.append(buff);
                }
            } else if(t->type == TRACK || t->type == SWITCH) {
                p = buff;
                for(int xx = 0; xx < NTTYPES; ++xx) {
		    wxSprintf(p, wxT("%d/"), t->speed[xx]);
		    p += wxStrlen(p);
	        }
                out.append(wxT("  speeds: "));
                out.append(buff);
                out.append(wxT("\n"));
            }
        }
        Train *train = findTrain(x, y);
        if(train) {
            out.append(wxT("train:\n"));
            out.append(wxT("  name: "));
            out.append(train->name);
            out.append(wxT("\n  from: "));
            out.append(train->entrance);
            out.append(wxT("\n  to: "));
            out.append(train->exit);
            out.append(wxT("\n  status: "));
            out.append(train->GetStatusName());
            out.append(wxT("\n  link: "));
            ToHex(link, sizeof(link), train->name);
            out.append(link);
            if(train->status == train_STOPPED) {
                out.append(wxT("\n  etd: "));
                out.append(format_time(train->timedep));
            }
            if(train->flags & TFLG_STRANDED) {
                out.append(wxT("\n  stranded: 1"));
            }
            if(train->shunting) {
                out.append(wxT("\n  shunting: 1"));
            }
            out.append(wxT("\n"));
        }
        return true;
}

class WS_Shapes : public Servlet
{
public:
        WS_Shapes() : Servlet("/war/shapes.yaml") { };
        ~WS_Shapes() { };

        bool    get(wxString& out, Char *args);
} ws_shapes;

#define SHAPE_ROW_SIZE  2040

bool    WS_Shapes::get(wxString& out, Char *args)
{
        int     i;
        Char    buff[SHAPE_ROW_SIZE];
        Char    *p;

        out.Append(wxT("shapes:\n"));
        for(i = 0; i < shapeManager._shapes.Length(); ++i) {
            Shape *shape = shapeManager._shapes.At(i);
            int w = shape->GetWidth();
            int h = shape->GetHeight();
            wxSprintf(buff, wxT("  width: %d\n  height: %d\n"), w, h);
            out.Append(wxT("- id: ")).Append(shape->_id->c_str());
            out.Append(wxT("\n")).Append(buff);
            out.Append(wxT("  rows:\n"));
            unsigned char *img = shape->GetRGBA();
            int x, y;
            int offs = 0;
            for(y = 0; y < h; ++y) {
                p = buff;
                out.Append(wxT("    - row: "));
                Char *sep = wxT("\"");
                for(x = 0; x < w; ++x) {
                    wxSprintf(p, wxT("%s%d,%d,%d,%d"), sep, img[offs], img[offs + 1], img[offs + 2], img[offs + 3]);
                    offs += 4;
                    p += wxStrlen(p);
                    sep = wxT(",");
                    if(p - buff > SHAPE_ROW_SIZE - 64) {
                        out.Append(buff);
                        p = buff;
                        *p = 0;
                    }
                }
                if(wxStrlen(buff) > 0)
                    out.Append(buff);
                out.Append(wxT("\"\n"));
            }
        }
        out.Append(wxT("canvas:\n"));
        wxSprintf(buff, wxT("  width: %d\n  height: %d\n"), XMAX, YMAX);
        out.Append(buff);
        return true;
}

class WS_ShapesJSON : public Servlet
{
public:
        WS_ShapesJSON() : Servlet("/war/shapes.json") { };
        ~WS_ShapesJSON() { };
        const char *getMimeType() const { return MIME_JSON; };

        bool    get(wxString& out, Char *args);
} ws_shapesJSON;


bool    WS_ShapesJSON::get(wxString& out, Char *args)
{
        int     i;
        Char    buff[SHAPE_ROW_SIZE];
        Char    *p;
        const Char *shapeSep = wxT("\n{\n");

        out.Append(wxT("{ \n\"canvas\": {\n"));
        json(out, wxT("width"), XMAX, false);
        json(out, wxT("height"), YMAX, true);
        out.Append(wxT("},\n\"shapes\": [\n"));
        for(i = 0; i < shapeManager._shapes.Length(); ++i) {
            Shape *shape = shapeManager._shapes.At(i);
            int w = shape->GetWidth();
            int h = shape->GetHeight();
            out.Append(shapeSep);
            json(out, wxT("id"), shape->_id->c_str());
            json(out, wxT("width"), w);
            json(out, wxT("height"), h);
            json(out, wxT("builtin"), shape->_builtin ? 1 : 0);
            out.Append(wxT("\n\"rows\": ["));
            unsigned char *img = shape->GetRGBA();
            int x, y;
            int offs = 0;
            const Char *rowSep = wxT("\n [");
            for(y = 0; y < h; ++y) {
                p = buff;
                out.Append(rowSep);
                Char *sep = wxT("");
                for(x = 0; x < w; ++x) {
                    wxSprintf(p, wxT("%s%d,%d,%d,%d"), sep, img[offs], img[offs + 1], img[offs + 2], img[offs + 3]);
                    offs += 4;
                    p += wxStrlen(p);
                    sep = wxT(",");
                    if(p - buff > SHAPE_ROW_SIZE - 64) {
                        out.Append(buff);
                        p = buff;
                        *p = 0;
                    }
                }
                if(wxStrlen(buff) > 0)
                    out.Append(buff);
                rowSep = wxT("],\n [");
            }
            if(p > buff)
                out.Append(wxT("]\n"));
            shapeSep = wxT("] },\n{\n");
        }
        if(*shapeSep == ']') // we emitted at least one shape, so close it
            out.Append(wxT("] }\n"));
        out.Append(wxT("\n]}\n")); // then close the array of shapes and the document
        return true;
}


class WS_Layout : public Servlet
{
public:
        WS_Layout() : Servlet("/war/layout.yaml") { };
        ~WS_Layout() { };

        bool    get(wxString& out, Char *args);
} ws_layout;

bool    WS_Layout::get(wxString& out, Char *args)
{
        Char    buff[512];

        out.Append(wxT("layout:\n"));
        Track   *trk;
        
        for(trk = layout; trk; trk = trk->next) {
            wxSprintf(buff, wxT("- x: %d\n  y: %d\n  type: %d\n  dir: %d\n"), trk->x, trk->y, trk->type, trk->direction);
            if(trk->_shape)
                wxSprintf(buff + wxStrlen(buff), wxT("  shape: %s\n"),  trk->_shape->_id->c_str());
            if(trk->type == SWITCH)
                wxSprintf(buff + wxStrlen(buff), wxT("  switched: %d\n"), trk->switched);
            if(trk->type == TSIGNAL) {
                Signal *sig = (Signal *)trk;
                if(!sig->_interpreterData)
                    continue;
                Shape *shape = sig->FindShape();
                if(shape)
                    wxSprintf(buff + wxStrlen(buff), wxT("  shape: %s\n"), shape->_id->c_str());
            } else if(trk->type == IMAGE && trk->station) {
                wxSprintf(buff + wxStrlen(buff), wxT("  shape: %s\n"), trk->station);
            }
            out.Append(buff);
        }
        return true;
}

class WS_LayoutJson : public Servlet
{
public:
        WS_LayoutJson() : Servlet("/war/layout.json") { };
        ~WS_LayoutJson() { };

        bool    get(wxString& out, Char *args);
} ws_layoutJson;

bool    WS_LayoutJson::get(wxString& out, Char *args)
{
        out.Append(wxT("{\n"));
        getLayout(*this, out);
        out.Append(wxT("}\n"));
        return true;
}


class WS_TrainPositions : public Servlet
{
public:
        WS_TrainPositions() : Servlet("/war/trainpositions.json") { };
        ~WS_TrainPositions() { };
        const char *getMimeType() const { return MIME_JSON; };

        bool    get(wxString& out, Char *args);
} ws_trainPositions;

bool    WS_TrainPositions::get(wxString& out, Char *args)
{
        Train   *trn;
        const Char    *sep = wxT("{\n");
        
        out.Append(wxT("{ \"trains\": [\n"));
        for(trn = schedule; trn; trn = trn->next) {
            switch(trn->status) {
            case train_RUNNING:
            case train_STOPPED:
            case train_ARRIVED:
            case train_WAITING:
            case train_STARTING:
                break;
            default:
                continue;
            }
            out.Append(sep);
            json(out, wxT("name"), trn->name, true);
            if(trn->position) {
                out.Append(wxT(", \"loco\": { "));
                Shape *shape = trn->direction == W_E ? trn->epix : trn->wpix;
                if(shape) {
                    json(out, wxT("shape"), shape->_id->c_str(), false);
                }
                json(out, wxT("x"), trn->position->x, false);
                json(out, wxT("y"), trn->position->y, true);
                out.Append(wxT(" }\n"));
            }
            if(trn->tail && trn->tail->position && trn->tail->position != trn->position) {
                out.Append(wxT(", \"car\": { "));
                Shape *shape = trn->direction == W_E ? trn->ecarpix : trn->wcarpix;
                if(shape) {
                    json(out, wxT("shape"), shape->_id->c_str(), false);
                }
                json(out, wxT("x"), trn->tail->position->x, false);
                json(out, wxT("y"), trn->tail->position->y, true);
                out.Append(wxT(" }\n"));
            }
            out.Append(wxT("}\n"));
            sep = wxT(",\n{\n");
        }
        out.Append(wxT("\n]\n}\n"));
        return true;
}


#include <wx/socket.h>
#include <wx/file.h>
#include <wx/stdpaths.h>

bool    ws_process(Char *cmd, wxSocketBase *out)
{
        const Char    *p = cmd;

        if(match(&p, wxT("fullstate"))) {
            out->Write(wxT("Hello world\n"), 12);
            return true;
        }
        return false;
}

extern  void    get_localized_strings_yaml(wxString& out);

class WS_Translations : public Servlet
{
public:
        WS_Translations() : Servlet("/war/translations.yaml") { };
        ~WS_Translations() { };

        bool    get(wxString& out, Char *args);
} ws_Translations;

bool    WS_Translations::get(wxString& out, Char *args)
{
        get_localized_strings_yaml(out);
        return true;
}



//----------------------------------------------------------


#include "ClientConnection.h"


extern  int mainLoopIteration;

class WSPoll : public Servlet
{
public:
    WSPoll() : Servlet("/poll/") { };
    ~WSPoll() { };
    const char *getMimeType() const { return MIME_JSON; };

    void    collectPart(wxString& out, Char *name)
    {
        if(!wxStrcmp(name, wxT("trains"))) {
            getAllTrains(*this, out);
            return;
        }
        if(!wxStrcmp(name, wxT("alerts"))) {
            getAlerts(*this, out);
            return;
        }
        if(!wxStrcmp(name, wxT("status"))) {
            getStatus(out);
            return;
        }
        if(!wxStrcmp(name, wxT("layout"))) {
            getLayout(*this, out);
            return;
        }
        if(!wxStrcmp(name, wxT("info"))) {
            // TODO: use web client locale
            // TODO: use web client page name (multi-page info)
            // TODO: abstract this block so it can be used for GUI too
            TDFile	infoFile(info_page);

            bool loaded = true;

            wxString ext;
            ext.Printf(wxT("%s.htm"), locale_name);
            infoFile.SetExt(ext);
            if(!infoFile.Load()) {
                infoFile.SetExt(wxT(".htm"));
	        if(!infoFile.Load()) {
                    infoFile.SetName(wxT("index.html"));
	            if(!infoFile.Load()) {
                        infoFile.SetName(wxT("index.htm"));
	                if(!infoFile.Load()) {
                            loaded = false;
                        }
	            }
	        }
            }

            Char    buff[1024];
            buff[sizeof(buff) / sizeof(buff[0]) - 1] = 0;
            /*
            Char    escaped[1024];
            out.Append(wxT("\"info\": ["));
            Char *sep = wxT("\n\"");
            if(loaded) {
                while(infoFile.ReadLine(buff, sizeof(buff) / sizeof(buff[0]) - 1)) {
                    Servlet::escape(escaped, sizeof(escaped) / sizeof(escaped[0]), buff);
                    out.Append(sep).Append(escaped);
                    sep = wxT("\",\n\"");
                }
            }
            if(*sep == '"')
                out.Append(wxT("\"\n]\n"));
            else
                out.Append(wxT("\n]\n"));
            */
            ext = infoFile.name.GetFullName();
            const Char *name = ext.c_str();
            json(out, wxT("info"), name, true);
            return;
        }
        if(!wxStrcmp(name, wxT("sboard"))) {
	    SwitchBoard *sb;
            const Char    *sep = wxT("\n");

            out.Append(wxT("\"switchboards\": ["));
	    for(sb = switchBoards; sb; sb = sb->_next) {
                out.Append(sep);
                sb->toJSON(out);
                sep = wxT(",");
	    }
            out.Append(wxT("\n]\n"));
            return;
        }
    }

    bool    get(wxString& out, Char *args)
    {
        long id = 0;
        Char    parts[512];
        Char    *pargs = args;
        Char    *part;

        parts[0] = 0;
        while(*pargs) {
            if(*pargs == '&')
                ++pargs;
            if (!wxStrncmp(pargs, wxT("parts="), 6)) {
                part = parts;
                for(pargs += 6; *pargs && *pargs != '&'; ) {
                    *part++ = *pargs++;
                    if(part == &parts[sizeof(parts)/sizeof(parts[0]) - 2])
                        break;
                }
                *part = 0;
                continue;
            }
        }
        if(!quitting) {
            ClientConnection *cconn = AddConnection();
            BlockConnection(cconn->_id);
            RemoveConnection(cconn->_id);
            delete cconn;
        }
        Char    *sep = wxT("");
        out.Append(wxT("{\n"));
        if(quitting) {
            out.append(wxT("\"status\": {\n"));
            Servlet::json(out, wxT("quitting"), 1, true);
            out.append(wxT("}\n"));
        } else {
            part = parts;
            for(pargs = parts; *pargs; ) {
                if(*pargs == ',') {
                    *pargs++ = 0;
                    out.Append(sep);
                    collectPart(out, part);
                    sep = wxT(",");
                    part = pargs;
                    continue;
                }
                ++pargs;
            }
            if(part < pargs) {
                out.Append(sep);
                collectPart(out, part);
                sep = wxT(",");
            }
        }
        out.Append(wxT("\n}\n"));
        return true;
    }
} wsPoll;


