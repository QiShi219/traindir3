/*	ItineraryDialog.h - Created by Giampiero Caprino

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

#ifndef _ITINERARYDIALOG_H
#define _ITINERARYDIALOG_H

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/radiobox.h>
#include <wx/listctrl.h>

class Itinerary;

class ItineraryDialog : public wxDialog
{
public:
	ItineraryDialog(wxWindow *parent);
	~ItineraryDialog();

	int	ShowModal(Itinerary *it);

	wxTextCtrl  *m_name,
		    *m_next;
	wxStaticText *m_start_point;
};

class ItineraryKeyDialog : public wxDialog
{
public:
	ItineraryKeyDialog(wxWindow *parent);
	~ItineraryKeyDialog();

	int	ShowModal();
private:
	void	DoSelect(wxCommandEvent& ev, bool forShunt);
	void	OnSelect(wxCommandEvent& ev);
	void	OnSelectForShunt(wxCommandEvent& ev);
	void	OnClear(wxCommandEvent& ev);
	void	OnClose(wxCommandEvent& ev);
	void	OnActivated(wxListEvent& event);
	long	GetSelectedItem();
	void	FillItineraryList();

	wxStaticText *m_header;
	wxTextCtrl  *m_edit;

	wxButton    *m_selectbutton,
                    *m_selectForShunt,
		    *m_clearbutton,
		    *m_closebutton;
	wxListCtrl  *m_list;
	DECLARE_EVENT_TABLE()
};

#endif // _ITINERARYDIALOG_H
