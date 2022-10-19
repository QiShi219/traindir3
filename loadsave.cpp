/*	loadsave.cpp - Created by Giampiero Caprino

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
#include <stdlib.h>

#if !defined(__unix__) && !defined(__WXMAC__)
#include <malloc.h>
#endif

#include <memory.h>
#include <string.h>
#include "wx/ffile.h"
#include "Traindir3.h"
#include "Html.h"
#include "TDFile.h"
#include "Itinerary.h"
#include "SwitchBoard.h"
#include "MotivePower.h"
#include "GTFS.h"
#include "Server.h" // for definition or mime types

extern	Shape	*w_train_pmap[NTTYPES];
extern	Shape	*e_train_pmap[NTTYPES];
extern	Shape	*w_car_pmap[NTTYPES];
extern	Shape	*e_car_pmap[NTTYPES];
extern  Array<TrackBase *> dwellingTracks;
extern  double Random_LogNormal(double miu,double sigma,double min,double max);

//***************
	extern long iTimeCounter;   //仿真计时器
	extern long  PosReg[10240000][7];//列车位置信息记录
	extern long  PosRecordid;// 列车位置信息记录条目序号
	extern long  jiange[100];// 段间时间
//*********************

Path	*paths;
Vector	*findPath(Track *t, int dir);
void	colorPartialPath(Vector *path, int state, int start);
void	colorPath(Vector *path, int state);
void	compute_train_numbers();
void	new_train_status(Train *t, int status);
void    setEncoding();
void    restoreTrainPixmaps(void);

bool	performance_hide_canceled = false;

pxmap	*pixmaps;
int	npixmaps, maxpixmaps;

pxmap	*carpixmaps;
int	ncarpixmaps, maxcarpixmaps;

extern  Shape   *itineraryButton;

GTFS    *gtfs;

extern	wxString	current_project;

	wxChar	dirPath[1024];

static	int	curtype = 0;
	int	save_prefs = 1;
const Char    *powerType[NTTYPES];
double  gauge[NTTYPES];
bool    powerSpecified;

static  wxChar	*linebuff;
static	int	maxline;
extern	int	layout_modified;
StringOption    gEncoding(wxT("encoding"), wxT("Character encoding for web pages"), wxT("Options"), wxT("Windows-1251"));
//StringOption    gEncoding(wxT("encoding"), wxT("Character encoding for web pages"), wxT("Options"), wxT("GBK"));

static	const wxChar	*getline(TDFile *fp)
{
	int	i;
        Char    ch;

	if(!linebuff) {
	    maxline = 256;
	    linebuff = (wxChar *)malloc(maxline * sizeof(linebuff[0]));
	    if (!linebuff)
		return NULL;
	}
	i = 0;
        while ((ch = *fp->nextChar)) {
            if(ch == wxT('\r')) {
                ++fp->nextChar;
                continue;
            }
            if(ch == wxT('\n')) {
                ++fp->nextChar;
                break;
            }
            if(i + 2 >= maxline) {
		maxline += 256;
		linebuff = (wxChar *)realloc(linebuff, maxline * sizeof(linebuff[0]));
		if (!linebuff)
		    return NULL;
            }
            linebuff[i++] = ch;
            ++fp->nextChar;
	}
	if(ch == 0 && i == 0)
	    return 0;
        linebuff[i] = 0;
	return linebuff;
}

bool	file_create(const wxChar *name, const wxChar *ext, wxFFile& fp)
{
	wxChar	buff[256];
	wxChar	*p;
	extern	int errno;

	wxStrncpy(buff, name, sizeof(buff)/sizeof(wxChar) - 1);
	buff[sizeof(buff)/sizeof(wxChar) - 1] = 0;
	for(p = buff + wxStrlen(buff); p > buff && *p != '.' && *p != '/' && *p != '\\'; --p);
	if(*p == '.')
	    wxStrcpy(p, ext);
	else
	    wxStrcat(p, ext);
	if(fp.Open(buff, wxT("w")))
	    return true;
	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s '%s' - %s."), L("Can't create file"), name, L("Error"));
	traindir->Error(buff);
	return false;
}

const wxChar	*locase(wxChar *s)
{
	wxChar	*p;

	for(p = s; *p; ++p)
	    *p = wxTolower(*p);
	return s;
}

wxChar	*skipblk(wxChar *p)
{
	while(*p == ' ' || *p == '\t') ++p;
	return p;
}

static bool match(wxChar **pp, wxChar *txt)
{
	wxChar	*p = *pp;

	while(*p == ' ' || *p == '\t')
	    ++p;
	*pp = p;
	if(wxStrncmp(p, txt, wxStrlen(txt)))
	    return false;
	p += wxStrlen(txt);
	while(*p == ' ' || *p == '\t')
	    ++p;
	*pp = p;
	return true;
}

void	clean_field(Track *layout)
{
	Track	*t;

	while(layout) {
	    t = layout->next;
	    if(layout->station)
		free(layout->station);
	    free(layout);
	    layout = t;
	}
        powerSpecified = false;
}

bool    power_specified(const Track *layout)
{
        while(layout) {
            if(layout->power)
                return true;
            layout = layout->next;
        }
        return false;
}

Track	*load_field_tracks(const wxChar *name, Itinerary **itinList)
{
	Track	*layout, *t, *lastTrack;
	TextList *tl, *tlast;
	Itinerary *it;
	wxChar	buff[1024];
	int	l;
	int	ttype;
	int	x, y, sw;
	wxChar	*p, *p1;
	TDFile	trkFile(name);

	// ----------for speed test only by cuidl 20191108
	short SpeedTest,SpeedTest1;
	// end of speedtest
	trkFile.SetExt(wxT(".trk"));
	if(!trkFile.Load()) {
	    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("File '%s' not found."), trkFile.name.GetFullPath().c_str());
	    traindir->Error(buff);
	    return 0;
	}
	lastTrack = 0;
	tlast = 0;
	layout = 0;
	while(trkFile.ReadLine(buff, sizeof(buff)/sizeof(wxChar))) {
	    //t = (Track *)malloc(sizeof(Track));
	    //memset(t, 0, sizeof(Track));
	    if(!wxStrncmp(buff, wxT("(script "), 8)) {
		p = buff + 8;
		x = wxStrtol(p, &p, 10);
		if(*p == wxT(',')) ++p;
		y = wxStrtol(p, &p, 10);

		wxString	script;
		while(trkFile.ReadLine(buff, sizeof(buff)/sizeof(wxChar)) && buff[0] != ')') {
		    wxStrcat(buff, wxT("\n"));
		    script += buff;
		}

		for(t = layout; t; t = t->next) {
		    if(t->x == x && t->y == y)
			break;
		}
		if(!t)
		    continue;
		if(t->stateProgram)
		    free(t->stateProgram);
		t->stateProgram = wxStrdup(script.c_str());
		continue;
	    }
	    if(!wxStrncmp(buff, wxT("(attributes "), 12)) {
		x = wxStrtol(buff + 12, &p, 10);
		if(*p == wxT(',')) ++p;
		y = wxStrtol(p, &p, 10);
		t = find_track(layout, x, y);
		while(trkFile.ReadLine(buff, sizeof(buff)/sizeof(wxChar)) && buff[0] != ')') {
		    if(!t)
			continue;
		    if(!wxStrcmp(buff, wxT("hidden"))) {
			t->invisible = 1;
			continue;
		    }
                    if(!wxStrncmp(buff, wxT("dwellTime:"), 10)) {
                        p = buff + 10;
		        while(*p == wxT(' ') || *p == wxT('\t'))
			    ++p;
                        t->_dwellTime = wxStrtol(p, &p, 10);
			continue;
		    }
		    if(!wxStrncmp(buff, wxT("icons:"), 6)) {	// ITIN and IMAGE
			p = buff + 6;
			int x = 0;
			int ch = 0;
			do {
			    while(*p == wxT(' ') || *p == wxT('\t'))
				++p;
			    wxChar *n = p;
			    while(*p && *p != wxT(','))
				++p;
			    ch = *p++;	// to check for end of string
			    *p = 0;
			    t->_flashingIcons[x++] = wxStrdup(n);
			} while(x < MAX_FLASHING_ICONS && ch);
                        continue;
		    }
                    if(!wxStrncmp(buff, wxT("locked"), 6)) {
			p = buff + 6;
                        t->_lockedBy = wxStrdup(p);
                        continue;
                    }
                    if(!wxStrncmp(buff, wxT("power:"), 6)) {
                        p = buff + 6;
                        while(*p && *p == ' ') ++p;
                        t->power = power_parse(p);
                        continue;
                    }
					 /*if(!wxStrncmp(buff, wxT("km_post:"), 6)) {
                        p = buff + 6;
                        while(*p && *p == ' ') ++p;
                        t->km_post = power_parse(p); 
                        continue;
                    }*/
                    if(!wxStrncmp(buff, wxT("intermediate"), 12)) {
			p = buff + 12;
                        t->_intermediate = wxStrtol(p, &p, 10) != 0;
                        t->_nReservations = 0;
#if 01
                        if(t->_intermediate) {
                            t->fleeted = 0;
                            t->nowfleeted = 0;
                            switch(t->direction) {
                            case signal_WEST_FLEETED: t->direction = E_W; break;
                            case signal_EAST_FLEETED: t->direction = W_E; break;
                            case signal_SOUTH_FLEETED: t->direction = N_S; break;
                            case signal_NORTH_FLEETED: t->direction = S_N; break;
                            }
                        }
#endif
                        continue;
                    }
                    if(!wxStrncmp(buff, wxT("dontstopshunters"), 16)) {
                        t->flags |= TFLG_DONTSTOPSHUNTERS;
                        continue;
                    }
		}
		continue;
	    }
	    if(!wxStrncmp(buff, wxT("(switchboard "), 13)) {
		p = buff + 13;
		if(wxStrchr(p, wxT(')')))
		    *wxStrchr(p, wxT(')')) = 0;
		SwitchBoard *sb = CreateSwitchBoard(p);
		sb->Load(p);
		continue;
	    }
	    t = new Track();
	    t->fgcolor = fieldcolors[COL_TRACK];
	    ttype = buff[0];

#if 0
	    // script lines

	    if(ttype == wxT('\t') || ttype == wxT(' ')) {
		wxStrcat(buff, wxT("\n"));
		append_to_script(buff);
	    }
#endif
	    p = buff + 1;
	    if(*p == wxT(',')) ++p;
	    t->x = wxStrtol(p, &p, 10);
	    if(*p == wxT(',')) ++p;
	    t->y = wxStrtol(p, &p, 10);
	    if(t->x >= ((XMAX - HCOORDBAR) / HGRID) ||
		    t->y >= ((YMAX - VCOORDBAR) / VGRID))
		continue;
	    if(*p == wxT(',')) ++p;
	    t->direction = (trkdir)wxStrtol(p, &p, 10);
	    if(*p == wxT(',')) ++p;
	    if(!layout)
		layout = t;
	    else
		lastTrack->next = t;
	    lastTrack = t;
            t->_lockedBy = 0;
	    switch(ttype) {
			/*************/
	    case wxT('0'):
		t->type = TRACK;
		t->isstation = (char)wxStrtol(p, &p, 10);
		if(*p == wxT(',')) ++p;
		t->length = wxStrtol(p, &p, 10);
		if(!t->length)
		    t->length = 1;
		if(*p == ',') ++p;
		t->wlinkx = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		t->wlinky = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		t->elinkx = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		t->elinky = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		if(*p == '@') {
		    int	    i;

		    t->speed[0] = wxStrtol(p + 1, &p, 10);
		    for(i = 1; i < NTTYPES && *p == '/'; ++i) {
			t->speed[i] = wxStrtol(p + 1, &p, 10);
			SpeedTest=t->speed[i];
			SpeedTest1=SpeedTest;
		    }
		    if(*p == ',') ++p;
		}
		if(!*p || !wxStrcmp(p, wxT("noname"))) {
		    t->isstation = 0;
		    break;
		}
		if(*p == '>') {
		    p = parse_km(t, p + 1);
		    if(*p == ',')
			++p;
		}
		if(*p == 'L') {   //  20201209ypx
			++p;
		    t->line_num = wxStrtol(p, &p, 10);
		    if(*p == ',')
			++p;
		}
		t->station = wxStrdup(p);
		break;

	    case '1':
		t->type = SWITCH;
		t->length = 1;
		t->wlinkx = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		t->wlinky = wxStrtol(p, &p, 10);
		if(*p == '@') {
		    int	    i;

		    t->speed[0] = wxStrtol(p + 1, &p, 10);
		    for(i = 1; i < NTTYPES && *p == '/'; ++i) {
			t->speed[i] = wxStrtol(p + 1, &p, 10);
		    }
		    if(*p == ',') ++p;
		}
		if(!*p || !wxStrcmp(p, wxT("noname")))
		    break;
		if(*p == '>') {
		    p = parse_km(t, p + 1);
		    if(*p == ',')
			++p;
		}
		t->station = wxStrdup(p);
		break;

	/* 2, x, y, type, linkx, linky [itinerary] */

	    case '2':
		t->type = TSIGNAL;
		t->status = ST_RED;
		if((l = t->direction) & 2) {
		    t->fleeted = 1;
		    l &= ~2;
		}
		if(l & 0x100)
		    t->fixedred = 1;
		if(l & 0x200)
		    t->nopenalty = 1;
		if(l & 0x400)
		    t->signalx = 1;
		if(l & 0x800)
		    t->noClickPenalty = 1;
		l &= ~0xF00;
		t->direction = (trkdir)((int)t->direction & ~0xF00);

		switch(l) {
		case 0:
		    t->direction = E_W;
		    break;

		case 1:
		    t->direction = W_E;
		    break;

		case N_S:
		case S_N:
		case signal_SOUTH_FLEETED:
		case signal_NORTH_FLEETED:
		    /* already there */
		    t->direction = (trkdir)l;
		    break;
		}
		t->wlinkx = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		t->wlinky = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		if(*p == '@') {
		    p1 = p + 1;
		    p = wxStrchr(p1, ',');
		    if(p)
			*p++ = 0;
		    t->stateProgram = wxStrdup(p1);
		}
		if(p && *p)			/* for itinerary definition */
		    t->station = wxStrdup(p);
		break;

	    case '3':
		t->type = PLATFORM;
		if(t->direction == 0)
		    t->direction = W_E;
		else
		    t->direction = N_S;
		break;

	    case '4':
		t->type = TEXT;
		t->station = wxStrdup(p);
		for(l = 0; t->station[l] && t->station[l] != ','; ++l);
		t->station[l] = 0;
		while(*p && *p != ',') ++p;
		if(*p == ',') ++p;
		t->wlinkx = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		t->wlinky = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		t->elinkx = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		t->elinky = wxStrtol(p, &p, 10);
		if(*p == '>')
		    p = parse_km(t, ++p);
		break;

	    case '5':
		t->type = IMAGE;
                if(*p == '@') {
                    t->wlinkx = wxStrtol(p + 1, &p, 10);
                    if(*p == ',') ++p;
                    t->wlinky = wxStrtol(p, &p, 10);
                    if(*p == ',') ++p;
                }
		t->station = wxStrdup(p);
		break;

	    case '6':			/* territory information */
		tl = (TextList *)malloc(sizeof(TextList));
		wxStrcat(p, wxT("\n"));	/* put it back, since we removed it */
		tl->txt = wxStrdup(p);
		if(!track_info)
		    track_info = tl;
		else
		    tlast->next = tl;
		tl->next = 0;
		tlast = tl;
		break;

	    case '7':			/* itinerary */
		for(p1 = p; *p && *p != ','; ++p);
		if(!*p)
		    break;
		*p++ = 0;
		it = (Itinerary *)calloc(sizeof(Itinerary), 1);
		it->name = wxStrdup(p1);
		for(p1 = p, l = 0; *p && (*p != ',' || l); ++p) {
		    if(*p == '(') ++l;
		    else if(*p == ')') --l;
		}
		if(!*p)
		    break;
		*p++ = 0;
		it->signame = wxStrdup(p1);
		for(p1 = p, l = 0; *p && (*p != ',' || l); ++p) {
		    if(*p == '(') ++l;
		    else if(*p == ')') --l;
		}
		if(!*p)
		    break;
		*p++ = 0;
		it->endsig = wxStrdup(p1);
		if(*p == '@') {
		    for(p1 = ++p, l = 0; *p && (*p != ',' || l); ++p) {
			if(*p == '(') ++l;
			else if(*p == ')') --l;
		    }
		    if(!*p)
			break;
		    *p++ = 0;
		    it->nextitin = wxStrdup(p1);
		}
		l = 0;
		while(*p) {
		    x = wxStrtol(p, &p, 0);
		    if(*p != ',')
			break;
		    y = wxStrtol(p + 1, &p, 0);
		    if(*p != ',')
			break;
		    sw = wxStrtol(p + 1, &p, 0);
		    add_itinerary(it, x, y, sw);
		    if(*p == ',') ++p;
		}
		it->next = *itinList;	/* all ok, add to the list */
		*itinList = it;
		break;

	    case '8':			/* itinerary placement */
		t->type = ITIN;
		t->station = wxStrdup(p);
                t->_shape = itineraryButton;
		break;

	    case '9':
		t->type = TRIGGER;
		t->wlinkx = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		t->wlinky = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		t->elinkx = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		t->elinky = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		for(l = 0; l < NTTYPES && *p != ','; ++l) {
		    t->speed[l] = wxStrtol(p, &p, 10);
		    if(*p == '/') ++p;
		}
		if(*p == ',') ++p;
		if(!*p || !wxStrcmp(p, wxT("noname")))
		    break;
		t->station = wxStrdup(p);
		break;
	    }
	}
	return layout;
}

Track	*load_field(const wxChar *name)
{
	TextList *tl;
	Track	*t;

        restoreTrainPixmaps();
	while((tl = track_info)) {
	    track_info = tl->next;
	    free(tl->txt);
	    free(tl);
	}
        RemoveAllSwitchBoards();
        delete_all_itineraries();
        powerSpecified = false;
        // No need to free the cache, since they're just string that can be re-used across layouts
#if 0
	if(script_text) {
	    free(script_text);
	    script_text = 0;
	}
#endif
        dwellingTracks.Clear();
	free_scripts();
	t = load_field_tracks(name, &itineraries);
	sort_itineraries();
	if(t) {
	    link_all_tracks(t);
	    link_signals(t);
            powerSpecified = power_specified(t);
//	    load_scripts(t);	    // too soon, need trainsim_init() first
	    current_project = name;
	}
	layout_modified = 0;
	//add_to_script(t);
	//parse_script();
	return t;
}

Track	*find_track(Track *layout, int x, int y)
{
	while(layout) {
	    if(layout->x == x && layout->y == y)
		return(layout);
	   layout = layout->next;
	}
	return 0;
}

void	link_signals(Track *layout)
{
	TrackBase	*t;

	for(t = layout; t; t = t->next)	    /* in case signal was relinked during edit */
	    t->esignal = t->wsignal = 0;

	for(t = layout; t; t = t->next) {

	    /*	link signals with the track they control	*/

	    if(t->type == TSIGNAL) {
		if(!(t->controls = findTrack(t->wlinkx, t->wlinky)))
		    continue;
		if(t->direction == W_E || t->direction == S_N)
		    t->controls->esignal = (Signal *)t;
		else
		    t->controls->wsignal = (Signal *)t;
	    }
	}
}

