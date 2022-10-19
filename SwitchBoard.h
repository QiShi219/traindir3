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

#ifndef _SWITCHBOARD_H
#define _SWITCHBOARD_H

#include "tdscript.h"
#include "Track.h"

class SwitchBoardCell;

class SwitchBoardCellAspect {
public:
	SwitchBoardCellAspect();
	~SwitchBoardCellAspect();

	SwitchBoardCellAspect *_next;
	wxChar	    *_name;		// aspect name
	wxChar	    *_icons[4];		// image to show in cell if any
	wxChar	    *_action;		// URL of action to perform when clicked
	wxChar	    *_bgcolor;		// background color for this aspect
};


class SwitchBoardInterpreterData : public InterpreterData {
public:
	SwitchBoardInterpreterData()
	{
	    _onClick = 0;
	    _onCleared = 0;
	    _onUpdate = 0;
	    _onInit = 0;
            _onRestart = 0;
	}

	SwitchBoardInterpreterData(SwitchBoardInterpreterData *base)
	{
	    _onClick = base->_onClick;
	    _onCleared = base->_onCleared;
	    _onUpdate = base->_onUpdate;
	    _onInit = base->_onInit;
	    _onRestart = base->_onRestart;

	    _signal = 0;
	    _track = 0;
	    _train = 0;
            _scopes.Clear();
	}

	~SwitchBoardInterpreterData()
	{
	}

	void	Free()
	{
	    if(_onAuto)
		delete _onAuto;
	    if(_onCleared)
		delete _onCleared;
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

	Statement *_onClick;	// list of actions (statements)
	Statement *_onCleared;	// list of actions (statements)
	Statement *_onShunt;	// list of actions (statements)
	Statement *_onUpdate;	// list of actions (statements)
	Statement *_onInit;	// list of actions (statements)
	Statement *_onRestart;	// list of actions (statements)
	Statement *_onCross;	// list of actions (statements)
	Statement *_onAuto;	// list of actions (statements)

	bool	Evaluate(ExprNode *expr, ExprValue& result);
};



class SwitchBoardCell {
public:

	SwitchBoardCell();
	~SwitchBoardCell();

	static void InitPixmaps();

	static void FreePixmaps();

	bool	toHTML(wxString& str, wxChar *urlBase);

	void	Draw();

	void	OnUpdate();	// some other signal change, see if we need to change, too

	void	OnInit();	// initial setting (when load)

	void	OnRestart();	// initial setting (when restart)

	void	OnFlash();	// display next flashing aspect

	void	OnClicked();	// for shunting signals

	void	ParseProgram();	// fill Track::interpreterData

	void	FreeProgram();	// release Track::interpreterData


	void	*FindIcon();	// find icon for current aspect and direction

	void	SetAspect(const wxChar *p); // called at run-time

	const Char	*GetAspect();	// called at run-time
	const Char	*GetAspect(int index);// called by puzzle
	const Char	*GetAction();	// get action for current aspect

	int	GetNAspects() const;	// called by puzzle

	bool	GetPropertyValue(const Char *prop, ExprValue& result);
	bool	SetPropertyValue(const Char *prop, ExprValue& val);

	bool	GetNextPath(Vector **ppath);

	SwitchBoardCell	*_next;	    // next cell in switchboard

	int	_x, _y;		    // position in SwitchBoard._cells
	TDString _itinerary;	    // linked itinerary
	TDString _text;		    // text to draw, if any

	Char	*_aspect;	    // current aspect
	Char	*_stateProgram;
	void	*_interpreterData;
};


class SwitchBoard {
public:
	SwitchBoard();
	~SwitchBoard();

	void	ParseAspect(const Char **p);
	void	ParseCell(const Char **p);
	bool	Load(const Char *fname);	// load from switchboard file
	bool	Save();				// TODO - when the SwitchBoard editor page is implemented
	void	Add(SwitchBoardCell *cell);
	void	Remove(SwitchBoardCell *cell);
	bool	toHTML(TDString& str, Char *urlBase);	// render this switchboard as HTML
        void    toYAML(TDString& str);
        void    toJSON(TDString& str);

	SwitchBoardCell *Find(int x, int y);	// find cell at x, y, if any

	bool	Select(int x, int y);		// activate cell x,y
        bool	Select(const Char *itinName);   // activate itinerary by name

	void	Change(int x, int y, Char *name, Char *itinName);

	int	GetNAspects() const;
	SwitchBoardCellAspect   *GetAspect(const Char *name);

	TDString	    _name;		// name of this SwitchBoard (usually a station) - for display
	TDString	    _fname;		// name of description file
	SwitchBoardCell	    *_cells;
	int		    _nCells;
	SwitchBoardCellAspect *_aspects;	// list of aspects (states)
	SwitchBoard	    *_next;
};

extern	SwitchBoard *switchBoards;  // array of SwitchBoards

extern	SwitchBoard *FindSwitchBoard(const wxChar *name);
extern	SwitchBoard *CreateSwitchBoard(const wxChar *name);
extern	void	    RemoveSwitchBoard(SwitchBoard *sb);
extern	void	    RemoveAllSwitchBoards();
extern	void	    SaveSwitchBoards(wxFFile& file);

#endif // _SWITCHBOAARD_H
