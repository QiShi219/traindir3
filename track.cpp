/*	track.cpp - Created by Giampiero Caprino

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

#include "Traindir3.h"
#include "Itinerary.h"
#include "Array.h"
#include "itracks.h"
#include "itools.h"
#include "isignals.h"
#include "iswitch.h"
#include "iactions.h"
#include "imovers.h"
#include "MotivePower.h"

extern	void	draw_itin_text(int x, int y, const wxChar *txt, int size);

extern	void	delete_script_data(TrackBase *t);

extern	const	Char *GetColorName(int color);
void	setBackgroundColor(wxColour& col);

extern  IntOption       editor_gauge;
extern  Array<TrackBase *>  dwellingTracks;

#define OLD		    /* old way of drawing tracks */

int	terse_status;

extern	int	current_tool;
extern	int	layout_modified;    /* user edited the layout */
extern  const Char    *eventName;   // for script diagnostic
extern  Char    eventObject[];      // for script diagnostic
extern  void    parserError(const Char *msg, const Char *ptr);
Track	*track_walkwest(Track *t, trkdir *ndir);
grcolor	fieldcolors[MAXFIELDCOL];

BoolOption draw_train_names(wxT("TrainNames"), wxT("Show train names instead of icons"), wxT("Preferences"), false);
BoolOption no_train_names_colors(wxT("NoTrainNamesColors"), wxT("Don't use colors when showing train names"), wxT("Preferences"), false);
BoolOption no_train_names_background(wxT("NoTrainNamesBackground"), wxT("Don't draw a box as background for train names"), wxT("Preferences"), false);
BoolOption small_train_names(wxT("SmallTrainNames"), wxT("Use small font for train names"), wxT("Preferences"), true);
int	auto_link = 1;
int	link_to_left = 0;
int	show_links = 1;
int	show_scripts = 0;
Coord	link_start;
Coord	move_start, move_end;
int	current_macro = -1;
wxChar	*current_macro_name;
Track	**macros;
int	nmacros, maxmacros;
Array<Track *> onIconUpdateListeners;

VLines	n_s_layout[] = {
	{ HGRID / 2 + 1, 0, HGRID / 2 + 1, VGRID - 1 },
	{ HGRID / 2 - 0, 0, HGRID / 2 - 0, VGRID - 1 },
	{ -1 }
};
SegDir	n_s_segs[] = { SEG_N, SEG_S, SEG_END };
SegDir	sw_n_segs[] = { SEG_SW, SEG_N, SEG_END };
SegDir	nw_s_segs[] = { SEG_NW, SEG_S, SEG_END };
SegDir	w_e_segs[] = { SEG_W, SEG_E, SEG_END };
SegDir	nw_e_segs[] = { SEG_NW, SEG_E, SEG_END };
SegDir	sw_e_segs[] = { SEG_SW, SEG_E, SEG_END };
SegDir	w_ne_segs[] = { SEG_W, SEG_NE, SEG_END };
SegDir	w_se_segs[] = { SEG_W, SEG_SE, SEG_END };
SegDir	nw_se_segs[] = { SEG_NW, SEG_SE, SEG_END };
SegDir	sw_ne_segs[] = { SEG_SW, SEG_NE, SEG_END };
SegDir	ne_s_segs[] = { SEG_NE, SEG_S, SEG_END };
SegDir	se_n_segs[] = { SEG_SE, SEG_N, SEG_END };
SegDir	itin_segs[] = { SEG_NW, SEG_SW, SEG_NE, SEG_SE, SEG_W, SEG_E, SEG_END };

VLines	sw_n_layout[] = {
	{ HGRID / 2 + 1, 0, HGRID / 2 + 1, VGRID / 2 },
	{ HGRID / 2 - 0, 0, HGRID / 2 - 0, VGRID / 2 },
	{ HGRID / 2 + 1, VGRID / 2, 1, VGRID - 1 },
	{ HGRID / 2 - 0, VGRID / 2, 0, VGRID - 1 },
	{ HGRID / 2 - 0, VGRID / 2 - 1, 0, VGRID - 2 },
	{ -1 }
};

VLines	nw_s_layout[] = {
	{ 1, 0, HGRID / 2 + 1, VGRID / 2 },
	{ 0, 0, HGRID / 2 - 0, VGRID / 2 },
	{ 0, 1, HGRID / 2 - 1, VGRID / 2 },
	{ HGRID / 2 + 1, VGRID / 2 - 0, HGRID / 2 + 1, VGRID - 1 },
	{ HGRID / 2 - 0, VGRID / 2 - 0, HGRID / 2 - 0, VGRID - 1 },
	{ -1 }
};

VLines	se_n_layout[] = {
	{ HGRID / 2 + 1, 0, HGRID / 2 + 1, VGRID / 2 },
	{ HGRID / 2 - 0, 0, HGRID / 2 - 0, VGRID / 2 },
	{ HGRID / 2 + 1, VGRID / 2, HGRID - 1, VGRID - 2 },
	{ HGRID / 2 - 0, VGRID / 2, HGRID - 1, VGRID - 1 },
	{ HGRID / 2 - 0, VGRID / 2 + 1, HGRID - 2, VGRID - 1 },
	{ -1 }
};

VLines	ne_s_layout[] = {
	{ HGRID / 2, VGRID / 2 - 1, HGRID - 2, 0 },
	{ HGRID / 2, VGRID / 2, HGRID - 1, 0 },
	{ HGRID / 2 + 1, VGRID / 2, HGRID - 1, 1 },
	{ HGRID / 2 + 1, VGRID / 2 - 0, HGRID / 2 + 1, VGRID - 1 },
	{ HGRID / 2 - 0, VGRID / 2 - 0, HGRID / 2 - 0, VGRID - 1 },
	{ -1 }
};

VLines	w_e_layout[] = {
	/*{ 0, VGRID / 2 - 1, HGRID - 1, VGRID / 2 - 1 },*/
	{ 0, VGRID / 2 - 0, HGRID - 1, VGRID / 2 - 0 },
	{ 0, VGRID / 2 + 1, HGRID - 1, VGRID / 2 + 1 },
	{ -1 }
};

VLines	nw_e_layout[] = {
	{ 1, 0, HGRID / 2, VGRID / 2 - 1 },
	{ 0, 0, HGRID / 2, VGRID / 2 - 0 },
	{ 0, 1, HGRID / 2, VGRID / 2 + 1 },
	/*{ HGRID / 2, VGRID / 2 - 1, HGRID - 1, VGRID / 2 - 1 },*/
	{ HGRID / 2, VGRID / 2 - 0, HGRID - 1, VGRID / 2 - 0 },
	{ HGRID / 2, VGRID / 2 + 1, HGRID - 1, VGRID / 2 + 1 },
	{ -1 }
};

VLines	sw_e_layout[] = {
	{ 0, VGRID - 2, HGRID / 2 - 1, VGRID / 2 /*- 1*/ },
	{ 0, VGRID - 1, HGRID / 2, VGRID / 2 - 0 },
	{ 1, VGRID - 1, HGRID / 2, VGRID / 2 + 1 },
	/*{ HGRID / 2, VGRID / 2 - 1, HGRID - 1, VGRID / 2 - 1 },*/
	{ HGRID / 2, VGRID / 2 - 0, HGRID - 1, VGRID / 2 - 0 },
	{ HGRID / 2, VGRID / 2 + 1, HGRID - 1, VGRID / 2 + 1 },
	{ -1 }
};

VLines	w_ne_layout[] = {
	/*{ 0, VGRID / 2 - 1, HGRID / 2, VGRID / 2 - 1 },*/
	{ 0, VGRID / 2 - 0, HGRID / 2, VGRID / 2 - 0 },
	{ 0, VGRID / 2 + 1, HGRID / 2, VGRID / 2 + 1 },
	{ HGRID / 2, VGRID / 2 - 1, HGRID - 2, 0 },
	{ HGRID / 2, VGRID / 2 - 0, HGRID - 1, 0 },
	{ HGRID / 2, VGRID / 2 + 1, HGRID - 1, 1 },
	{ -1 }
};

VLines	w_se_layout[] = {
	/*{ 0, VGRID / 2 - 1, HGRID / 2 - 0, VGRID / 2 - 1 },*/
	{ 0, VGRID / 2 - 0, HGRID / 2, VGRID / 2 - 0 },
	{ 0, VGRID / 2 + 1, HGRID / 2, VGRID / 2 + 1 },
	{ HGRID / 2 + 1, VGRID / 2 /*- 1*/, HGRID - 1, VGRID - 2 },
	{ HGRID / 2, VGRID / 2 - 0, HGRID - 1, VGRID - 1 },
	{ HGRID / 2, VGRID / 2 + 1, HGRID - 2, VGRID - 1 },
	{ -1 }
};

VLines	sweng_sw_ne_straight[] = {
	{ 0, VGRID - 2, HGRID - 2, 0 },
	{ 0, VGRID - 1, HGRID - 1, 0 },
	{ 1, VGRID - 1, HGRID - 1, 1 },

	{ 0, VGRID / 2, HGRID / 2 - 1, VGRID / 2 },
	{ 0, VGRID / 2 + 1, HGRID / 2 - 1, VGRID / 2 + 1 },

	{ HGRID / 2 + 1, VGRID / 2 + 1, HGRID - 1, VGRID / 2 + 1 },
	{ HGRID / 2 + 1, VGRID / 2 - 0, HGRID - 1, VGRID / 2 - 0 },
	{ -1 }
};

VLines	sweng_sw_ne_switched[] = {

	{ 0, VGRID / 2, HGRID - 2, 0 },
	{ 0, VGRID / 2 + 1, HGRID - 1, 0 },

	{ 0, VGRID - 1, HGRID - 1, VGRID / 2 },
	{ 1, VGRID - 1, HGRID - 1, VGRID / 2 + 1 },
	{ -1 }
};

VLines	sweng_nw_se_straight[] = {
	{ 1, 0, HGRID - 1, VGRID - 2 },
	{ 0, 0, HGRID - 1, VGRID - 1 },
	{ 0, 1, HGRID - 2, VGRID - 1 },

	{ 0, VGRID / 2, HGRID / 2 - 1, VGRID / 2 },
	{ 0, VGRID / 2 + 1, HGRID / 2 - 1, VGRID / 2 + 1 },

	{ HGRID / 2 + 1, VGRID / 2 + 1, HGRID - 1, VGRID / 2 + 1 },
	{ HGRID / 2 + 1, VGRID / 2 - 0, HGRID - 1, VGRID / 2 - 0 },
	{ -1 }
};

VLines	sweng_nw_se_switched[] = {

	{ 0, 0, HGRID - 1, VGRID / 2 },
	{ 0, 1, HGRID - 1, VGRID / 2 + 1 },

	{ 0, VGRID / 2, HGRID - 1, VGRID - 2 },
	{ 1, VGRID / 2 + 1, HGRID - 1, VGRID - 1 },
	{ -1 }
};

VLines	swengv_sw_ne_straight[] = {
	{ HGRID / 2 + 1, 0, HGRID / 2 + 1, VGRID - 1 },
	{ HGRID / 2 - 0, 0, HGRID / 2 - 0, VGRID - 1 },

	{ 0, VGRID - 2, HGRID - 2, 0 },
	{ 0, VGRID - 1, HGRID - 1, 0 },
	{ 1, VGRID - 1, HGRID - 1, 1 },

	{ -1 }
};

VLines	swengv_sw_ne_switched[] = {

	{ 0, VGRID - 2, HGRID / 2 - 0, 0 },
	{ 0, VGRID - 1, HGRID / 2 + 1, 0 },

	{ HGRID / 2 - 0, VGRID - 1, HGRID - 1, 0 },
	{ HGRID / 2 + 1, VGRID - 1, HGRID - 1, 1 },
	{ -1 }
};

VLines	swengv_nw_se_straight[] = {
	{ HGRID / 2 + 1, 0, HGRID / 2 + 1, VGRID - 1 },
	{ HGRID / 2 - 0, 0, HGRID / 2 - 0, VGRID - 1 },

	{ 1, 0, HGRID - 1, VGRID - 2 },
	{ 0, 0, HGRID - 1, VGRID - 1 },
	{ 0, 1, HGRID - 2, VGRID - 1 },

	{ -1 }
};

VLines	swengv_nw_se_switched[] = {

	{ 0, 0, HGRID / 2 - 1, VGRID - 1 },
	{ 0, 1, HGRID / 2 - 0, VGRID - 1 },

	{ HGRID / 2 - 0, 0, HGRID - 1, VGRID - 2 },
	{ HGRID / 2 + 1, 0, HGRID - 1, VGRID - 1 },
	{ -1 }
};

VLines	block_layout[] = {
	{ HGRID / 2, VGRID / 2 - 1, HGRID / 2, VGRID / 2 + 2 },
	{ -1 }
};

VLines	block_layout_ns[] = {
	{ HGRID / 2 - 1, VGRID / 2, HGRID / 2 + 2, VGRID / 2 },
	{ -1 }
};

VLines	station_block_layout[] = {
	{ HGRID / 2, 0, 0, VGRID / 2 },
	{ 0, VGRID / 2, HGRID / 2, VGRID - 1 },
	{ HGRID / 2, VGRID - 1, HGRID - 1, VGRID / 2 },
	{ HGRID / 2, 0, HGRID - 1, VGRID / 2 },
	{ -1 }
};

