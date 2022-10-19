/*	MainFrm.cpp - Created by Giampiero Caprino

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
#include "wx/splitter.h"
#include "wx/socket.h"
#include "wx/listctrl.h"
#include "wx/html/htmlwin.h"
#include "wx/image.h"
#include "wx/spinctrl.h"
#include "wx/textfile.h"
#include "wx/protocol/http.h"
#include "wx/numdlg.h"
#include "wx/intl.h"
#include "TimeTblView.h"
#include "HtmlView.h"
#include "TrainInfoList.h"
#include "AlertList.h"
#include "Canvas.h"
#include "MainFrm.h"
#include "Traindir3.h"
#include "Html.h"
#include "DaysDialog.h"
#include "OptionsDialog.h"
#include "ItineraryDialog.h"
#include "TrackDialog.h"
#include "TrackScriptDialog.h"
#include "SignalDialog.h"
#include "TriggerDialog.h"
#include "AssignDialog.h"
#include "TrainInfoDialog.h"
#include "ScenarioInfoDialog.h"
#include "StationInfoDialog.h"
#include "ConfigDialog.h"
#include "trsim.h"
#include "SwitchboardNameDialog.h"
#include "SkinColorsDialog.h"
#include "Options.h"

#define	NSTATUSBOXES	5

extern	void	save_schedule_status(HtmlPage& dest);
extern	int	current_tool;
extern	int	layout_modified;
extern	wxString    info_page;		/* HTML page to show in the Scenario Info window */
extern	TDSkin	*curSkin;
extern  StringOption user_name;

extern  void    bstreet_logout();

ItineraryKeyDialog *itinKeyDialog;

void	set_show_coord(bool opt);
void	set_zoom(bool zooming);
int	ask_to_save_layout();
void	get_switchboard(HtmlPage& page);

wxLocale    gLocale;

class MySplitterWindow : public wxSplitterWindow
{
public:
	MySplitterWindow(wxWindow *parent);

	// event handlers
	void OnPositionChanged(wxSplitterEvent& event);
	void OnPositionChanging(wxSplitterEvent& event);
	void OnDClick(wxSplitterEvent& event);
	void OnDoubleClickSash(int x, int y);

private:
	wxFrame	*m_frame;

	DECLARE_EVENT_TABLE()
//	DECLARE_NO_COPY_CLASS(MySplitterWindow)
};

BEGIN_EVENT_TABLE(MySplitterWindow, wxSplitterWindow)
    EVT_SPLITTER_SASH_POS_CHANGED(wxID_ANY, MySplitterWindow::OnPositionChanged)
    EVT_SPLITTER_SASH_POS_CHANGING(wxID_ANY, MySplitterWindow::OnPositionChanging)

    EVT_SPLITTER_DCLICK(wxID_ANY, MySplitterWindow::OnDClick)

//    EVT_SPLITTER_UNSPLIT(wxID_ANY, MySplitterWindow::OnUnsplitEvent)
END_EVENT_TABLE()

MySplitterWindow::MySplitterWindow(wxWindow *parent)
                : wxSplitterWindow(parent, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_3D | wxSP_LIVE_UPDATE |
                                   wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ )
{
//    m_frame = parent;
}

void MySplitterWindow::OnPositionChanged(wxSplitterEvent& event)
{
//    wxLogStatus(m_frame, _T("Position has changed, now = %d (or %d)"),
//                event.GetSashPosition(), GetSashPosition());

    event.Skip();
}

void MySplitterWindow::OnPositionChanging(wxSplitterEvent& event)
{
//    wxLogStatus(m_frame, _T("Position is changing, now = %d (or %d)"),
//                event.GetSashPosition(), GetSashPosition());

    event.Skip();
}

void MySplitterWindow::OnDClick(wxSplitterEvent& event)
{
#if wxUSE_STATUSBAR
//    m_frame->SetStatusText(_T("Splitter double clicked"), 1);
#endif // wxUSE_STATUSBAR

	event.StopPropagation();
//    event.Skip();
}

void MySplitterWindow::OnDoubleClickSash(int x, int y)
{
}

//
//	Show Dialogs
//

void	ShowTrackProperties(Track *trk)
{
	TrackDialog diag(traindir->m_frame);

	diag.ShowModal(trk);
}


void	ShowTrackScriptDialog(Track *trk)
{
	TrackScriptDialog diag(traindir->m_frame);

	diag.ShowModal(trk);
}


void	ShowSignalProperties(Signal *sig)
{
	SignalDialog	diag(traindir->m_frame);

	diag.ShowModal(sig);
}

void	ShowTriggerProperties(Track *trk)
{
	TriggerDialog	diag(traindir->m_frame);

	diag.ShowModal(trk);
}

void    switch_properties_dialog(Track *sw)
{
        TrackDialog diag(traindir->m_frame);

        diag.ShowModal(sw);
}

void	ShowPerformance(void)
{
	HtmlPage    page(wxT(""));
	show_schedule_status(page);
	
	traindir->m_frame->ShowHtml(L("Performance"), *page.content);
}

void	ShowSwitchboard(void)
{
	HtmlPage    page(wxT(""));
	get_switchboard(page);
	
	traindir->m_frame->ShowHtml(L("Switchboard"), *page.content);
}

void	ShowOptionsDialog(void)
{
	OptionsDialog  opts(traindir->m_frame);
	opts.ShowModal();
}

void	ShowDaySelectionDialog(void)
{
	DaysDialog  days(traindir->m_frame);
	days.ShowModal();
}

void	ShowTrainInfo(Train *trn)
{
	HtmlPage    page(wxT(""));

	train_print(trn, page);
	traindir->m_frame->ShowHtml(L("Train Info"), *page.content);
}

void	ShowTrainInfoDialog(Train *trn)
{
	TrainInfoDialog	diag(traindir->m_frame);
	diag.ShowModal(trn);
}

void	ShowScenarioInfoDialog()
{
	ScenarioInfoDialog	diag(traindir->m_frame);
	diag.ShowModal();
}

void	ShowAssignDialog(Train *trn)
{
	AssignDialog  diag(traindir->m_frame);
	diag.ShowModal(trn);
}

void	ShowStationSchedule(const wxChar *name, bool saveToFile)
{
	HtmlPage    page(wxT(""));

	if(!name)
	    return;
	do_station_list_print(name, page);
	if(!saveToFile) {
	    traindir->m_frame->ShowHtml(L("Station Schedule"), *page.content);
	    return;
	}
	traindir->SaveHtmlPage(page);
}

void	ShowStationScheduleDialog(const wxChar *name)
{
	StationInfoDialog	diag(traindir->m_frame);

	diag.ShowModal(name);
}

void	ShowItineraryDialog(Itinerary *it)
{
	ItineraryDialog	itin(traindir->m_frame);

	itin.ShowModal(it);
}

void	ShowWelcomePage(void)
{
	HtmlPage    page(wxT(""));

	traindir->BuildWelcomePage(page);
	traindir->m_frame->ShowHtml(L("Welcome"), *page.content);
}

void	track_info_dialogue(void)
{
//	HtmlPage    page(wxT(""));

//	print_track_info(page);
//	traindir->m_frame->ShowHtml(L("Info"), *page.content);
	ShowScenarioInfoDialog();
}

extern	SwitchBoard *FindSwitchBoard(const wxChar *name);

void	switchboard_name_dialog(const wxChar *name)
{
	SwitchboardNameDialog	sbname(traindir->m_frame);

	SwitchBoard *sb = 0;
	if(name && *name)
	    sb = FindSwitchBoard(name);
	sbname.ShowModal(sb);
}


void	switchboard_cell_dialog(int x, int y)
{
	SwitchboardCellDialog	sbcell(traindir->m_frame);

	sbcell.ShowModal(x, y);
}

void	skin_config_dialog()
{
	SkinColorsDialog	skin(traindir->m_frame, curSkin);

	skin.ShowModal();
}


