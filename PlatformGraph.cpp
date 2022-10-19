/*	PlatformGraph.cpp - Created by Giampiero Caprino

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
#include <stdlib.h>

#if !defined(__unix__) && !defined(__WXMAC__)
#include <malloc.h>
#endif

#include "wx/wx.h"
#include "wx/image.h"
#include "wx/dcbuffer.h"

#include "trsim.h"
#include "Canvas.h"
#include "Traindir3.h"
#include "PlatformGraphView.h"

#define	STATION_WIDTH 140
#define	KM_WIDTH 5
#define	HEADER_HEIGHT 20
#define	MAXWIDTH (2 * 60 * 24 + STATION_WIDTH + KM_WIDTH)
#define	Y_DIST	20

extern	Track	*layout;
extern	int	is_windows;

//static	Track	*stations[100];
//static	int	nstations;

static	grid	*platform_graph_grid;

#define	HEIGHT	700

BEGIN_EVENT_TABLE(PlatformGraphView, wxScrolledWindow)
    EVT_MOTION(PlatformGraphView::OnMouseMove)
    EVT_PAINT(PlatformGraphView::OnPaint)
END_EVENT_TABLE()

PlatformGraphData platformData;

//static	PlatformSegment	*segments;

static	void	DrawTimeGrid(grid *g, int y)
{
	int	h, m;
	int	x;
	wxChar	buff[20];

	x = STATION_WIDTH + KM_WIDTH;
	g->DrawLine(x, HEIGHT, x + 23 * 120 + 59 * 2, HEIGHT, 0);
	for(h = 0; h < 24; ++h)
	    for(m = 0; m < 60; ++m) {
		if((m % 10)) {
//		    g->DrawLine(
//			x + h * 120 + m * 2, y - 2,
//			x + h * 120 + m * 2, y + 2,
//			0);
		} else {
		    g->DrawLine(
			x + h * 120 + m * 2, 20,
			x + h * 120 + m * 2, HEIGHT,
			m ? 6 : 0);
		}
	    }

	for(h = 0; h < 24; ++h) {
	    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%2d:00"), h);
	    g->DrawText1(x + h * 120, 10, buff, 0);
	}
}

void	PlatformGraphView::DrawStations(PlatformGraphData& pd, grid *g)
{
	Track	*t;
        int     i;
	int	y = HEADER_HEIGHT;

        for(i = 0; i < pd.nStations; ++i) {
            t = pd.stations[i];
	    DrawTimeGrid(g, y);
	    g->DrawText1(0, y - 10, t->station, 0);
	    g->DrawLine(STATION_WIDTH + KM_WIDTH, y, STATION_WIDTH + KM_WIDTH + 24 * 120, y, 0);
	    y += Y_DIST;
	}
	if(!i) {
	    g->DrawText1(10, 10, L("Sorry, this feature is not available on this scenario."), 0);
	    g->DrawText1(10, 25, L("No station was found on the layout."), 0);
	}
}

static	void	graph_xy(long km, long tim, int *x, int *y)
{
	*x = tim / 60 * 2 + STATION_WIDTH + KM_WIDTH;
	*y = (km + 1) * Y_DIST;
}

static	void	time_to_time(grid *g, int x, int y, int nx, int ny, int type)
{
	int	color = fieldcolors[COL_TRAIN1 + type];

	if(nx < x)	/* ignore if arrival is next day */
	    return;
	g->DrawLine(x, y - 1, nx, y - 1, color);
	g->DrawLine(x, y, nx, y, color);
	g->DrawLine(x, y + 1, nx, y + 1, color);
}

static	int	samestation(const wxChar *st, const wxChar *arrdep)
{
	return wxStrcmp(st, arrdep) == 0;
}

void	PlatformGraphData::addSegment(Train *trn, int x0, int x1, int y, long timein, long timeout, Train *parent)
{
	PlatformSegment *segment = (PlatformSegment *)malloc(sizeof(PlatformSegment));

	segment->train = trn;
	segment->x0 = x0;
	segment->x1 = x1;
	segment->y = y;
	segment->timein = timein;
	segment->timeout = timeout;
	segment->parent = parent;
	segment->next = segments;
	segments = segment;
}

