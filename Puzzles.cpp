/*	trsim.cpp - Created by Giampiero Caprino

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
#endif

#include <stdlib.h>
#include "wx/ffile.h"
#include "Html.h"
#include "Traindir3.h"
#include "Itinerary.h"
#include "TDFile.h"
#include "MainFrm.h"

extern	int toggle_signal(Signal *s);

void	show_puzzle();
void	set_zoom(bool zooming);

#define	MAXPUZZLESIGNALS    20
#define	MAXPUZZLESWITCHES   20
#define	MAXPUZZLESPEEDS	    20
#define	MAXPUZZLELENGTHS    20
#define	MAXPUZZLESCRIPTS    100

#define	MAXSPEEDS   NTTYPES

class Speed {
public:
        Speed() { station = 0; };
	~Speed();

	Track	*station;
	int	speeds[MAXSPEEDS];
	int	nSpeeds;
};

class Puzzle {
public:
	Puzzle();
	~Puzzle();

	Signal	*signals[MAXPUZZLESIGNALS];
	int	nSignals;

	Track	*switches[MAXPUZZLESWITCHES];
	int	nSwitches;

	Speed	*speeds[MAXPUZZLESPEEDS];
	int	nSpeeds;

	Speed	*lengths[MAXPUZZLELENGTHS];
	int	nLengths;
};

int	mystery_signal = 0;

Puzzle::Puzzle()
{
	nSignals = 0;
	nSwitches = 0;
	nSpeeds = 0;
	nLengths = 0;
}

Puzzle::~Puzzle()
{
	nSignals = 0;
	nSwitches = 0;
	nSpeeds = 0;
	nLengths = 0;
}



Puzzle	*puzzle;

Signal	*get_signal(wxChar *p)
{
	if(*p >= '0' && *p <= '9') {
	    int x = wxStrtol(p, &p, 10);
	    if(*p == ',') ++p;
	    int y = wxStrtol(p, &p, 10);
	    return findSignal(x, y);
	}
	return findSignalNamed(p);
}

Track	*get_track(wxChar *p)
{
	if(*p >= '0' && *p <= '9') {
	    int x = wxStrtol(p, &p, 10);
	    if(*p == ',') ++p;
	    int y = wxStrtol(p, &p, 10);
	    return findTrack(x, y);
	}
	return findStationNamed(p);
}


void	load_puzzles(const wxChar *cmd)
{
	TDFile	puzzleFile(cmd);
	wxChar	buff[512];
	wxChar	*p;
	int	l;

	puzzleFile.SetExt(wxT(".tdp"));
	if(!puzzleFile.Load()) {
	    wxSnprintf(status_line, sizeof(status_line)/sizeof(wxChar), wxT("%s '%s.tdp'"), L("cannot load"), cmd);
	    traindir->Error(status_line);
	    return;
	}
	if(puzzle) {
	    delete puzzle;
	}
	puzzle = new Puzzle();

	while(puzzleFile.ReadLine(buff, sizeof(buff)/sizeof(buff[0]))) {
	    if(!buff[0] || buff[0] == '#')
		continue;
	    if(buff[0] == '.') {
		continue;
	    }
	    for(l = 0; buff[l]; ++l)
		if(buff[l] == '\t')
		    buff[l] = ' ';
	    while(l && (buff[l - 1] == ' ' || buff[l - 1] == '\t')) --l;
	    buff[l] = 0;
	    if(!wxStrncmp(buff, wxT("Layout: "), 8)) {
		for(p = buff + 8; *p == ' '; ++p);
		if(!(layout = load_field(p))) {
		    wxSnprintf(status_line, sizeof(status_line)/sizeof(wxChar), wxT("%s '%s.trk'"), L("cannot load"), cmd);
		    traindir->Error(status_line);
		    return;
		}
		continue;
	    }
	    if(!wxStrncmp(buff, wxT("Signal: "), 8)) {
		for(p = buff + 8; *p == ' '; ++p);
		Signal *sig = get_signal(p);
		if(puzzle->nSignals < MAXPUZZLESIGNALS) {
		    puzzle->signals[puzzle->nSignals++] = sig;
		}
		continue;
	    }
	    if(!wxStrncmp(buff, wxT("Switch: "), 8)) {
		for(p = buff + 8; *p == ' '; ++p);
		int x, y;
		x = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		y = wxStrtol(p, &p, 10);
		Track *trk = findSwitch(x, y);
		if(puzzle->nSwitches < MAXPUZZLESWITCHES) {
		    puzzle->switches[puzzle->nSwitches++] = trk;
		}
		continue;
	    }
	    if(!wxStrncmp(buff, wxT("Speed: "), 7)) {
		if(puzzle->nSpeeds >= MAXPUZZLESPEEDS)
		    continue;
		for(p = buff + 7; *p == ' '; ++p);
		Speed *speed = new Speed();
		memset(speed->speeds, 0, sizeof(speed->speeds));
		int i = 0;
		while(*p) {
		    speed->speeds[i++] = wxStrtol(p, &p, 10);
		    if(*p != '/' || i >= MAXSPEEDS)
			break;
		    ++p;
		}
		speed->nSpeeds = i;
		for(; *p == ' '; ++p);
		speed->station = get_track(p);
		puzzle->speeds[puzzle->nSpeeds++] = speed;
		continue;
	    }
	    if(!wxStrncmp(buff, wxT("Length: "), 8)) {
		if(puzzle->nLengths >= MAXPUZZLELENGTHS)
		    continue;
		for(p = buff + 8; *p == ' '; ++p);
		Speed *speed = new Speed();
		memset(speed->speeds, 0, sizeof(speed->speeds));
		int i = 0;
		while(*p) {
		    speed->speeds[i++] = wxStrtol(p, &p, 10);
		    if(*p != '/' || i >= MAXSPEEDS)
			break;
		    ++p;
		}
		speed->nSpeeds = i;
		for(; *p == ' '; ++p);
		speed->station = get_track(p);
		puzzle->lengths[puzzle->nLengths++] = speed;
		continue;
	    }
	}
	set_zoom(true);
	//show_puzzle();
}

void	show_puzzle()
{
	int	i, j;
	Signal	*sig;
	Signal	*s;
	int	which = rand() % puzzle->nSignals;

	for(i = 0; i < puzzle->nSignals; ++i) {
	    sig = puzzle->signals[i];
	    sig->invisible = 0;
	}
	sig = puzzle->signals[which];
	sig->invisible = true;
	mystery_signal = which;

	/*
	 *  Randomize lengths
	 */

	for(i = 0; i < puzzle->nLengths; ++i) {
	    which = rand() % puzzle->lengths[i]->nSpeeds;
	    puzzle->lengths[i]->station->length = puzzle->lengths[i]->speeds[which];
	}

	/*
	 *  Randomize speeds
	 */

	for(i = 0; i < puzzle->nSpeeds; ++i) {
	    which = rand() % puzzle->speeds[i]->nSpeeds;
	    puzzle->speeds[i]->station->speed[0] = puzzle->speeds[i]->speeds[which];
	}

	/*
	 *  Randomize switches
	 */

	for(i = 0; i < puzzle->nSwitches; ++i) {
	    which = rand() % 2;
	    puzzle->switches[i]->switched = which;
	}

	/*
	 *  Clear most signals
	 */

	for(i = 0; i < puzzle->nSignals; ++i) {
	    s = puzzle->signals[i];
	    if(!s->IsClear()) {
		toggle_signal(s);
	    }
	}

	/*
	 *  Get hidden signal's aspect
	 *  then select x other aspects
	 *  to be chosen by the user
	 */

	bool found;
	const wxChar *aspect = sig->_currentState;
	const wxChar *aspects[3];
	aspects[0] = aspect;
	int lim = sig->GetNAspects();
	for(i = 1; i < 3; ) {
	    which = rand() % lim;
	    aspect = sig->GetAspect(which);

	    // make sure we have not selected this aspect already
	    found = false;
	    for(j = 1; j < i; ++j) {
		if(!wxStrcmp(aspect, aspects[j])) {
		    found = true;
		    break;
		}
	    }
	    if(found)
		continue;
	    aspects[i++] = aspect;
	}

	/*
	 *  We now have 3 aspects to show the player,
	 *  one of them the valid one. Chose a random
	 *  order to show them to the user.
	 */

	i = 0;
	const wxChar *order[3];
	for(j = 2; j >= 0; --j) {
	    which = j ? (rand() % j) : 0;
	    order[i++] = aspects[which];
	    aspects[which] = aspects[j];
	}

	s = findSignalNamed(wxT("TRY1"));
	s->stateProgram = wxStrdup(sig->stateProgram);
	s->ParseProgram();
	s->SetAspect(order[0]);

	s = findSignalNamed(wxT("TRY2"));
	s->stateProgram = wxStrdup(sig->stateProgram);
	s->ParseProgram();
	s->SetAspect(order[1]);

	s = findSignalNamed(wxT("TRY3"));
	s->stateProgram = wxStrdup(sig->stateProgram);
	s->ParseProgram();
	s->SetAspect(order[2]);

}


void	puzzle_check(Track *t)
{
	Signal *hiddenSignal = puzzle->signals[mystery_signal];
	Signal *sig = 0;
	if(!wxStrcmp(t->station, wxT("B1"))) {
	    sig = findSignalNamed(wxT("TRY1"));
	} else if(!wxStrcmp(t->station, wxT("B2"))) {
	    sig = findSignalNamed(wxT("TRY2"));
	} else if(!wxStrcmp(t->station, wxT("B3"))) {
	    sig = findSignalNamed(wxT("TRY3"));
	} else
	    return;

	if(wxStrcmp(hiddenSignal->_currentState, sig->_currentState)) {
	    do_alert(L("Wrong answer."));
	} else {
	    traindir->AddAlert(L("Correct answer."));
	    hiddenSignal->invisible = 0;
	    repaint_all();
	}
}
