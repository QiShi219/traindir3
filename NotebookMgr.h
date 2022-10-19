/*	NotebookMgr.h - Created by Giampiero Caprino

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

#ifndef _NOTEBOOKMGR_H
#define _NOTEBOOKMGR_H

#include "wx/notebook.h"
#include "TConfig.h"

class NotebookManager : public wxNotebook
{
public:
	NotebookManager(wxWindow *parent, const wxChar *name, int id);
	~NotebookManager();

	void	OnPageChanged(wxNotebookEvent& event);

	int	FindPage(wxWindow *w);
	int	FindPage(const wxString& name);
	int	FindPageType(const wxChar *name);
	void	RemovePage(wxWindow *pView);

	void	SaveState(const wxString& header, TConfig& state);
	void	LoadState(const wxString& header, TConfig& state);

	wxString    m_name;
private:
	DECLARE_EVENT_TABLE()
};

#endif // _NOTEBOOKMGR_H
