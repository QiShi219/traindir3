/*	TriggerDialog.cpp - Created by Giampiero Caprino

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

#include "Traindir3.h"
#include "TriggerDialog.h"

BEGIN_EVENT_TABLE(TriggerDialog, wxDialog)
	EVT_CHOICE(ID_CHOICE, TriggerDialog::OnChoice)
END_EVENT_TABLE()

extern	int	set_track_properties(Track *t, const wxChar *len, const wxChar *station, const wxChar *speed,
			     const wxChar *distance, const wxChar *wlink, const wxChar *elink,const wxChar *num);

const wxChar	*trigger_actions[] = {
	wxT("click x,y"),
	wxT("rclick x,y"),
	wxT("ctrlclick x,y"),
	wxT("fast"),
	wxT("slow"),
	wxT("shunt train"),
	wxT("reverse train"),
	wxT("traininfo train"),
	wxT("stationinfo train"),
	wxT("accelerate speed train"),
	wxT("decelerate speed train"),
	wxT("assign train"),
	wxT("play sound"),
	wxT("itinerary"),
	wxT("script"),
	0
};

static	wxTextCtrl  *AddTextLine(wxDialog *dialog, wxBoxSizer *column, const wxChar *txt)
{
	wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );
	wxStaticText    *header = new wxStaticText( dialog, 0, LV(txt));
	wxTextCtrl	*txtctrl = new wxTextCtrl( dialog, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row->Add(header, 50, wxGROW | wxLEFT, 10);
	row->Add(txtctrl, 50, wxGROW | wxLEFT, 10);

	column->Add(row, 1, wxGROW | wxALL, 10);

	return txtctrl;
}

void	TriggerDialog::OnChoice(wxCommandEvent& event)
{
	int idx = action_list->GetSelection();
	m_name->SetValue(trigger_actions[idx]);
}

TriggerDialog::TriggerDialog(wxWindow *parent)
: wxDialog(parent, 0, L("Trigger properties"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Trigger properties"))
{
	int		i;
	wxArrayString   strings;
	wxBoxSizer	*column = new wxBoxSizer( wxVERTICAL );

	for(i = 0; trigger_actions[i]; ++i)
	    strings.Add(trigger_actions[i]);

	m_coord = new wxStaticText(this, wxID_ANY, wxT(""));
	column->Add(m_coord, 0, wxGROW | wxALL, 10);

	wxStaticText *txt = new wxStaticText(this, wxID_ANY, L("Action:      ('@' in action = name of triggering train)"));
	column->Add(txt, 0, wxGROW | wxALL, 10);

	wxBoxSizer	    *row = new wxBoxSizer( wxHORIZONTAL );
	action_list = new wxChoice( this, ID_CHOICE, wxDefaultPosition, wxDefaultSize, strings);
	m_name = new wxTextCtrl( this, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row->Add(action_list, 50, wxGROW | wxLEFT, 10);
	row->Add(m_name, 50, wxGROW | wxRIGHT, 10);

	column->Add(row, 1, wxGROW | wxALL, 10);

	m_links = AddTextLine(this, column, L("Linked to track at coord :"));
	m_probabilities = AddTextLine(this, column, L("Probabilities for action :"));

	m_invisible = new wxCheckBox(this, wxID_ANY, L("Hidden"), wxDefaultPosition, wxDefaultSize);
	column->Add(m_invisible, 0, wxLEFT|wxRIGHT|wxTOP, 10);

	column->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxGROW | wxALL, 10);

	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

TriggerDialog::~TriggerDialog()
{
}

int	TriggerDialog::ShowModal(Track *trk)
{
	int	    res;
	wxChar	    buff[512];
	int	    i;
	wxChar	    *p;
	wxString    str;

	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), L("Trigger at  %d,%d"), trk->x, trk->y);
	m_coord->SetLabel(buff);
	buff[0] = 0;
	if(trk->station)		// actually the name of the triggering train
	    wxStrcpy(buff, trk->station); 
	m_name->SetValue(buff);

   
	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%d,%d"), trk->wlinkx, trk->wlinky);
	m_links->SetValue(buff);
	p = buff;
	for(i = 0; i < NTTYPES; ++i) {
	    wxSprintf(p, wxT("%d/"), trk->speed[i]);
	    p += wxStrlen(p);
	}
	p[-1] = 0;
	m_probabilities->SetValue(buff);
	m_invisible->SetValue(trk->invisible);
	m_name->SetFocus();

	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
	if(res != wxID_OK)
	    return res;

	set_track_properties(trk, wxT(""), m_name->GetValue().c_str(),
	    m_probabilities->GetValue().c_str(), wxT(""), m_links->GetValue().c_str(), wxT(""),wxT(""));    //20201209
	trk->invisible = m_invisible->GetValue() ? 1 : 0;
	return res;
}
