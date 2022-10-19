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
#include "Itinerary.h"
#include "ItineraryDialog.h"

extern	void	FillItineraryTable();

ItineraryDialog::ItineraryDialog(wxWindow *parent)
: wxDialog(parent, 0, wxT("Itinerary Properties"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L(" Itinerary Properties "))
{
	wxArrayString   strings;
	wxChar	buff[256];
	wxBoxSizer	    *column = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer	    *row = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText	    *header = new wxStaticText( this, 0, L("&Name"));
	m_name = new wxTextCtrl( this, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT, 4);
	row->Add(m_name, 65, wxGROW | wxALIGN_RIGHT | wxLEFT, 6);

	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);

	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), L("From signal 'MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM'"), 0, 0); //sig->x, sig->y);

	m_start_point = new wxStaticText(this, 0, buff);
	column->Add(m_start_point, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);

	row = new wxBoxSizer( wxHORIZONTAL );

	header = new wxStaticText( this, 0, L("Ne&xt itinerary"));
	m_next = new wxTextCtrl( this, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT, 4);
	row->Add(m_next, 65, wxGROW | wxALIGN_RIGHT | wxLEFT, 6);

	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);
//	wxStaticLine *line = new wxStaticLine( this );

//	column->Add(line);

	column->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxGROW | wxALL, 10);

	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

ItineraryDialog::~ItineraryDialog()
{
}

int	ItineraryDialog::ShowModal(Itinerary *it)
{
	wxChar	buff[128], buff1[128];

	if(it->name)
	    m_name->SetValue(it->name);
	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), L("From signal '%s'"), it->signame);
	m_start_point->SetLabel(buff);
	if(it->nextitin)
	    m_next->SetValue(it->nextitin);

	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	m_name->SetFocus();
	int res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
        if(res != wxID_OK)
	    return wxID_CANCEL;

//	if(it->name)
//	    free(it->name);
//	it->name = 0;
	buff[sizeof(buff)/sizeof(buff[0]) - 1] = 0;
	wxStrncpy(buff, m_name->GetValue().c_str(), sizeof(buff)/sizeof(wxChar) - 1);
//	if(buff[0])
//	    it->name = strdup(buff);

//	if(it->nextitin)
//	    free(it->nextitin);
//	it->nextitin = 0;
	buff1[sizeof(buff1)/sizeof(buff1[0]) - 1] = 0;
	wxStrncpy(buff1, m_next->GetValue().c_str(), sizeof(buff1)/sizeof(wxChar) - 1);
//	if(buff[0])
//	    it->nextitin = strdup(buff);
	if(!set_itin_name(it, buff, buff1))
	    return wxID_CANCEL;
	FillItineraryTable();
	return wxID_OK;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


static	const wxChar	*en_titles2[] = { wxT("Name"), wxT("Start Signal"),
			    wxT("End Signal"), wxT("Next Itinerary"), 0 };
static	const wxChar	*titles2[sizeof(en_titles2)/sizeof(wxChar *)];
static	int	col_widths2[] = { 100, 80, 80, 100, 0 };

BEGIN_EVENT_TABLE(ItineraryKeyDialog, wxDialog)
	EVT_BUTTON(ID_ITINSELECT, ItineraryKeyDialog::OnSelect)
	EVT_BUTTON(ID_ITINFORSHUNT, ItineraryKeyDialog::OnSelectForShunt)
	EVT_BUTTON(ID_ITINCLEAR, ItineraryKeyDialog::OnClear)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, ItineraryKeyDialog::OnActivated)
//	EVT_BUTTON(wxID_CANCEL, ItineraryKeyDialog::OnClose)
END_EVENT_TABLE()

