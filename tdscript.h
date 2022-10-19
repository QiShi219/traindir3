/*	tdscript.h - Created by Giampiero Caprino

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

#ifndef _TDSCRIPT_H
#define _TDSCRIPT_H

#include "trsim.h"

//	scripting support

enum NodeOp {
	Equal,			// bool if left == right OR assignment
	NotEqual,		// bool if left != right
	Less,			// bool if left < right
	Greater,		// bool if left > right
	LessEqual,		// bool if left <= right
	GreaterEqual,		// bool if left >= right
	And,			// bool if left && right
	Or,			// bool if left || right
	Dot,			// result is left.value
	TrackRef,		// Track(x, y)   OR  Track(value)
	SwitchRef,		// Switch(x, y)  OR  Switch(value)
	SignalRef,		// Signal(x, y)  OR  Signal(value)
	NextSignalRef,		// Signal,
	NextApproachRef,	// Signal,
        LinkedRef,              // Image to Switch
	TrainRef,
	Addr,			// Ref + Dot
	Random,			// return 0..100
	Time_,			// current time, in decimal hhmm
        Day,                    // day being simulated (1=mon,7=sun)
	None,
	Bool,
	Number,
	String
};

class ExprValue {
public:
	ExprValue(NodeOp op)
	{
	    _op = op;
	    _txt = 0;
	    _val = 0;
	    _track = 0;
	    _signal = 0;
	    _train = 0;
	};

	~ExprValue() { };

	NodeOp	    _op;
	Track	    *_track;
	Signal	    *_signal;
	Train	    *_train;
	const wxChar  *_txt;
	int	    _val;
};

class ExprNode {
public:
        ExprNode(NodeOp op)
	{
	    _op = op;
	    _left = _right = 0;
	    _val = 0;
	    _txt = 0;
	    _x = _y = 0;
	}

	~ExprNode();

	NodeOp	_op;
	ExprNode *_left, *_right;
	wxChar	*_txt;		// value for aspects compares
	int	_val;
	int	_x, _y;		// coordinates of TrackRef, SwitchRef, SignalRef
};


class Statement {
public:
	Statement()
	{
	    _next = _child = _lastChild = _parent = 0;
	    _elseChild = _lastElseChild = 0;
	    _isElse = false;
	    _type = 0;
	    _text = 0;	    // todo: remove?
	    _expr = 0;
	}
	
	~Statement()
	{
	    while(_elseChild) {
		Statement *stmt = _elseChild;
		_elseChild = _elseChild->_next;
		delete stmt;
	    }
	    _lastElseChild = 0;
	    while(_child) {
		Statement *stmt = _child;
		_child = _child->_next;
		delete stmt;
	    }
	    _lastChild = 0;
	    if(_expr)
		delete _expr;
	    if(_text)
		free(_text);
	    _text = 0;
	}

	Statement	*_next;
	Statement	*_child, *_elseChild;
	Statement	*_lastChild, *_lastElseChild;
	Statement	*_parent;
	int		_type;
	bool		_isElse;
	wxChar		*_text;
	ExprNode	*_expr;
};

// Track::interpreterData for signals

//#define	MAXNESTING  50

class Signal;
class Itinerary;

class InterpreterData {
public:
	InterpreterData()
	{
	    _forCond = _forAddr = false;
            _scopes.Clear();
	    _track = 0;
	    _train = 0;
	    _signal = 0;
	}
	virtual ~InterpreterData() { }

	virtual void	Execute(Statement *stmt);
	virtual bool	Evaluate(ExprNode *expr, ExprValue& result);

	void		TraceCoord(int x, int y, const wxChar *label = wxT(""));

	bool		_forCond;
	bool		_forAddr;
	Array<Statement	*> _scopes; //[MAXNESTING];

	// run-time environment

	Track		*_track;
	Train		*_train;
	Signal		*_signal;
	Itinerary	*_itinerary;
};


#define	EXPR_BUFF_SIZE	(4096*4)
extern	wxChar	expr_buff[EXPR_BUFF_SIZE];

class   TDProc {
public:
        TDProc();
        ~TDProc();

        TDProc  *_next;
        Char    *_name;
        Statement *_body;
};

extern  TDProc  *procList;

TDProc  *find_proc(const Char *name);
TDProc  *add_proc(const Char *name);
const Char *parse_proc(const Char *p);
void    free_procs();

struct	Script {
	Script	*_next;
	wxChar	*_path;
	wxChar	*_text;

	bool	ReadFile();
};

extern	Script	*scriptList;

void	free_scripts();
Script	*find_script(const wxChar *path);
Script	*new_script(const wxChar *path);
void	load_scripts(Track *layout);

bool	    match(const wxChar **pp, const wxChar *txt);
const wxChar	*scan_line(const wxChar *src, wxChar *dst);
const wxChar	*scan_word(const wxChar *p, wxChar *dst);
const wxChar	*next_token(const wxChar *p);
Statement   *ParseStatements(const wxChar **pp);
void	    Trace(const wxChar *msg);


#endif // _TDSCRIPT_H
