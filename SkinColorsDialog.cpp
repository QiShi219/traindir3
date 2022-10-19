/*	SkinColorsDialog.cpp - Created by Giampiero Caprino

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
#include <wx/colordlg.h>

#include "Traindir3.h"
#include "SkinColorsDialog.h"


BEGIN_EVENT_TABLE(SkinColorsDialog, wxDialog)
	EVT_BUTTON(ID_CHOICE, SkinColorsDialog::OnColorChoice)
END_EVENT_TABLE()

SkinElementColor::SkinElementColor()
{
	m_b = 0;
	m_g = 0;
	m_r = 0;
	m_label = 0;
	m_rgbSelector = 0;
}

SkinElementColor::~SkinElementColor()
{
}



static	void	AddSkinRow(wxDialog *dialog, wxBoxSizer *column, const Char *txt, SkinElementColor& rgb, int rgbV)
{
	Char	buff[32];
	wxSize	sz(40, 20);
	wxStaticText *separator = new wxStaticText( dialog, 0, wxT("   "));

	wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );
	rgb.m_label = new wxStaticText( dialog, 0, LV(txt));
	rgb.m_rgbSelector = new wxButton( dialog, ID_CHOICE, L("Color..."), wxDefaultPosition, wxDefaultSize);
	sz.SetHeight(rgb.m_rgbSelector->GetSize().GetHeight());
	rgb.m_r = new wxTextCtrl( dialog, 0, wxEmptyString, wxDefaultPosition, sz);
	rgb.m_g = new wxTextCtrl( dialog, 0, wxEmptyString, wxDefaultPosition, sz);
	rgb.m_b = new wxTextCtrl( dialog, 0, wxEmptyString, wxDefaultPosition, sz);

	wxSprintf(buff, wxT("%d"), (rgbV >> 16) & 0xFF);
	rgb.m_r->SetValue(buff);
	wxSprintf(buff, wxT("%d"), (rgbV >> 8) & 0xFF);
	rgb.m_g->SetValue(buff);
	wxSprintf(buff, wxT("%d"), rgbV & 0xFF);
	rgb.m_b->SetValue(buff);

	row->Add(rgb.m_label, 35, wxALIGN_LEFT | wxRIGHT | wxTOP, 4);
	row->Add(rgb.m_r, 20, wxALIGN_RIGHT | wxTOP, 4);
	row->Add(rgb.m_g, 20, wxALIGN_RIGHT | wxTOP, 4);
	row->Add(rgb.m_b, 20, wxALIGN_RIGHT | wxTOP, 4);
	row->Add(separator, 2, wxTOP, 4);
	row->Add(rgb.m_rgbSelector, 20, wxALIGN_RIGHT | wxTOP, 4);

	column->Add(row, 1, wxGROW | wxLEFT | wxRIGHT, 10);
}

static	int RetrieveValue(SkinElementColor& el)
{
	int rv, gv, bv;
	wxString	str;

	str = el.m_r->GetValue();
	rv = wxStrtoul(str.c_str(), 0, 0);
	str = el.m_g->GetValue();
	gv = wxStrtoul(str.c_str(), 0, 0);
	str = el.m_b->GetValue();
	bv = wxStrtoul(str.c_str(), 0, 0);
	return (rv << 16) | (gv << 8) | bv;
}


SkinColorsDialog::SkinColorsDialog(wxWindow *parent, TDSkin *skn)
: wxDialog(parent, 0, L("Skin Colors"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Skin Colors"))
{

	m_skin = skn;

	wxBoxSizer	*column = new wxBoxSizer( wxVERTICAL );

	AddSkinRow(this, column, L("Background"), m_background, m_skin->background);
	AddSkinRow(this, column, L("Free Track"), m_freeTrack, m_skin->free_track);
	AddSkinRow(this, column, L("Reserved Track"), m_reservedTrack, m_skin->reserved_track);
	AddSkinRow(this, column, L("Reserved for Shunting"), m_reservedShunting, m_skin->reserved_shunting);
	AddSkinRow(this, column, L("Occupied"), m_occupiedTrack, m_skin->occupied_track);
	AddSkinRow(this, column, L("Reserved for Working"), m_workingTrack, m_skin->working_track);
	AddSkinRow(this, column, L("Switch Outline"), m_outline, m_skin->outline);
	AddSkinRow(this, column, L("Text"), m_text, m_skin->text);

	wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );
	row->Add(new wxButton(this, wxID_CANCEL, L("Cance&l")), 0, wxALL, 4);
	row->Add(new wxButton(this, wxID_OK, L("&Close")), 0, wxALL, 4);
	column->Add(row, 0, wxALIGN_RIGHT | wxGROW | wxALL, 6);

	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

SkinColorsDialog::~SkinColorsDialog()
{
}

void	SkinColorsDialog::OnColorChoice(wxCommandEvent& event)
{
	wxObject* obj = event.GetEventObject();
	SkinElementColor *el = 0;
	if(obj == m_background.m_rgbSelector) {
	    el = &m_background;
	} else if(obj == m_freeTrack.m_rgbSelector) {
	    el = &m_freeTrack;
	} else if(obj == m_reservedTrack.m_rgbSelector) {
	    el = &m_reservedTrack;
	} else if(obj == m_reservedShunting.m_rgbSelector) {
	    el = &m_reservedShunting;
	} else if(obj == m_occupiedTrack.m_rgbSelector) {
	    el = &m_occupiedTrack;
	} else if(obj == m_workingTrack.m_rgbSelector) {
	    el = &m_workingTrack;
	} else if(obj == m_outline.m_rgbSelector) {
	    el = &m_outline;
	} else if(obj == m_text.m_rgbSelector) {
	    el = &m_text;
	} else
	    return;

	int rv, gv, bv;
	wxString	str;
	str = el->m_r->GetValue();
	rv = wxStrtoul(str.c_str(), 0, 0);
	str = el->m_g->GetValue();
	gv = wxStrtoul(str.c_str(), 0, 0);
	str = el->m_b->GetValue();
	bv = wxStrtoul(str.c_str(), 0, 0);
	wxColor elCol(rv, gv, bv);
	wxColourData data;
	data.SetChooseFull(true);
	data.SetColour(elCol);
	/*
	for (int i = 0; i < 16; i++)
	{
	    wxColour colour(i*16, i*16, i*16);
	    data.SetCustomColour(i, colour);
	}
	*/

	wxColourDialog dialog(this, &data);
	if (dialog.ShowModal() == wxID_OK)
	{
	    wxColourData retData = dialog.GetColourData();
	    wxColour col = retData.GetColour();
	    int v = col.Red();
	    str.Printf(wxT("%d"), v);
	    el->m_r->SetValue(str);

	    v = col.Green();
	    str.Printf(wxT("%d"), v);
	    el->m_g->SetValue(str);

	    v = col.Blue();
	    str.Printf(wxT("%d"), v);
	    el->m_b->SetValue(str);

	    //wxBrush brush(col, wxSOLID);
	    //myWindow->SetBackground(brush);
	}
}


int	SkinColorsDialog::ShowModal()
{
	wxString    str;

	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	//m_script->SetFocus();

	
	int res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
	if(res != wxID_OK)
	    return wxID_CANCEL;


	m_skin->background = RetrieveValue(this->m_background);
	m_skin->free_track = RetrieveValue(this->m_freeTrack);
	m_skin->occupied_track = RetrieveValue(this->m_occupiedTrack);
	m_skin->outline = RetrieveValue(this->m_outline);
	m_skin->reserved_shunting = RetrieveValue(this->m_reservedShunting);
	m_skin->reserved_track = RetrieveValue(this->m_reservedTrack);
	m_skin->working_track = RetrieveValue(this->m_workingTrack);
	m_skin->text = RetrieveValue(this->m_text);
	return wxID_OK;
}
