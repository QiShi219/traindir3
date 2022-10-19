/*	MainFrm.h - Created by Giampiero Caprino

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

#ifndef _MAINFRM_H
#define _MAINFRM_H

#include "wx/toolbar.h"
#include "wx/textfile.h"
#include "wx/tglbtn.h"
#include "wx/splitter.h"
#include "wx/html/htmprint.h"
#include "wx/spinbutt.h"
#include "wx/spinctrl.h"

#include "defs.h"
#include "TConfig.h"
#include "TimeTblView.h"
#include "ItineraryView.h"
#include "ToolsView.h"
#include "GraphView.h"
#include "LateGraphView.h"
#include "PlatformGraphView.h"
#include "NotebookMgr.h"

class	Traindir;
class	Canvas;
class	TrainInfoList;
class	AlertList;

#define TIME_TABLE_NONE 0
#define	TIME_TABLE_TAB	1
#define	TIME_TABLE_SPLIT 2
#define TIME_TABLE_FRAME 3

class TimeFrame;

extern	void	ShowStationSchedule(const wxChar *station, bool saveToFile);


class CanvasManager
{
public:
        CanvasManager()
	{
	    memset(m_canvasList, 0, sizeof(m_canvasList));
	}
	~CanvasManager() { }

	Canvas	    *GetNewCanvas();
	void	    *ReleaseCanvas();

	Canvas	    *m_canvasList[NUMCANVASES];
};

class TimeTableViewManager
{
public:
	TimeTableViewManager()
	{
	    memset(m_timeTableList, 0, sizeof(m_timeTableList));
	}
	~TimeTableViewManager() { }

	TimeTableView   *GetNewTimeTableView(wxWindow *parent, const wxString& name)
	{
	    int	    i;

	    for(i = 0; i < NUMTTABLES; ++i) {
		if(!m_timeTableList[i])
		    break;
	    }
	    if(i >= NUMTTABLES)
		return 0;
	    TimeTableView *pTimeTable = new TimeTableView(parent, name);
	    m_timeTableList[i] = pTimeTable;
	    return pTimeTable;
	}

	void	    *ReleaseTimeTableView();
	bool	    IsTimeTable(wxWindow *pWin)
	{
	    int	    i;
	    for(i = 0; i < NUMTTABLES; ++i)
		if(pWin == m_timeTableList[i])
		    return true;
	    return false;
	}

	TimeTableView	*GetTimeTable(int i)
	{
	    if(i >= NUMTTABLES)
		return 0;
	    return m_timeTableList[i];
	}

	wxWindow	*m_parent;
	TimeTableView   *m_timeTableList[NUMTTABLES];
};

class HtmlManager
{
public:
	HtmlManager();
	~HtmlManager();

	Canvas	    *GetNewHtml();
	void	    *ReleaseHtml();

	Canvas	    *m_htmlList[NUMHTMLS];
};


class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title);
	virtual ~MainFrame();

	void	LoadState(const wxString& header, TConfig& state);
	void	SaveState(const wxString& header, TConfig& state);
	void	Finalize();
	void	AddTimeTable(NotebookManager *parent);

	void	MoveTimeTableToTab();
	void	MoveTimeTableToSplit();
	void	MoveTimeTableToFrame();

	// menus
	void	OnOpenFile(wxCommandEvent& event);
	void	OnRecent(wxCommandEvent& event);
	void	OnRestore(wxCommandEvent& event);
	void	OnSaveGame(wxCommandEvent& event);
	void	OnClose(wxCloseEvent& event);
	void	OnQuit(wxCommandEvent& event);
	void	OnTimer(wxTimerEvent& event);
	void	OnPrintSetup(wxCommandEvent& event);
	void	OnPrintPreview(wxCommandEvent& event);
	void	OnPrint(wxCommandEvent& event);

	void	OnEdit(wxCommandEvent& event);
	void	OnNewTrain(wxCommandEvent& event);
	void	OnItinerary(wxCommandEvent& event);
	void	OnSwitchboard(wxCommandEvent& event);
	void	OnSaveLayout(wxCommandEvent& event);
	void	OnPreferences(wxCommandEvent& event);
	void	OnEditSkin(wxCommandEvent& event);
	void	OnNewLayout(wxCommandEvent& event);
	void	OnInfo(wxCommandEvent& event);

	void	OnStartStop(wxCommandEvent& event);
	void	OnGraph(wxCommandEvent& event);
	void	OnLateGraph(wxCommandEvent& event);
	void	OnPlatformGraph(wxCommandEvent& event);
	void	OnRestart(wxCommandEvent& event);
	void	OnFast(wxCommandEvent& event);
	void	OnSlow(wxCommandEvent& event);
	void	OnSkip(wxCommandEvent& event);
	void	OnStationSched(wxCommandEvent& event);
	void	OnSetGreen(wxCommandEvent& event);
	void	OnSelectItin(wxCommandEvent& event);
	void	OnPerformance(wxCommandEvent& event);

	void	OnZoomIn(wxCommandEvent& event);
	void	OnZoomOut(wxCommandEvent& event);
	void	OnShowCoord(wxCommandEvent& event);
	void	OnShowLayout(wxCommandEvent& event);
	void	OnShowSchedule(wxCommandEvent& event);
	void	OnShowInfoPage(wxCommandEvent& event);
	void	OnShowStationsList(wxCommandEvent& event);
	void	OnTimeTableSplit(wxCommandEvent& event);
	void	OnTimeTableTab(wxCommandEvent& event);
	void	OnTimeTableFrame(wxCommandEvent& event);

	void	OnToolBar(wxCommandEvent& event);
	void	OnStatusBar(wxCommandEvent& event);

	void	OnRunButton(wxCommandEvent& event);

	void	OnAbout(wxCommandEvent& event);
	void	OnCopyright(wxCommandEvent& event);
	void	OnLanguage(wxCommandEvent& event);

	void	OnSpinUp(wxSpinEvent& event);
	void	OnSpinDown(wxSpinEvent& event);
//	void	OnSpin(wxSpinEvent& event);

	void	OnChar(wxKeyEvent& event);

	void	ShowTrainInfoList(Train *trn);
	void	ShowItinerary(bool show);
	void	ShowTools(bool show);
	void	ShowGraph(void);
	void	ShowLateGraph(void);
	void	ShowPlatformGraph(void);
	void	ShowHtml(const wxString& name, const wxString& page);
	void	ShowSwitchboardEditor();

	CanvasManager	m_canvasManager;	    // we can have different upper-left corners
	TimeTableViewManager m_timeTableManager;// we can have different stations
	TimeTableView	*m_timeTable;

	//  Current state (saved to config file)

	bool		m_showToolbar;
	bool		m_showStatusbar;
	long		m_timeTableLocation;

	//
	//  Toolbar objects
	//

	wxStaticText	*m_clock;
	wxSpinButton	*m_speedArrows;
	wxTextCtrl	*m_speed;
	wxToggleButton	*m_running;
	wxStaticText	*m_statusText;
	wxStaticText	*m_alertText;

	//
	//
	//

	Traindir	*m_app;
	NotebookManager	*m_top;		// top (pages are Canvases or TimeTable or Html)
	NotebookManager	*m_left;	// bottom-left
	NotebookManager	*m_right;	// bottom-right
	wxSplitterWindow *m_splitter;
	wxSplitterWindow *m_bottomSplitter;
	int		m_topSashValue;
	wxToolBar	*m_toolbar;
	wxStatusBar	*m_statusbar;
	wxTimer		m_timer;
	TimeFrame	*m_timeFrame;	// showing time table in separate frame
	wxHtmlEasyPrinting *m_printer;
	wxString	m_htmlPage;

	ItineraryView	*m_itineraryView;
	ToolsView	*m_toolsView;
	GraphView	*m_graphView;
	LateGraphView	*m_lateGraphView;
	PlatformGraphView *m_platformGraphView;
	TrainInfoList	*m_trainInfo;
	AlertList	*m_alertList;

	wxMenu		*m_viewMenu;
public:
	DECLARE_EVENT_TABLE()
//	DECLARE_NO_COPY_CLASS(MyFrame)
};

#endif // _MAINFRM_H
