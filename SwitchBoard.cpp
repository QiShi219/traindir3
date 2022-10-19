/*	TSignal.cpp - Created by Giampiero Caprino

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
#include "wx/ffile.h"

#ifdef WIN32
#define strcasecmp stricmp
#endif

#include "Traindir3.h"
#include "TDFile.h"
#include "Itinerary.h"
#include "SwitchBoard.h"
#include "Server.h"  // for Servlet::json

int	pathIsBusy(Train *tr, Vector *path, int dir);
Vector	*findPath(Track *t, int dir);
Track	*findNextTrack(trkdir direction, int x, int y);
const Char *GetColorName(int color);

extern	void	ShowSwitchboard(void);
extern	bool	file_create(const wxChar *name, const wxChar *ext, wxFFile& fp);

SwitchBoard *switchBoards;

SwitchBoard *curSwitchBoard;	    // TODO: move to a SwitchBoardCell field


SwitchBoardCellAspect::SwitchBoardCellAspect()
{
	_next = 0;
	_name = 0;
	_action = 0;
	_bgcolor = 0;
	memset(_icons, 0, sizeof(_icons));
}


SwitchBoardCellAspect::~SwitchBoardCellAspect()
{
	int	    i;

	for(i = 0; i < MAX_FLASHING_ICONS; ++i) {
	    if(_icons[i])
		free(_icons[i]);
            _icons[i] = 0;
	}
	if(_name)
	    free(_name);
        _name = 0;
	if(_action)
	    free(_action);
        _action = 0;
	if(_bgcolor)
	    free(_bgcolor);
        _bgcolor = 0;
}


SwitchBoardCell::SwitchBoardCell()
{
	_x = _y = 0;
	_aspect = 0;
	_stateProgram = 0;
	_interpreterData = 0;
}



SwitchBoardCell::~SwitchBoardCell()
{
	if(_stateProgram)
	    free(_stateProgram);
        _stateProgram = 0;
	if(_interpreterData)
	    delete _interpreterData;
        _interpreterData = 0;
        _aspect = 0;
}


//	Aspect: locked/free/avail
//	Bgcolor: color/#rrggbb
//	Icons: icon1 [icon2 ...]



void	SwitchBoard::ParseAspect(const wxChar **pp)
{
	wxChar	line[256];
	const wxChar	*p = *pp;
	wxChar	**dst;
	SwitchBoardCellAspect *asp = new SwitchBoardCellAspect();

	p = scan_line(p, line);
	if(line[0])
	    asp->_name = wxStrdup(line);
	do {
	    dst = 0;
	    if(match(&p, wxT("Icons:")))
		dst = &asp->_icons[0];
	    if(dst) {
		p = scan_line(p, line);
		if(line[0]) {
		    if(wxStrchr(line, ' ')) {
//			this->_isFlashing = true;
			int	nxt = 0;
			wxChar	*p1, *pp;

			pp = line;
			do {
			    for(p1 = pp; *pp && *pp != ' '; ++pp);
			    if(p1 != pp) {
				int oc = *pp;
				*pp = 0;
				*dst++ = wxStrdup(p1);
				*pp = oc;
				while(*pp == ' ') ++pp;
				if(++nxt >= MAX_FLASHING_ICONS)
				    break;
			    }
			} while(*pp);
		    } else
			*dst = wxStrdup(line);
		}
		continue;
	    }
	    if(match(&p, wxT("Action:"))) {
		p = scan_line(p, line);
		if(!line[0])
		    continue;
		if(asp->_action)
		    free(asp->_action);
		asp->_action = wxStrdup(line);
		continue;
	    }
	    if(match(&p, wxT("Bgcolor:"))) {
		p = scan_line(p, line);
		if(!line[0])
		    continue;
		if(asp->_bgcolor)
		    free(asp->_bgcolor);
		asp->_bgcolor = wxStrdup(line);
		continue;
	    }
	    break;
	    // unknown. Should we give an error?
	} while(1);
	asp->_next = _aspects;
	_aspects = asp;
	*pp = p;
}


//	Cell: x, y
//	    Itinerary:	name
//	    Text:	string


void	SwitchBoard::ParseCell(const wxChar **pp)
{
	Char	line[256];
	Char	*p1;
	const Char *p = *pp;
	const Char *p2;
	SwitchBoardCell	*cell = 0;

	p = scan_line(p, line);
	int x = wxStrtol(line, &p1, 10);
	if(*p1 == wxT(',')) ++p1;
	int y = wxStrtol(p1, &p1, 10);
	cell = new SwitchBoardCell();
	cell->_x = x;
	cell->_y = y;

	do {
	    p2 = p;
	    p = scan_line(p, line);
	    p1 = line;
	    if(match((const Char **)&p1, wxT("Itinerary:"))) {
		cell->_itinerary = p1;
		continue;
	    }
	    if(match((const Char **)&p1, wxT("Text:"))) {
		cell->_text = p1;
		continue;
	    }
	    p = p2;
	    break;
	} while(*p);
	Add(cell);
	*pp = p;
}



//
//	Execution of the Abstract Syntax Tree
//

extern	wxChar	expr_buff[EXPR_BUFF_SIZE];

extern	void	Trace(const wxChar *msg);


bool	SwitchBoardInterpreterData::Evaluate(ExprNode *n, ExprValue& result)
{
#if 0
	ExprValue left(None);
	ExprValue right(None);
	const wxChar	*prop;

	if(!n)
	    return false;
        switch(n->_op) {

	case NextSignalRef:

	    sig = GetNextSignal(_signal);
	    if(!sig)
		return false;
	    result._op = SignalRef;
	    result._signal = sig;
	    return true;

	case NextApproachRef:

	    if(!_signal->GetApproach(result))
		return false;
	    result._op = SignalRef;
	    return true;

	case Dot:
	    
	    result._op = Addr;
	    if(!(n->_left)) {
		result._signal = this->_signal;		// .<property> ->   this->signal
		result._op = SignalRef;
		if(!result._signal) {
		    wxStrcat(expr_buff, wxT("no current signal for '.'"));
		    return false;
		}
		TraceCoord(result._signal->x, result._signal->y);
	    } else if(n->_left && n->_left->_op == Dot) {
		bool oldForAddr = _forAddr;
		_forAddr = true;
		if(!Evaluate(n->_left, result)) {	// <signal>.<property>
		    _forAddr = oldForAddr;
		    return false;
		}
		_forAddr = oldForAddr;

		if(result._op != SignalRef)
		    return false;
/*		result._signal = GetNextSignal(result._signal);
		if(!result._signal) {
		    wxStrcat(expr_buff, wxT("no current signal for '.'"));
		    return false;
		} */
		TraceCoord(result._signal->x, result._signal->y);
	    } else {
		if(!Evaluate(n->_left, result))
		    return false;
	    }
	    if(n->_right) {
		switch(n->_right->_op) {
		case SignalRef:
		case NextSignalRef:
		    result._signal = GetNextSignal(result._signal);
		    if(!result._signal) {
			wxStrcat(expr_buff, wxT("no current signal for '.'"));
			return false;
		    }
		    TraceCoord(result._signal->x, result._signal->y);
		    break;

		case NextApproachRef:
		    if(!result._signal->GetApproach(result))
			return false;
		    result._op = SignalRef;
		    break;
		}
	    }
	    result._txt = (n->_right && n->_right->_txt) ? n->_right->_txt : n->_txt;
	    if(_forAddr)
		return true;

	    prop = result._txt;
	    if(!prop)
		return false;

	    switch(result._op) {
	    
	    case SwitchRef:

		if(!wxStrcmp(prop, wxT("thrown"))) {
		    result._op = Number;
		    if(!result._track || result._track->type != SWITCH)
			result._val = 0;
		    else
			result._val = result._track->switched;
		    return true;
		}

	    case Addr:
	    case TrackRef:

		if(!result._track)
		    return false;
		return result._track->GetPropertyValue(prop, result);

	    case SignalRef:

		if(!result._signal)
		    return false;
		return result._signal->GetPropertyValue(prop, result);

	    case TrainRef:

		if(!result._train)
		    return false;
		return result._train->GetPropertyValue(prop, result);

	    }
	    return false;

	default:

	    return InterpreterData::Evaluate(n, result);
	}
