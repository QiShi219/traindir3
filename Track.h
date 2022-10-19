/*	Track.h - Created by Giampiero Caprino

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

#ifndef _TRACK_H
#define _TRACK_H

#include "tdscript.h"

class Track;

class TrackInterpreterData : public InterpreterData {
public:
	TrackInterpreterData()
	{
	    _onInit = 0;
	    _onRestart = 0;
	    _onSetBusy = 0;
	    _onSetFree = 0;
	    _onEnter = 0;
	    _onExit = 0;
	    _onClicked = 0;
	    _onCanceled = 0;
	    _onCrossed = 0;
	    _onArrived = 0;
	    _onStopped = 0;
            _onIconUpdate = 0;
	}

	~TrackInterpreterData()
	{
	    if(_onInit)
		delete _onInit;
	    if(_onRestart)
		delete _onRestart;
	    if(_onSetBusy)
		delete _onSetBusy;
	    if(_onSetFree)
		delete _onSetFree;
	    if(_onEnter)
		delete _onEnter;
	    if(_onExit)
		delete _onExit;
	    if(_onClicked)
		delete _onClicked;
	    if(_onCanceled)
		delete _onCanceled;
	    if(_onCrossed)
		delete _onCrossed;
	    if(_onArrived)
		delete _onArrived;
	    if(_onStopped)
		delete _onStopped;
            if(_onIconUpdate)
                delete _onIconUpdate;
	};

	Statement *_onInit;	// list of actions (statements)
	Statement *_onRestart;
	Statement *_onSetBusy;
	Statement *_onSetFree;
	Statement *_onEnter;
	Statement *_onExit;
	Statement *_onClicked;
	Statement *_onCanceled;
	Statement *_onCrossed;
	Statement *_onArrived;
	Statement *_onStopped;
        Statement *_onIconUpdate;

	bool	Evaluate(ExprNode *expr, ExprValue& result);
	bool	GetNextPath(Track *, Vector **path);
};



class Track : public TrackBase {
public:

	Track() { };
	virtual ~Track() { };

	void	OnInit();	// initial setting (when load)

	void	OnRestart();	// initial setting (when restart)

	void	OnSetBusy();	// track has become busy

	void	OnSetFree();	// track has become free

	void	OnEnter(Train *trn);	// train entered track

	void	OnExit(Train *trn);	// train left track

	void	OnClicked();		// user clicked on track

	void	OnCanceled();		// user canceled an itinerary

	void	OnCrossed(Train *trn);	// train activated a trigger

	void	OnArrived(Train *trn);	// train activated at this station

	void	OnStopped(Train *trn);	// train activated at this station

        void    OnIconUpdate();         // icon may change aspect due to some other change in the layout

	void	ParseProgram();		// fill TrackInterpreterData

	void	RunScript(const wxChar *script, Train *trn = 0);

	void	FreeProgram();		// release TrackInterpreterData

	bool	GetPropertyValue(const wxChar *prop, ExprValue& result);
	bool	SetPropertyValue(const wxChar *prop, ExprValue& val);

	void	SetColor(grcolor color);
	bool	IsBusy() const;
};

extern  Array<Track *> onIconUpdateListeners;

#endif // _TRACK_H
