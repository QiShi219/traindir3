/*	AssignDialog.cpp - Created by Giampiero Caprino

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

#include <string.h>
#include "wx/wx.h"
#include "wx/image.h"
#include "MainFrm.h"
#include "Traindir3.h"
#include "AssignDialog.h"

extern	pxmap	*pixmaps;
extern	int	npixmaps;

static	const wxChar	*en_titles[] = { wxT("Train"), wxT("Departure"), wxT("Platform"),
				wxT("Destination"), wxT("Arrival"), wxT("Notes"), 0 };
static	const wxChar	*titles[sizeof(en_titles)/sizeof(wxChar *)];
static	int	col_widths[] = { 60, 60, 40, 150, 80, 80, 0 };

BEGIN_EVENT_TABLE(AssignDialog, wxDialog)
	EVT_BUTTON(ID_ASSIGN, AssignDialog::OnAssign)
	EVT_BUTTON(ID_ASSIGNSHUNT, AssignDialog::OnAssignAndShunt)
	EVT_BUTTON(ID_REVERSEASSIGN, AssignDialog::OnReverseAndAssign)
	EVT_BUTTON(ID_SHUNT, AssignDialog::OnShunt)
	EVT_BUTTON(ID_SPLIT, AssignDialog::OnSplit)
	EVT_BUTTON(ID_PROPERTIES, AssignDialog::OnProperties)
	EVT_UPDATE_UI(ID_LIST, AssignDialog::OnUpdate)
END_EVENT_TABLE()

static  AssignInfo  assignInfo;

AssignDialog::AssignDialog(wxWindow *parent)
: wxDialog(parent, 0, L("Assign rolling stock"),
	   wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Assign rolling stock"))
{
	int	    i;

        assignInfo.assign_tr = 0;
        assignInfo.candidates.Clear();
	if(!titles[0])
	    localizeArray(titles, en_titles);

	wxBoxSizer	    *row = new wxBoxSizer( wxHORIZONTAL );
	wxBoxSizer	    *col = new wxBoxSizer( wxVERTICAL );

	m_header = new wxStaticText(this, 0, wxString(wxT('M'), 60));
	col->Add(m_header, 0, wxRIGHT, 5);

	m_list = new wxListCtrl(this, ID_LIST, wxDefaultPosition,
	  wxDefaultSize, wxLC_REPORT|wxLC_HRULES|wxLC_SINGLE_SEL);

	col->Add(m_list, 1, wxGROW | wxTOP | wxBOTTOM, 5);

	row->Add(col, 1, wxGROW | wxALL, 5);

	col = new wxBoxSizer( wxVERTICAL );

	m_assign = new wxButton(this, ID_ASSIGN, L("&Assign"));
	m_assign->SetDefault();
	col->Add(m_assign, 0, wxTOP | wxGROW, 10);
	m_shunt = new wxButton(this, ID_SHUNT, L("S&hunt"));
	col->Add(m_shunt, 0, wxTOP | wxGROW, 10);
	m_assignshunt = new wxButton(this, ID_ASSIGNSHUNT, L("Assign&+Shunt"));
	col->Add(m_assignshunt, 0, wxTOP | wxGROW, 10);
	m_reverseassign = new wxButton(this, ID_REVERSEASSIGN, L("&Reverse+Assign"));
	col->Add(m_reverseassign, 0, wxTOP | wxGROW, 10);
	m_split = new wxButton(this, ID_SPLIT, L("Sp&lit"));
	col->Add(m_split, 0, wxTOP | wxGROW, 10);
	m_properties = new wxButton(this, ID_PROPERTIES, L("&Properties"));
	m_properties->Disable();
	col->Add(m_properties, 0, wxTOP | wxGROW, 10);
	m_cancel = new wxButton(this, wxID_CANCEL, L("&Cancel"));
	col->Add(m_cancel, 0, wxTOP | wxGROW, 10);

	row->Add(col, 0, wxALL, 5);

	wxListItem  listcol;

	//  Insert columns

	for(i = 0; titles[i]; ++i) {
	    listcol.SetText(titles[i]);
//	    listcol.SetImage(-1);
	    m_list->InsertColumn(i, listcol);
	    m_list->SetColumnWidth(i, col_widths[i]);
	}
	SetSizer(row);
	row->Fit(this);
	row->SetSizeHints(this);
	m_header->SetLabel(wxT(""));
}

AssignDialog::~AssignDialog()
{
	freeLocalizedArray(titles);
}

long	AssignDialog::assign_train_from_dialog(void)
{
	int	i = m_list->GetSelectedItemCount();

	if(i != 1)
	    return -1;

	long	l = m_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	if(l >= 0) {
	    assign_train(assignInfo.candidates[l], assignInfo.assign_tr);
	}
	EndModal(wxID_OK);
	return l;
}

void	AssignDialog::OnAssign(wxCommandEvent& event)
{
	assign_train_from_dialog();
}

void	AssignDialog::OnAssignAndShunt(wxCommandEvent& event)
{
	long t = assign_train_from_dialog();
	if(t >= 0)
	    shunt_train(assignInfo.candidates[t]);
}

void	AssignDialog::OnReverseAndAssign(wxCommandEvent& event)
{
	int	i = m_list->GetSelectedItemCount();

	if(i != 1)
	    return;

	long	l = m_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	if(l < 0)
	    return;

	if(!reverse_train(assignInfo.assign_tr))
	    return;
	assign_train_from_dialog();
}

void	AssignDialog::OnShunt(wxCommandEvent& event)
{
	shunt_train(assignInfo.assign_tr);
	EndModal(wxID_OK);
}

void	AssignDialog::OnSplit(wxCommandEvent& event)
{
	int	l = 0;

	if(assignInfo.assign_tr->length) {
	    l = ask_number(wxT("Split train"), wxT("Position where to split the train (meters from the head)"));
	    if(l < 0)
		return;
	}
	split_train(assignInfo.assign_tr, l);
	shunt_train(assignInfo.assign_tr);
	EndModal(wxID_OK);
}

void	AssignDialog::OnProperties(wxCommandEvent& event)
{
}

void	AssignDialog::OnUpdate(wxUpdateUIEvent& event)
{
	long	l = m_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	bool	enable = false;
	if(l >= 0)
	    enable = true;

	m_assign->Enable(enable);
	m_assignshunt->Enable(enable);
	m_reverseassign->Enable(enable);	
}

int	AssignDialog::ShowModal(Train *t)
{
	wxChar	buff[256];
	wxChar	*p;
	int	i;
	Track	*trk;

        if (!t->Get(assignInfo)) {
//	trk = findTrack(t->position->x, t->position->y);
	    wxMessageBox(L("Train has already been assigned."), wxT("Error"),
		wxOK|wxICON_ERROR, traindir->m_frame);
	    return 0;
	}
        trk = assignInfo.track;
        t = assignInfo.assign_tr;

        wxSnprintf(buff, 256, L("Assigning stock of train %s arrived at station %s"),
	    t->name, trk->station);
	m_header->SetLabel(buff);

	m_list->DeleteAllItems();
	m_list->Freeze();

	for(i = 0; i < assignInfo.candidates.Length(); ++i) {
	    Train   *t1 = assignInfo.candidates[i];
	    m_list->InsertItem(i, t1->name);
	    m_list->SetItem(i, 1, format_time(t1->timein));
	    wxStrcpy(buff, t1->entrance);
	    if((p = wxStrchr(buff, '@'))) {
		*p = 0;
	        m_list->SetItem(i, 2, p + 1);
	    }
	    m_list->SetItem(i, 3, t1->exit);
	    m_list->SetItem(i, 4, format_time(t1->timeout));

	    wxString	notes;
	    int		n;

	    for(n = 0; n < MAXNOTES; ++n) {
		notes += t1->notes[n] ? t1->notes[n] : wxT("");
		notes += wxT(" ");
	    }
	    m_list->SetItem(i, 5, notes);
	}
	m_list->Thaw();
        if(assignInfo.assign_to >= 0) {
            m_list->SetItemState(assignInfo.assign_to, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}

	if(t->flags & TFLG_STRANDED)
	    m_reverseassign->Disable();
	else
	    m_reverseassign->Enable();
	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	int res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
	return res;
}

