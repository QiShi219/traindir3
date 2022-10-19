/*	HtmlView.cpp - Created by Giampiero Caprino

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
#include "wx/html/htmprint.h"
#include "HtmlView.h"
#include "MainFrm.h"
#include "Traindir3.h"
#include "TDFile.h"


extern	void	ShowWelcomePage(void);
extern  void	WebUIOpenBrowser(const Char *cmd);

BEGIN_EVENT_TABLE(HtmlView, wxHtmlWindow)
	EVT_CONTEXT_MENU(HtmlView::OnContextMenu)	// not used
END_EVENT_TABLE()

HtmlView::HtmlView(wxWindow* parent)
: wxHtmlWindow(parent)
{
	SetName(wxT("htmlview"));
}

HtmlView::~HtmlView()
{
}

void HtmlView::OnLinkClicked(const wxHtmlLinkInfo& link)
{
	wxString    cmd;
	wxString    buff;

	cmd = link.GetHref();

	if(cmd == wxT("status")) {
	    trainsim_cmd(wxT("performance"));
	} else if(cmd == wxT("welcome")) {
	    ShowWelcomePage();
	} else if(cmd.StartsWith(wxT("open:"), &buff)) {
	    if(buff.Len() == 0)
		traindir->OnOpenFile();
	    else
		traindir->OpenFile(buff);
	} else if(cmd.StartsWith(wxT("edit:"), &buff)) {
	    int	pg = traindir->m_frame->m_top->FindPage(L("Layout"));
	    if(pg >= 0)
		traindir->m_frame->m_top->SetSelection(pg);
	    traindir->OnEdit();
        } else if(cmd.StartsWith(wxT("tdwebui"), &buff)) {
            WebUIOpenBrowser(buff.c_str() + 7);
        } else {
            TDFile infoFile(cmd);
	    if(infoFile.Load()) {
                SetPage(infoFile.content);
            } else {
	        trainsim_cmd(cmd);
            }
	}
}

//
//	It would be nice to activate the print feature
//	from a context menu.
//	Unfortunately the event is not delivered to us,
//	so the only way we have to print is through
//	the main menu (see MainFrame).
//

void	HtmlView::OnContextMenu(wxContextMenuEvent& event)
{
	wxMenu	menu;
        wxPoint pt = event.GetPosition();

	pt = event.GetPosition();
        pt = ScreenToClient(pt);

	menu.Append(MENU_HTML_PREVIEW, L("Pre&view"));
	menu.Append(MENU_HTML_PRINT, L("&Print"));
	PopupMenu(&menu, pt);
}

void	HtmlView::OnPrintPreview(wxCommandEvent& event)
{
	wxHtmlEasyPrinting *pr = traindir->m_frame->m_printer;

	FILE	*fp;
	if(!(fp = fopen("C:/Temp/tdir.prn", "w"))) {
	    return;
	}
	fwrite(m_content, 1, m_content.length(), fp);
	fclose(fp);

	pr->PreviewFile(wxT("C:/Temp/tdir.prn"));
	unlink("C:/Temp/tdir.prn");
}

bool	HtmlView::SetPage(const wxString& source)
{
	m_content = source;
	return wxHtmlWindow::SetPage(source);
}

void	HtmlView::OnPrint(wxCommandEvent& event)
{
	wxHtmlEasyPrinting *pr = traindir->m_frame->m_printer;
	FILE	*fp;

	if(!(fp = fopen("C:/Temp/tdir.prn", "w"))) {
	    return;
	}
	fwrite(m_content, 1, m_content.length(), fp);
	fclose(fp);

	pr->PrintFile(wxT("C:/Temp/tdir.prn"));
	unlink("C:/Temp/tdir.prn");
}

