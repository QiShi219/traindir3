/*	PlatformGraphView.h - Created by Giampiero Caprino

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

#ifndef _PLATFORMGRAPHVIEW_H
#define	_PLATFORMGRAPHVIEW_H

#include <wx/wx.h>
#include "wx/tooltip.h"

class	grid;

#define MAX_PLATFORMS   200

struct PlatformSegment {
	PlatformSegment *next;
	int	    y;
	int	    x0, x1;
	Train	    *train;
	Train	    *parent;
	long	    timein, timeout;
};

struct PlatformBlock {
public:
        PlatformBlock *next;
        int     x0, x1;
        int     y;
        int     color;
};

class PlatformGraphData {
public:
        PlatformGraphData();
        void    Clear();

        void    ComputeStations();
        void    ComputeTrains();

        void    AddBlock(int x0, int x1, int y, int color);
        void	addSegment(Train *trn, int x0, int x1, int y, long timein, long timeout, Train *parent);
        int	graphstation(const Char *st);

        int     nStations;
        Track   *stations[MAX_PLATFORMS];
        PlatformSegment *segments;
        PlatformBlock *blocks;
};

class PlatformGraphView : public wxScrolledWindow
{
public:
	PlatformGraphView(wxWindow *parent);
	virtual ~PlatformGraphView() { };
	void	Refresh(void);

	//  events

	virtual void OnPaint(wxPaintEvent& event);
	virtual void OnMouseMove(wxMouseEvent& event);

private:
	wxPoint GetEventPosition(wxPoint& pt);

	wxToolTip   *m_tooltip;
	void	DrawStations(PlatformGraphData& pd, grid *g);
	void	DrawTrains(PlatformGraphData& pd, grid *g);
	DECLARE_EVENT_TABLE()
};

#endif // _PLATFORMGRAPHVIEW_H