VLines	nw_se_layout[] = {
	{ 1, 0, HGRID - 1, VGRID - 2 },
	{ 0, 0, HGRID - 1, VGRID - 1 },
	{ 0, 1, HGRID - 2, VGRID - 1 },
	{ -1 }
};

VLines	sw_ne_layout[] = {
	{ 0, VGRID - 2, HGRID - 2, 0 },
	{ 0, VGRID - 1, HGRID - 1, 0 },
	{ 1, VGRID - 1, HGRID - 1, 1 },
	{ -1 }
};

VLines	switch_rect[] = {
	{ 0, 0, HGRID - 1, 0 },
	{ HGRID - 1, 0, HGRID - 1, VGRID - 1 },
	{ 0, 0, 0, VGRID - 1 },
	{ 0, VGRID - 1, HGRID - 1, VGRID - 1 },
	{ -1 }
};

VLines	w_e_platform_out[] = {
	{ 0, VGRID / 2 - 3, HGRID - 1, VGRID / 2 - 3 },
	{ 0, VGRID / 2 + 3, HGRID - 1, VGRID / 2 + 3 },
	{ 0, VGRID / 2 - 3, 0, VGRID / 2 + 3 },
	{ HGRID - 1, VGRID / 2 - 3, HGRID - 1, VGRID / 2 + 3 },
	{ -1 }
};

VLines	w_e_platform_in[] = {
	{ 1, VGRID / 2 - 2, HGRID - 2, VGRID / 2 - 2 },
	{ 1, VGRID / 2 - 1, HGRID - 2, VGRID / 2 - 1 },
	{ 1, VGRID / 2 - 0, HGRID - 2, VGRID / 2 - 0 },
	{ 1, VGRID / 2 + 1, HGRID - 2, VGRID / 2 + 1 },
	{ 1, VGRID / 2 + 2, HGRID - 2, VGRID / 2 + 2 },
	{ -1 }
};

VLines	n_s_platform_out[] = {
	{ HGRID / 2 - 3, 0, HGRID / 2 - 3, VGRID - 1 },
	{ HGRID / 2 + 3, 0, HGRID / 2 + 3, VGRID - 1 },
	{ HGRID / 2 - 3, 0, HGRID / 2 + 3, 0 },
	{ HGRID / 2 - 3, VGRID - 1, HGRID / 2 + 3, VGRID - 1 },
	{ -1 }
};

VLines	n_s_platform_in[] = {
	{ HGRID / 2 - 2, 1, HGRID / 2 - 2, VGRID - 2 },
	{ HGRID / 2 - 1, 1, HGRID / 2 - 1, VGRID - 2 },
	{ HGRID / 2 - 0, 1, HGRID / 2 - 0, VGRID - 2 },
	{ HGRID / 2 + 1, 1, HGRID / 2 + 1, VGRID - 2 },
	{ HGRID / 2 + 2, 1, HGRID / 2 + 2, VGRID - 2 },
	{ -1 }
};

VLines	itin_layout[] = {
	{ 0, 0, HGRID - 1, VGRID - 1 },
	{ 0, VGRID / 2, HGRID - 1, VGRID / 2 },
	{ 0, VGRID - 1, HGRID - 1, 0 },
	{ -1 }
};

VLines  etrigger_layout[] = {
	{ 1, 2, HGRID - 2, 2 },
	{ 1, 2, HGRID / 2, VGRID - 2 },
	{ HGRID / 2, VGRID - 2, HGRID - 2, 2 },
	{ -1 }
};

VLines  wtrigger_layout[] = {
	{ 1, VGRID - 2, HGRID - 2, VGRID - 2 },
	{ 1, VGRID - 2, HGRID / 2, 2 },
	{ HGRID / 2, 2, HGRID - 2, VGRID - 2 },
	{ -1 }
};

VLines  ntrigger_layout[] = {
	{ 2, 1, 2, VGRID - 2 },
	{ 2, 1, HGRID - 2, VGRID / 2 },
	{ 2, VGRID - 2, HGRID - 2, VGRID / 2 },
	{ -1 }
};

VLines  strigger_layout[] = {
	{ HGRID - 2, 1, HGRID - 2, VGRID - 2 },
	{ 2, VGRID / 2, HGRID - 2, 1 },
	{ 2, VGRID / 2, HGRID - 2, VGRID - 2 },
	{ -1 }
};

#if 0
VLines	w_link[] = {
	{ 0, VGRID / 2, HGRID / 2, VGRID / 2 },
	{ HGRID / 2, VGRID / 2, HGRID / 2 + 4, VGRID / 2 - 4 },
	{ HGRID / 2, VGRID / 2, HGRID / 2 + 4, VGRID / 2 + 4 },
	{ -1 }
};

VLines	e_link[] = {
	{ HGRID / 2 - 4, VGRID / 2 - 4, HGRID / 2, VGRID / 2 },
	{ HGRID / 2 - 4, VGRID / 2 + 4, HGRID / 2, VGRID / 2 },
	{ HGRID / 2, VGRID / 2, HGRID - 1, VGRID / 2 },
	{ -1 }
};

#endif

static  Shape	    *e_train_pmap_default[NTTYPES];
static  Shape	    *w_train_pmap_default[NTTYPES];
static  Shape	    *e_car_pmap_default[NTTYPES];
static  Shape	    *w_car_pmap_default[NTTYPES];

Shape	*e_train_pmap[NTTYPES];
static const char * e_train_xpm[] = {
"13 10 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"X      c #0000FFFFFFFF",*/
"             ",
"...........  ",
".XXXXXXXXX.. ",
".X..X..X..X..",
".XXXXXXXXXXX.",
".XXXXXXXXXXX.",
".............",
"  ...   ...  ",
"             ",
"             ",
0
};

Shape	*w_train_pmap[NTTYPES];
static const char * w_train_xpm[] = {
"13 10 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"X      c #0000FFFFFFFF",*/
"             ",
"  ...........",
" ..XXXXXXXXX.",
"..X.X..X..XX.",
".XXXXXXXXXXX.",
".XXXXXXXXXXX.",
".............",
"  ...   ...  ",
"             ",
"             ",
0
};

Shape	*w_car_pmap[NTTYPES];
Shape	*e_car_pmap[NTTYPES];
static	const char * car_xpm[] = {	/* same for both e and w */
"13 10 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"X      c #0000FFFFFFFF",*/
"             ",
"............ ",
".XXXXXXXXXX. ",
".X..X..X..X. ",
".XXXXXXXXXX. ",
"XXXXXXXXXXXX ",
"............ ",
" ...    ...  ",
"             ",
"             ",
0
};

static	Shape   *speedShape;
static	const char *speed_xpm[] = {
"8 3 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
"X      c #000000000000",
"  ....  ",
" ..  .. ",
"  ....  ",
0
};

Shape   *cameraShape;
static	const char *camera_xpm[] = {
"13 10 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
"X      c #0000FFFFFFFF",
"             ",
"   ..        ",
" ........... ",
" . ..      . ",
" .   ...   . ",
" .   . .   . ",
" .   ...   . ",
" .         . ",
" ........... ",
"             ",
0
};

Shape   *itineraryButton;
static	const char *itin_xpm[] = {
"8 9 4 1",
"       c lightgray",
".      c #000000000000",
"X      c gray",
"#      c black",
"        ",
"  ....  ",
" ...... ",
"..XXXX..",
".XXXXXX.",
"..XXXX..",
"#......#",
" #....# ",
"  ####  ",
0
};


static	const char	*ttypecolors[NTTYPES] = {
	"orange", "cyan", "blue", "yellow",
	"white", "red", "brown", "green",
        "magenta", "lightgray"
};


/*
 *	Tools-types pixmaps
 *	(created from xpms defined in the i*.h files)
 */

void	*tracks_pixmap, *switches_pixmap, *signals_pixmap,
	*tools_pixmap, *actions_pixmap,
	*move_start_pixmap, *move_end_pixmap, *move_dest_pixmap,
        *set_power_pixmap, *set_track_len_pixmap;

static	char	buff[256];

void    restoreTrainPixmaps(void)
{
        int     l;

	for(l = 0; l < NTTYPES; ++l) {
	    e_train_pmap[l] = e_train_pmap_default[l];
	    w_train_pmap[l] = w_train_pmap_default[l];
	    e_car_pmap[l] = e_car_pmap_default[l];
	    w_car_pmap[l] = w_car_pmap_default[l];
	}
}


void	init_pmaps(void)
{
	int	r, g, b;
	int	fgr, fgg, fgb;
	int	i;
        wxColor bgColor;
	char	bufffg[64];
	char	buffcol[64];
        Char    shapeName[64];

        shapeManager.ClearAll();

	getcolor_rgb(fieldcolors[COL_TRACK], &fgr, &fgg, &fgb);
	sprintf(bufffg, ".      c #%02x00%02x00%02x00", fgr, fgg, fgb);
//	getcolor_rgb(fieldcolors[COL_BACKGROUND], &r, &g, &b);

        setBackgroundColor(bgColor);
        r = bgColor.Red();
        g = bgColor.Green();
        b = bgColor.Blue();

	sprintf(buff, "       c #%02x00%02x00%02x00", r, g, b);
//	sprintf(buff, "       c lightgray", r, g, b);

	e_train_xpm[1] = w_train_xpm[1] = car_xpm[1] = buff;
	e_train_xpm[2] = w_train_xpm[2] = car_xpm[2] = bufffg;
	e_train_xpm[3] = w_train_xpm[3] = car_xpm[3] = buffcol;

	for(i = 0; i < NTTYPES; ++i) {
            wxSprintf(shapeName, wxT("eloc%02d"), i + 1);
	    sprintf(buffcol, "X      c %s", ttypecolors[i]);
	    //e_train_pmap_default[i] = get_pixmap(e_train_xpm);
            e_train_pmap_default[i] = shapeManager.NewShape(shapeName, e_train_xpm);
            e_train_pmap_default[i]->_builtin = true;
	    //w_train_pmap_default[i] = get_pixmap(w_train_xpm);
            shapeName[0] = 'w';
            w_train_pmap_default[i] = shapeManager.NewShape(shapeName, w_train_xpm);
            w_train_pmap_default[i]->_builtin = true;
            shapeName[1] = 'c'; shapeName[2] = 'a'; shapeName[3] = 'r';
	    //w_car_pmap_default[i] = get_pixmap(car_xpm);
            w_car_pmap_default[i] = shapeManager.NewShape(shapeName, car_xpm);
            w_car_pmap_default[i]->_builtin = true;
	    //e_car_pmap_default[i] = get_pixmap(car_xpm);
            shapeName[0] = 'e';
            e_car_pmap_default[i] = shapeManager.NewShape(shapeName, car_xpm);
            e_car_pmap_default[i]->_builtin = true;
	}

	Signal::InitPixmaps();

	sprintf(buff, "       c #%02x00%02x00%02x00", r, g, b);
	sprintf(bufffg, ".      c #%02x00%02x00%02x00", fgr, fgg, fgb);
	speed_xpm[1] = buff;
	speed_xpm[2] = bufffg;
	speedShape = shapeManager.NewShape(wxT("speed"), speed_xpm);
        speedShape->_builtin = true;

        camera_xpm[1] = buff;
        cameraShape = shapeManager.NewShape(wxT("camera"), camera_xpm);
        speedShape->_builtin = true;

        itin_xpm[1] = buff;
        itineraryButton = shapeManager.NewShape(wxT("itinButton"), itin_xpm);
        speedShape->_builtin = true;

        restoreTrainPixmaps();  // init trains to default pixmaps

	// tools-types pixmaps

	tracks_pixmap = get_pixmap(tracks_xpm);
	switches_pixmap = get_pixmap(switches_xpm);
	signals_pixmap = get_pixmap(signals_xpm);
	tools_pixmap = get_pixmap(tools_xpm);
	actions_pixmap = get_pixmap(actions_xpm);
	move_start_pixmap = get_pixmap(move_start_xpm);
	move_end_pixmap = get_pixmap(move_end_xpm);
	move_dest_pixmap = get_pixmap(move_dest_xpm);
        set_power_pixmap = get_pixmap(set_power_xpm);
        set_track_len_pixmap = get_pixmap(set_track_len_xpm);

}

void	free_pixmaps(void)
{
#if 0
	int	    i;

	for(i = 0; i < NTTYPES; ++i) {
	    delete_pixmap(e_train_pmap[i]);
	    delete_pixmap(w_train_pmap[i]);
	    delete_pixmap(e_car_pmap[i]);
	    delete_pixmap(w_car_pmap[i]);
	}
#else
        shapeManager.ClearAll();
#endif

	Signal::FreePixmaps();
	delete_pixmap(tracks_pixmap);
	delete_pixmap(switches_pixmap);
	delete_pixmap(signals_pixmap);
	delete_pixmap(tools_pixmap);
	delete_pixmap(actions_pixmap);
	delete_pixmap(move_start_pixmap);
	delete_pixmap(move_end_pixmap);
	delete_pixmap(move_dest_pixmap);
	delete_pixmap(set_power_pixmap);
	delete_pixmap(set_track_len_pixmap);
}

