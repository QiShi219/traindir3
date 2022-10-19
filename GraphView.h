/*	GraphView.h - Created by Giampiero Caprino

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

#ifndef _GRAPHVIEW_H
#define	_GRAPHVIEW_H

#include <wx/wx.h>

class	grid;
class   Track;

#define MAX_STATIONS    100

class GraphLine {
public:
    GraphLine *next;
    int     x0;
    int     y0;
    int     x1;
    int     y1;
    int     color;
    int     colorIndex;
};

class GraphViewData {
public:
        GraphViewData();
        ~GraphViewData();
        void    ComputeStationsPositions();
        void	DrawTrains();

        int     yStations[MAX_STATIONS];
        Char    *stationNames[MAX_STATIONS];
        Track   *stations[MAX_STATIONS];
        int     nStations;
        GraphLine *lines;

        int     highKm;
private:
        int	Km_to_y(int km);
        void	graph_xy(long km, long tim, int *x, int *y);
        void	time_to_time(int x, int y, int nx, int ny, int type);
        int	graphstation(const wxChar *st);
        void    AddLine(int x0, int y0, int x1, int y1, int color);
};

class GraphView : public wxScrolledWindow
{
public:
	GraphView(wxWindow *parent);
	virtual ~GraphView() { };
	void	Refresh(void);

	//  events

	virtual void OnPaint(wxPaintEvent& event);

private:
	void	DrawStations(GraphViewData& gd, grid *g);
	DECLARE_EVENT_TABLE()
};

#endif // _GRAPHVIEW_H