void	PlatformGraphView::DrawTrains(PlatformGraphData& pd, grid *g)
{
        PlatformBlock *block;

        for(block = pd.blocks; block; block = block->next) {
            time_to_time(g, block->x0, block->y, block->x1, block->y, block->color);
        }
}

PlatformGraphView::PlatformGraphView(wxWindow *parent)
	: wxScrolledWindow(parent, wxID_ANY, wxPoint(0, 0), wxSize(XMAX * 2 + STATION_WIDTH + KM_WIDTH, YMAX))
{
	SetScrollbars(1, 1, XMAX * 2 + STATION_WIDTH + KM_WIDTH, YMAX);
	grid	*g = new grid(this, XMAX * 2 + STATION_WIDTH + KM_WIDTH, YMAX);
	platform_graph_grid = g;
	m_tooltip = 0;
	wxToolTip::SetDelay(1000);
	wxToolTip::Enable(true);
	g->Clear();
}

void	PlatformGraphView::Refresh()
{
	grid	*g = platform_graph_grid;

	g->Clear();
        platformData.Clear();
        platformData.ComputeStations();
        platformData.ComputeTrains();
	DrawStations(platformData, g);
	DrawTimeGrid(g, 0);
	DrawTrains(platformData, g);
	wxScrolledWindow::Refresh();
}

void	PlatformGraphView::OnPaint(wxPaintEvent& event)
{
	if(platform_graph_grid)
	    platform_graph_grid->Paint(this);
}

wxPoint PlatformGraphView::GetEventPosition(wxPoint& pt)
{
	double	xScale, yScale;
	wxPoint	pos(pt);
	CalcUnscrolledPosition(pos.x, pos.y, &pos.x, &pos.y);
	field_grid->m_dc->GetUserScale(&xScale, &yScale);
	pos.x /= xScale;
	pos.y /= yScale;
	return pos;
}

void PlatformGraphView::OnMouseMove(wxMouseEvent& event)
{
	wxPoint pos = event.GetPosition();
	pos = GetEventPosition(pos);

	Coord	coord(pos.x, pos.y);

	wxChar	oldTooltip[sizeof(tooltipString)/sizeof(tooltipString[0])];
	wxStrcpy(oldTooltip, tooltipString);

//	pointer_at(coord);

	PlatformSegment *segment;

	for(segment = platformData.segments; segment; segment = segment->next) {
	    if(pos.x >= segment->x0 && pos.x < segment->x1 &&
		pos.y >= segment->y - 2 && pos.y < segment->y + 2) {
		if(segment->parent) {
		    wxSnprintf(tooltipString, sizeof(tooltipString)/sizeof(tooltipString[0]),
			L("Train %s              \nArrives %s\n"),
			segment->parent->name, format_time(segment->timein));
		    wxSprintf(tooltipString + wxStrlen(tooltipString),
			L("Departs %s\nas train %s"), format_time(segment->timeout), segment->train->name);
		} else {
		    wxSnprintf(tooltipString, sizeof(tooltipString)/sizeof(tooltipString[0]),
			L("Train %s              \nArrives %s\n"),
			segment->train->name, format_time(segment->timein));
		    wxSprintf(tooltipString + wxStrlen(tooltipString),
			L("Departs %s\n"), format_time(segment->timeout));
		}
		break;
	    }
	}
	if(!segment) {
	    SetToolTip(0);
//	    if(m_tooltip)
//		delete m_tooltip;
	    m_tooltip = 0;
	    tooltipString[0] = 0;
	} else if(wxStrcmp(oldTooltip, tooltipString)) {
#ifdef WIN32
	    wxToolTip *newTip = new wxToolTip(tooltipString);
	    SetToolTip(newTip);
//	    if(m_tooltip)
//		delete m_tooltip;
	    m_tooltip = newTip;
#else
//	    canvasHelp.AddHelp(this, tooltipString);
//	    canvasHelp.ShowHelp(this);
//	    canvasHelp.RemoveHelp(this);
#endif
	}
	event.Skip();
}

