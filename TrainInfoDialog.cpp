/*	TrainInfoDialog.cpp - Created by Giampiero Caprino

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
#include "TrainInfoDialog.h"

extern	void	ShowTrainInfo(Train *trn);

BEGIN_EVENT_TABLE(TrainInfoDialog, wxDialog)
	EVT_BUTTON(ID_PRINT, TrainInfoDialog::OnPrint)
END_EVENT_TABLE()

wxTextCtrl  *TrainInfoDialog::AddTextLine(wxBoxSizer *column, const wxChar *txt)
{
	wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );
	wxStaticText    *header = new wxStaticText( this, 0, LV(txt));
	wxTextCtrl	*txtctrl = new wxTextCtrl( this, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row->Add(header, 50, wxALIGN_LEFT | wxRIGHT, 5);
	row->Add(txtctrl, 50, wxGROW | wxALIGN_RIGHT, 5);

	column->Add(row, 1, wxGROW | wxALL, 5);

	return txtctrl;
}

TrainInfoDialog::TrainInfoDialog(wxWindow *parent)
: wxDialog(parent, 0, L("Train Properties"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Train Properties"))
{
	wxBoxSizer  *buttonCol = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer  *column = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer  *leftColumn = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer  *row = new wxBoxSizer( wxHORIZONTAL );
	wxListItem  col;

	m_name	     = AddTextLine(column, L("Train name"));
	m_type	     = AddTextLine(column, L("Train type"));
	m_entryPoint = AddTextLine(column, L("Entry point"));
	m_entryTime  = AddTextLine(column, L("Entry time (hh:mm)"));
	m_exitPoint  = AddTextLine(column, L("Exit point"));
	m_exitTime   = AddTextLine(column, L("Exit time (hh:mm)"));
	m_waitFor    = AddTextLine(column, L("Wait arrival of train"));
	m_stockFor   = AddTextLine(column, L("Stock for train"));
	m_runsOn     = AddTextLine(column, L("Runs on"));
	m_length     = AddTextLine(column, L("Train length :"));
	m_maxSpeed   = AddTextLine(column, L("Max. speed :"));

	m_stops	     = new TrainInfoList(this, L("Stops"));
//	m_stops      = new wxGrid();
	m_notes      = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
	    wxSize(200, 70), wxTE_MULTILINE);

	leftColumn->Add(m_stops, 8, wxGROW | wxALL, 3);
	leftColumn->Add(m_notes, 2, wxGROW | wxALL, 3);

//	wxStaticLine *line = new wxStaticLine( this );
//	column->Add(line, 1, wxGROW|wxTOP|wxBOTTOM, 5);

//	leftColumn->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxGROW | wxALL, 10);

	row->Add(column, 40, wxGROW | wxRIGHT | wxLEFT, 3);
	row->Add(leftColumn, 60, wxGROW | wxRIGHT | wxLEFT, 3);

	buttonCol->Add(row, 0, wxGROW | wxALL, 5);

	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxButton(this, ID_PRINT, L("&Print")), 0, wxALL, 4);
	row->Add(new wxButton(this, wxID_OK, L("&Close")), 0, wxALL, 4);

	column->Add(row, 0, wxALIGN_RIGHT | wxGROW | wxALL, 6);
	SetSizer(buttonCol);
	buttonCol->Fit(this);
	buttonCol->SetSizeHints(this);
}

TrainInfoDialog::~TrainInfoDialog()
{
}

void	TrainInfoDialog::OnPrint(wxCommandEvent& event)
{
	ShowTrainInfo(m_train);
}

int	TrainInfoDialog::ShowModal(Train *t)
{
	wxString    buff;
	int	    i;
	wxString    str;

	m_train = t;
	m_name->SetValue(t->name);
	buff.Printf(wxT("%d"), t->type + 1);
	m_type->SetValue(buff);
	m_entryPoint->SetValue(t->entrance);
	m_entryTime->SetValue(format_time(t->timein));
	m_exitPoint->SetValue(t->exit);
	m_exitTime->SetValue(format_time(t->timeout));
	m_waitFor->SetValue(t->waitfor ? t->waitfor : wxT(""));
	m_stockFor->SetValue(t->stock ? t->stock : wxT(""));
	if(t->days) {
	    buff.Empty();
	    for(i = 0; i < 7; ++i)
		if(t->days & (1 << i))
		    buff += wxString::Format(wxT("%d"), i+1);
	    m_runsOn->SetValue(buff);
	}
	buff[0] = 0;
	if(t->length)
	    buff.Printf(wxT("%d"), t->length);
	m_length->SetValue(buff);
	buff[0] = 0;
	if(t->maxspeed)
	    buff.Printf(wxT("%d"), t->maxspeed);
	m_maxSpeed->SetValue(buff);
	m_stops->Update(t);
	str.Empty();
	for(i = 0; i < MAXNOTES; ++i) {
	    str += t->notes[i];
	    str += wxT("\r\n");
	}
	m_notes->SetValue(str);
	m_name->SetFocus();

	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
	return wxID_OK;
}
