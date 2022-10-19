/*	TrackDialog.cpp - Created by Giampiero Caprino

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
#include <wx/fontdlg.h>

#include "Traindir3.h"
#include "TrackDialog.h"
#include "FontManager.h"
#include "MotivePower.h"

extern	void	ShowTrackScriptDialog(Track *trk);

extern	int	gFontSizeSmall, gFontSizeBig;
extern void send_msg(wxChar *msg,int send_len);// 发送限速

long jiange[100];

BEGIN_EVENT_TABLE(TrackDialog, wxDialog)
	EVT_BUTTON(ID_SCRIPT, TrackDialog::OnScript)
END_EVENT_TABLE()

static  wxStaticText *lastLabel;

static	wxTextCtrl  *AddTextLine(wxDialog *dialog, wxBoxSizer *column, const wxChar *txt)
{
	wxBoxSizer	*row = new wxBoxSizer( wxHORIZONTAL );
	wxStaticText    *header = new wxStaticText( dialog, 0, LV(txt));
	wxTextCtrl	*txtctrl = new wxTextCtrl( dialog, 0, wxEmptyString, wxDefaultPosition, wxDefaultSize);

	row->Add(header, 35, wxALIGN_LEFT | wxRIGHT | wxTOP, 4);
	row->Add(txtctrl, 65, wxGROW | wxALIGN_RIGHT | wxTOP, 4);

	column->Add(row, 1, wxGROW | wxLEFT | wxRIGHT, 10);

        lastLabel = header;

	return txtctrl;
}

TrackDialog::TrackDialog(wxWindow *parent)
: wxDialog(parent, 0, L("Track Properties"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Track Properties"))
{
	wxBoxSizer	    *column = new wxBoxSizer( wxVERTICAL );

	column->AddSpacer(10);
	m_length = AddTextLine( this, column, L("Track Length (m) :"));
	m_name = AddTextLine( this, column, L("Station name :"));
        m_stationLabel = lastLabel;
	m_km = AddTextLine(this, column, L("Km. :"));
	m_speeds = AddTextLine(this, column, L("Speed(sXXXX) :"));
        m_speedLabel = lastLabel;
	m_link_east = AddTextLine(this, column, L("Linked to east :"));
	m_link_west = AddTextLine(this, column, L("Linked to west :"));
	m_power = AddTextLine(this, column, L("Km Flag :"));
	m_line_num = AddTextLine(this, column, L("Linenum :"));
	m_invisible = new wxCheckBox(this, wxID_ANY, L("Hidden"), wxDefaultPosition, wxDefaultSize);
	column->Add(m_invisible, 0, wxLEFT|wxRIGHT|wxTOP, 10);
	m_dontstop = new wxCheckBox(this, wxID_ANY, L("Don't stop if shunting"), wxDefaultPosition, wxDefaultSize);
	column->Add(m_dontstop, 0, wxLEFT|wxRIGHT|wxTOP, 10);

///	column->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxGROW | wxALL, 10);

	wxBoxSizer *row = new wxBoxSizer(wxHORIZONTAL);
	m_scripts = new wxButton(this, ID_SCRIPT, L("&Script..."));
	row->Add(m_scripts, 0, wxALL, 4);
	row->Add(new wxButton(this, wxID_CANCEL, L("&Cancel")), 0, wxALL, 4);
	wxButton *buttonOk = new wxButton(this, wxID_OK, L("&Close"));
	buttonOk->SetDefault();
	row->Add(buttonOk, 0, wxALL, 4);
	column->Add(row, 0, wxALIGN_RIGHT | wxGROW | wxALL, 6);

	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

TrackDialog::~TrackDialog()
{
}

void	TrackDialog::OnScript(wxCommandEvent& event)
{
        ShowTrackScriptDialog(m_track);
}

int	TrackDialog::ShowModal(Track *trk)
{
	wxChar	    buff[256];
	wxChar      msg [256];

	wxChar	    *p;
	int	    i;
	wxString    str;
// ----------for speed test only by cuidl 20191108
//	short SpeedTest,SpeedTest1;
// end of speedtest
//	FontEntry   *f = 0;

//	if(trk->_fontIndex) {
//	    f = fonts.FindFont(trk->_fontIndex);
//	}
//	if(f) {
//	    m_font = wxFont(f->_size, f->_family, f->_style, f->_weight);
//	    m_textColor.Set((f->_color >> 16) & 0xff, (f->_color >> 8) & 0xff, f->_color & 0xff);
//	} else {
//	    m_font = wxFont(trk->direction ? gFontSizeSmall : gFontSizeBig, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL);
//	    m_textColor = *wxBLACK;
//	}
	m_track = trk;
	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%d"), trk->length);
	m_length->SetValue(buff);
	m_name->SetValue(trk->station);
	buff[0] = 0;
	if(trk->km) {
	    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%d.%d,"), trk->km / 1000, trk->km % 1000);
	}
    m_km->SetValue(buff);

	/*if(trk->line_num) {
	    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%d "), trk->line_num);
	}*/

	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%d "), trk->line_num);
    m_line_num->SetValue(buff);  //20201209ypx

	p = buff;
	*p = 0;
	for(i = 0; i < NTTYPES; ++i) {
	    wxSprintf(p, wxT("%d/"), trk->speed[i]);
	    p += wxStrlen(p);
	}
	if(p > buff)		    /* remove last '/' */
	    p[-1] = 0;
	m_speeds->SetValue(buff);
	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%d,%d"), trk->elinkx, trk->elinky);
	m_link_east->SetValue(buff);
	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%d,%d"), trk->wlinkx, trk->wlinky);
	m_link_west->SetValue(buff);
	m_invisible->SetValue(trk->invisible);
        m_dontstop->SetValue((trk->flags & TFLG_DONTSTOPSHUNTERS) ? 1 : 0); 
        m_link_east->Enable(true);
    
        if(trk->power)
            m_power->SetValue(trk->power);
