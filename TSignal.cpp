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

#ifdef WIN32
#define strcasecmp stricmp
#endif

#include "Traindir3.h"
#include "itracks.h"
#include "itools.h"
#include "isignals.h"
#include "iswitch.h"
#include "iactions.h"
#include "imovers.h"
#include "TDFile.h"

int	pathIsBusy(Train *tr, Vector *path, int dir);
Vector	*findPath(Track *t, int dir);
Track	*findNextTrack(trkdir direction, int x, int y);
const Char *GetColorName(int color);
void	setBackgroundColor(wxColour& col);
extern  const Char    *eventName;   // for script diagnostic
extern  Char    eventObject[];      // for script diagnostic
extern  void    parserError(const Char *msg, const Char *ptr);

BoolOption trace_script(wxT("traceScript"), wxT("Show scripts lines as they are executed"), wxT("Preferences"), false);

bool	gMustBeClearPath;

extern  ShapeManager    shapeManager;

extern	int	signals_changed;

static  Shape   *n_sig_pmap[2];         /* R, G */
static	Shape	*n_sigx_pmap[2];
static const char *n_sig_xpm[] = {
"7 9 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"X      c #0000FFFFFFFF",*/
"       ",
"  ...  ",
" .GGG. ",
" .GGG. ",
" .GGG. ",
"  ...  ",
"   .   ",
"   .   ",
" ..... ",
0
};
static const char *n_sigx_xpm[] = {
"7 9 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"X      c #0000FFFFFFFF",*/
"       ",
" ..... ",
" .GGG. ",
" .GGG. ",
" .GGG. ",
" ..... ",
"   .   ",
"   .   ",
" ..... ",
0
};

static  Shape   *s_sig_pmap[2];         /* R, G */
static  Shape   *s_sigx_pmap[2];
static const char *s_sig_xpm[] = {
"7 9 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"X      c #0000FFFFFFFF",*/
"       ",
" ..... ",
"   .   ",
"   .   ",
"  ...  ",
" .GGG. ",
" .GGG. ",
" .GGG. ",
"  ...  ",
0
};
static const char *s_sigx_xpm[] = {
"7 9 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"X      c #0000FFFFFFFF",*/
"       ",
" ..... ",
"   .   ",
"   .   ",
" ..... ",
" .GGG. ",
" .GGG. ",
" .GGG. ",
" ..... ",
0
};

static  Shape   *n_sig2_pmap[4];         /* R, G */
static  Shape   *n_sig2x_pmap[4];
static const char *n_sig2_xpm[] = {
"7 13 4 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"G      c #0000FFFFFFFF",*/
NULL, /*"X      c #0000FFFFFFFF",*/
"  ...  ",
" .GGG. ",
" .GGG. ",
" .GGG. ",
"  ...  ",
"  ...  ",
" .XXX. ",
" .XXX. ",
" .XXX. ",
"  ...  ",
"   .   ",
"   .   ",
" ..... ",
0
};
static const char *n_sig2x_xpm[] = {
"7 13 4 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"G      c #0000FFFFFFFF",*/
NULL, /*"X      c #0000FFFFFFFF",*/
" ..... ",
" .GGG. ",
" .GGG. ",
" .GGG. ",
" ..... ",
" ..... ",
" .XXX. ",
" .XXX. ",
" .XXX. ",
" ..... ",
"   .   ",
"   .   ",
" ..... ",
0
};

static  Shape   *s_sig2_pmap[4];         /* R, G */
static  Shape   *s_sig2x_pmap[4];        /* R, G */
static const char *s_sig2_xpm[] = {
"7 13 4 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"G      c #0000FFFFFFFF",*/
NULL, /*"X      c #0000FFFFFFFF",*/
" ..... ",
"   .   ",
"   .   ",
"  ...  ",
" .XXX. ",
" .XXX. ",
" .XXX. ",
"  ...  ",
"  ...  ",
" .GGG. ",
" .GGG. ",
" .GGG. ",
"  ...  ",
0
};
static const char *s_sig2x_xpm[] = {
"7 13 4 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"G      c #0000FFFFFFFF",*/
NULL, /*"X      c #0000FFFFFFFF",*/
" ..... ",
"   .   ",
"   .   ",
" ..... ",
" .XXX. ",
" .XXX. ",
" .XXX. ",
" ..... ",
" ..... ",
" .GGG. ",
" .GGG. ",
" .GGG. ",
" ..... ",
0
};

static	Shape   *e_sig_pmap[2];		/* R, G */
static	Shape   *e_sigx_pmap[2];
static const char *e_sig_xpm[] = {
"9 7 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"X      c #0000FFFFFFFF",*/
"         ",
"         ",
".    ... ",
".   .GGG.",
".....GGG.",
".   .GGG.",
".    ... ",
0
};
static const char *e_sigx_xpm[] = {
"9 7 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"X      c #0000FFFFFFFF",*/
"         ",
"         ",
".   .....",
".   .GGG.",
".....GGG.",
".   .GGG.",
".   .....",
0
};

static	Shape   *w_sig_pmap[2];		/* R, G */
static	Shape   *w_sigx_pmap[2];
static const char *w_sig_xpm[] = {
"9 7 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"X      c #0000FFFFFFFF",*/
"         ",
"         ",
" ...    .",
".GGG.   .",
".GGG.....",
".GGG.   .",
" ...    .",
0
};
static const char *w_sigx_xpm[] = {
"9 7 3 1",
"       c #FFFFFFFFFFFF",
".      c #000000000000",
NULL, /*"X      c #0000FFFFFFFF",*/
"         ",
"         ",
".....   .",
".GGG.   .",
".GGG.....",
".GGG.   .",
".....   .",
0
};

static	Shape   *e_sig2_pmap[4];		/* RR, GR, GG, GO */
static	Shape   *e_sig2x_pmap[4];
static const char *e_sig2_xpm[] = {
	"13 7 4 1",
	"       c #FFFFFFFFFFFF",
	".      c #000000000000",
	NULL, /*"G      c #0000FFFFFFFF",*/
	NULL, /*"X      c #0000FFFFFFFF",*/
	"             ",
	"             ",
	".   ...  ... ",
	".  .XXX..GGG.",
	"....XXX..GGG.",
	".  .XXX..GGG.",
	".   ...  ... ",
        0
};
static const char *e_sig2x_xpm[] = {
	"13 7 4 1",
	"       c #FFFFFFFFFFFF",
	".      c #000000000000",
	NULL, /*"G      c #0000FFFFFFFF",*/
	NULL, /*"X      c #0000FFFFFFFF",*/
	"             ",
	"             ",
	".  ..........",
	".  .XXX..GGG.",
	"....XXX..GGG.",
	".  .XXX..GGG.",
	".  ..........",
        0
};

static	Shape   *e_sigP_pmap[4];		/* RR, GR, GG, GO */
static const char *e_sigP_xpm[] = {
	"13 7 4 1",
	"       c #FFFFFFFFFFFF",
	".      c #000000000000",
	NULL, /*"G      c #0000FFFFFFFF",*/
	NULL, /*"X      c #0000FFFFFFFF",*/
	"             ",
	"             ",
        ". ...... ... ",
        ". XXXXX..GGG.",
        "....X.X..GGG.",
        ". ..XXX..GGG.",
        ". ...... ... ",
        0
};


