/*	Train.cpp - Created by Giampiero Caprino

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

#include "Traindir3.h"
#include "trsim.h"
#include "Train.h"

extern  const Char    *eventName;   // for script diagnostic
extern  Char    eventObject[];      // for script diagnostic
extern  void    parserError(const Char *msg, const Char *ptr);

extern  Shape   *e_train_pmap[], *w_train_pmap[];
extern  Shape   *e_car_pmap[], *w_car_pmap[];

class TrainInterpreterData : public InterpreterData {
public:
	TrainInterpreterData()
	{
	    _onInit = 0;
	    _onRestart = 0;
	    _onEntry = 0;
	    _onExit = 0;
	    _onStop = 0;
	    _onWaiting = 0;
	    _onStart = 0;
	    _onReverse = 0;
	    _onAssign = 0;
	    _onShunt = 0;
	    _onArrived = 0;
            _onMerged = 0;
	}

	~TrainInterpreterData()
	{
	    if(_onInit)
		delete _onInit;
	    if(_onRestart)
		delete _onRestart;
	    if(_onEntry)
		delete _onEntry;
	    if(_onExit)
		delete _onExit;
	    if(_onWaiting)
		delete _onWaiting;
	    if(_onStop)
		delete _onStop;
	    if(_onStart)
		delete _onStart;
	    if(_onReverse)
		delete _onReverse;
	    if(_onShunt)
		delete _onShunt;
	    if(_onAssign)
		delete _onAssign;
	    if(_onArrived)
		delete _onArrived;
	    if(_onMerged)
		delete _onMerged;
	};

	Statement *_onInit;	// list of actions (statements)
	Statement *_onRestart;	// list of actions (statements)
	Statement *_onEntry;
	Statement *_onExit;
	Statement *_onStop;
	Statement *_onWaiting;
	Statement *_onStart;
	Statement *_onReverse;
	Statement *_onAssign;
	Statement *_onShunt;
	Statement *_onArrived;
	Statement *_onMerged;

	bool	Evaluate(ExprNode *n, ExprValue& result);
//	bool	GetNextPath(Track *, Vector **path);
};


Train::Train()
{
	next = 0;
	name = 0;		/* train name or number */
	status = train_READY;	/* status: running, waiting etc. */
	sdirection = W_E;	/* starting direction: W_E or E_W */
	direction = W_E;	/* current direction: W_E or E_W */
	timein = 0;		/* time it shows up on territory */
	timeout = 0;		/* time it should be out of territory */
	entrance = 0;
	exit = 0;
	exited = 0;		/* if wrongdest, where we exited */
        alternateExits = 0;     // 3.9r
        alternateEntries = 0;   // 3.9r
	timeexited = 0;		/* when we exited */
	memset(notes, 0, sizeof(notes));
	nnotes = 0;
	wrongdest = 0;		/* train arrived at wrong destination */
	type = 0;		/* train type */
	_gotDelay = 0;		/* we computed a delay upon entry in the territory */
	_inDelay = 0;		/* the computed delay, in minutes */
	newsched = 0;		/* must update schedule window for this train */
	curspeed = 0;		/* current speed */
	maxspeed = 0;		/* absolute maximum speed */
	curmaxspeed = 0;	/* current (absolute or track) maximum speed */
	speedlimit = 0;		/* last speed limit seen */
	timelate = 0;		/* minutes late arriving at all stations */
	timedelay = 0;		/* minutes late entering territory */
	timered = 0;		/* minutes stopped at red signal */
	trackpos = 0;		/* how much of lengthy tracks we travelled */
	stops = 0;		/* list of scheduled stops */
	laststop = 0;		/* last in list of scheduled stops */
	length = 0;		/* current train length in meters */
	entryLength = 0;	/* train length at entry into territory */