// ----------------------------------------------------------------------------
// LogFilter
// ----------------------------------------------------------------------------

void	LogFilter::InstallLog()
{
	m_oldLog = wxLog::GetActiveTarget();
	wxLog::SetActiveTarget(this);
}

void	LogFilter::UninstallLog()
{
	wxLog::SetActiveTarget(m_oldLog);
	m_oldLog = 0;
}

void	LogFilter::DoLog(wxLogLevel level, const wxChar *szString, time_t t)
{
	if(m_parent->m_alertList) {
	    m_parent->m_alertList->AddLine(m_extraInfo + wxT(": ") + szString);
	}
//	if(m_oldLog)
//	    m_oldLog->DoLog(level, szString, t);
}

void	 LogFilter::SetExtraInfo(const wxChar *extra)
{
	m_extraInfo = extra;
}


LogFilter   gLogger;

// ----------------------------------------------------------------------------
// TimeFrame
// ----------------------------------------------------------------------------

class TimeFrame : public wxFrame
{
public:
	TimeFrame(MainFrame *parent, const wxString& title);
	virtual ~TimeFrame();
	void	OnClose(wxCloseEvent& event);

	MainFrame *m_parent;

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(TimeFrame, wxFrame)
	EVT_CLOSE(TimeFrame::OnClose)
END_EVENT_TABLE()

TimeFrame::TimeFrame(MainFrame *parent, const wxString& title)
	: wxFrame(parent, wxID_ANY, title),
	m_parent(parent)
{
}

TimeFrame::~TimeFrame()
{
}

//	When we are closed, we need to detach the schedule
//	list view and attach it to something that's still
//	visible, in this case the main frame's main view.

void	TimeFrame::OnClose(wxCloseEvent& event)
{
	wxMenuItem  *pItem;
	
	m_parent->m_timeTable->Reparent(m_parent->m_top);
	m_parent->m_top->AddPage(m_parent->m_timeTable, L("Schedule"), false, -1);
	m_parent->m_timeTableLocation = TIME_TABLE_TAB;
	m_parent->m_timeFrame = 0;
	if((pItem = m_parent->GetMenuBar()->FindItem(MENU_TIME_TAB, 0)))
	    pItem->Check(true);
	event.Skip();
}

// ----------------------------------------------------------------------------
// MainFrame
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_MENU(wxID_OPEN,  MainFrame::OnOpenFile)
	EVT_MENU(MENU_RECENT, MainFrame::OnRecent)
//	EVT_MENU(MENU_RESTORE, MainFrame::OnRestore)
	EVT_MENU(wxID_SAVE,  MainFrame::OnSaveGame)
	EVT_MENU(wxID_REVERT, MainFrame::OnRestore)
	EVT_MENU(MENU_HTML_PRINTSETUP, MainFrame::OnPrintSetup)
	EVT_MENU(MENU_HTML_PREVIEW, MainFrame::OnPrintPreview)
	EVT_MENU(MENU_HTML_PRINT, MainFrame::OnPrint)
	EVT_MENU(wxID_EXIT,  MainFrame::OnQuit)

	EVT_MENU(MENU_EDIT, MainFrame::OnEdit)
	EVT_MENU(MENU_NEW_TRAIN, MainFrame::OnNewTrain)
	EVT_MENU(MENU_ITINERARY, MainFrame::OnItinerary)
	EVT_MENU(MENU_SWITCHBOARD, MainFrame::OnSwitchboard)
	EVT_MENU(MENU_SAVE_LAYOUT, MainFrame::OnSaveLayout)
	EVT_MENU(MENU_PREFERENCES, MainFrame::OnPreferences)
	EVT_MENU(MENU_EDIT_SKIN, MainFrame::OnEditSkin)
	EVT_MENU(MENU_NEW_LAYOUT, MainFrame::OnNewLayout)
	EVT_MENU(MENU_INFO, MainFrame::OnInfo)
	EVT_MENU(MENU_STATIONS_LIST, MainFrame::OnShowStationsList)

	EVT_MENU(MENU_START, MainFrame::OnStartStop)
	EVT_MENU(MENU_GRAPH, MainFrame::OnGraph)
	EVT_MENU(MENU_LATEGRAPH, MainFrame::OnLateGraph)
	EVT_MENU(MENU_PLATFORMGRAPH, MainFrame::OnPlatformGraph)
	EVT_MENU(MENU_RESTART, MainFrame::OnRestart)
	EVT_MENU(MENU_FAST, MainFrame::OnFast)
	EVT_MENU(MENU_SLOW, MainFrame::OnSlow)
	EVT_MENU(MENU_SKIP, MainFrame::OnSkip)
	EVT_MENU(MENU_STATION_SCHED, MainFrame::OnStationSched)
	EVT_MENU(MENU_SETGREEN, MainFrame::OnSetGreen)
	EVT_MENU(MENU_SELECT_ITIN, MainFrame::OnSelectItin)
	EVT_MENU(MENU_PERFORMANCE, MainFrame::OnPerformance)

	EVT_MENU(MENU_ZOOMIN, MainFrame::OnZoomIn)
	EVT_MENU(MENU_ZOOMOUT, MainFrame::OnZoomOut)

	EVT_MENU(MENU_SHOW_COORD, MainFrame::OnShowCoord)
	EVT_MENU(MENU_SHOW_LAYOUT, MainFrame::OnShowLayout)
	EVT_MENU(MENU_SHOW_SCHEDULE, MainFrame::OnShowSchedule)
	EVT_MENU(MENU_INFO_PAGE, MainFrame::OnShowInfoPage)

	EVT_MENU(MENU_TIME_SPLIT, MainFrame::OnTimeTableSplit)
	EVT_MENU(MENU_TIME_TAB, MainFrame::OnTimeTableTab)
	EVT_MENU(MENU_TIME_FRAME, MainFrame::OnTimeTableFrame)

	EVT_MENU(MENU_TOOLBAR, MainFrame::OnToolBar)
	EVT_MENU(MENU_STATUSBAR, MainFrame::OnStatusBar)
	EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
	EVT_MENU(MENU_COPYRIGHT, MainFrame::OnCopyright)
	EVT_MENU(MENU_LANGUAGE, MainFrame::OnLanguage)

	EVT_TOGGLEBUTTON(ID_RUN, MainFrame::OnRunButton)

	EVT_SPIN_UP(ID_SPIN, MainFrame::OnSpinUp)
	EVT_SPIN_DOWN(ID_SPIN, MainFrame::OnSpinDown)
//	EVT_SPINCTRL(ID_SPIN, MainFrame::OnSpin)

