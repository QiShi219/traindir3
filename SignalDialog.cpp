/*	SignalDialog.cpp - Created by Giampiero Caprino

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
#include "SignalDialog.h"

BEGIN_EVENT_TABLE(SignalDialog, wxDialog)
	EVT_BUTTON(ID_PROPERTIES, SignalDialog::OnFileBrowser)
END_EVENT_TABLE()

extern	void	delete_script_data(TrackBase *t);

static	wxTextCtrl  *AddTextLine(wxDialog *dialog, wxBoxSizer *column, const wxChar *txt)
{
	wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );
	wxStaticText    *header = new wxStaticText( dialog, 0, LV(txt));
	wxTextCtrl	*txtctrl = new wxTextCtrl( dialog, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT, 4);
	row->Add(txtctrl, 65, wxGROW | wxALIGN_RIGHT | wxLEFT, 6);

	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);

	return txtctrl;
}

static	wxTextCtrl  *AddScriptLine(SignalDialog *dialog, wxBoxSizer *column, const wxChar *txt)
{
	wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );
	wxStaticText    *header = new wxStaticText( dialog, 0, LV(txt));
	wxTextCtrl	*txtctrl = new wxTextCtrl( dialog, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	wxSize		size = txtctrl->GetSize();

	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT, 4);
	row->Add(txtctrl, 65, wxGROW | wxALIGN_RIGHT | wxLEFT, 6);
	dialog->m_fileBrowser = new wxButton(dialog, ID_PROPERTIES, wxT("..."), wxDefaultPosition, wxSize(32, size.y + 2));
	row->Add(dialog->m_fileBrowser, 0, wxTOP | wxALIGN_RIGHT, 4);

	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);

	return txtctrl;
}

SignalDialog::SignalDialog(wxWindow *parent)
: wxDialog(parent, 0, L("Signal properties"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Signal properties"))
{
	wxBoxSizer	    *column = new wxBoxSizer( wxVERTICAL );

	m_coord = new wxStaticText(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	column->Add(m_coord, 0, wxLEFT|wxTOP, 10);

	m_name = AddTextLine(this, column, L("Signal name :"));
	m_link_east = AddTextLine(this, column, L("Linked to east :"));
	m_link_west = AddTextLine(this, column, L("Linked to west :"));
	m_locked_by = AddTextLine(this, column, L("Blocked by :"));

	m_always_red = new wxCheckBox(this, wxID_ANY, L("Signal is always red"), wxDefaultPosition, wxDefaultSize);
	column->Add(m_always_red, 0, wxLEFT|wxRIGHT|wxTOP, 10);

	m_square_frame = new wxCheckBox(this, wxID_ANY, L("Signal has square frame"), wxDefaultPosition, wxDefaultSize);
	column->Add(m_square_frame, 0, wxLEFT|wxRIGHT|wxTOP, 10);

	m_no_penalty_on_red = new wxCheckBox(this, wxID_ANY, L("No penalty for train stopping at this signal"), wxDefaultPosition, wxDefaultSize);
	column->Add(m_no_penalty_on_red, 0, wxLEFT|wxRIGHT|wxTOP, 10);

	m_no_penalty_on_click = new wxCheckBox(this, wxID_ANY, L("No penalty for un-necessary clicks"), wxDefaultPosition, wxDefaultSize);
	column->Add(m_no_penalty_on_click, 0, wxLEFT|wxRIGHT|wxTOP, 10);

	m_invisible = new wxCheckBox(this, wxID_ANY, L("Hidden"), wxDefaultPosition, wxDefaultSize);
	column->Add(m_invisible, 0, wxLEFT|wxRIGHT|wxTOP, 10);

	m_intermediate = new wxCheckBox(this, wxID_ANY, L("Intermediate"), wxDefaultPosition, wxDefaultSize);
	column->Add(m_intermediate, 0, wxLEFT|wxRIGHT|wxTOP, 10);

	m_script_path = AddScriptLine(this, column, L("Script file :"));

//	wxStaticLine *line = new wxStaticLine( this );
//	column->Add(line, 1, wxGROW|wxTOP|wxBOTTOM, 5);
	
	column->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxGROW | wxALL, 10);

	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

SignalDialog::~SignalDialog()
{
}

void	SignalDialog::OnFileBrowser(wxCommandEvent& event)
{
	wxChar	path[256];
	wxString strpath;

	strpath = m_script_path->GetValue();
	wxStrcpy(path, strpath.c_str());
	if(!traindir->OpenScriptDialog(path))
	    return;
	m_script_path->SetValue(path);
}


int	SignalDialog::ShowModal(Signal *trk)
{
	int	    res;
	wxChar	    buff[256];
	wxChar	    *p;
	wxString    str;

	wxStrcpy(buff, L("Signal at coordinates :"));
	wxSnprintf(buff + wxStrlen(buff), sizeof(buff)/sizeof(wxChar) - wxStrlen(buff), wxT("   %d, %d"), trk->x, trk->y);
	m_coord->SetLabel(buff);
	buff[0] = 0;
	if(trk->station)
	    wxStrcpy(buff, trk->station);
	m_name->SetValue(buff);
	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%d,%d"), trk->elinkx, trk->elinky);
	m_link_east->SetValue(buff);
	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%d,%d"), trk->wlinkx, trk->wlinky);
	m_link_west->SetValue(buff);
        if(trk->_lockedBy)
            m_locked_by->SetValue(trk->_lockedBy);
	m_always_red->SetValue(trk->fixedred != 0);
	m_square_frame->SetValue(trk->signalx != 0);
	m_no_penalty_on_red->SetValue(trk->nopenalty != 0);
	m_no_penalty_on_click->SetValue(trk->noClickPenalty != 0);
	m_invisible->SetValue(trk->invisible != 0);
        m_intermediate->SetValue(trk->_intermediate);
        //m_intermediate->Hide();
	m_script_path->SetValue(trk->stateProgram);

	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	m_name->SetFocus();
	res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
	if(res != wxID_OK)
	    return res;

	str = m_name->GetValue();
	if(trk->station)
	    free(trk->station);
	trk->station = 0;
	if(str.length() > 0)
	    trk->station = wxStrdup(str.c_str());
	str = m_link_east->GetValue();
	trk->elinkx = trk->elinky = 0;
	if(str.length() > 0) {
	    wxStrcpy(buff, str.c_str());
	    trk->elinkx = wxStrtol(buff, &p, 10);
	    if(*p != ',')
		trk->elinkx = 0;
	    else
		trk->elinky = wxStrtol(p + 1, &p, 10);
	}
	str = m_link_west->GetValue();
	trk->wlinkx = trk->wlinky = 0;
	if(str.length() > 0) {
	    wxStrcpy(buff, str.c_str());
	    trk->wlinkx = wxStrtol(buff, &p, 10);
	    if(*p != ',')
		trk->wlinkx = 0;
	    else
		trk->wlinky = wxStrtol(p + 1, &p, 10);
	}
        if(trk->_lockedBy)
            free(trk->_lockedBy);
        trk->_lockedBy = 0;
        str = m_locked_by->GetValue();
        if(str.length() > 0)
            trk->_lockedBy = wxStrdup(str.c_str());
	trk->fixedred = m_always_red->GetValue() != 0;
	trk->signalx = m_square_frame->GetValue() != 0;
	trk->nopenalty = m_no_penalty_on_red->GetValue() != 0;
	trk->noClickPenalty = m_no_penalty_on_click->GetValue() != 0;
	trk->invisible = m_invisible->GetValue() != 0;
        trk->_intermediate = m_intermediate->GetValue() != 0;
	str = m_script_path->GetValue();
	if(trk->stateProgram) {
	    if(!wxStrcmp(str.c_str(), trk->stateProgram))
		return wxID_OK;
	    free(trk->stateProgram);
	}
	Signal *sig = trk;

	delete_script_data(trk);
	trk->stateProgram = wxStrdup(str.c_str());
	sig->ParseProgram();
        return wxID_OK;
}
