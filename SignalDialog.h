/*	SignalDialog.h - Created by Giampiero Caprino

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

#ifndef _SIGNALDIALOG_H
#define _SIGNALDIALOG_H

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/textctrl.h>

class SignalDialog : public wxDialog
{
public:
	SignalDialog(wxWindow *parent);
	~SignalDialog();

	void	OnFileBrowser(wxCommandEvent& event);

	int	ShowModal(Signal *sig);

	wxStaticText *m_coord;
	wxTextCtrl  *m_length,
		    *m_name,
		    *m_link_east,
		    *m_link_west,
                    *m_locked_by,
		    *m_script_path;
	wxCheckBox  *m_always_red,
		    *m_square_frame,
		    *m_no_penalty_on_red,
		    *m_no_penalty_on_click,
		    *m_invisible,
                    *m_intermediate;
	wxButton    *m_fileBrowser;

	DECLARE_EVENT_TABLE()
};

#endif // _SIGNALDIALOG_H