	EVT_CLOSE(MainFrame::OnClose)
	EVT_TIMER(TIMER_ID,  MainFrame::OnTimer)
	EVT_CHAR(MainFrame::OnChar)
END_EVENT_TABLE()

void MainFrame::OnAbout(wxCommandEvent& event)
{
	ShowWelcomePage();
}

void MainFrame::OnCopyright(wxCommandEvent& event)
{
	wxString    notice;

	notice.Printf(wxT("%s - %s\n\n"), program_name.c_str(), __DATE__);// show version and build date
	notice += L("Created by");
	notice += wxT(" Giampiero Caprino\n\n");
	notice += L("This is free software, released under the\nGNU General Public License Version 2.\nThe author declines any responsibility for any damage\nthat might occur from the use of this program.");
	notice += wxT("    \n\n");
	notice += L("This is a game, and is not intended to\nbe used to actually control train traffic.");

	wxMessageBox(notice);
//	ShowWelcomePage();
}

void MainFrame::OnLanguage(wxCommandEvent& event)
{
	ConfigDialog	diag(this);

	if(!diag.ShowModal())
	    return;
}

//
//	OnQuit
//	    Called from the File+Exit menu,
//	    or from the Alt-F4 accelerator of the File+Exit menu
//

void MainFrame::OnQuit(wxCommandEvent& event)
{
	Close();
}

//
//	OnClose
//	    Called from the "Close" item of the system menu of the frame,
//	    or when the close button "x" in the frame is clicked.
//

void MainFrame::OnClose(wxCloseEvent& event)
{
	//  TODO: save in project-specific file

	if(layout_modified) {
	    if(ask_to_save_layout() < 0)
		return;
	}
	traindir->SaveState();
        if(user_name._sValue.Length() > 0) {
            bstreet_logout();
        }
	Destroy();
}

//
//
//

MainFrame::MainFrame(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title),
	  m_timer(this, TIMER_ID),
	  m_itineraryView(0),
	  m_toolsView(0),
	  m_graphView(0),
	  m_lateGraphView(0),
	  m_platformGraphView(0),
	  m_alertList(0),
	  m_trainInfo(0)
{
	//
	//	Menus
	//

	wxMenu	*fileMenu = new wxMenu;
	fileMenu->Append(wxID_OPEN, L("&Open...\tCtrl-O"), L("Open a simulation file."));
	fileMenu->Append(wxID_SAVE, L("&Save Game..."), L("Open a saved simulation file."));
	fileMenu->Append(wxID_REVERT, L("&Restore..."), L("Restore from the last saved state."));
	fileMenu->AppendSeparator();
	fileMenu->Append(MENU_HTML_PRINTSETUP, L("Page set&up"), L("Changes the page layout settings."));
	fileMenu->Append(MENU_HTML_PREVIEW, L("Pre&view"), L("Preview print output."));
	fileMenu->Append(MENU_HTML_PRINT, L("&Print"), L("Print all or part of the document."));
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, L("E&xit\tAlt-F4"), L("Quit this program."));

	wxMenu	*editMenu = new wxMenu;
	editMenu->Append(MENU_EDIT, L("&Edit\tCtrl-E"), L("Enter/exit layout editor."));
//	editMenu->Append(MENU_NEW_TRAIN, L("&New Train..."), L("Enter data about a new train."));
	editMenu->Append(MENU_ITINERARY, L("&Itinerary"), L("Enter/exit itinerary editor."));
	editMenu->Append(MENU_SWITCHBOARD, L("Switch&board"), L("Shows the switchboard editor."));
	editMenu->Append(MENU_SAVE_LAYOUT, L("&Save Layout"), L("Save changes to the layout."));
	editMenu->Append(MENU_PREFERENCES, L("&Preferences..."), L("Change program's preferences."));
//	editMenu->Append(MENU_EDIT_SKIN, L("S&kin colors..."), L("Change the colors of graphical elements."));
	editMenu->Append(MENU_NEW_LAYOUT, L("Ne&w"), L("Erase the layout."));
	editMenu->Append(MENU_INFO, L("In&fo"), L("Show information about the scenario."));
	editMenu->Append(MENU_STATIONS_LIST, L("Stations &List"), L("Show list of stations and entry points."));

	wxMenu	*runMenu = new wxMenu;
	runMenu->Append(MENU_START, L("&Start\tCtrl-S"), L("Start/stop the simulation."));
	runMenu->Append(MENU_GRAPH, L("&Graph\tCtrl-G"), L("Show the timetable graph."));
	runMenu->Append(MENU_LATEGRAPH, L("&Late Graph\tCtrl-L"), L("Show accumulation of late minutes over time."));
	runMenu->Append(MENU_PLATFORMGRAPH, L("&Platform Graph"), L("Show platforms occupancy over time."));
	runMenu->Append(MENU_RESTART, L("&Restart..."), L("Restart the simulation."));
	runMenu->Append(MENU_FAST, L("&Fast\tCtrl-X"), L("Speed up the simulation."));
	runMenu->Append(MENU_SLOW, L("&Slow\tCtrl-Z"), L("Slow down the simulation."));
	runMenu->Append(MENU_SKIP, L("S&kip ahead\tCtrl-K"), L("Skip to 3 minutes before next event."));
	runMenu->Append(MENU_STATION_SCHED, L("S&tation schedule\tF6"), L("Show the train schedule of each station."));
	runMenu->Append(MENU_SETGREEN, L("Set sig. to green"), L("Set all automatic signals to green."));
	runMenu->Append(MENU_SELECT_ITIN, L("Select Itinerary\tCtrl-I"), L("Select an itinerary by name."));
	runMenu->Append(MENU_PERFORMANCE, L("&Performance"), L("Show performance data."));

	m_viewMenu = new wxMenu;
	m_viewMenu->Append(MENU_SHOW_LAYOUT, L("Show layout\tF3"), L("Forcibly show the layout window."));
	m_viewMenu->Append(MENU_SHOW_SCHEDULE, L("Show schedule\tF4"), L("Forcibly show the schedule window."));
	m_viewMenu->Append(MENU_INFO_PAGE, L("Show info page\tF5"), L("Shows the scenario info page, if available."));
	m_viewMenu->AppendSeparator();
	m_viewMenu->AppendRadioItem(MENU_TIME_SPLIT, L("Timetable in split window"), L("View timetable in a split window."));
	m_viewMenu->AppendRadioItem(MENU_TIME_TAB, L("Timetable in tabbed window"), L("View timetable in a tab of the main window."));
	m_viewMenu->AppendRadioItem(MENU_TIME_FRAME, L("Timetable in separate window"), L("View timetable in a window separate from the main window."));

	m_viewMenu->AppendSeparator();

	m_viewMenu->Append(MENU_ZOOMIN, L("Zoom in"), L("Draw the layout at double the resolution."));
	m_viewMenu->Append(MENU_ZOOMOUT, L("Zoom out"), L("Draw the layour at normal resolution."));

	m_viewMenu->AppendSeparator();

	m_viewMenu->AppendCheckItem(MENU_SHOW_COORD, L("Coord bars"), L("View/hide the coordinate bars."));
	m_viewMenu->AppendCheckItem(MENU_TOOLBAR, L("Tool bar"), L("View/hide the tools bar."));
	m_viewMenu->AppendCheckItem(MENU_STATUSBAR, L("Status bar"), L("View/hide the status bar."));

	wxMenu	*helpMenu = new wxMenu;
	helpMenu->Append(wxID_ABOUT, L("Welcome\tF1"), L("Show the welcome page."));
	helpMenu->Append(MENU_COPYRIGHT, L("Copyright"), L("Show the copyright notice."));
	helpMenu->Append(MENU_LANGUAGE, L("Language"), L("Select the language to be used next time Traindir is started."));

	wxMenuBar *menuBar = new wxMenuBar;

	menuBar->Append(fileMenu, L("&File"));
	menuBar->Append(editMenu, L("&Edit"));
	menuBar->Append(runMenu,  L("&Run"));
	menuBar->Append(m_viewMenu, L("&View"));
	menuBar->Append(helpMenu, L("&Help"));

	SetMenuBar(menuBar);

	wxMenuItem  *pItem;
	
	if((pItem = menuBar->FindItem(MENU_TIME_SPLIT, 0)))
	    pItem->Check(true);
	m_timeTableLocation = TIME_TABLE_SPLIT;
	if((pItem = menuBar->FindItem(MENU_TOOLBAR, 0)))
	    pItem->Check(true);
	m_showToolbar = true;
	if((pItem = menuBar->FindItem(MENU_STATUSBAR, 0)))
	    pItem->Check(true);
	m_showStatusbar = true;
	if((pItem = menuBar->FindItem(MENU_SHOW_COORD, 0)))
	    pItem->Check(bShowCoord);

	//
	//	Toolbar
	//
	wxString	buff(wxT('M'), 70);

	m_toolbar = new wxToolBar(this, wxID_ANY,
			    wxDefaultPosition, wxDefaultSize,
			    wxTB_HORIZONTAL | wxNO_BORDER | wxTB_DOCKABLE);

