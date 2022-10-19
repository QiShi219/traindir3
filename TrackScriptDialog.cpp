/*	TrackScriptDialog.cpp - Created by Giampiero Caprino

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

#include <wx/wxprec.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/statline.h>

#include "Traindir3.h"
#include "TrackScriptDialog.h"

static	wxTextCtrl  *AddTextLine(wxDialog *dialog, wxBoxSizer *column, const wxChar *txt)
{
	wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );
	wxStaticText    *header = new wxStaticText( dialog, 0, LV(txt));
	wxTextCtrl	*txtctrl = new wxTextCtrl( dialog, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT | wxTOP, 4);
	row->Add(txtctrl, 65, wxGROW | wxALIGN_RIGHT | wxTOP, 4);

	column->Add(row, 1, wxGROW | wxLEFT | wxRIGHT, 10);

	return txtctrl;
}

TrackScriptDialog::TrackScriptDialog(wxWindow *parent)
: wxDialog(parent, 0, L("Track Script"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Track Properties"))
{
	wxBoxSizer	*column = new wxBoxSizer( wxVERTICAL );

	m_script = new wxTextCtrl( this, 0, wxEmptyString, wxDefaultPosition, wxSize(260, 200), wxTE_MULTILINE);
	column->Add(m_script, 0, wxALIGN_LEFT | wxGROW, 4);

	wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );
	row->Add(new wxButton(this, wxID_CANCEL, L("&Cancel")), 0, wxALL, 4);
	row->Add(new wxButton(this, wxID_OK, L("&Close")), 0, wxALL, 4);
	column->Add(row, 0, wxALIGN_RIGHT | wxGROW | wxALL, 6);

	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

TrackScriptDialog::~TrackScriptDialog()
{
}

int	TrackScriptDialog::ShowModal(Track *trk)
{
	wxString    str;

	if(trk->stateProgram)
	    m_script->SetValue(trk->stateProgram);
	else {
	    m_script->SetValue(wxT("OnEnter:\nend\n\nOnExit:\nend\n\nOnClicked:\nend\n\n"));
	}
	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	m_script->SetFocus();
	int res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
	if(res != wxID_OK)
	    return wxID_CANCEL;

	str = m_script->GetValue();
	if(trk->stateProgram)
	    free(trk->stateProgram);
        int len = str.length();
        if (len > 0 && str.GetChar(len - 1) != wxT('\n'))
            str.append(wxT("\n"));
	trk->stateProgram = wxStrdup(str.c_str());
	trk->ParseProgram();		// update internal representation of the script
					// TODO: check if parsed correctly
	return wxID_OK;
}