#endif
	return false;
}


void	*SwitchBoardCell::FindIcon()
{
#if 0
	SwitchBoardInterpreterData *interp = (SwitchBoardInterpreterData *)_interpreterData;
	SwitchBoardCellAspect	*asp = interp->_aspects;
	wxChar		**p = 0;
	int		ix;
	const wxChar	*curState;

	curState = this->_currentState;

	while(asp) {
	    if(!wxStricmp(asp->_name, curState))
		break;
	    asp = asp->_next;
	}
	if(!asp)
	    return 0;
	p = asp->_icons[0];
	if(!p || !*p)
	    return 0;
	if(_isFlashing) {
	    if(!p[_nextFlashingIcon])
		_nextFlashingIcon = 0;
	    p = &p[_nextFlashingIcon];
	}
	if((ix = get_pixmap_index(*p)) < 0)
	    return 0;
	return pixmaps[ix].pixels;
#else
	return 0;
#endif
}




void	SwitchBoardCell::OnUpdate()
{
	if(!_interpreterData)
	    return;

	SwitchBoardInterpreterData interp((SwitchBoardInterpreterData *)_interpreterData);
	if(!interp._onUpdate)
	    return;

	wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnUpdate(%d,%d)"), _stateProgram, _x, _y);
	Trace(expr_buff);
	interp.Execute(interp._onUpdate);
}