PlatformGraphData::PlatformGraphData()
{
        nStations = 0;
        segments = 0;
        blocks = 0;
}


void    PlatformGraphData::Clear()
{
	while(segments) {
	    PlatformSegment *next = segments->next;
	    free(segments);
	    segments = next;
	}
        while(blocks) {
            PlatformBlock *next = blocks->next;
            delete blocks;
            blocks = next;
        }
        nStations = 0;
}

int     byStationName(const void *p1, const void *p2)
{
        const Track *s1 = *(Track **)p1;
        const Track *s2 = *(Track **)p2;
        return wxStrcmp(s1->station, s2->station);
}

void    PlatformGraphData::ComputeStations()
{
        Track   *t;
        int     x;

        nStations = 0;
	for(t = layout; t && nStations < MAX_PLATFORMS; t = t->next) {
	    if(!t->isstation || !t->station)
		continue;
            for(x = 0; x < nStations; ++x)
                if(!wxStrcmp(t->station, stations[x]->station))
                    break;
            if(x >= nStations && nStations < MAX_PLATFORMS)
                stations[nStations++] = t;
        }
        qsort(stations, nStations, sizeof(stations[0]), byStationName);
}

void    PlatformGraphData::ComputeTrains()
{
	Train	*t;
	TrainStop *ts;
	Track	*trk;
	int	indx;
	int	x, y;
	int	nx, ny;

	for(t = schedule; t; t = t->next) {
	    if(t->days && run_day && !(t->days & run_day))
		continue;
	    x = y = -1;
	    for(trk = layout; trk; trk = trk->next) {
		if(trk->type == TRACK && trk->isstation &&
					samestation(trk->station, t->entrance))
		    break;
	    }
	    if(trk && (indx = graphstation(trk->station)) >= 0) {
		graph_xy(indx, t->timein, &x, &y);
		if(t->waitfor) {
		    Train *parent = findTrainNamed(t->waitfor);
		    if(parent) {
			graph_xy(indx, parent->timeout, &nx, &ny);
			AddBlock(x, nx, y, t->type);
			addSegment(t, x, nx, y, parent->timeout, t->timein, parent);
		    }
		}
	    }
	    for(ts = t->stops; ts; ts = ts->next) {
		indx = graphstation(ts->station);
		if(indx < 0)
		    continue;
		graph_xy(indx, ts->arrival, &nx, &ny);
		graph_xy(indx, ts->departure, &x, &y);
		AddBlock(nx, x, y, t->type);
		addSegment(t, nx, x, y, ts->arrival, ts->departure, 0);
	    }
	    if(t->stock) {
		for(trk = layout; trk; trk = trk->next) {
		    if(trk->type == TRACK && trk->isstation &&
					    samestation(trk->station, t->exit))
			break;
		}
		if(trk && (indx = graphstation(trk->station)) >= 0) {
		    Train *child = findTrainNamed(t->stock);
		    if(child) {
			graph_xy(indx, t->timeout, &x, &y);
			graph_xy(indx, child->timein, &nx, &ny);
			AddBlock(x, nx, y, t->type);
			addSegment(child, x, nx, y, t->timeout, child->timein, t);
		    }
		}
	    }
	}
}

void    PlatformGraphData::AddBlock(int x0, int x1, int y, int color)
{
        PlatformBlock *block = new PlatformBlock();
        block->x0 = x0;
        block->x1 = x1;
        block->y = y;
        block->color = color;
        block->next = blocks;
        blocks = block;
}

int	PlatformGraphData::graphstation(const Char *st)
{
	int	i;

	for(i = 0; i < nStations; ++i)
	    if(wxStrcmp(st, stations[i]->station) == 0)
		return i;
	return -1;
}

