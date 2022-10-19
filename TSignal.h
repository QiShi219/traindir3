/*	TSignal.h - Created by Giampiero Caprino

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

#ifndef _TSIGNAL_H
#define _TSIGNAL_H

#include "tdscript.h"
#include "Track.h"

class Signal;

class SignalAspect {
public:
	SignalAspect();
	~SignalAspect();

	SignalAspect *_next;
	wxChar	    *_name;
	wxChar	    *_iconN[MAX_FLASHING_ICONS],
		    *_iconE[MAX_FLASHING_ICONS],
		    *_iconS[MAX_FLASHING_ICONS],
		    *_iconW[MAX_FLASHING_ICONS];
	wxChar	    *_action;
};


class SignalAction {
public:
	SignalAction();
	~SignalAction();


	SignalAction *_next;
	wxChar	    *_name;
	wxChar	    *_program;
};

extern	SignalAction	*signalActionList;


class SignalInterpreterData : public TrackInterpreterData {
public:
	SignalInterpreterData()
	{
	    _aspects = 0;
	    _onClick = 0;
	    _onCleared = 0;
	    _onUncleared = 0;
	    _onShunt = 0;
	    _onUpdate = 0;
	    _onInit = 0;
	    _onRestart = 0;
	    _onCross = 0;
	    _onAuto = 0;
	    _mustBeClearPath = false;
	}

	SignalInterpreterData(SignalInterpreterData *base)
	{
	    _aspects = base->_aspects;
	    _onClick = base->_onClick;
	    _onCleared = base->_onCleared;
	    _onUncleared = base->_onUncleared;
	    _onShunt = base->_onShunt;
	    _onUpdate = base->_onUpdate;
	    _onInit = base->_onInit;
	    _onRestart = base->_onRestart;
	    _onCross = base->_onCross;
	    _onAuto = base->_onAuto;

	    _signal = 0;
	    _track = 0;
	    _train = 0;
            _scopes.Clear();
	    _mustBeClearPath = false;
	}

	~SignalInterpreterData()
	{
	}

	void	Free()
	{
	    while(_aspects) {
		SignalAspect *asp = _aspects;
		_aspects = asp->_next;
		delete asp;
	    }

	    if(_onAuto)
		delete _onAuto;
	    if(_onCleared)
		delete _onCleared;
	    if(_onUncleared)
		delete _onUncleared;
	    if(_onShunt)
		delete _onShunt;
	    if(_onClick)
		delete _onClick;
	    if(_onCross)
		delete _onCross;
	    if(_onInit)
		delete _onInit;
	    if(_onRestart)
		delete _onRestart;
	    if(_onUpdate)
		delete _onUpdate;
	};

	SignalAspect *_aspects;	// list of aspects (states)
	Statement *_onClick;	// list of actions (statements)
	Statement *_onCleared;	// list of actions (statements)
	Statement *_onUncleared;// list of actions (statements)
	Statement *_onShunt;	// list of actions (statements)
	Statement *_onUpdate;	// list of actions (statements)
	Statement *_onInit;	// list of actions (statements)
	Statement *_onRestart;	// list of actions (statements)
	Statement *_onCross;	// list of actions (statements)
	Statement *_onAuto;	// list of actions (statements)

	bool	Evaluate(ExprNode *expr, ExprValue& result);

	Signal	*GetNextSignal(Signal *sig);
	bool	GetNextPath(Signal *sig, Vector **ppath);
	bool	_mustBeClearPath;
};



class Signal : public Track {
public:

	Signal() { };
	~Signal() { };

	static void InitPixmaps();

	static void FreePixmaps();

	void	Draw();

	bool	IsClear();	// is signal green?

	bool	IsApproach();	// is signal an approach signal?

	bool	IsShuntingSignal(); // is a shunting signal?

	void	OnClear();	// set signal to green

	void	OnUnclear();	// set signal to red

	void	OnShunt();	// set signal to clear for shunting

	void	OnCross();	// set signal to red when a train enters the controlled section

	void	OnUnlock();	// set signal to green after path has become clear

	void	OnUnfleet();	// set fleeted signal to green after path has become clear

	void	OnUpdate();	// some other signal change, see if we need to change, too

	void	OnInit();	// initial setting (when load)

	void	OnRestart();	// initial setting (when restart)

	void	OnFlash();	// display next flashing aspect

	void	OnAuto();	// automatic signal has been enabled/disabled

	void	OnClicked();	// for shunting signals

	void	ParseProgram();	// fill Track::interpreterData

	void	FreeProgram();	// release Track::interpreterData

        Shape   *FindShapeBuiltin();   // find shape for built-in signals (no scripts)
        Shape   *FindShape();   // find shape for current aspect and direction
	void	*FindIcon();	// find icon for current aspect and direction

	void	ParseAspect(const wxChar **p);

	void	SetAspect(const wxChar *p); // called at run-time

	const wxChar	*GetAspect();	// called at run-time

	const wxChar	*GetAspect(int index);// called by puzzle

	const wxChar	*GetAction();	// get action for current aspect

	int	GetNAspects() const;	// called by puzzle

	bool	GetSpeedLimit(int *limit);  // get speed limit associated
					    // with current aspect

	bool	GetPropertyValue(const wxChar *prop, ExprValue& result);
	bool	SetPropertyValue(const wxChar *prop, ExprValue& val);

	Signal	*GetNextSignal();
	bool	GetNextPath(Vector **ppath);
	bool	GetApproach(ExprValue& result);

        bool    ToggleForShunting();

        void    RegisterAspectsIcons();
};

#endif // _TSIGNAL_H
