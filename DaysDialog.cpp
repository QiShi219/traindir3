/*	DaysDialog.cpp - Created by Giampiero Caprino

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
#include <wx/statline.h>
#include <wx/arrstr.h>

#include "Traindir3.h"
#include "DaysDialog.h"

static	const wxChar	*names[] = {
	wxT("Monday"),
	wxT("Tuesday"),
	wxT("Wednesday"),
	wxT("Thursday"),
	wxT("Friday"),
	wxT("Saturday"),
	wxT("Sunday"),
	0
};

DaysDialog::DaysDialog(wxWindow *parent)
: wxDialog(parent, 0, L("Day Selection"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L(" Days "))
{
	int		i;
	wxArrayString   strings;
	wxBoxSizer	*column = new wxBoxSizer( wxVERTICAL );
	wxStaticText    *header = new wxStaticText( this, 0, 
	    L("Not all trains run every day of the week.\nWhich day do you want to simulate?"));
    
	column->Add(header, 0, wxALIGN_LEFT | wxALL, 10);

	for(i = 0; names[i]; ++i) {
	    strings.Add(LV(names[i]));
	}

	m_radio_box = new wxRadioBox( this, ID_RADIOBOX,
	    L(""), wxDefaultPosition, wxDefaultSize,
	    strings, 1, wxRA_SPECIFY_COLS );

	column->Add(m_radio_box, 1, wxGROW | wxALIGN_LEFT | wxALL, 10);

//	wxStaticLine *line = new wxStaticLine( this, wxID_STATIC,
//	    wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

//	column->Add(line);

	wxButton	*ok = new wxButton( this, wxID_OK, L("&Continue"));

	ok->SetDefault();
	column->Add(ok, 0, wxALIGN_CENTER | wxALL, 10);

	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

DaysDialog::~DaysDialog()
{
}

int	DaysDialog::ShowModal()
{
	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	int res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
	run_day = 1 << m_radio_box->GetSelection();
	return res;
}

const wxChar	*format_day(int day)
{
	int	i;
	int	c;

	if(!day)
	    return wxT("");
	c = 0;
	for(i = 1; names[c]; i <<= 1, ++c) {
	    if(run_day & i) {
		return LV(names[c]);
	    }
	}
	return wxT("?");	    // impossible
}