Track	*track_new(void)
{
	Track	*t;

//	t = (Track *)malloc(sizeof(Track));
//	memset(t, 0, sizeof(Track));
	t = new Track();
	t->xsize = 1;
	t->ysize = 1;
	t->type = NOTRACK;
	t->direction = NODIR;
	t->fgcolor = fieldcolors[COL_TRACK];
	return(t);
}

void	track_delete(Track *t)
{
	Track	*t1, *old;

	if(t == layout)
	    layout = t->next;
	else {
	    old = layout;
	    for(t1 = old->next; t1 != t; t1 = t1->next)
		old = t1;
	    old->next = t->next;
	}
	if(t->station)
	    free(t->station);
	delete_script_data(t);
        onIconUpdateListeners.Remove(t);
	//free(t);
	delete t;
	link_all_tracks();
}

void	track_name(Track *t, wxChar *name)
{
	if(t->station)
	    free(t->station);
	t->station = wxStrdup(name);
}

int	translate_track_color(Track *t)
{
	int fg = curSkin->free_track;

	switch(t->status) {
	case ST_FREE:
		break;
	case ST_BUSY:
//		fg = color_red;
		return curSkin->occupied_track;
	case ST_READY:
//		fg = color_green;
		return curSkin->reserved_track;
	case ST_WORK:
//		fg = color_blue;
		return curSkin->working_track;
	}
	if(t->fgcolor == color_orange || t->fgcolor == color_red || t->fgcolor == color_bise)  //lrg 20210216
	    return curSkin->occupied_track;
	if(t->fgcolor == color_green)
	    return curSkin->reserved_track;
	if(t->fgcolor == color_white)
	    return curSkin->reserved_shunting;
	if(t->fgcolor == color_blue)
	    return curSkin->working_track;
	if(t->fgcolor == color_approach)
		return curSkin->text;
	return fg;
}


void	track_draw(Track *t)
{
	int	fg;
	int	tot;
	VLines	*lns = n_s_layout;	// provide dummy initialization - always overwritten

	fg = translate_track_color(t);
	switch(t->direction) {

	case TRK_N_S:
                if(t->power) {
                    draw_mid_point(t->x, t->y, -2, 0, fg);
                }
		lns = n_s_layout;
		break;

	case SW_N:
		lns = sw_n_layout;
		break;

	case NW_S:
		lns = nw_s_layout;
		break;

	case W_E:
                if(t->power) {
                    draw_mid_point(t->x, t->y, 0, -2, fg);
                }
		lns = w_e_layout;
		break;

	case NW_E:
		lns = nw_e_layout;
		break;

	case SW_E:
		lns = sw_e_layout;
		break;

	case W_NE:
		lns = w_ne_layout;
		break;

	case W_SE:
		lns = w_se_layout;
		break;

	case NW_SE:
                if(t->power) {
                    draw_mid_point(t->x, t->y, 2, -2, fg);
                }
		lns = nw_se_layout;
		break;

	case SW_NE:
                if(t->power) {
                    draw_mid_point(t->x, t->y, -2, -2, fg);
                }
		lns = sw_ne_layout;
		break;

	case NE_S:
		lns = ne_s_layout;
		break;

	case SE_N:
		lns = se_n_layout;
		break;

	case XH_NW_SE:
		fg = t->direction;
		t->direction = NW_SE;
		track_draw(t);
		t->direction = W_E;
		track_draw(t);
		t->direction = (trkdir)fg;
		return;

	case XH_SW_NE:
		fg = t->direction;
		t->direction = SW_NE;
		track_draw(t);
		t->direction = W_E;
		track_draw(t);
		t->direction = (trkdir)fg;
		return;

	case X_X:
		fg = t->direction;
		t->direction = SW_NE;
		track_draw(t);
		t->direction = NW_SE;
		track_draw(t);
		t->direction = (trkdir)fg;
		return;

	case X_PLUS:
		fg = t->direction;
		t->direction = TRK_N_S;
		track_draw(t);
		t->direction = W_E;
		track_draw(t);
		t->direction = (trkdir)fg;
		return;

	case N_NE_S_SW:
		fg = t->direction;
		t->direction = TRK_N_S;
		track_draw(t);
		t->direction = SW_NE;
		track_draw(t);
		t->direction = (trkdir)fg;
		return;

	case N_NW_S_SE:
		fg = t->direction;
		t->direction = TRK_N_S;
		track_draw(t);
		t->direction = NW_SE;
		track_draw(t);
		t->direction = (trkdir)fg;
		return;
	}
	draw_layout(t->x, t->y, lns, fg);
        if(show_blocks._iValue) {
	    if(t->direction == W_E && t->length >= 100)
	        draw_layout(t->x, t->y, block_layout, curSkin->outline); //fieldcolors[TRACK]);
	    if(t->direction == TRK_N_S && t->length >= 100)
	        draw_layout(t->x, t->y, block_layout_ns, curSkin->outline); //fieldcolors[TRACK]);
            if(t->station)
	        draw_layout(t->x, t->y, station_block_layout, curSkin->outline); //fieldcolors[TRACK]);
        }
	if(editing && show_links) {
	    if(t->wlinkx && t->wlinky)
		draw_link(t->x, t->y, t->wlinkx, t->wlinky, conf.linkcolor2);
	    if(t->elinkx && t->elinky)
		draw_link(t->x, t->y, t->elinkx, t->elinky, conf.linkcolor2);
	}
	if(!show_speeds._iValue)
	    return;
	tot = 0;
	for(fg = 0; fg < NTTYPES; ++fg)
	    tot += t->speed[fg];
	if(tot)
	    draw_pixmap(t->x, t->y, speedShape->GetImage());
}

void	switch_draw(Track *t)
{
	int	fg;
	int	tmp;

	fg = translate_track_color(t);
	tmp = t->direction;
	switch(tmp) {
	case 0:
		if(editing) {
		    t->direction = W_NE;
		    track_draw(t);
		    t->direction = W_E;
		    track_draw(t);
		} else if(t->switched) {
		    t->direction = W_NE;
		    track_draw(t);
		} else
		    t->direction = W_E;
		    track_draw(t);
		break;

	case 1:
		if(editing) {
		    t->direction = NW_E;
		    track_draw(t);
		    t->direction = W_E;
		    track_draw(t);
		} else if(t->switched) {
		    t->direction = NW_E;
		    track_draw(t);
		} else
		    t->direction = W_E;
		    track_draw(t);
		break;

	case 2:
		if(editing) {
		    t->direction = W_SE;
		    track_draw(t);
		    t->direction = W_E;
		    track_draw(t);
		} else if(t->switched) {
		    t->direction = W_SE;
		    track_draw(t);
		} else
		    t->direction = W_E;
		    track_draw(t);
		break;

	case 3:
		if(editing) {
		    t->direction = SW_E;
		    track_draw(t);
		    t->direction = W_E;
		    track_draw(t);
		} else if(t->switched) {
		    t->direction = SW_E;
		    track_draw(t);
		} else
		    t->direction = W_E;
		    track_draw(t);
		break;

	case 4:
		if(editing) {
		    t->direction = SW_E;
		    track_draw(t);
		    t->direction = SW_NE;
		} else if(t->switched)
		    t->direction = SW_E;
		else
		    t->direction = SW_NE;
		track_draw(t);
		break;

	case 5:
		if(editing) {
		    t->direction = W_NE;
		    track_draw(t);
		    t->direction = SW_NE;
		} else if(t->switched)
		    t->direction = W_NE;
		else
		    t->direction = SW_NE;
		track_draw(t);
		break;

	case 6:
		if(editing) {
		    t->direction = NW_E;
		    track_draw(t);
		    t->direction = NW_SE;
		} else if(t->switched) {
		    t->direction = NW_E;
		} else
		    t->direction = NW_SE;
		track_draw(t);
		break;

	case 7:
		if(editing) {
		    t->direction = W_SE;
		    track_draw(t);
		    t->direction = NW_SE;
		} else if(t->switched)
		    t->direction = W_SE;
		else
		    t->direction = NW_SE;
		track_draw(t);
		break;

	case 8:				/* horizontal english switch */
		if(t->switched && !editing)
		    draw_layout(t->x, t->y, sweng_sw_ne_switched, fg);
		else
		    draw_layout(t->x, t->y, sweng_sw_ne_straight, fg);
		break;

	case 9:				/* horizontal english switch */
		if(t->switched && !editing)
		    draw_layout(t->x, t->y, sweng_nw_se_switched, fg);
		else
		    draw_layout(t->x, t->y, sweng_nw_se_straight, fg);
		break;

	case 10:
		if(editing) {
		    t->direction = W_SE;
		    track_draw(t);
		    t->direction = W_NE;
		} else if (t->switched)
		    t->direction = W_SE;
		else
		    t->direction = W_NE;
		track_draw(t);
		break;

	case 11:
		if(editing) {
		    t->direction = SW_E;
		    track_draw(t);
		    t->direction = NW_E;
		} else if (t->switched)
		    t->direction = SW_E;
		else
		    t->direction = NW_E;
		track_draw(t);
		break;

	case 12:
		if(editing) {
		    t->direction = TRK_N_S;
		    track_draw(t);
		    t->direction = SW_N;
		} else if(t->switched)
		    t->direction = SW_N;
		else
		    t->direction = TRK_N_S;
		track_draw(t);
		break;

	case 13:
		if(editing) {
		    t->direction = TRK_N_S;
		    track_draw(t);
		    t->direction = SE_N;
		} else if(t->switched)
		    t->direction = SE_N;
		else
		    t->direction = TRK_N_S;
		track_draw(t);
		break;

	case 14:
		if(editing) {
		    t->direction = TRK_N_S;
		    track_draw(t);
		    t->direction = NW_S;
		} else if(t->switched)
		    t->direction = NW_S;
		else
		    t->direction = TRK_N_S;
		track_draw(t);
		break;

	case 15:
		if(editing) {
		    t->direction = TRK_N_S;
		    track_draw(t);
		    t->direction = NE_S;
		} else if(t->switched)
		    t->direction = NE_S;
		else
		    t->direction = TRK_N_S;
		track_draw(t);
		break;

	case 16:			/* vertical english switch */
		if(t->switched && !editing)
		    draw_layout(t->x, t->y, swengv_sw_ne_switched, fg);
		else
		    draw_layout(t->x, t->y, swengv_sw_ne_straight, fg);
		break;

	case 17:			/* vertical english switch */
		if(t->switched && !editing)
		    draw_layout(t->x, t->y, swengv_nw_se_switched, fg);
		else
		    draw_layout(t->x, t->y, swengv_nw_se_straight, fg);
		break;

	case 18:
		if(editing) {
		    t->direction = SW_NE;
		    track_draw(t);
		    t->direction = SW_N;
		} else if(t->switched)
		    t->direction = SW_N;
		else
		    t->direction = SW_NE;
		track_draw(t);
		break;

	case 19:
		if(editing) {
		    t->direction = SW_NE;
		    track_draw(t);
		    t->direction = NE_S;
		} else if(t->switched)
		    t->direction = NE_S;
		else
		    t->direction = SW_NE;
		track_draw(t);
		break;

	case 20:
		if(editing) {
		    t->direction = NW_SE;
		    track_draw(t);
		    t->direction = SE_N;
		} else if(t->switched)
		    t->direction = SE_N;
		else
		    t->direction = NW_SE;
		track_draw(t);
		break;

	case 21:
		if(editing) {
		    t->direction = NW_SE;
		    track_draw(t);
		    t->direction = NW_S;
		} else if(t->switched)
		    t->direction = NW_S;
		else
		    t->direction = NW_SE;
		track_draw(t);
		break;

	case 22:
		if(editing) {
		    t->direction = NW_S;
		    track_draw(t);
		    t->direction = NE_S;
		} else if(t->switched)
		    t->direction = NW_S;
		else
		    t->direction = NE_S;
		track_draw(t);
		break;

	case 23:
		if(editing) {
		    t->direction = SW_N;
		    track_draw(t);
		    t->direction = SE_N;
		} else if(t->switched)
		    t->direction = SW_N;
		else
		    t->direction = SE_N;
		track_draw(t);
		break;
	}
        if(!t->norect) {
            if(t->_leftDwellTime > 0)
                draw_layout(t->x, t->y, switch_rect, curSkin->working_track);
            else
	        draw_layout(t->x, t->y, switch_rect, curSkin->outline); //fieldcolors[TRACK]);
        }
	t->direction = (trkdir)tmp;
}

void	platform_draw(Track *t)
{
	switch(t->direction) {
	case W_E:
		draw_layout(t->x, t->y, w_e_platform_out, curSkin->free_track); //fieldcolors[TRACK]);
		draw_layout(t->x, t->y, w_e_platform_in, curSkin->outline);
		break;

	case N_S:
		draw_layout(t->x, t->y, n_s_platform_out, curSkin->free_track);//fieldcolors[TRACK]);
		draw_layout(t->x, t->y, n_s_platform_in, curSkin->outline);
		break;
	}
}

void	signal_draw(Track *t)
{
	Signal	*signal = (Signal *)t;
	signal->Draw();
}