static	Shape   *w_sig2_pmap[4];		/* RR, GR, GG, GO */
static	Shape   *w_sig2x_pmap[4];
static const char *w_sig2_xpm[] = {
	"13 7 4 1",
	"       c #FFFFFFFFFFFF",
	".      c #000000000000",
	NULL, /*"G      c #0000FFFFFFFF",*/
	NULL, /*"X      c #0000FFFFFFFF",*/
	"             ",
	"             ",
	" ...  ...   .",
	".GGG..XXX.  .",
	".GGG..XXX....",
	".GGG..XXX.  .",
	" ...  ...   .",
        0
};
static const char *w_sig2x_xpm[] = {
	"13 7 4 1",
	"       c #FFFFFFFFFFFF",
	".      c #000000000000",
	NULL, /*"G      c #0000FFFFFFFF",*/
	NULL, /*"X      c #0000FFFFFFFF",*/
	"             ",
	"             ",
	"..........  .",
	".GGG..XXX.  .",
	".GGG..XXX....",
	".GGG..XXX.  .",
	"..........  .",
        0
};

static	Shape   *w_sigP_pmap[4];		/* RR, GR, GG, GO */
static const char *w_sigP_xpm[] = {
	"13 7 4 1",
	"       c #FFFFFFFFFFFF",
	".      c #000000000000",
	NULL, /*"G      c #0000FFFFFFFF",*/
	NULL, /*"X      c #0000FFFFFFFF",*/
	"             ",
	"             ",
	" ... ...... .",
	".GGG..XXX.. .",
	".GGG..X.X....",
	".GGG..XXXXX .",
	" ... ...... .",
        0
};