//	pathpos = 0;		/* index into path[] of train head */
	path = 0;		/* track elements to be travelled by train head */
	tail = 0;		/* descriptor of train's end (if length != 0) */
	pathtravelled = 0;	/* meters travelled in current path */
	disttostop = 0;		/* distance until next stop */
	stoppoint = 0;
	disttoslow = 0;	    	/* distance until next speed limit signal */
	slowpoint = 0;
	position  = 0;		/* where the train is in the territory */
	timedep = 0;		/* expected time of departure from station */
	fleet = 0;		/* list of signals waiting for tail to pass */
	waitfor = 0;		/* must wait for this train to exit territory*/
	waittime = 0;	    	/* how many minutes after waitee has arrived we depart */
	stock = 0;		/* next train which uses this train's stock */
        epix = e_train_pmap[0];
        wpix = w_train_pmap[0];	/* shapes to east and west pixmaps */
	ecarpix = e_car_pmap[0];
        wcarpix = w_car_pmap[0];/* shapes of east and west car pixmaps (if length != 0) */
	outof = 0;		/* ignore this station when checking shunting */
	stopping = 0;		/* we are stopping/stopped at this station */
	oldstatus = train_READY;
	arrived = 0;		/* if true we are just shunting */
	shunting = 0;
	days = 0;		/* which day this train is running */
	flags = 0;		/* performance flags (TFLG_*) */
	needfindstop = 0;	/* terrible hack! */
	merging = 0;		/* will merge with this train */
	entryDelay = 0;
        isExternal = 0;         /* train does not run in this scenario */
        startDelay = 0;
        myStartDelay = 0;
        accelRate = 0;          // 3.8r
        power = 0;              // 3.9

		sta_waittime = 0;//lrg 20211017
		it_starts = 0;//lrg
		it_ends = 0;
		last_it = 0;
		selectline_flag = 0; //lrg 20210221 

	

	// start to use C++ methods to make the code cleaner

	stateProgram = 0;
	_interpreterData = 0;
        _lastUpdate = 0;
}

Train::~Train()	    // recursive for tails!
{
	TrainStop *ts, *ts1;

	if(this->tail) {
	    delete this->tail;
	    this->tail = 0;
	}
	if(this->path)
	    Vector_delete(this->path);
	if(this->name)
	    free(this->name);
	if(this->entrance)
	    free(this->entrance);
	if(this->exit)
	    free(this->exit);
	if(this->alternateExits)
	    free(this->alternateExits);
	for(ts = this->stops; ts; ts = ts1) {
	    ts1 = ts->next;
	    if(ts->station)
		free(ts->station);
	    free(ts);
	}
}


void    Train::Get(TrainInfo& info)
{
	Char    buff[1024];

	if(_gotDelay && _inDelay) {
	    wxSnprintf(buff, sizeof(buff)/sizeof(Char), wxT("%s"), format_time(this->timein));
	    wxSnprintf(buff + wxStrlen(buff), sizeof(buff)/sizeof(Char), wxT(" (%s)"), format_time(this->timein + this->_inDelay));
	    wxStrncpy(info.entering_time, buff, sizeof(info.entering_time)/sizeof(Char));
	} else
	    wxStrcpy(info.entering_time, format_time(this->timein));
	wxStrcpy(info.leaving_time, format_time(this->timeout));
	wxSnprintf(info.current_speed, sizeof(info.current_speed)/sizeof(wxChar), wxT("%d"), (int)this->curspeed);
	wxSnprintf(info.current_delay, sizeof(info.current_delay)/sizeof(wxChar), wxT("%d"), this->timedelay / 60);
	wxSnprintf(info.current_late, sizeof(info.current_late)/sizeof(wxChar), wxT("%d"), this->timelate);
	//disp_columns[4] = this->name;
	//disp_columns[1] = this->entrance;
	//disp_columns[2] = this->exit;
	wxStrcpy(info.current_status, train_status(this));
/*	wxSnprintf(current_status + wxStrlen(current_status), sizeof(current_status)/sizeof(wxChar) - wxStrlen(current_status),
	    wxT("  pos: %ld - %ld"), t->pathtravelled, t->trackpos);*/
}

Track	*path_find_station(Vector *path, Track *headpos)
{
	int	i;
	Track	*trk;

	for(i = path->_size - 1; i >= 0; --i) {
	    trk = path->TrackAt(i);
	    if(trk == headpos)
		break;
	}
	for(; i >= 0; --i) {
	    trk = path->TrackAt(i);
	    if(trk->station)
		return trk;
	}
	return 0;
}

bool    Train::Get(AssignInfo& info)
{
	Track	*trk;
        Train   *t;

	trk = this->position;
	if(trk && (!trk->station && this->tail && this->tail->path)) {
	    trk = path_find_station(this->tail->path, this->position);
	    if(!trk)
		trk = this->position;
	}
	if(!trk || !trk->station)
	    return false;
	info.assign_tr = this;
        info.track = trk;
	for(t = schedule; t; t = t->next) {
	    if(t->status == train_READY && 
		    sameStation(t->entrance, trk->station) &&	// 3.4: was assign_tr->exit
		    (!t->days || (t->days & run_day))) {
                info.candidates.Add(t);
	    }
	}
	if(this->stock) {
            int i;
            info.assign_to = -1;
	    for(i = 0; i < info.candidates.Length(); ++i)
		if(!wxStrcmp(this->stock, info.candidates[i]->name)) {
                    info.assign_to = i;
                    break;
		}
	}
        return true;
}