void	SwitchBoardCell::OnInit()
{
	if(!_interpreterData)
	    return;
	SwitchBoardInterpreterData interp((SwitchBoardInterpreterData *)_interpreterData);
	if(!interp._onInit)
	    return;
	wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnInit(%d,%d)"), _stateProgram, _x, _y);
	Trace(expr_buff);
	interp.Execute(interp._onInit);
	return;
}

void	SwitchBoardCell::OnRestart()
{
	if(!_interpreterData)
	    return;
	SwitchBoardInterpreterData interp((SwitchBoardInterpreterData *)_interpreterData);
	if(!interp._onRestart)
	    return;
	wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnRestart(%d,%d)"), _stateProgram, _x, _y);
	Trace(expr_buff);
	interp.Execute(interp._onRestart);
	return;
}

void	SwitchBoardCell::OnClicked()
{
	if(!_interpreterData)
	    return;

	SwitchBoardInterpreterData interp((SwitchBoardInterpreterData *)_interpreterData);
	if(!interp._onClick)
	    return;

	wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnClicked(%d,%d)"), _stateProgram, _x, _y);
	Trace(expr_buff);
	interp.Execute(interp._onClick);
}


void	SwitchBoardCell::OnFlash()
{
#if 0
	SwitchBoardAspect *asp;

	if(!_interpreterData)
	    return;
	if(!_currentState)
	    return;

	SwitchBoardInterpreterData interp((SwitchBoardInterpreterData *)_interpreterData);
	wxChar	    **p;

	for(asp = interp._aspects; asp; asp = asp->_next)
	    if(!wxStrcmp(_currentState, asp->_name)) {
		int	nxt = _nextFlashingIcon + 1;

		if(nxt >= MAX_FLASHING_ICONS)
		    nxt = 0;
		p = asp->_icons;
		if(!p || ! p[nxt])
		    nxt = 0;
		_nextFlashingIcon = nxt;
//    		change_coord(this->x, this->y);
		break;
	    }
#endif
}


void	SwitchBoardCell::SetAspect(const wxChar *aspect)
{
	if(!_aspect || wxStrcmp(_aspect, aspect)) {
//    	    change_coord(this->x, this->y);
//	    this->aspect_changed = 1;
	}

	if(_aspect)
	    free(_aspect);
	_aspect = wxStrdup(aspect);
//	_nextFlashingIcon = 0;	    // in case new aspect is not flashing
}


