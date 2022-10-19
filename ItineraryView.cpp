/*	ItineraryView.cpp - Created by Giampiero Caprino

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

#include "wx/wx.h"
#include "wx/listctrl.h"
#include "wx/image.h"
#include "wx/ffile.h"

#include "ItineraryView.h"
#include "MainFrm.h"
#include "Traindir3.h"
#include "Itinerary.h"

extern	pxmap	*pixmaps;
extern	int	npixmaps;
extern	void	ShowItineraryDialog(Itinerary *it);

static	const wxChar	*en_titles[] = { wxT("Name"), wxT("Sections"), NULL };
static	const wxChar	*titles[sizeof(en_titles)/sizeof(wxChar *)];
static	int	itinerary_widths[] = { 90, 300, 0 };

BEGIN_EVENT_TABLE(ItineraryView, wxListCtrl)
	EVT_CONTEXT_MENU(ItineraryView::OnContextMenu)
	EVT_MENU(MENU_ITIN_PROPERTIES, ItineraryView::OnProperties)
	EVT_MENU(MENU_ITIN_DELETE, ItineraryView::OnDelete)
	EVT_MENU(MENU_ITIN_SAVE, ItineraryView::OnSave)
END_EVENT_TABLE()

ItineraryView::ItineraryView(wxWindow *parent, const wxString& name)
		: ReportBase(parent, name)
{
	SetName(wxT("itinerary"));
	if(!titles[0])
	    localizeArray(titles, en_titles);
	DefineColumns(titles, itinerary_widths);
}

ItineraryView::~ItineraryView()
{
	freeLocalizedArray(titles);
}

void	FillItineraryTable()
{
	/* Here we do the actual adding of the text. It's done once for
	 * each row.
	 */

	int	i;
	ItineraryView *clist;
	Itinerary *it;
	clist = traindir->m_frame->m_itineraryView;

	if(!clist)
	    return;
	clist->DeleteAllItems();
	clist->Freeze();
	i = 0;
	for(it = itineraries; it; it = it->next) {
	    wxChar	buff[256];
	    wxListItem	item;

	    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s -> %s"), it->signame, it->endsig);
	    clist->InsertItem(i, it->name);
	    clist->SetItem(i, 1, buff);
	    item.SetId(i);
	    item.SetMask(wxLIST_MASK_DATA);
	    clist->GetItem(item);
	    item.SetData(it);
	    clist->SetItem(item);
	    ++i;
	}
	clist->Thaw();
}

void	ItineraryView::OnContextMenu(wxContextMenuEvent& event)
{
        wxPoint pt = event.GetPosition();
	pt = event.GetPosition();
        pt = ScreenToClient(pt);
	wxMenu	menu;
	int	res;

	menu.Append(MENU_ITIN_DELETE, L("Delete"));
	menu.Append(MENU_ITIN_PROPERTIES, L("Properties"));
	menu.Append(MENU_ITIN_SAVE, L("Save"));
	res = PopupMenu(&menu, pt);
}

void	ItineraryView::OnProperties(wxCommandEvent& event)
{
	Itinerary *it = (Itinerary *)GetSelectedData();
	if(!it)			// impossible
	    return;
	ShowItineraryDialog(it);
}

void	ItineraryView::OnDelete(wxCommandEvent& event)
{
	Itinerary *it = (Itinerary *)GetSelectedData();
	if(!it)			// impossible
	    return;
	delete_itinerary(it);
	FillItineraryTable();
}

void	ItineraryView::OnSave(wxCommandEvent& event)
{
	wxFFile	fp;
	Itinerary *it;
	wxChar	buff[512];

	if(!itineraries) {
	    wxMessageBox(L("No itineraries defined."), wxT("Info"),
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
	for(it = itineraries; it; it = it->next) {
	    fp.Write(wxString::Format(wxT("%s : %s -> %s\n"), it->name, it->signame, it->endsig));
	}
	fp.Close();
}