void	*get_train_pixels(Train *trn)
{
	void	*pixels;

	if(swap_head_tail && (trn->flags & TFLG_SWAPHEADTAIL)) {
	    if(trn->direction == W_E)
//		pixels = trn->wpix == -1 ?
//                    (trn->_locoShape = w_train_pmap[trn->type])->GetImage() : pixmaps[trn->wpix].pixels;
                pixels = trn->wpix->GetImage();
	    else
//		pixels = trn->epix == -1 ?
//                    (trn->_locoShape = e_train_pmap[trn->type])->GetImage() : pixmaps[trn->epix].pixels;
                pixels = trn->epix->GetImage();
	} else {
	    if(trn->direction == W_E)
//		pixels = trn->epix == -1 ?
//                    (trn->_locoShape = e_train_pmap[trn->type])->GetImage() : pixmaps[trn->epix].pixels;
                pixels = trn->epix->GetImage();
	    else
//		pixels = trn->wpix == -1 ?
//                    (trn->_locoShape = w_train_pmap[trn->type])->GetImage() : pixmaps[trn->wpix].pixels;
                pixels = trn->wpix->GetImage();
	}
	return pixels;
}

void	*get_car_pixels(Train *trn)
{
	void	*pixels;

	if(swap_head_tail && (trn->flags & TFLG_SWAPHEADTAIL)) {
	    if(trn->direction == W_E)
//		pixels = trn->wcarpix == -1 || trn->wcarpix >= ncarpixmaps ?
//                            (trn->_carShape = w_car_pmap[trn->type])->GetImage() : carpixmaps[trn->wcarpix].pixels;
                pixels = trn->wcarpix->GetImage();
	    else
//		pixels = trn->ecarpix == -1 || trn->ecarpix >= ncarpixmaps ?
//			    (trn->_carShape = e_car_pmap[trn->type])->GetImage() : carpixmaps[trn->ecarpix].pixels;
                pixels = trn->ecarpix->GetImage();
	} else {
	    if(trn->direction == W_E)
//		pixels = trn->ecarpix == -1 || trn->ecarpix >= ncarpixmaps ?
//			    (trn->_carShape = e_car_pmap[trn->type])->GetImage() : carpixmaps[trn->ecarpix].pixels;
                pixels = trn->ecarpix->GetImage();
	    else
//		pixels = trn->wcarpix == -1 || trn->wcarpix >= ncarpixmaps ?
//			    (trn->_carShape = w_car_pmap[trn->type])->GetImage() : carpixmaps[trn->wcarpix].pixels;
                pixels = trn->wcarpix->GetImage();
	}
	return pixels;
}

void    get_basic_name(Train *trn, Char *dest, int size)
{
        int     i;

        // isolate the main component of a train's name, usually the number
        wxStrncpy(dest, trn->name, size);
        dest[size - 1] = 0;
        for(i = 0; dest[i]; ++i)
            if(dest[i] == wxT(' '))
                break;
        dest[i] = 0;
}

void	train_draw(Track *t, Train *trn)
{
	void	*pixels;
        Char    name[256];
        int     smallText;

	if(!e_train_pmap[0]) {
	    init_pmaps();
	}
        if(draw_train_names._iValue) {
            get_basic_name(trn, name, sizeof(name)/sizeof(name[0]));
            smallText = small_train_names._iValue;
            if(no_train_names_colors._iValue) {
                if(no_train_names_background._iValue)
                    draw_text_with_foreground(t->x, t->y, name, smallText, color_green);
                else
                    draw_text_with_background(t->x, t->y, name, smallText, color_green);
            } else if(no_train_names_background._iValue)
                draw_text_with_foreground(t->x, t->y, name, smallText, fieldcolors[COL_TRAIN1 + trn->type]);
            else
                draw_text_with_background(t->x, t->y, name, smallText, fieldcolors[COL_TRAIN1 + trn->type]);
            return;
        }
	pixels = get_train_pixels(trn);
	if(swap_head_tail && (trn->flags & TFLG_SWAPHEADTAIL) && trn->length &&
	    trn->tail && trn->position && trn->position != trn->tail->position)
	    pixels = get_car_pixels(trn);
	draw_pixmap(t->x, t->y, pixels);
}

void	car_draw(Track *t, Train *trn)
{
	void	*pixels;
        Char    name[256];
        int     smallText;

	if(!e_car_pmap[0]) {
	    init_pmaps();
	}
        if(draw_train_names._iValue) {
            get_basic_name(trn, name, sizeof(name)/sizeof(name[0]));
            smallText = small_train_names._iValue;
            if(no_train_names_colors._iValue) {
                if(no_train_names_background._iValue)
                    draw_text_with_foreground(t->x, t->y, name, smallText, color_green);
                else
                    draw_text_with_background(t->x, t->y, name, smallText, color_green);
            } else if(no_train_names_background._iValue)
                draw_text_with_foreground(t->x, t->y, name, smallText, fieldcolors[COL_TRAIN1 + trn->type]);
            else
                draw_text_with_background(t->x, t->y, name, smallText, fieldcolors[COL_TRAIN1 + trn->type]);
            return;
        }
	pixels = get_car_pixels(trn);
	if(swap_head_tail && (trn->flags & TFLG_SWAPHEADTAIL))
	    pixels = get_train_pixels(trn);
	draw_pixmap(t->x, t->y, pixels);
}

void	text_draw(Track *t)
{
	if(!t->station)
	    return;
    	if(!editing && t->invisible)
	    return;
	tr_fillrect(t->x, t->y);
	if(t->_fontIndex)
	    draw_layout_text_font(t->x, t->y, t->station, t->_fontIndex);
	else
	    draw_layout_text1(t->x, t->y, t->station, t->direction);
	if(!editing || !show_links)
	    return;
	if(t->elinkx && t->elinky)
	    draw_link(t->x, t->y, t->elinkx, t->elinky, conf.linkcolor);
	else if(t->wlinkx && t->wlinky)
	    draw_link(t->x, t->y, t->wlinkx, t->wlinky, conf.linkcolor);
}

void	link_draw(Track *t)
{
	tr_fillrect(t->x, t->y);
	if(t->direction == W_E)
	    draw_layout_text1(t->x, t->y, wxT("...to..."), 1);
	else
	    draw_layout_text1(t->x, t->y, wxT("Link..."), 1);
}

void	macro_draw(Track *t)
{
	tr_fillrect(t->x, t->y);
	if(t->direction == 0)
	    draw_layout_text1(t->x, t->y, wxT("Macro"), 1);
	else
	    draw_layout_text1(t->x, t->y, wxT("Place"), 1);
}

void	itin_draw(Track *t)
{
        tr_fillrect(t->x, t->y);
	draw_pixmap(t->x, t->y, itineraryButton->GetImage());

	if(t->station) {
#if 0 // !Rask Ingemann Lambertsen
	    draw_itin_text(t->x, t->y, t->station, t->direction == 1);
#else
	    wxChar *label = wxStrrchr(t->station, '@');
	    if(label)
		label++;
	    else
		label = t->station;
	    draw_itin_text(t->x, t->y, label, t->direction == 1);
#endif
	}
}

void	mover_draw()
{
	draw_link(move_start.x, move_start.y, move_end.x, move_end.y, color_white);
}

void	trigger_draw(Track *t)
{
	VLines	*img;

	switch(t->direction) {
	case S_N:
	    img = ntrigger_layout;
	    break;
	case N_S:
	    img = strigger_layout;
	    break;
	case W_E:
	    img = etrigger_layout;
	    break;
	case E_W:
	    img = wtrigger_layout;
	    break;
	default:
	    return;
	}

	draw_layout(t->x, t->y, img, curSkin->working_track);
	if(editing && show_links) {
	    if(t->wlinkx && t->wlinky)
		draw_link(t->x, t->y, t->wlinkx, t->wlinky, conf.linkcolor);
	}
}

void	image_draw(Track *t)
{
	wxChar	buff[256];
	wxChar	*p;
	void	*pixels = 0;
	int	ix;

	if(t->direction || !t->station || !*t->station) {/* filename! */
	    pixels = cameraShape->GetImage();
	} else {
	    if(t->_isFlashing && t->_flashingIcons[t->_nextFlashingIcon])
		ix = get_pixmap_index(t->_flashingIcons[t->_nextFlashingIcon]);
	    else
		ix = get_pixmap_index(t->station);
	    if(ix < 0) {	    /* for UNIX, try lower case name */
		wxStrcpy(buff, t->station);
		for(p = buff; *p; ++p)
		    if(*p >= 'A' && *p <= 'Z')
			*p += ' ';
		ix = get_pixmap_index(buff);
	    }
	    if(ix < 0) {
		wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s '%s'."), L("Error reading"), t->station);
		do_alert(buff);
		pixels = cameraShape->GetImage();
		if(t->_isFlashing)
		    t->_flashingIcons[t->_nextFlashingIcon] = 0;
		else
		    t->station = 0;
	    } else
		pixels = pixmaps[ix].pixels;
	}
	draw_pixmap(t->x, t->y, pixels);
        if(editing && show_links && t->wlinkx && t->wlinky)
            draw_link(t->x, t->y, t->wlinkx, t->wlinky, conf.linkcolor);
}

void	track_paint(Track *t)
{
	tr_fillrect(t->x, t->y);
	if(!editing && t->invisible)
	    return;

	switch(t->type) {
	case TRACK:
		track_draw(t);
		break;

	case SWITCH:
		switch_draw(t);
		break;

	case PLATFORM:
		platform_draw(t);
		break;

	case TSIGNAL:
		signal_draw(t);
		break;

	case TRAIN:		/* trains are handled differently */
	/*	train_draw(t); */
		break;

	case TEXT:
		text_draw(t);
		break;

	case LINK:
		link_draw(t);
		break;

	case IMAGE:
		image_draw(t);
		break;

	case MACRO:
		macro_draw(t);
		break;

	case ITIN:
		itin_draw(t);
		break;

	case TRIGGER:
		trigger_draw(t);
		break;

	default:
		return;
	}
	if(editing && show_scripts && t->stateProgram) {
	    draw_layout(t->x, t->y, switch_rect, curSkin->working_track);//fieldcolors[COL_TRAIN2]);
	}
}

const Char *GetColorName(int color)
{
	if(color == conf.fgcolor)
	    return wxT("black");
	if(color == color_white)
	    return wxT("white");
	if(color == color_orange)
	    return wxT("orange");
	if(color == color_green)
	    return wxT("green");
	if(color == color_red)
	    return wxT("red");
	if(color == color_blue)
	    return wxT("blue");
	if(color == color_cyan)
	    return wxT("cyan");
	if(color == color_bise)//lrg  20210215
	    return wxT("bise");
	if(color == color_approach)//lrg  20210220
	    return wxT("approach");
	return wxT("unknown");
}

const wxChar	*train_next_stop(Train *t, int *final)
{
	Track   *tr;
	static	wxChar	buff[512];
	TrainStop   *ts, *last;

	*final = 0;
	buff[0] = 0;
	buff[1] = 0;
	if(t->status != train_RUNNING && t->status != train_WAITING &&
	    t->status != train_STOPPED)
	    return buff;
	buff[0] = 0;
	last = 0;
	for(ts = t->stops; ts; ts = ts->next) {
	    if(!ts->minstop)
		continue;
	    if(!(tr = findStationNamed(ts->station)) || tr->type != TRACK)
		continue;
	    if(ts->stopped)
		continue;
//	    if(!last || ts->arrival < last->arrival)
		last = ts;
		break;
	}
	if(!last) {
	    tr = findStationNamed(t->exit);
	    if(!tr || tr->type == TEXT)
		return buff;
	    *final = 1;
	    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT(" %s %s %s %s   "), L("Final stop"), t->exit, L("at"), format_time(t->timeout));
	} else
	    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT(" %s %s %s %s   "), L("Next stop"), last->station, L("at"), format_time(last->arrival));
	return buff;
}

bool	is_canceled(Train *t)
{
	if(!t->days || !run_day || (t->days & run_day))
	    return false;
	return true;
}

