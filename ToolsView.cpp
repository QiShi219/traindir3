/*	ToolsView.cpp - Created by Giampiero Caprino

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

#include "wx/wx.h"
#include "wx/image.h"
#include "wx/dcbuffer.h"
#include "Canvas.h"
#include "ToolsView.h"
#include "Traindir3.h"
#include "FontManager.h"
extern  void	getcolor_rgb(int col, int *r, int *g, int *b);

extern	int	current_toolset;
extern	int	current_tool;
extern	Track	*tool_tracks, *tool_switches, *tool_signals,
		*tool_misc, *tool_actions;
extern	void	*tracks_pixmap, *switches_pixmap, *signals_pixmap,
		*tools_pixmap, *actions_pixmap,
		*move_start_pixmap, *move_end_pixmap, *move_dest_pixmap,
                *set_power_pixmap, *set_track_len_pixmap;

int	gFontSizeSmall = 7;
int	gFontSizeBig = 10;
		
extern	unsigned char	colortable[15][3];

grid::grid(wxWindow *parent, int width, int height)
{
	m_pixmap = new wxBitmap(width, height);
	m_parent = parent;
	m_dc = new wxMemoryDC;

	m_hmult = 1;
	m_vmult = 1;

	m_xBase = 0;
	m_yBase = 0;
}

grid::~grid()
{
	if(m_pixmap)
	    delete m_pixmap;
	m_pixmap = 0;
	if(m_dc)
	    delete m_dc;
	m_dc = 0;
}


void	grid::Paint(wxWindow *dest, bool fillBg)
{
#ifndef WIN32
	wxClientDC   dc(dest);
#else
	wxBufferedPaintDC   dc(dest);
#endif

	dest->PrepareDC(dc);
	wxBufferedDC	wdc(&dc, *m_pixmap);
}

void	grid::DrawText1(int x, int y, const wxChar *txt, int size)
{
	CellToCoord(x, y);
	if(this != tools_grid)
	    y -= (size ? 3 : 4);
	wxFont	font(size ? gFontSizeSmall : gFontSizeBig, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL);
	m_dc->SelectObject(*m_pixmap);
	m_dc->SetFont(font);
	m_dc->SetBackgroundMode(wxTRANSPARENT);
	m_dc->SetTextForeground(curSkin->text);
	m_dc->SetTextBackground(*wxWHITE);
	wxPoint	pt(x, y);
	m_dc->DrawText(txt, pt);
	m_dc->SelectObject(wxNullBitmap);
}

void	grid::DrawTextFont(int x, int y, const wxChar *txt, int fontIndex)
{
	FontEntry *f = fonts.FindFont(fontIndex);
//	if(!f) {
	    DrawText1(x, y, txt, fontIndex == 1);
	    return;
//	}

	CellToCoord(x, y);
	y -= f->_size / 2;
	wxFont	font(f->_size, f->_family, f->_style, f->_weight);
	m_dc->SelectObject(*m_pixmap);
	m_dc->SetFont(font);
	m_dc->SetBackgroundMode(wxTRANSPARENT);
	m_dc->SetTextForeground(f->_color); // *wxBLACK);
	m_dc->SetTextBackground(*wxWHITE);
	wxPoint	pt(x, y);
	m_dc->DrawText(txt, pt);
	m_dc->SelectObject(wxNullBitmap);
}

void	grid::DrawTextWithBackground(int x, int y, const wxChar *txt, int size, int bgcolor)
{
	CellToCoord(x, y);
        int r, g, b;
    	getcolor_rgb(bgcolor, &r, &g, &b);
	if(this != tools_grid)
	    y -= (size ? 1 : 4);
	wxFont	font(size ? gFontSizeSmall : gFontSizeBig, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL);
	m_dc->SelectObject(*m_pixmap);
	m_dc->SetFont(font);
	m_dc->SetBackgroundMode(wxSOLID);
	m_dc->SetTextForeground(curSkin->text);
	m_dc->SetTextBackground(wxColor(r, g, b));
	wxPoint	pt(x, y);
	m_dc->DrawText(txt, pt);
	m_dc->SelectObject(wxNullBitmap);
}

void	grid::DrawTextWithForeground(int x, int y, const wxChar *txt, int size, int fgcolor)
{
	CellToCoord(x, y);
        int r, g, b;
    	getcolor_rgb(fgcolor, &r, &g, &b);
	if(this != tools_grid)
	    y -= (size ? 1 : 4);
	wxFont	font(size ? gFontSizeSmall : gFontSizeBig, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL);
	m_dc->SelectObject(*m_pixmap);
	m_dc->SetFont(font);
	m_dc->SetBackgroundMode(wxSOLID);
	m_dc->SetTextForeground(wxColor(r, g, b));
	m_dc->SetTextBackground(curSkin->background);
	wxPoint	pt(x, y);
	m_dc->DrawText(txt, pt);
	m_dc->SelectObject(wxNullBitmap);
}

void    grid::GetTextExtent(const wxChar *txt, int size, Coord& out)
{
	wxFont	font(size ? gFontSizeSmall : gFontSizeBig, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL);
	m_dc->SelectObject(*m_pixmap);
	m_dc->SetFont(font);
        wxCoord x, y;
        wxString str(txt);
        m_dc->GetTextExtent(str, &x, &y);
	m_dc->SelectObject(wxNullBitmap);
        out.x = (Pos)x;
        out.y = (Pos)y;
}

void	grid::DrawLayoutRGB(int x0, int y0, VLines *p, int rgb)
{
	wxRect update_rect;
	wxColour fg(rgb >> 16, (rgb >> 8) & 0xFF, rgb & 0xFF);
	int	x = x0;
	int	y = y0;

	CellToCoord(x, y);
	m_dc->SelectObject(*m_pixmap);
	m_dc->SetPen(wxPen(fg, 1));
	while(p->x0 >= 0) {
	    m_dc->DrawLine(
		x + p->x0 * m_hmult / HGRID,
		y + p->y0 * m_vmult / VGRID,
		x + p->x1 * m_hmult / HGRID,
		y + p->y1 * m_vmult / VGRID);
	    m_dc->DrawPoint(x + p->x1 * m_hmult / HGRID,
		y + p->y1 * m_vmult / VGRID);
	    ++p;
	}
	m_dc->SelectObject(wxNullBitmap);
}



void	grid::DrawLayout(int x0, int y0, VLines *p, grcolor col)
{
	DrawLayoutRGB(x0, y0, p, (colortable[col][0] << 16) | (colortable[col][1] << 8) | colortable[col][2]);
}

void	grid::DrawLineRGB(int x0, int y0, int x1, int y1, int rgb)
{
	wxColour fg(rgb >> 16, (rgb >> 8) & 0xFF, rgb & 0xFF);
	int	x = x0;
	int	y = y0;

	CellToCoord(x, y);
	m_dc->SelectObject(*m_pixmap);
	m_dc->SetPen(wxPen(fg, 1));
	m_dc->DrawLine(x, y, x1 * m_hmult, y1 * m_vmult);
	m_dc->DrawPoint(x1 * m_hmult, y1 * m_vmult);
	m_dc->SelectObject(wxNullBitmap);
}

void	grid::DrawLine(int x0, int y0, int x1, int y1, grcolor col)
{
	DrawLineRGB(x0, y0, x1, y1, (colortable[col][0] << 16) | (colortable[col][1] | 8) | colortable[col][2]);
}

void	grid::DrawLineCenterCellRGB(int x0, int y0, int x1, int y1, int rgb)
{
	wxColour fg(rgb >> 16, (rgb >> 8) & 0xFF, rgb & 0xFF);
	int	mx = HGRID / 2;
	int	my = VGRID / 2;
	int	x = x0;
	int	y = y0;

	CellToCoord(x, y);
	CellToCoord(x1, y1);
	m_dc->SelectObject(*m_pixmap);
	m_dc->SetPen(wxPen(fg, 1));
	m_dc->DrawLine(x + mx, y + my, x1 /* * m_hmult */ + mx, y1 /* * m_vmult */ + my);
	m_dc->SelectObject(wxNullBitmap);
}

