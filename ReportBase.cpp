/*	ReportBase.cpp - Created by Giampiero Caprino

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
#include "wx/listctrl.h"
#include "wx/image.h"

#include "ReportBase.h"

ReportBase::ReportBase(wxWindow *parent, const wxString& name)
	: wxListCtrl(parent, wxID_ANY, wxDefaultPosition,
	  wxDefaultSize, wxLC_REPORT|wxLC_HRULES|wxLC_SINGLE_SEL),
	  m_name(name)
{
}

ReportBase::~ReportBase()
{
}

void	ReportBase::DefineColumns(const Char *titles[], int widths[])
{
	int	    i;

	wxListItem  col;

	//  Insert columns

	for(i = 0; titles[i]; ++i) {
	    col.SetText(titles[i]);
//	    col.SetImage(-1);
	    InsertColumn(i, col);
	    SetColumnWidth(i, widths[i]);
	}
}

void	ReportBase::LoadState(const wxString& header, TConfig& state)
{
	int	nCols;
	int	i;
	int	w;
	wxString buff;

	if(!state.FindSection(header))
	    return;
	state.GetInt(wxT("nCols"), nCols);
	for(i = 0; i < nCols; ++i) {
	    buff.Printf(wxT("width%d"), i);
	    if(state.GetInt(buff, w))
		SetColumnWidth(i, w);
	}
}

void	ReportBase::SaveState(const wxString& header, TConfig& state)
{
	int		nCol = GetColumnCount();
	int		i;
	wxString	buff;

	state.StartSection(header.c_str());
	state.PutString(wxT("name"), m_name);
	state.PutString(wxT("type"), GetName());
	state.PutInt(wxT("nCols"), nCol);
	for(i = 0; i < nCol; ++i) {
	    int	w = GetColumnWidth(i);

	    buff.Printf(wxT("width%d"), i);
	    state.PutInt(buff, w);
	}
}

void	*ReportBase::GetSelectedData()
{
        wxListItem item;
	long	idx = -1;
	idx = GetNextItem(idx, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(idx == -1)
	    return 0;
	item.SetId(idx);
	item.SetMask(wxLIST_MASK_DATA);
	GetItem(item);
	return (void *)item.GetData();
}
