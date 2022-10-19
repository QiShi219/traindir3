/*	tgraph.cpp - Created by Giampiero Caprino

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if !defined(__unix__) && !defined(__WXMAC__)
#include <malloc.h>
#endif

#include <wx/wx.h>
#include "Html.h"
#include "trsim.h"
#include "Traindir3.h"
#include "SwitchboardView.h"
#include "SwitchBoard.h"

#define	HEADER_HEIGHT 20
#define	NAMEPANEL_WIDTH 180

#define	CELL_WIDTH 50
#define	CELL_HEIGHT 50

#define	MAXXCELLS   20
#define	MAXYCELLS   20

extern	SwitchBoard *curSwitchBoard;
extern  IntOption     http_server_port;

void	get_switchboard(HtmlPage& page)
{
	wxString	buff;
	wxString	buffs[9];
	const wxChar	*eol;
	int		i;

	eol = wxT("<br>\n");
	page.StartPage(L("Switchboard"));
	page.Add(wxT("<p>"));
	page.Add(L("Use this screen to define the layout of a switchboard and which itineraries are shown in it."));
	page.Add(eol);
	page.Add(L("Switchboards are accessed via an external web browser at the port"));
	buff.Printf(wxT(" %d"), http_server_port._iValue); //8081); //server_port);
	page.Add(buff);
	page.Add(eol);
        page.Add(wxT("<a href=\"sb-browser\">"));
        page.Add(L("Open the switchboard in a browser."));
        page.Add(wxT("</a><br>\n"));
        page.AddCenter();
	page.Add(wxT("<table><tr><td valign=\"top\">\n"));
	
	// 2 tables side by side
	// left table is the list of pages
	// right table is the switchboard for the current page

	page.Add(wxT("<table><tr><th width='180'>"));
	page.Add(L("Switchboards"));
	page.Add(wxT("</th></tr>\n"));

	SwitchBoard *sb;
	if(!curSwitchBoard)
	    curSwitchBoard = switchBoards;

	for(sb = switchBoards; sb; sb = sb->_next) {
	    if(sb == curSwitchBoard) {
		page.Add(wxT("<tr><td bgcolor=\"#c0ffc0\">"));
		page.Add(sb->_name);
		page.Add(wxT("&nbsp;&nbsp;&nbsp;<a href=\"sb-edit -e "));
		page.Add(sb->_fname);
		page.Add(wxT("\">"));
		page.Add(L("change"));
		page.Add(wxT("</a></td></tr>\n"));
	    } else {
		page.Add(wxT("<tr><td bgcolor=\"#e0e0e0\">"));
		page.Add(wxT("<a href=\"sb-edit "));
		page.Add(sb->_fname);
		page.Add(wxT("\">"));
		page.Add(sb->_name);
		page.Add(wxT("</a></td></tr>\n"));
	    }
	}
	page.Add(wxT("<tr><td><hr></td></tr>\n"));
	page.Add(wxT("<tr><td><a href=\"sb-edit\">"));
	page.Add(L("New board"));
//	page.Add(wxT("<tr><td><a href=\"sb-save\">"));
//	page.Add(L("Save"));
//	page.Add(wxT("</a></td></tr>\n"));
	page.Add(wxT("</a></td></tr>\n"));

	page.Add(wxT("</table></td>"));	    // end of left table

	sb = curSwitchBoard;
	if(!sb)
	    sb = switchBoards;

	page.Add(wxT("<td><table><tr valign=\"top\"><td width='40'>&nbsp;</td>\n"));
	for(i = 0; i < MAXXCELLS; ++i) {
	    page.Add(wxT("<th width='70'>"));
	    buff.Printf(wxT("%d"), i);
	    page.Add(buff);
	    page.Add(wxT("</th>\n"));
	}
	page.Add(wxT("</tr>\n"));
	if(!sb) {
	    page.Add(wxT("<tr><td>"));
	    page.Add(L("No selected switchboard."));
	    page.Add(wxT("</td></tr></table>\n"));
	    page.Add(wxT("</td></tr>\n"));

	    page.EndTable();
	    page.EndPage();
	    return;
	}

	for(i = 0; i < MAXYCELLS; ++i) {
	    int j;
	    page.Add(wxT("<tr>"));
	    buff.Printf(wxT("<td width='40'>%d</td>\n"), i);
	    page.Add(buff);
	    for(j = 0; j < MAXXCELLS; ++j) {
		SwitchBoardCell *cell = sb->Find(j, i);
		buff.Printf(wxT("<td width='70' align='center' valign='top'><a href=\"sb-cell %d,%d\">%s</a></td>\n"),
		    j, i, cell ? cell->_text.c_str() : wxT("?"));
		page.Add(buff);
	    }
	    page.Add(wxT("</tr>\n"));
	}
	page.Add(wxT("</tr></table>\n"));
	page.Add(wxT("</td></tr>\n"));

	page.EndTable();
	page.EndPage();
//	buff.Printf(wxT("%s : %s%s"),  L("Time"), format_time(current_time), eol);

}


void	SwitchboardOpenBrowser(const Char *cmd)
{
        wxString url;
        
        if(!curSwitchBoard)
            return;
        url.Printf(wxT("http://localhost:%d/switchboard/%s"), http_server_port._iValue, curSwitchBoard->_fname.c_str());

        wxLaunchDefaultBrowser(url);
}