ItineraryKeyDialog::ItineraryKeyDialog(wxWindow *parent)
: wxDialog(parent, 0, L("Select Itinerary"),
	   wxDefaultPosition, wxSize(400, 200),
	   wxDEFAULT_DIALOG_STYLE, L("Select Itinerary"))
{
	int	i;

	if(!titles2[0])
	    localizeArray(titles2, en_titles2);

	wxBoxSizer	    *row = new wxBoxSizer( wxHORIZONTAL );
	wxBoxSizer	    *col = new wxBoxSizer( wxVERTICAL );

	m_header = new wxStaticText(this, 0, wxString(wxT('M'), 80));
	col->Add(m_header, 0, wxRIGHT, 5);

	m_list = new wxListCtrl(this, ID_LIST, wxDefaultPosition,
	  wxDefaultSize, wxLC_REPORT|wxLC_HRULES|wxLC_SINGLE_SEL);

	col->Add(m_list, 1, wxGROW | wxTOP | wxBOTTOM, 5);

	row->Add(col, 1, wxGROW | wxALL, 5);

	col = new wxBoxSizer( wxVERTICAL );

	m_selectbutton = new wxButton(this, ID_ITINSELECT, L("&Select"));
	m_selectbutton->SetDefault();
	col->Add(m_selectbutton, 0, wxTOP | wxGROW, 10);
        m_selectForShunt = new wxButton(this, ID_ITINFORSHUNT, L("For S&hunting"));
	m_selectForShunt->SetDefault();
	col->Add(m_selectForShunt, 0, wxTOP | wxGROW, 10);
	m_clearbutton = new wxButton(this, ID_ITINCLEAR, L("&Clear"));
	col->Add(m_clearbutton, 0, wxTOP | wxGROW, 10);
	m_closebutton = new wxButton(this, wxID_CANCEL, L("Cl&ose"));
	col->Add(m_closebutton, 0, wxTOP | wxGROW, 10);

	row->Add(col, 0, wxALL, 5);

	wxListItem  listcol;

	//  Insert columns

	for(i = 0; titles2[i]; ++i) {
	    listcol.SetText(titles2[i]);
//	    listcol.SetImage(-1);
	    m_list->InsertColumn(i, listcol);
	    m_list->SetColumnWidth(i, col_widths2[i]);
	}
	SetSizer(row);
	row->Fit(this);
	row->SetSizeHints(this);
	m_header->SetLabel(L("Select itinerary"));
	SetSize(600, 400);
}

ItineraryKeyDialog::~ItineraryKeyDialog()
{
	freeLocalizedArray(titles2);
}

long	ItineraryKeyDialog::GetSelectedItem(void)
{
	int	i = m_list->GetSelectedItemCount();

	if(i != 1)
	    return -1;

	long	l = m_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	EndModal(wxID_OK);
	return l;
}

void	ItineraryKeyDialog::DoSelect(wxCommandEvent& event, bool forShunt)
{
	long i = GetSelectedItem();
	int j;
	Itinerary *it;

	if(i < 0)
	    return;

	j = 0;
	for(it = itineraries; it && j != i; it = it->next)
	    ++j;
	if(!it)		// impossible
	    return;
        it->Select(forShunt);
}

void	ItineraryKeyDialog::OnSelect(wxCommandEvent& event)
{
        DoSelect(event, false);
}

void	ItineraryKeyDialog::OnSelectForShunt(wxCommandEvent& event)
{
        DoSelect(event, true);
}

void	ItineraryKeyDialog::OnClear(wxCommandEvent& event)
{
	long i = GetSelectedItem();
	if(i < 0)
	    return;

	int j = 0;
	Itinerary *it;
	for(it = itineraries; it && j != i; it = it->next)
	    ++j;
	if(!it)		// impossible
	    return;

	it->Deselect(false);
}

void	ItineraryKeyDialog::OnClose(wxCommandEvent& event)
{
//	EndModal
}

void	ItineraryKeyDialog::OnActivated(wxListEvent& event)
{
	long i = GetSelectedItem();
	int j;
	Itinerary *it;

	if(i < 0)
	    return;

	j = 0;
	for(it = itineraries; it && j != i; it = it->next)
	    ++j;
	if(!it)		// impossible
	    return;
        it->Select(false);
}

void	ItineraryKeyDialog::FillItineraryList()
{
	Itinerary *it;
	int	i;

	m_list->DeleteAllItems();
	m_list->Freeze();

	i = 0;
	for(it = itineraries; it; it = it->next) {
	    m_list->InsertItem(i, it->name);
	    m_list->SetItem(i, 1, it->signame);
	    m_list->SetItem(i, 2, it->endsig);
	    m_list->SetItem(i, 3, it->nextitin);
	    ++i;
	}
	m_list->Thaw();
}

int	ItineraryKeyDialog::ShowModal()
{
	FillItineraryList();

	// Centre();	    // use last time's position
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	m_list->SetFocus();
	wxListItem item;
	item.SetId(0);
	item.SetMask(wxLIST_MASK_STATE);
	item.SetState(wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
	m_list->SetItem(item);
	int res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
	return res;
}