const wxChar *SwitchBoardCell::GetAspect()
{
	if(_aspect)
	    return _aspect;
	return wxT("blank");
}

const wxChar	*SwitchBoardCell::GetAction()
{
#if 0
	const wxChar	*name = GetAspect();
	SwitchBoardInterpreterData *interp = (SwitchBoardInterpreterData *)_interpreterData;
	SwitchBoardCellAspect *asp;

	if(!interp) {
	    return wxT("none");
	}
	for(asp = interp->_aspects; asp; asp = asp->_next) {
	    if(!wxStrcmp(name, asp->_name) && asp->_action)
		return asp->_action;
	}
#endif
	return wxT("none");
}

bool	SwitchBoardCell::toHTML(TDString& str, Char *urlBase)
{
	Itinerary   *it = find_itinerary(_itinerary);
	Char	buff[256];

	if(!it) {
	    str.Append(wxT("<td class=\"empty\">"));
	    str.Append(_text);
	    str.Append(wxT("</td>\n"));
	    return false;
	}

	str.Append(wxT("<td class=\""));
	if(it->CanSelect())
	    str.Append(wxT("available"));
	else if(it->IsSelected())
	    str.Append(wxT("selected"));
	else
	    str.Append(wxT("locked"));
	str.Append(wxT("\"><a href=\""));
	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), L("%s/%d/%d\">"), urlBase, this->_x, this->_y);
	str.Append(buff);
	str.Append(this->_text);
	str.Append(wxT("</a></td>\n"));
	return true;
}


int	SwitchBoard::GetNAspects() const
{
	int n = 0;
	SwitchBoardCellAspect *asp;

	for(asp = _aspects; asp; asp = asp->_next)
	    ++n;
	return n;
}

SwitchBoardCellAspect *SwitchBoard::GetAspect(const Char *name)
{
	int n = 0;
	SwitchBoardCellAspect *asp;

	for(asp = _aspects; asp; asp = asp->_next) {
	    if(!wxStrcmp(name, asp->_name))
		return asp;
	    ++n;
	}
	return 0;
}

bool	SwitchBoardCell::GetPropertyValue(const Char *prop, ExprValue& result)
{
#if 0
	bool	res;
	Vector	*path;
	int	i;

	Signal	*s = this;
	wxStrncat(expr_buff, prop, sizeof(expr_buff)-1);

	if(!wxStrcmp(prop, wxT("aspect"))) {
	    result._op = String;
	    result._txt = s->GetAspect();
	    wxSnprintf(expr_buff + wxStrlen(expr_buff),
		sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%s}"), result._txt);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("auto"))) {
	    result._op = Number;
	    result._val = s->fleeted;
	    wxSnprintf(expr_buff + wxStrlen(expr_buff),
		sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%d}"), result._val);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("enabled"))) {
	    result._op = Number;
	    result._val = s->fleeted && s->nowfleeted;
	    wxSnprintf(expr_buff + wxStrlen(expr_buff),
		sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%d}"), result._val);
	    return true;
	}

	result._op = Number;
	result._val = 0;
	if(!wxStrcmp(prop, wxT("switchThrown"))) {
	    res = s->GetNextPath(&path);
	    if(!path)
		return res;

	    for(i = 0; i < path->_size; ++i) {
		Track	*trk = path->TrackAt(i);

		if(trk->type != SWITCH)
		    continue;
		switch(trk->direction) {
		case 10:	// these are Y switches, which are always
		case 11:	// considered as going to the main line,
		case 22:	// thus ignored as far as signals are concerned.
		case 23:
		    continue;
		}
		if(trk->switched) {
		    result._val = 1;
		    break;
		}
	    }
	    wxSnprintf(expr_buff + wxStrlen(expr_buff),
		sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff),
		wxT("{%s}"), result._val ? wxT("switchThrown") : wxT("switchNotThrown"));
	    Vector_delete(path);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("nextLimit"))) {
	    res = s->GetNextPath(&path);
	    if(!path)
		return res;

	    int	    j;
	    int	    lowSpeed = 1000;
	    
	    for(i = 0; i < path->_size; ++i) {
		Track	*trk = path->TrackAt(i);

		for(j = 0; j < NTTYPES; ++j)
		    if(trk->speed[j] && trk->speed[j] < lowSpeed)
			lowSpeed = trk->speed[j];
	    }
	    result._val = lowSpeed;
	    Vector_delete(path);
	    wxSnprintf(expr_buff + wxStrlen(expr_buff),
		sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%d}"), lowSpeed);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("nextLength"))) {
	    res = s->GetNextPath(&path);
	    if(!path)
		return res;

	    int	    length = 0;
	    
	    for(i = 0; i < path->_size; ++i) {
		Track	*trk = path->TrackAt(i);
		length += trk->length;
	    }
	    result._val = length;
	    Vector_delete(path);
	    wxSnprintf(expr_buff + wxStrlen(expr_buff),
		sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%d}"), length);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("nextApproach"))) {
	    return GetApproach(result);
	}
	if(!wxStrcmp(prop, wxT("nextIsApproach"))) {
	    res = GetApproach(result);
	    result._op = Number;
	    result._val = res == true;
	    return true;
	}
	if(!wxStrcmp(prop, wxT("nextStation"))) {
	    result._op = String;
	    result._txt = wxT("");

	    res = s->GetNextPath(&path);
	    if(!path)
		return res;

	    for(i = 0; i < path->_size; ++i) {
		Track	*trk = path->TrackAt(i);

		if(!trk->isstation)
		    continue;
		result._txt = trk->station;
		break;
	    }
	    Vector_delete(path);
	    wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%s}"), result._txt);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("color"))) {
	    result._op = String;
	    result._txt = wxT("");
	    if(s->controls)
		result._txt = GetColorName(s->controls->fgcolor);
	    return true;
	}