	m_clock = new wxStaticText(m_toolbar, 0, wxEmptyString,
	    wxDefaultPosition, wxDefaultSize);
	m_clock->SetLabel(wxT("   00:00.00    "));
	m_toolbar->AddControl(m_clock);

	m_speed = new wxTextCtrl(m_toolbar, ID_SPEEDTEXT, wxT("10"), wxDefaultPosition,
		    wxSize(40,30));
	m_speed->Enable(false);
	m_toolbar->AddControl(m_speed);

	m_speedArrows = new wxSpinButton(m_toolbar, ID_SPIN);
	m_speedArrows->SetValue(5);
	m_toolbar->AddControl(m_speedArrows);

	m_toolbar->AddSeparator();

	m_running = new wxToggleButton(m_toolbar, ID_RUN,
	    L("Start"), wxDefaultPosition, wxSize(50, 30));
	m_toolbar->AddControl(m_running);

	m_toolbar->AddSeparator();

	m_statusText = new wxStaticText(m_toolbar, wxID_ANY, wxEmptyString,
	    wxDefaultPosition, wxDefaultSize);
	m_statusText->SetLabel(buff);
	m_toolbar->AddControl(m_statusText);

	m_toolbar->AddSeparator();

	m_alertText = new wxStaticText(m_toolbar, wxID_ANY, wxEmptyString,
	    wxDefaultPosition, wxDefaultSize);
	m_alertText->SetLabel(buff);
	m_toolbar->AddControl(m_alertText);

	m_toolbar->Realize();
	SetToolBar(m_toolbar);

	m_statusText->SetLabel(wxT(""));
	m_alertText->SetLabel(wxT(""));

	//
	//	Status bar
	//

	wxStatusBar *m_statusBar = new wxStatusBar(this, wxID_ANY);
	int widths[NSTATUSBOXES] = { -50, -30, -20, -30 , -50 };
	m_statusBar->SetFieldsCount(WXSIZEOF(widths), widths);
	SetStatusBar(m_statusBar);

	//
	//	Client area
	//
	//	m_splitter controls the top and bottom views
	//

	m_splitter = new MySplitterWindow(this);
	m_splitter->SetSashGravity(1.0);

	//
	//	the top view is the layout Canvas
	//	inside a managing notebook
	//

	m_top = new NotebookManager(m_splitter, wxT("top"), ID_NOTEBOOK_TOP);

	Canvas *pCanvas = new Canvas(m_top);
	pCanvas->SetName(wxT("canvas"));
	m_top->AddPage(pCanvas, L("Layout"), true, -1);

	//
	//	the bottom view is another splitter
	//

	m_bottomSplitter = new MySplitterWindow(m_splitter);
	m_bottomSplitter->SetSashGravity(0.5);

	//
	//	the bottom left view has a managed
	//	notebook which will hold the schedule
	//	list, edit tools view and itinerary list
	//

	m_left = new NotebookManager(m_bottomSplitter, wxT("left"), ID_NOTEBOOK_LEFT);
	m_timeTable = m_timeTableManager.GetNewTimeTableView(m_left, L("Schedule"));
	m_itineraryView = new ItineraryView(m_left, L("Itinerary"));
	m_itineraryView->Show(false);

	//
	//	the bottom right view has a managed
	//	notebook which will hold the train
	//	info list and the alerts list
	//

	m_right = new NotebookManager(m_bottomSplitter, wxT("right"), ID_NOTEBOOK_RIGHT);
	m_alertList = new AlertList(m_right, L("Alerts"));
	m_trainInfo = new TrainInfoList(m_right, L("Train Info"));

	m_timeFrame = false;

	// you can also do this to start with a single window
#if 0
	m_top->Show(false);
	m_splitter->Initialize(m_left);
#else
	// you can also try -100
	m_splitter->SplitHorizontally(m_top, m_bottomSplitter, 300);

	wxSize	sz = GetSize();
	m_bottomSplitter->SplitVertically(m_left, m_right, -300);

	m_top->Show(true);
	m_left->Show(true);
	m_right->Show(true);
	m_timer.Start(100);
#endif
	gLogger.SetParent(this);

	m_printer = new wxHtmlEasyPrinting(L(""), this);

	m_timeFrame = 0;

	track_properties_dialog = ShowTrackProperties;
	signal_properties_dialog = ShowSignalProperties;
	trigger_properties_dialog = ShowTriggerProperties;
	performance_dialog = ShowPerformance;
	options_dialog = ShowOptionsDialog;
	select_day_dialog = ShowDaySelectionDialog;
	train_info_dialog = ShowTrainInfoDialog;
	assign_dialog = ShowAssignDialog;
	station_sched_dialog = ShowStationScheduleDialog;
	itinerary_dialog = ShowItineraryDialog;
	about_dialog = ShowWelcomePage;
}

//
//
//

MainFrame::~MainFrame()
{
	if (m_splitter) {
	    delete m_splitter;
	    m_splitter = 0;
	}
	if (m_printer) {
	    delete m_printer;
	    m_printer = 0;
	}
}

//	The travels of the time table view...
//
//	The time table can be in any of 3 states:
//	1) default it is a tab in the left notebook of the horizontal splitter
//	2) it's a tab in the top (only) notebook
//	3) it's the only child of a separate frame
//
//	The following routines implement the transitions
//	between these 3 states

void	MainFrame::MoveTimeTableToTab()
{
	if(m_timeTableLocation == TIME_TABLE_SPLIT) {
	    m_topSashValue = m_splitter->GetSashPosition();
	    m_left->RemovePage(m_timeTable);
	} else if(m_timeTableLocation == TIME_TABLE_FRAME) {
	    m_timeFrame->Show(false);
	}
	m_timeTable->Reparent(m_top);
	m_top->AddPage(m_timeTable, L("Schedule"));
	m_splitter->Unsplit();
	m_timeTableLocation = TIME_TABLE_TAB;
//	AddTimeTable(m_top);
}

void	MainFrame::MoveTimeTableToSplit()
{
	if(m_timeTableLocation == TIME_TABLE_TAB) {
	    m_top->RemovePage(m_timeTable);
	} else if(m_timeTableLocation == TIME_TABLE_FRAME) {
	    m_timeFrame->Show(false);
	}
	m_splitter->SplitHorizontally(m_top, m_bottomSplitter, m_topSashValue);
	m_timeTableLocation = TIME_TABLE_SPLIT;
	m_timeTable->Reparent(m_left);
	m_left->AddPage(m_timeTable, L("Schedule"));
}

void	MainFrame::MoveTimeTableToFrame()
{
	if(!m_timeFrame)
	    m_timeFrame = new TimeFrame(this, L("Schedule"));

	if(m_timeTableLocation == TIME_TABLE_TAB) {
	    m_top->RemovePage(m_timeTable);
	} else if(m_timeTableLocation == TIME_TABLE_SPLIT) {
	    m_topSashValue = m_splitter->GetSashPosition();
	    m_left->RemovePage(m_timeTable);
	    m_splitter->Unsplit();
	}

	m_timeTable->Reparent(m_timeFrame);
	wxSize size = m_timeFrame->GetClientSize();
	m_timeTable->SetSize(0, 0, size.x, size.y);
	m_timeFrame->Show(true);
	m_timeTable->Show(true);
	m_timeTable->Refresh();
	m_timeTableLocation = TIME_TABLE_FRAME;
}

//
//	LoadState()
//	    Reload the state of the frame window (position, size)
//	    from file saved on the last run.
//

