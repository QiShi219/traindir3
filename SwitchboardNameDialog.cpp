/*	ItineraryDialog.cpp - Created by Giampiero Caprino

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
#include "SwitchBoard.h"
#include "SwitchboardNameDialog.h"

BEGIN_EVENT_TABLE(SwitchboardNameDialog, wxDialog)
	EVT_BUTTON(wxID_RESET, SwitchboardNameDialog::OnRemove)
	EVT_BUTTON(wxID_OK, SwitchboardNameDialog::OnSave)
	EVT_BUTTON(wxID_CANCEL, SwitchboardNameDialog::OnClose)
//	EVT_BUTTON(wxID_CANCEL, ItineraryKeyDialog::OnClose)
END_EVENT_TABLE()


extern	void	do_command(const Char *cmd, bool sendToClients);
extern	SwitchBoard *curSwitchBoard;
extern	void	ShowSwitchboard(void);

SwitchboardNameDialog::SwitchboardNameDialog(wxWindow *parent)
: wxDialog(parent, 0, wxT("Switchboard Name"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Switchboard Name"))
{
	wxArrayString   strings;
	wxBoxSizer	*column = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText	*header = new wxStaticText( this, 0, L("&Visible Name:"));
	m_name = new wxTextCtrl( this, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT, 4);
	row->Add(m_name, 65, wxGROW | wxALIGN_RIGHT | wxLEFT, 6);

	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);

	header = new wxStaticText( this, 0, L("&File name:"));
	m_path = new wxTextCtrl( this, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row = new wxBoxSizer( wxHORIZONTAL );

	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT, 4);
	row->Add(m_path, 65, wxGROW | wxALIGN_RIGHT | wxLEFT, 6);

	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);

	//	wxStaticLine *line = new wxStaticLine( this );

//	column->Add(line);

	row = new wxBoxSizer( wxHORIZONTAL );

	m_removebutton = new wxButton(this, wxID_RESET, L("&Remove"));
	row->Add(m_removebutton, 0, wxTOP | wxGROW, 10);
	m_savebutton = new wxButton(this, wxID_OK, L("&Save"));
	m_savebutton->SetDefault();
	row->Add(m_savebutton, 0, wxTOP | wxGROW, 10);
	m_closebutton = new wxButton(this, wxID_CANCEL, L("Cl&ose"));
	row->Add(m_closebutton, 0, wxTOP | wxGROW, 10);

	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);

	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

SwitchboardNameDialog::~SwitchboardNameDialog()
{
}

int	SwitchboardNameDialog::ShowModal(SwitchBoard *sb)
{
	wxChar	buff[128];
	wxChar	buff2[128];
	wxString    buff1;

	if(sb) {
	    m_name->SetValue(sb->_name);
	    m_path->SetValue(sb->_fname);
	}

	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	m_name->SetFocus();
	int res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;

	buff[sizeof(buff)/sizeof(buff[0]) - 1] = 0;
	wxStrncpy(buff, m_name->GetValue().c_str(), sizeof(buff)/sizeof(wxChar) - 1);
	buff2[sizeof(buff2)/sizeof(buff2[0]) - 1] = 0;
	wxStrncpy(buff2, m_path->GetValue().c_str(), sizeof(buff2)/sizeof(wxChar) - 1);

	if(res == wxID_RESET) {
	    buff1.Printf(wxT("sb-edit -d %s"), buff2);
	    do_command(buff1.c_str(), false);
	    return wxID_OK;
	}
        if(res != wxID_OK)
	    return wxID_CANCEL;

	buff1.Printf(wxT("sb-edit -a %s %s"), buff2, buff);
	do_command(buff1.c_str(), false);
	return wxID_OK;
}

void	SwitchboardNameDialog::OnRemove(wxCommandEvent& event)
{
	EndModal(wxID_RESET);
}

void	SwitchboardNameDialog::OnSave(wxCommandEvent& event)
{
	EndModal(wxID_OK);
}

void	SwitchboardNameDialog::OnClose(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL);
}


//
//
//


BEGIN_EVENT_TABLE(SwitchboardCellDialog, wxDialog)
	EVT_BUTTON(wxID_RESET, SwitchboardCellDialog::OnRemove)
	EVT_BUTTON(wxID_OK, SwitchboardCellDialog::OnSave)
	EVT_BUTTON(wxID_CANCEL, SwitchboardCellDialog::OnClose)
//	EVT_BUTTON(wxID_CANCEL, ItineraryKeyDialog::OnClose)
END_EVENT_TABLE()


SwitchboardCellDialog::SwitchboardCellDialog(wxWindow *parent)
: wxDialog(parent, 0, wxT("Switchboard Name"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Switchboard Name"))
{
	wxArrayString   strings;
	wxBoxSizer	*column = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText	*header = new wxStaticText( this, 0, L("&Label"));
	m_name = new wxTextCtrl( this, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT, 4);
	row->Add(m_name, 65, wxGROW | wxALIGN_RIGHT | wxLEFT, 6);

	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);

	row = new wxBoxSizer( wxHORIZONTAL );

	header = new wxStaticText( this, 0, L("&Itinerary Name"));
	m_itin = new wxTextCtrl( this, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT, 4);
	row->Add(m_itin, 65, wxGROW | wxALIGN_RIGHT | wxLEFT, 6);

	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);
//	wxStaticLine *line = new wxStaticLine( this );

//	column->Add(line);

	row = new wxBoxSizer( wxHORIZONTAL );

	m_removebutton = new wxButton(this, wxID_RESET, L("&Remove"));
	row->Add(m_removebutton, 0, wxTOP | wxGROW, 10);
	m_savebutton = new wxButton(this, wxID_OK, L("&Save"));
	m_savebutton->SetDefault();
	row->Add(m_savebutton, 0, wxTOP | wxGROW, 10);
	m_closebutton = new wxButton(this, wxID_CANCEL, L("Cl&ose"));
	row->Add(m_closebutton, 0, wxTOP | wxGROW, 10);

	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);

	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

SwitchboardCellDialog::~SwitchboardCellDialog()
{
}

int	SwitchboardCellDialog::ShowModal(int x, int y)
{
	Char	buff[128];
	Char	itinName[128];
	SwitchBoard *sb = curSwitchBoard;
	SwitchBoardCell *cell;

	if(sb) {
	    cell = sb->Find(x, y);
	    if(cell) {
		m_name->SetValue(cell->_text);
		m_itin->SetValue(cell->_itinerary);
	    }
	}
	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	m_name->SetFocus();
	int res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
        if(res != wxID_OK) {
	    if(res == wxID_RESET && cell) {
		sb->Remove(cell);
	    }
	    return wxID_CANCEL;
	}

	buff[sizeof(buff)/sizeof(buff[0]) - 1] = 0;
	wxStrncpy(buff, m_name->GetValue().c_str(), sizeof(buff)/sizeof(wxChar) - 1);
	itinName[sizeof(itinName)/sizeof(itinName[0]) - 1] = 0;
	wxStrncpy(itinName, m_itin->GetValue().c_str(), sizeof(itinName)/sizeof(wxChar) - 1);
///	buff1.Printf(wxT("sb-cell %d,%d %s, %s"), x, y, buff, itinName);
///	do_command(buff1.c_str(), false);

	if(!cell) {
	    cell = new SwitchBoardCell();
	    cell->_x = x;
	    cell->_y = y;
	    sb->Add(cell);
	}
	cell->_text = buff;
	cell->_itinerary = itinName;
	ShowSwitchboard();
	return wxID_OK;
}

void	SwitchboardCellDialog::OnRemove(wxCommandEvent& event)
{
	EndModal(wxID_RESET);
}

void	SwitchboardCellDialog::OnSave(wxCommandEvent& event)
{
	EndModal(wxID_OK);
}

void	SwitchboardCellDialog::OnClose(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL);
}

