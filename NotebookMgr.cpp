/*	NotebookMgr.cpp - Created by Giampiero Caprino

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

#include "NotebookMgr.h"
#include "ReportBase.h"
#include "TimeTblView.h"
#include "ItineraryView.h"
#include "AlertList.h"
#include "TrainInfoList.h"
#include "MainFrm.h"
#include "Traindir3.h"

BEGIN_EVENT_TABLE(NotebookManager, wxNotebook)
	EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK_TOP, NotebookManager::OnPageChanged)
	EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK_LEFT, NotebookManager::OnPageChanged)
	EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK_RIGHT, NotebookManager::OnPageChanged)
END_EVENT_TABLE()

NotebookManager::NotebookManager(wxWindow *parent, const wxChar *name, int id)
    : wxNotebook(parent, id, wxDefaultPosition, wxDefaultSize, wxNB_BOTTOM),
    m_name(name)
{
}

NotebookManager::~NotebookManager()
{
}

void	NotebookManager::OnPageChanged(wxNotebookEvent& event)
{
	event.Skip();
}

int	NotebookManager::FindPage(wxWindow *w)
{
	size_t	    i;

	for(i = 0; i < GetPageCount(); ++i) {
	    const wxWindow *pPage = GetPage(i);
	    if(pPage == w) {
		return i;
	    }
	}
	return -1;
}

int	NotebookManager::FindPage(const wxString& title)
{
	size_t	    i;

	for(i = 0; i < GetPageCount(); ++i) {
	    const wxString& name = GetPageText(i);
	    if(title == name) {
		return i;
	    }
	}
	return -1;
}

int	NotebookManager::FindPageType(const wxChar *name)
{
	size_t	    i;

	for(i = 0; i < GetPageCount(); ++i) {
	    wxWindow *pPage = GetPage(i);
	    if(pPage->GetName() == wxT("traininfo")) {
		return i;
	    }
	}
	return -1;
}

void	NotebookManager::RemovePage(wxWindow *pView)
{
	wxWindow    *pChild;
	size_t	    i;

	for(i = 0; i < GetPageCount(); ++i) {
	    pChild = GetPage(i);
	    if(pChild == pView) {
		wxNotebook::RemovePage(i);
		break;
	    }
	}
}

void	NotebookManager::SaveState(const wxString& header, TConfig& state)
{
	unsigned int	i;
	int		pageCnt = 0;
	wxString	ttheader;

	ttheader.Printf(wxT("%s-%s"), header.c_str(), m_name.c_str());
	state.StartSection(ttheader.c_str());
	state.PutInt(wxT("nPages"), GetPageCount());

	for(i = 0; i < GetPageCount(); ++i) {
	    wxWindow	*pPage = GetPage(i);
	    wxString	type = pPage->GetName();

	    ttheader.Printf(wxT("%s-%s-%d"), header.c_str(), m_name.c_str(), pageCnt++);

	    if(type == wxT("timetable") || type == wxT("itinerary") || 
	       type == wxT("alerts") || type == wxT("traininfo")) {
		ReportBase *pReport = (ReportBase *)pPage;
		pReport->SaveState(ttheader, state);
	    }
	}
}

void	NotebookManager::LoadState(const wxString& header, TConfig& state)
{
	wxString    ttheader;
	wxString    tabhdr;
	int	    nTabs;
	int	    nt;

	ttheader.Printf(wxT("%s-%s"), header.c_str(), m_name.c_str());
	state.PushSection(ttheader.c_str());
	state.GetInt(wxT("nPages"), nTabs);

	for(nt = 0; nt < nTabs; ++nt) {
	    tabhdr.Printf(wxT("%s-%s-%d"), header.c_str(), m_name.c_str(), nt);
	    wxString	name;
	    ReportBase	*pReport;

	    if(!state.PushSection(tabhdr.c_str()))
		continue;
	    state.GetString(wxT("type"), name);
	    if(name == wxT("timetable")) {
//		state.GetString(wxT("name"), name);
		name = wxT("Schedule");
//		TimeTableView *pTimeTable = traindir->m_frame->m_timeTableManager.GetNewTimeTableView(this, name);
//		pReport = pTimeTable;
		pReport = traindir->m_frame->m_timeTable;
//	    } else if(name == wxT("itinerary")) {
//		state.GetString(wxT("name"), name);
//		ItineraryView *pItinerary = traindir->m_frame->m_itineraryView;
//		pReport = pItinerary;
	    } else if(name == wxT("alerts")) {
//		state.GetString(wxT("name"), name);
		name = wxT("Alerts");
		pReport = traindir->m_frame->m_alertList;
	    } else if(name == wxT("traininfo")) {
//		state.GetString(wxT("name"), name);
		name = wxT("Train Info");
		pReport = traindir->m_frame->m_trainInfo;
	    } else
		continue;
	    if(FindPage(pReport) < 0)
		AddPage(pReport, LV(name), false, -1);
	    wxString	name1;
	    name1.Printf(wxT("%s-%s"), ttheader.c_str(), name.c_str());
	    pReport->LoadState(tabhdr, state);
	    state.PopSection();
	}
	state.PopSection();
}