void	MainFrame::LoadState(const wxString& header, TConfig& state)
{
	wxString    line;
	bool	    gotDimensions = false;
	bool	    bMaximized = false;
	wxSize	    size(-1, -1);
	wxPoint	    pos(-1, -1);
	int	    nTimetables = 0;
	int	    sashPosition = 100;
	int	    lowerSashPosition = 100;
	bool	    bGotSash = false;
	bool	    bGotLowerSash = false;

	if(!state.FindSection(header))
	    return;

	int	    v;

	//
	//  Reload information about the size and position of the window
	//

	if(state.GetInt(wxT("isMaximized"), v))
	    bMaximized = v ? true : false;
	if(state.GetInt(wxT("showStatusBar"), v))
	    m_showStatusbar = v ? true : false;
	if(state.GetInt(wxT("showToolBar"), v))
	    m_showToolbar = v ? true : false;
	if(state.GetInt(wxT("showCoord"), v))
	    bShowCoord = v ? true : false;

	if(state.GetInt(wxT("x"), v)) {
	    gotDimensions = true;
	    pos.x = v;
	}
	if(state.GetInt(wxT("y"), v)) {
	    gotDimensions = true;
	    pos.y = v;
	}
	if(state.GetInt(wxT("w"), v)) {
	    gotDimensions = true;
	    size.x = v;
	}
	if(state.GetInt(wxT("h"), v)) {
	    gotDimensions = true;
	    size.y = v;
	}

	//
	//  Reload state of splitter
	//

	bGotSash = state.GetInt(wxT("sash"), sashPosition);
	bGotLowerSash = state.GetInt(wxT("lower-sash"), lowerSashPosition);

	if(gotDimensions) {
	    SetPosition(pos);
	    SetSize(size);
	}
	if(bGotSash) {
	    m_splitter->SetSashPosition(sashPosition);
	    m_topSashValue = sashPosition;
	}
	if(bGotLowerSash) {
	    m_bottomSplitter->SetSashPosition(lowerSashPosition);
	}

//	Maximize(bMaximized);

	//  Synchronize menus

	wxMenuBar   *pBar = GetMenuBar();
	wxMenuItem  *pItem;
	
	if(m_toolbar) {
	    if((pItem = pBar->FindItem(MENU_TOOLBAR, 0)))
		pItem->Check(m_showToolbar);
	    m_toolbar->Show(pItem->IsChecked());
	}
	wxStatusBar *pStatus;
	
	if((pStatus = GetStatusBar())) {
	    if((pItem = pBar->FindItem(MENU_STATUSBAR, 0)))
		pItem->Check(m_showStatusbar);
	    pStatus->Show(m_showStatusbar);
	}

	set_show_coord(bShowCoord);
	if((pItem = pBar->FindItem(MENU_SHOW_COORD, 0)))
	    pItem->Check(bShowCoord);

	int val;
	int loc;
	if(!state.GetInt(wxT("timeTableLocation"), loc))
	    loc = m_timeTableLocation;
	switch(loc) {
	case TIME_TABLE_TAB:

	    val = MENU_TIME_TAB;
	    MoveTimeTableToTab();
	    break;

	case TIME_TABLE_FRAME:
	    
	    m_timeFrame = new TimeFrame(this, L("Schedule"));
	    gotDimensions = false;
	    if(state.GetInt(wxT("timex"), v)) {
		gotDimensions = true;
		pos.x = v;
	    }
	    if(state.GetInt(wxT("timey"), v)) {
		gotDimensions = true;
		pos.y = v;
	    }
	    if(state.GetInt(wxT("timew"), v)) {
		gotDimensions = true;
		size.x = v;
	    }
	    if(state.GetInt(wxT("timeh"), v)) {
		gotDimensions = true;
		size.y = v;
	    }
	    if(gotDimensions) {
		m_timeFrame->SetPosition(pos);
		m_timeFrame->SetSize(size);
	    }
	    MoveTimeTableToFrame();
	    val = MENU_TIME_FRAME;
	    break;

	case TIME_TABLE_SPLIT:
	default:
	    val = MENU_TIME_SPLIT;
	}
	m_timeTableLocation = loc;

	if((pItem = pBar->FindItem(val, 0)))
	    pItem->Check(m_showToolbar);

	SendSizeEvent();

	//
	//  Reload the dimensions of each list view for each
	//  of the views.
	//

	m_top->LoadState(header, state);
	m_left->LoadState(header, state);
	m_right->LoadState(header, state);
        if(m_timeTableLocation == TIME_TABLE_FRAME) {
            m_timeTable->LoadState(wxT("time-frame"), state);
        }
}

//
//	SaveState()
//	    Save the state (size, position) of the frame window
//	    and the state of any child view, mainly the list views
//

void	MainFrame::SaveState(const wxString& header, TConfig& state)
{
	int	nTT = 0;

	m_top->SaveState(header, state);
	m_left->SaveState(header, state);
	m_right->SaveState(header, state);
	if(m_timeFrame) {
            if(m_timeTableLocation == TIME_TABLE_FRAME) {
                m_timeTable->SaveState(wxT("time-frame"), state);
            }
        }

	state.StartSection(header);
	state.PutInt(wxT("isMaximized"), IsMaximized());
	state.PutInt(wxT("showStatusBar"), m_showStatusbar);
	state.PutInt(wxT("showToolBar"), m_showToolbar);
	state.PutInt(wxT("showCoord"), bShowCoord);
	state.PutInt(wxT("timeTableLocation"), m_timeTableLocation);

	wxSize	size = GetSize();
	wxPoint pos = GetPosition();

	state.PutInt(wxT("x"), pos.x);
	state.PutInt(wxT("y"), pos.y);
	state.PutInt(wxT("w"), size.x);
	state.PutInt(wxT("h"), size.y);
	state.PutInt(wxT("sash"), m_splitter->GetSashPosition());
	state.PutInt(wxT("lower-sash"), m_bottomSplitter->GetSashPosition());

	state.PutInt(wxT("selected"), m_top->GetSelection());

	if(m_timeFrame) {
	    size = m_timeFrame->GetSize();
	    pos = m_timeFrame->GetPosition();
	    state.PutInt(wxT("timex"), pos.x);
	    state.PutInt(wxT("timey"), pos.y);
	    state.PutInt(wxT("timew"), size.x);
	    state.PutInt(wxT("timeh"), size.y);
	}
}

//
//
//

void	MainFrame::AddTimeTable(NotebookManager *parent)
{
	unsigned int	i;

	for(i = 0; i < parent->GetPageCount(); ++i) {
	    wxWindow *pPage = parent->GetPage(i);
	    if(pPage->GetName() == wxT("timetable")) {
		break;
	    }
	}
	if(i >= parent->GetPageCount()) {	// not found, maybe first time
	    TimeTableView *pTimeTable =
		m_timeTableManager.GetNewTimeTableView(parent, L("Schedule"));
	    parent->AddPage(pTimeTable, L("Schedule"), false, -1);
	    parent->SetSelection(i);
	}
}

//
//
//

void	MainFrame::Finalize()
{
	int	i;

	switch(m_timeTableLocation) {
	case TIME_TABLE_TAB:
//	    AddTimeTable(m_top);
	    if(m_top->FindPage(m_timeTable) < 0)
		m_top->AddPage(m_timeTable, L("Schedule"), false, -1);
	    m_timeTable->Reparent(m_top);
	    break;

	case TIME_TABLE_SPLIT:
//	    AddTimeTable(m_left);
//	    if((i = m_left->FindPageType(wxT("timetable"))) < 0)
	    if(m_left->FindPage(m_timeTable) < 0)
		m_left->AddPage(m_timeTable, L("Schedule"), false, -1);
	    m_timeTable->Reparent(m_left);
	    break;

	case TIME_TABLE_FRAME:
	    break;
	}

	// see if we already created the alerts tab
	// when loading the state of the right notebook

	i = m_right->FindPageType(wxT("alerts"));
	if(i < 0)	// not found, maybe first time
	    m_right->AddPage(m_alertList, L("Alerts"), false, -1);

	// see if we already created the traininfo tab
	// when loading the state of the right notebook

	i = m_right->FindPageType(wxT("traininfo"));
	if(i < 0)	// not found, maybe first time
	    m_right->AddPage(m_trainInfo, L("Train Info"), false, -1);

	repaint_labels();
}