///        if(trk->gauge) {
///            wxSprintf(buff, wxT("%g"), trk->gauge);
///            m_gauge->SetValue(buff);
///        }

        m_stationLabel->SetLabel(L("Station name :"));
        m_speedLabel->SetLabel(L("Speed(s) :"));
	bool	enable = true;
	if(trk->type == TEXT) {
	    m_length->Enable(false);
	    m_km->Enable(false);
	    m_speeds->Enable(false);
            m_power->Enable(false);
            m_dontstop->Enable(false);
            m_invisible->Enable(true);
            if(trk->elinkx || trk->elinky)
                m_link_east->Enable(false);
            else
                m_link_west->Enable(false);
//	    m_scripts->SetLabel(L("&Font..."));
//	} else {
//	    m_scripts->SetLabel(L("&Script..."));
        } else if(trk->type == SWITCH) {
            m_stationLabel->SetLabel(L("Switch name :"));
            m_speedLabel->SetLabel(L("Branch Speed(s) :"));
	    m_length->Enable(false);
	    m_km->Enable(false);
            m_dontstop->Enable(false);
            m_invisible->Enable(false);
            m_link_east->Enable(false);
        } else {
	    m_length->Enable(true);
	    m_km->Enable(true);
	    m_speeds->Enable(true);
        m_line_num->Enable(true);
            m_power->Enable(true);
            m_dontstop->Enable(true);
            m_invisible->Enable(true);
            m_link_east->Enable(true);
        }
///        m_gauge->Enable(enable);

	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	m_length->SetFocus();
	int res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;
	if(res != wxID_OK)
	    return wxID_CANCEL;

	str = m_length->GetValue();
	if(str.c_str()[0]) {
	    trk->length = wxAtoi(str.c_str());
	    if(trk->length < 0)
		trk->length = 0;
	}
	str = m_name->GetValue();
	if(trk->station)
	    free(trk->station);
	trk->station = 0;
	trk->isstation = 0;
	if(str.length() > 0) {
	    trk->station = wxStrdup(str.c_str());
	    trk->isstation = 1;
	}
	str = m_km->GetValue();
	trk->km = 0;
	if(str.c_str()[0]) {
	    parse_km(trk, str.c_str());
	}

    str = m_line_num->GetValue();     //20201209ypx  获取线路编号
	trk->line_num = 0;
	if(str.c_str()[0]) {
	  trk->line_num =  wxAtoi(str.c_str());
	}


	str = m_speeds->GetValue();
	memset(trk->speed, 0, sizeof(trk->speed));
	if(str.length() > 0){
	    wxStrcpy(buff, str.c_str());
	    trk->speed[0] = wxStrtol(buff, &p, 10);  

		//trk->speed[0]=60000/jiange[0];//20191121

	    for(i = 1; i < NTTYPES && *p == '/'; ++i) {
		trk->speed[i] = (short)wxStrtol(p + 1, &p, 10);
        
	    }
		 wxSnprintf(msg, sizeof(msg)/sizeof(Char), "%d,%d,%s\n", trk->x, trk->y, buff);// 20200426by YPX 
       send_msg(buff,wxStrlen(buff));
	}
	trk->invisible = m_invisible->GetValue() ? 1 : 0;
        if(m_dontstop->GetValue() != 0)
            trk->flags |= TFLG_DONTSTOPSHUNTERS;
        else
	    trk->flags &= ~TFLG_DONTSTOPSHUNTERS;
	str = m_link_east->GetValue();
	trk->elinkx = trk->elinky = 0;
	if(str.length() > 0) {
	    wxStrcpy(buff, str.c_str());
	    trk->elinkx = wxStrtol(buff, &p, 10);
	    if(*p != ',')
		trk->elinkx = 0;
	    else
		trk->elinky = wxStrtol(p + 1, &p, 10);
	}
	str = m_link_west->GetValue();
	trk->wlinkx = trk->wlinky = 0;
	if(str.length() > 0) {
	    wxStrcpy(buff, str.c_str());
	    trk->wlinkx = wxStrtol(buff, &p, 10);
	    if(*p != ',')
		trk->wlinkx = 0;
	    else
		trk->wlinky = wxStrtol(p + 1, &p, 10);
	}
        str = m_power->GetValue();
        if(str.length() > 0) {
            trk->power = power_parse(str.c_str());   //YPX修改
			/*trk->km_post= power_parse(str.c_str());*/
        } else {
            trk->power = 0;
        }
///        str = m_gauge->GetValue();
///        if(str.length() > 0) {
///            trk->gauge = wxAtof(str.c_str());
///        }
//	i = fonts.FindFont(m_font.GetPointSize(), m_font.GetFamil`y(), m_font.GetStyle(), m_font.GetWeight(),
//	    (m_textColor.Red()<<16) + (m_textColor.Green()<<8) + m_textColor.Blue());
//	if(i == 0 || i == 1) {
//	    trk->_fontIndex = 0;
//	    trk->direction = (trkdir)i;
//	} else if(i > 1) {
//	    trk->_fontIndex = i;
//	    trk->direction = (trkdir)0;
//	} else {
//	    trk->_fontIndex = fonts.AddFont(m_font.GetPointSize(), m_font.GetFamily(), m_font.GetStyle(), m_font.GetWeight(),
//		(m_textColor.Red()<<16) + (m_textColor.Green()<<8) + m_textColor.Blue());
//	    trk->direction = (trkdir)0;
//	}
	return wxID_OK;
}