const wxChar	*train_status0(Train *t, int full)
{
	static	wxChar	buff[1024];
	int	i, j, k, final;
        long    timedep;
        TrainStop *stp = 0;

	if(terse_status)
	    full = 0;
	buff[0] = 0;
	i = 0;
        if(t->isExternal) {
            return L("external");
        }
	switch(t->status) {
	case train_READY:
		if(!is_canceled(t)) {
		    if(!t->entryDelay || !t->entryDelay->nSeconds)
			return L("ready");
		    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar),
			wxT("%s ETA %s"), L("ready"), format_time(t->timein + t->entryDelay->nSeconds));
		    return buff;
		}
		wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s "), L("Canceled - runs on"));
		k = wxStrlen(buff);
		for(i = 1, j = '1'; i < 0x80; i <<= 1, ++j)
		    if(t->days & i)
			buff[k++] = j;
		buff[k] = 0;
		return buff;

	case train_RUNNING:
		if(full)
		    wxStrcpy(buff, train_next_stop(t, &final));
		if(t->shunting)
		    wxStrcpy(buff + wxStrlen(buff), L("Shunting"));
		else if(full) {
		    if(final)
			wxSnprintf(buff + wxStrlen(buff), sizeof(buff)/sizeof(wxChar) - wxStrlen(buff), wxT("%s: %d Km/h"), L("Speed"), (int)t->curspeed);
		    else
			wxSnprintf(buff + wxStrlen(buff), sizeof(buff)/sizeof(wxChar) - wxStrlen(buff), wxT("%s: %d Km/h %s %s"), L("Speed"),
				    (int)t->curspeed, L("to"), t->exit);
		} else
		    wxSnprintf(buff + wxStrlen(buff), sizeof(buff)/sizeof(wxChar) - wxStrlen(buff), wxT("%s %s"), L("Running. Dest."), t->exit);
		return buff;

	case train_STOPPED:
	        timedep = t->timedep;
	        stp = findStop(t, t->position);
	        if(stp && stp->depDelay && stp->depDelay->nSeconds)
		    timedep += stp->depDelay->nSeconds;
	        else if(t->position->station &&
		        sameStation(t->entrance, t->position->station) &&
		        t->entryDelay)
		    timedep += t->entryDelay->nSeconds;
		if(full) {
		    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %s "), L("Stopped. ETD"), format_time(timedep));
		    if(full)
			wxStrcat(buff, train_next_stop(t, &final));
		    if(!final) {
			wxStrcat(buff, L("Dest"));
			wxStrcat(buff, wxT(" "));
			wxStrcat(buff, t->exit);
		    }
		} else {
		    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %s  %s %s"), L("Stopped. ETD"), format_time(timedep),
			    L("Dest."), t->exit);
		}
		return buff;

	case train_DELAY:
		wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %s"), L("Delayed entry at"), t->entrance);
		return buff;

	case train_WAITING:
		wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s. %s%s %s"), L("Waiting"),
			full ? train_next_stop(t, &final) : wxT(""), L("Dest."), t->exit);
		return buff;

	case train_DERAILED:
		return L("derailed");

        case train_STARTING:
                wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s (-%d)"), L("Starting"), t->startDelay);
		return buff;

	case train_ARRIVED:
		if(t->wrongdest)
		    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %s %s %s"), L("Arrived at"), t->exited, L("instead of"), t->exit);
		else if(t->timeexited / 60 > t->timeout / 60)
		    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %d %s %s"), L("Arrived"),
			(t->timeexited - t->timeout0) / 60, L("min. late at"), t->exit);
		else
		    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), L("Arrived on time"));
		if(t->stock)
		    wxSnprintf(buff + wxStrlen(buff), sizeof(buff)/sizeof(wxChar) - wxStrlen(buff), wxT(" - %s %s"), L("stock for"), t->stock);
		return buff;
	}
	return wxT("");
}

const wxChar	*train_status(Train *t)
{
	return train_status0(t, 0);
}

void	walk_vertical(Track *trk, Track *t, trkdir *ndir)
{
	if(*ndir == N_S) {//lrg0604转线部分 加!t->isstation
	    if(t->elinkx && t->elinky && !t->isstation ) {
		trk->x = t->elinkx;
		trk->y = t->elinky;
		return;
	    }
	    trk->x = t->x;
	    trk->y = t->y + 1;
	    return;
	}
	if(t->wlinkx && t->wlinky) {
	    trk->x = t->wlinkx;
	    trk->y = t->wlinky;
	    return;
	}
	trk->x = t->x;
	trk->y = t->y - 1;
}

void	walk_vertical_switch(Track *trk, Track *t, trkdir *ndir)
{
	switch(t->direction) {
	case 12:
		if(*ndir == W_E)
		    *ndir = S_N;
		if(*ndir == S_N) {
		    trk->x = t->x;
		    trk->y = t->y - 1;
		} else if(t->switched) {
		    trk->x = t->x - 1;
		    trk->y = t->y + 1;
		    *ndir = E_W;
		} else {
		    trk->x = t->x;
		    trk->y = t->y + 1;
		}
		break;

	case 13:
		if(*ndir == E_W)
		    *ndir = S_N;
		if(*ndir == S_N) {
		    trk->x = t->x;
		    trk->y = t->y - 1;
		} else if(t->switched) {
		    trk->x = t->x + 1;
		    trk->y = t->y + 1;
		    *ndir = W_E;
		} else {
		    trk->x = t->x;
		    trk->y = t->y + 1;
		}
		break;

	case 14:
		if(*ndir == W_E)
		    *ndir = N_S;
		if(*ndir == N_S) {
		    trk->x = t->x;
		    trk->y = t->y + 1;
		} else if(t->switched) {
		    trk->x = t->x - 1;
		    trk->y = t->y - 1;
		    *ndir = E_W;
		} else {
		    trk->x = t->x;
		    trk->y = t->y - 1;
		}
		break;

	case 15:
		if(*ndir == E_W)
		    *ndir = N_S;
		if(*ndir == N_S) {
		    trk->x = t->x;
		    trk->y = t->y + 1;
		} else if(t->switched) {
		    trk->x = t->x + 1;
		    trk->y = t->y - 1;
		    *ndir = W_E;
		} else {
		    trk->x = t->x;
		    trk->y = t->y - 1;
		}
		break;

	case 18:
		if(t->switched) {
		    if(*ndir == W_E)
			*ndir = S_N;
		    if(*ndir == S_N) {
			trk->x = t->x;
			trk->y = t->y - 1;
		    } else {
			trk->x = t->x - 1;
			trk->y = t->y + 1;
			*ndir = E_W;
		    }
		    break;
		}
		if(*ndir == W_E) {
		    trk->x = t->x + 1;
		    trk->y = t->y - 1;
		} else {
		    trk->x = t->x - 1;
		    trk->y = t->y + 1;
		}
		break;

	case 19:
		if(t->switched) {
		    if(*ndir == E_W)
			*ndir = N_S;
		    if(*ndir == S_N) {
			trk->x = t->x + 1;
			trk->y = t->y - 1;
			*ndir = W_E;
		    } else {
			trk->x = t->x;
			trk->y = t->y + 1;
		    }
		    break;
		}
		if(*ndir == W_E || *ndir == S_N) {
		    trk->x = t->x + 1;
		    trk->y = t->y - 1;
		} else {
		    trk->x = t->x - 1;
		    trk->y = t->y + 1;
		}
		break;

	case 20:
		if(t->switched) {
		    if(*ndir == E_W)
			*ndir = S_N;
		    if(*ndir == N_S) {
			trk->x = t->x + 1;
			trk->y = t->y + 1;
			*ndir = W_E;
		    } else {
			trk->x = t->x;
			trk->y = t->y - 1;
		    }
		    break;
		}
		if(*ndir == W_E) {
		    trk->x = t->x + 1;
		    trk->y = t->y + 1;
		} else {
		    trk->x = t->x - 1;
		    trk->y = t->y - 1;
		}
		break;

	case 21:
		if(t->switched) {
		    if(*ndir == W_E)
			*ndir = N_S;
		    if(*ndir == S_N) {
			trk->x = t->x - 1;
			trk->y = t->y - 1;
			*ndir = E_W;
		    } else {
			trk->x = t->x;
			trk->y = t->y + 1;
		    }
		    break;
		}
		if(*ndir == W_E) {
		    trk->x = t->x + 1;
		    trk->y = t->y + 1;
		} else {
		    trk->x = t->x - 1;
		    trk->y = t->y - 1;
		}
		break;

	case 22:
		if(t->switched) {
		    if(*ndir == S_N) {
			trk->x = t->x - 1;
			trk->y = t->y - 1;
			*ndir = E_W;
			break;
		    }
		} else if(*ndir == S_N) {
		    trk->x = t->x + 1;
		    trk->y = t->y - 1;
		    *ndir = W_E;
		    break;
		}
		trk->x = t->x;
		trk->y = t->y + 1;
		*ndir = N_S;
		break;

	case 23:
		if(t->switched) {
		    if(*ndir == N_S) {
			trk->x = t->x - 1;
			trk->y = t->y + 1;
			*ndir = E_W;
			break;
		    }
		} else if(*ndir == N_S) {
		    trk->x = t->x + 1;
		    trk->y = t->y + 1;
		    *ndir = W_E;
		    break;
		}
		trk->x = t->x;
		trk->y = t->y - 1;
		*ndir = S_N;
		break;
	}
}

Track	*track_walkeast(Track *t, trkdir *ndir)//轨道变化******魏七龙
{
	static	Track	trk;
	Track	*trk1;//

	if(t->direction != TRK_N_S && t->elinkx && t->elinky) {
	    trk.x = t->elinkx;
	    trk.y = t->elinky;
	    return &trk;
	}
	if(t->direction == TRK_N_S && t->elinkx && t->elinky && t->wlinkx==0) {
	    trk.x = t->elinkx;
	    trk.y = t->elinky;
	    return &trk;
	}
	//S-N方向列车转至E-W方向列车 lrg20210816修改
	if(t->direction==W_E && *ndir==NODIR)
	{
		trk1 = track_walkwest(t,ndir);
		trk = *trk1;
		return &trk;
	}
	trk.x = t->x + 1;
	trk.y = t->y;
	switch(t->direction) {
	case NW_SE:
	case W_SE:
		++trk.y;
		break;
	case SW_NE:
	case W_NE:
		--trk.y;
		break;
	case SW_N:
		if(*ndir == N_S) {
		    trk.x = t->x - 1;
		    trk.y = t->y + 1;
		    *ndir = E_W;
		    break;
		}
		trk.y = t->y - 1;
		trk.x = t->x;
		*ndir = S_N;
		break;
	case NW_S:
		if(*ndir == S_N) {
		    *ndir = E_W;
		    trk.x = t->x - 1;
		    trk.y = t->y - 1;
		    break;
		}
		trk.x = t->x;
		trk.y = t->y + 1;
		*ndir = N_S;
		break;
	case NE_S:
		if(*ndir == S_N) {
		    *ndir = W_E;
		    trk.x = t->x + 1;
		    trk.y = t->y - 1;
		    break;
		}
		trk.x = t->x;
		trk.y = t->y + 1;
		*ndir = N_S;
		break;

	case SE_N:
		if(*ndir == N_S) {
		    trk.x = t->x + 1;
		    trk.y = t->y + 1;
		    *ndir = W_E;
		    break;
		}
		trk.y = t->y - 1;
		trk.x = t->x;
		*ndir = S_N;
		break;

	case TRK_N_S:
		walk_vertical(&trk, t, ndir);
		break;

        case X_X:
                break;

	default:
		*ndir = W_E;
	}
	return &trk;
}

Track	*track_walkwest(Track *t, trkdir *ndir)
{
	static	Track	trk;

	if(t->direction != TRK_N_S && t->wlinkx && t->wlinky) {
	    trk.x = t->wlinkx;
	    trk.y = t->wlinky;
	    return &trk;
	}
	trk.x = t->x - 1;
	trk.y = t->y;
	////20210605转线操作lrg
	//if(t->_pathDir != *ndir)
	//{
	//	*ndir = t->_pathDir;
	//}
	//if(t->direction ==TRK_N_S && (t->esignal!=0 || t->wsignal!=0))
	//{
	//trk.x = t->x-1;
	//trk.y = t->y-1;
	//}

	switch(t->direction) {
	case SW_N:
		if(*ndir == N_S) {
		    ++trk.y;
		    *ndir = E_W;
		    break;
		}
		*ndir = S_N;
	case SW_NE:
	case SW_E:
		++trk.y;
		break;
	case NW_S:
		if(*ndir == N_S) {
		    trk.x = t->x;
		    trk.y = t->y + 1;
		    break;
		}
		*ndir = E_W;
	case NW_SE:
	case NW_E:
		--trk.y;
		break;
	case NE_S:
		if(*ndir == S_N) {
		    trk.x = t->x + 1;
		    trk.y = t->y - 1;
		    *ndir = W_E;
		    break;
	        }
		*ndir = N_S;
		trk.y = t->y + 1;
		trk.x = t->x;
		break;
	case SE_N:
		if(*ndir == N_S) {
		    trk.x = t->x + 1;
		    trk.y = t->y + 1;
		    *ndir = W_E;
		    break;
		}
		*ndir = S_N;
		trk.x = t->x;
		trk.y = t->y - 1;
		break;
	case TRK_N_S:
		walk_vertical(&trk, t, ndir);
		break;

        case X_X:
                break;

	default:
		*ndir = E_W;
	}
	return &trk;
}

Track	*swtch_walkeast(Track *t, trkdir *ndir)
{
	static	Track	trk;

	trk.x = t->x;
	trk.y = t->y;
	switch(t->direction) {
	case 0:
		++trk.x;
		if(t->switched)
		    --trk.y;
		break;

	case 1:
	case 3:
	case 11:
		++trk.x;
		break;

	case 2:
		++trk.x;
		if(t->switched)
		    ++trk.y;
		break;

	case 4:
		++trk.x;
		if(!t->switched)
		    --trk.y;
		break;

	case 5:
		++trk.x;
		--trk.y;
		break;

	case 6:
		++trk.x;
		if(!t->switched)
		    ++trk.y;
		break;

	case 7:
		++trk.x;
		++trk.y;
		break;

	case 8:		    /* These are special cases handled in findPath() */
	case 9:
	case 16:
	case 17:
		break;

	case 10:
		++trk.x;
		if(t->switched)
		    ++trk.y;
		else
		    --trk.y;
		break;

	case 12:
	case 13:
	case 14:
	case 15:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
		walk_vertical_switch(&trk, t, ndir);
		break;

	}
	return &trk;
}

