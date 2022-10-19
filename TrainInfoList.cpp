/*	TrainInfoList.cpp - Created by Giampiero Caprino

This file is part of Train Director 3

Train Director is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; using exclusively version 2.
It is expressly forbidden the use of higher versions of the GNU
eneral Public License.

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
#include "TrainInfoList.h"
#include "MainFrm.h"
#include "Traindir3.h"

extern	pxmap	*pixmaps;
extern	int	npixmaps;

static	const wxChar	*en_titles[] = { wxT("Station"), wxT("Platform"), wxT("Arrival"), wxT("Departure"), wxT("Min.Stop"), wxT("Late"), 0 };
static	const wxChar	*titles[sizeof(en_titles)/sizeof(wxChar *)];
static	int	schedule_widths[] = { 200, 50, 80, 80, 80, 80, 0 };

TrainInfoList::TrainInfoList(wxWindow *parent, const wxString& name)
: ReportBase(parent, name)
{
	SetName(wxT("traininfo"));
	if(!titles[0])
	    localizeArray(titles, en_titles);
	DefineColumns(titles, schedule_widths);
}

TrainInfoList::~TrainInfoList()
{
	freeLocalizedArray(titles);
}

void	TrainInfoList::Update(Train *trn)
{
	wxListItem	item;
	wxChar	buff[512];
	wxChar	*p;
	int	i;

	DeleteAllItems();
	if(!trn)
	    return;
	Freeze();
	TrainStop   *ts;

	i = 0;
	for(ts = trn->stops; ts; ts = ts->next) {
	    wxStrncpy(buff, ts->station, sizeof(buff)/sizeof(buff[0]) - 1);
	    buff[sizeof(buff)/sizeof(buff[0]) - 1] = 0;
	    if((p = wxStrchr(buff, '@')))
		*p = 0;
	    InsertItem(i, buff);
	    if(p)
		SetItem(i, 1, p + 1);
	    SetItem(i, 2, ts->minstop ? format_time(ts->arrival) : wxT(""));
	    SetItem(i, 3, format_time(ts->departure));
	    buff[0] = 0;
	    if(ts->minstop)
		wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%d"), ts->minstop);
	    SetItem(i, 4, buff);
	    buff[0] = 0;
	    if(ts->delay)
		wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%d"), ts->delay);
	    SetItem(i, 5, buff);

	    item.SetId(i);
	    GetItem(item);
	    if(!ts->minstop)
		item.SetTextColour(*wxBLUE);
	    else if(!findStationNamed(ts->station))
		item.SetTextColour(*wxRED);
	    else
		item.SetTextColour(*wxBLACK);
	    SetItem(item);

	    ++i;
	}
	Thaw();
}

