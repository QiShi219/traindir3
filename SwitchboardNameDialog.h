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

#ifndef _SWITCHBOARDNAMEDIALOG_H
#define _SWITCHBOARDNAMEDIALOG_H

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/radiobox.h>
#include <wx/listctrl.h>

class SwitchBoard;

class SwitchboardNameDialog : public wxDialog
{
public:
	SwitchboardNameDialog(wxWindow *parent);
	~SwitchboardNameDialog();

	int	ShowModal(SwitchBoard *sb);
	void	OnRemove(wxCommandEvent& ev);
	void	OnSave(wxCommandEvent& ev);
	void	OnClose(wxCommandEvent& ev);

	wxTextCtrl  *m_name;
	wxTextCtrl  *m_path;
	wxButton    *m_removebutton,
		    *m_savebutton,
		    *m_closebutton;
	DECLARE_EVENT_TABLE()
};

class SwitchboardCellDialog : public wxDialog
{
public:
	SwitchboardCellDialog(wxWindow *parent);
	~SwitchboardCellDialog();

	int	ShowModal(int x, int y);
	void	OnRemove(wxCommandEvent& ev);
	void	OnSave(wxCommandEvent& ev);
	void	OnClose(wxCommandEvent& ev);

	wxTextCtrl  *m_name;
	wxTextCtrl  *m_itin;
	wxButton    *m_removebutton,
		    *m_savebutton,
		    *m_closebutton;
	DECLARE_EVENT_TABLE()
};

#endif // _SWITCHBOARDNAMEDIALOG_H