Track	*swtch_walkwest(Track *t, trkdir *ndir)
{
	static	Track	trk;

	trk.x = t->x;
	trk.y = t->y;
	switch(t->direction) {
	case 1:
		--trk.x;
		if(t->switched)
		    --trk.y;
		break;

	case 0:
	case 2:
	case 10:
		--trk.x;
		break;

	case 3:
		--trk.x;
		if(t->switched)
		    ++trk.y;
		break;

	case 4:
		--trk.x;
		++trk.y;
		break;

	case 5:
		--trk.x;
		if(!t->switched)
		    ++trk.y;
		break;

	case 7:
		--trk.x;
		if(!t->switched)
		    --trk.y;
		break;

	case 6:
		--trk.x;
		--trk.y;
		break;

	case 8:		    /* These are special cases handled in findPath() */
	case 9:
	case 16:
	case 17:
		break;

	case 11:
		--trk.x;
		if(t->switched)
		    ++trk.y;
		else
		    --trk.y;
		break;

	case 12:
	case 13:
	case 14:
	case 15:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
		walk_vertical_switch(&trk, t, ndir);
	}
	return &trk;
}

void	check_layout_errors(void)
{  
	Track	*t, *t1;
	wxChar	buff[512];
	int firsttime = 1;

	for(t = layout; t; t = t->next) {
	    buff[0] = 0;
	    if(t->type == TSIGNAL) {
		if(!t->controls)
		    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %d,%d %s.\n"), L("Signal at"), t->x, t->y, L("not linked to any track"));
		else switch(t->direction) {
		case E_W:
		case signal_WEST_FLEETED:
		case N_S:
		    if(!t->controls->wsignal)
			wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %d,%d - %s %d,%d.\n"), L("Track at"),
				t->controls->x, t->controls->y,
				L("not controlled by signal at"), t->x, t->y);
		    break;
		case W_E:
		case signal_EAST_FLEETED:
		case S_N:
		    if(!t->controls->esignal)
			wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %d,%d - %s %d,%d.\n"), L("Track at"),
				t->controls->x, t->controls->y,
				L("not controlled by signal at"), t->x, t->y);
		    break;
		}
	    }
	    if(t->type == TRACK || t->type == IMAGE) {
		if(t->wlinkx && t->wlinky) {
		    if(!(t1 = findTrack(t->wlinkx, t->wlinky)))
			wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %d,%d %s %d,%d.\n"),
			    L("Track at"), t->x, t->y,
			    L("linked to non-existant track at"), t->wlinkx, t->wlinky);
		    else if(!findTrack(t1->elinkx, t1->elinky) &&
			    !findTrack(t1->wlinkx, t1->wlinky))
			wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %d,%d %s %d,%d.\n"),
			    L("Track at"), t1->x, t1->y,
			    L("not linked back to"), t->x, t->y);
		} else if(t->elinkx && t->elinky) {
		    if(!(t1 = findTrack(t->elinkx, t->elinky)))
			wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %d,%d %s %d,%d.\n"),
			    L("Track at"), t->x, t->y,
			    L("linked to non-existant track at"), t->elinkx, t->elinky);
		    else if(!findTrack(t1->elinkx, t1->elinky) &&
			    !findTrack(t1->wlinkx, t1->wlinky))
			wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %d,%d %s %d,%d.\n"),
			    L("Track at"), t1->x, t1->y,
			    L("not linked back to"), t->x, t->y);
		}

	    }
	    if(t->type == SWITCH) {
		if(t->wlinkx && t->wlinky) {
		    if(!(t1 = findSwitch(t->wlinkx, t->wlinky)))
			wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %d,%d %s %d,%d.\n"),
			    L("Switch at"), t->x, t->y,
			    L("linked to non-existant switch at"), t->wlinkx, t->wlinky);
		    else if(t1->wlinkx != t->x || t1->wlinky != t->y)
			wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %d,%d %s %d,%d.\n"),
			    L("Switch at"), t1->x, t1->y,
			    L("not linked back to switch at"), t->x, t->y);
		}
	    }
	    if(buff[0]) {
		if(firsttime)
		    traindir->layout_error(L("Checking for errors in layout...\n"));
		firsttime = 0;
		traindir->layout_error(buff);
	    }
	}
	traindir->end_layout_error();
}

void	link_tracks(Track *t, Track *t1)
{
	switch(t->type) {
	case TRACK:
		if(t1->type != TRACK) {
		    traindir->Error(L("Only like tracks can be linked."));
		    return;
		}
		if(t1->direction != W_E && t1->direction != TRK_N_S) {
		    traindir->Error(L("Only horizontal or vertical tracks can be linked automatically.\nTo link other track types, use the track properties dialog."));
		    return;
		}
/*
		if(t->direction != t1->direction) {
		    error(wxT("You can't link horizontal to vertical tracks."));
		    return;
		}
*/		if(t->direction == TRK_N_S) {
		    if(!findTrack(t->x, t->y + 1)) {
			t->elinkx = t1->x;
			t->elinky = t1->y;
		    } else {
			t->wlinkx = t1->x;
			t->wlinky = t1->y;
		    }
		    if(!findTrack(t1->x , t1->y + 1)) {
			t1->elinkx = t->x;
			t1->elinky = t->y;
		    } else {
			t1->wlinkx = t->x;
			t1->wlinky = t->y;
		    }
		    break;
		}
		if(!findTrack(t->x + 1, t->y) &&
			!findSwitch(t->x + 1, t->y)) {
		    t->elinkx = t1->x;
		    t->elinky = t1->y;
		} else {
		    t->wlinkx = t1->x;
		    t->wlinky = t1->y;
		}
		if(!findTrack(t1->x - 1, t1->y) &&
			!findSwitch(t1->x - 1, t1->y)) {
		    t1->wlinkx = t->x;
		    t1->wlinky = t->y;
		} else {
		    t1->elinkx = t->x;
		    t1->elinky = t->y;
		}
		break;

	case SWITCH:
		if(t1->type != SWITCH) {
		    traindir->Error(L("Only like tracks can be linked."));
		    return;
		}
		t->wlinkx = t1->x;
		t->wlinky = t1->y;
		t1->wlinkx = t->x;
		t1->wlinky = t->y;
		break;

	case TSIGNAL:
		if(t1->type != TRACK) {
		    traindir->Error(L("Signals can only be linked to a track."));
		    return;
		}
		t->wlinkx = t1->x;
		t->wlinky = t1->y;
		t->controls = findTrack(t1->x, t1->y);
		break;

	case TRIGGER:
		if(t1->type != TRACK) {
		    traindir->Error(L("Triggers can only be linked to a track."));
		    return;
		}
		t->wlinkx = t1->x;
		t->wlinky = t1->y;
		t->controls = findTrack(t1->x, t1->y);
		break;

        case IMAGE:
                t->wlinkx = t1->x;
                t->wlinky = t1->y;
                t->controls = t1;  // t1 could be a signal or a switch
                break;

	case TEXT:
		if(t1->type != TRACK) {
		    traindir->Error(L("Entry/Exit points can only be linked to a track."));
		    return;
		}
#if 0
		if(t1->direction == TRK_N_S)
		{
		    if(t1->y < t->y) {
			t->elinkx = t1->x;
			t->elinky = t1->y;
		    } else {
			t->wlinkx = t1->x;
			t->wlinky = t1->y;
		    }
		}
#endif
		if(t1->x < t->x) {
		    t->wlinkx = t1->x;
		    t->wlinky = t1->y;
		} else {
		    t->elinkx = t1->x;
		    t->elinky = t1->y;
		}
		break;
	}
}

bool	isInside(const Coord& upleft, const Coord& downright, int x, int y)
{
	if(x >= upleft.x && x <= downright.x &&
	    y >= upleft.y && y <= downright.y)
	    return true;
	return false;
}

//	Move all track elements in the rectangle
//	comprised by (move_start,move_end) to
//	the coordinarte x,y (upper-left corner)

void	move_layout0(const Coord& start, const Coord& end, int x, int y)
{
	int	dx, dy;
	Track	*t, *t1;

	dx = x - start.x;
	dy = y - start.y;
	for(t = layout; t; t = t->next) {
	    x = t->x;
	    y = t->y;
	    if(isInside(start, end, x, y)) {
		if((t1 = find_track(layout, t->x + dx, t->y + dy)))
		    track_delete(t1);
		t->x += dx;
		t->y += dy;
	    }
	    if(t->elinkx && t->elinky &&
		isInside(start, end, t->elinkx, t->elinky)) {
		t->elinkx += dx;
		t->elinky += dy;
	    }
	    if(t->wlinkx && t->wlinky &&
		isInside(start, end, t->wlinkx, t->wlinky)) {
		t->wlinkx += dx;
		t->wlinky += dy;
	    }
	}

	//  I hope this is right :)

	Itinerary   *it;
	int	    n;

	for(it = itineraries; it; it = it->next) {
	    for(n = 0; n < it->nsects; ++n) {
		if(isInside(start, end, it->sw[n].x, it->sw[n].y)) {
		    it->sw[n].x += dx;
		    it->sw[n].y += dy;
		}
	    }
	}
}

void	move_layout(int x, int y)
{
	Coord	start, end;
	if(move_end.x < move_start.x) {
	    start.x = move_end.x;
	    end.x = move_start.x;
	} else {
	    start.x = move_start.x;
	    end.x = move_end.x;
	}
	if(move_end.y < move_start.y) {
	    start.y = move_end.y;
	    end.y = move_start.y;
	} else {
	    start.y = move_start.y;
	    end.y = move_end.y;
	}
	// avoid overlaps by moving the original tracks
	// to an area where there cannot be any other track
	move_layout0(start, end, start.x + 1000, start.y + 1000);
	// move back from the temporary area to the
	// final destination area.
	start.x += 1000;
	start.y += 1000;
	end.x += 1000;
	end.y += 1000;
	move_layout0(start, end, x, y);
	start.x -= 1000;
	start.y -= 1000;
	end.x -= 1000;
	end.y -= 1000;
        link_all_tracks(layout);
}

void	auto_link_track(Track *t)
{
	int	x, y;
	Track	*t1;

	x = t->x;
	y = t->y;
	if(link_to_left) {
	    switch(t->direction) {
	    case W_E:   --y;    break;
	    case E_W:   ++y;    break;
	    case N_S:   ++x;    break;
	    case S_N:   --x;    break;
	    }
	} else {
	    switch(t->direction) {
	    case W_E:   ++y;    break;
	    case E_W:   --y;    break;
	    case N_S:   --x;    break;
	    case S_N:   ++x;    break;
	    }
	}
	t1 = findTrack(x, y);
	if(t1 && t1->type == TRACK &&
		(t1->direction == W_E || t1->direction == TRK_N_S))
	    link_tracks(t, t1);
}

int	macro_select(void)
{
	Track	*t;
	Itinerary *nextItin, *itinList = NULL;	// +Rask Ingemann Lambertsen
	wxChar	buff[512];

	if(!macros) {
	    maxmacros = 1;
	    macros = (Track **)calloc(sizeof(Track *), maxmacros);
	}
	buff[0] = 0;
	if(!traindir->OpenMacroFileDialog(buff))
	    return 0;
	remove_ext(buff);
	if(!(t = load_field_tracks(buff, &itinList)))
	    return 0;
	if(current_macro_name)
	    free(current_macro_name);
	current_macro_name = wxStrdup(buff);
	clean_field(t);
	for(; itinList; itinList = nextItin) {	// +Rask Ingemann Lambertsen
	    nextItin = itinList->next;
	    free_itinerary(itinList);
	}
/*	if(macros[0])
	    clean_field(macros[0]);
	macros[0] = t;
	current_macro = 0;
	nmacros = 1;
	maxmacros = 1;
*/
	return 1;
}

// begin +Rask Ingemann Lambertsen
static void relocate_itinerary(Itinerary *it, int xbase, int ybase)
{
	int	i;

	for(i = 0; i < it->nsects; ++i) {
	    it->sw[i].x += xbase;
	    it->sw[i].y += ybase;
	}
}
// end +Rask Ingemann Lambertsen

void	macro_place(int xbase, int ybase)
{
	Track	*mp;
	Track	*t, *t1;
	int	x, y;
	int	oldtool;
	Itinerary *itn, *itinList = NULL;	// +Rask Ingemann Lambertsen

	if(!current_macro_name)
	    return;
	oldtool = current_tool;
	mp = load_field_tracks(current_macro_name, &itinList);
	while(mp) {
	    t1 = mp->next;
	    x = mp->x + xbase;
	    y = mp->y + ybase;
	    if((t = findTrack(x, y)) || (t = findSwitch(x, y)) ||
		(t = (Track *)findSignal(x, y)) || (t = findText(x, y)) ||
		(t = findPlatform(x, y)) || (t = findImage(x, y)) ||
		(t = findTrackType(x, y, ITIN)) ||
		(t = findTrackType(x, y, TRIGGER))) {
		track_delete(t);
	    }
	    mp->x = x;
	    mp->y = y;
	    if(mp->elinkx || mp->elinky) {
		mp->elinkx += xbase;
		mp->elinky += ybase;
	    }
	    if(mp->wlinkx || mp->wlinky) {
		mp->wlinkx += xbase;
		mp->wlinky += ybase;
	    }
	    mp->next = layout;
	    layout = mp;
	    link_all_tracks();
	    mp = t1;
	    layout_modified = 1;
	}
	// begin +Rask Ingemann Lambertsen
	/* Link in the itineraries from the macro.  Delete duplicates  */
	if(itinList) {
	    for(itn = itinList; itn->next; itn = itn->next) {
		relocate_itinerary(itn, xbase, ybase);
		delete_itinerary(itn->name);
	    }
	    relocate_itinerary(itn, xbase, ybase);
	    delete_itinerary(itn->name);
	    itn->next = itineraries;
	    itineraries = itinList;
	    layout_modified = 1;
	}
	// end+Rask Ingemann Lambertsen
	sort_itineraries();
	invalidate_field();
	repaint_all();
	current_tool = oldtool;
}

