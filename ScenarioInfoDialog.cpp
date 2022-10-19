/*	ScenarioInfoDialog.cpp - Created by Giampiero Caprino

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
#include "wx/listctrl.h"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/grid.h>
#include "TrainInfoList.h"

#include "Traindir3.h"
#include "ScenarioInfoDialog.h"

extern	void	ShowScenarioInfo(Train *trn);

BEGIN_EVENT_TABLE(ScenarioInfoDialog, wxDialog)
//	EVT_BUTTON(ID_PRINT, ScenarioInfoDialog::OnPrint)
END_EVENT_TABLE()


ScenarioInfoDialog::ScenarioInfoDialog(wxWindow *parent)
: wxDialog(parent, 0, L("Scenario Information"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Scenario Information"))
{
	wxBoxSizer  *buttonCol = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer  *column = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer  *leftColumn = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer  *row = new wxBoxSizer( wxHORIZONTAL );
	wxListItem  col;

	m_notes      = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
	    wxDefaultPosition, wxSize(200,100), wxTE_MULTILINE);

	leftColumn->Add(m_notes, 2, wxGROW | wxALL, 3);

	row->Add(leftColumn, 60, wxGROW | wxRIGHT | wxLEFT, 3);

	buttonCol->Add(row, 0, wxGROW | wxALL, 5);

	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxButton(this, wxID_OK, L("&OK")), 0, wxALL, 4);
	row->Add(new wxButton(this, wxID_CANCEL, L("&Cancel")), 0, wxALL, 4);

	buttonCol/*column*/->Add(row, 0, wxALIGN_RIGHT | wxGROW | wxALL, 6);
	SetSizer(buttonCol);
	buttonCol->Fit(this);
	buttonCol->SetSizeHints(this);
}

ScenarioInfoDialog::~ScenarioInfoDialog()
{
}

int	ScenarioInfoDialog::ShowModal()
{
	wxChar	    buff[512];
	int	    i;
	const wxChar *p;
	wxString    str;

	str = wxT("");
	TextList    *t, *last;

	for(t = track_info; t; t = t->next) {
	    str += t->txt;
//	    str += wxT("\n");
	}
	m_notes->SetValue(str);
	m_notes->SetFocus();

	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	if(wxDialog::ShowModal() == wxID_OK) {
	    str = m_notes->GetValue();
	    while(track_info) {
		t = track_info->next;
		free(track_info->txt);
		free(track_info);
		track_info = t;
	    }
	    last = 0;
	    i = 0;
	    p = str.c_str();
	    if(wxStrlen(p) > 0 && p[wxStrlen(p)-1] != '\n')
		str += '\n';
	    for(p = str.c_str(); *p; ) {
		if(*p != '\n') {
		    buff[i++] = *p++;
		    continue;
		}
		buff[i] = 0;
		++p;
		i = 0;
		t = (TextList *)malloc(sizeof(TextList));
		t->next = 0;
		t->txt = wxStrdup(buff);
		if(!track_info)
		    track_info = t;
		else
		    last->next = t;
		last = t;
	    }
	}
	traindir->m_ignoreTimer = oldIgnore;
	return wxID_OK;
}