#endif
	return false;
}


bool	SwitchBoardCell::SetPropertyValue(const Char *prop, ExprValue& val)
{
#if 0
	if(!wxStrcmp(prop, wxT("aspect"))) {
	    s->SetAspect(val._txt);
	} else if(!wxStrcmp(prop, wxT("click"))) {
	} else
	    return false;

#endif
	return true;
}




SwitchBoard::SwitchBoard()
{
	_cells = 0;
	_nCells = 0;
	_aspects = 0;
}


SwitchBoard::~SwitchBoard()
{
	while(_aspects) {
	    SwitchBoardCellAspect *asp = _aspects;
	    _aspects = asp->_next;
	    delete asp;
	}
	while(_cells) {
	    SwitchBoardCell *cell = _cells;
	    _cells = cell->_next;
	    delete cell;
	}
}


bool	SwitchBoard::Load(const wxChar *fname)
{
	const Char	*p;
        Char    buff[256];
        wxStrcpy(buff, fname);
        wxStrcat(buff, wxT(".swb"));
	Script	*s = new Script();
	s->_next = 0;
	s->_path = wxStrdup(buff);
	s->_text = 0;
	if(!s->ReadFile()) {
	    free(s->_path);
	    s->_path = 0;
	    delete s;
	    return false;
	}

	_fname = fname;

	p = s->_text;
	while(*p) {
	    const Char	*p1 = p;
	    while(*p1 == ' ' || *p1 == '\t' || *p1 == '\r' || *p1 == '\n')
		++p1;
	    p = p1;
	    if(match(&p, wxT("Aspect:"))) {
		p1 = p;
		ParseAspect(&p);
	    } else if(match(&p, wxT("Cell:"))) {
		p1 = p + 5;
		ParseCell(&p);
            } else if(match(&p, wxT("Name:"))) {
                while(*p == ' ' || *p == '\t') ++p;
                p1 = p;
                if(*p) {
                    int i = 0;
                    while(*p && *p != '\r' && *p != '\n')
                        buff[i++] = *p++;
                    buff[i] = 0;
                    this->_name = buff;
                    if(!*p++)
                        break;
                }
	    }
	    if(p1 == p)	    // error! couldn't parse token
		break;
	}
	free(s->_path);
	s->_path = 0;
	delete s;
	return true;
}