void	track_place(int x, int y)
{
	Track	*t, *t1;
	int	needall;

	if(current_tool >= 0 && tooltbl[current_tool].type == MACRO) {
	    if(!current_macro_name || tooltbl[current_tool].direction == 0) {
		select_tool(current_tool - 1);
		return;
	    }
	    macro_place(x, y);
	    return;
	}
	if(current_tool >= 0 && tooltbl[current_tool].type == MOVER) {
	    if(tooltbl[current_tool].direction == 0) {
		move_start.x = x;
		move_start.y = y;
		move_end.x = move_end.y = -1;
		select_tool(current_tool + 1);
		return;
	    }
	    if((short)move_start.x == -1) {
		select_tool(current_tool - 1);
		return;
	    }
	    if((short)move_end.x == -1) {
		move_end.x = x;
		move_end.y = y;
		select_tool(current_tool + 1);
		mover_draw();
		return;
	    }
#if 0
	    // avoid overlaps by moving the original tracks
	    // to an area where there cannot be any other track
	    move_layout(move_start.x + 1000, move_start.y + 1000);
	    // move back from the temporary area to the
	    // final destination area.
	    move_start.x += 1000;
	    move_start.y += 1000;
	    move_end.x += 1000;
	    move_end.y += 1000;
	    move_layout(x, y);
#else
	    move_layout(x, y);
#endif
	    layout_modified = 1;
	    invalidate_field();
	    repaint_all();
	    select_tool(current_tool - 2);
	    move_start.x = move_start.y = -1;
	    move_end.x = move_end.y = -1;
	    return;
	}
	if(current_tool >= 0 && tooltbl[current_tool].type == LINK) {
	    if(tooltbl[current_tool].direction == 0) {
		if(!findTrack(x, y) && !findSignal(x, y) &&
			!findSwitch(x, y) && !findText(x, y) &&
			!findTrackType(x, y, TRIGGER) &&
                        !findTrackType(x, y, IMAGE))
		    return;		/* there must be a track */
		link_start.x = x;
		link_start.y = y;
		select_tool(current_tool + 1);
		return;
	    }
	    if(link_start.x == -1) {
		select_tool(current_tool - 1);
		return;
	    }
	    if(!(t = findTrack(link_start.x, link_start.y)) &&
		    !(t = findSwitch(link_start.x, link_start.y)) &&
		    !(t = (Track *)findSignal(link_start.x, link_start.y)) &&
		    !(t = findText(link_start.x, link_start.y)) &&
		    !(t = findTrackType(link_start.x, link_start.y, TRIGGER)) &&
                    !(t = findTrackType(link_start.x, link_start.y, IMAGE))) {
		return;
	    }
	    if(!(t1 = findTrack(x, y)) && !(t1 = (Track *)findSignal(x, y)) &&
			!(t1 = findSwitch(x, y)) && !(t1 = findText(x, y))) {
		return;
	    }
	    if(t->type == TRIGGER && t1->type != TRACK)
		return;
            if(t->type == IMAGE && (t1->type != SWITCH && t1->type != TSIGNAL))
                return;
	    link_start.x = -1;
	    link_start.y = -1;
	    link_tracks(t, t1);
	    layout_modified = 1;
	    select_tool(current_tool - 1);
	    return;
	}
	needall = 0;
	if((t = findTrack(x, y)) || (t = findSwitch(x, y)) ||
	   (t = (Track *)findSignal(x, y)) || (t = findText(x, y)) ||
	   (t = findPlatform(x, y)) || (t = findImage(x, y)) ||
	   (t = findTrackType(x, y, ITIN)) ||
	   (t = findTrackType(x, y, TRIGGER))) {
	    needall = 1;
	    track_delete(t);
	    link_all_tracks();
	    layout_modified = 1;
	}
	if(current_tool == 0) {		/* delete element */
	    repaint_all();
	    return;
	}
	t = track_new();
	t->x = x;
	t->y = y;
	t->type = (trktype)tooltbl[current_tool].type;
	t->direction = (trkdir)tooltbl[current_tool].direction;
        t->power = gEditorMotivePower;
        t->gauge = editor_gauge._iValue;
	t->next = layout;
	if(t->type == TEXT)
	    t->station = wxStrdup(wxT("Abc"));
	else if(t->type == IMAGE)
	    t->direction = (trkdir)0;
	else if(t->type == TSIGNAL) {
	    if(t->direction & 2) {
		t->fleeted = 1;
		t->direction = (trkdir)((int)t->direction & (~2));
	    } else
		t->fleeted = 0;
	    if(auto_link)
		auto_link_track(t);
	} else if(t->type == TRIGGER && auto_link)
	    auto_link_track(t);
	layout = t;
	link_all_tracks();
	layout_modified = 1;
	if(needall || is_windows)
	    repaint_all();
	else
	    track_paint(t);
}

void	track_properties(int x, int y)
{
	Track	*t;
	Signal	*sig;
	wxChar	buff[1024];

	if((t = findImage(x, y))) {
	    buff[0] = 0;
	    if(t->station)
		wxStrcpy(buff, t->station);
	    if(!traindir->OpenImageDialog(buff))
		return;
	    remove_ext(buff);
	    wxStrcat(buff, wxT(".xpm"));
	    if(t->station)
		free(t->station);
	    t->pixels = 0;
	    t->station = wxStrdup(buff);
	    layout_modified = 1;
	    repaint_all();
	    return;
	}
	if((sig = findSignal(x,y)) && signal_properties_dialog) {
	    signal_properties_dialog(sig);
	    layout_modified = 1;
	    return;
	}

	if((t = findTrackType(x, y, TRIGGER)) && trigger_properties_dialog) {
	    trigger_properties_dialog(t);
	    layout_modified = 1;
	    return;
	}

        if((t = findSwitch(x, y))) {
            switch_properties_dialog(t);
            layout_modified = 1;
            return;
        }

	if((t = findTrack(x, y)) || (t = findText(x, y)) ||
			(t = (Track *)findSignal(x, y)) || /* (t = findImage(x, y)) || */
			(t = findTrackType(x, y, ITIN)) ||
//                      (t = findSwitch(x, t)) ||
			(t = findTrackType(x, y, TRIGGER))) {
	    track_properties_dialog(t);
	    layout_modified = 1;
            return;
	}
}

//
//	Scripting support
//


bool	Track::GetPropertyValue(const wxChar *prop, ExprValue& result)
{
	Track	*t = this;

	// move to Track::GetPropertyValue()
	if(!wxStrcmp(prop, wxT("length"))) {
	    result._op = Number;
	    result._val = t->length;
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%d}"), result._val);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("station"))) {
	    result._op = String;
	    result._txt = !t->station ? wxT("") : t->station;
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%s}"), result._txt);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("busy"))) {
	    result._op = Number;
	    result._val = (t->fgcolor != conf.fgcolor) || findTrain(t->x, t->y);
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%d}"), result._val);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("free"))) {
	    result._op = Number;
	    result._val = t->fgcolor == conf.fgcolor && !findTrain(t->x, t->y);
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%d}"), result._val);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("thrown"))) {
	    result._op = Number;
	    if(t->type == SWITCH) {
/*		switch(t->direction) {
		case 10:	// Y switches could be considered always set to a siding
		case 11:	// but it conflicts with the option of reading the status
		case 22:	// then throwing the switch, so this is not enabled.
		case 23:
		    result._val = 1;
		    break;

		default: */
		    result._val = t->switched;
		//}
	    } else
		result._val = 0;
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%d}"), result._val);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("color"))) {
	    result._op = String;
	    result._txt = GetColorName(t->fgcolor);
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%s}"), result._txt);
	    return true;
	}
        if(!wxStrcmp(prop, wxT("pathdir"))) {
	    result._op = Number;
	    result._val = t->_pathDir;
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%d}"), result._val);
	    return true;
        }
        if(!wxStrcmp(prop, wxT("linked"))) {
            int x, y;
            if(!(x = t->wlinkx) || !(y = t->wlinky)) {
                x = t->elinkx;
                y = t->elinky;
            }
            Track *lnk = findTrack(x, y);
            if(!lnk) {
                if(trace_script._iValue)
	            wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%d,%d} - not found"), x, y);
	        result._op = Number;
	        result._val = 0;
	        return false;
            }
            if(lnk->type == TSIGNAL) {
                result._signal = (Signal *)lnk;
                result._op = SignalRef;
            } else {
                result._track = lnk;
                result._op = TrackRef;
            }
            return true;
        }

	result._op = Number;
	result._val = 0;
	return false;
}

const Char *TrackBase::GetTypeName() const
{
        switch(type) {
        case TRACK:     return wxT("track");
        case SWITCH:    return wxT("switch");
        case PLATFORM:  return wxT("platform");
        case TSIGNAL:   return wxT("signal");
        case TEXT:      return wxT("text");
        case IMAGE:     return wxT("image");
        case ITIN:      return wxT("itin");
        case TRIGGER:   return wxT("trigger");
        }
        return wxT("element");
}

bool    TrackBase::ToggleDwelling()
{
        int     indx;

        _leftDwellTime = _dwellTime;
        if(_leftDwellTime <= 0)
            return false;                   // no dwell time associated with this track
        indx = dwellingTracks.Find(this);
        if (indx < 0)
            dwellingTracks.Add(this);       // start dwell time when switch is activated
        else {
            dwellingTracks.RemoveAt(indx);  // was activated but never started, so deactivate it
            _leftDwellTime = 0;
        }
        return true;
}

bool    TrackBase::IsDwelling() const
{
        if(!_dwellTime)     // not needed for this element
            return false;
        if(!_leftDwellTime) // never activated or already finished
            return false;
        if(_leftDwellTime == _dwellTime)    // activated but not started yet
            return false;
        return true;
}

bool	Track::SetPropertyValue(const wxChar *prop, ExprValue& val)
{
	if(!wxStrcmp(prop, wxT("thrown"))) {
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("=%d"), val._val);
	    if(type != SWITCH)
		return false;
	    switched = val._val != 0;
            change_coord(this->x, this->y);
            repaint_all();
	    return true;
	}
	if(!wxStrcmp(prop, wxT("click"))) {
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("=%d"), val._val);
	    track_selected(this->x, this->y);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("color"))) {
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("=%d"), val._val);
            grcolor col = conf.fgcolor;
            if(wxStrcmp(val._txt, wxT("blue")) == 0)
                col = color_blue;
            else if(wxStrcmp(val._txt, wxT("white")) == 0)
                col = color_white;
            else if(wxStrcmp(val._txt, wxT("red")) == 0)
                col = color_red;
			else if(wxStrcmp(val._txt, wxT("bise")) == 0)//lrg 20210215
                col = color_bise;
			else if(wxStrcmp(val._txt, wxT("approach")) == 0)//lrg 20210220
                col = color_approach;
            else if(wxStrcmp(val._txt, wxT("green")) == 0)
                col = color_green;
            else if(wxStrcmp(val._txt, wxT("orange")) == 0)
                col = color_orange;
            else if(wxStrcmp(val._txt, wxT("black")) == 0)
                col = color_black;
            else if(wxStrcmp(val._txt, wxT("cyan")) == 0)
                col = color_cyan;
	    SetColor(col);
	    return true;
	}
        if(!wxStrcmp(prop, wxT("icon"))) {
            if(this->type == IMAGE) {
                if(this->station) {
                    if(!wxStrcmp(this->station, val._txt) && 0)
                        return true;
                    free(this->station);
                }
                this->station = wxStrdup(val._txt);
                change_coord(this->x, this->y);
                repaint_all();
            }
            return true;
        }

	return false;
}


TrackBase::TrackBase()
{
	next = 0;
	next1 = 0;
	x = y = 0;
	xsize = ysize = 0;
	type = NOTRACK;
	direction = W_E;
	status = ST_FREE;
	wlinkx = wlinky = 0;
	elinkx = elinky = 0;
	isstation = 0;
	switched = 0;
	busy = 0;
	fleeted = 0;
	nowfleeted = 0;
	norect = 0;
	fixedred = 0;
	nopenalty = 0;
	noClickPenalty = 0;
	invisible = 0;
	wtrigger = 0;
	etrigger = 0;
	signalx = 0;
	aspect_changed = 0;
	flags = 0;		/* performance flags (TFLG_*) */
	station = 0;
	lock = 0;
        _lockedBy = 0;
	memset(speed, 0, sizeof(speed));
	icon = 0;
	length = 0;
	line_num = 0;
	wsignal = 0;		/* signal controlling this track */
	esignal = 0;		/* signal controlling this track */
	controls = 0;		/* track controlled by this signal */
	fgcolor = 0;
	pixels = 0;		/* for IMAGE pixmap */
	km = 0;			/* station distance (in meters) */
	stateProgram = 0;	/* 4.0: name of function describing state changes */
	_currentState = 0;	/* 4.0: name of current state in state program */
	_interpreterData = 0;	/* 4.0: intermediate data for program interpreter */
	_isFlashing = 0;	/* 4.0: flashing signal */
	_isShuntingSignal = 0;	/* 4.0: only affects shunting trains */
	_nextFlashingIcon = 0;	/* 4.0: index in list of icons when flashing */
	for(int i = 0; i < MAX_FLASHING_ICONS; ++i)
	    _flashingIcons[i] = 0;
	_fontIndex = 0;
        _intermediate = false;
        _nReservations = 0;
        power = 0;              // 3.9: motive power allowed (diesel, electric)
        gauge = 0;              // 3.9: track gauge
        _maxPathHint = 0;
        _shape = 0;
        _leftDwellTime = 0;     // 3.9k: waiting time until operation is complete
        _dwellTime = 0;         // 3.9k: how long it takes to operate this element
}