void	clean_pixmap_cache(void)
{
	int	i;

	for(i = 0; i < npixmaps; ++i)
	    if(pixmaps[i].name) {
		free(pixmaps[i].name);
//		delete_pixmap(pixmaps[i].pixels);
	    }
	npixmaps = 0;
	for(i = 0; i < ncarpixmaps; ++i)
	    if(carpixmaps[i].name) {
		free(carpixmaps[i].name);
//		delete_pixmap(carpixmaps[i].pixels);
	    }
	ncarpixmaps = 0;
}

int	get_pixmap_index(const wxChar *mapname)
{
	int	i;

	for(i = 0; i < npixmaps; ++i)
	    if(!wxStrcmp(mapname, pixmaps[i].name))
		return i;
	if(npixmaps >= maxpixmaps) {
	    maxpixmaps += 10;
	    if(!pixmaps)
		pixmaps = (pxmap *)malloc(sizeof(pxmap) * maxpixmaps);
	    else
		pixmaps = (pxmap *)realloc(pixmaps, sizeof(pxmap) * maxpixmaps);
	}
	if(!(pixmaps[npixmaps].pixels = (char *)get_pixmap_file(mapname)))
	    return -1;          /* failed! file does not exist */
	pixmaps[npixmaps].name = wxStrdup(mapname);
	return npixmaps++;
}

int	get_carpixmap_index(const wxChar *mapname)
{
	int	i;

	for(i = 0; i < ncarpixmaps; ++i)
	    if(!wxStrcmp(mapname, carpixmaps[i].name))
		return i;
	if(ncarpixmaps >= maxcarpixmaps) {
	    maxcarpixmaps += 10;
	    if(!carpixmaps)
		carpixmaps = (pxmap *)malloc(sizeof(pxmap) * maxcarpixmaps);
	    else
		carpixmaps = (pxmap *)realloc(carpixmaps, sizeof(pxmap) * maxcarpixmaps);
	}
	if(!(carpixmaps[ncarpixmaps].pixels = (char *)get_pixmap_file(mapname)))
	    return -1;          /* failed! file does not exist */
	carpixmaps[ncarpixmaps].name = wxStrdup(mapname);
	return ncarpixmaps++;
}

void	clean_trains(Train *sched)
{
	Train	*t;

	clean_pixmap_cache();
	while(sched) {
	    t = sched->next;
	    delete sched;
	    sched = t;
	}
}

void	clean_trainss(Train *sched)
{
	Train	*t;

	//clean_pixmap_cache();
	while(sched) {
	    t = sched->next;
	    free(sched);
	    sched = t;
	}
}
int	trcmp(const void *a, const void *b)
{
	const Train *ap = *(const Train **)a;
	const Train *bp = *(const Train **)b;
	if(ap->timein < bp->timein)
	    return -1;
	if(ap->timein > bp->timein)
	    return 1;
	return 0;
}

Train	*sort_schedule(Train *sched)
{
	Train	**qb, *t;
	int	ntrains;
	int	l;

	for(t = sched, ntrains = 0; t; t = t->next)
	    ++ntrains;
	if(!ntrains)
	    return sched;
	qb = (Train **)malloc(sizeof(Train *) * ntrains);
	for(t = sched, l = 0; l < ntrains; ++l, t = t->next)
	    qb[l] = t;
	qsort(qb, ntrains, sizeof(Train *), trcmp);
	for(l = 0; l < ntrains - 1; ++l)
	    qb[l]->next = qb[l + 1];
	qb[ntrains - 1]->next = 0;
	t = qb[0];
	free(qb);
	return t;
}

wxChar	*convert_station(wxChar *p)
{
	return(p);
}

Train	*cancelTrain(wxChar *p, Train *sched)
{
	Train	*t, *t1;

	t1 = 0;
	for(t = sched; t && wxStrcmp(t->name, p); t = t->next)
	    t1 = t;
	if(!t)
	    return sched;
	if(t == sched)
	    sched = t->next;
	else
	    t1->next = t->next;
	free(t->name);
	free(t);
	return sched;
}

Train	*cancelTrains(wxChar *p, Train *sched1)
{
	Train	*t, *t1;

	t1 = 0;
	for(t = sched1; t && wxStrcmp(t->name, p); t = t->next)
	    t1 = t;
	if(!t)
	    return sched1;
	if(t == sched1)
	    sched1 = t->next;
	else
	    t1->next = t->next;
	free(t->name);
	free(t);
	return sched1;
}
Train	*find_train(Train *sched, const Char *name)
{
	Train	*t;

	for(t = sched; t; t = t->next)
	    if(!wxStrcmp(t->name, name))
		break;
	return t;
}
Train	*find_trains(Train *sched1, const Char *name)
{
	Train	*t;

	for(t = sched1; t; t = t->next)
	    if(!wxStrcmp(t->name, name))
		break;
	return t;
}

static	Train	*sched;
static	Train	*sched1;


int	scanline(wxChar *dst, int size, wxChar **ptr)
{
	wxChar	*p = *ptr;
	int	c;

	while((c = *p++) && c != '\n') {
	    if(c == '\r')
		continue;
	    *dst++ = c;
	    if(--size < 2) {
		*dst = 0;
		break;
	    }
	}
	*ptr = p;
	return c != 0;
}

static	Char	*gtfs_dirname;
static	Char	gtfs_filename[256];

Char	*build_gtfs_name(Char *fileName)
{
	if(*gtfs_dirname)
	    wxSnprintf(gtfs_filename, sizeof(gtfs_filename), wxT("%s/%s"), gtfs_dirname, fileName);
	else
	    wxStrncpy(gtfs_filename, fileName, sizeof(gtfs_filename));
	return gtfs_filename;
}

void	get_fields(Char **dst, Char *src)
{
	int	i;

	for(i = 0; *src; ++i) {
	    dst[i] = src;
	    while(*src && *src != ',')
		++src;
            if(!*src) {
                if(src != dst[i])
                    ++i;
		break;
            }
	    *src++ = 0;
	}
	dst[i] = 0;
}

Train	*read_gtfs(Train *sched, Char *dirname)
{
	Char	buff[256];
	Char	*fields[32];
	Char	*p;
	Train	*tr;
	TrainStop *stp;

	while(*dirname == ' ' || *dirname == '\t')
	    ++dirname;
	gtfs_dirname = dirname;
        gtfs->Load(dirname);

        // match routes with types

        TDFile	stops(build_gtfs_name(wxT("stop_times.txt")));
        if(!stops.Load()) {
            // TODO: free stop
            return sched;
        }
	stops.ReadLine(buff, sizeof(buff)/sizeof(wxChar));	// first line is fields names
	// TODO: get field names positions in each record
	while(stops.ReadLine(buff, sizeof(buff)/sizeof(wxChar))) {
	    get_fields(fields, buff);
	    tr = find_train(sched, fields[0]);
	    if(!tr) {
		tr = new Train();
		tr->name = wxStrdup(fields[0]);
		tr->next = sched;
		tr->type = curtype;
		tr->epix = e_train_pmap[curtype];
                tr->wpix = w_train_pmap[curtype];
		tr->ecarpix = e_car_pmap[curtype];
                tr->wcarpix = w_car_pmap[curtype];
		sched = tr;
	    }

	    stp = (TrainStop *)calloc(1, sizeof(TrainStop));
	    stp->minstop = 30;
	    p = fields[1];
	    stp->arrival = parse_time(&p);	/* arrival */
	    p = fields[2];
	    stp->departure = parse_time(&p);
	    if(stp->departure == stp->arrival)
		stp->departure = stp->arrival + stp->minstop;
	    else if(stp->minstop > stp->departure - stp->arrival)
		stp->minstop = stp->departure - stp->arrival;
	    stp->station = wxStrdup(fields[3]);
	    if(!tr->stops)
		tr->stops = stp;
	    else
		tr->laststop->next = stp;
	    tr->laststop = stp;
	}
        for(tr = sched; tr; tr = tr->next) {
            if(!tr->entrance && !tr->exit && tr->stops) {
                stp = tr->stops;
                tr->stops = tr->stops->next;
                tr->entrance = stp->station;
                tr->timein = stp->departure;
                free(stp);
                TrainStop *ostop = 0;
                for(stp = tr->stops; stp && stp != tr->laststop; stp = stp->next)
                    ostop = stp;
                tr->exit = tr->laststop->station;
                tr->timeout = tr->laststop->arrival;
                tr->laststop = ostop;
                if(ostop)
                    ostop->next = 0;
                else
                    tr->stops = 0;
                if(stp) {
                    free(stp);
                } else {
                    ostop = 0;
                }
            }
        }

        GTFS_Route *selected[NTTYPES];
        int nSelected = 0;
        GTFS_Route *route;
        int s, r;

        GTFS_Trip *trip;
        int x;

        // find the subset of routes used in the schedule
        // and with different colors
        for(x = 0; x < gtfs->_trips.Length(); ++x) {
            trip = gtfs->_trips.At(x);
            if(gtfs->IgnoreRoute(trip->_routeId))
                continue;
            route = gtfs->FindRouteById(trip->_routeId);
            if(!route)
                continue;
            for(s = 0; s < nSelected; ++s)
                if(selected[s]->_routeColor == route->_routeColor)
                    break;
            if(s >= nSelected && nSelected < NTTYPES) {
                selected[nSelected++] = route;
            }
        }

        for(x = 0; x < gtfs->_trips.Length(); ++x) {
            trip = gtfs->_trips.At(x);
            tr = find_train(sched, trip->_tripId);
            if(!tr)             // impossible
                continue;
            if(gtfs->IgnoreRoute(trip->_routeId)) {
                tr->isExternal = 1;
                continue;
            }
            GTFS_Calendar *calEntry = gtfs->FindCalendarByService(trip->_serviceId);
            if(calEntry)
                tr->days = calEntry->GetMask();
            // set type based on routeId
            route = gtfs->FindRouteById(trip->_routeId);
            if(!route)
                continue;
            for(r = 0; r < nSelected; ++r) {
                if(selected[r]->_routeColor == route->_routeColor) {
                    tr->type = r;
    		    tr->epix = e_train_pmap[r];
                    tr->wpix = w_train_pmap[r];
		    tr->ecarpix = e_car_pmap[r];
                    tr->wcarpix = w_car_pmap[r];
                    break;
                }
            }
        }
	return sched;
}

Train	*read_gtfss(Train *sched, Char *dirname)
{
	Char	buff[256];
	Char	*fields[32];
	Char	*p;
	Train	*tr;
	TrainStop *stp;

	while(*dirname == ' ' || *dirname == '\t')
	    ++dirname;
	gtfs_dirname = dirname;
        gtfs->Load(dirname);

        // match routes with types

        TDFile	stops(build_gtfs_name(wxT("stop_times.txt")));
        if(!stops.Load()) {
            // TODO: free stop
            return sched1;
        }
	stops.ReadLine(buff, sizeof(buff)/sizeof(wxChar));	// first line is fields names
	// TODO: get field names positions in each record
	while(stops.ReadLine(buff, sizeof(buff)/sizeof(wxChar))) {
	    get_fields(fields, buff);
	    tr = find_trains(sched1, fields[0]);
	    if(!tr) {
		tr = new Train();
		tr->name = wxStrdup(fields[0]);
		tr->next = sched1;
		tr->type = curtype;
		tr->epix = e_train_pmap[curtype];
                tr->wpix = w_train_pmap[curtype];
		tr->ecarpix = e_car_pmap[curtype];
                tr->wcarpix = w_car_pmap[curtype];
		sched1 = tr;
	    }

	    stp = (TrainStop *)calloc(1, sizeof(TrainStop));
	    stp->minstop = 30;
	    p = fields[1];
	    stp->arrival = parse_time(&p);	/* arrival */
	    p = fields[2];
	    stp->departure = parse_time(&p);
	    if(stp->departure == stp->arrival)
		stp->departure = stp->arrival + stp->minstop;
	    else if(stp->minstop > stp->departure - stp->arrival)
		stp->minstop = stp->departure - stp->arrival;
	    stp->station = wxStrdup(fields[3]);
	    if(!tr->stops)
		tr->stops = stp;
	    else
		tr->laststop->next = stp;
	    tr->laststop = stp;
	}
        for(tr = sched1; tr; tr = tr->next) {
            if(!tr->entrance && !tr->exit && tr->stops) {
                stp = tr->stops;
                tr->stops = tr->stops->next;
                tr->entrance = stp->station;
                tr->timein = stp->departure;
                //free(stp);
                TrainStop *ostop = 0;
                for(stp = tr->stops; stp && stp != tr->laststop; stp = stp->next)
                    ostop = stp;
                tr->exit = tr->laststop->station;
                tr->timeout = tr->laststop->arrival;
                tr->laststop = ostop;
                if(ostop)
                    ostop->next = 0;
                else
                    tr->stops = 0;
                if(stp) {
                    free(stp);
                } 
			    else {
                    ostop = 0;
                }
            }
        }

        GTFS_Route *selected[NTTYPES];
        int nSelected = 0;
        GTFS_Route *route;
        int s, r;

        GTFS_Trip *trip;
        int x;

        // find the subset of routes used in the schedule
        // and with different colors
        for(x = 0; x < gtfs->_trips.Length(); ++x) {
            trip = gtfs->_trips.At(x);
            if(gtfs->IgnoreRoute(trip->_routeId))
                continue;
            route = gtfs->FindRouteById(trip->_routeId);
            if(!route)
                continue;
            for(s = 0; s < nSelected; ++s)
                if(selected[s]->_routeColor == route->_routeColor)
                    break;
            if(s >= nSelected && nSelected < NTTYPES) {
                selected[nSelected++] = route;
            }
        }

        for(x = 0; x < gtfs->_trips.Length(); ++x) {
            trip = gtfs->_trips.At(x);
            tr = find_trains(sched1, trip->_tripId);
            if(!tr)             // impossible
                continue;
            if(gtfs->IgnoreRoute(trip->_routeId)) {
                tr->isExternal = 1;
                continue;
            }
            GTFS_Calendar *calEntry = gtfs->FindCalendarByService(trip->_serviceId);
            if(calEntry)
                tr->days = calEntry->GetMask();
            // set type based on routeId
            route = gtfs->FindRouteById(trip->_routeId);
            if(!route)
                continue;
            for(r = 0; r < nSelected; ++r) {
                if(selected[r]->_routeColor == route->_routeColor) {
                    tr->type = r;
    		    tr->epix = e_train_pmap[r];
                    tr->wpix = w_train_pmap[r];
		    tr->ecarpix = e_car_pmap[r];
                    tr->wcarpix = w_car_pmap[r];
                    break;
                }
            }
        }
	return sched1;
}
wxChar	*parse_delay(wxChar *p, TDDelay *del)
{
	del->nDelays = 0;
	do {
	    int	secs = wxStrtoul(p, &p, 0);
	    if(*p == '/')
		++p;
	    int prob = wxStrtoul(p, &p, 0);
	    if(del->nDelays < MAX_DELAY) {
		del->prob[del->nDelays] = prob;
		del->seconds[del->nDelays] = secs;
		++del->nDelays;
	    }
	} while(*p && *p++ == ',');
	return p;
}