const Char *Train::GetStatusName() const
{
        switch(status) {
        case train_READY:    return wxT("ready");
        case train_ARRIVED:  return wxT("arrived");
        case train_DERAILED: return wxT("derailed");
        case train_DELAY:    return wxT("delayed");
        case train_RUNNING:  return wxT("running");
        case train_STARTING: return wxT("starting");
        case train_STOPPED:  return wxT("stopped");
        case train_WAITING:  return wxT("waiting");
        }
        return wxT("?");
}

bool    Train::CanTravelOn(Vector *path)
{
        int     i;
        
        if(!this->power)    // no power specified, means can travel anywhere
            return true;
        for(i = 0; i < path->_size; ++i) {
            Track *trk = path->TrackAt(i);
            if(trk->type == TEXT) // we reached an exit
                return true;
            if(!trk->power)
                return false;
            if(wxStrcmp(trk->power, this->power)) {
                return false; // different power specified (e.g. 3000V vs. 10000V)
            }
        }
        return true;
}

// 3.9r: check that name is one of the allowed alternate exits, if any
// the alternateExits can be one or more station names, separated by comma

bool    Train::IsAllowedAlternate(const Char *name, const Char *alts)
{
        Char    buff[1024];
        const Char *s;
        Char    *d;

        if(!(s = alts))
            return false;
        for(d = buff; *s; ) {
            if(*s == ',' || *s == '|') {
                while(d > buff && d[-1] == ' ')
                    --d;
                *d = 0;
                for(d = buff; *d == ' '; ++d);
                if(sameStation(name, d))
                    return true;
                d = buff;
                ++s;
            }
            *d++ = *s++;
        }
        while(d > buff && d[-1] == ' ')
            --d;
        if(d > buff) {
            *d = 0;
            for(d = buff; *d == ' '; ++d);
            if(sameStation(name, d))
                return true;
        }
        return false;
}

wxString& Train::GetNameAndAlternates(wxString& out, const Char *name, const Char *alternates)
{
        out.Clear();
        out.Append(name);
        if(alternates) {
            out.Append(wxT(" (")).Append(alternates).Append(wxT(")"));
        }
        return out;
}

void	Train::ParseProgram()
{
	const wxChar	*p;

	if(!this->stateProgram || !*this->stateProgram)
	    return;
	if(_interpreterData)	    // previous script
	    delete (TrainInterpreterData *)_interpreterData;
	_interpreterData = new TrainInterpreterData;

	TrainInterpreterData *interp = (TrainInterpreterData *)_interpreterData;
	p = this->stateProgram;
        wxSprintf(eventObject, wxT("Train(%s) "), this->name);
	while(*p) {
	    const wxChar	*p1 = p;
	    while(*p1 == ' ' || *p1 == '\t' || *p1 == '\r' || *p1 == '\n')
		++p1;
	    p = p1;
	    if(match(&p, eventName = wxT("OnInit:"))) {
		p1 = p;
		interp->_onInit = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnRestart:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onRestart = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnEntry:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onEntry = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnExit:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onExit = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnStop:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onStop = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnWaiting:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onWaiting = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnStart:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onStart = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnAssign:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onAssign = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnArrived:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onArrived = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnMerged:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onMerged = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnReverse:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onReverse = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnShunt:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onShunt = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("procedure ")) || match(&p, eventName = wxT("proc ")) ||
                      match(&p, eventName = wxT("procedure\t")) || match(&p, eventName = wxT("proc\t"))) {
                p1 = parse_proc(p);
	    }
            eventName = 0;
            if(p1 == p) {    // error! couldn't parse token
                if(*p)
                    parserError(wxT("Unknown event name"), p);
		break;
            }
	}
        eventObject[0] = 0;
}

///
///	TRAIN  STATE
///


