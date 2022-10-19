/*	html.cpp - Created by Giampiero Caprino

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
#include <wx/string.h>
#include "Html.h"
#include "trsim.h"

HtmlPage::HtmlPage(const wxChar *title)
{
	content = new wxString(wxT("<html>\n"));
	StartPage(title);
}

HtmlPage::~HtmlPage()
{
	if(content)
	    delete content;
	content = 0;
}

void	HtmlPage::StartPage(const wxChar *title)
{
	*content = wxT("<head><title>");
	*content << title;
	*content << wxT("</title></head>\n");
	*content << wxT("<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n");
	if(title && *title) {
	    *content << wxT("<center><h1>");
	    *content << title;
	    *content << wxT("</h1></center>\n");
	    *content << wxT("<hr>\n");
	}
}

void	HtmlPage::EndPage()
{
	*content << wxT("</body></html>\n");
}

void	HtmlPage::AddCenter()
{
	*content << wxT("<center><br>");
}

void	HtmlPage::EndCenter()
{
	*content << wxT("</center><br>");
}

void	HtmlPage::AddHeader(const wxChar *hdr)
{
	*content << wxT("<h1>");
	*content << hdr;
	*content << wxT("</h1>\n");
}

void	HtmlPage::Add(const wxChar *txt)
{
	*content << txt;
}

void	HtmlPage::AddLine(const wxChar *txt)
{
	*content << txt;
	*content << wxT("<br>\n");
}

void	HtmlPage::AddRuler()
{
        *content << wxT("<hr>\n");
}

void	HtmlPage::StartTable(const wxChar *const *headers)
{
	int	i;
	
	*content << wxT("<center><table cellspacing=3>\n");
	*content << wxT("<tr valign=top bgcolor=\"#00ffcc\">\n");
	for(i = 0; headers[i]; ++i) {
	    *content << wxT("<td valign=top>");
	    *content << headers[i];
	    *content << wxT("</td>\n");
	}
	*content << wxT("</tr>\n\n");
}

void	HtmlPage::AddTableRow(const wxChar *const *values)
{
	int	i;

	*content << wxT("<tr VALIGN=TOP>\n");
	for(i = 0; values[i]; ++i) {
	    *content << wxT("<td valign=top>");
	    *content << (*values[i] ? values[i] : wxT("&nbsp;"));
	    *content << wxT("</td>\n");
	}
	*content << wxT("</tr>\n\n");
}

void	HtmlPage::AddTableRow(int nValues, wxString *values[])
{
	int	i;
	const wxString    nbsp(wxT("&nbsp;"));

	*content << wxT("<tr VALIGN=TOP>\n");
	for(i = 0; i < nValues; ++i) {
	    *content << wxT("<td valign=top>");
	    *content << (values[i]->size() ? *values[i] : nbsp);
	    *content << wxT("</td>\n");
	}
	*content << wxT("</tr>\n\n");
}

void	HtmlPage::EndTable()
{
	*content << wxT("</table>\n");
}