// static
void	Signal::InitPixmaps()
{
	char	bufffg[64];
	char	buff[64];
	int	fgr, fgg, fgb;
	int	r, g, b;
	const char *green_name = "G      c #0000d8000000";

	getcolor_rgb(fieldcolors[COL_TRACK], &fgr, &fgg, &fgb);
        wxColour bgColor;
        setBackgroundColor(bgColor);
        r = bgColor.Red();
        g = bgColor.Green();
        b = bgColor.Blue();

	sprintf(bufffg, ".      c #%02x00%02x00%02x00", fgr, fgg, fgb);
	sprintf(buff, "       c #%02x00%02x00%02x00", r, g, b);
        n_sig_xpm[1] = s_sig_xpm[1] = e_sig_xpm[1] = w_sig_xpm[1] = 
	n_sigx_xpm[1] = s_sigx_xpm[1] = e_sigx_xpm[1] = w_sigx_xpm[1] = buff;

        n_sig_xpm[2] = s_sig_xpm[2] = e_sig_xpm[2] = w_sig_xpm[2] =
	n_sigx_xpm[2] = s_sigx_xpm[2] = e_sigx_xpm[2] = w_sigx_xpm[2] = bufffg;
        n_sig_xpm[3] = s_sig_xpm[3] = e_sig_xpm[3] = w_sig_xpm[3] =
	n_sigx_xpm[3] = s_sigx_xpm[3] = e_sigx_xpm[3] = w_sigx_xpm[3] = "G      c red";

#if 0
        n_sig_pmap[0] = get_pixmap(n_sig_xpm);
        s_sig_pmap[0] = get_pixmap(s_sig_xpm);
        e_sig_pmap[0] = get_pixmap(e_sig_xpm);
        w_sig_pmap[0] = get_pixmap(w_sig_xpm);
        n_sigx_pmap[0] = get_pixmap(n_sigx_xpm);
        s_sigx_pmap[0] = get_pixmap(s_sigx_xpm);
        e_sigx_pmap[0] = get_pixmap(e_sigx_xpm);
        w_sigx_pmap[0] = get_pixmap(w_sigx_xpm);

        n_sig_xpm[3] = s_sig_xpm[3] = e_sig_xpm[3] = w_sig_xpm[3] =
	n_sigx_xpm[3] = s_sigx_xpm[3] = e_sigx_xpm[3] = w_sigx_xpm[3] = green_name;
        n_sig_pmap[1] = get_pixmap(n_sig_xpm);
        s_sig_pmap[1] = get_pixmap(s_sig_xpm);
        e_sig_pmap[1] = get_pixmap(e_sig_xpm);
        w_sig_pmap[1] = get_pixmap(w_sig_xpm);
        n_sigx_pmap[1] = get_pixmap(n_sigx_xpm);
        s_sigx_pmap[1] = get_pixmap(s_sigx_xpm);
        e_sigx_pmap[1] = get_pixmap(e_sigx_xpm);
        w_sigx_pmap[1] = get_pixmap(w_sigx_xpm);

	e_sig2_xpm[1] = w_sig2_xpm[1] =
		n_sig2_xpm[1] = s_sig2_xpm[1] =
		e_sig2x_xpm[1] = w_sig2x_xpm[1] =
		n_sig2x_xpm[1] = s_sig2x_xpm[1] =
		e_sigP_xpm[1] = w_sigP_xpm[1] = buff;
	e_sig2_xpm[2] = w_sig2_xpm[2] =
		n_sig2_xpm[2] = s_sig2_xpm[2] =
		e_sig2x_xpm[2] = w_sig2x_xpm[2] =
		n_sig2x_xpm[2] = s_sig2x_xpm[2] =
		e_sigP_xpm[2] = w_sigP_xpm[2] = bufffg;
	e_sig2_xpm[3] = w_sig2_xpm[3] =
		n_sig2_xpm[3] = s_sig2_xpm[3] =
		e_sig2x_xpm[3] = w_sig2x_xpm[3] =
		n_sig2x_xpm[3] = s_sig2x_xpm[3] =
		e_sigP_xpm[3] = w_sigP_xpm[3] = "G      c red";
	e_sig2_xpm[4] = w_sig2_xpm[4] =
		n_sig2_xpm[4] = s_sig2_xpm[4] =
		e_sig2x_xpm[4] = w_sig2x_xpm[4] =
		n_sig2x_xpm[4] = s_sig2x_xpm[4] = "X      c red";
	e_sigP_xpm[4] = w_sigP_xpm[4] = "X      c gray";
	e_sig2_pmap[0] = get_pixmap(e_sig2_xpm);
	w_sig2_pmap[0] = get_pixmap(w_sig2_xpm);
	e_sigP_pmap[0] = get_pixmap(e_sigP_xpm);
	w_sigP_pmap[0] = get_pixmap(w_sigP_xpm);
	n_sig2_pmap[0] = get_pixmap(n_sig2_xpm);
	s_sig2_pmap[0] = get_pixmap(s_sig2_xpm);
	e_sig2x_pmap[0] = get_pixmap(e_sig2x_xpm);
	w_sig2x_pmap[0] = get_pixmap(w_sig2x_xpm);
	n_sig2x_pmap[0] = get_pixmap(n_sig2x_xpm);
	s_sig2x_pmap[0] = get_pixmap(s_sig2x_xpm);

	e_sig2_xpm[3] = w_sig2_xpm[3] =
	n_sig2_xpm[3] = s_sig2_xpm[3] =
	e_sig2x_xpm[3] = w_sig2x_xpm[3] =
	n_sig2x_xpm[3] = s_sig2x_xpm[3] = green_name;
	e_sigP_xpm[3] = w_sigP_xpm[3] = green_name;
	e_sig2_xpm[4] = w_sig2_xpm[4] =
	n_sig2_xpm[4] = s_sig2_xpm[4] =
	e_sig2x_xpm[4] = w_sig2x_xpm[4] =
	n_sig2x_xpm[4] = s_sig2x_xpm[4] = "X      c red";
	e_sigP_xpm[4] = w_sigP_xpm[4] = "X      c gray";
	e_sig2_pmap[1] = get_pixmap(e_sig2_xpm);
	w_sig2_pmap[1] = get_pixmap(w_sig2_xpm);
	e_sigP_pmap[1] = get_pixmap(e_sigP_xpm);
	w_sigP_pmap[1] = get_pixmap(w_sigP_xpm);
	n_sig2_pmap[1] = get_pixmap(n_sig2_xpm);
	s_sig2_pmap[1] = get_pixmap(s_sig2_xpm);
	e_sig2x_pmap[1] = get_pixmap(e_sig2x_xpm);
	w_sig2x_pmap[1] = get_pixmap(w_sig2x_xpm);
	n_sig2x_pmap[1] = get_pixmap(n_sig2x_xpm);
	s_sig2x_pmap[1] = get_pixmap(s_sig2x_xpm);

	e_sig2_xpm[3] = w_sig2_xpm[3] =
	n_sig2_xpm[3] = s_sig2_xpm[3] =
	e_sig2x_xpm[3] = w_sig2x_xpm[3] =
	n_sig2x_xpm[3] = s_sig2x_xpm[3] = green_name;
	e_sig2_xpm[4] = w_sig2_xpm[4] =
	n_sig2_xpm[4] = s_sig2_xpm[4] =
	e_sig2x_xpm[4] = w_sig2x_xpm[4] =
	n_sig2x_xpm[4] = s_sig2x_xpm[4] = "X      c #0000d8000000";
	e_sigP_xpm[4] = w_sigP_xpm[4] = "X      c white";
	e_sig2_pmap[2] = get_pixmap(e_sig2_xpm);
	w_sig2_pmap[2] = get_pixmap(w_sig2_xpm);
	e_sigP_pmap[2] = get_pixmap(e_sigP_xpm);
	w_sigP_pmap[2] = get_pixmap(w_sigP_xpm);
	n_sig2_pmap[2] = get_pixmap(n_sig2_xpm);
	s_sig2_pmap[2] = get_pixmap(s_sig2_xpm);
	e_sig2x_pmap[2] = get_pixmap(e_sig2x_xpm);
	w_sig2x_pmap[2] = get_pixmap(w_sig2x_xpm);
	n_sig2x_pmap[2] = get_pixmap(n_sig2x_xpm);
	s_sig2x_pmap[2] = get_pixmap(s_sig2x_xpm);

	e_sig2_xpm[3] = w_sig2_xpm[3] =
	n_sig2_xpm[3] = s_sig2_xpm[3] =
	e_sig2x_xpm[3] = w_sig2x_xpm[3] =
	n_sig2x_xpm[3] = s_sig2x_xpm[3] = "G      c red";
	e_sigP_xpm[3] = w_sigP_xpm[3] = "G      c red";
	e_sig2_xpm[4] = w_sig2_xpm[4] =
	n_sig2_xpm[4] = s_sig2_xpm[4] =
	e_sig2x_xpm[4] = w_sig2x_xpm[4] =
	n_sig2x_xpm[4] = s_sig2x_xpm[4] = "X      c orange";
	e_sigP_xpm[4] = w_sigP_xpm[4] = "X      c white";
	e_sig2_pmap[3] = get_pixmap(e_sig2_xpm);
	w_sig2_pmap[3] = get_pixmap(w_sig2_xpm);
	e_sigP_pmap[3] = get_pixmap(e_sigP_xpm);
	w_sigP_pmap[3] = get_pixmap(w_sigP_xpm);
	/*n_sig2_pmap[3] = get_pixmap(n_sig2_xpm);
	s_sig2_pmap[3] = get_pixmap(s_sig2_xpm);*/
	e_sig2x_pmap[3] = get_pixmap(e_sig2x_xpm);
	w_sig2x_pmap[3] = get_pixmap(w_sig2x_xpm);
#else
        n_sig_pmap[0] = shapeManager.NewShape(wxT("sigRn"), n_sig_xpm);
        s_sig_pmap[0] = shapeManager.NewShape(wxT("sigRs"), s_sig_xpm);
        e_sig_pmap[0] = shapeManager.NewShape(wxT("sigRe"), e_sig_xpm);
        w_sig_pmap[0] = shapeManager.NewShape(wxT("sigRw"), w_sig_xpm);
        n_sigx_pmap[0] = shapeManager.NewShape(wxT("sigRxn"), n_sigx_xpm);
        s_sigx_pmap[0] = shapeManager.NewShape(wxT("sigRxs"), s_sigx_xpm);
        e_sigx_pmap[0] = shapeManager.NewShape(wxT("sigRxe"), e_sigx_xpm);
        w_sigx_pmap[0] = shapeManager.NewShape(wxT("sigRxw"), w_sigx_xpm);

        n_sig_xpm[3] = s_sig_xpm[3] = e_sig_xpm[3] = w_sig_xpm[3] =
	n_sigx_xpm[3] = s_sigx_xpm[3] = e_sigx_xpm[3] = w_sigx_xpm[3] = green_name;

        n_sig_pmap[1] = shapeManager.NewShape(wxT("sigGn"), n_sig_xpm);
        s_sig_pmap[1] = shapeManager.NewShape(wxT("sigGs"), s_sig_xpm);
        e_sig_pmap[1] = shapeManager.NewShape(wxT("sigGe"), e_sig_xpm);
        w_sig_pmap[1] = shapeManager.NewShape(wxT("sigGw"), w_sig_xpm);
        n_sigx_pmap[1] = shapeManager.NewShape(wxT("sigGxn"), n_sigx_xpm);
        s_sigx_pmap[1] = shapeManager.NewShape(wxT("sigGxs"), s_sigx_xpm);
        e_sigx_pmap[1] = shapeManager.NewShape(wxT("sigGxe"), e_sigx_xpm);
        w_sigx_pmap[1] = shapeManager.NewShape(wxT("sigGxw"), w_sigx_xpm);

	e_sig2_xpm[1] = w_sig2_xpm[1] =
		n_sig2_xpm[1] = s_sig2_xpm[1] =
		e_sig2x_xpm[1] = w_sig2x_xpm[1] =
		n_sig2x_xpm[1] = s_sig2x_xpm[1] =
		e_sigP_xpm[1] = w_sigP_xpm[1] = buff;
	e_sig2_xpm[2] = w_sig2_xpm[2] =
		n_sig2_xpm[2] = s_sig2_xpm[2] =
		e_sig2x_xpm[2] = w_sig2x_xpm[2] =
		n_sig2x_xpm[2] = s_sig2x_xpm[2] =
		e_sigP_xpm[2] = w_sigP_xpm[2] = bufffg;
	e_sig2_xpm[3] = w_sig2_xpm[3] =
		n_sig2_xpm[3] = s_sig2_xpm[3] =
		e_sig2x_xpm[3] = w_sig2x_xpm[3] =
		n_sig2x_xpm[3] = s_sig2x_xpm[3] =
		e_sigP_xpm[3] = w_sigP_xpm[3] = "G      c red";
	e_sig2_xpm[4] = w_sig2_xpm[4] =
		n_sig2_xpm[4] = s_sig2_xpm[4] =
		e_sig2x_xpm[4] = w_sig2x_xpm[4] =
		n_sig2x_xpm[4] = s_sig2x_xpm[4] = "X      c red";
	e_sigP_xpm[4] = w_sigP_xpm[4] = "X      c gray";

        n_sig2_pmap[0] = shapeManager.NewShape(wxT("sigRRn"), n_sig2_xpm);
        s_sig2_pmap[0] = shapeManager.NewShape(wxT("sigRRs"), s_sig2_xpm);
        e_sig2_pmap[0] = shapeManager.NewShape(wxT("sigRRe"), e_sig2_xpm);
        w_sig2_pmap[0] = shapeManager.NewShape(wxT("sigRRw"), w_sig2_xpm);
        n_sig2x_pmap[0] = shapeManager.NewShape(wxT("sigRRxn"), n_sig2x_xpm);
        s_sig2x_pmap[0] = shapeManager.NewShape(wxT("sigRRxs"), s_sig2x_xpm);
        e_sig2x_pmap[0] = shapeManager.NewShape(wxT("sigRRxe"), e_sig2x_xpm);
        w_sig2x_pmap[0] = shapeManager.NewShape(wxT("sigRRxw"), w_sig2x_xpm);
        e_sigP_pmap[0] = shapeManager.NewShape(wxT("sigRRPe"), e_sigP_xpm);
        w_sigP_pmap[0] = shapeManager.NewShape(wxT("sigRRPw"), w_sigP_xpm);
#if 0
	e_sig2_pmap[0] = get_pixmap(e_sig2_xpm);
	w_sig2_pmap[0] = get_pixmap(w_sig2_xpm);
	e_sigP_pmap[0] = get_pixmap(e_sigP_xpm);
	w_sigP_pmap[0] = get_pixmap(w_sigP_xpm);
	n_sig2_pmap[0] = get_pixmap(n_sig2_xpm);
	s_sig2_pmap[0] = get_pixmap(s_sig2_xpm);
	e_sig2x_pmap[0] = get_pixmap(e_sig2x_xpm);
	w_sig2x_pmap[0] = get_pixmap(w_sig2x_xpm);
	n_sig2x_pmap[0] = get_pixmap(n_sig2x_xpm);
	s_sig2x_pmap[0] = get_pixmap(s_sig2x_xpm);
#endif
	e_sig2_xpm[3] = w_sig2_xpm[3] =
	n_sig2_xpm[3] = s_sig2_xpm[3] =
	e_sig2x_xpm[3] = w_sig2x_xpm[3] =
	n_sig2x_xpm[3] = s_sig2x_xpm[3] = green_name;
	e_sigP_xpm[3] = w_sigP_xpm[3] = green_name;
	e_sig2_xpm[4] = w_sig2_xpm[4] =
	n_sig2_xpm[4] = s_sig2_xpm[4] =
	e_sig2x_xpm[4] = w_sig2x_xpm[4] =
	n_sig2x_xpm[4] = s_sig2x_xpm[4] = "X      c red";
	e_sigP_xpm[4] = w_sigP_xpm[4] = "X      c gray";

        n_sig2_pmap[1] = shapeManager.NewShape(wxT("sigGRn"), n_sig2_xpm);
        s_sig2_pmap[1] = shapeManager.NewShape(wxT("sigGRs"), s_sig2_xpm);
        e_sig2_pmap[1] = shapeManager.NewShape(wxT("sigGRe"), e_sig2_xpm);
        w_sig2_pmap[1] = shapeManager.NewShape(wxT("sigGRw"), w_sig2_xpm);
        n_sig2x_pmap[1] = shapeManager.NewShape(wxT("sigGRxn"), n_sig2x_xpm);
        s_sig2x_pmap[1] = shapeManager.NewShape(wxT("sigGRxs"), s_sig2x_xpm);
        e_sig2x_pmap[1] = shapeManager.NewShape(wxT("sigGRxe"), e_sig2x_xpm);
        w_sig2x_pmap[1] = shapeManager.NewShape(wxT("sigGRxw"), w_sig2x_xpm);
        e_sigP_pmap[1] = shapeManager.NewShape(wxT("sigGRPe"), e_sigP_xpm);
        w_sigP_pmap[1] = shapeManager.NewShape(wxT("sigGRPw"), w_sigP_xpm);

#if 0
        e_sig2_pmap[1] = get_pixmap(e_sig2_xpm);
	w_sig2_pmap[1] = get_pixmap(w_sig2_xpm);
	e_sigP_pmap[1] = get_pixmap(e_sigP_xpm);
	w_sigP_pmap[1] = get_pixmap(w_sigP_xpm);
	n_sig2_pmap[1] = get_pixmap(n_sig2_xpm);
	s_sig2_pmap[1] = get_pixmap(s_sig2_xpm);
	e_sig2x_pmap[1] = get_pixmap(e_sig2x_xpm);
	w_sig2x_pmap[1] = get_pixmap(w_sig2x_xpm);
	n_sig2x_pmap[1] = get_pixmap(n_sig2x_xpm);
	s_sig2x_pmap[1] = get_pixmap(s_sig2x_xpm);
#endif

	e_sig2_xpm[3] = w_sig2_xpm[3] =
	n_sig2_xpm[3] = s_sig2_xpm[3] =
	e_sig2x_xpm[3] = w_sig2x_xpm[3] =
	n_sig2x_xpm[3] = s_sig2x_xpm[3] = green_name;
	e_sig2_xpm[4] = w_sig2_xpm[4] =
	n_sig2_xpm[4] = s_sig2_xpm[4] =
	e_sig2x_xpm[4] = w_sig2x_xpm[4] =
	n_sig2x_xpm[4] = s_sig2x_xpm[4] = "X      c #0000d8000000";
	e_sigP_xpm[4] = w_sigP_xpm[4] = "X      c white";

        n_sig2_pmap[2] = shapeManager.NewShape(wxT("sigGOn"), n_sig2_xpm);
        s_sig2_pmap[2] = shapeManager.NewShape(wxT("sigGOs"), s_sig2_xpm);
        e_sig2_pmap[2] = shapeManager.NewShape(wxT("sigGOe"), e_sig2_xpm);
        w_sig2_pmap[2] = shapeManager.NewShape(wxT("sigGOw"), w_sig2_xpm);
        n_sig2x_pmap[2] = shapeManager.NewShape(wxT("sigGOxn"), n_sig2x_xpm);
        s_sig2x_pmap[2] = shapeManager.NewShape(wxT("sigGOxs"), s_sig2x_xpm);
        e_sig2x_pmap[2] = shapeManager.NewShape(wxT("sigGOxe"), e_sig2x_xpm);
        w_sig2x_pmap[2] = shapeManager.NewShape(wxT("sigGOxw"), w_sig2x_xpm);
        e_sigP_pmap[2] = shapeManager.NewShape(wxT("sigGOPe"), e_sigP_xpm);
        w_sigP_pmap[2] = shapeManager.NewShape(wxT("sigGOPw"), w_sigP_xpm);

#if 0
	e_sig2_pmap[2] = get_pixmap(e_sig2_xpm);
	w_sig2_pmap[2] = get_pixmap(w_sig2_xpm);
	e_sigP_pmap[2] = get_pixmap(e_sigP_xpm);
	w_sigP_pmap[2] = get_pixmap(w_sigP_xpm);
	n_sig2_pmap[2] = get_pixmap(n_sig2_xpm);
	s_sig2_pmap[2] = get_pixmap(s_sig2_xpm);
	e_sig2x_pmap[2] = get_pixmap(e_sig2x_xpm);
	w_sig2x_pmap[2] = get_pixmap(w_sig2x_xpm);
	n_sig2x_pmap[2] = get_pixmap(n_sig2x_xpm);
	s_sig2x_pmap[2] = get_pixmap(s_sig2x_xpm);
#endif

	e_sig2_xpm[3] = w_sig2_xpm[3] =
	n_sig2_xpm[3] = s_sig2_xpm[3] =
	e_sig2x_xpm[3] = w_sig2x_xpm[3] =
	n_sig2x_xpm[3] = s_sig2x_xpm[3] = "G      c red";
	e_sigP_xpm[3] = w_sigP_xpm[3] = "G      c red";
	e_sig2_xpm[4] = w_sig2_xpm[4] =
	n_sig2_xpm[4] = s_sig2_xpm[4] =
	e_sig2x_xpm[4] = w_sig2x_xpm[4] =
	n_sig2x_xpm[4] = s_sig2x_xpm[4] = "X      c orange";
	e_sigP_xpm[4] = w_sigP_xpm[4] = "X      c white";

        n_sig2_pmap[3] = shapeManager.NewShape(wxT("sigROn"), n_sig2_xpm);
        s_sig2_pmap[3] = shapeManager.NewShape(wxT("sigROs"), s_sig2_xpm);
        e_sig2_pmap[3] = shapeManager.NewShape(wxT("sigROe"), e_sig2_xpm);
        w_sig2_pmap[3] = shapeManager.NewShape(wxT("sigROw"), w_sig2_xpm);
        n_sig2x_pmap[3] = shapeManager.NewShape(wxT("sigROxn"), n_sig2x_xpm);
        s_sig2x_pmap[3] = shapeManager.NewShape(wxT("sigROxs"), s_sig2x_xpm);
        e_sig2x_pmap[3] = shapeManager.NewShape(wxT("sigROxe"), e_sig2x_xpm);
        w_sig2x_pmap[3] = shapeManager.NewShape(wxT("sigROxw"), w_sig2x_xpm);
        e_sigP_pmap[3] = shapeManager.NewShape(wxT("sigROPe"), e_sigP_xpm);
        w_sigP_pmap[3] = shapeManager.NewShape(wxT("sigROPw"), w_sigP_xpm);

#if 0
        e_sig2_pmap[3] = get_pixmap(e_sig2_xpm);
	w_sig2_pmap[3] = get_pixmap(w_sig2_xpm);
	e_sigP_pmap[3] = get_pixmap(e_sigP_xpm);
	w_sigP_pmap[3] = get_pixmap(w_sigP_xpm);
	/*n_sig2_pmap[3] = get_pixmap(n_sig2_xpm);
	s_sig2_pmap[3] = get_pixmap(s_sig2_xpm);*/
	e_sig2x_pmap[3] = get_pixmap(e_sig2x_xpm);
	w_sig2x_pmap[3] = get_pixmap(w_sig2x_xpm);
#endif
#endif
}