bool	Train::GetPropertyValue(const wxChar *prop, ExprValue& result)
{
        bool    ignoreTransits = false;

	if(!wxStrcmp(prop, wxT("name"))) {
	    result._op = String;
	    result._txt = this->name;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("type"))) {
	    result._op = Number;
	    result._val = this->type;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("speed"))) {
	    result._op = Number;
	    result._val = this->curspeed;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("length"))) {
	    result._op = Number;
	    result._val = this->length;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("arrived"))) {
	    result._op = Number;
	    result._val = this->status == train_ARRIVED;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("stopped"))) {
	    result._op = Number;
	    result._val = this->status == train_STOPPED;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("direction"))) {
	    result._op = Number;
	    result._val = this->direction;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("days"))) {
	    result._op = Number;
	    result._val = this->days;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("entry"))) {
	    result._op = String;
	    result._txt = this->entrance;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("exit"))) {
	    result._op = String;
	    result._txt = this->exit;
	    return true;
	}
        if(!wxStrcmp(prop, wxT("station"))) {
	    result._op = String;
            if(this->position && this->position->station)
	        result._txt = this->position->station;
            else
                result._txt = wxT("?");
            return true;
        }
	if(!wxStrcmp(prop, wxT("nextStation")) || (ignoreTransits = !wxStrcmp(prop, wxT("nextStop")))) {
	    TrainStop *stop = this->stops;
            while(stop) {
                if(!stop->stopped) { // we never stopped here
                    if(!ignoreTransits) // and we want to consider transits
                        break; // so use this stop
                    if(stop->minstop) // we stop here (not a transit)
                        break; // so use this stop
                }
		stop = stop->next;
            }
            if(stop) {
	        result._txt = stop->station;
            } else {
                result._txt = this->exit;
            }
            result._op = String;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("stock"))) {
	    result._op = String;
	    result._txt = this->stock;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("waitfor"))) {
	    result._op = String;
	    result._txt = this->waitfor;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("status"))) {
	    result._op = String;
	    switch(this->status) {
	    case train_READY:
		result._txt = wxT("ready");
		break;
	    case train_RUNNING:
		if(this->shunting)
		    result._txt = wxT("shunting");
		else
		    result._txt = wxT("running");
		break;
	    case train_STOPPED:
		result._txt = wxT("stopped");
		break;
	    case train_DELAY:
		result._txt = wxT("delayed");
		break;
	    case train_WAITING:
		result._txt = wxT("waiting");
		break;
	    case train_DERAILED:
		result._txt = wxT("derailed");
		break;
	    case train_ARRIVED:
		result._txt = wxT("arrived");
		break;
	    }
	    return true;
	}
	return false;
}


bool	Train::SetPropertyValue(const wxChar *prop, ExprValue& value)
{
	if(!wxStrcmp(prop, wxT("shunt"))) {
	    if(status != train_STOPPED && status != train_WAITING && status != train_ARRIVED) {
		do_alert(L("Train is not stopped nor arrived."));
		return false;
	    }
	    shunt_train(this);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("wait"))) {
	    if(status != train_RUNNING || curspeed != 0) {
		do_alert(L("Train is not stopped nor arrived."));
		return false;
	    }
	    timedep += value._val;
	    return true;
	}
	return false;
}


///
///	TRAIN  EVENTS
///


void	Train::OnEntry()
{
	if(_interpreterData) {
	    TrainInterpreterData& interp = *(TrainInterpreterData *)_interpreterData;
	    if(interp._onEntry) {
		interp._train = this;
		interp._track = this->position;
		interp._signal = 0;
                interp._scopes.Clear();
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnEntry(%d,%d)"), this->name, this->position->x, this->position->y);
		Trace(expr_buff);
		interp.Execute(interp._onEntry);
		return;
	    }
	}
}


void	Train::OnStart()
{
	if(_interpreterData) {
	    TrainInterpreterData& interp = *(TrainInterpreterData *)_interpreterData;
	    if(interp._onStart) {
		interp._train = this;
		interp._track = this->position;
		interp._signal = 0;
                interp._scopes.Clear();
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnStart(%d,%d)"), this->name, this->position->x, this->position->y);
		Trace(expr_buff);
		interp.Execute(interp._onStart);
		return;
	    }
	}
}


void	Train::OnStopped()
{
	if(_interpreterData) {
	    TrainInterpreterData& interp = *(TrainInterpreterData *)_interpreterData;
	    if(interp._onStop) {
		interp._train = this;
		interp._track = this->position;
		interp._signal = 0;
                interp._scopes.Clear();
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnStopped(%d,%d)"), this->name, this->position->x, this->position->y);
		Trace(expr_buff);
		interp.Execute(interp._onStop);
		return;
	    }
	}
}


void	Train::OnWaiting(Signal *sig)
{
	if(_interpreterData) {
	    TrainInterpreterData& interp = *(TrainInterpreterData *)_interpreterData;
	    if(interp._onWaiting) {
		interp._train = this;
		interp._track = this->position;
		interp._signal = sig;
                interp._scopes.Clear();
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnWaiting(%d,%d)"), this->name, this->position->x, this->position->y);
		Trace(expr_buff);
		interp.Execute(interp._onWaiting);
		return;
	    }
	}
}