Train	*parse_newformat(TDFile& schFile)
{
	Train	*t;
	TrainStop *stp;
	wxChar	buff[1024];
	int	l;
	long prestopdep0,prestopdep;//  上一停车站的发车时间
	wxChar	*p, *p1;
	wxString fileinc;
	wxChar	*nw, *ne;
	//void	*pmw, *pme;
        Shape   *pmw, *pme;

	srand(time(0));

	t = 0;
	while(schFile.ReadLine(buff, sizeof(buff)/sizeof(wxChar))) {
	    if(buff[0] == '.') {
		t = 0;
		continue;
	    }
	    for(l = 0; buff[l]; ++l)
		if(buff[l] == '\t')
		    buff[l] = ' ';
	    while(l && (buff[l - 1] == ' ' || buff[l - 1] == '\t')) --l;
	    buff[l] = 0;
	    if(!buff[0] || buff[0] == '#')
		continue;
            p = buff;
			if(match(&p, wxT("Include:"))) {
		if(!*p)
		    continue;

		TDFile	incFile(p);

		if(incFile.Load())
		    parse_newformat(incFile);
		else {
		    fileinc.Printf(wxT("%s/%s"), dirPath, locase(p));
		    TDFile  incFile1(fileinc);

		    if(!incFile1.Load())
			continue;
		    parse_newformat(incFile1);
		}
#if 0
		if(!sched) {
		    sched = t;
		    continue;
		}
		for(t1 = t; t->next; t = t->next);
		t->next = sched;
		sched = t1;
#endif
		t = 0;
		continue;
	    }
            if(match(&p, wxT("Routes:"))) {
                if(!*p)
                    continue;
                gtfs->SetOurRoutes(p);
		continue;
	    }
            if(match(&p, wxT("GTFS:"))) {
                if(!*p)
                    continue;
		sched = read_gtfs(sched, p);
		continue;
	    }
            if(match(&p, wxT("Cancel:"))) {
		if(!*p)
		    continue;
		sched = cancelTrain(p, sched);
		t = 0;
		continue;
	    }
            if(match(&p, wxT("Today:"))) {
		for(l = 0; *p >= '0' && *p <= '9'; ++p)
		    l |= 1 << (*p - '1');
		run_day = l;
		continue;
	    }
            if(match(&p, wxT("Encoding:"))) {
                if(*p)
                    gEncoding.Set(p);
                continue;
            }
            if(match(&p, wxT("Start:"))) {
		current_time = start_time = parse_time(&p);
		continue;
	    }
            if(match(&p, wxT("Train:"))) {
		t = find_train(sched, p);
		if(t)
		    continue;
		t = new Train();
		t->laststop = new TrainStop();
		t->name = wxStrdup(buff + 6);
		t->next = sched;
		t->type = curtype;
		t->epix = e_train_pmap[curtype];
                t->wpix = w_train_pmap[curtype];
		t->ecarpix = e_car_pmap[curtype];
                t->wcarpix = w_car_pmap[curtype];
		sched = t;
		continue;
	    }
	    if(!t) {
                if(match(&p, wxT("Type:"))) {
		    if((l = wxStrtol(p, &p, 0) - 1) >= NTTYPES || l < 0)
			continue;
		    curtype = l;
		    if(!p)
			continue;
		    while(*p == ' ' || *p == '\t') ++p;
		    if(!*p)
			continue;
                    if(*p == '+') {
                        startDelay[curtype] = wxStrtol(++p, &p, 10);
                    }
                    if(*p == '>') {
                        accelRate[curtype] = wxAtof(++p);
                        while(*p && *p != ' ' && *p != '\t')
                            ++p;
                    }
		    while(*p == ' ' || *p == '\t') ++p;
		    if(!*p)
			continue;
		    nw = p;
		    while(*p && *p != ' ' && *p != '\t') ++p;
		    if(!*p)
			continue;
		    *p++ = 0;
		    while(*p == ' ' || *p == '\t') ++p;
		    ne = p;
		    while(*p && *p != ' ' && *p != '\t') ++p;
		    l = *p;
		    *p++ = 0;
//		    if(!(pmw = get_pixmap_file(locase(nw))))
//			continue;
//		    if(!(pme = get_pixmap_file(locase(ne))))
//			continue;
                    pmw = shapeManager.LoadXpm(locase(nw));
                    if(!pmw)
                        continue;
                    pme = shapeManager.LoadXpm(locase(ne));
                    if(pme)
                        continue;
		    w_train_pmap[curtype] = pmw;
		    e_train_pmap[curtype] = pme;
		    if(!l)
			continue;
		    while(*p == ' ' || *p == '\t') ++p;
		    ne = p;
		    while(*p && *p != ' ' && *p != '\t') ++p;
		    l = *p;
		    *p++ = 0;
//		    if(!(pmw = get_pixmap_file(locase(ne))))
//			continue;
                    pmw = shapeManager.LoadXpm(locase(ne));
                    if(!pmw)
                        continue;
		    w_car_pmap[curtype] = pmw;
		    e_car_pmap[curtype] = pmw;
		    if(!l)
			continue;
		    while(*p == ' ' || *p == '\t') ++p;
		    if(!*p)
			continue;
//		    if(!(pme = get_pixmap_file(locase(p))))
//			continue;
                    pme = shapeManager.LoadXpm(locase(p));
                    if(!pme)
                        continue;
		    e_car_pmap[curtype] = pme;
                    continue;
		}
                if(match(&p, wxT("Power:"))) {
		    if((l = wxStrtol(p, &p, 0) - 1) >= NTTYPES || l < 0)
			continue;
                    powerType[l] = power_parse(p);
                    continue;
                }
                if(match(&p, wxT("Gauge:"))) {
		    if((l = wxStrtol(p, &p, 0) - 1) >= NTTYPES || l < 0)
			continue;
                    gauge[l] = wxAtof(p);
                    continue;
                }
		continue;
	    }
            if(match(&p, wxT("Wait:"))) {
		for(nw = p; *nw && *nw != ' '; ++nw);
		if(*nw)
		   *nw++ = 0;
		else
		    nw = 0;
		t->waitfor = wxStrdup(p);
		t->waittime = nw ? wxAtoi(nw) : 60;
		continue;
	    }
            if(match(&p, wxT("StartDelay:"))) {
                t->myStartDelay = wxAtoi(p);
                continue;
            }
            if(match(&p, wxT("AccelRate:"))) {
                t->accelRate = wxAtof(p);
                continue;
            }
            if(match(&p, wxT("Power:"))) {
                t->power = power_parse(p);
                continue;
            }
			/*if(match(&p, wxT("km_post:"))) {
                t->power = power_parse(p);
                continue;
            }*/
            if(match(&p, wxT("Gauge:"))) {
                t->gauge = wxAtof(p);
                continue;
            }
            if(match(&p, wxT("When:"))) {
		for(l = 0; *p >= '0' && *p <= '9'; ++p)
		    l |= 1 << (*p - '1');
		t->days = l;
		continue;
	    }
            if(match(&p, wxT("Speed:"))) {
		t->maxspeed = wxAtoi(p);
		continue;
	    }
            if(match(&p, wxT("Type:"))) {
		if((l = wxStrtol(p, &p, 0)) - 1 < NTTYPES)
		    t->type = l - 1;
                t->wpix = w_train_pmap[t->type];
                t->epix = e_train_pmap[t->type];
                t->wcarpix = w_car_pmap[t->type];
                t->ecarpix = e_car_pmap[t->type];
		if(!p || !*p)
		    continue;
		while(*p == ' ' || *p == '\t') ++p;
		if(!*p)
		    continue;
		nw = p;
		while(*p && *p != ' ' && *p != '\t') ++p;
		if(!*p)
		    continue;
		*p++ = 0;
		while(*p == ' ' || *p == '\t') ++p;
                pmw = shapeManager.LoadXpm(locase(nw));
                if(!pmw)
                    continue;
                t->wpix = pmw;
//		if((t->wpix = get_pixmap_index(locase(nw))) < 0)
//		    continue;
//		t->epix = get_pixmap_index(locase(p));
                pme = shapeManager.LoadXpm(locase(p));
                if(!pme)
                    continue;
                t->epix = pme;
		continue;
	    }
            if(match(&p, wxT("Stock:"))) {
		t->stock = wxStrdup(p);
		continue;
	    }
            if(match(&p, wxT("Length:"))) {
		t->length = wxStrtol(p, &p, 0);
		t->entryLength = t->length;
		t->tail = (Train *)calloc(sizeof(Train), 1);
		t->ecarpix = e_car_pmap[0];
                t->wcarpix = w_car_pmap[0];
		while(*p == ' ' || *p == '\t') ++p;
		if(!*p)
		    continue;
		ne = p;
		while(*p && *p != ' ' && *p != '\t') ++p;
		l = *p;
		*p++ = 0;
                pmw = shapeManager.LoadXpm(locase(ne)); // get_carpixmap_index(locase(ne));
                if(!pmw)
                    continue;
                t->ecarpix = t->wcarpix = pmw;
		if(!l)
		    continue;
		while(*p == ' ' || *p == '\t') ++p;
		if(!*p)
		    continue;
                pmw = shapeManager.LoadXpm(locase(p)); // get_carpixmap_index(locase(p));
                if(!pmw)
                    continue;
                t->wcarpix = pmw;
		continue;
	    }
			if(match(&p, wxT("Enter:"))) {
		t->timein = parse_time(&p);
		if(*p == DELAY_CHAR) {
		    t->entryDelay = (TDDelay *)calloc(sizeof(TDDelay), 1);
		    p = parse_delay(++p, t->entryDelay);
		}
		if(*p == ',') ++p;
		while(*p == ' ' || *p == '\t') ++p;
                p1 = wxStrchr(p, '|');
                if(p1) {
                    t->alternateEntries = wxStrdup(p1 + 1);
                    *p1 = 0;
                }
		t->entrance = wxStrdup(convert_station(p));
		continue;
	    }
            if(match(&p, wxT("Notes:"))) {
		if(t->nnotes < MAXNOTES)
		    t->notes[t->nnotes++] = wxStrdup(p);
		continue;
	    }
            if(match(&p, wxT("Script:"))) {
		wxString scr;
		while(schFile.ReadLine(buff, sizeof(buff)/sizeof(wxChar))) {
                    for(p = buff + wxStrlen(buff) - 1; p > buff && (p[0] == ' ' || p[0] == '\t'); --p);
                    p[1] = 0;
                    p = skipblk(buff);
		    if(!buff[0] || buff[0] == '#')
			continue;
		    if(!wxStrcmp(buff, wxT("EndScript"))) {
			break;
		    }
		    wxStrcat(buff, wxT("\n"));
		    scr += buff;
		}
		t->stateProgram = wxStrdup(scr.c_str());
		continue;
	    }
	    stp = (TrainStop *)malloc(sizeof(TrainStop));
	    memset(stp, 0, sizeof(TrainStop));
	   /* stp->minstop = 30;*/
		stp->minstop = 30;
		stp->delta1 = 0;
		stp->delta2 = 0;
		//stp->delta1 = (int)Random_LogNormal(2.76695,1.18678,0.1,180);   //产生3分钟内的随机晚点时间
		//stp->delta2 = (int)Random_LogNormal(2.76695,1.18678,0.1,180);
	    if(*p == '-') {		//doesn't stop 
		while(*++p == ' ' || *p == '\t');
		stp->minstop = 0;
	    } else if(*p == '+') {	// arrival: delta from previous departure
		++p;
		l = wxStrtoul(p, &p, 10);
		if(!t->stops)
		    l += t->timein;
		else
		    l += t->laststop->departure;
	    } else
	        l = parse_time(&p);	//arrival 
	    if(*p == '+') {
		++p;
		stp->minstop = wxStrtoul(p, &p, 10);
	    }
	    if(*p == ',') ++p;
	    while(*p == ' ' || *p == '\t') ++p;

	    if(*p == '-') {
		if(t->exit)		//already processed exit point! 
		    continue;
		t->timeout0 = l;
		t->timeout = l;
		stp->arrival_actual = stp->arrival = stp->arrival0 = t->timeout0;  
		t->laststop->next = stp;
		//t->timeout  = prestopdep + stp->delta1  + t->timeout0 - prestopdep0; 
		stp = (TrainStop *)malloc(sizeof(TrainStop));
		free(stp);
		prestopdep = prestopdep0 =0;
		while(*++p == ' ' || *p == '\t');
		if(*p == ',') ++p;
		while(*p == ' ' || *p == '\t') ++p;
                p1 = wxStrchr(p, '|');
                if(p1) {
                    t->alternateExits = wxStrdup(p1 + 1);
                    *p1 = 0;
                }
		t->exit = wxStrdup(convert_station(p));
		t->laststop->next->station = wxStrdup(convert_station(p));
	/*	stop_temp ->next = t->exit;*/
		continue;
	    }
	    if (*p == '+') {
		stp->departure = wxStrtoul(p + 1, &p, 10);
		if(stp->departure < stp->minstop)
		    stp->departure = stp->minstop;
		if(!stp->minstop) {	// doesn't stop
		    if(!t->stops)
		        l = t->timein;
		    else
		        l = t->laststop->departure;
		}
		stp->departure += l;
	    } else
		stp->departure = parse_time(&p);
	    if(!stp->minstop)
		stp->arrival = stp->departure;
	    else {
		stp->arrival = l;
		//if(stp->departure == stp->arrival)  //         YPX20201129
		    //stp->departure = stp->arrival + stp->minstop;
		 if(stp->minstop > stp->departure - stp->arrival)	// +Rask Ingemann Lambertsen  
		    stp->minstop = stp->departure - stp->arrival;	// +Rask Ingemann Lambertsen

	    }
	    if(*p == DELAY_CHAR) {
		stp->depDelay = (TDDelay *)calloc(sizeof(TDDelay), 1);
		p = parse_delay(++p, stp->depDelay);
	    }
	    if(*p == ',') ++p;
	    while(*p == ' ' || *p == '\t') ++p;
	    stp->station = wxStrdup(convert_station(p));

		stp->arrival_actual=stp->arrival0 = stp->arrival;
		stp->departure_actual=stp->departure0 = stp->departure;
		//if(!t->laststop->station){                       //始发站
		//	prestopdep0 = stp->departure0;

		//	stp->arrival = stp->arrival0;          //到达
		//	if(stp->departure0 !=stp->arrival0)
		//	    stp->departure = prestopdep = stp->departure0 + stp->delta2;      //发车
		//	else
		//	    prestopdep = stp->departure = stp->departure0;
		//	
		//}
		//else {
		//	if(stp->arrival0 != stp->departure0){
		//	stp->arrival = prestopdep + stp->delta1 + stp->arrival0 - prestopdep0; //上一站发车 + 站间扰动 + 站间计划运行时间
		//	stp->departure = prestopdep = stp->arrival + stp->delta2 + stp->departure0 -stp->arrival0;  //本站到站时间 + 站内扰动 + 计划停车时间
		//	prestopdep0 = stp->departure0;
		//	}
		//}

		if(!t->stops){          
		t->stops = stp;
		t->nxtstop = stp;

		}
	    else
		t->laststop->next = stp;
	    t->laststop = stp;

	}
        setEncoding();
	return sched;
}

Train	*parse_newformats(TDFile& schFile)
{
	Train	*t,*q;
	TrainStop *stp;
	wxChar	buff[1024];
    int	l,j;
	long prestopdep0,prestopdep;//  上一停车站的发车时间
	wxChar	*p, *p1;
	wxString fileinc;
	wxChar	*nw, *ne;
	//void	*pmw, *pme;
        Shape   *pmw, *pme;
		
	srand(time(0));

	t = 0;
	while(schFile.ReadLine(buff, sizeof(buff)/sizeof(wxChar))) {
	    if(buff[0] == '.') {
		t = 0;
		continue;
	    }
	    for(l = 0; buff[l]; ++l)
		if(buff[l] == '\t')
		    buff[l] = ' ';
	    while(l && (buff[l - 1] == ' ' || buff[l - 1] == '\t')) --l;
	    buff[l] = 0;
	    if(!buff[0] || buff[0] == '#')
		continue;
            p = buff;
			if(match(&p, wxT("Include:"))) {
		if(!*p)
		    continue;

		TDFile	incFile(p);

		if(incFile.Load())
		    parse_newformats(incFile);
		else {
		    fileinc.Printf(wxT("%s/%s"), dirPath, locase(p));
		    TDFile  incFile1(fileinc);

		    if(!incFile1.Load()) 
			continue;
		    parse_newformats(incFile1);
		}
#if 0
		if(!sched) {
		    sched = t;
		    continue;
		}
		for(t1 = t; t->next; t = t->next);
		t->next = sched;
		sched = t1;
#endif
		t = 0;
		continue;
	    }
            if(match(&p, wxT("Routes:"))) {
                if(!*p)
                    continue;
                gtfs->SetOurRoutes(p);
		continue;
	    }
            if(match(&p, wxT("GTFS:"))) {
                if(!*p)
                    continue;
		sched1 = read_gtfss(sched1, p);
		continue;
	    }
            if(match(&p, wxT("Cancel:"))) {
		if(!*p)
		    continue;
		sched1 = cancelTrains(p, sched1);
		t = 0;
		continue;
	    }
            if(match(&p, wxT("Today:"))) {
		for(l = 0; *p >= '0' && *p <= '9'; ++p)
		    l |= 1 << (*p - '1');
		run_day = l;
		continue;
	    }
            if(match(&p, wxT("Encoding:"))) {
                if(*p)
                    gEncoding.Set(p);
                continue;
            }
            if(match(&p, wxT("Start:"))) {
		start_time = parse_time(&p);
		continue;
	    }
            if(match(&p, wxT("Train:"))) {
		t = find_train(sched1, p);
		if(t)
		    continue;
		t = new Train();
		t->laststop = new TrainStop();
		t->name = wxStrdup(buff + 6);
		t->next = sched1;
		t->type = curtype;
		t->epix = e_train_pmap[curtype];
                t->wpix = w_train_pmap[curtype];
		t->ecarpix = e_car_pmap[curtype];
                t->wcarpix = w_car_pmap[curtype];
		sched1 = t;
		continue;
	    }
	    if(!t) {
                if(match(&p, wxT("Type:"))) {
		    if((l = wxStrtol(p, &p, 0) - 1) >= NTTYPES || l < 0)
			continue;
		    curtype = l;
		    if(!p)
			continue;
		    while(*p == ' ' || *p == '\t') ++p;
		    if(!*p)
			continue;
                    if(*p == '+') {
                        startDelay[curtype] = wxStrtol(++p, &p, 10);
                    }
                    if(*p == '>') {
                        accelRate[curtype] = wxAtof(++p);
                        while(*p && *p != ' ' && *p != '\t')
                            ++p;
                    }
		    while(*p == ' ' || *p == '\t') ++p;
		    if(!*p)
			continue;
		    nw = p;
		    while(*p && *p != ' ' && *p != '\t') ++p;
		    if(!*p)
			continue;
		    *p++ = 0;
		    while(*p == ' ' || *p == '\t') ++p;
		    ne = p;
		    while(*p && *p != ' ' && *p != '\t') ++p;
		    l = *p;
		    *p++ = 0;
//		    if(!(pmw = get_pixmap_file(locase(nw))))
//			continue;
//		    if(!(pme = get_pixmap_file(locase(ne))))
//			continue;
                    pmw = shapeManager.LoadXpm(locase(nw));
                    if(!pmw)
                        continue;
                    pme = shapeManager.LoadXpm(locase(ne));
                    if(pme)
                        continue;
		    w_train_pmap[curtype] = pmw;
		    e_train_pmap[curtype] = pme;
		    if(!l)
			continue;
		    while(*p == ' ' || *p == '\t') ++p;
		    ne = p;
		    while(*p && *p != ' ' && *p != '\t') ++p;
		    l = *p;
		    *p++ = 0;
//		    if(!(pmw = get_pixmap_file(locase(ne))))
//			continue;
                    pmw = shapeManager.LoadXpm(locase(ne));
                    if(!pmw)
                        continue;
		    w_car_pmap[curtype] = pmw;
		    e_car_pmap[curtype] = pmw;
		    if(!l)
			continue;
		    while(*p == ' ' || *p == '\t') ++p;
		    if(!*p)
			continue;
//		    if(!(pme = get_pixmap_file(locase(p))))
//			continue;
                    pme = shapeManager.LoadXpm(locase(p));
                    if(!pme)
                        continue;
		    e_car_pmap[curtype] = pme;
                    continue;
		}
                if(match(&p, wxT("Power:"))) {
		    if((l = wxStrtol(p, &p, 0) - 1) >= NTTYPES || l < 0)
			continue;
                    powerType[l] = power_parse(p);
                    continue;
                }
                if(match(&p, wxT("Gauge:"))) {
		    if((l = wxStrtol(p, &p, 0) - 1) >= NTTYPES || l < 0)
			continue;
                    gauge[l] = wxAtof(p);
                    continue;
                }
		continue;
	    }
            if(match(&p, wxT("Wait:"))) {
		for(nw = p; *nw && *nw != ' '; ++nw);
		if(*nw)
		   *nw++ = 0;
		else
		    nw = 0;
		t->waitfor = wxStrdup(p);
		t->waittime = nw ? wxAtoi(nw) : 60;
		continue;
	    }
            if(match(&p, wxT("StartDelay:"))) {
                t->myStartDelay = wxAtoi(p);
                continue;
            }
            if(match(&p, wxT("AccelRate:"))) {
                t->accelRate = wxAtof(p);
                continue;
            }
            if(match(&p, wxT("Power:"))) {
                t->power = power_parse(p);
                continue;
            }
			
            if(match(&p, wxT("Gauge:"))) {
                t->gauge = wxAtof(p);
                continue;
            }
            if(match(&p, wxT("When:"))) {
		for(l = 0; *p >= '0' && *p <= '9'; ++p)
		    l |= 1 << (*p - '1');
		t->days = l;
		continue;
	    }
            if(match(&p, wxT("Speed:"))) {
		t->maxspeed = wxAtoi(p);
		continue;
	    }
            if(match(&p, wxT("Type:"))) {
		if((l = wxStrtol(p, &p, 0)) - 1 < NTTYPES)
		    t->type = l - 1;
                t->wpix = w_train_pmap[t->type];
                t->epix = e_train_pmap[t->type];
                t->wcarpix = w_car_pmap[t->type];
                t->ecarpix = e_car_pmap[t->type];
		if(!p || !*p)
		    continue;
		while(*p == ' ' || *p == '\t') ++p;
		if(!*p)
		    continue;
		nw = p;
		while(*p && *p != ' ' && *p != '\t') ++p;
		if(!*p)
		    continue;
		*p++ = 0;
		while(*p == ' ' || *p == '\t') ++p;
                pmw = shapeManager.LoadXpm(locase(nw));
                if(!pmw)
                    continue;
                t->wpix = pmw;
//		if((t->wpix = get_pixmap_index(locase(nw))) < 0)
//		    continue;
//		t->epix = get_pixmap_index(locase(p));
                pme = shapeManager.LoadXpm(locase(p));
                if(!pme)
                    continue;
                t->epix = pme;
		continue;
	    }
            if(match(&p, wxT("Stock:"))) {
		t->stock = wxStrdup(p);
		continue;
	    }
            if(match(&p, wxT("Length:"))) {
		t->length = wxStrtol(p, &p, 0);
		t->entryLength = t->length;
		t->tail = (Train *)calloc(sizeof(Train), 1);
		t->ecarpix = e_car_pmap[0];
                t->wcarpix = w_car_pmap[0];
		while(*p == ' ' || *p == '\t') ++p;
		if(!*p)
		    continue;
		ne = p;
		while(*p && *p != ' ' && *p != '\t') ++p;
		l = *p;
		*p++ = 0;
                pmw = shapeManager.LoadXpm(locase(ne)); // get_carpixmap_index(locase(ne));
                if(!pmw)
                    continue;
                t->ecarpix = t->wcarpix = pmw;
		if(!l)
		    continue;
		while(*p == ' ' || *p == '\t') ++p;
		if(!*p)
		    continue;
                pmw = shapeManager.LoadXpm(locase(p)); // get_carpixmap_index(locase(p));
                if(!pmw)
                    continue;
                t->wcarpix = pmw;
		continue;
	    }
			if(match(&p, wxT("Enter:"))) {
		t->timein = parse_time(&p);
		if(*p == DELAY_CHAR) {
		    t->entryDelay = (TDDelay *)calloc(sizeof(TDDelay), 1);
		    p = parse_delay(++p, t->entryDelay);
		}
		if(*p == ',') ++p;
		while(*p == ' ' || *p == '\t') ++p;
                p1 = wxStrchr(p, '|');
                if(p1) {
                    t->alternateEntries = wxStrdup(p1 + 1);
                    *p1 = 0;
                }
		t->entrance = wxStrdup(convert_station(p));
		continue;
	    }
            if(match(&p, wxT("Notes:"))) {
		if(t->nnotes < MAXNOTES)
		    t->notes[t->nnotes++] = wxStrdup(p);
		continue;
	    }
            if(match(&p, wxT("Script:"))) {
		wxString scr;
		while(schFile.ReadLine(buff, sizeof(buff)/sizeof(wxChar))) {
                    for(p = buff + wxStrlen(buff) - 1; p > buff && (p[0] == ' ' || p[0] == '\t'); --p);
                    p[1] = 0;
                    p = skipblk(buff);
		    if(!buff[0] || buff[0] == '#')
			continue;
		    if(!wxStrcmp(buff, wxT("EndScript"))) {
			break;
		    }
		    wxStrcat(buff, wxT("\n"));
		    scr += buff;
		}
		t->stateProgram = wxStrdup(scr.c_str());
		continue;
		}
/***************************time**********************/

	    stp = (TrainStop *)malloc(sizeof(TrainStop));
	    memset(stp, 0, sizeof(TrainStop));
	   /* stp->minstop = 30;*/
		stp->minstop = 30;
		stp->delta1 = 0;
		stp->delta2 = 0;
	    if(*p == '-') {		//doesn't stop 
		while(*++p == ' ' || *p == '\t');
		stp->minstop = 0;
	    } else if(*p == '+') {	// arrival: delta from previous departure
		++p;
		l = wxStrtoul(p, &p, 10);
		if(!t->stops)   //l是始发站还是停站
		    l += t->timein;
		else
		    l += t->laststop->departure;
	    } else
	        l = parse_time(&p);	/* arrival */
	    if(*p == '+') {
		++p;
		stp->minstop = wxStrtoul(p, &p, 10);
	    }
	    if(*p == ',') ++p;
	    while(*p == ' ' || *p == '\t') ++p;
/* 下面是最后一行的操作 */
	    if(*p == '-') {
		if(t->exit)		//already processed exit point! 
		    continue;
		t->timeout = l;
		stp->arrival_actual = stp->arrival = t->timeout0;  
		t->laststop->next = stp;
		//t->timeout  = prestopdep + stp->delta1  + t->timeout0 - prestopdep0; 
		stp = (TrainStop *)malloc(sizeof(TrainStop));
		free(stp);
		prestopdep = prestopdep0 =0;
		while(*++p == ' ' || *p == '\t');
		if(*p == ',') ++p;
		while(*p == ' ' || *p == '\t') ++p;
                p1 = wxStrchr(p, '|');
                if(p1) {
                    t->alternateExits = wxStrdup(p1 + 1);
                    *p1 = 0;
                }
		t->exit = wxStrdup(convert_station(p));
		t->laststop->next->station = wxStrdup(convert_station(p));
		continue;
	    }
	    if (*p == '+') {
		stp->departure = wxStrtoul(p + 1, &p, 10);
		if(stp->departure < stp->minstop)
		    stp->departure = stp->minstop;
		if(!stp->minstop) {	// doesn't stop
		    if(!t->stops)
		        l = t->timein;
		    else
		        l = t->laststop->departure;
		}
		stp->departure += l;
	    } else
		stp->departure = parse_time(&p);
	    if(!stp->minstop)
		stp->arrival = stp->departure;
	    else {
		stp->arrival = l;
		//if(stp->departure == stp->arrival)  //         YPX20201129
		//stp->departure = stp->arrival + stp->minstop;
		 if(stp->minstop > stp->departure - stp->arrival)	// +Rask Ingemann Lambertsen  
		    stp->minstop = stp->departure - stp->arrival;	// +Rask Ingemann Lambertsen

	    }
	    if(*p == DELAY_CHAR) {
		stp->depDelay = (TDDelay *)calloc(sizeof(TDDelay), 1);
		p = parse_delay(++p, stp->depDelay);
	    }
	    if(*p == ',') ++p;
	    while(*p == ' ' || *p == '\t') ++p;
	    stp->station = wxStrdup(convert_station(p));
		stp->arrival_actual= stp->arrival;
		stp->departure_actual= stp->departure;
		if(!t->stops){
		t->stops = stp;
		t->nxtstop = stp;
		}
	    else
		t->laststop->next = stp;
	    t->laststop = stp;
	}
        setEncoding();
		Train *tr1,*tr2;
		TrainStop *s1,*s2; 
		
		for(tr1 = schedule; tr1; tr1 = tr1->next){
			for(tr2 = sched1;tr2; tr2 = tr2->next){    //遍历更新列车的计划时刻表，赋值原计划
				if(!wxStrcmp(tr1->name,tr2->name)){
					tr2->timeout0 = tr1->timeout0;
					s2= tr2->stops;
					for(s1 = tr1->stops;s1;s1=s1->next){
						s2->arrival0 = s1->arrival0;
						s2->departure0 = s1->departure0;
						s2->km_post = s1->km_post;
						s2=s2->next;
					}
				}
			}
		}
		return sched1;
}
void	check_delayed_entries(Train *sched)
{
	Train	*t, *t1;
	Track	*trk, *tk1;
	int	firsttime = 1;
	int	i;
	wxChar	buff[256];

	/*  Check entrance conflicts */

	for(t = sched; t; t = t->next) {
            if(t->isExternal)
                continue;
	    for(t1 = t->next; t1; t1 = t1->next) {
                if(t1->isExternal)
                    continue;
		if(t->timein != t1->timein)
		    continue;
		if(t->days && t1->days && run_day)
		    if(!(t->days & t1->days & run_day))
			continue;
		if(wxStrcmp(t->entrance, t1->entrance))
		    continue;
		for(trk = layout; trk; trk = trk->next)
		    if(trk->type == TRACK && trk->isstation &&
			    !wxStrcmp(t->entrance, trk->station))
			break;
		if(trk)
		    continue;
		if(firsttime) {
		    traindir->layout_error(L("These trains will be delayed on entry:"));
		    traindir->layout_error(wxT("\n"));
		}
		firsttime = 0;
		wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), L("%s and %s both enter at %s on %s"),
			t->name, t1->name, t->entrance, format_time(t->timein));
		wxStrcat(buff, wxT("\n"));
		traindir->layout_error(buff);
	    }
	}
	firsttime = 1;
	for(t = sched; t; t = t->next) {
            if(t->isExternal)
                continue;
	    trk = findStationNamed(t->entrance);
	    if(!trk) {
		wxStrcpy(buff, t->entrance);
		for(i = 0; buff[i] && buff[i] != ' '; ++i);
		buff[i] = 0;
		trk = findStationNamed(buff);
	    }
	    tk1 = findStationNamed(t->exit);
	    if(!tk1) {
		wxStrcpy(buff, t->exit);
		for(i = 0; buff[i] && buff[i] != ' '; ++i);
		buff[i] = 0;
		tk1 = findStationNamed(buff);
	    }
	    if(trk && tk1)
		continue;
	    if(firsttime) {
		traindir->layout_error(L("These trains have unknown entry or exit points:"));
		traindir->layout_error(wxT("\n"));
	    }
	    firsttime = 0;
	    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), L("%s enters from '%s', exits at '%s'"),
                        t->name, t->entrance, t->exit);
	    wxStrcat(buff, wxT("\n"));
	    traindir->layout_error(buff);
	}
	traindir->end_layout_error();
}