// static
void	Signal::FreePixmaps()
{
#if 0
	int	i;
	for(i = 0; i < 4; ++i) {
	    delete_pixmap(e_sig2_pmap[i]);
	    delete_pixmap(w_sig2_pmap[i]);
	    delete_pixmap(e_sigP_pmap[i]);
	    delete_pixmap(w_sigP_pmap[i]);
	    delete_pixmap(n_sig2_pmap[i]);
	    delete_pixmap(s_sig2_pmap[i]);
	    delete_pixmap(e_sig2x_pmap[i]);
	    delete_pixmap(w_sig2x_pmap[i]);
	    delete_pixmap(n_sig2x_pmap[i]);
	    delete_pixmap(s_sig2x_pmap[i]);
	}
	for(i = 0; i < 2; ++i) {
	    delete_pixmap(n_sig_pmap[i]);
	    delete_pixmap(s_sig_pmap[i]);
	    delete_pixmap(e_sig_pmap[i]);
	    delete_pixmap(w_sig_pmap[i]);
	    delete_pixmap(n_sigx_pmap[i]);
	    delete_pixmap(s_sigx_pmap[i]);
	    delete_pixmap(e_sigx_pmap[i]);
	    delete_pixmap(w_sigx_pmap[i]);
	}
#else
#endif
}