bool	TrackInterpreterData::Evaluate(ExprNode *n, ExprValue& result)
{
	Track	*t = 0;
	Signal	*sig = 0;
	ExprValue left(None);
	ExprValue right(None);
	const wxChar	*prop;

	if(!n)
	    return false;
        switch(n->_op) {

        case LinkedRef:

            t = this->_track;
            if(!t->wlinkx || !t->wlinky)
                return false;
            result._track = findSwitch(t->x, t->y);
            if(!result._track)
                return false;
            result._op = SwitchRef;
            return true;

	case Dot:

	    result._op = Addr;
	    if(!(n->_left)) {
		result._track = this->_track;
		result._op = TrackRef;
	    } else if(n->_left && n->_left->_op == Dot) {
		bool oldForAddr = _forAddr;
		_forAddr = true;
		if(!Evaluate(n->_left, result)) {	// <signal>.<property>
		    _forAddr = oldForAddr;
		    return false;
		}
		_forAddr = oldForAddr;

		if(result._op == TrackRef || result._op == SwitchRef)
		    TraceCoord(result._track->x, result._track->y);
                else if(result._op == SignalRef) {
    		    TraceCoord(result._signal->x, result._signal->y);
                    goto not_track;
                } else
		    return false;
	    } else {
		if(!Evaluate(n->_left, result))
		    return false;

		if(result._op == TrainRef)
		    goto not_track;
		if(result._op == SignalRef)
		    goto not_track;
		if(result._op != TrackRef && result._op != SwitchRef)
		    return false;
	    }
	    if(!result._track) {
                if(trace_script._iValue)
		    wxStrcat(expr_buff, wxT("no current track for '.'"));
		return false;
	    }
	    t = result._track;
	    TraceCoord(t->x, t->y);

not_track:
	    if(n->_right) {
		switch(n->_right->_op) {
		case LinkedRef:
                    if(!t) {
                        return false;
                    }
                    result._signal = 0;
                    result._track = findTrack(t->wlinkx, t->wlinky);
                    if(!result._track) {
                        result._track = findSwitch(t->wlinkx, t->wlinky);
                        if(result._track)
                            result._op = SwitchRef;
                        else {
                            result._signal = findSignal(t->wlinkx, t->wlinky);
                            if(result._signal)
                                result._op = SignalRef;
                            else {
                                result._track = findImage(t->wlinkx, t->wlinky);
                                if(result._track)
                                    result._op = TrackRef;
                                else {
                                    result._track = findTrackType(t->wlinkx, t->wlinky, ITIN);
                                    if(result._track) // do signal instead?
                                        result._op = TrackRef;
                                }
                            }
                        }
                    } else
                        result._op = TrackRef;
                    if(result._track) {
		        TraceCoord(result._track->x, result._track->y);
		        break;
                    }
                    if(result._signal) {
		        TraceCoord(result._signal->x, result._signal->y);
		        break;
                    }
                    if(trace_script._iValue)
		        wxStrcat(expr_buff, wxT("no linked track for '.'"));
		    return false;
		}
	    }
	    result._txt = (n->_right && n->_right->_op == String) ? n->_right->_txt : n->_txt;
	    if(_forAddr)
		return true;

	    prop = result._txt;
	    if(!prop)
		return false;

            if(trace_script._iValue)
	        wxStrcat(expr_buff, prop);
	    switch(result._op) {

	    case SwitchRef:

		if(!wxStrcmp(prop, wxT("thrown"))) {
		    result._op = Number;
		    result._val = t->switched;
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

	case Equal:

	    result._op = Number;
	    result._val = 0;
	    //if(_forCond)
		return InterpreterData::Evaluate(n, result);
	    //return false;

	default:

	    return InterpreterData::Evaluate(n, result);
	}

	return false;
}


void	Track::OnInit()
{
	TrackInterpreterData *interp = (TrackInterpreterData *)_interpreterData;
	if(!interp)
	    return;
	if(!interp->_onInit)
	    return;
	interp->_track = this;
	interp->_train  = 0;
	interp->_signal = 0;
	interp->_scopes.Clear();
	interp->TraceCoord(x, y, wxT("Track::OnInit"));
	Trace(expr_buff);
	interp->Execute(interp->_onInit);
	return;
}

void	Track::OnRestart()
{
	TrackInterpreterData *interp = (TrackInterpreterData *)_interpreterData;
	if(!interp)
	    return;
	if(!interp->_onRestart)
	    return;
	interp->_track = this;
	interp->_train  = 0;
	interp->_signal = 0;
	interp->_scopes.Clear();
	interp->TraceCoord(x, y, wxT("Track::OnRestart"));
	Trace(expr_buff);
	interp->Execute(interp->_onRestart);
	return;
}

void	Track::OnSetBusy()
{
	TrackInterpreterData *interp = (TrackInterpreterData *)_interpreterData;
	if(!interp)
	    return;
	if(!interp->_onSetBusy)
	    return;
	interp->_track = this;
	interp->_train  = 0;
	interp->_signal = 0;
        interp->_scopes.Clear();
	interp->TraceCoord(x, y, wxT("Track::OnSetBusy"));
	Trace(expr_buff);
	interp->Execute(interp->_onSetBusy);
	return;
}

void	Track::OnSetFree()
{
	TrackInterpreterData *interp = (TrackInterpreterData *)_interpreterData;
	if(!interp)
	    return;
	if(!interp->_onSetFree)
	    return;
	interp->_track = this;
	interp->_train  = 0;
	interp->_signal = 0;
        interp->_scopes.Clear();
	interp->TraceCoord(x, y, wxT("Track::OnSetFree"));
	Trace(expr_buff);
	interp->Execute(interp->_onSetFree);
	return;
}

void	Track::OnEnter(Train *trn)
{
	TrainStop *stp; 
	stp = trn->stops;
	TrackInterpreterData *interp = (TrackInterpreterData *)_interpreterData;
	if(!interp)
	    return;
	if(!interp->_onEnter)
	    return;
	interp->_track = this;
	interp->_train = trn;
        interp->_scopes.Clear();
	interp->_signal = 0;
	interp->TraceCoord(x, y, wxT("Track::OnEnter"));
	Trace(expr_buff);
	interp->Execute(interp->_onEnter);
	return;
}

void	Track::OnExit(Train *trn)
{
	TrackInterpreterData *interp = (TrackInterpreterData *)_interpreterData;
	if(!interp)
	    return;
	if(!interp->_onExit)
	    return;
	interp->_track = this;
	interp->_train = trn;
        interp->_scopes.Clear();
	interp->_signal = 0;
	interp->TraceCoord(x, y, wxT("Track::OnExit"));
	Trace(expr_buff);
	interp->Execute(interp->_onExit);
	return;
}

void	Track::OnClicked()
{
	TrackInterpreterData *interp = (TrackInterpreterData *)_interpreterData;
	if(!interp)
	    return;
	if(!interp->_onClicked)
	    return;
	interp->_track = this;
	interp->_train = 0;
	interp->_signal = 0;
	interp->_scopes.Clear();
	interp->TraceCoord(x, y, wxT("Track::OnClicked"));
	Trace(expr_buff);
	interp->Execute(interp->_onClicked);
	return;
}

void	Track::OnCanceled()
{
	if(this->type != ITIN)
	    return;
	if(_interpreterData) {
	    TrackInterpreterData& interp = *(TrackInterpreterData *)_interpreterData;
	    if(interp._onCanceled) {
		interp._track = this;
		Itinerary *it;
		for(it = itineraries; it; it = it->next)
		    if(!wxStrcmp(it->name, this->station)) {
			interp._itinerary = it;
			break;
		    }
                if(trace_script._iValue)
		    wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("Track::OnCanceled(%s)"), this->station);
		Trace(expr_buff);
		interp.Execute(interp._onCanceled);
		return;
	    }
	}
}

void	Track::OnCrossed(Train *trn)
{
	TrackInterpreterData *interp = (TrackInterpreterData *)_interpreterData;
	if(!interp)
	    return;
	if(!interp->_onCrossed)
	    return;
	interp->_track = this;
	interp->_train = trn;
	interp->_signal = 0;
        interp->_scopes.Clear();
	interp->TraceCoord(x, y, wxT("Track::OnCrossed"));
	Trace(expr_buff);
	interp->Execute(interp->_onCrossed);
	return;
}

void	Track::OnArrived(Train *trn)
{
	TrackInterpreterData *interp = (TrackInterpreterData *)_interpreterData;
	if(!interp)
	    return;
	if(!interp->_onArrived)
	    return;
	interp->_track = this;
	interp->_train = trn;
	interp->_signal = 0;
        interp->_scopes.Clear();
	interp->TraceCoord(x, y, wxT("Track::OnArrived"));
	Trace(expr_buff);
	interp->Execute(interp->_onArrived);
	return;
}

void	Track::OnStopped(Train *trn)
{
	TrackInterpreterData *interp = (TrackInterpreterData *)_interpreterData;
	if(!interp)
	    return;
	if(!interp->_onStopped)
	    return;
	interp->_track = this;
	interp->_train = trn;
	interp->_signal = 0;
        interp->_scopes.Clear();
	interp->TraceCoord(x, y, wxT("Track::OnStopped"));
	Trace(expr_buff);
	interp->Execute(interp->_onStopped);
	return;
}

void	Track::OnIconUpdate()
{
	TrackInterpreterData *interp = (TrackInterpreterData *)_interpreterData;
	if(!interp)
	    return;
	if(!interp->_onIconUpdate)
	    return;
	interp->_track = this;
	interp->_train = 0;
	interp->_signal = 0;
        interp->_scopes.Clear();
	interp->TraceCoord(x, y, wxT("Track::OnIconUpdate"));
	Trace(expr_buff);
	interp->Execute(interp->_onIconUpdate);
	return;
}

void	Track::ParseProgram()
{
	const wxChar	*p;

	if(!this->stateProgram || !*this->stateProgram)
	    return;
	if(_interpreterData)	    // previous script
	    delete _interpreterData;
	_interpreterData = new TrackInterpreterData;

	TrackInterpreterData *interp = (TrackInterpreterData *)_interpreterData;
        wxSprintf(eventObject, wxT("(%d,%d) "), this->x, this->y);
	p = this->stateProgram;
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
	    } else if(match(&p, eventName = wxT("OnSetBusy:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onSetBusy = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnSetFree:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onSetFree = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnEnter:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onEnter = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnCrossed:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onCrossed = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnArrived:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onArrived = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnStopped:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onStopped = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnExit:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onExit = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnClicked:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onClicked = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnIconUpdate:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onIconUpdate = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnCanceled:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onCanceled = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("procedure ")) || match(&p, eventName = wxT("proc ")) ||
                      match(&p, eventName = wxT("procedure\t")) || match(&p, eventName = wxT("proc\t"))) {
                p = parse_proc(p);
	    }
            eventName = 0;
            if(p1 == p)	{    // error! couldn't parse token
                if(*p) {
                    parserError(wxT("Unknown event name"), p);
                }
		break;
            }
	}
        eventObject[0] = 0;
}

void	Track::RunScript(const Char *script, Train *trn)
{
	Script	*s = find_script(script);
	if(!s) {
	    s = new_script(script);
	    // return;
	}
	if(!s->ReadFile())
	    return;

	// is it different from current one?
	if(!this->stateProgram || wxStrcmp(s->_text, this->stateProgram)) {
	    if(this->stateProgram)
		free(this->stateProgram);
	    this->stateProgram = wxStrdup(s->_text);
	    ParseProgram();
	}
	OnEnter(trn);
}


void	Track::SetColor(grcolor color)
{
	if(this->fgcolor == color)
	    return;
	this->fgcolor = color;//在这一步实现绿色改为占用颜色以及将占用改为绿色
	change_coord(this->x, this->y);
	if((color == conf.fgcolor)||(color == color_bise))  //lrg 20210216
	    OnSetFree();
	else
	    OnSetBusy();
}


bool	Track::IsBusy() const
{
	if((this->fgcolor != conf.fgcolor)&&(this->fgcolor != color_bise))//lrg  20210216
	    return true;
	return false;
}


//int check_stright(Itinerary  *it)
//{
// for(int i=0;i<20;i++)
//	  {
//	  if(wxStrncmp(it_name, , 1))
//	    { it->Select(false);
//	      return 0;
//	    }
//	  }
//     return 1;
//}