#define	MAX_SWBD_X  40
#define	MAX_SWBD_Y  40

bool	SwitchBoard::toHTML(TDString& str, Char *urlBase)
{
	int	xMax = 0;
	int	yMax = 0;
	int	x, y;
	SwitchBoardCell *cell;
	SwitchBoardCell *grid[MAX_SWBD_X][MAX_SWBD_Y];

	memset(grid, 0, sizeof(grid));
	for(cell = _cells; cell; cell = cell->_next) {
	    if(cell->_x < MAX_SWBD_X && cell->_y < MAX_SWBD_Y) {
		grid[cell->_x][cell->_y] = cell;
		if(cell->_x + 1 > xMax)
		    xMax = cell->_x + 1;
		if(cell->_y + 1 > yMax)
		    yMax = cell->_y + 1;
	    }
	}
	if(!xMax || !yMax)
	    return false;

	curSwitchBoard = this;	    // TODO: remove
	++xMax;
	++yMax;
	str.Append(wxT("<table class=\"switchboard\">\n"));
	for(y = 0; y < yMax; ++y) {
	    str.Append(wxT("<tr>\n"));
	    for(x = 0; x < xMax; ++x) {
		cell = grid[x][y];
		if(!cell)
		    str.Append(wxT("<td class=\"empty\">&nbsp;</td>\n"));
		else
		    cell->toHTML(str, urlBase);
	    }
	    str.Append(wxT("</tr>\n"));
	}
	str.Append(wxT("</table>\n"));
	return true;
}

void    SwitchBoard::toYAML(TDString& out)
{
        Char    buff[512];

        wxSprintf(buff, wxT("- name: %s\n"), _name.c_str());
        out.append(buff);
        SwitchBoardCell *cell;
        for(cell = _cells; cell; cell = cell->_next) {
            wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]),
                wxT("  - cell:\n    x: %d\n    y: %d\n    itinerary: %s\n    text: %s\n    aspect: %s\n"),
                cell->_x, cell->_y, cell->_itinerary.c_str(), cell->_text.c_str(), cell->_aspect);
            out.append(buff);
        }
        SwitchBoardCellAspect *aspect;
        for(aspect = _aspects; aspect; aspect = aspect->_next) {
            wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]),
                wxT("  - aspect:\n    name: %s\n    action: %s\n    bgcolor: %s\n"),
                aspect->_name, aspect->_action, aspect->_bgcolor);
            out.append(buff);
        }
}

void    SwitchBoard::toJSON(TDString& out)
{
        Char    *sep = wxT("{");

        out.Append(wxT("{"));
        Servlet::json(out, wxT("fname"), _fname.c_str(), false);
        Servlet::json(out, wxT("name"), _name.c_str(), false);
        out.Append(wxT("\n\"cells\": ["));
        SwitchBoardCell *cell;
        for(cell = _cells; cell; cell = cell->_next) {
            out.Append(sep);
            Servlet::json(out, wxT("x"), cell->_x, false);
            Servlet::json(out, wxT("y"), cell->_y, false);
            Servlet::json(out, wxT("itinerary"), cell->_itinerary.c_str(), false);
            Servlet::json(out, wxT("text"), cell->_text.c_str(), false);

            // TODO: move to common code (see toHTML)
            Itinerary   *it = find_itinerary(cell->_itinerary.c_str());
            Char    *cl = wxT("locked");
	    if(!it)
                cl = wxT("empty");
	    else if(it->CanSelect())
	        cl = wxT("available");
	    else if(it->IsSelected())
	        cl = wxT("selected");
            Servlet::json(out, wxT("class"), cl, false);
            Servlet::json(out, wxT("aspect"), cell->_aspect, true);
            sep = wxT("},\n{");
        }
        if(_cells)
            out.Append(wxT("\n}\n"));
        out.Append(wxT("],\n\"aspects\": [\n"));
        SwitchBoardCellAspect *aspect;
        sep = wxT("{");
        for(aspect = _aspects; aspect; aspect = aspect->_next) {
            out.Append(sep);
            Servlet::json(out, wxT("name"), aspect->_name, false);
            Servlet::json(out, wxT("action"), aspect->_action, false);
            Servlet::json(out, wxT("bgcolor"), aspect->_bgcolor, true);
            sep = wxT("},\n{");
        }
        if(_aspects)
            out.Append(wxT("\n}\n"));
        out.Append(wxT("]\n}\n"));
}

