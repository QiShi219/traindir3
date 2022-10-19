/*	AlertList.cpp - Created by Giampiero Caprino

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
#include "wx/listctrl.h"
#include "wx/image.h"
#include "wx/ffile.h"
#include "AlertList.h"
#include "MainFrm.h"
#include "Traindir3.h"

extern	pxmap	*pixmaps;
extern	int	npixmaps;

static	const Char	*en_titles[] = { wxT("Alerts"), 0 };
static	const Char	*titles[sizeof(en_titles)/sizeof(wxChar *)];
static	int	schedule_widths[] = { 500, 0 };

BEGIN_EVENT_TABLE(AlertList, wxListCtrl)
	EVT_CONTEXT_MENU(AlertList::OnContextMenu)
	EVT_MENU(MENU_ALERT_CLEAR, AlertList::OnClear)
	EVT_MENU(MENU_ALERT_SAVE, AlertList::OnSave)
	EVT_MENU(MENU_ALERT_LAST_ON_TOP, AlertList::OnLastOnTop)
END_EVENT_TABLE()

class AlertListListener : public EventListener
{
public:
        AlertListListener(AlertList *list) { _list = list; }
        ~AlertListListener() { };

        void    OnEvent(void *list);
        AlertList   *_list;
};

void    AlertListListener::OnEvent(void *dummy)
{
        if(alerts._firstItem == 0) {
            _list->DeleteAllItems();
        } else {
            int nItems = _list->GetItemCount();
            int x = 0;
            AlertLine *line;
            for(line = alerts._firstItem; line && x < nItems; line = line->_next)
                ++x;
            while(line) {
                _list->AddLine(line->_text);
                line = line->_next;
            }
        }
}

AlertList::AlertList(wxWindow *parent, const wxString& name)
: ReportBase(parent, name)
{
	SetName(wxT("alerts"));
	if(!titles[0])
	    localizeArray(titles, en_titles);
	DefineColumns(titles, schedule_widths);
        _listener = new AlertListListener(this);
        alerts.AddListener(_listener);
}

AlertList::~AlertList()
{
	freeLocalizedArray(titles);
}

//
//
//

void	AlertList::LoadState(const wxString& header, TConfig& state)
{
	ReportBase::LoadState(header, state);
        state.Get(alert_last_on_top);
}

//
//
//

void	AlertList::SaveState(const wxString& header, TConfig& state)
{
	ReportBase::SaveState(header, state);
        state.Put(alert_last_on_top);
}

//
//
//

void	AlertList::AddLine(const wxChar *txt)
{
        if(alert_last_on_top._iValue) {
	    InsertItem(0, txt);
	    EnsureVisible(0);
        } else {
	    InsertItem(GetItemCount(), txt);
	    EnsureVisible(GetItemCount() - 1);
        }
}

void	AlertList::OnContextMenu(wxContextMenuEvent& event)
{
	wxMenu	menu;
        wxPoint pt = event.GetPosition();

	pt = event.GetPosition();
        pt = ScreenToClient(pt);

	menu.Append(MENU_ALERT_CLEAR, L("Clear"));
	menu.Append(MENU_ALERT_SAVE, L("Save"));
	m_lastOnTop = menu.Append(MENU_ALERT_LAST_ON_TOP, L("Last on Top"), L("Show last alert message at the top of the view"), wxITEM_CHECK);
        m_lastOnTop->Check(alert_last_on_top._iValue != 0);
	PopupMenu(&menu, pt);
}

void	AlertList::OnClear(wxCommandEvent& event)
{
	if(ask(L("Remove all alerts from this list?")) != ANSWER_YES)
	    return;
//	DeleteAllItems();
        traindir->ClearAlert();
}

void	AlertList::OnSave(wxCommandEvent& event)
{
	wxFFile	fp;
	int	i;
	wxChar	buff[512];

	if(GetItemCount() == 0) {
	    wxMessageBox(L("No alerts to save."), wxT("Info"),
		wxOK|wxICON_INFORMATION, traindir->m_frame);
	    return;
	}
	if(!traindir->SaveTextFileDialog(buff))
	    return;
	if(!(fp.Open(buff, wxT("w")))) {
	    wxMessageBox(L("Cannot open file for save."),
		wxT("Info"), wxOK|wxICON_STOP, traindir->m_frame);
	    return;
	}
	for(i = 0; i < GetItemCount(); ++i) {
	    wxString	txt = GetItemText(i);
	    fp.Write(txt);
            fp.Write(wxT("\n"));
	}
	fp.Close();
}

void	AlertList::OnLastOnTop(wxCommandEvent& event)
{
        Char  **lines;
        int     nLines;

        alert_last_on_top.Set(!alert_last_on_top._iValue);
        nLines = GetItemCount();
        if(nLines < 2)
            return;
        lines = (Char **)malloc(sizeof(Char *) * nLines);
        int indx = 0;
        while(--nLines >= 0) {
            wxString ln = GetItemText(nLines);
            lines[indx++] = wxStrdup(ln.c_str());
        }
        for(nLines = 0; nLines < indx; ++nLines) {
            SetItemText(nLines, lines[nLines]);
            free(lines[nLines]);
        }
        free(lines);
}