static	Path	*find_path(const wxChar *from, const wxChar *to)
{
	Path	*pt;

	for(pt = paths; pt; pt = pt->next) {
	    if(!pt->from || !pt->to || !pt->enter)
		continue;
	    if(sameStation(from, pt->from) && sameStation(to, pt->to))
		return pt;
	}
	return 0;
}

static	void	resolve_path(Train *t)
{
	Path	*pt, *pth;
	TrainStop *ts, *tt, *pts;
	long	t0;
	int	f1;

	if(findStationNamed(t->entrance)) {
	    f1 = 1;
	    goto xit;
	}
        pth = 0;
	f1 = 0;
	for(ts = t->stops; ts; ts = ts->next) {
	    if((pt = find_path(t->entrance, ts->station))) {
                pth = pt; // path from outside to inside
                pts = ts; // stop inside the layout
                break;
	    }
	}
        // select the path with the closes stop outside the layout
	for(tt = t->stops; tt; tt = tt->next) {
            Char *outerStation = tt->station;
            if(findStationNamed(outerStation)) {
                // we reached the first station inside the layout, so stop searching
                break;
            }
	    for(ts = tt->next; ts; ts = ts->next) {
		if((pt = find_path(outerStation, ts->station))) {
                    pth = pt; // path from outside to inside
                    pts = ts; // stop inside the layout
                    break;
		}
	    }
	}
        if(pth) {
	    t->entrance = wxStrdup(pth->enter);
	    t->timein = pts->arrival - pth->times[t->type];
	    f1 = 1;
        }
xit:
        if(f1 && t->timein < 0)         // first stop is close to midnight but path time is longer
            t->timein += 24 * 60 * 60;  // so move time in to the end of this day
	if(findStationNamed(t->exit)) {
	    if(f1)			// both entrance and exit in layout
		return;
	    pth = 0;
	    for(tt = t->stops; tt; tt = tt->next)
		if((pt = find_path(tt->station, t->exit)))
		    pth = pt;
	    if(!pth)
		pth = find_path(t->entrance, t->exit);
	    if(!pth)
		return;
	    t->entrance = wxStrdup(pth->enter);
	    t->timein = t->timeout - pth->times[t->type];
            if(t->timein < 0)
                t->timein += 24 * 60 * 60;
	    return;
	}
	pth = 0;
	for(tt = t->stops; tt; tt = tt->next) {
	    for(ts = tt->next; ts; ts = ts->next) {
		if((pt = find_path(tt->station, ts->station))) {
		    t0 = tt->departure;
		    pth = pt;
		}
	    }
	}
	for(ts = t->stops; ts; ts = ts->next)
	    if((pt = find_path(ts->station, t->exit))) {
		t0 = ts->departure;
		pth = pt;
	    }
	if(pth) {
	    t->exit = wxStrdup(pth->enter);
	    t->timeout = t0 + pth->times[t->type];
	    return;
	}
	if(!f1)
	    return;
	for(ts = t->stops; ts; ts = ts->next)
	    if((pt = find_path(t->entrance, ts->station))) {
		t->exit = wxStrdup(pt->enter);
		t->timeout = t->timein + pt->times[t->type];
		return;
	    }
	if((pt = find_path(t->entrance, t->exit))) {
	    t->exit = wxStrdup(pt->enter);
	    t->timeout = t->timein + pt->times[t->type];
	}
}

void	resolve_paths(Train *schedule)
{
	Train	*t;

	if(!paths)
	    return;
	for(t = schedule; t; t = t->next)
	    resolve_path(t);
}

void	load_paths(const wxChar *name)
{
	Path	*pt;
	wxChar	buff[1024];
	int	l;
	wxChar	*p, *p1;
	int	errors;

	while(paths) {
	    pt = paths->next;
	    if(paths->from) free(paths->from);
	    if(paths->to) free(paths->to);
	    if(paths->enter) free(paths->enter);
	    free(paths);
	    paths = pt;
	}

	TDFile	pthFile(name);

	pthFile.SetExt(wxT(".pth"));
	if(!pthFile.Load())
	    return;
	pt = 0;
	errors = 0;
	while(pthFile.ReadLine(buff, sizeof(buff)/sizeof(wxChar))) {
	    p = skipblk(buff);
	    if(!*p || *p == '#')
		continue;
	    if(!wxStrcmp(p, wxT("Path:"))) {
		if(pt) {			/* end previous entry */
		    if(!pt->from || !pt->to || !pt->enter) {
			++errors;
			paths = pt->next;	/* ignore last entry */
			free(pt);
		    }
		}
		p += 5;
		pt = (Path *)calloc(sizeof(Path), 1);
		pt->next = paths;
		paths = pt;
		continue;
	    }
	    if(!wxStrncmp(p, wxT("From: "), 6))
		pt->from = wxStrdup(skipblk(p + 6));
	    if(!wxStrncmp(p, wxT("To: "), 4))
		pt->to = wxStrdup(skipblk(p + 4));
	    if(!wxStrncmp(p, wxT("Times: "), 7)) {
		p += 7;
		for(p1 = p; *p1 && *p1 != ' '; ++p1);
		if(!*p1)			/* no entry point! */
		    continue;
		*p1++ = 0;
		for(l = 0; l < NTTYPES; ++l) {
		    pt->times[l] = wxStrtol(p, &p, 10) * 60;
		    if(*p == '/' || *p == ',') ++p;
		}
		p = skipblk(p1);
		pt->enter = wxStrdup(p);
	    }
	}
}

bool    load_trains_from_gtfs()
{
        gtfs = new GTFS();
        return gtfs->Load(dirPath);
}

/*void load__sch()
{Train	*t;
	TrainStop *stp;
		wxChar	buff[1024];
	int	l;
	wxChar	*p, *p1;
	int	newformat;

	sched = 0;
	newformat = 0;
	curtype = 0;
        memset(startDelay, 0, sizeof(startDelay));
        for(int i = 0; i < NTTYPES; ++i) { // 3.9
            powerType[i] = 0;
            gauge[i] = 0;
        }

        if(gtfs)
            delete gtfs;
        gtfs = new GTFS();

        gEncoding.Set(wxT("Windows-1252"));

	    TDFile	schFile(name);
	schFile.SetExt(wxT(".sch"));
	schFile.GetDirName(dirPath, sizeof(dirPath)/sizeof(wxChar));
        if(!schFile.Load()) {
            if(!load_trains_from_gtfs())
                return 0;
        } else while(schFile.ReadLine(buff, sizeof(buff)/sizeof(wxChar))) {
	    if(!buff[0])
		continue;
	    if(newformat || !wxStrcmp(buff, wxT("#!trdir"))) {
		newformat = 1;
		t = parse_newformat(schFile);
		if(!t)
		    continue;
		sched = t;
		continue;
	    }
	    if(buff[0] == '#')
		continue;
	    t = new Train();
	    t->next = sched;
	    sched = t;
	    for(p = buff; *p && *p != ','; ++p);
	    if(!*p)
		continue;
	    *p++ = 0;
	    t->name = wxStrdup(buff);
	    t->status = train_READY;
	    t->direction = t->sdirection = (trkdir)wxStrtol(p, &p, 10);
	    if(*p == ',') ++p;
	    t->timein = parse_time(&p);
	    if(*p == ',') ++p;
	    p1 = p;
	    while(*p && *p != ',') ++p;
	    if(!*p)
		continue;
	    *p++ = 0;
	    t->entrance = wxStrdup(p1);
	    t->timeout = parse_time(&p);
	    if(*p == ',') ++p;
	    p1 = p;
	    while(*p && *p != ',') ++p;
	    if(!*p)
		continue;
	    *p++ = 0;
	    t->exit = wxStrdup(p1);
	    t->maxspeed = wxStrtol(p, &p, 10);
	    if(*p == ',') ++p;
	    while(*p) {
		for(p1 = p; *p && *p != ','; ++p);
		if(!*p)
		    continue;
		*p++ = 0;
		stp = (TrainStop *)malloc(sizeof(TrainStop));
		memset(stp, 0, sizeof(TrainStop));
		if(!t->stops)
		    t->stops = stp;
		else
		    t->laststop->next = stp;
		t->laststop = stp;
		stp->station = wxStrdup(p1);
		stp->arrival = parse_time(&p);
		if(*p == ',') ++p;
		stp->departure = parse_time(&p);
		if(*p == ',') ++p;
		stp->minstop = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
	    }
	}

	/* check correctness of schedule 

	l = 0;
	for(t = sched; t; t = t->next) {
	    if(!t->exit) {
		t->exit = wxStrdup(wxT("?"));
		++l;
	    }
	    if(!t->entrance) {
		t->entrance = wxStrdup(wxT("?"));
		++l;
	    }
	}
	if(l)
	    traindir->Error(L("Some train has unknown entry/exit point!"));

        // 3.9: propagate motive power

        for(t = sched; t; t = t->next) {
            if(t->power)
                continue;
            t->power = powerType[t->type]; // either NULL or real power spec.
        }

	load_paths(name);
	resolve_paths(sched);

	sched = sort_schedule(sched);

	for(t = sched; t; t = t->next)
	    if(t->stateProgram)
		t->ParseProgram();

	return sched;
}*/
Speedlim *load_spdlim(const wxChar *name)
{
	    Speedlim  *lim;
		Speedlim  *spd1,*spd2;
		wxChar	  buff[1024];
		wxChar    *p;
		int       l;
		lim = new Speedlim;
		lim = (Speedlim *)malloc(sizeof(Speedlim));
        memset(lim, 0, sizeof(Speedlim));
		spd2 = (Speedlim *)malloc(sizeof(Speedlim));
	    memset(spd2, 0, sizeof(Speedlim));
		spd2=lim;


     //gEncoding.Set(wxT("Windows-1252"));
        TDFile	schFile(name);
	    schFile.SetExt(wxT(".lim"));    //将后缀名由trk改为sch
	    schFile.GetDirName(dirPath, sizeof(dirPath)/sizeof(wxChar));
	  	//lim = new Speedlim();
        if(!schFile.Load()) {     //下载文件到content
                return 0;
		} else while(schFile.ReadLine(buff, sizeof(buff)/sizeof(wxChar))) {
	    if(!buff[0])
		continue;

	    for(l = 0; buff[l]; ++l)   //循环将本行所有\t变为空格
		if(buff[l] == '\t')
		    buff[l] = ' ';
	    while(l && (buff[l - 1] == ' ' || buff[l - 1] == '\t')) --l;
	    buff[l] = 0;
	    if(!buff[0] || buff[0] == '#')
		continue;
            p = buff; 
		spd1 = new Speedlim;
		spd1->LineId = wxStrtol(p, &p, 10);
		if(*p == wxT(' ')) ++p;
		spd1->KmBegin = wxStrtol(p, &p, 10);
		if(*p == wxT(' ')) ++p;
		spd1->KmEnd = wxStrtol(p, &p, 10);
		if(*p == wxT(' ')) ++p;
		spd1->speed = wxStrtol(p, &p, 10);
		spd2->next = spd1;
	    spd2=spd1;
 
		
		}
	spd2->next =NULL;
	lim = lim->next;
	return lim;

}