Shape   *Signal::FindShapeBuiltin()
{
	int	i;
	void	*p = 0;
        Shape   *shp = 0;
	Signal	*t = this;

	i = 0;					/* RR */
        if(t->fleeted) {
            if(t->status == ST_GREEN) {
                if(t->nowfleeted)
	            i = 2;			/* GG */
                else
	            i = 1;			/* GR */
            } else if(t->nowfleeted)
                i = 3;			        /* RO */
            switch(t->direction) {
            case W_E:
                shp = signal_traditional ?
	            (t->signalx ? e_sig2x_pmap[i] : e_sig2_pmap[i]) : e_sigP_pmap[i];
                break;
            case E_W:
                shp = signal_traditional ?
	            (t->signalx ? w_sig2x_pmap[i] : w_sig2_pmap[i]) : w_sigP_pmap[i];
                break;
            case N_S:
                shp = t->signalx ? s_sig2x_pmap[i] : s_sig2_pmap[i];
                break;
            case S_N:
                shp = t->signalx ? n_sig2x_pmap[i] : n_sig2_pmap[i];
                break;
            }
            return shp;
        }
        if(t->status == ST_GREEN)
            i = 1;
        switch(t->direction) {
        case W_E:
            shp = t->signalx ? e_sigx_pmap[i] : e_sig_pmap[i];
            break;
        case E_W:
            shp = t->signalx ? w_sigx_pmap[i] : w_sig_pmap[i];
            break;
        case N_S:
            shp = t->signalx ? s_sigx_pmap[i] : s_sig_pmap[i];
            break;
        case S_N:
            shp = t->signalx ? n_sigx_pmap[i] : n_sig_pmap[i];
            break;
        }
        return shp;
}

void	Signal::Draw()
{
        Shape   *shp;
	void	*p = 0;
	Signal	*t = this;

	if(!t->_interpreterData || !(p = FindIcon())) {
            shp = FindShapeBuiltin();
            if(!shp)
                return;
            p = shp->GetImage();
	}
	if(p)
	    draw_pixmap(t->x, t->y, p);
	if(editing && show_links && t->controls)
	    draw_link(t->x, t->y, t->controls->x, t->controls->y, conf.linkcolor);
}


SignalAspect::SignalAspect()
{
	_next = 0;
	_name = 0;
	_action = wxStrdup(wxT("none"));
	memset(_iconN, 0, sizeof(_iconN));
	memset(_iconE, 0, sizeof(_iconE));
	memset(_iconS, 0, sizeof(_iconS));
	memset(_iconW, 0, sizeof(_iconW));
}


SignalAspect::~SignalAspect()
{
	int	    i;

	for(i = 0; i < MAX_FLASHING_ICONS; ++i) {
	    if(_iconN[i])
		free(_iconN[i]);
	    if(_iconE[i])
		free(_iconE[i]);
	    if(_iconS[i])
		free(_iconS[i]);
	    if(_iconW[i])
		free(_iconW[i]);
	}
	if(_name)
	    free(_name);
}


