/*	StationInfoDialog.cpp - Created by Giampiero Caprino

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
#include "StationInfoDialog.h"
#include "MainFrm.h"

extern	int	set_track_properties(Track *t, const wxChar *len, const wxChar *station, const wxChar *speed,
			     const wxChar *distance, const wxChar *wlink, const wxChar *elink,const wxChar *num);

extern	Track	**get_station_list();
static	int	station_idx;
static	int	sort_station;
extern	int	platform_schedule;

static	const wxChar	*en_titles[] = { wxT("Train"), wxT("Arrival"), wxT("From"), wxT("Departure"), wxT("To"), wxT("Plat."), wxT("Runs on"), wxT("Notes"), 0 };
static	const wxChar	*titles[9];

BEGIN_EVENT_TABLE(StationInfoDialog, wxDialog)
	EVT_CHECKBOX(ID_CHECKBOX, StationInfoDialog::OnCheckbox)
	EVT_CHOICE(ID_CHOICE, StationInfoDialog::OnChoice)
	EVT_BUTTON(ID_PRINT, StationInfoDialog::OnPrint)
END_EVENT_TABLE()

StationInfoDialog::StationInfoDialog(wxWindow *parent)
: wxDialog(parent, 0, L("Station schedule"), wxDefaultPosition, wxDefaultSize,
	   wxDEFAULT_DIALOG_STYLE, L("Station schedule"))
{
	int		i;
	wxArrayString   strings;
	wxBoxSizer	*column = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer	*row = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText	*txt = new wxStaticText(this, wxID_ANY, L("&Schedule for station :"));
	row->Add(txt, 25, wxALL, 4);

	m_stations = new wxChoice(this, ID_CHOICE,
				wxDefaultPosition, wxDefaultSize, strings);
	row->Add(m_stations, 50, wxGROW|wxALL, 4);

	m_check = new wxCheckBox(this, ID_CHECKBOX, L("&Ignore platform number"));
	m_check->SetValue(1);

	row->Add(m_check, 25, wxALL, 4);

	column->Add(row, 10, wxALL, 4);

	m_stops = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
	    wxLC_REPORT);
	wxListItem  col;

	if(!titles[0])
	    localizeArray(titles, en_titles);

	//  Insert columns

	for(i = 0; titles[i]; ++i) {
	    col.SetText(titles[i]);
//	    col.SetImage(-1);
	    m_stops->InsertColumn(i, col);
	    m_stops->SetColumnWidth(i, 80);
	}

	column->Add(m_stops, 100, wxGROW | wxALL, 4);

	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxButton(this, ID_PRINT, L("&Print")), 0, wxALL, 4);
	row->Add(new wxButton(this, wxID_OK, L("&Close")), 0, wxALL, 4);

	column->Add(row, 0, wxALIGN_RIGHT | wxGROW | wxALL, 6);

	SetSizer(column);
	column->Fit(this);
	column->SetSizeHints(this);
}

StationInfoDialog::~StationInfoDialog()
{
	freeLocalizedArray(titles);
}

bool	StationInfoDialog::LoadStationList(const wxChar *station, bool ignorePlatforms)
{
	int	i;
	wxChar	*p;
	wxChar	buff[256];
	wxString    str;

        stations.Build(ignorePlatforms);
        if(!stations._nItems) {
	    wxMessageBox(L("No stations found."));
	    return false;
	}
	m_stations->Clear();
	station_idx = 0;
        for(i = 0; stations._items[i]; ++i) {
	    if(station && sameStation(stations._items[i]->station, station))
		station_idx = i;
	    wxStrcpy(buff, stations._items[i]->station);
	    if(ignorePlatforms && (p = wxStrrchr(buff, '@')))
		*p = 0;
	    m_stations->Append(buff);
	}

	m_stations->SetSelection(station_idx);
	return true;
}

void	StationInfoDialog::OnCheckbox(wxCommandEvent& event)
{
	int	i;

	station_idx = m_stations->GetSelection();
	wxString origStation = stations._items[station_idx]->station;

	platform_schedule = !m_check->GetValue();
	LoadStationList(origStation, !platform_schedule);
        for(i = 0; stations._items[i]; ++i)
            if(sameStation(origStation.c_str(), stations._items[i]->station)) {
		m_stations->SetSelection(i);
		break;
	    }
	OnChoice(event);
}

void	StationInfoDialog::OnChoice(wxCommandEvent& event)
{
	station_idx = m_stations->GetSelection();
        _sched._ignorePlatforms = this->stations._ignorePlatforms;
        _sched.Build(stations._items[station_idx]->station);
	FillStops();
}

void	StationInfoDialog::FillStops()
{
	int	i, r;
	struct station_sched *sc;
	wxChar	*p;
	wxChar	buff[80];

	m_stops->DeleteAllItems();
	i = 0;
	for(sc = _sched._items; sc; sc = sc->next) {
	    long    id = m_stops->InsertItem(i, sc->tr->name);
	    m_stops->SetItem(id, 1, sc->arrival != -1 ? format_time(sc->arrival) : wxT(""));
	    m_stops->SetItem(id, 2, sc->tr->entrance);
	    m_stops->SetItem(id, 3, sc->departure != -1 ? format_time(sc->departure) : wxT(""));
	    m_stops->SetItem(id, 4, sc->tr->exit);
	    buff[0] = 0;
	    if(sc->stopname && (p = wxStrchr(sc->stopname, '@')))
		wxStrcpy(buff, p + 1);
	    m_stops->SetItem(id, 5, buff);
	    int x = 0;
	    if(sc->tr->days) {
		for(r = 0; r < 7; ++r)
		    if(sc->tr->days & (1 << r))
			buff[x++] = r + '1';
	    }
	    buff[x] = 0;
	    m_stops->SetItem(id, 6, buff);
	    ++i;
	}
}

void	StationInfoDialog::OnPrint(wxCommandEvent& event)
{
	station_idx = m_stations->GetSelection();
	ShowStationSchedule(stations._items[station_idx]->station, false);
	
}

int	StationInfoDialog::ShowModal(const wxChar *station)
{
	int	    res;

	if(!LoadStationList(station, !platform_schedule))
	    return wxID_CANCEL;
	m_check->SetValue(!platform_schedule);
        _sched.Build(stations._items[station_idx]->station);
	FillStops();

	Centre();
	bool oldIgnore = traindir->m_ignoreTimer;
	traindir->m_ignoreTimer = true;
	m_stations->SetFocus();
	res = wxDialog::ShowModal();
	traindir->m_ignoreTimer = oldIgnore;

	return res;
}