void	grid::DrawLineCenterCell(int x0, int y0, int x1, int y1, grcolor col)
{
	DrawLineCenterCellRGB(x0, y0, x1, y1, (colortable[col][0] << 16) | (colortable[col][1] << 8) | colortable[col][2]);
}

void	grid::DrawPoint(int x0, int y0, int dx, int dy, int rgb)
{
	wxRect update_rect;
	wxColour fg(rgb >> 16, (rgb >> 8) & 0xFF, rgb & 0xFF);
	int	x = x0;
	int	y = y0;

	CellToCoord(x, y);
        x += m_hmult / 2 + dx;
        y += m_vmult / 2 + dy;
	m_dc->SelectObject(*m_pixmap);
	m_dc->SetPen(wxPen(fg, 1));
        m_dc->DrawPoint(x, y);
	m_dc->SelectObject(wxNullBitmap);
}

void	setBackgroundColor(wxColour& col)
{
	int rgb = curSkin->background;
	col.Set(rgb >> 16, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

void	grid::FillCell(int x, int y)
{
	CellToCoord(x, y);
	m_dc->SelectObject(*m_pixmap);
	wxRect rect(x, y, m_hmult, m_vmult);
//	int	bgix = fieldcolors[COL_BACKGROUND];
//	wxColour background_color(colortable[bgix][0], colortable[bgix][1], colortable[bgix][2]);
	wxColour background_color;
	setBackgroundColor(background_color);
	m_dc->SetBrush(wxBrush(background_color));
	m_dc->SetPen(wxPen(background_color, 1));
	m_dc->DrawRectangle(rect);
	m_dc->SelectObject(wxNullBitmap);
}

void	grid::Clear()
{
	wxColour bg;
	setBackgroundColor(bg);
	m_dc->SelectObject(*m_pixmap);
//	bg = g->dc->GetBackground();
	m_dc->SetBrush(wxBrush(bg));
	wxRect	rect(wxPoint(0, 0), wxSize(m_pixmap->GetWidth(), m_pixmap->GetHeight()));
	m_dc->DrawRectangle(rect);
	m_dc->SelectObject(wxNullBitmap);
}


void	grid::ClearField()
{
	wxColour bg;
	setBackgroundColor(bg);
	m_dc->SelectObject(*m_pixmap);
//	bg = g->dc->GetBackground();
	m_dc->SetBrush(wxBrush(bg));
	wxRect rect(
		    cliprect.left * m_hmult,
		    cliprect.top * m_vmult,
		    (cliprect.right - cliprect.left + 1) * m_hmult,
		    (cliprect.bottom - cliprect.top + 1) * m_vmult
		    );
	m_dc->DrawRectangle(rect);
	m_dc->SelectObject(wxNullBitmap);
}

//	Draw the point grid in the canvas.
//	This gives the player a reference
//	for positioning track elements.
//	Only called if the corresponding
//	option in Preferences is set.

void	grid::Paint()
{
	int	x, y;
	wxColour bg(colortable[color_darkgray][0], colortable[color_darkgray][1], colortable[color_darkgray][2]);

	m_dc->SelectObject(*m_pixmap);
	m_dc->SetBrush(wxBrush(bg));
	m_dc->SetPen(wxPen(bg, 1));
	for(x = m_xBase; x < XMAX; x += HGRID)
	    for(y = m_yBase; y < YMAX; y += VGRID)
		if(x >= cliprect.left * HGRID && x <= cliprect.right * HGRID &&
		   y >= cliprect.top * VGRID && y <= cliprect.bottom * VGRID)
		    m_dc->DrawPoint(x, y);
	m_dc->SelectObject(wxNullBitmap);
}


// ----------------------------------------------------------------------------
// ToolsView
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(ToolsView, wxWindow)
    EVT_PAINT(ToolsView::OnPaint)
//    EVT_MOUSE_EVENTS(ToolsView::OnMouse)
    EVT_LEFT_DOWN(ToolsView::OnMouseLeft)
    EVT_RIGHT_DOWN(ToolsView::OnMouseRight)
    EVT_LEFT_DCLICK(ToolsView::OnMouseDblLeft)
    EVT_RIGHT_DCLICK(ToolsView::OnMouseDblRight)
END_EVENT_TABLE()

ToolsView::ToolsView(wxWindow* parent)
        : wxWindow(parent, wxID_ANY, wxPoint(0, 0),
			wxSize(XMAX, YMAX))
{
	tools_grid = new grid(this);
	tools_grid->m_hmult = HGRID * 3;
	tools_grid->m_vmult = VGRID * 3;
	tools_grid->Clear();
}

ToolsView::~ToolsView()
{
	if(tools_grid)
	    delete tools_grid;
	tools_grid = 0;
}

void ToolsView::OnPaint(wxPaintEvent& event)
{
	Track *trk_tools;

	if(!tools_grid)
	    return;
	grid	*old = current_grid;
	current_grid = tools_grid;
	switch(current_toolset) {
	case 1:
	    trk_tools = tool_tracks;
	    break;

	case 2:
	    trk_tools = tool_switches;
	    break;

	case 3:
	    trk_tools = tool_signals;
	    break;
	
	case 4:
	    trk_tools = tool_misc;
	    break;
	
	case 5:
	    trk_tools = tool_actions;
	    break;

	default:
	    return;
	}
	tools_grid->m_dc->SelectObject(*tools_grid->m_pixmap);
	tools_grid->m_dc->SetBrush(wxBrush(*wxTRANSPARENT_BRUSH));
	layout_paint(trk_tools);
	tools_grid->m_dc->SelectObject(*tools_grid->m_pixmap);
	tools_grid->m_dc->SetBrush(wxBrush(*wxTRANSPARENT_BRUSH));
	int	i;
	for(i = 0; tooltbl[i].type != -1; ++i) {
	    if(current_tool == i)
		tools_grid->m_dc->SetPen(*wxCYAN_PEN);
	    else
		tools_grid->m_dc->SetPen(*wxBLACK_PEN);
	    tools_grid->m_dc->DrawRectangle(
		tooltbl[i].x * tools_grid->m_hmult,
		tooltbl[i].y * tools_grid->m_vmult,
		tools_grid->m_hmult,
		tools_grid->m_vmult);
	}
	tools_grid->m_dc->SelectObject(wxNullBitmap);

	draw_pixmap(1, 0, tracks_pixmap);
	draw_pixmap(2, 0, switches_pixmap);
	draw_pixmap(3, 0, signals_pixmap);
	draw_pixmap(4, 0, tools_pixmap);
	draw_pixmap(5, 0, actions_pixmap);
	if(current_toolset == 5) {
	    draw_pixmap(8, 1, move_start_pixmap);
	    draw_pixmap(9, 1, move_end_pixmap);
	    draw_pixmap(10, 1, move_dest_pixmap);
	    draw_pixmap(11, 1, set_power_pixmap);
	    draw_pixmap(12, 1, set_track_len_pixmap);
	}
	tools_grid->Paint(this, true);
	current_grid = old;
}

void ToolsView::OnMouseLeft(wxMouseEvent& event)
{
	wxPoint	pos(event.GetPosition());

	if(event.ControlDown()) {
	} else if(event.AltDown()) {
	} else if(event.ShiftDown()) {
	}
/////	CalcUnscrolledPosition(pos.x, pos.y, &pos.x, &pos.y);
	// Now pos has the absolute position in the ToolsView
	wxChar	buff[64];

	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("selecttool %d,%d"),
	    pos.x / tools_grid->m_hmult,
	    pos.y / tools_grid->m_vmult);
	trainsim_cmd(buff);
}

void ToolsView::OnMouseRight(wxMouseEvent& event)
{
	wxPoint	pos(event.GetPosition());

	if(event.ControlDown()) {
	} else if(event.AltDown()) {
	} else if(event.ShiftDown()) {
	}
/////	CalcUnscrolledPosition(pos.x, pos.y, &pos.x, &pos.y);
}

void ToolsView::OnMouseDblLeft(wxMouseEvent& event)
{
	wxPoint	pos(event.GetPosition());

/////	CalcUnscrolledPosition(pos.x, pos.y, &pos.x, &pos.y);
	// Now pos has the absolute position in the ToolsView
}

void ToolsView::OnMouseDblRight(wxMouseEvent& event)
{
	wxPoint	pos(event.GetPosition());

/////	CalcUnscrolledPosition(pos.x, pos.y, &pos.x, &pos.y);
	// Now pos has the absolute position in the ToolsView
}