void	Signal::ParseAspect(const wxChar **pp)
{
	wxChar	line[1024];
	const wxChar	*p = *pp;
	wxChar	**dst;
	SignalAspect *asp = new SignalAspect();
	SignalInterpreterData *interp = (SignalInterpreterData *)_interpreterData;

	p = scan_line(p, line);
	if(line[0])
	    asp->_name = wxStrdup(line);
	do {
	    dst = 0;
	    if(match(&p, wxT("IconN:")))
		dst = &asp->_iconN[0];
	    else if(match(&p, wxT("IconE:")))
		dst = &asp->_iconE[0];
	    else if(match(&p, wxT("IconS:")))
		dst = &asp->_iconS[0];
	    else if(match(&p, wxT("IconW:")))
		dst = &asp->_iconW[0];
	    if(dst) {
		p = scan_line(p, line);
		if(line[0]) {
		    if(wxStrchr(line, ' ')) {
			this->_isFlashing = true;
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
	    break;
	    // unknown. Should we give an error?
	} while(1);
	asp->_next = interp->_aspects;
	interp->_aspects = asp;
	*pp = p;
}


void	Signal::ParseProgram()
{
	const wxChar	*p;

	if(!this->stateProgram || !*this->stateProgram)
	    return;
	Script	*s = find_script(this->stateProgram);
	SignalInterpreterData *interp;
	if(!s) {
	    s = new_script(this->stateProgram);
	    // return;
	}
	if(!s->ReadFile())
	    return;

	if(!_interpreterData)
	    _interpreterData = new SignalInterpreterData;

        wxSprintf(eventObject, wxT("%s"), s->_path);
	interp = (SignalInterpreterData *)_interpreterData;
	p = s->_text;
	while(*p) {
	    const wxChar	*p1 = p;
	    while(*p1 == ' ' || *p1 == '\t' || *p1 == '\r' || *p1 == '\n')
		++p1;
	    p = p1;
	    if(match(&p, wxT("Aspect:"))) {
		p1 = p;
		ParseAspect(&p);
	    } else if(match(&p, eventName = wxT("OnClick:"))) {
		p1 = p;
		interp->_onClick = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnCleared:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onCleared = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnUncleared:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onUncleared = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnShunt:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onShunt = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnInit:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onInit = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnRestart:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onRestart = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnUpdate:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onUpdate = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnAuto:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onAuto = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("OnCross:"))) {
		p = next_token(p);
		p1 = p;
		interp->_onCross = ParseStatements(&p);
	    } else if(match(&p, eventName = wxT("procedure ")) || match(&p, eventName = wxT("proc ")) ||
                      match(&p, eventName = wxT("procedure\t")) || match(&p, eventName = wxT("proc\t"))) {
                p1 = parse_proc(p);
	    }
            eventName = 0;
            if(p1 == p)	{    // error! couldn't parse token
                if(*p)
                    parserError(wxT("Unknown event name"), p);
		break;
            }
	}
        eventObject[0] = 0;
}

Signal	*Signal::GetNextSignal()
{
	Signal	*sig = this;

	Track	*t;
	trkdir	dir;
	Vector	*path;

	if(!sig->controls)
	    return 0;

	path = findPath(sig->controls, sig->direction);
	if(!path)
	    return 0;
	t = path->TrackAt(path->_size - 1);
	dir = (trkdir)path->FlagAt(path->_size - 1);
	Vector_delete(path);
	Track *t1;
	if(!(t1 = findNextTrack(dir, t->x, t->y))) {
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff),
		    wxT("no next track after (%d,%d)"), t->x, t->y);
	    return 0;
	}
	t = t1;
	sig = (Signal *)((dir == W_E || dir == S_N) ? t->esignal : t->wsignal);
	if(!sig) {
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff),
		    wxT("no signal after (%d,%d)"), t->x, t->y);
	    return 0;
	}
        if(trace_script._iValue)
	    wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff),
	        wxT("(%d,%d)"), sig->x, sig->y);
	return sig;
}

bool	Signal::GetNextPath(Vector **ppath)
{
	Signal	*s = this;

	*ppath = 0;
	if(s->fixedred) {
            if(trace_script._iValue)
	        wxStrncat(expr_buff, wxT("signal is always red"), sizeof(expr_buff)-1);
	    return true;
	}
	if(s->controls)
	    *ppath = findPath(s->controls, s->direction);
	if(!*ppath) {
            if(trace_script._iValue)
	        wxStrncat(expr_buff, wxT("no valid path"), sizeof(expr_buff)-1);
	    return false;
	}
	if(gMustBeClearPath) {
//	    if(!s->IsClear()) { // t->status == ST_GREEN) {
//		Vector_delete(path);
//		return true;
//	    }
	    if(pathIsBusy(NULL, *ppath, s->direction)) {
		Vector_delete(*ppath);
		*ppath = 0;
                if(trace_script._iValue)
		    wxStrncat(expr_buff, wxT("path is busy"), sizeof(expr_buff)-1);
		return true;
	    }
	}
	return true;
}

//
//	Execution of the Abstract Syntax Tree
//

wxChar	expr_buff[EXPR_BUFF_SIZE];

void	Trace(const wxChar *msg)
{
	if(!trace_script._iValue)
	    return;
	traindir->AddAlert(msg);
}

void    TraceIntResult(int val)
{
	if(!trace_script._iValue)
	    return;
        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%d}"), val);
}

void    TraceStringResult(const Char *val)
{
	if(!trace_script._iValue)
	    return;
        wxSnprintf(expr_buff + wxStrlen(expr_buff), sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%s}"), val);
}

Signal	*SignalInterpreterData::GetNextSignal(Signal *sig)
{
	return sig->GetNextSignal();
}

bool	SignalInterpreterData::GetNextPath(Signal *sig, Vector **ppath)
{
	gMustBeClearPath = _mustBeClearPath;
	bool res = sig->GetNextPath(ppath);
	gMustBeClearPath = false;
	return res;
}

bool	SignalInterpreterData::Evaluate(ExprNode *n, ExprValue& result)
{
	ExprValue left(None);
	ExprValue right(None);
	const wxChar	*prop;
	Signal	*sig;

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
                    if(trace_script._iValue)
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
                        if(trace_script._iValue)
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
	return false;
}


Shape   *Signal::FindShape()
{
	SignalInterpreterData *interp = (SignalInterpreterData *)_interpreterData;
        if(!interp) {
            return FindShapeBuiltin();
        }
	SignalAspect	*asp = interp->_aspects;
	wxChar		**p = 0;
	const wxChar	*curState;

	if(this->_currentState)
	    curState = this->_currentState;
	else if(this->status == ST_GREEN)
	    curState = wxT("green");
	else
	    curState = wxT("red");

	while(asp) {
	    if(!wxStricmp(asp->_name, curState))
		break;
	    asp = asp->_next;
	}
	if(!asp)
	    return 0;
	switch(this->direction) {
	case W_E:
		p = asp->_iconE;
		break;

	case E_W:
		p = asp->_iconW;
		break;

	case N_S:
		p = asp->_iconS;
		break;

	case S_N:
		p = asp->_iconN;
		break;
	}
	if(!p || !*p)
	    return 0;
	if(_isFlashing) {
	    if(!p[_nextFlashingIcon])
		_nextFlashingIcon = 0;
	    p = &p[_nextFlashingIcon];
	}
        return shapeManager.FindShape(*p);
}

void    *Signal::FindIcon()
{
        Shape *shape = FindShape();
        if(!shape)
            return 0;
        return shape->GetImage();
}




bool	Signal::IsClear()
{
	if(this->_currentState) {
	    if(_isShuntingSignal) {
		return wxStrcmp(this->_currentState, wxT("yellow")) != 0;
	    }
	    return wxStrcmp(GetAction(), wxT("stop")) != 0;	// !Rask
	}
	return this->status == ST_GREEN;
}


void	Signal::OnClear()
{
	signals_changed = 1;
	if(_interpreterData) {
	    SignalInterpreterData interp((SignalInterpreterData *)_interpreterData);
	    if(interp._onCleared) {
		interp._signal = this;
		interp._mustBeClearPath = true;
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnClear(%d,%d)"), this->stateProgram, this->x, this->y);
		Trace(expr_buff);
		interp.Execute(interp._onCleared);
		return;
	    }
	}

	this->status = ST_GREEN;
	_currentState = wxT("green");
	_nextFlashingIcon = 0;	    // in case new aspect is not flashing
}


void	Signal::OnUnclear()
{
	signals_changed = 1;
	if(_interpreterData) {
	    SignalInterpreterData interp((SignalInterpreterData *)_interpreterData);
	    if(interp._onUncleared) {
		interp._signal = this;
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnUnclear(%d,%d)"), this->stateProgram, this->x, this->y);
		Trace(expr_buff);
		interp.Execute(interp._onUncleared);
		return;
	    }
	}

	this->status = ST_RED;
	SetAspect(wxT("red")); // _currentState = wxT("red");
	_nextFlashingIcon = 0;	    // in case new aspect is not flashing
}