SwitchBoardCell *SwitchBoard::Find(int x, int y)
{
	SwitchBoardCell *cell;

	for(cell = _cells; cell; cell = cell->_next)
	    if(cell->_x == x && cell->_y == y)
		return cell;
	return 0;
}


void	SwitchBoard::Add(SwitchBoardCell *cell)
{
	SwitchBoardCell *c;

	// make sure it's not already there,
	// to prevend loops in the list
	for(c = _cells; c; c = c->_next)
	    if(c == cell)
		return;
	cell->_next = _cells;
	_cells = cell;
}


void	SwitchBoard::Remove(SwitchBoardCell *cell)
{
	SwitchBoardCell *c, *old = 0;

	for(c = _cells; c && c != cell; old = c, c = c->_next);
	if(!c)		    // not in list - impossible
	    return;
	if(old)
	    old->_next = cell->_next;
	else
	    _cells = cell->_next;
	cell->_next = 0;
}


bool	SwitchBoard::Select(int x, int y)
{
	SwitchBoardCell *cell = Find(x, y);

	if(!cell)
	    return false;
	Itinerary *it = find_itinerary(cell->_itinerary);
	if(!it)
	    return false;
	if(it->CanSelect())
	    it->Select(false);
	else if(it->IsSelected())
	    it->Deselect(false);
	return true;
}


bool	SwitchBoard::Select(const Char *itinName)
{
	SwitchBoardCell *cell;

	for(cell = _cells; cell; cell = cell->_next)
            if(cell->_text.CompareTo(itinName) == 0)
		break;
	if(!cell)
	    return false;

	Itinerary *it = find_itinerary(cell->_itinerary);
	if(!it)
	    return false;
	if(it->CanSelect())
	    it->Select(false);
	else if(it->IsSelected())
	    it->Deselect(false);
	return true;
}



void	SwitchBoard::Change(int x, int y, Char *name, Char *itinName)
{
	SwitchBoardCell *cell = Find(x, y);

	if(!cell) {
	    cell = new SwitchBoardCell();
	    cell->_x = x;
	    cell->_y = y;
	}
	cell->_text = name;
	cell->_itinerary = itinName;
}


//
//
//
//




SwitchBoard *FindSwitchBoard(const wxChar *name)
{
	SwitchBoard *sb;

	for(sb = switchBoards; sb; sb = sb->_next) {
	    if(!wxStrcmp(name, sb->_fname))
		return sb;
	}
	return 0;
}


SwitchBoard *CreateSwitchBoard(const wxChar *name)
{
	SwitchBoard *sb = FindSwitchBoard(name);
	RemoveSwitchBoard(sb);
	sb = new SwitchBoard();
	sb->_name = name;
	sb->_fname = name;
	sb->_next = switchBoards;
	switchBoards = sb;
	return sb;
}


void	RemoveSwitchBoard(SwitchBoard *sb)
{
	SwitchBoard *old = 0;
	SwitchBoard *s;

	for(s = switchBoards; s && s != sb; s = s->_next)
	    old = s;
	if(s) {
	    if(!old)
		switchBoards = s->_next;
	    else
		old->_next = s->_next;
	}
	if(sb)
	    delete sb;
}


void	RemoveAllSwitchBoards()
{
	SwitchBoard *sb;

	while(switchBoards) {
	    sb = switchBoards;
	    switchBoards = sb->_next;
	    delete sb;
	}
        curSwitchBoard = 0;
}


