/*	OptionsDialog.cpp - Created by Giampiero Caprino

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
#include <wx/radiobox.h>
#include <wx/statline.h>
#include <wx/notebook.h>
#include <wx/colordlg.h>

#include "Traindir3.h"
#include "OptionsDialog.h"
#include "MotivePower.h"
#include "StringBuilder.h"

extern  IntOption   http_server_port;
extern  BoolOption  http_server_enabled;
extern  StringOption user_name;

BEGIN_EVENT_TABLE(OptionsDialog, wxDialog)
	EVT_BUTTON(ID_CHOICE, OptionsDialog::OnColorChoice)
	EVT_BUTTON(ID_PATH1, OptionsDialog::OnPath1)
	EVT_BUTTON(ID_PATH2, OptionsDialog::OnPath2)
	EVT_BUTTON(ID_PATH3, OptionsDialog::OnPath3)
END_EVENT_TABLE()

SetTrackLengthDialog *gSetTrackLengthDialog;

static	struct opt {
    const wxChar        *id;
    const wxChar	*name;
    int		*optp;
} opts[NUM_OPTIONS - 10 + 1] = {
    { wxT("terse"), wxT("Short train info"), &terse_status },
//    { wxT("beepOnAlert"), wxT("Alert sound on"), &beep_on_alert },
//    { wxT("beepOnEnter"), wxT("Alert on train entering layout"), &beep_on_enter },
//    { wxT("showSpeeds"), wxT("View speed limits"), &show_speeds },
    { wxT("autoLink"), wxT("Automatically link signals"), &auto_link },
    { wxT("linkToLeft"), wxT("Link signals to left track"), &link_to_left },
//    { wxT("showGrid"), wxT("Show grid"), &show_grid },
//    { wxT("showBlocks"), wxT("View long blocks"), &show_blocks },
//    { wxT("showSeconds"), wxT("Show seconds on clock"), &show_seconds },
    { wxT("showTraditionals"), wxT("Traditional signals"), &signal_traditional },
    { wxT("hardCounters"), wxT("Strong performance checking"), &hard_counters },
    { wxT("showLinks"), wxT("Show linked objects in editor"), &show_links },
    { wxT("showScripts"), wxT("Show scripted objects in editor"), &show_scripts },
//    { wxT("showTrainIcons"), wxT("Show trains icons"), &show_icons },
    { wxT("showTooltip"), wxT("Show trains tooltip"), &show_tooltip },
    { wxT("randomDelays"), wxT("Enable random delays"), &random_delays },
    { wxT("synchronousSounds"), wxT("Wait while playing sounds"), &play_synchronously },
    { wxT("swapHeadTail"), wxT("Swap head and tail icons"), &swap_head_tail },
//    { wxT("drawTrainNames"), wxT("Show train names instead of icons"), &draw_train_names },
//    { wxT("noTrainNamesColors"), wxT("Don't show train names colors"), &no_train_names_colors },
//    { wxT("checkRealTime"), wxT("Check real-time train status"), &use_real_time },
    { 0 }
};

extern  Option  *intopt_list[];
extern  TDSkin	*curSkin;

OptionsDialog::OptionsDialog(wxWindow *parent)
: wxDialog(parent, 0, L("Preferences"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Preferences"))
{
	int		i;
	wxArrayString   strings;

        wxPanel         *page1 = new wxPanel(this, wxID_ANY);
	wxBoxSizer	*column3 = new wxBoxSizer(wxVERTICAL);
    
	    wxStaticText    *header = new wxStaticText(page1, 0, 
	        L("Check the desired options:"));
	    column3->Add(header, 0, wxALL, 10);

            wxBoxSizer	*column = new wxBoxSizer( wxVERTICAL );
	    wxBoxSizer	*column2 = new wxBoxSizer( wxVERTICAL );
	    for(i = 0; opts[i].name; ++i) {

	        m_boxes[i] = new wxCheckBox( page1, ID_CHECKBOX,
		    LV(opts[i].name), wxDefaultPosition, wxDefaultSize);

	        if(i > NUM_OPTIONS / 2) {
		    column2->Add(m_boxes[i], 0, wxLEFT, 10);
		    column2->AddSpacer(6);
	        } else {
		    column->Add(m_boxes[i], 0, wxLEFT, 10);
		    column->AddSpacer(6);
	        }
	    }
            int x;
            for (x = 0; intopt_list[x]; ++x, ++i) {
                if(i > sizeof(m_boxes) / sizeof(m_boxes[0])) // safety check - should be impossible
                    continue;
	        m_boxes[i] = new wxCheckBox( page1, ID_CHECKBOX,
		    LV(intopt_list[x]->_descr), wxDefaultPosition, wxDefaultSize);

	        if(i > 9) {
		    column2->Add(m_boxes[i], 0, wxLEFT, 10);
		    column2->AddSpacer(6);
	        } else {
		    column->Add(m_boxes[i], 0, wxLEFT, 10);
		    column->AddSpacer(6);
	        }
            }
    
	    wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );
	    row->Add(column, 1, wxGROW|wxRIGHT, 5);
	    row->Add(column2, 1, wxGROW|wxRIGHT, 5);
	    column3->Add(row, 0, wxALL, 10);

        page1->SetSizer(column3);

        wxPanel *page2 = CreatePage2();
        wxNotebook      *noteBook = new wxNotebook(this, wxID_ANY);

        wxPanel *page3 = CreatePage3();

        wxPanel *page4 = CreatePage4();     // Remote servers

        noteBook->AddPage(page1, L("Options"), true, 0);
        noteBook->AddPage(page2, L("Environment"), false, 1);
        noteBook->AddPage(page3, L("Skin"), false, 2);
        noteBook->AddPage(page4, L("Server"), false, 3);

        wxBoxSizer	*column4 = new wxBoxSizer( wxVERTICAL );
        column4->Add(noteBook);

	column4->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxGROW | wxALL, 10);

	SetSizer(column4);
	column4->Fit(this);
	column4->SetSizeHints(this);
}

OptionsDialog::~OptionsDialog()
{
}

wxBoxSizer *OptionsDialog::AddFileRow(wxPanel *page, Option& option, FileRow& out, int id)
{
        wxBoxSizer      *row = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText    *label = new wxStaticText(page, wxID_ANY, localize(option._descr.c_str()));
            //row->Add(label);
	    row->Add(label, 55, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 4);
        out._path = new wxTextCtrl(page, wxID_ANY);
            out._path->SetValue(option._sValue);
            //row->Add(out._path);
            row->Add(out._path, 75, wxRIGHT | wxGROW | wxTOP, 4);
        out._button = new wxButton(page, id, wxT("..."));
            //row->Add(out._button);
            row->Add(out._button, 20, wxRIGHT | wxTOP, 4);
        out._option = &option;
        return row;
}

extern  FileOption  alert_sound;
extern  FileOption  entry_sound;
extern  FileOption  searchPath;

wxPanel *OptionsDialog::CreatePage2()
{
        wxPanel    *page2 = new wxPanel(this, wxID_ANY);

        wxBoxSizer *column = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *row;
    
            row = AddFileRow(page2, alert_sound, _alert, ID_PATH1);
                column->Add(row);
            row = AddFileRow(page2, entry_sound, _enter, ID_PATH2);
                column->Add(row);
            row = AddFileRow(page2, searchPath, _search, ID_PATH3);
                column->Add(row);

        page2->SetSizer(column);
        return page2;
}


static	void	AddSkinRow(wxPanel *parent, wxBoxSizer *column, const Char *txt, SkinElementColor& rgb, int rgbV)
{
	Char	buff[32];
	wxSize	sz(40, 20);
	wxStaticText *separator = new wxStaticText(parent, 0, wxT("   "));

	wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );
	rgb.m_label = new wxStaticText(parent, 0, LV(txt));
	rgb.m_rgbSelector = new wxButton(parent, ID_CHOICE, L("Color..."), wxDefaultPosition, wxDefaultSize);
	sz.SetHeight(rgb.m_rgbSelector->GetSize().GetHeight());
	rgb.m_r = new wxTextCtrl(parent, 0, wxEmptyString, wxDefaultPosition, sz);
	rgb.m_g = new wxTextCtrl(parent, 0, wxEmptyString, wxDefaultPosition, sz);
	rgb.m_b = new wxTextCtrl(parent, 0, wxEmptyString, wxDefaultPosition, sz);

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


wxPanel *OptionsDialog::CreatePage3()
{
        wxPanel         *page = new wxPanel(this, wxID_ANY);

	wxBoxSizer	*column = new wxBoxSizer( wxVERTICAL );

        TDSkin *m_skin = curSkin;

	AddSkinRow(page, column, L("Background"), m_background, m_skin->background);
	AddSkinRow(page, column, L("Free Track"), m_freeTrack, m_skin->free_track);
	AddSkinRow(page, column, L("Reserved Track"), m_reservedTrack, m_skin->reserved_track);
	AddSkinRow(page, column, L("Reserved for Shunting"), m_reservedShunting, m_skin->reserved_shunting);
	AddSkinRow(page, column, L("Occupied"), m_occupiedTrack, m_skin->occupied_track);
	AddSkinRow(page, column, L("Reserved for Working"), m_workingTrack, m_skin->working_track);
	AddSkinRow(page, column, L("Switch Outline"), m_outline, m_skin->outline);
	AddSkinRow(page, column, L("Text"), m_text, m_skin->text);

	page->SetSizer(column);
        return page;
}

wxPanel *OptionsDialog::CreatePage4()
{
        wxPanel         *page = new wxPanel(this, wxID_ANY);

	wxBoxSizer	*column = new wxBoxSizer( wxVERTICAL );

	    wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );

            _httpServerEnabled = new wxCheckBox( page, ID_CHECKBOX,
                L("Enable HTTP server"), wxDefaultPosition, wxDefaultSize);

            row->Add(_httpServerEnabled, 1, wxGROW | wxLEFT | wxRIGHT, 10);

	column->Add(row, 1, wxGROW | wxLEFT | wxRIGHT, 10);

	    row = new wxBoxSizer( wxHORIZONTAL );

	    wxStaticText *label = new wxStaticText(page, 0, L("HTTP Server Port"));
	    _httpPort = new wxTextCtrl(page, 0, wxEmptyString, wxDefaultPosition);

            row->Add(label, 40);
            row->Add(_httpPort, 60);

	column->Add(row, 1, wxGROW | wxLEFT | wxRIGHT, 10);

	    row = new wxBoxSizer( wxHORIZONTAL );

	    label = new wxStaticText(page, 0, L("User name"));
            _userName = new wxTextCtrl(page, 0, wxEmptyString, wxDefaultPosition);

            row->Add(label, 40);
            row->Add(_userName, 60);

	column->Add(row, 1, wxGROW | wxLEFT | wxRIGHT, 10);

	page->SetSizer(column);
        return page;
}

void	OptionsDialog::OnColorChoice(wxCommandEvent& event)
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

void    OptionsDialog::EnterPath(FileRow &row)
{
	wxFileDialog *fileDialog = new wxFileDialog(NULL, _("Open a file"), wxT(""), wxT(""),
		wxT("Sound File (*.wav)|*.wav|All Files (*.*)|*.*"),
		wxOPEN | wxFILE_MUST_EXIST);
        fileDialog->SetPath(row._option->_sValue);
	if(fileDialog->ShowModal() != wxID_OK)
	    return;
        row._path->SetValue(fileDialog->GetPath());
}

void    OptionsDialog::EnterDirPath(FileRow &row)
{
	wxDirDialog *fileDialog = new wxDirDialog(NULL, L("Select Directory"), wxT(""),
		wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        fileDialog->SetPath(row._option->_sValue);
	if(fileDialog->ShowModal() != wxID_OK)
	    return;
        row._path->SetValue(fileDialog->GetPath());
}

void	OptionsDialog::OnPath1(wxCommandEvent& event)
{
        EnterPath(_alert);
}

void	OptionsDialog::OnPath2(wxCommandEvent& event)
{
        EnterPath(_enter);
}

void	OptionsDialog::OnPath3(wxCommandEvent& event)
{
        EnterDirPath(_search);
}

int	OptionsDialog::ShowModal()
{
	int	    i;
	int	    res;

	for(i = 0; opts[i].name; ++i) {
	    m_boxes[i]->SetValue(*opts[i].optp != 0);
	}
        for(int x = 0; intopt_list[x]; ++x, ++i)
	    m_boxes[i]->SetValue(intopt_list[x]->_iValue != 0);

        _alert._path->SetValue(alert_sound._sValue);
        _enter._path->SetValue(entry_sound._sValue);
        _search._path->SetValue(searchPath._sValue);

        _httpServerEnabled->SetValue(http_server_enabled._iValue != 0);
        _httpPort->SetValue(http_server_port._sValue);
        _userName->SetValue(user_name._sValue);

	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
	if(res == wxID_OK) {
	    for(i = 0; opts[i].name; ++i) {
		*opts[i].optp = m_boxes[i]->GetValue() ? 1 : 0;
	    }
            for(int x = 0; intopt_list[x]; ++x, ++i)
                intopt_list[x]->_iValue = m_boxes[i]->GetValue() ? 1 : 0;
            alert_sound.Set(_alert._path->GetValue());
            entry_sound.Set(_enter._path->GetValue());
            searchPath.Set(_search._path->GetValue());

            if(curSkin == defaultSkin) {
                TDSkin *tmpSkin = new TDSkin();
	        tmpSkin->name = wxStrdup(wxT("Skin1"));
	        tmpSkin->next = skin_list;
	        skin_list = tmpSkin;
	        curSkin = tmpSkin;
            }
            TDSkin *m_skin = curSkin;
	    m_skin->background = RetrieveValue(this->m_background);
	    m_skin->free_track = RetrieveValue(this->m_freeTrack);
	    m_skin->occupied_track = RetrieveValue(this->m_occupiedTrack);
	    m_skin->outline = RetrieveValue(this->m_outline);
	    m_skin->reserved_shunting = RetrieveValue(this->m_reservedShunting);
	    m_skin->reserved_track = RetrieveValue(this->m_reservedTrack);
	    m_skin->working_track = RetrieveValue(this->m_workingTrack);
	    m_skin->text = RetrieveValue(this->m_text);

            http_server_enabled.Set(_httpServerEnabled->GetValue());
            http_server_port.Set(_httpPort->GetValue());
            user_name.Set(_userName->GetValue());
	}
	return res;
}



//
//
//

SelectPowerDialog::SelectPowerDialog(wxWindow *parent)
: wxDialog(parent, 0, wxT("Select Motive Power"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L(" Motive Power "))
{
	wxArrayString   strings;
	wxBoxSizer	    *column = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer	    *row = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText	    *header = new wxStaticText( this, 0, L("Motive &Power"));
        m_power = new wxComboBox(this, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT, 4);
	row->Add(m_power, 65, wxGROW | wxALIGN_RIGHT | wxLEFT, 6);

	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);

///	row = new wxBoxSizer(wxHORIZONTAL);

///     header = new wxStaticText(this, 0, L("Track &Gauge: "));
///	m_gauge = new wxTextCtrl(this, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

///	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT, 4);
///	row->Add(m_gauge, 65, wxGROW | wxALIGN_RIGHT | wxLEFT, 6);

///	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);
//	wxStaticLine *line = new wxStaticLine( this );

//	column->Add(line);

	column->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxGROW | wxALL, 10);

	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

SelectPowerDialog::~SelectPowerDialog()
{
}

int	SelectPowerDialog::ShowModal()
{
//        Char    buff[64];

///        wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%d"), editor_gauge._iValue);
///        m_gauge->SetValue(buff);
        m_power->Clear();

        for (int i = 0; i < gMotivePowerCache.Length(); ++i) {
            const Char *p = gMotivePowerCache[i];
            m_power->AppendString(p);
        }
        if (gEditorMotivePower)
            m_power->SetValue(gEditorMotivePower);

	Center();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	m_power->SetFocus();
	int res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
        if(res != wxID_OK)
	    return wxID_CANCEL;

        power_select(m_power->GetValue().c_str());
///        editor_gauge.Set(m_gauge->GetValue().c_str());
	return wxID_OK;
}



//
//
//

SetTrackLengthDialog::SetTrackLengthDialog(wxWindow *parent)
: wxDialog(parent, 0, wxT("Select Motive Power"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L(" Motive Power "))
{
	wxArrayString   strings;
	wxBoxSizer	    *column = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer	    *row = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText	    *header = new wxStaticText( this, 0, L("Motive &Power"));
        m_length = new wxTextCtrl(this, wxID_ANY);

	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT, 4);
	row->Add(m_length, 65, wxGROW | wxALIGN_RIGHT | wxLEFT, 6);
	column->Add(row, 1, wxGROW | wxTOP | wxRIGHT | wxLEFT, 10);

	column->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxGROW | wxALL, 10);
	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

SetTrackLengthDialog::~SetTrackLengthDialog()
{
}

int	SetTrackLengthDialog::ShowModal()
{
	Center();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	m_length->SetFocus();
	int res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
        if(res != wxID_OK)
	    return -1;

        int len = wxAtoi(m_length->GetValue().c_str());
	return len;
}



void    rgbToString(Char *dest, int rgbV)
{
	wxSprintf(dest, wxT("%d,%d,%d"), (rgbV >> 16) & 0xFF, (rgbV >> 8) & 0xFF, rgbV & 0xFF);
}

void    OptionToYaml(StringBuilder &out, Option &o, int level)
{
        Char    buff[1024];

        wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  - id: %s\n"), level * 2, o._name.c_str());
        out.Append(buff);
        wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*    value: %s\n"), level * 2, o._iValue ? wxT("on") : wxT("off"));
        out.Append(buff);
        wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*    name: %s\n"), level * 2, LV(o._descr.c_str()));
        out.Append(buff);
}

void    OptionsToYaml(StringBuilder& out, int level)
{
        Char    buff[1024];
        Char colorBuff[64];
        int i;
        Option *o;

        wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*- options:\n"), level * 2);
        out.Append(buff);
        for(i = 0; opts[i].name; ++i) {
            wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  - id: %s\n"), level * 2, opts[i].id);
            out.Append(buff);
            wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*    value: %s\n"), level * 2, opts[i].optp[0] ? wxT("on") : wxT("off"));
            out.Append(buff);
            wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*    name: %s\n"), level * 2, opts[i].name);
            out.Append(buff);
        }
        for(i = 0; intopt_list[i]->_name; ++i) {
            o = intopt_list[i];
            wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  - id: %s\n"), level * 2, o->_name.c_str());
            out.Append(buff);
            wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*    value: %s\n"), level * 2, o->_iValue ? wxT("on") : wxT("off"));
            out.Append(buff);
            wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*    name: %s\n"), level * 2, LV(o->_descr.c_str()));
            out.Append(buff);
        }
        OptionToYaml(out, trace_script, level);
        wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*- paths:\n"), level * 2);
        out.Append(buff);
        wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  entrySound: %s\n"), level * 2, entry_sound._sValue.c_str());
        out.Append(buff);
        wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  alertSound: %s\n"), level * 2, alert_sound._sValue.c_str());
        out.Append(buff);
        wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  search: %s\n"), level * 2, searchPath._sValue.c_str());
        out.Append(buff);
        wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*- skins:\n"), level * 2);
        out.Append(buff);

        TDSkin *m_skin = curSkin;
        rgbToString(colorBuff, m_skin->background);    wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  background: %s\n"), level * 2, colorBuff);
        out.Append(buff);
        rgbToString(colorBuff, m_skin->free_track);    wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  freeTrack: %s\n"), level * 2, colorBuff);
        out.Append(buff);
        rgbToString(colorBuff, m_skin->reserved_track);    wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  reservedTrack: %s\n"), level * 2, colorBuff);
        out.Append(buff);
        rgbToString(colorBuff, m_skin->reserved_shunting);    wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  reservedShunting: %s\n"), level * 2, colorBuff);
        out.Append(buff);
        rgbToString(colorBuff, m_skin->occupied_track);    wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  occupied: %s\n"), level * 2, colorBuff);
        out.Append(buff);
        rgbToString(colorBuff, m_skin->working_track);    wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  working: %s\n"), level * 2, colorBuff);
        out.Append(buff);
        rgbToString(colorBuff, m_skin->outline);    wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  switchOutline: %s\n"), level * 2, colorBuff);
        out.Append(buff);
        rgbToString(colorBuff, m_skin->text);    wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%*  text: %s\n"), level * 2, colorBuff);
        out.Append(buff);

}

void    set_option(Char *id, const Char *sval, int val)
{
        int i;

        for(i = 0; intopt_list[i]; ++i) {
            Option *opt = intopt_list[i];
            if(opt->_name.Cmp(id) == 0) {
                opt->_iValue = val;
                opt->Set(val);
                return;
            }
        }
        if(!wxStrcmp(id, trace_script._name.c_str())) {
            trace_script._iValue = val;
            return;
        }
        if(!wxStrcmp(id, wxT("free_track"))) {
            curSkin->free_track = val;
            return;
        }
        if(!wxStrcmp(id, wxT("reserved_track"))) {
            curSkin->reserved_track = val;
            return;
        }
        if(!wxStrcmp(id, wxT("reserved_shunting"))) {
            curSkin->reserved_shunting = val;
            return;
        }
        if(!wxStrcmp(id, wxT("occupied_track"))) {
            curSkin->occupied_track = val;
            return;
        }
        if(!wxStrcmp(id, wxT("working_track"))) {
            curSkin->working_track = val;
            return;
        }
        if(!wxStrcmp(id, wxT("background"))) {
            curSkin->background = val;
            return;
        }
        if(!wxStrcmp(id, wxT("outline"))) {
            curSkin->outline = val;
            return;
        }
        if(!wxStrcmp(id, wxT("text"))) {
            curSkin->text = val;
            return;
        }
        if(!wxStrcmp(id, alert_sound._name)) {
            alert_sound._sValue = sval;
            return;
        }
        if(!wxStrcmp(id, entry_sound._name)) {
            entry_sound._sValue = sval;
            return;
        }
        if(!wxStrcmp(id, searchPath._name)) {
            searchPath._sValue = sval;
            return;
        }
}