void	Signal::OnShunt()
{
	signals_changed = 1;
	if(_interpreterData) {
	    SignalInterpreterData interp((SignalInterpreterData *)_interpreterData);
	    if(interp._onShunt) {
		interp._signal = this;
		interp._mustBeClearPath = false;
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnShunt(%d,%d)"), this->stateProgram, this->x, this->y);
		Trace(expr_buff);
		interp.Execute(interp._onShunt);
		return;
	    }
	}

	this->status = ST_WHITE;
	_currentState = wxT("white");
	_nextFlashingIcon = 0;	    // in case new aspect is not flashing
}



void	Signal::OnCross()
{
//	if(this->aspect_changed)
//	    return;

	if(_interpreterData) {
	    SignalInterpreterData interp((SignalInterpreterData *)_interpreterData);
	    if(interp._onCross) {
		interp._signal = this;
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnCross(%d,%d)"), this->stateProgram, this->x, this->y);
		Trace(expr_buff);
		interp.Execute(interp._onCross);
		return;
	    }
	}
	signals_changed = 1;
	this->status = ST_RED;
	SetAspect(wxT("red"));
	_nextFlashingIcon = 0;	    // in case new aspect is not flashing
        if(_intermediate) {
            if(_nReservations < 2) {
                _nReservations = 0;
                nowfleeted = 0;
                fleeted = 0;
            } else
                --_nReservations;
        }
}


void	Signal::OnUnlock()
{
	signals_changed = 1;
	this->status = ST_GREEN;
}


void	Signal::OnUnfleet()
{
	signals_changed = 1;
	this->status = ST_GREEN;
}

void	Signal::OnUpdate()
{
//	if(this->aspect_changed)
//	    return;

	if(!_interpreterData)
	    return;

	SignalInterpreterData interp((SignalInterpreterData *)_interpreterData);
	if(!interp._onUpdate)
	    return;

	interp._signal = this;
	wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnUpdate(%d,%d)"), this->stateProgram, this->x, this->y);
	Trace(expr_buff);
	interp.Execute(interp._onUpdate);
}

void	Signal::OnInit()
{
	if(!_interpreterData)
	    return;
	SignalInterpreterData interp((SignalInterpreterData *)_interpreterData);
	if(!interp._onInit)
	    return;
	interp._signal = this;
	interp._mustBeClearPath = true;
	wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnInit(%d,%d)"), this->stateProgram, this->x, this->y);
	Trace(expr_buff);
	interp.Execute(interp._onInit);
	return;
}

void	Signal::OnRestart()
{
	if(!_interpreterData)
	    return;
	SignalInterpreterData interp((SignalInterpreterData *)_interpreterData);
	if(!interp._onRestart)
	    return;
	interp._signal = this;
	interp._mustBeClearPath = true;
	wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnRestart(%d,%d)"), this->stateProgram, this->x, this->y);
	Trace(expr_buff);
	interp.Execute(interp._onRestart);
	return;
}

void	Signal::OnAuto()
{
	if(!_interpreterData)
	    return;

	SignalInterpreterData interp((SignalInterpreterData *)_interpreterData);
	if(!interp._onAuto)
	    return;

	interp._signal = this;
	interp._mustBeClearPath = true;
	wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnAuto(%d,%d)"), this->stateProgram, this->x, this->y);
	Trace(expr_buff);
	interp.Execute(interp._onAuto);
}


void	Signal::OnClicked()
{
	if(!_interpreterData)
	    return;

	SignalInterpreterData interp((SignalInterpreterData *)_interpreterData);
	if(!interp._onClick)
	    return;

	interp._signal = this;
	interp._mustBeClearPath = true;
	wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("%s::OnClicked(%d,%d)"), this->stateProgram, this->x, this->y);
	Trace(expr_buff);
	interp.Execute(interp._onClick);
}


void	Signal::OnFlash()
{
	SignalAspect *asp;

	if(!_interpreterData)
	    return;
	if(!_currentState)
	    return;

	SignalInterpreterData interp((SignalInterpreterData *)_interpreterData);
	wxChar	    **p;

	for(asp = interp._aspects; asp; asp = asp->_next)
	    if(!wxStrcmp(_currentState, asp->_name)) {
		int	nxt = _nextFlashingIcon + 1;

		if(nxt >= MAX_FLASHING_ICONS)
		    nxt = 0;
		p = 0;
		switch(this->direction) {
		case W_E:
			p = asp->_iconE;
			break;

		case E_W:
			p = asp->_iconW;
			break;

		case N_S:
			p = asp->_iconS;
			break;

		case S_N:
			p = asp->_iconN;
			break;
		}
		if(!p || ! p[nxt])
		    nxt = 0;
		_nextFlashingIcon = nxt;
    		change_coord(this->x, this->y);
		break;
	    }
}

static  void    RegisterIcons(Char **p)
{
        int     i;

        for(i = 0; p[i]; ++i) {
            shapeManager.LoadXpm(p[i]);
        }
}

void    Signal::RegisterAspectsIcons()
{
	SignalAspect *asp;

	if(!_interpreterData)
	    return;

	SignalInterpreterData interp((SignalInterpreterData *)_interpreterData);

        for(asp = interp._aspects; asp; asp = asp->_next) {
            RegisterIcons(asp->_iconE);
            RegisterIcons(asp->_iconN);
            RegisterIcons(asp->_iconW);
            RegisterIcons(asp->_iconS);
        }
}


void stop_here()
{
}

void	Signal::SetAspect(const wxChar *aspect)
{
	if(!_currentState || wxStrcmp(_currentState, aspect)) {
	    signals_changed = 1;
    	    change_coord(this->x, this->y);
	    this->aspect_changed = 1;
	}

	_currentState = aspect;
	_nextFlashingIcon = 0;	    // in case new aspect is not flashing
}


const wxChar *Signal::GetAspect()
{
	if(_currentState)
	    return _currentState;
	if(this->status == ST_RED)
	    return wxT("red");
	return wxT("green");
}

const wxChar	*Signal::GetAction()
{
	const wxChar	*name = GetAspect();
 	SignalInterpreterData *interp = (SignalInterpreterData *)_interpreterData;
	SignalAspect *asp;

	if(interp) { 
	    for(asp = interp->_aspects; asp; asp = asp->_next) {
	        if(!wxStrcmp(name, asp->_name) && asp->_action)
		    return asp->_action;
	    }
        }
        if(!wxStrcmp(name, wxT("red")))
	    return wxT("stop");
	return wxT("proceed");	    // broken signal? maybe we should stop.
}

int	Signal::GetNAspects() const
{
	int n = 0;
	SignalInterpreterData *interp = (SignalInterpreterData *)_interpreterData;
	SignalAspect *asp;

	if(!interp) {
	    if(this->fixedred)
		return 1;	    // only "stop" possible
	    return 2;		    // only "stop" and "proceed"
	}
	for(asp = interp->_aspects; asp; asp = asp->_next)
	    ++n;
	return n;
}

const wxChar *Signal::GetAspect(int index)
{
	int n = 0;
	SignalInterpreterData *interp = (SignalInterpreterData *)_interpreterData;
	SignalAspect *asp;

	if(!interp) {
	    if(this->fixedred)
		return wxT("red");
	    return index == 0 ? wxT("red") : wxT("green");
	}
	for(asp = interp->_aspects; asp; asp = asp->_next) {
	    if(n == index)
		return asp->_name;
	    ++n;
	}
	return wxT("red");	    // should be impossible to come here
}

bool	Signal::IsApproach()
{
	return wxStrcmp(GetAction(), wxT("none")) == 0 || _isShuntingSignal;
}

bool	Signal::IsShuntingSignal()
{
	return _isShuntingSignal;
}