Train	*load_trains(const wxChar *name)
{
	Train	*t;
	TrainStop *stp;
	wxChar	buff[1024];
	int	l;
	wxChar	*p, *p1;
	int	newformat;
	sched = 0;
	newformat = 0;
	start_time = 0;
	curtype = 0;
        memset(startDelay, 0, sizeof(startDelay));
        for(int i = 0; i < NTTYPES; ++i) { // 3.9
            powerType[i] = 0;
            gauge[i] = 0;
        }

        if(gtfs)
            delete gtfs;
        gtfs = new GTFS();
        gEncoding.Set(wxT("Windows-1252"));
		//gEncoding.Set(wxT("GBK"));
        TDFile	schFile(name);
	schFile.SetExt(wxT(".sch"));    //将后缀名由trk改为sch
	schFile.GetDirName(dirPath, sizeof(dirPath)/sizeof(wxChar));
        if(!schFile.Load()) {     //下载文件到content
            if(!load_trains_from_gtfs())
                return 0;
        } else while(schFile.ReadLine(buff, sizeof(buff)/sizeof(wxChar))) {
	    if(!buff[0])
		continue;
	    if(newformat || !wxStrcmp(buff, wxT("#!trdir"))) {
		newformat = 1;
		t = parse_newformat(schFile);//t=sched; sched 是文件里面的信息
		if(!t)
		    continue;
		sched = t;
		continue;
		}
	    if(buff[0] == '#')
		continue;
	    t = new Train();
	    t->next = sched;
	    sched = t;//迭代
	    for(p = buff; *p && *p != ','; ++p);
	    if(!*p)
		continue;
	    *p++ = 0;
	    t->name = wxStrdup(buff);
	    t->status = train_READY;
	    t->direction = t->sdirection = (trkdir)wxStrtol(p, &p, 10);
	    if(*p == ',') ++p;
	    t->timein = parse_time(&p);
	    if(*p == ',') ++p;
	    p1 = p;
	    while(*p && *p != ',') ++p;
	    if(!*p)
		continue;
	    *p++ = 0;
	    t->entrance = wxStrdup(p1);
	    t->timeout = parse_time(&p);
	    if(*p == ',') ++p;
	    p1 = p;
	    while(*p && *p != ',') ++p;
	    if(!*p)
		continue;
	    *p++ = 0;
	    t->exit = wxStrdup(p1);
	    t->maxspeed = wxStrtol(p, &p, 10);
	    if(*p == ',') ++p;
	    while(*p) {
		for(p1 = p; *p && *p != ','; ++p);
		if(!*p)
		    continue;
		*p++ = 0;
		stp = (TrainStop *)malloc(sizeof(TrainStop));
		memset(stp, 0, sizeof(TrainStop));
		if(!t->stops)
		    t->stops = stp;
		else
		    t->laststop->next = stp;
		t->laststop = stp;
		stp->station = wxStrdup(p1);
		stp->arrival = parse_time(&p);
		if(*p == ',') ++p;
		stp->departure = parse_time(&p);
		if(*p == ',') ++p;
		stp->minstop = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
	    }
	}

	/* check correctness of schedule */

	l = 0;
	for(t = sched; t; t = t->next) {
	    if(!t->exit) {
		t->exit = wxStrdup(wxT("?"));
		++l;
	    }
	    if(!t->entrance) {
		t->entrance = wxStrdup(wxT("?"));
		++l;
	    }
	}
	if(l)
	    traindir->Error(L("Some train has unknown entry/exit point!"));

        // 3.9: propagate motive power

        for(t = sched; t; t = t->next) {
            if(t->power)
                continue;
            t->power = powerType[t->type]; // either NULL or real power spec.
        }

	load_paths(name);
	resolve_paths(sched);

	sched = sort_schedule(sched);

	for(t = sched; t; t = t->next)
	    if(t->stateProgram)
		t->ParseProgram();

	return sched;
}
Train	*update_trains(const wxChar *name)
{
	Train	*t,*q,*a;
	TrainStop *stp,*stp1;
	wxChar	buff[1024];
	int	l;
	wxChar	*p, *p1;
	int	newformat;
	sched1 = 0;
	newformat = 0;
	start_time = 0;
	curtype = 0;
        memset(startDelay, 0, sizeof(startDelay));
        for(int i = 0; i < NTTYPES; ++i) { // 3.9
            powerType[i] = 0;
            gauge[i] = 0;
        }

        if(gtfs)
            delete gtfs;
        gtfs = new GTFS();

        gEncoding.Set(wxT("Windows-1252"));
		//gEncoding.Set(wxT("GBK"));
        TDFile	schFile(name);
	schFile.SetExt(wxT(".sch"));
	schFile.GetDirName(dirPath, sizeof(dirPath)/sizeof(wxChar));
	        if(!schFile.Load()) {
            if(!load_trains_from_gtfs())
			{;}
        } else while(schFile.ReadLine(buff, sizeof(buff)/sizeof(wxChar))) {
	    if(!buff[0])
		continue;
	    if(newformat || !wxStrcmp(buff, wxT("#!trdir"))) {
		newformat = 1;

		t = parse_newformats(schFile);//t=sched; sched 是文件里面的信息
		if(!t)
		    continue;
		sched1 = t;
		continue;
	    }
	    if(buff[0] == '#')
		continue;

	    t = new Train();
	    t->next = sched1;
	    sched1 = t;//迭代
	    for(p = buff; *p && *p != ','; ++p);
	    if(!*p)
		continue;
	    *p++ = 0;
	    t->name = wxStrdup(buff);
	    t->status = train_READY;
	    t->direction = t->sdirection = (trkdir)wxStrtol(p, &p, 10);
	    if(*p == ',') ++p;
	    t->timein = parse_time(&p);
	    if(*p == ',') ++p;
	    p1 = p;
	    while(*p && *p != ',') ++p;
	    if(!*p)
		continue;
	    *p++ = 0;
	    t->entrance = wxStrdup(p1);
	    t->timeout = parse_time(&p);
	    if(*p == ',') ++p;
	    p1 = p;
	    while(*p && *p != ',') ++p;
	    if(!*p)
		continue;
	    *p++ = 0;
	    t->exit = wxStrdup(p1);
	    t->maxspeed = wxStrtol(p, &p, 10);
	    if(*p == ',') ++p;
	    while(*p) {
		for(p1 = p; *p && *p != ','; ++p);
		if(!*p)
		    continue;
		*p++ = 0;
		stp = (TrainStop *)malloc(sizeof(TrainStop));
		memset(stp, 0, sizeof(TrainStop));
		if(!t->stops)
		    t->stops = stp;
		else
		    t->laststop->next = stp;
		t->laststop = stp;
		stp->station = wxStrdup(p1);
		stp->arrival = parse_time(&p);
		if(*p == ',') ++p;
		stp->departure = parse_time(&p);
		if(*p == ',') ++p;
		stp->minstop = wxStrtol(p, &p, 10);
		if(*p == ',') ++p;
		for(q=sched;q;q->next)
		{if(q->name==t->name)
		{ for(stp1=q->stops;stp1;stp1->next)
		{ if(stp1->station==stp->station)
		{   stp1->arrival=stp->arrival;
		stp1->departure=stp->departure;
		stp1->minstop=stp->minstop;}
		}
		}
		}
	    }
	}

	/* check correctness of schedule */

	l = 0;
	for(t = sched1; t; t = t->next) {
	    if(!t->exit) {
		t->exit = wxStrdup(wxT("?"));
		++l;
	    }
	    if(!t->entrance) {
		t->entrance = wxStrdup(wxT("?"));
		++l;
	    }
	}
	if(l)
	    traindir->Error(L("Some train has unknown entry/exit point!"));

        // 3.9: propagate motive power

        for(t = sched1; t; t = t->next) {
            if(t->power)
                continue;
            t->power = powerType[t->type]; // either NULL or real power spec.
        }

	load_paths(name);
	resolve_paths(sched1);
	//q=sched1;
	//f=sched1;
	//a=sched1;
	/*FILE *w =fopen("C://Users//Administrator//Documents//output","w");
	   //while(a)
   //{
      
       //输出链表节点数据到文件output.txt 
	  //{fprintf(w,"%d ",a->name);}
      // a=a->next;        
   //}     
   int o=2;
    fprintf(w,"%d ",o);
   fprintf(w,"\n");
   fclose(w);*/

	/*while(q!=NULL)
	{ q=f->next;
	  free(f);
	  f=q;}*/

	sched = sort_schedule(sched1);

	//for(t = sched; t; t = t->next)
	  //  if(t->stateProgram)
		//t->ParseProgram();

	return sched;
}




/* ================================= */

int	save_layout(const wxChar *name, Track *layout)
{
	wxFFile file;
	Track	*t;
	TextList *tl;
	Itinerary *it;
	int	i;
	int	ch;

	if(!file_create(name, wxT(".trk"), file))
	    return 0;
	for(t = layout; t; t = t->next) {
	    switch(t->type) {
	    case TRACK:
		file.Write(wxString::Format(wxT("0,%d,%d,%d,"), t->x, t->y, t->direction));
		file.Write(wxString::Format(wxT("%d,%d,"), t->isstation, t->length));
		file.Write(wxString::Format(wxT("%d,%d,%d,%d,"), t->wlinkx, t->wlinky,
					t->elinkx, t->elinky));
                for(i = 0; i < NTTYPES; ++i)
                    if(t->speed[i])
                        break;
		if(i < NTTYPES) {   // some types have a speed limit, so save them all
		    ch = '@';

		    for(i = 0; i < NTTYPES; ++i) {
			file.Write(wxString::Format(wxT("%c%d"), ch, t->speed[i]));
			ch = '/';
		    }
		    file.Write(wxT(','));
		}
		if(t->km)
		    file.Write(wxString::Format(wxT(">%d.%d,"), t->km / 1000, t->km % 1000));
		if(t->line_num)
		    file.Write(wxString::Format(wxT("L%d,"), t->line_num));   //ypx20201209
		if(t->isstation && t->station)
		    file.Write(wxString::Format(wxT("%s\n"), t->station));
		else
		    file.Write(wxString::Format(wxT("noname\n")));
		break;

	    case SWITCH:
		file.Write(wxString::Format(wxT("1,%d,%d,%d,"), t->x, t->y, t->direction));
		file.Write(wxString::Format(wxT("%d,%d"), t->wlinkx, t->wlinky));
                if(t->speed[0]) {
		    ch = '@';

		    for(i = 0; i < NTTYPES; ++i) {
			file.Write(wxString::Format(wxT("%c%d"), ch, t->speed[i]));
			ch = '/';
		    }
		    file.Write(wxT(','));
		}
		if(t->km)
		    file.Write(wxString::Format(wxT(">%d.%d,"), t->km / 1000, t->km % 1000));
		if(t->isstation && t->station)
		    file.Write(wxString::Format(wxT("%s\n"), t->station));
		else
		    file.Write(wxString::Format(wxT("noname\n")));
		break;

	    case TSIGNAL:
		file.Write(wxString::Format(wxT("2,%d,%d,%d,"), t->x, t->y,
			t->direction + t->fleeted * 2 +
			(t->fixedred << 8) +
			(t->nopenalty << 9) + (t->signalx << 10) +
			(t->noClickPenalty << 11)));
		file.Write(wxString::Format(wxT("%d,%d"), t->wlinkx, t->wlinky));
		if(t->stateProgram) {
		    for(i = wxStrlen(t->stateProgram); i >= 0; --i)
			if(t->stateProgram[i] == '/' || t->stateProgram[i] == '\\')
			    break;
		    file.Write(wxString::Format(wxT(",@%s"), t->stateProgram + i + 1));
		}
		if(t->station && *t->station)	/* for itineraries */
		    file.Write(wxString::Format(wxT(",%s"), t->station));
		file.Write(wxString::Format(wxT("\n")));
		break;

	    case PLATFORM:
		file.Write(wxString::Format(wxT("3,%d,%d,%d\n"), t->x, t->y, t->direction == W_E ? 0 : 1));
		break;

	    case TEXT:
		file.Write(wxString::Format(wxT("4,%d,%d,%d,%s,"), t->x, t->y, t->direction, t->station));
		file.Write(wxString::Format(wxT("%d,%d,%d,%d"), t->wlinkx, t->wlinky,
						t->elinkx, t->elinky));
		if(t->km)
		    file.Write(wxString::Format(wxT(">%d.%d"), t->km / 1000, t->km % 1000));
		file.Write(wxString::Format(wxT("\n")));
		break;

	    case IMAGE:
		if(!t->station)
		    t->station = wxStrdup(wxT(""));
		for(i = wxStrlen(t->station); i >= 0; --i)
		    if(t->station[i] == '/' || t->station[i] == '\\')
			break;
                if(t->wlinkx && t->wlinky)
		    file.Write(wxString::Format(wxT("5,%d,%d,0,@%d,%d,%s\n"), t->x, t->y,
                            t->wlinkx, t->wlinky, t->station + i + 1));
                else
		    file.Write(wxString::Format(wxT("5,%d,%d,0,%s\n"), t->x, t->y, t->station + i + 1));
		break;

	    case ITIN:
		file.Write(wxString::Format(wxT("8,%d,%d,%d,%s\n"), t->x, t->y, t->direction, t->station));
		break;

	    case TRIGGER:
		file.Write(wxString::Format(wxT("9,%d,%d,%d,"), t->x, t->y, t->direction));
		file.Write(wxString::Format(wxT("%d,%d,%d,%d"), t->wlinkx, t->wlinky,
					t->elinkx, t->elinky));
		ch = ',';
		for(i = 0; i < NTTYPES; ++i) {
		    file.Write(wxString::Format(wxT("%c%d"), ch, t->speed[i]));
		    ch = '/';
		}
		file.Write(wxString::Format(wxT(",%s\n"), t->station));
		break;
	    }
	}
	for(tl = track_info; tl; tl = tl->next)
	    file.Write(wxString::Format(wxT("6,0,0,0,%s\n"), tl->txt));

	for(it = itineraries; it; it = it->next) {
	    file.Write(wxString::Format(wxT("7,0,0,0,%s,%s,%s,"), it->name,
					it->signame, it->endsig));
	    if(it->nextitin)
		file.Write(wxString::Format(wxT("@%s,"), it->nextitin));
	    for(i = 0; i < it->nsects; ++i)
		file.Write(wxString::Format(wxT("%d,%d,%d,"), it->sw[i].x, it->sw[i].y,
					it->sw[i].switched));
	    file.Write(wxString::Format(wxT("\n")));
	}

	for(t = layout; t; t = t->next) {
	    switch(t->type) {
	    case ITIN:
	    case IMAGE:

		if(t->_flashingIcons[0]) {
		    file.Write(wxString::Format(wxT("(attributes %d,%d\nicons:"), t->x, t->y));
		    for(int x = 0; ;) {
			file.Write(t->_flashingIcons[x]);
			++x;
			if(x >= MAX_FLASHING_ICONS || !t->_flashingIcons[x])
			    break;
			file.Write(wxT(","));
		    }
		    file.Write(wxT("\n)\n"));
		}

	    case TRACK:
	    case SWITCH:
	    case TRIGGER:

		if(t->stateProgram) {
		    file.Write(wxString::Format(wxT("(script %d,%d\n%s)\n"), t->x, t->y, t->stateProgram));
		}
                if(t->power && *t->power) {
                    file.Write(wxString::Format(wxT("(attributes %d,%d\npower:%s\n)\n"), t->x, t->y, t->power));
                }
                if(t->gauge) {
                    file.Write(wxString::Format(wxT("(attributes %d,%d\ngauge:%s\n)\n"), t->x, t->y, t->gauge));
                }
				/*if(t->km_post && *t->km_post) {
                    file.Write(wxString::Format(wxT("(attributes %d,%d\nkm_post:%s\n)\n"), t->x, t->y, t->km_post));
                }   */ //  YPX0916

	    case TSIGNAL:
                if((unsigned long)t->_lockedBy == (unsigned long)0xcdcdcdcd) {
                    t->_lockedBy = 0;
                }
                if(t->_lockedBy) {
                    file.Write(wxString::Format(wxT("(attributes %d,%d\nlocked %s\n)\n"), t->x, t->y, t->_lockedBy));
                }
                if(t->flags & TFLG_DONTSTOPSHUNTERS) {
                    file.Write(wxString::Format(wxT("(attributes %d,%d\ndontstopshunters\n)\n"), t->x, t->y));
                }
                if(t->_intermediate) {
                    file.Write(wxString::Format(wxT("(attributes %d,%d\nintermediate %d\n)\n"),
                        t->x, t->y, t->_intermediate));
                }
                // fall through

	    case TEXT:
		if(t->invisible) {
		    file.Write(wxString::Format(wxT("(attributes %d,%d\nhidden\n)\n"), t->x, t->y));
		}
		if(t->_dwellTime)
                    file.Write(wxString::Format(wxT("(attributes %d,%d\ndwellTime: %d\n)\n"), t->x, t->y, t->_dwellTime));
                continue;
	    }
	}

	SaveSwitchBoards(file);

	file.Close();
	layout_modified = 0;
	return 1;
}

#define	MAXSHORTNAME 10

static void	short_station_name(wxChar *d, const wxChar *s)
{
	int	i;

	for(i = 0; *s && *s != ' ' && i < MAXSHORTNAME - 1; ++i)
	    *d++ = *s++;
	*d = 0;
}

void    setEncoding()
{
#if wxUSE_UNICODE
        char *cp = strdup(gEncoding._sValue.mb_str());
#else
        char *cp = strdup(gEncoding._sValue.c_str());
#endif
        sprintf(MIME_JSON, "application/json; charset=%s", cp);
        sprintf(MIME_TEXT, "text/plain; charset=%s", cp);
        sprintf(MIME_HTML, "text/html; charset=%s", cp);
        sprintf(MIME_XML,  "text/xml; charset=%s", cp);
        free(cp);
}


#define OUT_wxHTML  0
#define OUT_TEXT    1
#define	OUT_HTML    2