//
//
//

void	MainFrame::OnPrintSetup(wxCommandEvent& event)
{
	m_printer->PageSetup();
}

//
//
//

void	MainFrame::OnPrintPreview(wxCommandEvent& event)
{
	wxWindow *w = m_top->GetCurrentPage();
	if(!w)
	    return;
	if(w->GetName() == wxT("htmlview")) {
	    HtmlView *p = (HtmlView *)w;
	    p->OnPrintPreview(event);
	} else
	    wxMessageBox(L("Printing of this page is not supported."),
		    wxT("Error"), wxOK | wxICON_STOP, this);
}

//
//
//

void	MainFrame::OnPrint(wxCommandEvent& event)
{
	wxWindow *w = m_top->GetCurrentPage();
	if(!w)
	    return;
	wxString name = w->GetName();
	if(name == wxT("htmlview")) {
	    HtmlView *p = (HtmlView *)w;
	    p->OnPrint(event);
	} else if(name == wxT("canvas")) {
	    Canvas *p = (Canvas *)w;
	    p->DoPrint();
	} else {
	    wxMessageBox(L("Printing of this page is not supported."),
		    wxT("Error"), wxOK | wxICON_STOP, this);
	}
}

//
//
//

void	MainFrame::OnTimer(wxTimerEvent& event)
{
	traindir->OnTimer();	
}

//
//	Customize user interface
//


void	MainFrame::OnZoomIn(wxCommandEvent& event)
{
	set_zoom(true);
	int	pg = m_top->FindPage(L("Layout"));
	if(pg >= 0) {
	    m_top->SetSelection(pg);
	    m_top->Refresh();
	}
}

//
//
//

void	MainFrame::OnZoomOut(wxCommandEvent& event)
{
	set_zoom(false);
	int	pg = m_top->FindPage(L("Layout"));
	if(pg >= 0) {
	    m_top->SetSelection(pg);
	    m_top->Refresh();
	}
}

//
//
//

void	MainFrame::OnShowCoord(wxCommandEvent& event)
{
	wxMenuBar   *pBar = GetMenuBar();
	wxMenuItem  *pItem;
	
	if((pItem = pBar->FindItem(MENU_SHOW_COORD, 0))) {
	    bShowCoord = pItem->IsChecked();
	}
//	bShowCoord = !bShowCoord;
	set_show_coord(bShowCoord);
	invalidate_field();
	repaint_all();
}

//
//
//

void	MainFrame::OnShowLayout(wxCommandEvent& event)
{
	int	pg = m_top->FindPage(L("Layout"));
	if(pg >= 0) {
	    m_top->SetSelection(pg);
	    m_top->Refresh();
	}
}

//
//
//

void	MainFrame::OnShowSchedule(wxCommandEvent& event)
{
	int	pg;
	NotebookManager *parent;
	
	if(m_timeTableLocation == TIME_TABLE_TAB)
	    parent = m_top;
	else if(m_timeTableLocation == TIME_TABLE_SPLIT)
	    parent = m_left;
	else // TODO: bring frame to top
	    return;
	pg = parent->FindPage(L("Schedule"));
	if(pg >= 0) {
	    parent->SetSelection(pg);
//	    parent->Refresh();
	}
}

//
//
//

void	MainFrame::OnShowInfoPage(wxCommandEvent& event)
{
	if(!info_page[0])
	    return;
	wxString    page(wxT("showinfo "));
	page += info_page;
	trainsim_cmd(page.c_str());
}

wxChar	anchor[512];
wxChar	htmltext[512];

struct HourLinks {
	HourLinks   *m_next;
	wxString    m_hour;
	wxString    m_station;
	wxString    m_link;
};

HourLinks   *pHourLinks;

//
//
//

void	TraverseNodes(wxHtmlCell* node)
{
	wxHtmlCell  *child;

	if(!node) {
	    return;
	}
	wxString    txt = node->ConvertToText(NULL);
	wxStrncpy(htmltext, txt, sizeof(htmltext)/sizeof(wxChar));
	wxHtmlLinkInfo *link = node->GetLink();

	anchor[0] = 0;
	if(link) {
	    wxStrncpy(anchor, link->GetHref(), sizeof(anchor)/sizeof(wxChar));
	}
	if(htmltext[0]) {
	    if(htmltext[2] == ':' && wxStrlen(htmltext) == 5 && wxIsdigit(htmltext[1]) && anchor[0]) {
		HourLinks *p = new HourLinks;
		p->m_next = pHourLinks;
		pHourLinks = p;
		p->m_link = anchor;
		p->m_hour = htmltext;
	    }
	}
	for(child = node->GetFirstChild(); child; ) {
	    TraverseNodes(child);
	    child = child->GetNext();
	}
}

//
//
//

int	GetHtmlPage(const wxChar *url)
{
#if 0
	const wxChar	*pc;
	wxChar *pc1;

	if(strncmp(url, wxT("http://"), 7))
	    return 0;

	pc = url + 7;
	for(pc1 = anchor; *pc && *pc != '/'; *pc1++ = *pc++);
	*pc1 = 0;

	wxHTTP	http;
	http.Connect(anchor/*"reiseauskunft.bahn.de"*/, 80);
	wxInputStream *inp = http.GetInputStream(pc);//"/bin/zuginfo.exe/en/338733/224970/999712/386945/80/");

	FILE	*fp;
	int	ch;
	fp = fopen("C:/temp/tt.htm", "w");
	while(!inp->Eof()) {
	    ch = inp->GetC();
	    fputc(ch, fp);
	}
	fclose(fp);
#endif
	return 0;
}

int	m_TrainPageStatus = 0;
//
//
//

void	GetListOfTrains(wxHtmlCell *node)
{
	wxHtmlCell  *child;

	if(!node) {
	    return;
	}
	wxString    txt = node->ConvertToText(NULL);
	wxStrncpy(htmltext, txt, sizeof(htmltext)/sizeof(wxChar));
	wxHtmlLinkInfo *link = node->GetLink();

	anchor[0] = 0;
	if(link) {
	    wxStrncpy(anchor, link->GetHref(), sizeof(anchor)/sizeof(wxChar));
	}
	if(htmltext[0]) {
	    switch(m_TrainPageStatus) {
	    case 5:
		return;

	    case 4:
		if(!wxStrcmp(htmltext, wxT("shown"))) {
		    m_TrainPageStatus = 5;
		    break;
		}
		goto st2;

	    case 3:
		if(!wxStrcmp(htmltext, wxT("stops"))) {
		    m_TrainPageStatus = 4;
		    break;
		}
st2:
		m_TrainPageStatus = 2;
		// fall through

	    case 2:
		if(!wxStrcmp(htmltext, wxT("All"))) {
		    m_TrainPageStatus = 3;
		    break;
		}
#if 0
		if(0 && htmltext[2] == ':' && wxStrlen(htmltext) == 5 && wxIsdigit(htmltext[1]) && anchor[0]) {
		    HourLinks *p = new HourLinks;
		    p->m_next = pHourLinks;
		    pHourLinks = p;
		    p->m_link = anchor;
		    p->m_hour = htmltext;
		}
#endif
		break;
		

	    case 1:
		if(!wxStrcmp(htmltext, wxT("Train"))) {
		    m_TrainPageStatus = 2;
		    break;
		}

		m_TrainPageStatus = 0;
		// fall through

	    case 0:
		if(!wxStrcmp(htmltext, wxT("Time ")))
		    m_TrainPageStatus = 1;
		break;
	    }
	}
	for(child = node->GetFirstChild(); child; ) {
	    GetListOfTrains(child);
	    child = child->GetNext();
	}
}

//
//
//

