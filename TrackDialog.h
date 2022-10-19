/*	TrackDialog.h - Created by Giampiero Caprino

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

#ifndef _TRACKDIALOG_H
#define _TRACKDIALOG_H

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/textctrl.h>

class TrackDialog : public wxDialog
{
public:
	TrackDialog(wxWindow *parent);
	~TrackDialog();

	int	ShowModal(Track *);

	void	OnScript(wxCommandEvent& event);

	wxTextCtrl  *m_length,
		    *m_name,
		    *m_km,
		    *m_speeds,
		    *m_link_east,
		    *m_link_west,
                    *m_power,
                    *m_line_num;
	wxButton    *m_scripts;
	wxCheckBox  *m_invisible;
	wxCheckBox  *m_dontstop;

        wxStaticText *m_stationLabel;
        wxStaticText *m_speedLabel;

	Track	    *m_track;

	wxFont	    m_font;
	wxColor	    m_textColor;

	DECLARE_EVENT_TABLE()
};

#endif // _TRACKDIALOG_H