void	SaveSwitchBoards(wxFFile& file)
{
	SwitchBoard *sb;

	for(sb = switchBoards; sb; sb = sb->_next) {
	    file.Write(wxString::Format(wxT("(switchboard %s)\n"), sb->_fname.c_str()));
	    wxFFile file;
	    if(!file_create(sb->_fname, wxT(".swb"), file))
		break;
	    SwitchBoardCellAspect *asp;
	    for(asp = sb->_aspects; asp; asp = asp->_next) {
		file.Write(wxString::Format(wxT("Aspect: %s\n"), asp->_name));
		file.Write(wxString::Format(wxT("Bgcolor: %s\n\n"), asp->_bgcolor));
	    }
	    SwitchBoardCell *cell;
            file.Write(wxString::Format(wxT("Name: %s\n"), sb->_name.c_str()));
	    for(cell = sb->_cells; cell; cell = cell->_next) {
		file.Write(wxString::Format(wxT("Cell: %d,%d\n"), cell->_x, cell->_y));
		file.Write(wxString::Format(wxT("Itinerary: %s\n"), cell->_itinerary.c_str()));
		file.Write(wxString::Format(wxT("Text: %s\n\n"), cell->_text.c_str()));
	    }
	    file.Close();
	}
}

extern	void	switchboard_name_dialog(const wxChar *name);
extern	void	switchboard_cell_dialog(int x, int y);

#define	MAX_NAME_LEN	256

void	SwitchboardEditCommand(const Char *cmd)
{
	SwitchBoard *sb;
	Char	buff[MAX_NAME_LEN];
	int	i;

	while(*cmd == wxT(' '))
	    ++cmd;
	if(!*cmd) {
	    switchboard_name_dialog(0);
	    ShowSwitchboard();
	    return;
	}
	if(*cmd == wxT('-') && cmd[1] == wxT('a')) {
	    cmd += 2;
	    while(*cmd == wxT(' '))
		++cmd;
	    for(i = 0; i < MAX_NAME_LEN - 1 && *cmd && *cmd != wxT(' '); ++i)
		buff[i] = *cmd++;
	    buff[i] = 0;
	    sb = FindSwitchBoard(buff);
	    if(!sb)
		curSwitchBoard = CreateSwitchBoard(buff);
	    else
		curSwitchBoard = sb;
	    while(*cmd == wxT(' '))
		++cmd;
            if(*cmd)
	        curSwitchBoard->_name = cmd;
	    ShowSwitchboard();
	    return;
	}
	if(*cmd == wxT('-') && cmd[1] == wxT('e')) {
	    cmd += 2;
	    while(*cmd == wxT(' '))
		++cmd;
	    sb = FindSwitchBoard(cmd);
	    if(!sb)		// not there - nothing to do
		return;
	    curSwitchBoard = sb;
	    switchboard_name_dialog(cmd);
	    ShowSwitchboard();
	    return;
	}
	if(*cmd == wxT('-') && cmd[1] == wxT('d')) {
	    cmd += 2;
	    while(*cmd == wxT(' '))
		++cmd;
	    sb = FindSwitchBoard(cmd);
	    if(!sb)		// not there - nothing to do
		return;
	    RemoveSwitchBoard(sb);
	    curSwitchBoard = switchBoards;
	    ShowSwitchboard();
	    return;
	}
	sb = FindSwitchBoard(cmd);
	if(!sb)		// not there - nothing to do
	    return;
	curSwitchBoard = sb;
	ShowSwitchboard();
}

void	SwitchboardCellCommand(const Char *cmd)
{
	SwitchBoard *sb = curSwitchBoard;
	int	x, y;

	if(!sb)			// impossible
	    return;

	Char *p;

	while(*cmd == wxT(' '))
	    ++cmd;
	x = wxStrtol(cmd, &p, 10);
	if(*p == wxT(','))
	    ++p;
	y = wxStrtol(p, &p, 10);
	while(*p == wxT(' '))
	    ++p;
	if(!*p) {
	    switchboard_cell_dialog(x, y);
	    ShowSwitchboard();
	    return;
	}
	// label, itinName
}
