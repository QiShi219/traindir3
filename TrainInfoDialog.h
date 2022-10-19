/*	TrainInfoDialog.h - Created by Giampiero Caprino

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

#ifndef _TRAININFODIALOG_H
#define _TRAININFODIALOG_H

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/grid.h>

class TrainInfoDialog : public wxDialog
{
public:
	TrainInfoDialog(wxWindow *parent);
	~TrainInfoDialog();

	void	OnPrint(wxCommandEvent& event);

	int	ShowModal(Train *trig);

	wxTextCtrl	*AddTextLine(wxBoxSizer *column, const wxChar *txt);
	wxTextCtrl	*m_name,
			*m_type,
			*m_entryPoint, *m_entryTime,
			*m_exitPoint, *m_exitTime,
			*m_waitFor, *m_stockFor,
			*m_runsOn, *m_length,
			*m_maxSpeed;
	wxTextCtrl	*m_notes;
//	wxGrid		*m_stops;
	TrainInfoList	*m_stops;
	Train		*m_train;

	DECLARE_EVENT_TABLE()
};

#endif // _TRAININFODIALOG_H