void	schedule_status_print(HtmlPage& page, int outFormat)
{
	wxString	buff;
	wxString	buffs[13];
	const wxChar	*cols[13];
	Train		*t;
	TrainStop	*ts;
	const wxChar	*eol;

	if(outFormat == OUT_TEXT) {
	    *page.content = wxT("");
	    eol = wxT("\n");
	} else {
	    eol = wxT("<br>\n");
	    page.StartPage(L("仿真结果"));
	    page.AddCenter();
	    page.AddLine(wxT("<table><tr><td valign=top>"));
	}
	buff.Printf(wxT("%s : %s%s"),  L("时间"), format_time(current_time), eol);
	page.Add(buff);
	if(run_day) {
	    buff.Printf(wxT("%s : %s%s"),  L("Day"), format_day(run_day), eol);
	    page.Add(buff);
	}
	buff.Printf(wxT("%s : %ld%s"), L("Total points"), run_points, eol);
	//page.Add(buff);
	buff.Printf(wxT("%s : %d%s"),  L("发车延误时间"), total_delay / 60, eol);
	page.Add(buff);
	buff.Printf(wxT("%s : %ld%s"), L("总晚点时间"), total_late, eol);
	page.Add(buff);
	if(outFormat == OUT_wxHTML) {
	    buff.Printf(wxT("<br><a href=\"save_perf_text\">%s</a>"), L("Save as text"));
	    page.AddLine(buff);
	    if(performance_hide_canceled) {
		buff.Printf(wxT("<br><a href=\"performance_toggle_canceled\">%s</a>"), L("(show canceled trains)"));
	    } else {
		buff.Printf(wxT("<br><a href=\"performance_toggle_canceled\">%s</a>"), L("(hide canceled trains)"));
	    }
	    page.AddLine(buff);
//	    buff.Printf(wxT("<br><a href=\"save_perf_HTML\">%s</a>"), L("Save as HTML"));
//	    page.AddLine(buff);
	}
//	fprintf(fp, "%s : %ld\n",     LCS("Total performance penalties"), performance());
//	fprintf(fp, "</blockquote>\n");
	if(outFormat == OUT_TEXT) {
	    buff.Printf(wxT("%-40s : %5d x %3d = %-5d\n"), L("Wrong destinations"),
		perf_tot.wrong_dest, perf_vals.wrong_dest,
		perf_tot.wrong_dest * perf_vals.wrong_dest);
	    page.Add(buff);
	    buff.Printf(wxT("%-40s : %5d x %3d = %-5d\n"), L("Late trains"),
		perf_tot.late_trains, perf_vals.late_trains,
		perf_tot.late_trains * perf_vals.late_trains);
	    buff.Printf(wxT("%-40s : %5d x %3d = %-5d\n"), L("Wrong platforms"),
		perf_tot.wrong_platform, perf_vals.wrong_platform,
		perf_tot.wrong_platform * perf_vals.wrong_platform);
	    page.Add(buff);
	    buff.Printf(wxT("%-40s : %5d x %3d = %-5d\n"), L("Commands denied"),
		perf_tot.denied, perf_vals.denied,
		perf_tot.denied * perf_vals.denied);
	    page.Add(buff);
	    buff.Printf(wxT("%-40s : %5d x %3d = %-5d\n"), L("Trains waiting at signals"),
		perf_tot.waiting_train, perf_vals.waiting_train,
		perf_tot.waiting_train * perf_vals.waiting_train);

	    buff.Printf(wxT("%-40s : %5d x %3d = %-5d\n"), L("Thrown switches"),
		perf_tot.thrown_switch, perf_vals.thrown_switch,
		perf_tot.thrown_switch * perf_vals.thrown_switch);
	    page.Add(buff);
	    buff.Printf(wxT("%-40s : %5d x %3d = %-5d\n"), L("Cleared signals"),
		perf_tot.cleared_signal, perf_vals.cleared_signal,
		perf_tot.cleared_signal * perf_vals.cleared_signal);
	    page.Add(buff);
	    buff.Printf(wxT("%-40s : %5d x %3d = %-5d\n"), L("Reversed trains"),
		perf_tot.turned_train, perf_vals.turned_train,
		perf_tot.turned_train * perf_vals.turned_train);
	    page.Add(buff);
	    buff.Printf(wxT("%-40s : %5d x %3d = %-5d\n"), L("Missed station stops"),
		perf_tot.nmissed_stops, perf_vals.nmissed_stops,
		perf_tot.nmissed_stops * perf_vals.nmissed_stops);
	    page.Add(buff);
	    buff.Printf(wxT("%-40s : %5d x %3d = %-5d\n\n"), L("Wrong stock assignments"),
		perf_tot.wrong_assign, perf_vals.wrong_assign,
		perf_tot.wrong_assign * perf_vals.wrong_assign);
	    page.Add(buff);
	} else {
	    page.Add(wxT("</td><td valign=top>\n"));
	    buff.Printf(wxT("<table><tr><td valign=top>%s</td>\n"), L("方向错误"));
	    page.Add(buff);
	    buff.Printf(wxT("<td align=right valign=top>%d&nbsp;x</td><td align=right valign=top>%d&nbsp;=</td>"),
			    perf_tot.wrong_dest, perf_vals.wrong_dest);
	    page.Add(buff);
	    buff.Printf(wxT("<td>%d</td></tr>\n"), perf_tot.wrong_dest * perf_vals.wrong_dest);
	    page.Add(buff);
	    buff.Printf(wxT("<tr><td valign=top>%s</td>\n"), L("晚点列车数"));
	    page.Add(buff);
	    buff.Printf(wxT("<td align=right valign=top>%d&nbsp;x</td><td align=right valign=top>%d&nbsp;=</td>"),
			    perf_tot.late_trains, perf_vals.late_trains);
	    page.Add(buff);
	    buff.Printf(wxT("<td valign=top>%d</td></tr>\n"), perf_tot.late_trains * perf_vals.late_trains);
	    page.Add(buff);
	    buff.Printf(wxT("<tr><td valign=top>%s</td>\n"), L("站台错误"));
	    page.Add(buff);
	    buff.Printf(wxT("<td valign=top align=right>%d&nbsp;x</td><td valign=top align=right>%d&nbsp;=</td>"),
			    perf_tot.wrong_platform, perf_vals.wrong_platform);
	    page.Add(buff);
	    buff.Printf(wxT("<td valign=top>%d</td></tr>\n"), perf_tot.wrong_platform * perf_vals.wrong_platform);
	    page.Add(buff);
	    buff.Printf(wxT("<tr><td valign=top>%s</td>\n"), L("指令未被执行"));
	    page.Add(buff);
	    buff.Printf(wxT("<td align=right valign=top>%d&nbsp;x</td><td align=right valign=top>%d&nbsp;=</td>"),
			    perf_tot.denied, perf_vals.denied);
	    page.Add(buff);
	    buff.Printf(wxT("<td valign=top>%d</td></tr>\n"), perf_tot.denied * perf_vals.denied);
	    page.Add(buff);
	    buff.Printf(wxT("<tr><td valign=top>%s</td>\n"), L("列车在信号机前等待"));
	    page.Add(buff);
	    buff.Printf(wxT("<td align=right valign=top>%d&nbsp;x</td><td align=right valign=top>%d&nbsp;=</td>"),
			    perf_tot.waiting_train, perf_vals.waiting_train);
	    page.Add(buff);
	    buff.Printf(wxT("<td valign=top>%d</td></tr>\n"), perf_tot.waiting_train * perf_vals.waiting_train);
	    page.Add(buff);

	    buff.Printf(wxT("</table></td><td valign=top><table>"));
	    page.Add(buff);
	    buff.Printf(wxT("<tr><td valign=top>%s</td>\n"), L("道岔错误"));
	    page.Add(buff);
	    buff.Printf(wxT("<td align=right valign=top>%d&nbsp;x</td><td align=right valign=top>%d&nbsp;=</td>"),
			    perf_tot.thrown_switch, perf_vals.thrown_switch);
	    page.Add(buff);
	    buff.Printf(wxT("<td valign=top>%d</td></tr>\n"), perf_tot.thrown_switch * perf_vals.thrown_switch);
	    page.Add(buff);
	    buff.Printf(wxT("<tr><td valign=top>%s</td>\n"), L("信号机错误"));
	    page.Add(buff);
	    buff.Printf(wxT("<td align=right valign=top>%d&nbsp;x</td><td align=right valign=top>%d&nbsp;=</td>"),
			    perf_tot.cleared_signal, perf_vals.cleared_signal);
	    page.Add(buff);
	    buff.Printf(wxT("<td valign=top>%d</td></tr>\n"), perf_tot.cleared_signal * perf_vals.cleared_signal);
	    page.Add(buff);
	    buff.Printf(wxT("<tr><td valign=top>%s</td>\n"), L("反向行车"));
	    page.Add(buff);
	    buff.Printf(wxT("<td align=right valign=top>%d&nbsp;x</td><td align=right valign=top>%d&nbsp;=</td>"),
			    perf_tot.turned_train, perf_vals.turned_train);
	    page.Add(buff);
	    buff.Printf(wxT("<td valign=top>%d</td></tr>\n"), perf_tot.turned_train * perf_vals.turned_train);
	    page.Add(buff);
	    buff.Printf(wxT("<tr><td valign=top>%s</td>\n"), L("错过停车站"));
	    page.Add(buff);
	    buff.Printf(wxT("<td align=right valign=top>%d&nbsp;x</td><td align=right valign=top>%d&nbsp;=</td>"),
			    perf_tot.nmissed_stops, perf_vals.nmissed_stops);
	    page.Add(buff);
	    buff.Printf(wxT("<td valign=top>%d</td></tr>\n"), perf_tot.nmissed_stops * perf_vals.nmissed_stops);
	    page.Add(buff);
	    buff.Printf(wxT("<tr><td valign=top>%s</td>\n"), L("列车分配错误"));
	    page.Add(buff);
	    buff.Printf(wxT("<td align=right valign=top>%d&nbsp;x</td><td align=right valign=top>%d&nbsp;=</td>"),
			    perf_tot.wrong_assign, perf_vals.wrong_assign);
	    page.Add(buff);
	    buff.Printf(wxT("<td valign=top>%d</td></tr>\n"), perf_tot.wrong_assign * perf_vals.wrong_assign);
	    page.Add(buff);
	    page.Add(wxT("</table></td></tr></table>"));
	    page.AddLine(wxT("</center>"));
	}

	cols[0] = L("Train");
	cols[1] = L("Enters");
	cols[2] = L("At");
	cols[3] = L("Exits");
	cols[4] = L("Before");
	cols[5] = L("Delay");
	cols[6] = L("Late");
	cols[7] = L("Status");
	cols[8] = L("Time");
	cols[9] = L("Position");
	cols[10] = L("Speed");
	cols[11] = 0;
	if(outFormat == OUT_TEXT) {
	} else
	    page.StartTable(&cols[0]);

	cols[0] = buffs[0];
	cols[1] = buffs[1];
	cols[2] = buffs[2];
	cols[3] = buffs[3];
	cols[4] = buffs[4];
	cols[5] = buffs[5];
	cols[6] = buffs[6];
	cols[7] = buffs[7];
	cols[8] = 0;//buffs[8];
	/*cols[9] = buffs[9];
	cols[10] = buffs[10];
	cols[11] = 0;*/

        TrainInfo   info;
	for(t = schedule; t; t = t->next) {
	    if(performance_hide_canceled && is_canceled(t))
		continue;
	    print_train_info(t);
            t->Get(info);
	    buffs[0].Printf(wxT("<a href=\"traininfopage %s\">%s</a>"), t->name, t->name);
	    cols[0] = buffs[0];
	    buffs[1] = t->entrance;
	    cols[1] = buffs[1];
	    cols[2] = info.entering_time;
	    buffs[3] = t->exit;
	    cols[3] = buffs[3];
	    cols[4] = info.leaving_time;
	    cols[5] = info.current_delay;
	    cols[6] = info.current_late;
	    cols[7] = info.current_status;
       
		//******************************新加的*****************************************//
	/*	cols[8] = info.current_status;
		cols[9] = info.current_status;
		cols[10] = info.current_status;*/

	 /*if(outFormat == OUT_TEXT) {
			wxString	buff_Record;
		   buff_Record.Printf(wxT("Recording:		X	Y	SPEED\r\n"));
		page.Add(buff_Record);
		for (int k=0;k<PosRecordid;k++)
		{
			buff_Record.Printf(wxT("\t %16d %16d %16d\r\n"),PosReg[k][4],PosReg[k][3],PosReg[k][2]);
			page.Add(buff_Record);
		}
	 }*/

	 	 if(outFormat == OUT_TEXT) {
			wxString	buff_Record;
		    buff_Record.Printf(wxT("Recording:		 RecordID	 iTimeCounter	travelled	speed	Current_time	x	y	name\r\n"));
		page.Add(buff_Record);
		for (int k=0;k<PosRecordid;k++)
		{
			buff_Record.Printf(wxT("\t	%16d%16d%16d%16d%16d%16d%16d%16d;\r\n"),k,PosReg[k][0],PosReg[k][1],PosReg[k][2],PosReg[k][3],PosReg[k][4],PosReg[k][5],PosReg[k][6]);
			page.Add(buff_Record);
		}

	 }

	 ///buffs[8]=PosReg[][4];
	 //   cols[8] = buffs[8];

		//buffs[9].Printf(wxT("curtime: %l"),b[10240]);
		////buffs[9]=PosReg[][3];
		//cols[9] = buffs[8];

		//buffs[10] .Printf(wxT("speed: %f"), c[10240]);
		////buffs[10]=PosReg[][2];
		//cols[10] = buffs[10];
		 //**********************************************新加的********************************************//
	    if(outFormat == OUT_TEXT) {
			buff.Printf(wxT("%-64.64s  %-20.20s  %12s  %-20.20s  %12s %3s %3s %s\n"),
		    t->name, cols[1], cols[2], cols[3], cols[4], cols[5], cols[6], cols[7]);
			page.Add(buff);
	    } else
		page.AddTableRow(cols);
	    cols[0] = wxT("&nbsp;");
	    cols[1] = wxT("&nbsp;");
	    cols[2] = wxT("&nbsp;");
	    cols[3] = wxT("&nbsp;");
	    cols[4] = wxT("&nbsp;");
	    cols[5] = wxT("&nbsp;");
	    buff = wxT("");
	    for(ts = t->stops; ts; ts = ts->next) {
		if(outFormat == OUT_TEXT && t->status != train_READY) {
		    wxString s;

		    s.Printf(wxT("   %d: %s"), ts->delay, ts->station);
		    if(buff.length() + s.length() > 127) {
			page.Add(buff);
			page.Add(wxT("\n"));
			buff = wxT("");
		    }
		    buff += s;
		    continue;
		}
		if(!ts->delay)
		    continue;
		buffs[6].Printf(wxT("%c%d"), ts->delay > 0 ? '+' : ' ', ts->delay);
		cols[6] = buffs[6];
		cols[7] = ts->station;
		page.AddTableRow(cols);
	    }
	    if(buff.length() > 0) {
		page.Add(buff);
		page.Add(wxT("\n\n"));
	    }
	}
	if(outFormat != OUT_TEXT) {
	    page.EndTable();
	    page.EndPage();
	}
}


void	show_schedule_status(HtmlPage& dest)
{
	schedule_status_print(dest, OUT_wxHTML);
}

void	save_schedule_status(HtmlPage& dest)
{
	schedule_status_print(dest, OUT_TEXT);
}


//void	save_train_status(HtmlPage& dest)
//{
//	train_status_print(dest, OUT_TEXT);
//}


void	performance_toggle_canceled()
{
	performance_hide_canceled = !performance_hide_canceled;
}

void	train_print(Train *t, HtmlPage& page)
{
	TrainStop	*ts;
	wxString	buff;
	int		i;
	const wxChar	*beg, *end;
	int		status;
	wxString	buffs[7];
	const wxChar	*cols[7];

	buff.Printf(wxT("%s %s"), L("Train"), t->name);
	page.StartPage(buff);
	cols[0] = L("Station");
	cols[1] = L("Arrival");
	cols[2] = L("Departure");
	cols[3] = L("Min.stop");
	cols[4] = 0 /*"Stopped";
	cols[5] = "Delay";
	cols[6] = 0*/;
	page.StartTable(cols);
	cols[0] = buffs[0];
	cols[1] = buffs[1];
	cols[2] = buffs[2];
	cols[3] = buffs[3];
	cols[4] = 0 /*buffs[4];
	cols[5] = buffs[5];
	cols[6] = 0 */;

	status = 0;
	beg = wxT(""), end = wxT("");
	for(ts = t->stops; ts; ts = ts->next) {
	    if(ts->arrival >= t->timein/* && findStation(ts->station)*/) {
		if(status == 0) {
		    buffs[0].Printf(wxT("<b><a href=\"stationinfopage %s\">%s</a></b>"), t->entrance, t->entrance), cols[0] = buffs[0];
		    buffs[1].Printf(wxT("&nbsp;")), cols[1] = buffs[1];
		    buffs[2].Printf(wxT("<b>%s</b>"), format_time(t->timein)), cols[2] = buffs[2];
		    buffs[3].Printf(wxT("&nbsp;")), cols[3] = buffs[3];
		    cols[4] = 0;
		    page.AddTableRow(cols);
		    status = 1;
		}
	    }
	    if(ts->arrival > t->timeout && status == 1) {
		buffs[0].Printf(wxT("<b><a href=\"stationinfopage %s\">%s</a></b>"), t->exit, t->exit), cols[0] = buffs[0];
		buffs[1].Printf(wxT("<b>%s</b>"), format_time(t->timeout)), cols[1] = buffs[1];
		buffs[2].Printf(wxT("&nbsp;")), cols[2] = buffs[2];
		buffs[3].Printf(wxT("&nbsp;")), cols[3] = buffs[3];
		cols[4] = 0;
		page.AddTableRow(cols);
		status = 2;
	    }
	    buffs[0] = wxString(ts->station).BeforeFirst(wxT('@'));
	    if(findStationNamed(buffs[0]))
		beg = wxT("<b>"), end = wxT("</b>");
	    else
		beg = wxT(""), end = wxT("");
	    buffs[0].Printf(wxT("%s<a href=\"stationinfopage %s\">%s</a>%s"), beg, ts->station, ts->station, end), cols[0] = buffs[0];
	    if(!ts->arrival || !ts->minstop)
		cols[1] = wxT("&nbsp;");
	    else
		buffs[1].Printf(wxT("%s%s%s"), beg, format_time(ts->arrival), end), cols[1] = buffs[1];
	    buffs[2].Printf(wxT("%s%s%s"), beg, format_time(ts->departure), end), cols[2] = buffs[2];
	    if(status != 1)
		cols[3] = wxT("&nbsp;");
	    else
	        buffs[3].Printf(wxT("%ld"), ts->minstop), cols[3] = buffs[3];
/*	    sprintf(cols[4], ts->stopped ? "<b>Yes</b>" : "<b>No</b>");
	    sprintf(cols[5], "%s%ld%s", beg, (long)ts->delay, end);
*/	    cols[4] = 0;
	    page.AddTableRow(cols);
	}
	if(status < 1) {
	    buffs[0].Printf(wxT("<b><a href=\"stationinfopage %s\">%s</a></b>"), t->entrance, t->entrance), cols[0] = buffs[0];
	    buffs[1].Printf(wxT("&nbsp;")), cols[1] = buffs[1];
	    buffs[2].Printf(wxT("<b>%s</b>"), format_time(t->timein)), cols[2] = buffs[2];
	    buffs[3].Printf(wxT("&nbsp;")), cols[3] = buffs[3];
	    cols[4] = 0;
	    page.AddTableRow(cols);
	    ++status;
	}
	if(status < 2) {
	    buffs[0].Printf(wxT("<b><a href=\"stationinfopage %s\">%s</a></b>"), t->exit, t->exit), cols[0] = buffs[0];
	    buffs[1].Printf(wxT("<b>%s</b>"), format_time(t->timeout)), cols[1] = buffs[1];
	    buffs[2].Printf(wxT("&nbsp;")), cols[2] = buffs[2];
	    buffs[3].Printf(wxT("&nbsp;")), cols[3] = buffs[3];
	    cols[4] = 0;
	    page.AddTableRow(cols);
	}
	page.EndTable();
	page.Add(wxT("<blockquote><blockquote>\n"));
	if(t->days) {
	    buff.Printf(wxT("%s : "), L("Runs on"));
	    for(i = 0; i < 7; ++i)
		if(t->days & (1 << i))
		    buff += wxString::Format(wxT("%d"), i+1);
	    page.AddLine(buff);
	}
	if(t->nnotes) {
	    buff.Printf(wxT("%s: "), L("Notes"));
	    page.Add(buff);
	    for(status = 0; status < t->nnotes; ++status) {
		buff.Printf(wxT("%s.<br>\n"), t->notes[status]);
		page.Add(buff);
	    }
	}
	page.AddLine(wxT("</blockquote></blockquote>"));
	page.EndPage();
}