void	ParseStationPage(HtmlView *htmlView, const wxChar *fname)
{
	wxFileName  name(fname);
	htmlView->LoadFile(name);
	wxHtmlContainerCell *root = htmlView->GetInternalRepresentation();
	m_TrainPageStatus = 0;
	GetListOfTrains(root);
}

//
//
//

void	MainFrame::OnShowStationsList(wxCommandEvent& event)
{
#if 01
	traindir->ShowStationsList();
#else
	int	pg = m_top->FindPage(wxT("stations"));
	HtmlView *htmlView;

	if(pg < 0) {
	    htmlView = new HtmlView(m_top);
	    m_top->AddPage(htmlView, wxT("stations"), true, -1);
	} else
	    htmlView = (HtmlView *)m_top->GetPage(pg);

	pHourLinks = 0;

	GetHtmlPage(wxT("http://reiseauskunft.bahn.de/bin/zuginfo.exe/en/338733/224970/999712/386945/80/"));

	wxFileName  name(wxT("C:/temp/vc.htm"));
	htmlView->LoadFile(name);
	wxHtmlContainerCell *root = htmlView->GetInternalRepresentation();
	TraverseNodes(root);

	HourLinks   *ph;
	for(ph = pHourLinks; ph; ph = ph->m_next) {
	    GetHtmlPage(ph->m_link.fn_str());
	    ParseStationPage(htmlView, wxT("C:/temp/tt.htm"));
/***/	    break;
	}
	wxFileName name1(wxT("C:/temp/tt.htm"));
	htmlView->LoadFile(name1);

	pg = m_top->FindPage(wxT("stations"));
	m_top->SetSelection(pg);
#endif
}

//
//
//

void	MainFrame::OnTimeTableSplit(wxCommandEvent& event)
{
	MoveTimeTableToSplit();
	OnShowLayout(event);
}

//
//
//

void	MainFrame::OnTimeTableTab(wxCommandEvent& event)
{
	MoveTimeTableToTab();
}

//
//
//

void	MainFrame::OnTimeTableFrame(wxCommandEvent& event)
{
	MoveTimeTableToFrame();
}

//
//
//

void	MainFrame::OnToolBar(wxCommandEvent& event)
{
	wxMenuBar   *pBar = GetMenuBar();
	wxMenuItem  *pItem;
	
	if((pItem = pBar->FindItem(MENU_TOOLBAR, 0))) {
	    m_toolbar->Show(m_showToolbar = pItem->IsChecked());
	    SendSizeEvent();
	}
}

//
//
//

void	MainFrame::OnStatusBar(wxCommandEvent& event)
{
	wxMenuBar   *pBar = GetMenuBar();
	wxMenuItem  *pItem;
	wxStatusBar *pStatus;
	
	if((pItem = pBar->FindItem(MENU_STATUSBAR, 0))) {
	    pStatus = GetStatusBar();
	    pStatus->Show(m_showStatusbar = pItem->IsChecked());
	    SendSizeEvent();
	}
}

//
//
//

void MainFrame::OnRunButton(wxCommandEvent& event)
{
	bool	pressed = m_running->GetValue();
	traindir->OnStartStop();
}

//
//
//

void MainFrame::OnOpenFile(wxCommandEvent& event)
{
	traindir->OnOpenFile();
}

#if 01
//
//
//

void MainFrame::OnSpinUp(wxSpinEvent& event)
{
	trainsim_cmd(wxT("fast"));
	event.StopPropagation();
}

//
//
//

void MainFrame::OnSpinDown(wxSpinEvent& event)
{
	trainsim_cmd(wxT("slow"));
	event.StopPropagation();
}
#endif

//	OnSpin()
//
//	The if() below is required because on wxWidgets 2.8 on Linux
//	only the EVT_SPINCTRL() event is sent, thus we
//	have to find out the desired direction (up/down)
//	and whether we reached the upper/lower limit.
//
#if 0
void MainFrame::OnSpin(wxSpinEvent& event)
{
	int x = event.GetPosition();
	if((x == time_mult && time_mult != 1) || x >= time_mult)
	    trainsim_cmd(wxT("fast"));
	else
	    trainsim_cmd(wxT("slow"));
	repaint_labels(true);
	event.Skip();
}
#endif

//
//
//

void	MainFrame::OnRecent(wxCommandEvent& event)
{
	traindir->OnRecent();
}

void	MainFrame::OnRestore(wxCommandEvent& event)
{
	traindir->OnRestore();
}

void	MainFrame::OnSaveGame(wxCommandEvent& event)
{
	traindir->OnSaveGame();
}

void	MainFrame::OnEdit(wxCommandEvent& event)
{
	traindir->OnEdit();
}

void	MainFrame::OnNewTrain(wxCommandEvent& event)
{
	traindir->OnNewTrain();
}

void	MainFrame::OnItinerary(wxCommandEvent& event)
{
	traindir->OnItinerary();
}

void	MainFrame::OnSwitchboard(wxCommandEvent& event)
{
	ShowSwitchboard();
}

void	MainFrame::OnSaveLayout(wxCommandEvent& event)
{
	traindir->OnSaveLayout();
}

void	MainFrame::OnPreferences(wxCommandEvent& event)
{
	traindir->OnPreferences();
}

void	MainFrame::OnEditSkin(wxCommandEvent& event)
{
	TDSkin	*tmpSkin = new TDSkin();

	tmpSkin->name = wxStrdup(curSkin->name);
	tmpSkin->background = curSkin->background;
	tmpSkin->free_track = curSkin->free_track;
	tmpSkin->next = 0;
	tmpSkin->occupied_track = curSkin->occupied_track;
	tmpSkin->outline = curSkin->outline;
	tmpSkin->reserved_shunting = curSkin->reserved_shunting;
	tmpSkin->reserved_track = curSkin->reserved_track;
	tmpSkin->working_track = curSkin->working_track;
	tmpSkin->text = curSkin->text;

	SkinColorsDialog	skin(traindir->m_frame, tmpSkin);

	if(skin.ShowModal() != wxID_OK)
	    return;

	if(curSkin == defaultSkin) {
	    tmpSkin->name = wxStrdup(wxT("Skin1"));
	    tmpSkin->next = skin_list;
	    skin_list = tmpSkin;
	    curSkin = tmpSkin;
	} else {
	    curSkin->background = tmpSkin->background;
	    curSkin->free_track = tmpSkin->free_track;
	    curSkin->occupied_track = tmpSkin->occupied_track;
	    curSkin->outline = tmpSkin->outline;
	    curSkin->reserved_shunting = tmpSkin->reserved_shunting;
	    curSkin->reserved_track = tmpSkin->reserved_track;
	    curSkin->working_track = tmpSkin->working_track;
	    curSkin->text = tmpSkin->text;
	    delete tmpSkin;
	}
}

void	MainFrame::OnNewLayout(wxCommandEvent& event)
{
	traindir->OnNewLayout();
}

void	MainFrame::OnInfo(wxCommandEvent& event)
{
	traindir->OnInfo();
}

void	MainFrame::OnStartStop(wxCommandEvent& event)
{
	traindir->OnStartStop();
}

void	MainFrame::OnGraph(wxCommandEvent& event)
{
	traindir->OnGraph();
}

void	MainFrame::OnLateGraph(wxCommandEvent& event)
{
	ShowLateGraph();
}

void	MainFrame::OnPlatformGraph(wxCommandEvent& event)
{
	ShowPlatformGraph();
}

void	MainFrame::OnRestart(wxCommandEvent& event)
{
	traindir->OnRestart();
}

void	MainFrame::OnFast(wxCommandEvent& event)
{
	traindir->OnFast();
}

void	MainFrame::OnSlow(wxCommandEvent& event)
{
	traindir->OnSlow();
}

void	MainFrame::OnSkip(wxCommandEvent& event)
{
	traindir->OnSkipToNext();
}

void	MainFrame::OnStationSched(wxCommandEvent& event)
{
	traindir->OnStationSched();
}