void	Train::OnExit()
{
	if(_interpreterData) {
	    TrainInterpreterData& interp = *(TrainInterpreterData *)_interpreterData;
	    if(interp._onExit) {
		interp._train = this;
		interp._track = 0;
		interp._signal = 0;
                interp._scopes.Clear();
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnExit()"), this->name);
		Trace(expr_buff);
		interp.Execute(interp._onExit);
		return;
	    }
	}
}


void	Train::OnArrived()
{
	if(_interpreterData) {
	    TrainInterpreterData& interp = *(TrainInterpreterData *)_interpreterData;
	    if(interp._onArrived) {
		interp._train = this;
		interp._track = this->position;
		interp._signal = 0;
                interp._scopes.Clear();
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnArrived(%d,%d)"), this->name, this->position->x, this->position->y);
		Trace(expr_buff);
		interp.Execute(interp._onArrived);
		return;
	    }
	}
}


void	Train::OnAssign()
{
	if(_interpreterData) {
	    TrainInterpreterData& interp = *(TrainInterpreterData *)_interpreterData;
	    if(interp._onAssign) {
		interp._train = this;
		interp._track = this->position;
		interp._signal = 0;
                interp._scopes.Clear();
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnAssign(%d,%d)"), this->name, this->position->x, this->position->y);
		Trace(expr_buff);
		interp.Execute(interp._onAssign);
		return;
	    }
	}
}


void	Train::OnMerged()
{
	if(_interpreterData) {
	    TrainInterpreterData& interp = *(TrainInterpreterData *)_interpreterData;
	    if(interp._onMerged) {
		interp._train = this;
		interp._track = this->position;
		interp._signal = 0;
                interp._scopes.Clear();
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnMerged(%d,%d)"), this->name, this->position->x, this->position->y);
		Trace(expr_buff);
		interp.Execute(interp._onMerged);
		return;
	    }
	}
}


void	Train::OnReverse()
{
	if(_interpreterData) {
	    TrainInterpreterData& interp = *(TrainInterpreterData *)_interpreterData;
	    if(interp._onReverse) {
		interp._train = this;
		interp._track = this->position;
		interp._signal = 0;
                interp._scopes.Clear();
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnReverse(%d,%d)"), this->name, this->position->x, this->position->y);
		Trace(expr_buff);
		interp.Execute(interp._onReverse);
		return;
	    }
	}
}


void	Train::OnShunt()
{
	if(_interpreterData) {
	    TrainInterpreterData& interp = *(TrainInterpreterData *)_interpreterData;
	    if(interp._onShunt) {
		interp._train = this;
		interp._track = this->position;
		interp._signal = 0;
                interp._scopes.Clear();
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnShunt(%d,%d)"), this->name, this->position->x, this->position->y);
		Trace(expr_buff);
		interp.Execute(interp._onShunt);
		return;
	    }
	}
}

///
///
///


bool	TrainInterpreterData::Evaluate(ExprNode *n, ExprValue& result)
{
	ExprValue left(None);
	ExprValue right(None);
	const wxChar	*prop;

	if(!n)
	    return false;
        switch(n->_op) {

	case Dot:
	    
	    result._op = Addr;
	    if(!(n->_left)) {
		result._train = this->_train;		// .<property> ->   this->train
		result._op = TrainRef;
		if(!result._train) {
                    if(trace_script._iValue)
		        wxStrcat(expr_buff, wxT("no current train for '.'"));
		    return false;
		}
		if(result._train->position)
		    TraceCoord(result._train->position->x, result._train->position->y);
	    } else {
		if(!Evaluate(n->_left, result))
		    return false;
	    }
	    result._txt = (n->_right && n->_right->_op == String) ? n->_right->_txt : n->_txt;
	    if(_forAddr)
		return true;

	    prop = result._txt;
	    if(!prop)
		return false;

	    switch(result._op) {
	    
	    case TrainRef:

		if(!result._train)
		    return false;
		return result._train->GetPropertyValue(prop, result);

	    case SwitchRef:

		if(!wxStrcmp(prop, wxT("thrown")) && result._track) {
		    result._op = Number;
		    result._val = result._track->switched;
		    return true;
		}

	    case Addr:
	    case TrackRef:
	    default:

		if(!result._track)
		    return false;
		return result._track->GetPropertyValue(prop, result);

	    case SignalRef:

		if(!result._signal)
		    return false;
		return result._signal->GetPropertyValue(prop, result);

	    }

	default:

	    return InterpreterData::Evaluate(n, result);
	}
	return false;
}
