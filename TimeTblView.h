/*	TimeTblView.h - Created by Giampiero Caprino

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

#ifndef _TIMETBLVIEW_H
#define _TIMETBLVIEW_H

#include "ReportBase.h"

struct Train;

class TimeTableView : public ReportBase
{
public:
	TimeTableView(wxWindow *parent, const wxString& name);
	virtual ~TimeTableView();

	void	UpdateItem(int i, Train *t);
	void	DeleteRow(int x);

	void	LoadState(const wxString& header, TConfig& state);
	void	SaveState(const wxString& header, TConfig& state);

	void	ShowLast();
	void	ShowFirst();

	void	OnItemActivated(wxListEvent& event);
	void	OnContextMenu(wxContextMenuEvent& event);
	void	OnShowCanceled(wxCommandEvent& event);
	void	OnShowArrived(wxCommandEvent& event);
	void	OnAssign(wxCommandEvent& event);
        void    OnStartNow(wxCommandEvent& event);
        void    OnSetDelay(wxCommandEvent& event);
	void	OnTrackFirst(wxCommandEvent& event);
	void	OnTrackLast(wxCommandEvent& event);
	void	OnPrintTrain(wxCommandEvent& event);

	wxString    m_name;

	// options

	bool	    m_bTrackFirst;
	bool	    m_bTrackLast;

private:
	DECLARE_EVENT_TABLE()
};

#endif // _TIMETBLVIEW_H