bool	Signal::GetSpeedLimit(int *limit)
{
	wxChar	buff[256];
	const wxChar	*action = GetAction();
	action = scan_word(action, buff);

	if(!wxStrcmp(buff, wxT("speedLimit"))) {
	    *limit = wxAtoi(action);
	    return true;
	}
	return false;
}

bool	Signal::GetApproach(ExprValue& result)
{
	bool	res;
	Vector	*path;
	int	i;

	res = GetNextPath(&path);
	if(!path)
	    return res;

	for(i = 0; i < path->_size; ++i) {
	    Track	*trk = path->TrackAt(i);
	    trkdir	dir = (trkdir)path->FlagAt(i);
	    if(dir == E_W || dir == N_S) {
		if(trk->wsignal) {
		    Signal *sig = trk->wsignal;
		    if(sig != this && !sig->IsShuntingSignal() && sig->IsApproach()) {
			result._op = Addr;
			result._signal = sig;
			result._track = sig;
			break;
		    }
		}
	    } else if(trk->esignal) {
		Signal *sig = trk->esignal;
		if(sig != this && !sig->IsShuntingSignal() && sig->IsApproach()) {
		    result._op = Addr;
		    result._signal = sig;
		    result._track = sig;
		    break;
		}
	    }
	}
	res = (i >= path->_size) ? 0 : 1;
	Vector_delete(path);
        if(trace_script._iValue)
	    wxSnprintf(expr_buff + wxStrlen(expr_buff),
                sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), res ? wxT(" approach found ") : wxT(" no approach "));
	return res;
}


bool	Signal::GetPropertyValue(const wxChar *prop, ExprValue& result)
{
	bool	res;
	Vector	*path;
	int	i;

	Signal	*s = this;
        if(trace_script._iValue)
	    wxStrncat(expr_buff, prop, sizeof(expr_buff)-1);

	if(!wxStrcmp(prop, wxT("aspect"))) {
	    result._op = String;
	    result._txt = s->GetAspect();
            TraceStringResult(result._txt);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("auto"))) {
	    result._op = Number;
	    result._val = s->fleeted;
            TraceIntResult(result._val);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("enabled"))) {
	    result._op = Number;
	    result._val = s->fleeted && s->nowfleeted;
            TraceIntResult(result._val);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("name"))) {
	    result._op = String;
            result._txt = s->station;
            TraceStringResult(result._txt);
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
            TraceStringResult(result._val ? wxT("switchThrown") : wxT("switchNotThrown"));
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

                if(trk->type == SWITCH) {
		    switch(trk->direction) {
		    case 10:	// these are Y switches, which are always
		    case 11:	// considered as going to the main line,
		    case 22:	// thus ignored as far as signals are concerned.
		    case 23:
		        continue;
		    }
                    if(!trk->switched)
                        continue;
                }
		for(j = 0; j < NTTYPES; ++j)
		    if(trk->speed[j] && trk->speed[j] < lowSpeed)
			lowSpeed = trk->speed[j];
	    }
	    result._val = lowSpeed;
	    Vector_delete(path);
            if(trace_script._iValue)
	        wxSnprintf(expr_buff + wxStrlen(expr_buff),
		    sizeof(expr_buff)/sizeof(wxChar) - wxStrlen(expr_buff), wxT("{%d}"), lowSpeed);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("occupied")) || !wxStrcmp(prop, wxT("locked"))) {
	    res = s->GetNextPath(&path);
	    if(!path)
		return res;

	    int     oldcolor = -1;
            Track   *trk = 0;
	    for(i = 0; i < path->_size; ++i) {
		trk = path->TrackAt(i);
                if(trk->fgcolor != conf.fgcolor)
                    break;
	    }
            if(i >= path->_size) // path is all black
	        result._val = 0;
            else if(i == 0 && trk->fgcolor == color_green) // path is all green
                result._val = wxStrcmp(prop, wxT("locked")) == 0;
            else // path has an element that is red or blue or is white or is half black and half green
                result._val = 1;
            Vector_delete(path);
            TraceIntResult(result._val);
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
            TraceIntResult(result._val);
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
            TraceStringResult(result._txt);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("redDistance"))) {
	    result._op = Number;
	    result._val = 50000;
            int distance = 0;
            Signal *nextSig = s;
            if(!s->IsClear())
                return true;
            do {
                res = nextSig->GetNextPath(&path);
	        if(!path)
		    return res;
                path->ComputeLength();
                distance += path->_pathlen;
                Vector_delete(path);

                nextSig = nextSig->GetNextSignal();
                if(!nextSig)
                    return true;
            } while(nextSig->IsClear());
            result._val = distance;
            TraceIntResult(result._val);
	    return true;
	}
	if(!wxStrcmp(prop, wxT("color"))) {
	    result._op = String;
	    result._txt = wxT("");
	    if(s->controls)
		result._txt = GetColorName(s->controls->fgcolor);
	    return true;
	}
        if(!wxStrcmp(prop, wxT("script"))) {
            result._op = String;
            result._txt = this->stateProgram ? this->stateProgram : wxT("None");
            return true;
        }
	return false;
}


bool	Signal::SetPropertyValue(const wxChar *prop, ExprValue& val)
{
	Signal	*s = this;
    
	if(!wxStrcmp(prop, wxT("thrown"))) {
	    // call t->Throw(val._val);
	} else if(!wxStrcmp(prop, wxT("aspect"))) {
	    s->SetAspect(val._txt);
	} else if(!wxStrcmp(prop, wxT("auto"))) {
	    s->fleeted = val._val;
	} else if(!wxStrcmp(prop, wxT("enabled"))) {
	    s->nowfleeted = val._val;
	} else if(!wxStrcmp(prop, wxT("fleeted"))) {
	    s->fleeted = val._val;
	    s->nowfleeted = val._val;
	} else if(!wxStrcmp(prop, wxT("shunting"))) {
	    s->_isShuntingSignal = val._val != 0;
	} else if(!wxStrcmp(prop, wxT("click"))) {
	    track_selected(s->x, s->y);
	} else if(!wxStrcmp(prop, wxT("rclick"))) {
	    track_selected1(s->x, s->y);
	} else if(!wxStrcmp(prop, wxT("ctrlclick"))) {
	    Coord c(s->x, s->y);
	    track_control_selected(c);
	} else
	    return false;
	return true;
}


bool    Signal::ToggleForShunting()
{
        Signal  *t = this;
        Track   *trk;
	Vector	*path;
	int	i;

        if(t->status == ST_GREEN)
	    return false;
        if(t->fixedred) {
	    /* ADD PENALTY */
	    return false;
        }
        // we ignore true approach signals
        if((t->IsApproach() && !t->_isShuntingSignal) || t->_intermediate)
	    return false;
        path = findPath(t->controls, t->direction);
        if(!path)
	    return false;
        trk = path->TrackAt(0);
        if(trk->fgcolor == color_white) {
	    for(i = 0; i < path->_size; ++i) {
	        trk = path->TrackAt(i);
	        if(trk->fgcolor != color_white)
		    break;
	        trk->SetColor(conf.fgcolor);
	        t->OnUnclear();	// set to red
	    }
        } else {
	    i = setShuntingPath(path, t->direction, 0);
	    if(i < 0) {
	        Vector_delete(path);
	        return false;
	    }
	    change_coord(t->x, t->y);
	    t->status = ST_WHITE;		// TODO: remove
	    t->SetColor(color_white);	// TODO: remove
	    colorPathStart(path, ST_WHITE, i - 1);
	    t->OnShunt();
        }
        Vector_delete(path);
        t->aspect_changed = 1;
        return true;
}
