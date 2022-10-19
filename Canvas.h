/*	Canvas.h - Created by Giampiero Caprino

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

#ifndef _CANVAS_H
#define _CANVAS_H

#include "color.h"
#include "defs.h"
#include "wx/tooltip.h"

class TDLayout;

class Canvas : public wxScrolledWindow
{
public:
	Canvas(wxWindow *parent);
	virtual ~Canvas();

	//  events

	virtual void OnPaint(wxPaintEvent& event);
	virtual void OnMouseMove(wxMouseEvent& event);
	virtual void OnMouseLeft(wxMouseEvent& event);
	virtual void OnMouseRight(wxMouseEvent& event);
	virtual void OnMouseDblLeft(wxMouseEvent& event);
	virtual void OnMouseDblRight(wxMouseEvent& event);
	virtual void OnChar(wxKeyEvent& ev);
	virtual void CoordMenu(wxMouseEvent& event, bool verticalCoord);
	void	OnCoordDel1(wxCommandEvent& event);
	void	OnCoordDelN(wxCommandEvent& event);
	void	OnCoordIns1(wxCommandEvent& event);
	void	OnCoordInsN(wxCommandEvent& event);

	virtual void OnEraseBackground(wxEraseEvent& event);

	wxPoint GetEventPosition(wxPoint& pt);

	void	    DoPrint();

	TDLayout	*m_layout;


	int		m_xyCoord;

	wxToolTip	*m_tooltip;

private:
	DECLARE_EVENT_TABLE()

//	DECLARE_NO_COPY_CLASS(MyCanvas)
};

struct VLines;

class grid {
public:
	grid(wxWindow* parent, int width = XMAX, int height = YMAX);
	~grid();

	void	Paint(wxWindow* dest, bool fillBg = false);
	void	DrawText1(int x, int y, const wxChar *txt, int size);
	void	DrawTextFont(int x, int y, const wxChar *txt, int fontIndex);
        void	DrawTextWithForeground(int x, int y, const wxChar *txt, int size, int fgcolor);
        void	DrawTextWithBackground(int x, int y, const wxChar *txt, int size, int bgcolor);
	void	DrawLayoutRGB(int x0, int y0, VLines *p, int rgb);
	void	DrawLineRGB(int x0, int y0, int x1, int y1, int rgb);
	void	DrawLineCenterCellRGB(int x0, int y0, int x1, int y1, int rgb);
        void	DrawPoint(int x0, int y0, int dx, int dy, int rgb);

	void	DrawLayout(int x0, int y0, VLines *p, grcolor col);
	void	DrawLine(int x0, int y0, int x1, int y1, grcolor col);
	void	DrawLineCenterCell(int x0, int y0, int x1, int y1, grcolor col);
	// cell coord to canvas coord
	void	CellToCoord(int& x, int& y)
	{
	    x = (x * m_hmult) + m_xBase;
	    y = (y * m_vmult) + m_yBase;
	}
        void    GetTextExtent(const wxChar *txt, int size, Coord& out);
//	void	DrawLineAbsolute(int x0, int y0, int x1, int y1, grcolor col);
	void	FillCell(int x, int y);
	void	Clear();
	void	ClearField();
	void	Paint();

	wxBitmap *m_pixmap;
	wxWindow *m_parent;
	wxMemoryDC *m_dc;
	int	m_hmult, m_vmult;
	int	m_xBase, m_yBase;
};

#ifdef __cplusplus
extern "C" {
#endif

extern	grid	*current_grid, *field_grid, *tools_grid;

#ifdef __cplusplus
};
#endif

#endif // _CANVAS_H