//  Test if there's any condition that prevents us to save
//  the game, such as trains currently on an X switch.

bool    can_save_game()
{
        Train   *train;
        Track *track;

        for(train = schedule; train; train = train->next) {
            if(!(track = train->position))
                continue;
            if(track->type == SWITCH) {
                switch(track->direction) {
                case 8:
                case 9:
                case 16:
                case 17:
                    return false;
                }
            } else if(track->type == TRACK) {
                switch(track->direction) {
                case N_NE_S_SW:
                case N_NW_S_SE:
                case XH_NW_SE:
                case XH_SW_NE:
                case X_X:
                case X_PLUS:
                    return false;
                }
            }
        }
        return true;
}


int	save_game(const wxChar *name)
{
	wxFFile file;
	Track	*t;
	Train	*tr;
	TrainStop *ts;
	int	i;

	if(!file_create(name, wxT(".sav"), file))
	    return 0;
	file.Write(wxString::Format(wxT("%s\n"), current_project.c_str()));

	file.Write(wxString::Format(wxT("%d,%ld,%d,%d,%d,%d,%d,%d,%d,%ld\n"),
		cur_time_mult, start_time, show_speeds._iValue,
		show_blocks._iValue, beep_on_alert._iValue, run_points, total_delay,
		total_late, time_mult, current_time));

	/* Save the state of every switch */

	for(t = layout; t; t = t->next) {
	    if(t->type != SWITCH || !t->switched)
		continue;
	    file.Write(wxString::Format(wxT("%d,%d,%d\n"), t->x, t->y, t->switched));
	}
	file.Write(wxString::Format(wxT("\n")));

	/* Save the state of every signal */

	for(t = layout; t; t = t->next) {
	    if(t->type != TSIGNAL)
		continue;
	    Signal *sig = (Signal *)t;
	    if(!sig->IsClear() && !t->nowfleeted)
		continue;
	    file.Write(wxString::Format(wxT("%d,%d,%d,%d"), t->x, t->y,
			sig->IsClear(), t->nowfleeted != 0));
            if(sig->_intermediate)
                file.Write(wxString::Format(wxT("/%d"), sig->_nReservations));
	    if(sig->_currentState)
		file.Write(wxString::Format(wxT(",%s"), sig->_currentState));
	    file.Write(wxString::Format(wxT("\n")));
	}
	file.Write(wxString::Format(wxT("\n")));

	/* Save the position of every train */

	for(tr = schedule; tr; tr = tr->next) {
	    if(tr->status == train_READY && (!tr->entryDelay || !tr->entryDelay->nSeconds))
		continue;
	    file.Write(wxString::Format(wxT("%s\n"), tr->name));
	    file.Write(wxString::Format(wxT("  %d,%d,%s\n"), tr->status, tr->direction,
				tr->exited ? tr->exited : wxT("")));
	    file.Write(wxString::Format(wxT("  %d,%d,%g,%d,%d,%g,%d,%d,%d"), tr->timeexited,
				tr->wrongdest, tr->curspeed, tr->maxspeed,
				tr->curmaxspeed, tr->trackpos, tr->timelate,
				tr->timedelay, tr->timered));
	    if(tr->entryDelay) {
		file.Write(wxString::Format(wxT(",%d"), tr->entryDelay->nSeconds));
	    }
	    file.Write(wxT("\n"));
	    file.Write(wxString::Format(wxT("  %ld,%d,%g,%g,%d\n"), tr->timedep, 0, //tr->pathpos,
				tr->pathtravelled, tr->disttostop, tr->shunting));
	    if(!tr->stoppoint)
		file.Write(wxString::Format(wxT("  0,0,0,")));
	    else
		file.Write(wxString::Format(wxT("  %d,%d,%g,"), tr->stoppoint->x, tr->stoppoint->y,
				tr->disttoslow));
	    if(!tr->slowpoint)
		file.Write(wxString::Format(wxT("0,0")));
	    else
		file.Write(wxString::Format(wxT("%d,%d"), tr->slowpoint->x, tr->slowpoint->y));
	    file.Write(wxString::Format(wxT(",%d\n"), tr->needfindstop));
	    if(tr->fleet && tr->fleet->_size) {
		/* file.Write(wxString::Format("  %d,%d\n", tr->fleet->x, tr->fleet->y)); */
		file.Write(wxT("  "));
		for(i = 0; i < tr->fleet->_size; ++i) {
		    t = tr->fleet->TrackAt(i);
		    if(i)
			file.Write(wxT(','));
		    file.Write(wxString::Format(wxT("%d,%d"), t->x, t->y));
		}
		file.Write(wxT('\n'));
	    } else
		file.Write(wxString::Format(wxT("  0,0\n")));	    /* length has fleet info at end */
	    if(tr->position)
		file.Write(wxString::Format(wxT("  %d,%d"), tr->position->x, tr->position->y));
	    else
		file.Write(wxString::Format(wxT("  0,0")));
	    file.Write(wxString::Format(wxT(",%d,%d,%d\n"), tr->waittime, tr->flags, tr->arrived));
	    file.Write(wxString::Format(wxT("  %d,%s\n"), tr->oldstatus,
				tr->outof ? tr->outof->station : wxT("")));

            if(tr->startDelay) {
                file.Write(wxString::Format(wxT(":startDelay %d\n"), tr->startDelay));
            }

            /* Save status of each stop */

	    for(ts = tr->stops; ts; ts = ts->next) {
//		if(ts->stopped || ts->delay)
		if(!ts->depDelay)
		    file.Write(wxString::Format(wxT("    %s,%d,%d\n"),
			    ts->station, ts->stopped, ts->delay));
		else
		    file.Write(wxString::Format(wxT("    %s,%d,%d,%d\n"),
			    ts->station, ts->stopped, ts->delay, ts->depDelay->nSeconds));
	    }
	    if(tr->tail && tr->tail->path) {
		Train *tail = tr->tail;

		file.Write(wxString::Format(wxT(".\n")));	/* marks beginning of tail path */
		file.Write(wxString::Format(wxT("  %s\n"), tr->stopping ? tr->stopping->station : wxT("")));
		if(tr->length) {
		    /* save the length. This may be different than
		     * the length specified in the sch file because
		     * it may have been changed by a split/merge operation.
		     */
		    file.Write(wxString::Format(wxT("=length %d\n"), tr->length));
		    file.Write(wxString::Format(wxT("=icons %d %d\n"), tr->ecarpix, tr->wcarpix));
		}
		if(tail->fleet && tail->fleet->_size) {
		    for(i = 0; i < tail->fleet->_size; ++i) {
			t = tail->fleet->TrackAt(i);
			file.Write(wxString::Format(wxT("%c%d,%d"), i ? ',' : '!', t->x, t->y));
		    }
		    file.Write(wxT('\n'));
		}
		file.Write(wxString::Format(wxT("  %d,%g,%d,%d"), !tail->position ? -1 : 0, //tail->pathpos,
				tail->trackpos, tail->tailentry, tail->tailexit));
		for(i = 0; i < tail->path->_size; ++i) {
		    t = tail->path->TrackAt(i);
		    file.Write(wxString::Format(wxT(",%d,%d,%d"), t->x, t->y, tail->path->FlagAt(i)));
		}
	    }
	    file.Write(wxT('\n'));
	}
	file.Write(wxT(".\n"));

	/* save white tracks (to allow merging trains) */
	for(t = layout; t; t = t->next)
	    if((t->type == TRACK || t->type == SWITCH) && t->fgcolor == color_white)
		break;

	if(t) {	    /* we found a white track */
	    file.Write(wxT("(white tracks\n"));
	    for(t = layout; t; t = t->next) {
		if((t->type == TRACK || t->type == SWITCH) && t->fgcolor == color_white)
		    file.Write(wxString::Format(wxT("%d,%d\n"), t->x, t->y));
	    }
	    file.Write (wxT(")\n"));
	}

        // 3.9j: save tracks with dwell time
	for(t = layout; t; t = t->next)
            if(t->_leftDwellTime)
		break;

	if(t) {	    // we found an element with dwell time
	    file.Write(wxT("(dwell\n"));
	    for(t = layout; t; t = t->next) {
                if(t->_leftDwellTime)
                    file.Write(wxString::Format(wxT("%d,%d,%d\n"), t->x, t->y, t->_leftDwellTime));
	    }
	    file.Write (wxT(")\n"));
	}

	/* Save the position of every stranded train */

	for(tr = stranded; tr; tr = tr->next) {
	    file.Write (wxT("(stranded\n"));
	    file.Write(wxString::Format(wxT("%d,%d,%d,%d,%d,%d,%d,%d,%d"),
		tr->type, tr->position->x, tr->position->y,
		tr->direction, tr->ecarpix, tr->wcarpix,
		tr->maxspeed, tr->curmaxspeed,
		tr->length));
	    if(tr->length) {
		if(tr->tail && tr->tail->path) {
		    int	f;
		    const wxChar *sep = wxT("");

		    file.Write(wxString::Format(wxT(",%d\n"), tr->tail->path->_size));
		    for(i = 0; i < tr->tail->path->_size; ++i) {
			t = tr->tail->path->TrackAt(i);
			f = tr->tail->path->FlagAt(i);
			file.Write(wxString::Format(wxT("%s%d,%d,%d"), sep, t->x, t->y, f));
			sep = wxT(",");
		    }
		    file.Write(wxT('\n'));
		} else
		    file.Write(wxT(",0\n"));
	    } else
		file.Write(wxT('\n'));
	    file.Write(wxT(")\n"));
	}

	int	m;
	file.Write(wxT("(late minutes\n"));
	for(i = m = 0; i < 24 * 60; ++i) {
	    file.Write(wxString::Format(wxT(" %d"), late_data[i]));
	    if(++m == 15) {	// 15 values per line
		file.Write(wxT("\n"));
		m = 0;
	    }
	}
	file.Write (wxT(")\n"));

	file.Write(wxString::Format(wxT("%d,%d,%d,%d,%d\n"), run_day, terse_status, status_on_top,
		show_seconds._iValue, signal_traditional));
	file.Write(wxString::Format(wxT("%d,%d\n"), auto_link, show_grid._iValue));
	file.Write(wxString::Format(wxT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"),
	    perf_tot.wrong_dest, perf_tot.late_trains, perf_tot.thrown_switch,
	    perf_tot.cleared_signal, perf_tot.denied, perf_tot.turned_train,
	    perf_tot.waiting_train, perf_tot.wrong_platform,
	    perf_tot.ntrains_late, perf_tot.ntrains_wrong,
	    perf_tot.nmissed_stops, perf_tot.wrong_assign));
	file.Write(wxString::Format(wxT("%d\n"), hard_counters));
	file.Write(wxString::Format(wxT("%d\n"), show_canceled._iValue));
	file.Write(wxString::Format(wxT("%d\n"), show_links));
	file.Write(wxString::Format(wxT("%d\n"), beep_on_enter._iValue));
	file.Write(wxString::Format(wxT("%d\n"), bShowCoord));
	file.Write(wxString::Format(wxT("%d\n"), show_icons._iValue));
	file.Write(wxString::Format(wxT("%d\n"), show_tooltip));
	file.Write(wxString::Format(wxT("%d\n"), show_scripts));
	file.Write(wxString::Format(wxT("%d\n"), random_delays));
	file.Write(wxString::Format(wxT("%d\n"), link_to_left));
	file.Write(wxString::Format(wxT("%d\n"), play_synchronously));

	file.Close();
	return 1;
}


void	position_tail(Train *tr)
{
	Train	*tail;

	// moved from above
	if((tail = tr->tail) && tail->path) {
	    tail->position = 0;
//	    if(tr->status == train_ARRIVED) {
//		Vector_delete(tail->path);
//		tail->path = 0;
//	    } else {
		colorPartialPath(tail->path, ST_RED, 0); //tail->pathpos + 1);
		if(tr->path) {
		    colorPath(tr->path, ST_GREEN);
		    tr->position->fgcolor = conf.fgcolor;
		}
//		if(tail->pathpos >= 0 && tail->pathpos < tail->path->_size)
		if(tail->path->_size > 0)
		    tail->position = tail->path->TrackAt(0); //tail->pathpos);
//		else
//		    tail->pathpos = 0;
///?		if(notOnTrack)
///?		    tr->status = tr->curspeed ? train_RUNNING : train_STOPPED;
///?		notOnTrack = 0;
//	    }
            // sometimes the saved tail path does not include all
            // track elements that the train's path has. This causes
            // some tracks to be left colored red because the tail "skips"
            // them when it is advanced. The following loop tries to correct
            // this situation, even though the real problem is clearly
            // somewhere else (the tail's path should always contain all
            // elements of the head's path).
            int i;
            if(tr->path) {      // if we are not exiting...
                for(i = 0; i < tr->path->_size; ++i) {
                    Track *trk = tr->path->TrackAt(i);
                    if(tail->path->Find(trk) < 0)
                        tail->path->Add(trk, tr->path->FlagAt(i));
                }
            }
	}
}


void	clear_delays()
{
	Train	*trn;
	TrainStop   *ts;

	for(trn = schedule; trn; trn = trn->next) {
	    if(trn->entryDelay)
		trn->entryDelay->nSeconds = 0;
	    for(ts = trn->stops; ts; ts = ts->next) {
		if(ts->depDelay)
		    ts->depDelay->nSeconds = 0;
	    }
	    trn->flags &= ~(TFLG_ENTEREDLATE | TFLG_GOTDELAYATSTOP);
	}
}

