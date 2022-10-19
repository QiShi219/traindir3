/*	html.h - Created by Giampiero Caprino

This file is part of Train Director

Train Director is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

Train Director is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Train Director; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#ifndef _HTML_H
#define	_HTML_H

#include "wx/defs.h"
#include "wx/wxchar.h"

class HtmlPage {
public:
	HtmlPage(const wxChar *title);
	~HtmlPage();

	void	StartPage(const wxChar *title);
	void	EndPage();
	void	AddHeader(const wxChar *hdr);
	void	AddRuler();
	void	AddCenter();
	void	EndCenter();
	void	Add(const wxChar *txt);
	void	AddLine(const wxChar *txt);
	void	StartTable(const wxChar *const *headers);
	void	AddTableRow(const wxChar *const *values);
	void	AddTableRow(int nValues, wxString *values[]);
	void	EndTable();

	wxString *content;
};

#endif /* _HTML_H */

