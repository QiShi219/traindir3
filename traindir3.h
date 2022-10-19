/*	traindir3.h - Created by Giampiero Caprino

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

#ifndef _TRAINDIR3_H
#define _TRAINDIR3_H

#include "wx/wx.h"

#include "defs.h"

#include "trsim.h"
#include "Track.h"

#define	MAX_OLD_SIMULATIONS  16
#define	MAX_SOUNDS  16

class TDFrame;
class MainFrame;

class LogFilter : public wxLog
{
public:
        LogFilter() : m_parent(0), m_oldLog(0) { };
	~LogFilter() { };
	void	SetParent(MainFrame *pParent) { m_parent = pParent; };
	void	InstallLog();
	void	UninstallLog();
	void	DoLog(wxLogLevel level, const wxChar *szString, time_t t);
	void	SetExtraInfo(const wxChar *extra);

	MainFrame   *m_parent;
	wxLog	    *m_oldLog;
	wxString    m_extraInfo;
};

extern	LogFilter   gLogger;

class TDProject
{
public:
	TDProject();
	~TDProject();

	wxString    m_name;

	Track	    *m_layout;
};

extern  int     lastModTime;

#include "Alerts.h"

class Traindir : public wxApp
{
public:
	Traindir() { };
	~Traindir();

	void	GetUserDir(wxString& path);
	void	GetAppDir(wxString& path);

	void	LoadPreferences();
	void	LoadState();
	void	SaveState();

	void	Error(const wxChar *msg);
	void	layout_error(const wxChar *msg);
	void	end_layout_error(void);
	void	AddAlert(const wxChar *msg);
	void	ClearAlert();
	void	PlaySound(const wxChar *path);

	void	OpenFile(const wxChar *path, bool restore = false);
	void	OpenFile(wxString& path, bool restore = false);
	bool	OpenMacroFileDialog(wxChar *buff);
	bool	OpenImageDialog(wxChar *buff);
	bool	OpenScriptDialog(wxChar *buff);
	bool	OpenSelectPowerDialog();
	int	OpenSetTrackLengthDialog();
	bool	SaveTextFileDialog(wxChar *buff);
	bool	SaveHtmlFileDialog(wxChar *buff);
	bool	SavePerfText();
	bool	SaveHtmlPage(HtmlPage& page);

	void	ShowTrainInfo(const wxChar *name);
	void	BuildWelcomePage(HtmlPage& page);

	void	SetTimeSlice(int cnt);
	void	OnTimer(void);

	virtual bool	OnInit();
//	virtual void	OnIdle(wxIdleEvent& event);
//	virtual void	OnTimer(wxTimerEvent& event);
//	virtual void	Cleanup();
	void		OnEndSession(wxCloseEvent& event);
	void		OnQuit(wxCommandEvent& event);

	void	OnOpenFile();
	void	OnRecent();
	void	OnRestore();
	void	OnSaveGame();
	void	OnEdit();
	void	OnNewTrain();
	void	OnItinerary();
	bool	OnSaveLayout();
	void	OnSkipToNext();
	void	OnPreferences();
	void	OnNewLayout();
	void	OnInfo();

	void	OnStartStop();
	void	OnGraph();
	void	OnRestart();
	void	OnFast();
	void	OnSlow();
	void	OnStationSched();
	void	OnSetGreen();
	void	OnPerformance();
	void	ShowStationsList();

	static	void	Panic();

	void	OnFatalException();

	void	server_mode();

	MainFrame	*m_frame;

	TDProject	*m_project;

	int		m_nOldSimulations;
	wxString	m_oldSimulations[MAX_OLD_SIMULATIONS];

	bool		m_ignoreTimer;
	int		m_timeSlice;
	int		m_timeSliceCount;

	// colors for the time table view

	wxColor		m_colorCanceled;
	wxColor		m_colorReady;
	wxColor		m_colorArrived;
	wxColor		m_colorDerailed;
	wxColor		m_colorWaiting;
	wxColor		m_colorRunning;
        wxColor         m_colorStopped;

//	DECLARE_EVENT_TABLE()
};

extern	Traindir    *traindir;	    // the application itself

#endif // _TRAINDIR3_H