void	restore_game(const wxChar *name)
{
	FILE		*fp1;
	const wxChar	*buffptr;
	wxChar		buff[1024];
	wxChar		*p;
	int	x, y;
	int	notOnTrack;
	Track	*t;
	Train	*tr, *tail;
	TrainStop *ts;

	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s.sav"), name);
	TDFile fp(buff);
	if(!(fp.Load())) {
#if wxUSE_UNICODE
	    perror(wxSafeConvertWX2MB(buff));	/* No wxPerror()? */
#else
	    perror(buff);
#endif
	    return;
	}
	clear_delays();
	wxStrcpy(buff, wxT("load "));
	buffptr = getline(&fp);
	wxStrncat(buff, buffptr, sizeof(buff)/sizeof(wxChar) - 6);
	if(wxStrstr(buff, wxT(".zip")) || wxStrstr(buff, wxT(".ZIP")))
	    ;
	else if(!wxStrstr(buff, wxT(".trk")) && !wxStrstr(buff, wxT(".TRK")))
	    wxStrcat(buff, wxT(".trk"));
	if(!(fp1 = wxFopen(buff + 5, wxT("r")))) {		// path not there
	    p = (wxChar *) (buffptr + wxStrlen(buffptr));	// try isolating only the file name
	    while(--p > buffptr && *p != '\\' && *p != '/' && *p != ':');
	    if(p > buffptr)
		wxStrcpy(buff + 5, p + 1);
	} else
	    fclose(fp1);
	// trainsim_cmd(buff);
	traindir->OpenFile(buff + 5, true);

        int     _show_speeds;
        int     _show_blocks;
        int     _beep_on_alert;

	buffptr = getline(&fp);
	wxSscanf(buffptr, wxT("%d,%ld,%d,%d,%d,%d,%d,%d,%d,%ld"),
		&cur_time_mult, &start_time, &_show_speeds,
		&_show_blocks, &_beep_on_alert, &run_points, &total_delay,
		&total_late, &time_mult, &current_time);
        show_speeds.Set(_show_speeds != 0);
        show_blocks.Set(_show_blocks != 0);
        beep_on_alert.Set(_beep_on_alert != 0);

	/* reload state of all switches */

	while((buffptr = getline(&fp))) {
	    if(!buffptr[0])
		break;
	    x = wxStrtol(buffptr, &p, 0);
	    if(*p == ',') ++p;
	    y = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    if(!(t = findSwitch(x, y)))
		continue;
	    t->switched = wxAtoi(p);
	    if(t->switched)
		change_coord(t->x, t->y);
	}

	/* reload state of all signals */

	while((buffptr = getline(&fp))) {
	    Signal  *sig;

	    if(!buffptr[0])
		break;
	    x = wxStrtol(buffptr, &p, 0);
	    if(*p == ',') ++p;
	    y = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    if(!(sig = findSignal(x, y)))
		continue;
	    sig->status = wxStrtol(p, &p, 0) == 1 ? ST_GREEN : ST_RED;
	    if(*p == ',') ++p;
	    sig->nowfleeted = wxStrtol(p, &p, 0);
            if(*p == '/') {
                ++p;
                sig->_nReservations = wxStrtol(p, &p, 0);
            }
            if(sig->_intermediate && sig->nowfleeted)
                sig->fleeted = 1;

	    if(*p == ',') ++p;
	    if(*p)
		sig->_currentState = wxStrdup(p); // this is a small memory leak!
	    if(!sig->IsApproach() && sig->IsClear())
		signal_unlock(sig);
	    change_coord(sig->x, sig->y);
	}

	/* reload state of all trains */

	while((buffptr = getline(&fp))) {
	    if(!buffptr[0] || buffptr[0] == '.')
		break;			/* end of file */
	    tr = findTrainNamed(buffptr);
	    if(!tr) {
		/* the train could not be found in the schedule.
		 * Warn the user, and ignore all lines up to the
		 * next empty line.
		 */
		do {
		    buffptr = getline(&fp);
		} while(buffptr && buffptr[0] && buffptr[0] != '.');
		continue;
	    }
	    /* second line */
	    buffptr = getline(&fp);
	    for(p = (wxChar *) buffptr; *p == ' '; ++p);
	    tr->status = (trainstat)wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    tr->direction = (trkdir)wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    if(*p)
		tr->exited = wxStrdup(p);

	    /* third line */
	    buffptr = getline(&fp);
	    for(p = (wxChar *) buffptr; *p == ' '; ++p);
	    tr->timeexited = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    tr->wrongdest = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
//	    tr->curspeed = wxStrtol(p, &p, 0);
            Char lbuf[64];
            for(x = 0; x < (sizeof(lbuf) - 1) && *p && *p != ','; ++x)
                lbuf[x] = *p++;
            lbuf[x] = 0;
            wxSscanf(lbuf, wxT("%lg"), &tr->curspeed);
            while(*p && *p != ',') ++p;
	    if(*p == ',') ++p;
	    tr->maxspeed = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    tr->curmaxspeed = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    //tr->trackpos = wxStrtol(p, &p, 0);
            for(x = 0; x < (sizeof(lbuf) - 1) && *p && *p != ','; ++x)
                lbuf[x] = *p++;
            lbuf[x] = 0;
            wxSscanf(lbuf, wxT("%lg"), &tr->trackpos);
	    if(*p == ',') ++p;
	    tr->timelate = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    tr->timedelay = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    tr->timered = wxStrtol(p, &p, 0);
	    if(*p == ',') {
		int nDelEntry = wxStrtol(++p, &p, 0);
		if(tr->entryDelay)
		    tr->entryDelay->nSeconds = nDelEntry;
	    }

	    /* fourth line */
	    buffptr = getline(&fp);
	    for(p = (wxChar *) buffptr; *p == ' '; ++p);
	    tr->timedep = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    /* tr->pathpos = */ wxStrtol(p, &p, 0); // backward compatibility
	    if(*p == ',') ++p;
	    tr->pathtravelled = wxAtof(p); // wxStrtol(p, &p, 0);
            while(*p && *p != ',') ++p;
	    if(*p == ',') ++p;
	    tr->disttostop = wxAtof(p); // wxStrtol(p, &p, 0);
            while(*p && *p != ',') ++p;
	    if(*p == ',') {
		++p;
		tr->shunting = wxStrtol(p, &p, 0);
	    }

	    /* fifth line */
	    buffptr = getline(&fp);
	    for(p = (wxChar *) buffptr; *p == ' '; ++p);
	    x = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    y = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    if(!(tr->stoppoint = findTrack(x, y)))
		tr->stoppoint = findSwitch(x, y);
	    tr->disttoslow = wxAtof(p); //wxStrtol(p, &p, 0);
            while(*p && *p != ',') ++p;
	    if(*p == ',') ++p;
	    x = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    y = wxStrtol(p, &p, 0);
	    if(!(tr->slowpoint = findTrack(x, y)))
		tr->slowpoint = findSwitch(x, y);
	    if(*p == ',') {
		++p;
		tr->needfindstop = wxStrtol(p, &p, 0);
	    }

	    /* sixth line */
	    buffptr = getline(&fp);
	    for(p = (wxChar *) buffptr; *p == ' '; ++p);
	    while(*p) {		/* list of fleeting signals */
	      x = wxStrtol(p, &p, 0);
	      if(*p == ',') ++p;
	      y = wxStrtol(p, &p, 0);
/*	    tr->fleet = findSignal(x, y);	*/
	      if(x && y) {
		if(!tr->fleet)
		  tr->fleet = new_Vector();
		tr->fleet->Add(findSignal(x, y), 0); // TODO
	      }
	    }

	    /* seventh line */
	    notOnTrack = 0;
	    buffptr = getline(&fp);
	    for(p = (wxChar *) buffptr; *p == ' '; ++p);
	    x = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    y = wxStrtol(p, &p, 0);
	    if(!(tr->position = findTrack(x, y)))
		if(!(tr->position = findSwitch(x, y))) {
		    switch(tr->status) {
		    case train_READY:
		    case train_ARRIVED:
		    case train_DERAILED:
		    case train_DELAY:
			break;

		    case train_WAITING:
		    case train_STOPPED:
		    case train_RUNNING:
			notOnTrack = 1;
		    }
		}
	    if(*p == ',') ++p;
	    tr->waittime = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    tr->flags = wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    tr->arrived = wxStrtol(p, &p, 0);

	    /* eighth line */
	    buffptr = getline(&fp);
	    for(p = (wxChar *) buffptr; *p == ' '; ++p);
	    tr->oldstatus = (trainstat)wxStrtol(p, &p, 0);
	    if(*p == ',') ++p;
	    if(*p)
		tr->outof = findStation(p);

            if(!buffptr)
                break;

            /* nineth line */
	    while((buffptr = getline(&fp))) {
                if(!wxStrncmp(buffptr, wxT(":startDelay"), 11)) {
                    tr->startDelay = wxStrtol(buffptr + 11, &p, 0);
                    continue;
                }
		if(!buffptr[0] || buffptr[0] == '.')
		    break;
		if(!(p = (wxChar *)wxStrchr(buffptr, ',')))
		    continue;
		*p++ = 0;
		for(ts = tr->stops; ts; ts = ts->next)
		    if(!wxStrcmp(ts->station, buffptr + 4))
			break;
		if(!ts)
		    continue;
		ts->stopped = wxStrtol(p, &p, 0);
		if(*p == ',') ++p;
		ts->delay = wxStrtol(p, &p, 0);
		if(*p) {
		    int nSecDelay = wxStrtol(++p, &p, 0);
		    if(ts->depDelay)
			ts->depDelay->nSeconds = nSecDelay;
		}
	    }
            if(!buffptr)
                break;
	    if(buffptr[0] == '.') {	/* tail path info present */
		buffptr = getline(&fp);
		if(!buffptr[0] || buffptr[0] == '.')
		    break;
		if(!(tail = tr->tail))	/* maybe length was removed in .sch */ {
		    tr->tail = tail = (Train *)calloc(sizeof(Train), 1);
                    tr->ecarpix = e_car_pmap[tr->type];
                    tr->wcarpix = w_car_pmap[tr->type];
		}
		for(p = (wxChar *) buffptr; *p == ' '; ++p);
		if(*p)			/* stopping at station name present */
		    tr->stopping = findStation(p);
		buffptr = getline(&fp);
		for(p = (wxChar *) buffptr; *p == ' '; ++p);
		while(*p == '=') {
		    if(!wxStrncmp(++p, wxT("length"), 6)) {
			tr->length = wxStrtol(p + 6, &p, 0);
		    } else if(!wxStrncmp(p, wxT("icons"), 5)) {
//			tr->ecarpix = wxStrtol(p + 5, &p, 0);
//			tr->wcarpix = wxStrtol(p, &p, 0);
                    }
		    buffptr = getline(&fp);
		    for(p = (wxChar *) buffptr; *p == ' '; ++p);
		}
		if(*p == '!') {
		    ++p;
		    while(*p) {		/* list of fleeting signals */
		      x = wxStrtol(p, &p, 0);
		      if(*p == ',') ++p;
		      y = wxStrtol(p, &p, 0);
		      if(x && y) {
			if(!tail->fleet)
			  tail->fleet = new_Vector();
			tail->fleet->Add(findSignal(x, y), 0);	// TODO
		      }
		      if(*p == ',') ++p;
		    }
		    buffptr = getline(&fp);
		    for(p = (wxChar *) buffptr; *p == ' '; ++p);
		}
		/*tail->pathpos = */ wxStrtol(p, &p, 0);    // backward compatibility
		if(*p == ',') ++p;
		//tail->trackpos = wxStrtol(p, &p, 0);
                for(x = 0; x < (sizeof(lbuf) - 1) && *p && *p != ','; ++x)
                    lbuf[x] = *p++;
                lbuf[x] = 0;
                wxSscanf(lbuf, wxT("%lg"), &tail->trackpos);
		if(*p == ',') ++p;
		tail->tailentry = wxStrtol(p, &p, 0);
		if(*p == ',') ++p;
		tail->tailexit = wxStrtol(p, &p, 0);
		while(*p == ',') {
		    x = wxStrtol(p + 1, &p, 0);
		    if(*p == ',') ++p;
		    y = wxStrtol(p, &p, 0);
		    if(!tail->path)
			tail->path = new_Vector();
		    if(!(t = findTrack(x, y)))
			if(!(t = findSwitch(x, y)))
			    t = findText(x, y);
		    if(!t) {		/* maybe layout changed? */
			if(tail->path)	/* disable length for this train */
			    Vector_delete(tail->path);
			tail->path = 0;
			tr->tail = 0;
			tr->length = 0;
			break;
		    }
		    if(*p == ',') ++p;
		    tail->path->Add(t, wxStrtol(p, &p, 0));
		}
                if(tr->status == train_DELAY && tail && tail->path) {
                    Vector_delete(tail->path);
                    tail->path = 0;
                }
	    }
//	    if(notOnTrack) {
//		do_alert(wxT("Train is not on track!"));
//		new_train_status(tr, train_DERAILED);
//	    }
	    update_schedule(tr);
	}

	while((buffptr = getline(&fp)) && buffptr[0] == '(') {
	    if(!wxStrncmp(buffptr, wxT("(white tracks"), 13)) {
		while((buffptr = getline(&fp)) && buffptr[0] != ')') {
		    wxSscanf(buffptr, wxT("%d,%d"), &x, &y);
		    if(!(t = findTrack(x, y)))
			t = findSwitch(x, y);
		    if(t) {
			t->fgcolor = color_white;
			change_coord(x, y);
		    }
		}
		continue;
	    }
	    if(!wxStrncmp(buffptr, wxT("(stranded"), 9)) {
		while((buffptr = getline(&fp)) && buffptr[0] != ')') {
		    Train *stk = new Train();
		    stk->next = stranded;
		    stranded = stk;
		    stk->name = wxStrdup(wxT(""));
		    p = (wxChar *) buffptr;
		    stk->type = wxStrtoul(p, &p, 0);
		    if(*p == ',') ++p;
		    x = wxStrtoul(p, &p, 0);
		    if(*p == ',') ++p;
		    y = wxStrtoul(p, &p, 0);
		    stk->position = findTrack(x, y);
		    if(!stk->position)
			stk->position = findSwitch(x, y);
		    stk->flags = TFLG_STRANDED;
		    stk->status = train_ARRIVED;
		    if(*p == ',') ++p;
		    stk->direction = (trkdir)wxStrtoul(p, &p, 0);
		    if(*p == ',') ++p;
		    /* stk->ecarpix = */ wxStrtoul(p, &p, 0);
		    if(*p == ',') ++p;
		    /* stk->wcarpix = */ wxStrtoul(p, &p, 0);
		    if(*p == ',') ++p;
		    stk->maxspeed = wxStrtoul(p, &p, 0);
		    if(*p == ',') ++p;
		    stk->curmaxspeed = wxStrtoul(p, &p, 0);
		    if(*p == ',') ++p;
		    stk->length = wxStrtoul(p, &p, 0);
		    if(stk->length) {
			int	tailLength, l, f;
			int	pathLength = 0;

			if(*p == ',') ++p;
			tailLength = wxStrtoul(p, &p, 0);
			if(tailLength) {
			    if(!(buffptr = getline(&fp)))
				break;
			    p = (wxChar *) buffptr;
			    stk->tail = new Train();
			    stk->tail->path = new_Vector();
			    for(l = 0; l < tailLength; ++l) {
				if(*p == ',') ++p;
				x = wxStrtoul(p, &p, 0);
				if(*p == ',') ++p;
				y = wxStrtoul(p, &p, 0);
				if(*p == ',') ++p;
				f = wxStrtoul(p, &p, 0);
				if(!(t = findTrack(x, y)))
				    t = findSwitch(x, y);
				if(!t)
				    break;
				t->fgcolor = color_orange;
				change_coord(x, y);

				stk->tail->path->Add(t, f);
				pathLength += t->length;
			    }
			    stk->tail->length = pathLength;
			    stk->tail->position = stk->tail->path->TrackAt(0);
			}
//			stk->position = stk->path->TrackAt(0);
		    }
		    stk->position->fgcolor = color_black;
		}
		continue;
	    }
	    if(!wxStrncmp(buffptr, wxT("(late minutes"), 13)) {
		x = 0;
		while((buffptr = getline(&fp)) && buffptr[0] != ')') {
		    p = (wxChar *)buffptr;
		    while(*p) {
			late_data[x % (24 * 60)] = wxStrtoul(p, &p, 0);
			++x;
		    }
		}
		continue;
	    }
	    if(!wxStrncmp(buffptr, wxT("(dwell"), 6)) {
		while((buffptr = getline(&fp)) && buffptr[0] != ')') {
		    x = wxStrtoul(buffptr, &p, 0);
		    if(*p == ',') ++p;
		    y = wxStrtoul(p, &p, 0);
		    if(*p == ',') ++p;
	            if((t = findTrack(x, y)) || (t = findSwitch(x, y)) ||
		        (t = (Track *)findSignal(x, y)) || (t = findText(x, y)) ||
		        (t = findPlatform(x, y)) || (t = findImage(x, y)) ||
		        (t = findTrackType(x, y, ITIN)) || (t = findTrackType(x, y, TRIGGER))) {
                            t->_leftDwellTime = wxStrtoul(p, &p, 0);
                    }
                }
                continue;
            }
	}
	if(buffptr) {
            int _show_seconds;
            //int _signal_traditional;
	    wxSscanf(buffptr, wxT("%d,%d,%d,%d,%d"), &run_day, &terse_status, &status_on_top,
		&_show_seconds, &signal_traditional);
            show_seconds.Set(_show_seconds != 0);
	}

        if((buffptr = getline(&fp))) {
            int _show_grid;
	    wxSscanf(buffptr, wxT("%d,%d"), &auto_link, &_show_grid);
            show_grid.Set(_show_grid != 0);
        }
	memset(&perf_tot, 0, sizeof(perf_tot));
	if((buffptr = getline(&fp)))
	    wxSscanf(buffptr, wxT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
	    &perf_tot.wrong_dest, &perf_tot.late_trains, &perf_tot.thrown_switch,
	    &perf_tot.cleared_signal, &perf_tot.denied, &perf_tot.turned_train,
	    &perf_tot.waiting_train, &perf_tot.wrong_platform,
	    &perf_tot.ntrains_late, &perf_tot.ntrains_wrong,
	    &perf_tot.nmissed_stops, &perf_tot.wrong_assign);
	if((buffptr = getline(&fp)) && *buffptr)
	    hard_counters = wxAtoi(buffptr);
	if((buffptr = getline(&fp)) && *buffptr)
	    show_canceled.Set(buffptr);
	if((buffptr = getline(&fp)) && *buffptr)
	    show_links = wxAtoi(buffptr);
	if((buffptr = getline(&fp)) && *buffptr)
	    beep_on_enter.Set(buffptr);
	if((buffptr = getline(&fp)) && *buffptr)
	    bShowCoord = wxAtoi(buffptr) != 0;
	if((buffptr = getline(&fp)) && *buffptr)
	    show_icons.Set(buffptr);
	if((buffptr = getline(&fp)) && *buffptr)
	    show_tooltip = wxAtoi(buffptr) != 0;
	if((buffptr = getline(&fp)) && *buffptr)
	    show_scripts = wxAtoi(buffptr) != 0;
	if((buffptr = getline(&fp)) && *buffptr)
	    random_delays = wxAtoi(buffptr) != 0;
	if((buffptr = getline(&fp)) && *buffptr)
	    link_to_left = wxAtoi(buffptr) != 0;
	if((buffptr = getline(&fp)) && *buffptr)
	    play_synchronously = wxAtoi(buffptr) != 0;

	/* needs to be here, after we reloaded the list
	 * of stranded rolling stock, because it might
	 * reduce the path a shunting train can travel.
	 */

	// first need to position trains
	for(tr = schedule; tr; tr = tr->next) {
	    if(tr->shunting || (tr->flags & TFLG_MERGING))
		continue;
	    if(tr->position) {
		tr->path = findPath(tr->position, tr->direction);
    //	    tr->pathpos = 1;
		if(tr->path && tr->path->_size > 1) {
		    Track *tt = tr->path->TrackAt(1);
		    if(tt->fgcolor != color_white) {
			colorPath(tr->path, ST_GREEN);
		    }
		}
		tr->position->fgcolor = conf.fgcolor;
	    }
	    position_tail(tr);
	}
	// then need to position shunting material
	// beacause we need to know the position
	// of material we are merging to
	for(tr = schedule; tr; tr = tr->next) {
	    if(!tr->shunting && !(tr->flags & TFLG_MERGING))
		continue;
	    if(!tr->position)
		continue;

	    int i;
	    Train *trn;

	    tr->path = findPath(tr->position, tr->direction);
//	    tr->pathpos = 1;
	    if(tr->path) {
		// 0 the the position of the train
		for(i = 1; i < tr->path->_size; ++i) {
		    t = tr->path->TrackAt(i);
		    if(!(trn = findTrain(t->x, t->y))) {
			if(!(trn = findTail(t->x, t->y)))
			    if(!(trn = findStranded(t->x, t->y)))
				trn = findStrandedTail(t->x, t->y);
		    }
		    if(trn) {
			tr->merging = trn;
			tr->flags |= TFLG_MERGING;
			tr->path->_size = i;
			trn->flags |= TFLG_WAITINGMERGE;
			break;
		    }
		    // if conf.fgcolor, the path was clear when we saved the game
		    if(t->fgcolor  == conf.fgcolor)
			t->fgcolor = color_green;
		    // else the path must have been colored white by (white tracks) above
		    if(t->fgcolor != color_white && t->fgcolor != color_green)
			break;	// impossible (should be caught by findStranded above)
		}
	    }
	    tr->position->fgcolor = conf.fgcolor;
//	    position_tail(tr);
	    Train	*tail;

	    // moved from above
	    if((tail = tr->tail) && tail->path) {
		tail->position = 0;
		for(i = 0; i < tail->path->_size; ++i) {
		    t = tail->path->TrackAt(i);
		    if(t == tr->position)
			break;
		    t->fgcolor = color_red;
			t->fgcolor = color_bise;//lrg 20210215
		}
		if(tail->path->_size > 0)
		    tail->position = tail->path->TrackAt(0);
	    }
	}
	compute_train_numbers();
}

void	print_track_info(HtmlPage& page)
{
	wxString	buff;
	TextList *tl;

	buff.Printf(wxT("%s : %s"), L("Territory"), current_project.c_str());
	page.StartPage(buff);
	page.AddRuler();
	page.Add(wxT("<blockquote>\n"));
	for(tl = track_info; tl; tl = tl->next) {
	    buff.Printf(wxT("%s\n"), tl->txt);
	    page.Add(buff);
	}
	page.EndPage();
}

static	const wxChar	*en_headers[] = { wxT("Station Name"), wxT("Coordinates"), wxT("&nbsp;"), wxT("Entry/Exit"), wxT("Coordinates"), 0 };
static	const wxChar	*headers[sizeof(en_headers)/sizeof(en_headers[0])];

void	print_entry_exit_stations(HtmlPage& page)
{
	Track	**stations = get_station_list();
	Track	**entry_exit = get_entry_list();
	Track	*trk;
	int	i, j;
	wxString	buff;

	if(!headers[0])
	    localizeArray(headers, en_headers);

	page.StartPage(L("Stations and Entry/Exit Points"));
	page.Add(wxT("<blockquote>\n"));
	page.StartTable(headers);
	for(i = j = 0; (stations && stations[i]) || (entry_exit && entry_exit[j]); ) {
	    page.Add(wxT("<tr><td>"));
	    if(stations && (trk = stations[i])) {
		buff.Printf(wxT("<a href=\"stationinfopage %s\">%s</a>"), trk->station, trk->station);
		page.Add(buff);
		page.Add(wxT("</td><td>"));
		buff.Printf(wxT("%d, %d"), trk->x, trk->y);
		page.Add(buff);
		++i;
	    } else {
		page.Add(wxT("&nbsp;</td><td>&nbsp;"));
	    }
	    page.Add(wxT("</td><td>&nbsp;&nbsp;&nbsp;</td>\n<td>"));
	    if(entry_exit && (trk = entry_exit[j])) {
		buff.Printf(wxT("<a href=\"stationinfopage %s\">%s</a>"), trk->station, trk->station);
		page.Add(buff);
		page.Add(wxT("</td><td>"));
		buff.Printf(wxT("%d, %d"), trk->x, trk->y);
		page.Add(buff);
		++j;
	    } else {
		page.Add(wxT("&nbsp;</td><td>&nbsp;"));
	    }
	    page.Add(wxT("</td></tr>\n"));
	}
	page.EndTable();
	page.EndPage();
}

/*	Set default preferences		*/

void default_prefs(void)
{
	terse_status = 1;
	status_on_top = 1;
	beep_on_alert.Set(true);
	beep_on_enter.Set(false);
	show_speeds.Set(true);
	auto_link = 1;
	link_to_left = 0;
	show_grid.Set(false);
	show_blocks.Set(true);
	show_seconds.Set(false);
	show_icons.Set(true);
	signal_traditional = 0;
	hard_counters = 0;
	random_delays = 1;
	save_prefs = 1;
	bShowCoord = true;
}