void	MainFrame::OnSetGreen(wxCommandEvent& event)
{
	traindir->OnSetGreen();
}

void	MainFrame::OnSelectItin(wxCommandEvent& event)
{
	if(!itinKeyDialog)
	    itinKeyDialog = new ItineraryKeyDialog(this);

	itinKeyDialog->ShowModal();
}

void	MainFrame::OnPerformance(wxCommandEvent& event)
{
	traindir->OnPerformance();
}

void	MainFrame::ShowTrainInfoList(Train *trn)
{
	int	pg = m_right->FindPage(L("Train Info"));
	if(pg >= 0)
	    m_right->SetSelection(pg);
	m_trainInfo->Update(trn);
}

void	MainFrame::ShowItinerary(bool show)
{
	if(!show) {
	    m_itineraryView->Show(false);
	    m_left->RemovePage(m_itineraryView);
	    return;
	}
	m_itineraryView->Show(true);
	m_left->AddPage(m_itineraryView, L("Itinerary"), true, -1);
}

void	MainFrame::ShowTools(bool show)
{
	bool	firstTime = false;

	if(!show) {
	    if(!m_toolsView)
		return;
	    m_toolsView->Show(false);
	    m_left->RemovePage(m_toolsView);
	    return;
	}
	if(!m_toolsView) {
	    ToolsView *pView = new ToolsView(m_left);
	    m_toolsView = pView;
	    firstTime = true;
	}
	m_toolsView->Show(true);
	m_left->AddPage(m_toolsView, L("Tools"), true, -1);

	int	pg = m_left->FindPage(L("Tools"));
	if(pg >= 0) {
	    m_left->SetSelection(pg);
	    m_left->Refresh();
	}
	if(firstTime)
	    trainsim_cmd(wxT("selecttool 1,0"));
}

void	MainFrame::ShowHtml(const wxString& name, const wxString& page)
{
	int	pg = m_top->FindPage(name);
	HtmlView *htmlView;

	if(pg < 0) {
	    htmlView = new HtmlView(m_top);
	    m_top->AddPage(htmlView, name, true, -1);
	} else
	    htmlView = (HtmlView *)m_top->GetPage(pg);
	htmlView->SetPage(page);
	pg = m_top->FindPage(name);
	m_top->SetSelection(pg);
}

void	MainFrame::ShowGraph(void)
{
	if(!m_graphView) {
	    GraphView *pView = new GraphView(m_top);
	    m_graphView = pView;
	}
	m_graphView->Show(true);

	int	pg = m_top->FindPage(L("Graph"));
	if(pg >= 0)
	    m_top->SetSelection(pg);
	else
	    m_top->AddPage(m_graphView, L("Graph"), true, -1);
	m_graphView->Refresh();
	m_top->Refresh();
}

void	MainFrame::ShowLateGraph(void)
{
	if(!m_lateGraphView) {
	    LateGraphView *pView = new LateGraphView(m_top);
	    m_lateGraphView = pView;
	}
	m_lateGraphView->Show(true);

	int	pg = m_top->FindPage(L("Late Graph"));
	if(pg >= 0)
	    m_top->SetSelection(pg);
	else
	    m_top->AddPage(m_lateGraphView, L("Late Graph"), true, -1);
	m_lateGraphView->Refresh();
	m_top->Refresh();
}

void	MainFrame::ShowPlatformGraph(void)
{
	if(!m_platformGraphView) {
	    PlatformGraphView *pView = new PlatformGraphView(m_top);
	    m_platformGraphView = pView;
	}
	m_platformGraphView->Show(true);

	int	pg = m_top->FindPage(L("Platform Graph"));
	if(pg >= 0)
	    m_top->SetSelection(pg);
	else
	    m_top->AddPage(m_platformGraphView, L("Platform Graph"), true, -1);
	m_platformGraphView->Refresh();
	m_top->Refresh();
}

void MainFrame::OnChar(wxKeyEvent& event)
{
	if(event.KeyCode() == 9) {
	    return;
	}
	event.Skip();
}


//
//
//

const wxChar	*fileName(const wxChar *p)
{
	const wxChar	*p1 = p + wxStrlen(p);

	while(p1 > p) {
	    if(*p1 == ':' || *p1 == '\\' || *p1 == '/')
		return p1 + 1;
	    --p1;
	}
	return p1;
}

//
//      Update the Window status bar sections (labels).
//      Originally designed to show each piece of info in its own box
//      on the Window status bar. Now all information is shown as a
//      single string, so the loop is probably not necessary anymore.
//

void	repaint_labels(bool force)
{
	int	i;

	for(i = 0; i < 8; ++i)
	    if(//labelList[i].handle &&
		force ||
		wxStrcmp(labelList[i].text, labelList[i].oldtext)) {
		if(i == 7)
		    traindir->m_frame->m_statusText->SetLabel(labelList[i].text);
		else if(i == 2)
		    traindir->m_frame->m_alertText->SetLabel(labelList[i].text);
		else if(i == 0) {
		    wxString	buff = labelList[i].text;
		    size_t	p;

		    p = buff.find(wxT('('));
		    if(p == wxString::npos)
			p = buff.find(wxT('x'));
		    if(p != wxString::npos) {
			wxString buff;
			buff.Printf (wxT("x %d"), time_mult);
			traindir->m_frame->m_speed->SetValue(buff);
			traindir->m_frame->m_speedArrows->SetValue(cur_time_mult);
		    }
		    traindir->m_frame->m_clock->SetLabel(buff.substr (0, p));
		} else if(i < NSTATUSBOXES)
		    traindir->m_frame->SetStatusText(labelList[i].text, i);
		wxStrncpy(labelList[i].oldtext, labelList[i].text,
			sizeof(labelList[i].oldtext) / sizeof(wxChar));
		labelList[i].oldtext[sizeof(labelList[i].oldtext) / sizeof(wxChar)- 1] = 0;
	    }
	wxString    title;
	
	if(traindir->m_frame->m_showToolbar) {
	    title << program_name;
	    title << wxT(" - ");
	    title << fileName(current_project);
	    if(layout_modified)
		title << wxT(" *");
	    title << wxT(" - ");
	    title << labelList[0].text;
	    title << wxT(" - ");
	    title << gRunInfoStr.points_msg;
	} else {
	    title << labelList[0].text;
	    title << wxT(" - ");
	    title << gRunInfoStr.points_msg;
	    title << wxT(" - ");
	    title << labelList[7].text;
	}
	traindir->m_frame->SetTitle(title);
}


void	select_tool(int i)
{
	current_tool = i;
	tools_grid->Clear();
	traindir->m_frame->m_toolsView->Refresh();
}

void	show_table(void)	// originally to show start/stop,fast/slow buttons and labels
{
}

void	hide_table(void)	// originally to hide start/stop,fast/slow buttons and labels
{
}

void	show_tooltable(void)	// show editing tools
{
	traindir->m_frame->ShowTools(true);
}

void	hide_tooltable(void)	// hide editing tools
{
	traindir->m_frame->ShowTools(false);
}

void	FillItineraryTable();

void	show_itinerary(void)	// show itinerary table
{
	traindir->m_frame->ShowItinerary(true);
	FillItineraryTable();
}

void	hide_itinerary(void)	// hide itinerary table
{
	traindir->m_frame->ShowItinerary(false);
}

int	create_tgraph(void)
{
	traindir->m_frame->ShowGraph();
	return 0;
}

int	ask_number(const wxChar *title, const wxChar *question)
{
	wxTextEntryDialog diag(traindir->m_frame, LV(question), LV(title));

	if(diag.ShowModal() != wxID_OK)
	    return -1;
	wxString result = diag.GetValue();
	return wxAtoi(result.c_str());
}

void    alert_dialog(const Char *msg)
{
        wxString message(msg);
        wxMessageBox(message, wxT("Info"));
}
