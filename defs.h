/*	defs.h - Created by Giampiero Caprino

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

#ifndef _DEFS_H
#define _DEFS_H

#define	HGRID	9
#define	VGRID	9

#define	HCOORDBAR   20
#define	VCOORDBAR   30

#define	XNCELLS	1500//440	    // 3.7n - was 226
#define	YNCELLS 285//228	    // 3.7o - was 114

#define	XMAX	((XNCELLS * HGRID) + HCOORDBAR) /* 440*9 */
#define	YMAX	((YNCELLS * VGRID) + VCOORDBAR) /* 1026 */

enum MenuIDs {
    MENU_TIME_SPLIT = 100,
    MENU_TIME_TAB,
    MENU_TIME_FRAME,

    MENU_SHOW_LAYOUT,
    MENU_SHOW_SCHEDULE,
    MENU_INFO_PAGE,

    MENU_ZOOMIN,
    MENU_ZOOMOUT,

    MENU_SHOW_COORD,
    MENU_TOOLBAR,
    MENU_STATUSBAR,
    MENU_COPYRIGHT,
    MENU_LANGUAGE,

    MENU_RECENT,
    MENU_RESTORE,
    MENU_EDIT,
    MENU_NEW_TRAIN,
    MENU_ITINERARY,
    MENU_SWITCHBOARD,
    MENU_SAVE_LAYOUT,
    MENU_PREFERENCES,
    MENU_EDIT_SKIN,
    MENU_NEW_LAYOUT,
    MENU_INFO,
    MENU_STATIONS_LIST,

    MENU_START,
    MENU_GRAPH,
    MENU_LATEGRAPH,
    MENU_PLATFORMGRAPH,
    MENU_RESTART,
    MENU_FAST,
    MENU_SLOW,
    MENU_SKIP,
    MENU_STATION_SCHED,
    MENU_SETGREEN,
    MENU_SELECT_ITIN,
    MENU_PERFORMANCE,

    MENU_ITIN_DELETE,
    MENU_ITIN_PROPERTIES,
    MENU_ITIN_SAVE,

    MENU_ALERT_CLEAR,
    MENU_ALERT_SAVE,
    MENU_ALERT_LAST_ON_TOP,

    MENU_HTML_PRINTSETUP,
    MENU_HTML_PREVIEW,
    MENU_HTML_PRINT,

    MENU_SCHED_SHOW_CANCELED,
    MENU_SCHED_SHOW_ARRIVED,
    MENU_SCHED_ASSIGN,
    MENU_SCHED_STARTNOW,
    MENU_SCHED_TRACK_FIRST,
    MENU_SCHED_TRACK_LAST,
    MENU_SCHED_PRINT_TRAIN,
    MENU_SCHED_SET_DELAY,

    MENU_COORD_DEL_1,
    MENU_COORD_DEL_N,
    MENU_COORD_INS_1,
    MENU_COORD_INS_N,

    ID_RADIOBOX,
    ID_CHECKBOX,
    ID_LIST,
    ID_NOTEBOOK_TOP,
    ID_NOTEBOOK_LEFT,
    ID_NOTEBOOK_RIGHT,

    ID_SPEEDTEXT,
    ID_SPIN,
    ID_RUN,
    ID_ASSIGN,
    ID_SHUNT,
    ID_SPLIT,
    ID_PROPERTIES,
    ID_PRINT,
    ID_ASSIGNSHUNT,
    ID_REVERSEASSIGN,
    ID_SCRIPT,

    ID_CHOICE,

    ID_PATH1, // paths in preferences' environment tab
    ID_PATH2,
    ID_PATH3,

    ID_ITINSELECT,
    ID_ITINCLEAR,
    ID_ITINFORSHUNT,

    TIMER_ID = 1000,

    FIRST_CANVAS = 1100,
    LAST_CANVAS = 1199,

    FIRST_TTABLE = 1200,
    LAST_TTABLE = 1299,

    FIRST_HTML = 1300,
    LAST_HTML = 1399
};

#define	STATE_FILE_NAME "tdir3.ini"

#define NUMTTABLES (LAST_TTABLE - FIRST_TTABLE + 1)
#define NUMCANVASES (LAST_CANVAS - FIRST_CANVAS + 1)
#define NUMHTMLS (LAST_HTML - FIRST_HTML + 1)

typedef unsigned short Pos;		/* reduce memory occupation of Track */

//	Coord
//
//	A location on the canvas.
//	The coords are multiplied by HGRID and VGRID
//	before drawing on the canvas.
//	Conversely, the coords are divided by HGRID and VGRID
//	when converting from canvas coord.

class	Coord {
public:
        Coord() { x = y = 0; };
	Coord(Pos _x, Pos _y) { x = _x; y = _y; };
	~Coord() { };

	Coord& operator=(Coord& other)
	{
	    x = other.x;
	    y = other.y;
	    return *this;
	}

	void Set(Pos _x, Pos _y) { x = _x; y = _y; };

	bool operator==(Coord& other) const
	{
	    return x == other.x && y == other.y;
	}

	bool operator!=(Coord& other) const
	{
	    return x != other.x || y != other.y;
	}

	Pos	x;
	Pos	y;
};

typedef wxChar	Char;
typedef wxString TString;
typedef wxString TDString;

const	Char	*localize(const Char *string);
#define	L(s)	localize(wxT(s))
#define	LV(s)	localize(s)

void	localizeArray(const Char *localized[], const Char *english[]);
void	freeLocalizedArray(const Char *localized[]);


#endif // _DEFS_H
