/*	AssignDialog.h - Created by Giampiero Caprino

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

#ifndef _ASSIGNDIALOG_H
#define _ASSIGNDIALOG_H

#include <wx/listctrl.h>
#include <wx/button.h>

class AssignDialog : public wxDialog
{
public:
	AssignDialog(wxWindow *parent);
	~AssignDialog();

	int	ShowModal(Train *tr);

	wxStaticText	*m_header;
	wxListCtrl	*m_list;
	wxButton	*m_assign,
			*m_shunt,
			*m_assignshunt,
			*m_reverseassign,
			*m_split,
			*m_properties,
			*m_cancel;

	void	OnAssign(wxCommandEvent& event);
	void	OnShunt(wxCommandEvent& event);
	void	OnAssignAndShunt(wxCommandEvent& event);
	void	OnReverseAndAssign(wxCommandEvent& event);
	void	OnSplit(wxCommandEvent& event);
	void	OnProperties(wxCommandEvent& event);
	void	OnUpdate(wxUpdateUIEvent& event);

private:
	long	assign_train_from_dialog(void);

	DECLARE_EVENT_TABLE()
};

#endif // _ASSIGNDIALOG_H
