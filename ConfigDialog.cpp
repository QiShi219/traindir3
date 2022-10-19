/*	ConfigDialog.cpp - Created by Giampiero Caprino

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
#include "ConfigDialog.h"

extern	const wxChar	*locale_name;

const wxChar	*languages[] = { wxT("English"), wxT("Espanol"), wxT("Francaise"), wxT("Italiano"), wxT("Magyar"), wxT("Russian"), 0 };
const wxChar	*locales[] = { wxT(".en"), wxT(".es"), wxT(".fr"), wxT(".it"), wxT(".hu"), wxT(".ru"), 0 };

ConfigDialog::ConfigDialog(wxWindow *parent)
: wxDialog(parent, 0, L("Language Selection"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Language Selection"))
{
	int		i;
	wxArrayString   strings;
	wxBoxSizer	*column = new wxBoxSizer( wxVERTICAL );
	wxStaticText    *header = new wxStaticText( this, 0, 
	    L("Language to use next time\nTraindir is started:"));
    
	column->Add(header, 0, wxALIGN_LEFT | wxALL, 10);

	for(i = 0; languages[i]; ++i) {
	    strings.Add(languages[i]);
	}

	m_radio_box = new wxRadioBox( this, ID_RADIOBOX,
	    L(""), wxDefaultPosition, wxDefaultSize,
	    strings, 1, wxRA_SPECIFY_COLS );

	for(i = 0; locales[i]; ++i) {
	    if(!wxStrcmp(locale_name, locales[i])) {
		m_radio_box->SetSelection(i);
		break;
	    }
	}
	column->Add(m_radio_box, 1, wxGROW | wxALIGN_LEFT | wxALL, 10);
	column->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxGROW | wxALL, 10);

	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

ConfigDialog::~ConfigDialog()
{
}

int	ConfigDialog::ShowModal()
{
	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	int res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
	if(res != wxID_OK)
	    return res;

	res = m_radio_box->GetSelection();
	locale_name = locales[res];
	return wxID_OK;
}

