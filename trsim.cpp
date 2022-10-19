/*	trsim.cpp - Created by Giampiero Caprino

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
#include <fstream>
#include <vector>

#if !defined(__unix__) && !defined(__WXMAC__)
#include <malloc.h>
#endif

#include <stdlib.h>
#include "wx/ffile.h"
#include "Html.h"
#include "Traindir3.h"
#include "Itinerary.h"
#include "TDFile.h"
#include "MainFrm.h"
#include "SwitchBoard.h"

using   namespace   std;

const wxChar	*version = wxT("NEU_1.0");  // REMEMBER to enable beep on alert
const wxChar    *timetablePath = "../TD_timetable.trk";  //强化学习交互实验案例在本机路径
const wxChar  *stop_name[30];
const wxChar  *exit_name;
#define PRETIME1 180
#define PRETIME2 0

#if defined(__linux__)
const wxChar	*host = wxT(" Linux");
#elif defined(__WXMAC__)
const wxChar    *host = wxT(" MacOS X");
#elif defined(__FreeBSD__)
const wxChar    *host = wxT(" FreeBSD");
#else
const wxChar	*host = wxT("");
#endif

ShapeManager    shapeManager;

struct _conf conf;

grcolor	color_white;
grcolor	color_black;
grcolor	color_green;
grcolor	color_yellow;
grcolor	color_red;
grcolor	color_orange;
grcolor	color_brown;
grcolor	color_gray;
grcolor	color_lightgray;
grcolor color_darkgray;
grcolor	color_magenta;
grcolor	color_blue;
grcolor	color_cyan;
//grcolor	color_red1;
grcolor	color_bise;//lrg 20210215,显示占用区间颜色
grcolor	color_approach;//lrg 20210220,显示进路颜色



void	(*track_properties_dialog)(Track *);
void    (*signal_properties_dialog)(Signal *);
void    switch_properties_dialog(Track *);
void    (*trigger_properties_dialog)(Track *);
void	(*performance_dialog)(void);
void	(*options_dialog)(void);
void	(*select_day_dialog)(void);
void	(*train_info_dialog)(Train *t);
void	(*assign_dialog)(Train *t);
void	(*station_sched_dialog)(const wxChar *);
void	(*itinerary_dialog)(Itinerary *it);
void	(*about_dialog)(void);

void	performance_toggle_canceled();

extern	void	ShowTrainInfo(Train *trn);
extern	void	save_assign_train(Train *newTrain, Train *oldTrain);
extern	void	leave_track(Train *t);
extern	int	ask_to_save_layout();	// -1 == cancel operation
extern	void	load_puzzles(const wxChar *cmd);
extern	void	show_puzzle();
extern	void	clear_delays();
extern  Vector	*findPath(Track *trk, int dir);
extern  void    insert_file(wxString& str, const Char *fname);


void	SwitchboardEditCommand(const Char *cmd);
void	SwitchboardCellCommand(const Char *cmd);
void	SwitchboardOpenBrowser(const Char *cmd);

extern  void    set_option(Char *id, const Char *sval, int val);

extern  void    bstreet_playing();
extern  volatile bool   server_command_done;

extern	int	ntoolrows;
extern	int	screen_width;
extern  const wxChar	*locale_name;
extern  const Char  *gEditorMotivePower;
extern  Coord   move_start, move_end;

//extern int wxhonoff; // wxh
extern  void loadato();//ypx
extern  int     waitAto;   //等待通信接口接收ato曲线
extern  int  waitAction;//等待接口发送下一步动作（时刻表）
extern int accuracy;
extern int selectline_rang(Track *t); //lrg  
const wxChar *check_name;
extern int tcp_flag;
extern void    impot_curve(Train *t);

etype     errortype = NOERR;
const wxChar  *errorname;
//ypx
void test_send(); //发送UDP请求曲线
void send_DMI(Train *t);
extern std::vector<TrainMessage*> sendMsg;
extern std::vector<Train*> sendDecMsg;
extern std::vector<Train*> sendADMsg;

extern  bool    isInside(const Coord& start, const Coord& end, int x, int y);

void    set_track_lengths(int len);

void    apply_power();
int wxhonoff=1;
int wxhonoff1=0;
vector<int> SpeedLimitStop;
vector<int> SpeedLimitValue;
vector<int> SpeedStart;
vector<int> SpeedEnd;


int FREEFAIL;//  
wxChar  temp_name[15]; 
int	is_windows;
int	time_mults[] = { 1, 2, 3, 5, 7, 10, 15, 20, 30, 60, 120, 240, 300, -1 };
int	cur_time_mult = 5;	/* start with T x 10 */
long	start_time;
BoolOption show_speeds(wxT("viewspeed"), wxT("View speed limits"), wxT("Preferences"), true);
int	signal_traditional = 1;
BoolOption show_blocks(wxT("showblocks"), wxT("View long blocks"), wxT("Preferences"), true);
BoolOption show_icons(wxT("ShowIcons"), wxT("Show trains icons"), wxT("Preferences"), true);
#ifdef WIN32
int	show_tooltip = 1;
#else
int	show_tooltip = 0;
#endif
BoolOption beep_on_alert(wxT("alertsound"), wxT("Play a sound on alerts"), wxT("Preferences"), true);
BoolOption beep_on_enter(wxT("entersound"), wxT("Alert on train entering layout"), wxT("Preferences"), false);
BoolOption show_seconds(wxT("showsecs"), wxT("Show seconds on clock"), wxT("Preferences"), false); // TODO: seems not used
int	hard_counters = 0;
int	platform_schedule;
BoolOption show_canceled(wxT("show_canceled"), wxT("Show canceled trains in timetable"), wxT("Preferences"), true);
BoolOption show_arrived(wxT("show_arrived"), wxT("Show arrived trains in timetable"), wxT("Preferences"), true);
BoolOption alert_last_on_top(wxT("alert_on_top"), wxT("Show last alert message at the top of the view"), wxT("Preferences"), false);
int	showing_graph = 0;	/* windows only */
int	use_real_time = 0;
int	layout_modified = 0;	/* user edited the layout */
int	enable_training = 0;	/* enable signal training menu */
int	random_delays = 1;	/* enable delayed entrances and departures */
int	play_synchronously = 1;	/* stop simulation while playing sounds */
int	swap_head_tail = 0;	/* swap head and tail icons when reversing train */

extern  Array<TrackBase *>  dwellingTracks;

Char	*days_short_names[] = {
	wxT("Mon"),
	wxT("Tue"),
	wxT("Wed"),
	wxT("Thu"),
	wxT("Fri"),
	wxT("Sat"),
	wxT("Sun")
};

wxFFile	flog;
TDFile	*frply;

//Char	entering_time[40];
//wxChar	leaving_time[20];
//wxChar	current_speed[20];
//wxChar	current_delay[20];
//wxChar	current_late[20];
//wxChar	current_status[250];
TrainInfo   train_info;
wxString    current_project;	/* name of files that we loaded */
wxString    info_page;		/* HTML page to show in the Scenario Info window */

/*
const wxChar	*disp_columns[9] = {
	entering_time, wxT(""),
	wxT(""), leaving_time, wxT(""),
	current_speed, current_delay, current_late, current_status
};
*/
//限速值

Track	*layout;
Train	*schedule,*newsch;
Train	*stranded;
Speedlim *speedlim;
TextList *track_info;

Track	*signal_list,
	*track_list,
	*text_list,
	*switch_list;
Track   *track_array[XNCELLS][YNCELLS];

struct tr_rect cliprect;
int	ignore_cliprect;
unsigned char update_map[XNCELLS * YNCELLS];
#if 0
#define	UPDATE_MAP(x, y) (update_map[(y) * XNCELLS + (x)])
#else
#define	UPDATE_MAP(x, y) \
        (((x) < 0 || (x) >= XNCELLS || (y) < 0 || (y) >= YNCELLS) ? (do_alert(wxT("Update out of bounds")), 0) : update_map[(y) * XNCELLS + (x)])
#endif

//struct station_sched *stat_sched;

perf	perf_easy = {		/* performance tracking */
	100,			/* wrong dest */
	10,			/* late trains */
	1,			/* thrown switch */
	1,			/* cleared signal */
	1,			/* command denied */
	0,			/* turned train */
	0,			/* waiting train */
	5,			/* wrong platform */
	0,			/* number of late trains */
	0,			/* number of wrong destinations */
	0,			/* number of missed stops */
	0,			/* wrong rolling stock assignments */
};
perf	perf_hard = {		/* performance tracking */
	100,			/* wrong dest */
	10,			/* late trains */
	1,			/* thrown switch */
	3,			/* cleared signal */
	1,			/* command denied */
	1,			/* turned train */
	1,			/* waiting train */
	5,			/* wrong platform */
	0,			/* number of late trains */
	0,			/* number of wrong destinations */
	0,			/* number of missed stops */
	5			/* wrong rolling stock assignments */
};
perf	perf_vals;		/* currrent performance values */
perf	perf_tot;		/* performace counters */

int	editing;
int	editing_itinerary;
int	running;
int	run_points;
int	total_delay;
int	total_late;
int	time_mult;
long	current_time;
long   last_time = 0;
int	run_day;
int	total_track_number;	/* to prevent endless loops in findPath() */

int	run_point_base = 1;	/* 10 points per second travelled */

wxChar	delay_points_msg[64];
wxChar	time_mult_msg[32];
wxChar	late_points_msg[512];
wxChar	alert_msg[1024];
wxChar	status_line[1024];
wxChar	dummy_line[20];

wxChar	tooltipString[1024];	// 3.4c: tooltip shown on mouse move

RunInfoStr  gRunInfoStr;

TrLabel labelList[] = {
	{ gRunInfoStr.time_msg },
	{ gRunInfoStr.points_msg },
	{ alert_msg },
	{ time_mult_msg },
	{ delay_points_msg },
	{ late_points_msg },
	{ dummy_line },		/* could be used for additional msgs */
	{ status_line },
	{ 0 }
};

struct edittools tooltbl1024[] = {
	{ TEXT, 0, 0, 0 },
	{ TRACK, W_E, 0, 1 },
	{ TRACK, NW_SE, 1, 0 },
	{ TRACK, SW_NE, 1, 1 },
	{ TRACK, W_NE, 2, 0 },
	{ TRACK, W_SE, 2, 1 },
	{ TRACK, NW_E, 3, 0 },
	{ TRACK, SW_E, 3, 1 },
	{ TRACK, XH_NW_SE, 23, 0 },
	{ TRACK, XH_SW_NE, 23, 1 },
	{ TRACK, X_X, 24, 0 },
	{ TRACK, X_PLUS, 24, 1 },
	{ SWITCH, 0, 4, 0 },
	{ SWITCH, 1, 4, 1 },
	{ SWITCH, 2, 5, 0 },
	{ SWITCH, 3, 5, 1 },
	{ SWITCH, 4, 6, 0 },
	{ SWITCH, 5, 6, 1 },
	{ SWITCH, 10, 7, 0 },
	{ SWITCH, 11, 7, 1 },
	{ SWITCH, 6, 8, 0 },
	{ SWITCH, 7, 8, 1 },
	{ SWITCH, 8, 9, 0 },
	{ SWITCH, 9, 9, 1 },

	{ SWITCH, 12, 10, 0 },	    /* vertical switches */
	{ SWITCH, 13, 10, 1 },
	{ SWITCH, 14, 11, 0 },
	{ SWITCH, 15, 11, 1 },
	{ TRACK, NW_S, 12, 0 },
	{ TRACK, SW_N, 12, 1 },
	{ TRACK, NE_S, 13, 0 },
	{ TRACK, SE_N, 13, 1 },
	{ TRACK, TRK_N_S, 14, 0 },
	{ ITIN, 0, 14, 1 },
	{ IMAGE, 1, 15, 0 },
	{ PLATFORM, 1, 15, 1 },
	{ TSIGNAL, 0, 16, 0 },
	{ TSIGNAL, 1, 16, 1 },
	{ TSIGNAL, 2, 17, 0 },
	{ TSIGNAL, 3, 17, 1 },
	{ TSIGNAL, S_N, 18, 0 },
	{ TSIGNAL, N_S, 18, 1 },
	{ TSIGNAL, signal_NORTH_FLEETED, 19, 0 },
	{ TSIGNAL, signal_SOUTH_FLEETED, 19, 1 },
	{ TEXT, 0, 20, 0 },
	{ TEXT, 1, 20, 1 },
	{ LINK, 0, 21, 0 },
	{ LINK, 1, 21, 1 },
	{ MACRO, 0, 22, 0 },
	{ MACRO, 1, 22, 1 },
	{ TRIGGER, W_E, 25, 0 },
	{ TRIGGER, E_W, 25, 1 },
	{ TRIGGER, N_S, 26, 0 },
	{ TRIGGER, S_N, 26, 1 },
	{ -1 }
};

struct edittools tooltbl800[] = {   /* used when screen is 800x600 */
	{ TEXT, 0, 0, 0 },
	{ TRACK, W_E, 0, 1 },
	{ TRACK, NW_SE, 1, 0 },
	{ TRACK, SW_NE, 1, 1 },
	{ TRACK, W_NE, 2, 0 },
	{ TRACK, W_SE, 2, 1 },
	{ TRACK, NW_E, 3, 0 },
	{ TRACK, SW_E, 3, 1 },
	{ TRACK, XH_NW_SE, 15, 0 },
	{ TRACK, XH_SW_NE, 15, 1 },
	{ TRACK, X_X, 16, 0 },
	{ TRACK, X_PLUS, 16, 1 },
	{ SWITCH, 0, 4, 0 },
	{ SWITCH, 1, 4, 1 },
	{ SWITCH, 2, 5, 0 },
	{ SWITCH, 3, 5, 1 },
	{ SWITCH, 4, 6, 0 },
	{ SWITCH, 5, 6, 1 },
	{ SWITCH, 10, 7, 0 },
	{ SWITCH, 11, 7, 1 },
	{ SWITCH, 6, 8, 0 },
	{ SWITCH, 7, 8, 1 },
	{ SWITCH, 8, 9, 0 },
	{ SWITCH, 9, 9, 1 },

	{ SWITCH, 12, 10, 0 },	    /* vertical switches */
	{ SWITCH, 13, 10, 1 },
	{ SWITCH, 14, 11, 0 },
	{ SWITCH, 15, 11, 1 },
	{ TRACK, NW_S, 12, 0 },
	{ TRACK, SW_N, 12, 1 },
	{ TRACK, NE_S, 13, 0 },
	{ TRACK, SE_N, 13, 1 },
	{ TRACK, TRK_N_S, 14, 0 },
	{ ITIN, 0, 14, 1 },
	{ IMAGE, 1, 15, 2 },
	{ PLATFORM, 1, 0, 2 },
	{ TSIGNAL, 0, 1, 2 },
	{ TSIGNAL, 1, 2, 2 },
	{ TSIGNAL, 2, 3, 2 },
	{ TSIGNAL, 3, 4, 2 },
	{ TSIGNAL, S_N, 5, 2 },
	{ TSIGNAL, N_S, 6, 2 },
	{ TSIGNAL, signal_NORTH_FLEETED, 7, 2 },
	{ TSIGNAL, signal_SOUTH_FLEETED, 8, 2 },
	{ TEXT, 0, 9, 2 },
	{ TEXT, 1, 10, 2 },
	{ LINK, 0, 11, 2 },
	{ LINK, 1, 12, 2 },
	{ MACRO, 0, 13, 2 },
	{ MACRO, 1, 14, 2 },
	{ TRIGGER, W_E, 17, 0 },
	{ TRIGGER, E_W, 17, 1 },
	{ TRIGGER, N_S, 18, 0 },
	{ TRIGGER, S_N, 18, 1 },
	{ -1 }
};
struct edittools tooltbltracks[] = {   /* used when screen is 800x600 */
	{ TEXT, 0, 0, 0 },
	{ TRACK, TRK_N_S, 0, 1 },
	{ TRACK, W_E, 1, 1 },
	{ TRACK, NW_SE, 2, 1 },
	{ TRACK, SW_NE, 3, 1 },
	{ TRACK, W_NE, 4, 1 },
	{ TRACK, W_SE, 5, 1 },
	{ TRACK, NW_E, 6, 1 },
	{ TRACK, SW_E, 7, 1 },
	{ TRACK, NW_S, 8, 1 },
	{ TRACK, SW_N, 9, 1 },
	{ TRACK, NE_S, 10, 1 },
	{ TRACK, SE_N, 11, 1 },
	{ TRACK, XH_NW_SE, 12, 1 },
	{ TRACK, XH_SW_NE, 13, 1 },
	{ TRACK, X_X, 14, 1 },
	{ TRACK, X_PLUS, 15, 1 },
	{ TRACK, N_NE_S_SW, 16, 1 },	// no switch  / |
	{ TRACK, N_NW_S_SE, 17, 1 },	// no switch  \ |
	{ -1 }
};
struct edittools tooltblswitches[] = {
	{ TEXT, 0, 0, 0 },
	{ SWITCH, 0, 0, 1 },
	{ SWITCH, 1, 1, 1 },
	{ SWITCH, 2, 2, 1 },
	{ SWITCH, 3, 3, 1 },
	{ SWITCH, 4, 4, 1 },
	{ SWITCH, 5, 5, 1 },
	{ SWITCH, 6, 6, 1 },
	{ SWITCH, 7, 7, 1 },
	{ SWITCH, 8, 8, 1 },
	{ SWITCH, 9, 9, 1 },
	{ SWITCH, 10, 10, 1 },
	{ SWITCH, 11, 11, 1 },

	{ SWITCH, 12, 12, 1 },	    /* vertical switches */
	{ SWITCH, 13, 13, 1 },
	{ SWITCH, 14, 14, 1 },
	{ SWITCH, 15, 15, 1 },
	{ SWITCH, 16, 16, 1 },
	{ SWITCH, 17, 17, 1 },
	{ SWITCH, 18, 18, 1 },
	{ SWITCH, 19, 19, 1 },
	{ SWITCH, 20, 20, 1 },
	{ SWITCH, 21, 21, 1 },
	{ SWITCH, 22, 22, 1 },
	{ SWITCH, 23, 23, 1 },
	{ -1 }
};
struct edittools tooltblsignals[] = {
	{ TEXT, 0, 0, 0 },
	{ TSIGNAL, 0, 0, 1 },
	{ TSIGNAL, 1, 1, 1 },
	{ TSIGNAL, 2, 2, 1 },
	{ TSIGNAL, 3, 3, 1 },
	{ TSIGNAL, S_N, 4, 1 },
	{ TSIGNAL, N_S, 5, 1 },
	{ TSIGNAL, signal_NORTH_FLEETED, 6, 1 },
	{ TSIGNAL, signal_SOUTH_FLEETED, 7, 1 },
	{ -1 }
};
struct edittools tooltblmisc[] = {
	{ TEXT, 0, 0, 0 },
	{ TEXT, 0, 0, 1 },
	{ TEXT, 1, 1, 1 },
	{ ITIN, 0, 2, 1 },
	{ ITIN, 1, 3, 1 },
	{ IMAGE, 1, 4, 1 },
	{ PLATFORM, 1, 5, 1 },
	{ -1 }
};
struct edittools tooltblactions[] = {
	{ TEXT, 0, 0, 0 },
	{ LINK, 0, 0, 1 },
	{ LINK, 1, 1, 1 },
	{ MACRO, 0, 2, 1 },
	{ MACRO, 1, 3, 1 },
	{ TRIGGER, W_E, 4, 1 },
	{ TRIGGER, E_W, 5, 1 },
	{ TRIGGER, N_S, 6, 1 },
	{ TRIGGER, S_N, 7, 1 },
	{ MOVER, 0, 8, 1 },
	{ MOVER, 1, 9, 1 },
	{ MOVER, 2, 10, 1 },
	{ POWERTOOL, 0, 11, 1 },
	{ TRACKLENTOOL, 0, 12, 1 },
	{ -1 }
};

int	current_toolset;
int	current_tool;
struct edittools *tooltbl = tooltbltracks /* tooltbl800 */;
Track	*tool_layout;
Track	*tool_tracks, *tool_signals, *tool_switches, *tool_misc, *tool_actions;

const wxChar	*en_station_titles[] = { wxT("Train"), wxT("Arrival"), wxT("From"), wxT("Departure"),
				wxT("To&nbsp;&nbsp;&nbsp;"), wxT("Runs&nbsp;on&nbsp;&nbsp;"),
				wxT("Platform"), wxT("Notes"),
				NULL };
const wxChar	*station_titles[9];

int	ntrains_arrived;
int     ntrains_starting;
int	ntrains_running;
int	ntrains_waiting;
int	ntrains_stopped;
int	ntrains_ready;

int	sameStation_s1(const wxChar *s1, const wxChar *s2)  //对比变量1在@之前的字符是否和s2相同
{
	while(*s1 && *s1 != '@' && *s1 == *s2)
	    ++s1, ++s2;
	if(!*s1 || *s1 == '@')
		return 1;
	return 0;
}

void	itinerary_cmd();

const wxChar    *skip_blanks(const wxChar *p)
{
        while(*p == wxT(' ') || *p == wxT('\t'))
            ++p;
        return p;
}

void	remove_ext(wxChar *buff)
{
	wxChar	*p;

	/* remove extension. Will be added back by open cmd */
	for(p = buff + wxStrlen(buff); *p != ' ' && *p != '/' &&
	    *p != '\\' && *p != '.'; --p);
	if(*p == '.')
	    *p = 0;
}

wxChar	*format_time(long tim)
{
	static	wxChar	buff[64];

	// !Rask Ingemann Lambersten - added seconds
	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%3d:%02d:%02d "), (tim / 3600) % 24, (tim / 60) % 60, tim % 60);
	return(buff);
}

int	parse_time(wxChar **pp)
{
	wxChar	*p = *pp;
	int	v = 0, v1 = 0, v2 = 0;

	while(*p == ' ') ++p;
	if(*p)
	    v = *p++ - '0';
	if(*p != ':')
	    v = v * 10 + (*p++ - '0');
	if(*p == ':')
	    ++p;
	if(*p)
	    v1 = *p++ - '0';
	if(*p >= '0' && *p <= '9')
	    v1 = v1 * 10 + (*p++ - '0');
	if(*p == ':') {	    // +Rask Ingemann Lambersten
	    ++p;
	    if(*p >= '0' && *p <= '9')
		v2 = *p++ - '0';
	    if(*p >= '0' && *p <= '9')
		v2 = v2 * 10 + (*p++ - '0');
	}		    // +Rask Ingemann Lambersten
	*pp = p;
	return v * 3600 + v1 * 60 + v2;
}

wxChar	*parse_km(Track *t, const wxChar *p)
{
	wxChar	*pp;

	t->km = wxStrtol(p, &pp, 10) * 1000;
	if(*pp == '.')
	    t->km += wxStrtol(pp + 1, &pp, 10) % 1000;
	return pp;
}

bool    parseCoords(const Char** pcmd, Coord& pos)
{
        const Char *cmd = *pcmd;
        while(*cmd == ' ' || *cmd == '\t')
            ++cmd;
        *pcmd = cmd;
        if(*cmd >= '0' && *cmd <= '9') {
	    wxChar *end;
	    pos.x = wxStrtol(cmd, &end, 10);
	    if(isalpha(*end))
	        return false;
	    if(*end == ',') ++end;
	    pos.y = wxStrtol(end, &end, 10);
            *pcmd = end;
            return true;
        }
        return false;
}

void	compute_train_numbers(void)
{
	Train	*t;

	ntrains_arrived = 0;
	ntrains_waiting = 0;
	ntrains_stopped = 0;
	ntrains_ready = 0;
        ntrains_starting = 0;
	ntrains_running = 0;
	for(t = schedule; t; t = t->next) {
	    switch(t->status) {
	    case train_READY:
		if(!run_day || (t->days & run_day))
		    ++ntrains_ready;
		break;
            case train_STARTING:
                ++ntrains_starting;
                break;
	    case train_RUNNING:
		++ntrains_running;
		break;
	    case train_WAITING:
		++ntrains_waiting;
		break;
	    case train_STOPPED:
		++ntrains_stopped;
		break;
	    case train_ARRIVED:
		++ntrains_arrived;
	    }
	}
}

long	performance(void)
{
	long	tot;

	tot = perf_tot.wrong_dest * perf_vals.wrong_dest;
	tot += perf_tot.late_trains * perf_vals.late_trains;
	tot += perf_tot.thrown_switch * perf_vals.thrown_switch;
	tot += perf_tot.cleared_signal * perf_vals.cleared_signal;
	tot += perf_tot.turned_train * perf_vals.turned_train;
	tot += perf_tot.waiting_train * perf_vals.waiting_train;
	tot += perf_tot.wrong_platform * perf_vals.wrong_platform;
	tot += perf_tot.denied * perf_vals.denied;
	return tot;
}

void    printRunInfo(RunInfoStr& out)
{
	wxStrcpy(out.time_msg, wxT("   "));
	if(show_seconds._iValue)
	    wxSnprintf(out.time_msg + 3, sizeof(out.time_msg)/sizeof(Char) - 3, wxT("%3ld:%02ld.%02ld "), (current_time / 3600) % 24,
					(current_time / 60) % 60,
					current_time % 60);
	else
	    wxStrcpy(out.time_msg + 3, format_time(current_time));

	// show name of current day, if any
	if(run_day) {
	    int	i;
	    for(i = 0; i < 7 && !(run_day & (1 << i)); ++i);
	    if(i < 7) {
	    	wxSnprintf(out.time_msg + wxStrlen(out.time_msg), sizeof(out.time_msg)/sizeof(Char) - wxStrlen(out.time_msg),
		    wxT(" (%s) "), days_short_names[i]);
	    }
	}
	wxSnprintf(out.time_msg + wxStrlen(out.time_msg), sizeof(out.time_msg)/sizeof(wxChar) - wxStrlen(out.time_msg), wxT("   x%d    "), time_mult);
	wxSnprintf(out.time_msg + wxStrlen(out.time_msg), sizeof(out.time_msg)/sizeof(wxChar) - wxStrlen(out.time_msg), wxT("R %d/S %d/r %d/w %d/s %d/a %d"),
			ntrains_running, ntrains_starting, ntrains_ready, ntrains_waiting,
			ntrains_stopped, ntrains_arrived);
/*	wxSnprintf(time_mult_msg, sizeof(time_mult_msg)/sizeof(wxChar),    wxT("   Time multiplier: %4ld"), time_mult);
	wxSnprintf(gRunInfoStr.points_msg, sizeof(total_points_msg)/sizeof(wxChar), wxT("Performance    : -%4ld"), performance());
	wxSnprintf(delay_points_msg, sizeof(delay_points_msg)/sizeof(wxChar), wxT("Delay minutes  : %4ld"), total_delay / 60);
	wxSnprintf(late_points_msg, sizeof(late_points_msg)/sizeof(wxChar),  wxT("Late arrivals  : %4ld min"), total_late); */
	wxSnprintf(out.points_msg, sizeof(out.points_msg)/sizeof(Char), wxT("Pt:%4ld, Del:%4ld, Late:%4ld"),
			    -performance(), total_delay / 60, total_late);
}

void	update_labels(void)
{
        printRunInfo(gRunInfoStr);
	repaint_labels();
}

void	print_train_info(Train *t)
{
        t->Get(train_info);
}

void	invalidate_field(void)	/* next time, repaint whole field */
{
	cliprect.top = 0;
	cliprect.left = 0;
	cliprect.bottom = YNCELLS;
	cliprect.right = XNCELLS;
	ignore_cliprect = 1;
}

void	reset_clip_rect(void)	/* next time, don't paint anything */
{
	cliprect.top = YNCELLS;
	cliprect.bottom = 0;
	cliprect.left = XNCELLS;
	cliprect.right = 0;
	ignore_cliprect = 0;
	memset(update_map, 0, sizeof(update_map));
}

void	change_coord(int x, int y, int w, int h)/* next time, paint within clip rectangle */
{
	int	i;

        if(x < 0) x = 0;
        if(y < 0) y = 0;
        if(x + w >= XNCELLS)
            w = XNCELLS - x;
        if(y + h >= YNCELLS)
            h = YNCELLS - y;
	if(x < cliprect.left)
	    cliprect.left = x;
	if(x + w > cliprect.right)
	    cliprect.right = x + w;
	if(y < cliprect.top)
	    cliprect.top = y;
	if(y + h > cliprect.bottom)
	    cliprect.bottom = y + h;
	while(h-- >= 0) {
	    for(i = 0; i <= w; ++i) {
#if 1
                if((x + i) < 0 || (x + i) >= XNCELLS || y < 0 || y >= YNCELLS)
                    do_alert(wxT("coords out of bounds2"));
                else
                    update_map[y * XNCELLS + (x + i)] = 1;
#else
		UPDATE_MAP(x + i, y) = 1;
#endif
	    }
	    ++y;
	}
}

Track	*init_tool_from_array(struct edittools *tbl)
{
	int	i;
	Track	*t;
	Track	*lst;

	lst = NULL;
	for(i = 0; tbl[i].type != -1; ++i) {
	    t = track_new();
	    tbl[i].trk = t;
	    t->x = tbl[i].x;
	    t->y = tbl[i].y;
	    t->type = (trktype)tbl[i].type;
	    t->direction = (trkdir)tbl[i].direction;
	    t->norect = 1;
	    t->next = lst;
	    if(t->type == TEXT)
		t->station = wxStrdup(i == 0 ? wxT("Del") : wxT("Abc"));
	    else if(t->type == ITIN)
		t->station = wxStrdup(wxT("A"));
	    else if(t->type == TSIGNAL && (t->direction & 2)) {
		t->fleeted = 1;
		t->direction = (trkdir)((int)t->direction & (~2));
	    }
	    lst = t;
	}
	return lst;
}

void	init_tool_layout(void)
{
	tool_layout = init_tool_from_array(tooltbl);	/* old way */
	tool_tracks = init_tool_from_array(tooltbltracks);/* new way */
	tool_switches = init_tool_from_array(tooltblswitches);
	tool_signals = init_tool_from_array(tooltblsignals);
	tool_misc = init_tool_from_array(tooltblmisc);
	tool_actions = init_tool_from_array(tooltblactions);
}

void	free_tool_list(Track *t)
{
	Track	*nxt;

	while(t) {
	    nxt = t->next;
	    if(t->station)
		free(t->station);
	    free(t);
	    t = nxt;
	}
}

void	free_tool_layout(void)
{
	free_tool_list(tool_layout);
	free_tool_list(tool_tracks);
	free_tool_list(tool_switches);
	free_tool_list(tool_signals);
	free_tool_list(tool_misc);
	free_tool_list(tool_actions);
}

void	tool_selected(int x, int y)
{
	int	i;

        if(y == 0) {
	    switch(x) {
	    case 1:
		current_toolset = x;
		tooltbl = tooltbltracks;
		select_tool(0);
		return;

	    case 2:
		current_toolset = x;
		tooltbl = tooltblswitches;
		select_tool(0);
		return;

	    case 3:
		current_toolset = x;
		tooltbl = tooltblsignals;
		select_tool(0);
		return;

	    case 4:
		current_toolset = x;
		tooltbl = tooltblmisc;
		select_tool(0);
		return;

	    case 5:
		current_toolset = x;
		tooltbl = tooltblactions;
		select_tool(0);
		return;
	    }
	}
	for(i = 0; tooltbl[i].type != -1; ++i)
	    if(tooltbl[i].x == x && tooltbl[i].y == y) {
		break;
	    }
	if(tooltbl[i].type == -1)
	    return;
	if(tooltbl[i].type == MACRO) {
	    if(!macro_select())
		return;
	    ++i;	    // automatically select "Place"
	}
        if(tooltbl[i].type == POWERTOOL) {
            traindir->OpenSelectPowerDialog();
            if(current_tool != -1 && tooltbl[current_tool].type == MOVER &&
	        tooltbl[current_tool].direction == 2) {
                // we have a rectangle active
                // we want to apply the power to the tracks in the rectangle
                apply_power();
            }
            return;
        }
        if(tooltbl[i].type == TRACKLENTOOL) {
            if(current_tool != -1 && tooltbl[current_tool].type == MOVER &&
	        tooltbl[current_tool].direction == 2) {
                int len = traindir->OpenSetTrackLengthDialog();
                if(len < 0)
                    return;
                set_track_lengths(len);
                return;
            }
            return;
        }
	select_tool(i);
}

int	track_updated(Track *trk)
{
	if(trk->x < (cliprect.left - 1) || trk->x > cliprect.right)
	    return 0;
	if(trk->y < (cliprect.top - 1) || trk->y > cliprect.bottom)
	    return 0;
	/* it's inside the clip rect, but do we really need to update it? */
#if 0
	int	i;
	int	j;

	for(j = 0; j < 2; ++j)
	    for(i = 0; i < 3; ++i)
		if(UPDATE_MAP(trk->x, trk->y))
		    return 1;
#endif
	if(ignore_cliprect || UPDATE_MAP(trk->x, trk->y))
	    return 1;
	return 0;
}

void	layout_paint(Track *lst)
{
	Track	*trk;
	int	x, y;

        if(!ignore_cliprect) {
#if 01
            if(cliprect.top < 0)
                cliprect.top = 0;
            if(cliprect.top >= YNCELLS)
                cliprect.top = YNCELLS - 1;
            if(cliprect.left < 0)
                cliprect.left = 0;
            if(cliprect.left >= XNCELLS)
                cliprect.left = XNCELLS - 1;
            if(cliprect.bottom < 0)
                cliprect.bottom = 0;
            if(cliprect.bottom >= YNCELLS)
                cliprect.bottom = YNCELLS - 1;
            if(cliprect.right < 0)
                cliprect.right = 0;
            if(cliprect.right >= XNCELLS)
                cliprect.right = XNCELLS - 1;
#if 0
            if(!editing)
                /*&&
                (cliprect.top < 0 || cliprect.top >= YNCELLS ||
                cliprect.bottom < 0 || cliprect.bottom >= YNCELLS ||
                cliprect.left < 0 || cliprect.left >= XNCELLS ||
                cliprect.right < 0 || cliprect.right >= XNCELLS))
                */
            {
                trk = 0;
                return;
            }
#endif
#endif
	    for(y = cliprect.top; y <= cliprect.bottom; ++y)
		for(x = cliprect.left; x <= cliprect.right; ++x)
		    if(UPDATE_MAP(x, y))
			tr_fillrect(x, y);
        }

	for(trk = lst; trk; trk = trk->next)
	    if(editing || track_updated(trk)) {
#if 1
                if(trk->x < 0 || trk->x >= XNCELLS || trk->y < 0 || trk->y >= YNCELLS)
                    do_alert(wxT("coords out of bounds1"));
                else
                    update_map[trk->y * XNCELLS + trk->x] = 0;
#else
		UPDATE_MAP(trk->x, trk->y) = 0;
#endif
		track_paint(trk);
	    }
}

void	trains_paint(Train *trn)
{
	for(; trn; trn = trn->next) {
	    if(trn->position) {
                if(!show_icons._iValue) {
		    int tmp = trn->position->fgcolor;
		    trn->position->fgcolor = color_orange;
		    track_paint(trn->position);
		    trn->position->fgcolor = tmp;
		    continue;
		} else if(trn->flags & TFLG_STRANDED) {
		    if(findTrain(trn->position->x, trn->position->y))
			continue;
		    car_draw(trn->position, trn);
		} else
		    train_draw(trn->position, trn);
	    }
	    if(trn->tail && trn->tail->position &&
		    trn->tail->position != trn->position)
		car_draw(trn->tail->position, trn);
	}
}

//      called when restarting the simulation
void    resetDwellingTracks()
{
        Track   *t;

        dwellingTracks.Clear();
        for(t = layout; t; t = t->next) {
            t->_leftDwellTime = 0;
        }
}

void    fillDwellingTracks()
{
        Track   *t;

        dwellingTracks.Clear();
        for(t = layout; t; t = t->next) {
            if(t->_leftDwellTime > 0)
                dwellingTracks.Add(t);
        }
}

void	link_all_tracks(Track *layout)
{
	Track	*t, *l;

	for(t = layout; t; t = t->next)
            if(t->x >= 0 && t->x < XNCELLS && t->y >= 0 && t->y < YNCELLS)
                track_array[t->x][t->y] = t;
	l = 0;
	for(t = layout; t; t = t->next)
	    if(t->type == TRACK) {
		t->next1 = l;
		l = t;
	    }
	track_list = l;
	l = 0;
	for(t = layout; t; t = t->next)
	    if(t->type == TSIGNAL) {
		t->next1 = l;
		l = t;
	    }
	signal_list = l;
	l = 0;
	for(t = layout; t; t = t->next)
	    if(t->type == SWITCH) {
		t->next1 = l;
		l = t;
	    }
	switch_list = l;
	l = 0;
	for(t = layout; t; t = t->next)
	    if(t->type == TEXT) {
		t->next1 = l;
		l = t;
	    }
	text_list = l;
}

void	link_all_tracks()
{
        memset(track_array, 0, sizeof(track_array));
	link_all_tracks(layout);
}

void	init_sim(void)
{
	if(!tool_layout)
	    init_tool_layout();
	time_mult = 10;
	cur_time_mult = 5;
	run_points = 0;
	total_delay = 0;
	total_late = 0;
	memset(late_data, 0, sizeof(late_data));
	alert_msg[0] = 0;
	wxChar *p;
	int i;
	for(i = 0; i < 7; ++i) {
	    p = days_short_names[i];
	    days_short_names[i] = (Char *)LV(p);
	}
}

void	trainsim_init(void)
{
	Track	*t;
	Train	*trn;

	ntoolrows = 3;
	tooltbl = tooltbl800;
	if(!tool_layout)
	    init_tool_layout();
	conf.fgcolor = fieldcolors[COL_TRACK];
	conf.linkcolor = color_red;
	conf.linkcolor2 = color_blue;
	current_time = start_time;
    last_time = 0;
	run_points = 0;
	total_delay = 0;
	total_late = 0;
	time_mult = 10;
	cur_time_mult = 5;
	alert_msg[0] = 0;
	waitAto = 0;
	waitAction = 0;
	tcp_flag = 0;
	perf_vals = hard_counters ? perf_hard : perf_easy;
	memset(&perf_tot, 0, sizeof(perf_tot));
	link_all_tracks();
	total_track_number = 0;
	for(t = track_list; t; t = t->next1)
	    ++total_track_number;
	showing_graph = 0;
	reset_schedule();
	/*
	while((trn = stranded)) {
	    stranded = trn->next;
	    leave_track(trn);
	    delete trn;
	} */
	trn = schedule;
	schedule = stranded;
	stranded = 0;
	reset_schedule();
	schedule = trn;
	fill_schedule(schedule, 0);
	compute_train_numbers();
	update_labels();
}

void	trainsim_inits(void)
{
	//Track	*t;
	//Train	*trn;

	/*ntoolrows = 3;
	tooltbl = tooltbl800;
	if(!tool_layout)
	    init_tool_layout();
	conf.fgcolor = fieldcolors[COL_TRACK];
	conf.linkcolor = color_red;
	conf.linkcolor2 = color_blue;
	//current_time = start_time;
	run_points = 0;
	total_delay = 0;
	total_late = 0;
	time_mult = 10;
	cur_time_mult = 5;
	alert_msg[0] = 0;
	perf_vals = hard_counters ? perf_hard : perf_easy;
	memset(&perf_tot, 0, sizeof(perf_tot));
	link_all_tracks();
	total_track_number = 0;
	for(t = track_list; t; t = t->next1)
	    ++total_track_number;
	showing_graph = 0;*/
	//reset_schedule();
	/*
	while((trn = stranded)) {
	    stranded = trn->next;
	    leave_track(trn);
	    delete trn;
	} */
	//trn = schedule;
	//schedule = stranded;
	//stranded = 0;
	//reset_schedule();
	//schedule = trn;
	fill_schedule(schedule, 0);
	//compute_train_numbers();
	//update_labels();
}

void	init_all(void)
{
	while(layout)
	    track_delete(layout);
        onIconUpdateListeners.Clear();
//	if(script_text)
//	    free(script_text);
//	script_text = 0;
	clean_trains(schedule);
	schedule = 0;
	clean_trains(stranded);
	stranded = 0;
	start_time = 0;
	trainsim_init();
	invalidate_field();
	repaint_all();
}

static	Track	*find_in_list(Track *t, int x, int y)
{
	for(; t; t = t->next1)
	    if(t->x == x && t->y == y)
		return t;
	return 0;
}

Track   *findTrackFast(int x, int y, trktype type)
{
        Track   *t;

        if(x >= 0 && x < XNCELLS && y >= 0 && y < YNCELLS) {
            t = track_array[x][y];
            if(t && t->type == type)
                return t;
        }
        return 0;
}

Track	*findTrackType(int x, int y, trktype type)
{
	Track	*t;

        t = findTrackFast(x, y, type);
        return t;
#if 0
	switch(type) {
	case TRACK:
            t = findTrackFast(x, y, type);
            if(t)
                return t;
	    return find_in_list(track_list, x, y);
	case TSIGNAL:
            t = findTrackFast(x, y, type);
            if(t)
                return t;
	    return find_in_list(signal_list, x, y);
	case SWITCH:
            t = findTrackFast(x, y, type);
            if(t)
                return t;
	    return find_in_list(switch_list, x, y);
	case TEXT:
            t = findTrackFast(x, y, type);
            if(t)
                return t;
	    return find_in_list(text_list, x, y);
	}
	for(t = layout; t; t = t->next)
	    if(t->x == x && t->y == y && t->type == type)
		return t;
	return 0;
#endif
}

Track	*findLinkTo(int x, int y)
{
	Track	*t;

	for(t = layout; t; t = t->next)
	    if(t->type == TEXT) {
		if(t->wlinkx == x && t->wlinky == y)
		    return t;
		if(t->elinkx == x && t->elinky == y)
		    return t;
	    }
	return 0;
}

Track	*findTriggerTo(int x, int y)
{
	Track	*t;

	for(t = layout; t; t = t->next)
	    if(t->type == TRIGGER) {
		if(t->wlinkx == x && t->wlinky == y)
		    return t;
		if(t->elinkx == x && t->elinky == y)
		    return t;
	    }
	return 0;
}

Track	*findStationNamed(const wxChar *name)
{
	Track	*t;
	const wxChar	*p;
	int	l;

	l = wxStrlen(name);
	if((p = wxStrchr(name, PLATFORM_SEP)))
	    l = p - name;
	for(t = layout; t; t = t->next) {
	    if(!t->station)
		continue;
	    if(t->type == TRACK && t->isstation && !wxStrncmp(name, t->station, l)) {
		if(!t->station[l] || t->station[l] == PLATFORM_SEP)
		    return t;
	    }
	    if(t->type == TEXT && !wxStrcmp(name, t->station) &&
		    ((t->wlinkx && t->wlinky) || (t->elinkx && t->elinky)))
		return t;
	    if(t->type == SWITCH && !wxStrcmp(name, t->station))
		return t;
	}
	return 0;
}

Track	*findStation(const wxChar *name)
{
	Track	*t, *l;

	for(t = layout; t; t = t->next) {
	    if(t->type == TRACK && t->isstation)
		if(!wxStrcmp(name, t->station))
		    return t;
	    if(t->type == TEXT && !wxStrcmp(name, t->station) &&
		    ((t->wlinkx && t->wlinky) || (t->elinkx && t->elinky)))
		return t;
	    l = t;
	}
	return 0;
}

Signal	*findSignalNamed(const wxChar *name)
{
	Track	*t;

	for(t = layout; t; t = t->next)
	    if(t->type == TSIGNAL && t->station && !wxStrcmp(name, t->station))
		return (Signal *)t;
	return 0;
}

Track	*findItineraryNamed(const wxChar *name)
{
	Track	*t;

	for(t = layout; t; t = t->next)
	    if(t->type == ITIN && t->station && !wxStrcmp(name, t->station))
		return t;
	return 0;
}

TrainStop *findStop(Train *trn, Track *trk)
{
	TrainStop *stp;

	if(!trk || !trk->station)
	    return 0;
	for(stp = trn->stops; stp; stp = stp->next)
	    if(sameStation(stp->station, trk->station))
		break;
	return stp;
}

Train	*findTrain(int x, int y)
{
	Train	*tr;

	for(tr = schedule; tr; tr = tr->next)
	    if(tr->position)
                if(tr->position->x == x && tr->position->y == y)
		    return tr;
	return 0;
}

Train	*findTrainNamed(const wxChar *name)
{
	Train	*t;

	for(t = schedule; t; t = t->next)
	    if(!wxStrcmp(name, t->name))
		return t;
	return 0;
}

Train   *findTrainAtCoord(const Char *pos)
{
        Train *t;
        int x, y;
        Char *npos;

        x = wxStrtoul(pos, &npos, 10);
        if (*npos == ',') ++npos;
        y = wxStrtoul(npos, &npos, 10);
	for(t = schedule; t; t = t->next)
	    if(t->position && t->position->x == x && t->position->y == y) // TODO: handle tail
		return t;
        return 0;
}

Train	*findTail(int x, int y)
{
	Train	*tr;

	for(tr = schedule; tr; tr = tr->next)
	    if(tr->tail && tr->tail->position &&
		    tr->tail->position->x == x && tr->tail->position->y == y)
		return tr;
	return 0;
}

Train	*findStranded(int x, int y)
{
	Train	*tr;

	for(tr = stranded; tr; tr = tr->next)
	    if(tr->position && tr->position->x == x && tr->position->y == y)
		return tr;
	return 0;
}

Train	*findStrandedTail(int x, int y)
{
	Train	*trn;

	for(trn = stranded; trn; trn = trn->next)
	    if(trn->tail && trn->tail->position &&
		    trn->tail->position->x == x && trn->tail->position->y == y)
		return trn;
	return 0;
}

void	remove_from_stranded_list(Train *tr)
{
	Train	*old = 0;
	Train	*t;

	for(t = stranded; t && t != tr; t = t->next)
	    old = t;
	if(!t)
	    return;
	if(old)
	    old->next = t->next;
	else
	    stranded = t->next;
}

int	sameStationPlatform(const wxChar *s1, const wxChar *s2, bool ignorePlatforms)
{
	if(ignorePlatforms)
	    return sameStation(s1, s2);
	return !wxStrcmp(s1, s2);
}


void	NameWithoutPlatform(wxString& out, const Char *name)
{
	Char	temp[512];
	Char	*p;

	wxStrcpy(temp, name);
	if((p = wxStrchr(temp, PLATFORM_SEP)))
	    *p = 0;
	out = temp;
}


static	Track **array_append(int *nstations, int *maxstations, Track **stations, Track *t)
{
	int	i;

	for(i = 0; i < *nstations; ++i)
	    if(!platform_schedule) {
		if(sameStation(stations[i]->station, t->station))
		    break;
	    } else if(!wxStrcmp(stations[i]->station, t->station))
		break;
	if(i < *nstations)		/* already in list */
	    return stations;
	if(*nstations + 1 >= *maxstations) {
	    *maxstations += 10;
	    if(!stations)
		stations = (Track **)malloc(sizeof(Track *) * *maxstations);
	    else
		stations = (Track **)realloc(stations, sizeof(Track *) * *maxstations);
	}
	stations[*nstations] = t;
	++*nstations;
	return stations;
}

int	cmp_names(const void *a, const void *b)
{
	wxChar *pa = *(wxChar **)a;
	wxChar *pb = *(wxChar **)b;
	return(wxStrcmp(pa, pb));
}

int	cmp_stations(const void *a, const void *b)
{
	Track	*ap = *(Track **)a;
	Track	*bp = *(Track **)b;

	return(wxStrcmp(ap->station, bp->station));
}

void    StationList::Add(Track *t)
{
	int	i;

        if(t) {
	    for(i = 0; i < _nItems; ++i)
	        if(_ignorePlatforms) {
		    if(sameStation(_items[i]->station, t->station))
		        break;
	        } else if(!wxStrcmp(_items[i]->station, t->station))
		    break;
	    if(i < _nItems)		/* already in list */
	        return;
        }
	if(_nItems + 1 >= _maxItems) {
	    _maxItems += 10;
	    if(!_items)
		_items = (Track **)malloc(sizeof(Track *) * _maxItems);
	    else
		_items = (Track **)realloc(_items, sizeof(Track *) * _maxItems);
	}
	_items[_nItems] = t;
	++_nItems;
}

void    StationList::Free()
{
        if(_items)
            free(_items);
        _items = 0;
        _nItems = 0;
        _maxItems = 0;
}

void    StationList::Build(bool ignorePlatforms)
{
        Track   *t;

        _ignorePlatforms = ignorePlatforms;
        Free();
	for(t = layout; t; t = t->next) {
	    if(!t->isstation || !t->station)
		continue;
	    Add(t);
	}
	if(_nItems) {
	    qsort(_items, _nItems, sizeof(Track *), cmp_stations);
	    Add(0); // mark end of array
	}
}

Track	**get_station_list(void)
{
	Track	*t;
	Track	**stations;
	int	nstations, maxstations;

	stations = 0;
	nstations = 0;
	maxstations = 0;
	for(t = layout; t; t = t->next) {
	    if(!t->isstation || !t->station)
		continue;
	    stations = array_append(&nstations, &maxstations, stations, t);
	}
	if(stations) {
	    qsort(stations, nstations, sizeof(Track *), cmp_stations);
	    stations[nstations] = 0;
	}
	return stations;
}

Track	**get_entry_list(void)
{
	Track	*t;
	Track	**stations;
	int	nstations, maxstations;

	stations = 0;
	nstations = 0;
	maxstations = 0;
	for(t = layout; t; t = t->next) {
	    if(t->type != TEXT)
		continue;
	    if(t->wlinkx && t->wlinky) {
	    } else if(t->elinkx && t->elinky) {
	    } else
		continue;
	    stations = array_append(&nstations, &maxstations, stations, t);
	}
	if(stations) {
	    qsort(stations, nstations, sizeof(Track *), cmp_stations);
	    stations[nstations] = 0;
	}
	return stations;
}

wxChar	**name_append(int *nnames, int *maxnames, wxChar **names, wxChar *str)
{
	int	i;

	for(i = 0; i < *nnames; ++i)
	    if(sameStation(names[i], str))
		break;
	if(i != *nnames)		/* already in list */
	    return names;
	if(*nnames  + 1 >= *maxnames) {
	    *maxnames += 20;
	    if(!names)
		names = (wxChar **)malloc(sizeof(wxChar *) * *maxnames);
	    else
		names = (wxChar **)realloc(names, sizeof(wxChar *) * *maxnames);
	}
	names[*nnames] = str;
	++*nnames;
	return names;
}

wxChar	**get_all_station_list(void)
{
	Track	*t;
	Train	*tr;
	TrainStop *ts;
	wxChar	**names;
	int	nnames, maxnames;

	names = 0;
	nnames = 0;
	maxnames = 0;
	for(t = layout; t; t = t->next) {
	    if(!t->isstation || !t->station)
		continue;
	    names = name_append(&nnames, &maxnames, names, t->station);
	}
	for(tr = schedule; tr; tr = tr->next)
	    for(ts = tr->stops; ts; ts = ts->next)
		names = name_append(&nnames, &maxnames, names, ts->station);
	if(names) {
	    qsort(names, nnames, sizeof(wxChar *), cmp_names);
	    names[nnames] = 0;
	}
	return names;
}

void	show_alert(const Char *msg)
{
	wxStrcpy(alert_msg, msg);
	repaint_labels();
	traindir->AddAlert(msg);
}

void	do_alert(const Char *msg)
{
        show_alert(msg);
        if(beep_on_alert._iValue)
	    alert_beep();
}

void    add_linked_info_to_status(Track *t)
{
        if(t->wlinkx && t->wlinky) {
            wxSnprintf(status_line + wxStrlen(status_line),
                sizeof(status_line)/sizeof(status_line[0]) - wxStrlen(status_line),
                wxT(" (%s %d,%d)"), L("linked to"), t->wlinkx, t->wlinky);
        } else if(t->elinkx && t->elinky) {
            wxSnprintf(status_line + wxStrlen(status_line),
                sizeof(status_line)/sizeof(status_line[0]) - wxStrlen(status_line),
                wxT(" (%s %d,%d)"), L("linked to"), t->elinkx, t->elinky);
        }
}

void	pointer_at(Coord cell)
{
	Track	*t;
	Signal	*sig;
	Train	*tr;
	int	x = cell.x;
	int	y = cell.y;
	wxChar	*p;

	tooltipString[0] = 0;
	if((tr = findTrain(x, y))) {
	    wxSnprintf(status_line, sizeof(status_line)/sizeof(status_line[0]), wxT("%d,%d: %s %s"), x, y, tr->name, train_status0(tr, 1));
	    tr->SetTooltip();
	} else if((t = findTrack(x, y)) || (t = findSwitch(x, y))) {
	    wxSnprintf(status_line, sizeof(status_line)/sizeof(status_line[0]), wxT("%d,%d: %s "), x, y, L("speed"));
	    p = status_line + wxStrlen(status_line);
	    for(x = 0; x < NTTYPES; ++x) {
		wxSprintf(p, wxT("%d/"), t->speed[x]);
		p += wxStrlen(p);
	    }
	    wxSprintf(--p, wxT(" Km/h, %s %d m"), L("length"), t->length);
	    if(t->isstation)
		wxSnprintf(status_line + wxStrlen(status_line),
		     sizeof(status_line)/sizeof(status_line[0]) - wxStrlen(status_line),
			 wxT("  %s: %s"), t->type == SWITCH ? L("Switch") : L("Station"), t->station);
	} else if((t = findText(x, y))) {
	    wxSnprintf(status_line, sizeof(status_line)/sizeof(status_line[0]), wxT("%d,%d: %s %s"), x, y, L("entry/exit"), t->station);
            add_linked_info_to_status(t);
	} else if((sig = findSignal(x, y))) {
	    if(sig->controls)
		wxSnprintf(status_line, sizeof(status_line)/sizeof(status_line[0]), wxT("%d,%d: %s %s %s %d, %d"), x, y,
				L("Signal"), sig->station ? sig->station : wxT(""),
				L("controls"), sig->controls->x, sig->controls->y);
	    else
		wxSnprintf(status_line, sizeof(status_line)/sizeof(status_line[0]), wxT("%d,%d: %s %s"), x, y,
				L("Signal"), sig->station ? sig->station : wxT(""));
	    if(sig->stateProgram) {
		wxSnprintf(status_line + wxStrlen(status_line),
		    sizeof(status_line)/sizeof(status_line[0]) - wxStrlen(status_line),
		    wxT("  %s: \"%s\""), L("script"), sig->stateProgram);
		wxSnprintf(status_line + wxStrlen(status_line),
		    sizeof(status_line)/sizeof(status_line[0]) - wxStrlen(status_line),
		    wxT("  %s: \"%s\""), L("aspect"), sig->_currentState ? sig->_currentState : wxT("?"));
	    }
            if(sig->IsClear()) {
                Vector *path = findPath(sig->controls, sig->direction);
                if(path) {
                    wxSnprintf(status_line + wxStrlen(status_line),
                        sizeof(status_line)/sizeof(status_line[0]) - wxStrlen(status_line),
                                wxT("  %s: %d m"), L("block length"), path->_pathlen);
                    Vector_delete(path);
                }
            }
	} else if((t = findTrackType(x, y, TRIGGER))) {
	    wxSnprintf(status_line, sizeof(status_line)/sizeof(status_line[0]), wxT("%d,%d: %s - %s  -> (%d,%d)  Prob.: "),
		x, y, L("Trigger"), t->station ? t->station : wxT(""), t->wlinkx, t->wlinky);
	    p = status_line + wxStrlen(status_line);
	    for(x = 0; x < NTTYPES; ++x) {
		wxSprintf(p, wxT("%d/"), t->speed[x]);
		p += wxStrlen(p);
	    }
	    p[-1] = 0;
	} else if((t = findTrackType(x, y, ITIN))) {
	    wxSnprintf(status_line, sizeof(status_line)/sizeof(status_line[0]), wxT("%d,%d: %s - %s"), x, y,
			    L("Itinerary"), t->station ? t->station : wxT(""));
	} else if((t = findTrackType(x, y, IMAGE))) {
	    wxSnprintf(status_line, sizeof(status_line)/sizeof(status_line[0]), wxT("%d,%d: %s %s"), x, y,
			    L("Image"), t->station ? t->station : wxT(""));
            add_linked_info_to_status(t);
	} else {
	    status_line[0] = 0;
	}
	repaint_labels();
}

void	update_schedule(Train *t)
{
	int	i;
	Train	*t0;

	for(i = 0, t0 = schedule; t0 && t0 != t; t0 = t0->next) {
	    if(!t->entrance)
		continue;
            if(show_canceled._iValue || !is_canceled(t0))
		++i;
	}
	if(!t0)
	    return;
	//print_train_info(t);
	gr_update_schedule(t, i);
	t->newsched = 0;
        t->_lastUpdate = lastModTime++;
}

void	edit_cmd()
{
	if(editing)
	    return;
	if(editing_itinerary)		/* exit edit itinerary mode */
	    itinerary_cmd();
	editing = 1;			/* enter edit layout mode */
	hide_table();
	show_tooltable();
	repaint_all();
}

void	noedit_cmd()
{
	Track	*t;

	if(!editing)
	    return;
	editing = 0;
	link_all_tracks();
	total_track_number = 0;
	for(t = track_list; t; t = t->next1)
	    ++total_track_number;
	hide_tooltable();
	show_table();
	link_signals(layout);
	invalidate_field();
	repaint_all();
	check_layout_errors();
}

void	itinerary_cmd()
{
	if(editing_itinerary) {		/* back to simulation mode */
	    editing_itinerary = 0;
	    hide_itinerary();
	    show_table();
	} else {
	    if(editing)			/* exit edit layout mode */
		noedit_cmd();
	    editing_itinerary = 1;	/* enter edit itinerary mode */
	    hide_table();
	    show_itinerary();
	}
	repaint_all();
}

void	do_replay(void)
{
	long	issue_time;
	size_t	pos;
	wxChar	*p;
	wxChar	buff[256];

	while(frply) {
	    pos = frply->GetPos();
	    if(!frply->ReadLine(buff, sizeof(buff)/sizeof(wxChar)))
		break;
	    buff[sizeof(buff)/sizeof(buff[0]) - 1] = 0;
	    p = buff + wxStrlen(buff);
	    if(p > buff && p[-1] == '\n') --p;
	    if(p > buff && p[-1] == '\r') --p;
	    *p = 0;
	    issue_time = wxStrtoul(buff, &p, 10);
	    if(*p == ',') ++p;
	    if(issue_time > current_time) {	/* goes into next time slice */
		frply->SetPos(pos);		/* back off to cmd start */
		return;				/* nothing else to do */
	    }
	    trainsim_cmd(p);
	}
	if(frply) {
	    delete frply;
	    frply = NULL;
	}
}

void    StationSchedule::Free()
{
	struct station_sched *sc;

	while((sc = _items)) {
	    _items = sc->next;
	    free(sc);
	}
}

static	int	stschcmp(const void *a, const void *b)
{
	struct station_sched *ap = *(struct station_sched **)a;
	struct station_sched *bp = *(struct station_sched **)b;
        long	t1, t2;

	if((t1 = ap->arrival) == -1)
	    t1 = ap->departure;
	if((t2 = bp->arrival) == -1)
	    t2 = bp->departure;
	return(t1 < t2 ? -1 : t1 == t2 ? 0 : 1);
}

struct station_sched *sort_station_schedule(struct station_sched *sched)
{
	struct station_sched **qb;
	struct station_sched *t;
	int	ntrains;
	int	l;

	for(t = sched, ntrains = 0; t; t = t->next)
	    ++ntrains;
	qb = (struct station_sched **)malloc(sizeof(struct station_sched *) * ntrains);
	for(t = sched, l = 0; l < ntrains; ++l, t = t->next)
	    qb[l] = t;
	qsort(qb, ntrains, sizeof(struct station_sched *), stschcmp);
	for(l = 0; l < ntrains - 1; ++l)
	    qb[l]->next = qb[l + 1];
	qb[ntrains - 1]->next = 0;
	t = qb[0];
	free(qb);
	return t;
}

void	StationSchedule::Build(const Char *station)
{
	Train	*tr;
	TrainStop *ts;
	struct station_sched *sc;

	Free();
	for(tr = schedule; tr; tr = tr->next) {
	    if(sameStationPlatform(tr->entrance, station, _ignorePlatforms)) {
		sc = (struct station_sched *)malloc(sizeof(struct station_sched));
		memset(sc, 0, sizeof(struct station_sched));
		sc->tr = tr;
		sc->arrival = -1;
		sc->departure = tr->timein;
		sc->stopname = tr->entrance;
		sc->next = _items;
		_items = sc;
	    } else if(sameStationPlatform(tr->exit, station, _ignorePlatforms)) {
		sc = (struct station_sched *)malloc(sizeof(struct station_sched));
		memset(sc, 0, sizeof(struct station_sched));
		sc->tr = tr;
		sc->arrival = tr->timeout;
		sc->departure = -1;
		sc->stopname = tr->exit;
		sc->next = _items;
		_items = sc;
	    } else for(ts = tr->stops; ts; ts = ts->next) {
		if(sameStationPlatform(ts->station, station, _ignorePlatforms)) {
		    sc = (struct station_sched *)malloc(sizeof(struct station_sched));
		    memset(sc, 0, sizeof(struct station_sched));
		    sc->tr = tr;
		    sc->arrival = ts->arrival;
		    sc->departure = ts->departure;
		    sc->stopname = ts->station;
		    if(!ts->minstop)
			sc->transit = 1;
		    sc->next = _items;
		    _items = sc;
		    break;
		}
	    }
	}
	if(_items)
	    _items = sort_station_schedule(_items);
}

void	do_station_list_print(const wxChar *station_name, HtmlPage& page)
{
	wxChar	*p;
	int	i;
	static	wxChar	buff[256];
	struct station_sched	*sc;
	wxChar	buffs[8][250];
//	wxChar	*cols[9];
	wxString *cols[9];
	wxString values[9];
        StationSchedule sched;

        sched._ignorePlatforms = !platform_schedule;
        sched.Build(station_name);

	wxStrcpy(buff, wxT("station.htm"));
	cols[0] = &values[0];
	cols[1] = &values[1];
	cols[2] = &values[2];
	cols[3] = &values[3];
	cols[4] = &values[4];
	cols[5] = &values[5];
	cols[6] = &values[6];
	cols[7] = &values[7];
	cols[8] = NULL;

	wxStrcpy(buffs[0], station_name);
	if((p = wxStrchr(buffs[0], PLATFORM_SEP)))
	    *p = 0;
	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s %s"), L("Station of"), buffs[0]);
	if(p && !sched._ignorePlatforms)
	    wxSnprintf(buff + wxStrlen(buff), sizeof(buff)/sizeof(wxChar) - wxStrlen(buff), wxT(" - %s %s"), L("Platform"), p + 1);
	page.StartPage(buff);
	wxSnprintf(buff, sizeof(buff)/sizeof(wxChar),
	    wxT("<br><a href=\"savestationinfopage %s\">%s</a><br>\n"), station_name, L("Save as text"));
	page.Add(buff);
	for(i = 0; en_station_titles[i]; ++i)
	    station_titles[i] = LV(en_station_titles[i]);
	page.StartTable(station_titles);
        for(sc = sched._items; sc; sc = sc->next) {
	/* when reassigning train stock, we consider only
	    trains that are scheduled to depart at the same
	    station where the assignee has arrived. */
	    wxSnprintf(buffs[0], sizeof(buffs[0])/sizeof(wxChar), wxT("<a href=\"traininfopage %s\">%s</a>"), sc->tr->name, sc->tr->name);
	    values[0] = buffs[0];
////	    wxStrcpy(cols[0], sc->tr->name);
	    buffs[2][0] = 0;
	    if(sc->transit) {
		values[1] = wxT("");
		wxStrcpy(buffs[2], sc->tr->entrance);
		if((p = wxStrchr(buffs[2], PLATFORM_SEP)))
		    *p = 0;
	    } else if(sc->arrival != -1) {
		values[1] = format_time(sc->arrival);
		wxStrcpy(buffs[2], sc->tr->entrance);
		if((p = wxStrchr(buffs[2], PLATFORM_SEP)))
		    *p = 0;
	    } else {
		values[1] = wxT("");
	    }
	    values[2] = buffs[2];
	    if(sc->departure != -1) {
		if(sc->transit)
		    wxSnprintf(buffs[3], sizeof(buffs[3])/sizeof(wxChar), wxT("(%s)"), format_time(sc->departure));
		else
		    wxStrcpy(buffs[3], format_time(sc->departure));
		values[3] = buffs[3];
		wxStrcpy(buffs[4], sc->tr->exit);
		if((p = wxStrchr(buffs[4], PLATFORM_SEP)))
		    *p = 0;
		values[4] = buffs[4];
	    } else {
		values[3] = wxT("");
		values[4] = wxT("");
	    }
	    buffs[5][0] = 0;
	    for(i = 0; i < 7; ++i)
		if(sc->tr->days & (1 << i))
		    wxSnprintf(buffs[5] + wxStrlen(buffs[5]), sizeof(buffs[5])/sizeof(wxChar) - wxStrlen(buffs[5]), wxT("%d"), i+1);
	    values[5] = buffs[5];
	    buffs[6][0] = 0;
	    if(sc->stopname && (p = wxStrchr(sc->stopname, PLATFORM_SEP)))
		wxSnprintf(buffs[6], sizeof(buffs[6])/sizeof(wxChar), wxT("%s"), p + 1);
	    values[6] = buffs[6];
	    buffs[7][0] = 0;
	    if(sc->tr->nnotes)
		wxStrncpy(buffs[7], sc->tr->notes[0], sizeof(buffs[7])/sizeof(wxChar));
	    values[7] = buffs[7];
	    page.AddTableRow(8, cols);
	}
	page.EndTable();
	page.EndPage();
}

int	all_trains_everyday(Train *t)
{
	while(t) {
	    if(t->days)
		return 0;
	    t = t->next;
	}
	return 1;
}

void	do_itinerary_dialog(int x, int y)
{
	Itinerary *it = 0;
	Signal	*sig;
	wxChar	buff[256];

	sig = findSignal(x, y);
	if(!sig)
	    return;
	if(!sig->station || !*sig->station) {
	    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("(%d,%d)"), sig->x, sig->y);
	    sig->station = wxStrdup(buff);
	}
/*	for(it = itineraries; it; it = it->next)
	    if(!wxStrcmp(it->signame, sig->station))
		break;
*/	if(!it) {
	    it = (Itinerary *)calloc(sizeof(Itinerary), 1);
	    it->signame = wxStrdup(sig->station);
	    it->name = wxStrdup(wxT(""));
	    it->next = itineraries;
	    itineraries = it;
	}
	fill_itinerary(it, (Signal *)sig);
	itinerary_dialog(it);
}

int	set_itin_name(Itinerary *it, wxChar *name, wxChar *nextit)
{
	Itinerary *it1, *it2;
	wxChar	*p;

	if((p = wxStrchr(name, ',')))	/* no commas allowed */
	    *p = 0;
	it2 = 0;
	for(it1 = itineraries; it1; it2 = it1, it1 = it1->next)
	    if(it1 != it && !wxStrcmp(name, it1->name)) {
		if(ask(L("An itinerary by the same name already exists.\n"
			"Do you want to replace the old itinerary with the new one?"))
			== ANSWER_YES) {
		    if(!it2)
			itineraries = it1->next;
		    else
			it2->next = it1->next;
		    if(it1->signame) free(it1->signame);
		    free(it1->name);
		    free(it1);
		    break;
		}
		return 0;		/* let user change name */
	    }
	if(it->name)
	    free(it->name);
	it->name = wxStrdup(name);

	if((p = wxStrchr(nextit, ',')))	/* no commas allowed */
	    *p = 0;
	if(it->nextitin)
	    free(it->nextitin);
	it->nextitin = wxStrdup(nextit);
	return 1;
}

int	set_track_properties(Track *t, const wxChar *len, const wxChar *station, const wxChar *speed,
			     const wxChar *distance, const wxChar *wlink, const wxChar *elink,const wxChar *num)
{
	wxChar	*p;
	int	flag;

	t->length = wxAtol(len);
	t->speed[0] = (short)wxStrtol(speed, &p, 10);
	for(flag = 1; flag < NTTYPES && *p == '/'; ++flag) {
	    t->speed[flag] = (short)wxStrtol(p + 1, &p, 10);
	}
	t->isstation = 0;
	t->line_num = wxAtol(num);    //ypx20201209
	if(t->station)
	    free(t->station);
	t->station = 0;
	flag = 0;
	if(station && *station) {
	    t->station = wxStrdup(station);
	    if(t->type != TEXT && t->type != TSIGNAL)
		t->isstation = 1;
	    else
		flag = 1;
	}
	if(*distance)
	    parse_km(t, distance);
	else
	    t->km = 0;
	
	
	t->wlinkx = (unsigned short)wxStrtol(wlink, &p, 10);
	if(*p == ',') ++p;
	t->wlinky = (unsigned short)wxStrtol(p, &p, 10);

	if(t->type == IMAGE)
	    t->pixels = 0;	/* will reload image from file */
	else if(t->type != TSIGNAL && t->type != SWITCH) {
	    t->elinkx = (unsigned short)wxStrtol(elink, &p, 10);
	    if(*p == ',') ++p;
	    t->elinky = (unsigned short)wxStrtol(p, &p, 10);
	}
	return flag;
}

void    apply_power()
{
        Coord	start, end;
	Track	*t;

	if(move_end.x < move_start.x) {
	    start.x = move_end.x;
	    end.x = move_start.x;
	} else {
	    start.x = move_start.x;
	    end.x = move_end.x;
	}
	if(move_end.y < move_start.y) {
	    start.y = move_end.y;
	    end.y = move_start.y;
	} else {
	    start.y = move_start.y;
	    end.y = move_end.y;
	}
	for(t = layout; t; t = t->next) {
	    if(isInside(start, end, t->x, t->y)) {
	        t->power = gEditorMotivePower;
            }
        }
}

void    set_track_lengths(int len)
{
        Coord	start, end;
	Track	*t;

	if(move_end.x < move_start.x) {
	    start.x = move_end.x;
	    end.x = move_start.x;
	} else {
	    start.x = move_start.x;
	    end.x = move_end.x;
	}
	if(move_end.y < move_start.y) {
	    start.y = move_end.y;
	    end.y = move_start.y;
	} else {
	    start.y = move_start.y;
	    end.y = move_end.y;
	}
	for(t = layout; t; t = t->next) {
	    if(isInside(start, end, t->x, t->y)) {
                t->length = len;
            }
        }
}

void    load_scenario_info(const Char *cmd)
{
	TDFile	infoFile(cmd);

        wxString ext;
        ext.Printf(wxT("%s.htm"), locale_name);
        infoFile.SetExt(ext);
        if(infoFile.Load()) {
	    traindir->m_frame->ShowHtml(L("Scenario Info"), infoFile.content);
	    info_page = infoFile.name.GetName();
        } else {
            infoFile.SetExt(wxT(".htm"));
	    if(infoFile.Load()) {
	        traindir->m_frame->ShowHtml(L("Scenario Info"), infoFile.content);
	        info_page = infoFile.name.GetName();
	    } else {
	        TDFile	indexFile(wxT("index.htm"));
	        if(indexFile.Load()) {
		    traindir->m_frame->ShowHtml(L("Scenario Info"), indexFile.content);
		    info_page = wxT("index.htm");
	        } else
		    info_page = wxT("");
	    }
        }
}

void	load_new_scenario(const wxChar *cmd, int fl)
{
	while(*cmd == ' ') ++cmd;
	if(running)
	    start_stop();
	if(layout_modified) {
	    if(ask_to_save_layout() < 0)	// cancel selected
		return;
	}
	clean_trains(schedule);
	clean_trains(stranded);
	schedule = 0;
	stranded = 0;
	invalidate_field();
	enable_training = 0;
	if(fl == 2) {
	    load_puzzles(cmd);
	    trainsim_init();		/* clear counters, timer */
	    load_scripts(layout);	// run OnInit scripts
	    enable_training = 1;
	} else {
	    if(!(layout = load_field(cmd))) {
		wxSnprintf(status_line, sizeof(status_line)/sizeof(wxChar), wxT("%s '%s.trk'"), L("cannot load"), cmd);
		traindir->Error(status_line);
		return;
	    }
	    if(!(schedule = load_trains(cmd)))
		traindir->Error(L("No schedule for this territory!"));
		if(!(speedlim = load_spdlim(cmd)))    //ypx  限速配置
		traindir->Error(L("No speedlimit for this territory!"));
	    if(fl && !all_trains_everyday(schedule) && select_day_dialog)
		select_day_dialog();
	    if(fl)
		check_delayed_entries(schedule);
	    /* fill_schedule(schedule, 0); */
	    trainsim_init();		/* clear counters, timer */
	    load_scripts(layout);	// run OnInit scripts
            fillDwellingTracks();
            bstreet_playing();
	}

        load_scenario_info(cmd);
	if(fl == 2) {
	    show_puzzle();
	}
	repaint_all();
        timetable._lastReloaded = ++lastModTime;
        timetable.NotifyListeners();
}

Itinerary *parse_itinerary(const wxChar *cmd)
{
        for(; *cmd == wxT(' ') || *cmd == wxT('\t'); ++cmd);
        const wxChar *nameend = wxStrrchr(cmd, wxT('@'));
        int	    namelen;
        Itinerary *it;

        if(nameend)
	    namelen = nameend - cmd;
        else
	    namelen = wxStrlen(cmd);
        for(it = itineraries; it; it = it->next) {
	    if(!wxStrncmp(it->name, cmd, namelen) && wxStrlen(it->name) == namelen)
                break;
        }
        return it;
}

void    sendRestart()
{
        Track   *t;
        Itinerary *it;

        for(t = layout; t; t = t->next) {
            t->OnRestart();
        }
        for(it = itineraries; it; it = it->next) {
            it->OnRestart();
        }
}

extern void send_msg(wxChar *msg,int send_len);
extern void send_msg1(wxChar *msg,int send_len);
extern void send_udp1_msg(wxChar *msg,int sendlen,int flag);
extern void	send_udp2_msg(wxChar *msg,int send_len);
extern  bool    doDelayedRepaint;
void Stopcopy(TrainStop *s1, TrainStop *s2){
	while(s1){
		if(!wxStrcmp(s1->station,s2->station)){
			s1->arrival = s2->arrival;
			s1->departure = s2->departure;
			//strcpy(s1->station,s2->station );
			
	}
		s1=s1->next;
		s2=s2->next;
	}
}

void	do_command(const wxChar *cmd, bool sendToClients)
{
	const wxChar	*p;
	Train	*t; 
	Track	*trk;
	int	x, y, fl;
	int limitStop;
    int limitSpeed = 0, limitBegin = 0,limitEnd = 0,limitKm = 0;

        Coord   pos;
        wxChar	buff[1024];
	    const wxChar  *it_name;
	    wxChar  *next_name;

		

	if(!wxStrncmp(cmd, wxT("log"), 3)) {
	    if(!flog.IsOpened()) {
		if(!(flog.Open(wxT("log"), wxT("w"))))
		    do_alert(L("Cannot create log file."));
		return;
	    }	
	    flog.Close();
	    return;
	}

	if(!wxStrncmp(cmd, wxT("replay"), 6)) {
	    for(p = cmd + 6; *p == ' ' || *p == '\t'; ++p);
	    wxSnprintf(buff, sizeof(buff)/sizeof(wxChar), wxT("%s.log"), p);
	    if(!(frply = new TDFile(buff))) {
		do_alert(L("Cannot read log file."));
		return;
	    }
	    /* replay commands are issued whenever the clock is updated */
	    return;
	}
	if(flog.IsOpened())
	    flog.Write(wxString::Format(wxT("%ld,%s\n"), current_time, cmd));
	wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]), wxT("%ld,%s\n"), current_time, cmd);
	if(sendToClients)
	    send_msg(buff,wxStrlen(buff)); 
	if(!wxStrncmp(cmd, wxT("quit"), 4))
	    main_quit_cmd();
	else if(!wxStrncmp(cmd, wxT("about"), 5)) {
	    about_dialog();
	} else if(!wxStrcmp(cmd, wxT("edititinerary"))) {
	    itinerary_cmd();

	} else if(!wxStrncmp(cmd, wxT("edit"), 4)) {
	    if(running)
		start_stop();
	    edit_cmd();
	} else if(!wxStrncmp(cmd, wxT("noedit"), 6))
	    noedit_cmd();
	else if(!wxStrncmp(cmd, wxT("stationsched"), 12))
	    station_sched_dialog(NULL);
	else if(!wxStrncmp(cmd, wxT("paths"), 5))
	    create_path_window();
///////////////////////////20211017 接收晚点后调度算法传输的单列车单车站停站时间/////////////////
	else if(!wxStrncmp(cmd, wxT("waittime"), 8))
	    do_command(wxT("run"), false);
///////////////////////////2020310 TCPIP/////////////////
	else if(!wxStrncmp(cmd, wxT("newsch"), 5)){
	   //do_command(wxT("graph"), false);
		tcp_flag = 0;
		if(newsch){   //释放内存
			Train *temp;
			temp = newsch;
			while(temp){
				newsch =newsch->next;
				delete temp;
			    temp = newsch;
			}
		}
		newsch = update_trains(wxT("D:/received"));
		Train *tr1,*tr2;
		for(tr1 = schedule; tr1; tr1 = tr1->next){
			for(tr2 = newsch;tr2; tr2 = tr2->next){    //遍历更新列车的stop信息
				if(!wxStrcmp(tr1->name,tr2->name)){
					Stopcopy( tr1->stops , tr2->stops);
					tr1->timeout = tr2->timeout;
					for(TrainStop *s = tr2->nxtstop;s;s=s->next){   //遍历更新列车的nxtstop信息
						if(!wxStrcmp(tr1->nxtstop->station,s->station)){
							if((tr1->decflag == 2) && (tr1->nxtstop->arrival != s->arrival))   //发车决策时且接收到的时刻表有变化
							{
							tr1->pathtravelled = 0;
							Stopcopy(tr1->stops,s);
							impot_curve(tr1);
							tr1->curve_travelled = 1.111;
							}
							Stopcopy(tr1->nxtstop,s);
							break;
						}
					}
				break;
				}
			}
		}
		
///////////////////////////20220117 限速设置/////////////////
	}else if(!wxStrncmp(cmd, wxT("speedlimit"), 10)){
		wxChar *s = new char[strlen(cmd+1)];
		strcpy(s,cmd+11);
		strtok(s,",");
		limitStop = wxAtoi(s);
		limitBegin = wxAtoi(strtok(NULL,","));
		limitEnd = wxAtoi(strtok(NULL,","));
		limitSpeed = wxAtoi(strtok(NULL,","));
		SpeedLimitStop.push_back(limitStop);
		SpeedLimitValue.push_back(limitSpeed);
		SpeedStart.push_back(limitBegin);
		SpeedEnd.push_back(limitEnd);
		delete s;
	/////////////////////20200426 速度位置曲线///////////
	}else if(!wxStrncmp(cmd, wxT("deletelimit"), 11)){
		limitStop = 0;
		limitKm = limitBegin = limitEnd = limitSpeed =0;
		SpeedLimitStop.clear();
		SpeedLimitValue.clear();
		SpeedStart.clear();
		SpeedEnd.clear();
	}else if(!wxStrncmp(cmd, wxT("newatoo"), 5)){
	   loadato();
	   if(time_mults[cur_time_mult + 1] != -1)
		time_mult = time_mults[cur_time_mult];
	    update_labels();
//////////////////////////20200406 查询空闲站////////////////
	}else if(!wxStrncmp(cmd, wxT("freetrk"), 7)){
      Itinerary *it = parse_itinerary(cmd + 7);
	  it_name=it->name;
	  int itflag=1;
	  //stop_name=schedule->stops->station;
	  /*stop_name=t->stops->station;*/

 	  for (int i=0;stop_name[i];i++)
	  {
		  //if(stop_name[i])
			 // break;
	  if(sameStation_s1(stop_name[i],it_name)||sameStation_s1(exit_name,it_name))
	    {
	       itflag=0;
	       break;
	    }
	  }

	  errortype = NOERR;
	  if (itflag==0){
		  if (!it->Find(false))
		  //if (!it->Find_special(false,it->name))//lrg 20211220修改
	     errortype = TRKFIND;
	  }
		//wxSnprintf(temp_name, sizeof(buff)/sizeof(wxChar), wxT("click %d %d"), coord.x, coord.y);
		/*wxStrcpy(temp_name,cmd);*/
	  else
		  if (!it->Select(false))
		 errortype = TRKSELECT;
	  memset(stop_name, 0, sizeof(stop_name));
	 
///////////转线选择///////////////////////////////////////////

	  }else if(!wxStrncmp(cmd, wxT("selectline"), 10)){

      cmd+=10;
	    for(; *cmd == wxT(' ') || *cmd == wxT('\t'); ++cmd);

      //wxChar	itinerary_cmd[50]="itinerary ";
      wxChar	itinerary_cmd[20];
	  wxStrcpy(itinerary_cmd,cmd);

	  for (int i=0;stop_name[i];i++)
	  {
	    if((!wxStrncmp(cmd, stop_name[i], 4))||(!wxStrncmp(cmd,exit_name , 4)))
	      {
           wxStrncat(itinerary_cmd,stop_name[i+1],4);
	       break;
	      }
	  }
	 errortype = NOERR;
	/* do_command(itinerary_cmd, false);*/
	   Itinerary *it = parse_itinerary(itinerary_cmd);
	    if(it)
			if(!it->Select(false)){
					errortype = SELECT;
					errorname=wxStrdup(itinerary_cmd);
			}
     memset(stop_name, 0, sizeof(stop_name));
     memset(itinerary_cmd, 0, sizeof(itinerary_cmd));

	  }else if(!wxStrncmp(cmd, wxT("fast"), 4)) {
	    if(time_mults[cur_time_mult + 1] != -1)
		time_mult = time_mults[++cur_time_mult];
	    update_labels();
	} else if(!wxStrncmp(cmd, wxT("slow"), 4)) {
	    if(cur_time_mult > 0) {
		time_mult = time_mults[--cur_time_mult];
		update_labels();
	    }
	} else if(!wxStrncmp(cmd, wxT("t0"), 2)) {
	   // if(cmd[2] == '!' || cont(L("Do you want to restart the simulation?")) == ANSWER_YES) {
		if(cmd[2] != '!' && !all_trains_everyday(schedule))
		    select_day_dialog();
		clear_delays();
		remove("C:/Users/DELL/Desktop/DATA/speed.txt");
		remove("C:/Users/DELL/Desktop/DATA/data.txt");
		fill_schedule(schedule, 0);
	        wxSnprintf(status_line, sizeof(status_line)/sizeof(wxChar), L("Simulation restarted."));
	        trainsim_init();
                resetDwellingTracks();
		invalidate_field();
		update_button(wxT("stop"), L("Stop"));
                sendRestart();
		repaint_all();
	   // }
	} else if(match(&cmd, wxT("speeds"))) {
            show_speeds.Set(!show_speeds._iValue);
	    invalidate_field();
	    repaint_all();
	} else if(!wxStrncmp(cmd, wxT("traditional"), 6)) {
	    signal_traditional = !signal_traditional;
	    invalidate_field();
	    repaint_all();
	} else if(!wxStrncmp(cmd, wxT("graph"), 6)) {
	    create_tgraph();
		//update_trains(wxT("C://Users////Desktop//ydm//case_linbo"));
		
	} else if(match(&cmd, wxT("blocks"))) {
          /*  show_blocks.Set(!show_blocks._iValue);
	    invalidate_field();
	    repaint_all();*/
		int ncounter=0;
       if(wxhonoff)
		 wxhonoff=0;//wxh
       else
         wxhonoff=1;
	} else if(!wxStrncmp(cmd, wxT("alert"), 5)) {
	    beep_on_alert.Set(!beep_on_alert._iValue);
	} else if(!wxStrncmp(cmd, wxT("sched"), 5)) {
	    create_schedule(0);
	}///////////////////指定列车////////////////////
	else if(!wxStrncmp(cmd, wxT("check"), 5)) {
		int ncounter=0;
		 if(wxhonoff1)
		 wxhonoff1=0;//wxh
		 else{
	   /*  const wxChar *check;
	     check=cmd+5;*/
		 cmd = cmd+5;
		 for(; *cmd== wxT(','); ++cmd);
		 check_name=cmd;
		 }
	}else if(!wxStrncmp(cmd, wxT("deletecheck"), 11)){
		check_name = 0;
		wxhonoff1 = 1;
	
	} else if(!wxStrncmp(cmd, wxT("run"), 3)) {
	    start_stop();
	    update_button(wxT("run"), running ? L("Stop") : L("Start"));
        } else if(match(&cmd, wxT("startsim"))) {
            if(!running)
                start_stop();
        } else if(match(&cmd, wxT("stopsim"))) {
            if(running)
                start_stop();
	} else if(!wxStrncmp(cmd, wxT("newtrain"), 8)) {
	    create_train();
	} else if(!wxStrncmp(cmd, wxT("greensigs"), 9)) {
	    open_all_signals();
	} else if(!wxStrncmp(cmd, wxT("shunt"), 5)) {
	    cmd += 5;
	    while(*cmd == ' ' || *cmd == '\t') ++cmd;
            if(!(t = findTrainNamed(cmd))) {
                if (!(t = findTrainAtCoord(cmd)))
                    return;
            }
	    shunt_train(t);
	} else if(!wxStrncmp(cmd, wxT("traininfopage"), 13)) {
	    cmd += 13;
	    while(*cmd == ' ' || *cmd == '\t') ++cmd;
	    if(!(t = findTrainNamed(cmd)))
		return;
	    ShowTrainInfo(t);
	} else if(!wxStrncmp(cmd, wxT("traininfo"), 9)) {
	    cmd += 9;
	    while(*cmd == ' ' || *cmd == '\t') ++cmd;
	    if(!(t = findTrainNamed(cmd)))
		return;
	    train_info_dialog(t);
	} else if(!wxStrncmp(cmd, wxT("decelerate"), 10)) {
	    long    val;
	    wxChar *end;

	    cmd += 10;
	    while(*cmd == ' ' || *cmd == '\t') ++cmd;
	    val = wxStrtol(cmd, &end, 0);
	    while(*end == ' ' || *end == '\t') ++end;
	    if(!(t = findTrainNamed(end)))
		return;
	    decelerate_train(t, val);
	} else if(!wxStrncmp(cmd, wxT("accelerate"), 10)) {
	    long    val;
	    wxChar *end;

	    cmd += 10;
	    while(*cmd == ' ' || *cmd == '\t') ++cmd;
	    val = wxStrtol(cmd, &end, 0);
	    while(*end == ' ' || *end == '\t') ++end;
	    if(!(t = findTrainNamed(end)))
		return;
	    accelerate_train(t, val);
	} else if(!wxStrncmp(cmd, wxT("stationinfopage"), 15)) {
	    cmd += 15;
	    while(*cmd == ' ' || *cmd == '\t') ++cmd;
	    ShowStationSchedule(cmd, false);
	} else if(!wxStrncmp(cmd, wxT("savestationinfopage"), 19)) {
	    cmd += 19;
	    while(*cmd == ' ' || *cmd == '\t') ++cmd;
	    ShowStationSchedule(cmd, true);
	} else if(!wxStrncmp(cmd, wxT("stationinfo"), 11)) {
	    cmd += 11;
	    while(*cmd == ' ' || *cmd == '\t') ++cmd;
	    station_sched_dialog(cmd);
	} else if(!wxStrncmp(cmd, wxT("reverse"), 7)) {
	    cmd += 7;
	    while(*cmd == ' ' || *cmd == '\t') ++cmd;
            if(!(t = findTrainNamed(cmd))) {
                if (!(t = findTrainAtCoord(cmd)))
                    return;
            }
	    reverse_train(t);
        } else if(match(&cmd, wxT("startnow"))) {
	    if(!(t = findTrainNamed(cmd)))
		return;
            start_running_now(t);
	} else if(!wxStrncmp(cmd, wxT("new"), 3)) {
	    if(running)
		start_stop();
	    if(layout_modified) {
		if(ask_to_save_layout() < 0)	// cancel selected
		    return;
	    }
	    init_all();
	} else if(!wxStrncmp(cmd, wxT("save "), 5)) {
	    if(save_layout(cmd + 5, layout))
		wxSnprintf(status_line, sizeof(status_line)/sizeof(wxChar), wxT("%s '%s.trk'."), L("Layout saved in file"), cmd + 5);
	    repaint_labels();
	} else if(!wxStrncmp(cmd, wxT("savegame "), 9)) {
	    if(save_game(cmd + 9))
		wxSnprintf(status_line, sizeof(status_line)/sizeof(wxChar), wxT("%s '%s.sav'."), L("Game status saved in file"), cmd + 9);
	    repaint_labels();
	} else if(!wxStrncmp(cmd, wxT("restore "), 8)) {
	    if(layout_modified) {
		if(ask_to_save_layout() < 0)	// cancel selected
		    return;
	    }
	    restore_game(cmd + 8);
	    invalidate_field();
	    repaint_all();
	    fill_schedule(schedule, 0);
	    update_labels();
	} else if(!wxStrncmp(cmd, wxT("open"), 4) || !wxStrncmp(cmd, wxT("load"), 4)) {
	    fl = cmd[0] == 'o';		/* open vs. load */
	    cmd += 4;
 	    load_new_scenario(cmd, fl);
		remove("D:/data.txt");
		remove("D:/speed.txt");
	}else if(!wxStrncmp(cmd, wxT("forceRestart"),11)){
		 fl  = 'o';
		 load_new_scenario(timetablePath, fl);
		 remove("D:/data.txt");
		 remove("D:/speed.txt");
		 open_all_signals();
	}else if(!wxStrncmp(cmd, wxT("puzzle"), 6)) {
	    cmd += 6;
	    load_new_scenario(cmd, 2);
	} else if(match(&cmd, wxT("click"))) {
            if(!parseCoords(&cmd, pos)) {
		if(!(trk = findItineraryNamed(cmd)))
		    return;		/* impossible ? */
		pos.x = trk->x;
		pos.y = trk->y;
	    }
	    track_selected(pos.x, pos.y);
	} else if(match(&cmd, wxT("rclick"))) {
            if(!parseCoords(&cmd, pos)) {
		if(!(trk = findItineraryNamed(cmd)))
		    return;		/* impossible ? */
		pos.x = trk->x;
		pos.y = trk->y;
	    }
	    track_selected1(pos.x, pos.y);
	} else if(match(&cmd, wxT("shiftclick"))) {
            if(parseCoords(&cmd, pos))
                track_shift_selected(pos);
//      } else if(match(&cmd, wxT("shiftrclick"))) {
//            if(parseCoords(&cmd, pos)) {
//                track_shift_selected_right(pos);
	} else if(match(&cmd, wxT("ctrlclick"))) {
            if(!parseCoords(&cmd, pos)) {
		if(!(trk = findItineraryNamed(cmd)))
		    return;		/* impossible ? */
		pos.x = trk->x;
		pos.y = trk->y;
	    }
	    track_control_selected(pos);
	} else if(match(&cmd, wxT("sigopen"))) {
            Signal *sig;
            if(parseCoords(&cmd, pos)) {
                sig = findSignal(pos.x, pos.y);
            } else if(!(sig = findSignalNamed(cmd))) {
		return;
	    }
            if(!sig->IsClear()) { // try to clear the signal
                track_selected(sig->x, sig->y);
            }
            // else do nothing
	} else if(match(&cmd, wxT("sigclose"))) {
            Signal *sig;
            if(parseCoords(&cmd, pos)) {
                sig = findSignal(pos.x, pos.y);
            } else if(!(sig = findSignalNamed(cmd))) {
		return;
	    }
            if(sig->IsClear()) { // turn the signal to red
                track_selected(sig->x, sig->y);
            }
            // else do nothing
	} else if(match(&cmd, wxT("selecttool"))) {
            if(parseCoords(&cmd, pos))
                tool_selected(pos.x, pos.y);
	} else if(!wxStrncmp(cmd, wxT("itinerary"), 9)) {
            Itinerary *it = parse_itinerary(cmd + 9);
			errortype = NOERR;
	    if(it)
			if(! it->Select(false))
				errortype = ITINERARY;
	} else if(!wxStrncmp(cmd, wxT("delitinerary"), 9)) {
            Itinerary *it = parse_itinerary(cmd + 12);
	    if(it)
                it->Deselect(false);
	} else if(!wxStrcmp(cmd, wxT("info"))) {
	    track_info_dialogue();
	} else if(!wxStrncmp(cmd, wxT("sb-edit"), 7)) {
	    SwitchboardEditCommand(cmd + 7);
	} else if(!wxStrncmp(cmd, wxT("sb-browser"), 10)) {
	    SwitchboardOpenBrowser(cmd + 10);
	} else if(!wxStrncmp(cmd, wxT("sb-cell"), 7)) {
	    SwitchboardCellCommand(cmd + 7);
	} else if(!wxStrcmp(cmd, wxT("performance"))) {
	    performance_dialog();
	} else if(!wxStrcmp(cmd, wxT("performance_toggle_canceled"))) {
	    performance_toggle_canceled();
	    performance_dialog();	// update page
	} else if(match(&cmd, wxT("options"))) {
	    options_dialog();
	    if(hard_counters)
		perf_vals = perf_hard;
	    else
		perf_vals = perf_easy;
	    invalidate_field();
	    repaint_all();
	    update_labels();
	    new_status_position();
	} else if(match(&cmd, wxT("option"))) {
	    cmd = skip_blanks(cmd);
            x = 0;
	    while(*cmd && *cmd != '=') {
		buff[x++] = *cmd++;
	    }
            while(x > 0 && buff[x - 1] == ' ')
                --x;
	    buff[x] = 0;
            if(*cmd) {
                int val = wxAtoi(++cmd);
                set_option(buff, cmd, val);
	        if(hard_counters)
		    perf_vals = perf_hard;
	        else
		    perf_vals = perf_easy;
	        invalidate_field();
	        repaint_all();
	        update_labels();
	        new_status_position();
            }
	} else if(!wxStrncmp(cmd, wxT("assign"), 6)) {
	    Train   *t1;

	    for(cmd += 6; *cmd == ' ' || *cmd == '\t'; ++cmd);
	    x = 0;
	    while(*cmd && *cmd != ',') {
		buff[x++] = *cmd++;
	    }
	    buff[x] = 0;
	    if(!(t = findTrainNamed(buff))) {
		wxSnprintf(status_line, sizeof(status_line)/sizeof(wxChar), L("Cannot assign %s: train not found."), buff);
		do_alert(status_line);
		return;
	    }
	    if(*cmd == ',') {
		while(*++cmd == ' ' || *cmd == '\t');
	    } else {
		if(!t->stock) {
                    wxSnprintf(status_line, sizeof(status_line)/sizeof(wxChar), L("Train %s has no default stock assignment."), t->name);
		    do_alert(status_line);
		    return;
		}
		cmd = t->stock;
	    }
	    if(!(t1 = findTrainNamed(cmd))) {
		wxSnprintf(status_line, sizeof(status_line)/sizeof(wxChar), L("Cannot assign %s: train not found."), cmd);
		do_alert(status_line);
		return;
	    }
	    save_assign_train(t1, t);
//	    invalidate_field();
//	    repaint_all();
	} else if(!wxStrncmp(cmd, wxT("play"), 4)) {
	    cmd += 4;
	    while(*cmd == ' ') ++cmd;
	    traindir->PlaySound(cmd);
	} else if(!wxStrcmp(cmd, wxT("skip"))) {
	    skip_to_next_event();
	} else if(!wxStrcmp(cmd, wxT("save_perf_text"))) {
	    traindir->SavePerfText();
	} else if(!wxStrncmp(cmd, wxT("split"), 5)) {
	    int length;

	    for(cmd += 5; *cmd == wxT(' ') || *cmd == wxT('\t'); ++cmd);
	    x = 0;
	    while(*cmd && *cmd != ',') {
		buff[x++] = *cmd++;
	    }
	    buff[x] = 0;
	    if(!(t = findTrainNamed(buff))) {
		// trace(L("Cannot split %s: train not found."));
		return;
	    }
	    if(*cmd == ',') {
		while(*++cmd == ' ' || *cmd == '\t');
		length = wxAtoi(cmd);
	    } else {
		length = 0;
	    }
	    split_train(t, length);
	} else if(!wxStrncmp(cmd, wxT("script"), 6)) {
	    wxChar *end;
	    for(cmd += 6; *cmd == ' ' || *cmd == '\t'; ++cmd);
	    x = wxStrtol(cmd, &end, 10);
	    if(*end == ',') ++end;
	    y = wxStrtol(end, &end, 10);
	    while(*end == ' ' || *end == '\t') ++end;
	    if(!*end)
		return;
	    trk = find_track(layout, x, y);
            if (!trk)
                return;
	    switch(trk->type) {
	    case TRACK:
	    case TRIGGER:
	    case SWITCH:
            case TSIGNAL:
		trk->RunScript(end);
	    }

	} else if(match(&cmd, wxT("showinfo"))) {
	    TDFile	infoFile(cmd);

	    infoFile.SetExt(wxT(".htm"));
	    if(infoFile.Load()) {
		traindir->m_frame->ShowHtml(L("Scenario Info"), infoFile.content);
		info_page = infoFile.name.GetName();
	    }
	} else if(match(&cmd, wxT("showalert"))) {
	    traindir->AddAlert(cmd);
	} else if(match(&cmd, wxT("clearalert"))) {
	    traindir->ClearAlert();
        } else if(match(&cmd, wxT("switch"))) {
	    wxChar *end;
	    cmd = skip_blanks(cmd);
            if(*cmd != '\'') {
	        x = wxStrtol(cmd, &end, 10);
	        if(*end == ',') ++end;
	        y = wxStrtol(end, &end, 10);
                end = (wxChar *)skip_blanks(end);
                if(*end) {
	            SwitchBoard *sw = FindSwitchBoard(end);
	            if(sw)
	                sw->Select(x, y);
                }
            } else {
                end = (Char *)++cmd;
                while(*end && *end != '\'')
                    ++end;
                *end++ = 0;
                end = (wxChar *)skip_blanks(end);
                if(*end) {
	            SwitchBoard *sw = FindSwitchBoard(end);
	            if(sw)
	                sw->Select(cmd);
                }
            }
            server_command_done = true;
	} else {
	    wxSnprintf(status_line, sizeof(status_line)/sizeof(status_line[0]), wxT("Command: %s"), cmd);
	    repaint_labels();
	}
        if(doDelayedRepaint)
            repaint_all();
}

void	trainsim_cmd(const wxChar *cmd)
{
	do_command(cmd, false);  //yangpx 20201103修改 为调试与matlab接口关闭发送任何命令
}


//2字节拆分为单字节
int twoByte2single(char *msg,unsigned short buff,int index){
	char a = (buff>>8)&0xff;
	char b = buff&0xff;
	msg[index++] = buff&0xff;
	msg[index++] = (buff>>8)&0xff;
	return index;
}
//4字节拆分为单字节
int fourByte2single(char *msg,int buff,int index){
	for(int i = 0;i<=3;i++){
	msg[index++] = (buff>>(8*i))&0xff;
	}
	return index;	
}

//赋值发送数组
void add_send_msg(TrainMessage *m,byte cmd,char *name,int distance,int time,int initialSpeed,int terminalSpeed,int accuracy,int start,int end,int spdNum,int spdLimit[15][3]){
	
	/*TrainMessage *m;
	m=(TrainMessage *)malloc(sizeof(TrainMessage));*/
	//车辆ID
	m->cmd = cmd;
	m->name = name;
	m->distance = distance;
	m->time = time;
	m->initialSpeed = initialSpeed;
	m->terminalSpeed = terminalSpeed;
	m->accuracy = accuracy;
	m->start = start;
	m->end = end;
	m->spdNum = spdNum;
	
	for(int i = 0;spdLimit[i][2]>0;i++){
	m->spdLimit[i][0] = spdLimit[i][0];
	m->spdLimit[i][1] = spdLimit[i][1];
	m->spdLimit[i][2] = spdLimit[i][2];
	}

	//msg = m;

}
//重载
void add_send_msg(TrainMessage *m,byte cmd,char *name){
	m->cmd = cmd;
	m->name = name;
}
//udp发送
int set_dataframe(byte cmd,char *msg,TrainMessage* m,int i){

	unsigned short trainId;

	if(cmd == 0x51 || cmd == 0x52 || cmd == 0x53 || cmd == 0x54){
	//车辆ID
	trainId = atoi(m->name+1);
	i = twoByte2single(msg,trainId,i);

	//到目标点距离distance,time,initialSpeed,terminalSpeed,accuracy,start,end,spdNum;
	i = fourByte2single(msg,m->distance,i);

	//到目标时间
	i = fourByte2single(msg,m->time,i);
	
	i = fourByte2single(msg,m->initialSpeed,i);

	i = fourByte2single(msg,m->terminalSpeed,i);

	i = fourByte2single(msg,m->accuracy,i);

	i = fourByte2single(msg,m->start,i);

	i = fourByte2single(msg,m->end,i);

	i = fourByte2single(msg,m->spdNum,i);

	for(int index = 0;index<m->spdNum;index++){
		i = fourByte2single(msg,m->spdLimit[index][0],i);
		i = fourByte2single(msg,m->spdLimit[index][1],i);
		i = fourByte2single(msg,m->spdLimit[index][2],i);
	}
	}

	if(cmd == 0x5A){
	trainId = atoi(m->name+1);
	i = twoByte2single(msg,trainId,i);
	}
	return i;
}
	unsigned char packetId = 0;
void test_send(){

	char msg_51[100],msg_52[100],msg_53[100],msg_54[100],msg_5A[100];

	int spdLimit[15][3];
	
	//byte cmd,packetId,trainNum;
	int i=0;
	unsigned short trainId;
	int distance,time,v_start,v_end,x0,x1,limitNum;

	
	TrainMessage *m;
	m=(TrainMessage *)malloc(sizeof(TrainMessage));
	memset(m, 0, sizeof(TrainMessage));

	msg_51[0] = 0x51;
	msg_52[0] = 0x52;
	msg_53[0] = 0x53;
	msg_54[0] = 0x54;
	msg_5A[0] = 0x5A;

	int size1,size2,size3,size4,sizea;
	size1 = size2 =size3 =size4 =sizea =3;
	int trainNum1,trainNum2,trainNum3,trainNum4,trainNuma;
	trainNum1=trainNum2=trainNum3=trainNum4=trainNuma=0;

	if(packetId == 255)
		packetId =0;

	for(int m=0;m<sendMsg.size();m++){
		if(sendMsg[m]->cmd == 0x51){
			msg_51[2] = ++trainNum1;
			size1 = set_dataframe(0x51,msg_51,sendMsg[m],size1);
		}
		if(sendMsg[m]->cmd == 0x52){
			msg_52[2] = ++trainNum2;
			size2 = set_dataframe(0x52,msg_52,sendMsg[m],size2);
		}
		if(sendMsg[m]->cmd == 0x53){
			msg_53[2] = ++trainNum3;
			size3 = set_dataframe(0x53,msg_53,sendMsg[m],size3);
		}
		if(sendMsg[m]->cmd == 0x54){
			msg_54[2] = ++trainNum4;
			size4 = set_dataframe(0x54,msg_54,sendMsg[m],size4);
		}
		if(sendMsg[m]->cmd == 0x5A){
			msg_5A[2] = ++trainNuma;
			sizea = set_dataframe(0x5A,msg_5A,sendMsg[m],sizea);
		}
	}
	if(msg_51[2]>0){
	msg_51[1] = packetId++;
	send_udp1_msg(msg_51,size1,0);  
	}
	if(msg_52[2]>0){
	Sleep(500);
	msg_52[1] = packetId++;
	send_udp1_msg(msg_52,size2,0);  
	}
	if(msg_53[2]>0){
	Sleep(500);
	msg_53[1] = packetId++;
	send_udp1_msg(msg_53,size3,0);  
	}
	if(msg_54[2]>0){
	Sleep(500);
	msg_54[1] = packetId++;
	send_udp1_msg(msg_54,size4,0);  
	}
	if(msg_5A[2]>0){
	Sleep(500);
	msg_5A[1] = packetId++;
	send_udp1_msg(msg_5A,sizea,0);  
	}

	if(msg_51[2]>0 || msg_52[2]>0 || msg_53[2]>0 || msg_54[2]>0){
	waitAto = 1;
	do_command(wxT("run"), false);
	}
	
}

//所有列车速度位置数据
int setTrainVP(char*msg,int startByte){
	int i =startByte,travelled = 0,number = 0;
	const wxChar *nextTime;
	short size =0;
    i = twoByte2single(msg,number,i);//列车总数

	for(Train* t = schedule; t; t = t->next) {
		if(t->status == train_ARRIVED)
			continue;
		if(!t->position)
			break;
		size = short(strlen(t->name));
		i = twoByte2single(msg,size,i);  //车次号长度
		
		strcpy(msg+i,t->name);//车次号
		i+=size;

		if(t->position->isstation)//是否站内
		{   travelled = 0;
			msg[i++] = 2;
		}
		else{
			travelled =t->curve_travelled;
			msg[i++] = 1;
		}

		if(atoi(t->name+1)%2==0)//上下行
			msg[i++] =0;
		else
			msg[i++] =1;

		i= fourByte2single(msg,int(t->lastkm),i);

		i= fourByte2single(msg,int(travelled),i);
		
		i= fourByte2single(msg,int(t->curspeed),i);

		nextTime = format_time(t->estNextArrival);//预计时间
		msg[i++] = char(atoi(nextTime+1));
		msg[i++] = char(atoi(nextTime+4));
		msg[i++] = char(atoi(nextTime+7));

		for(int m=0;m<4;m++){//预留信息
		msg[i++] = 0;
		}
		number++;
		
	}
    twoByte2single(msg,number,startByte);//列车总数修正
	return i-startByte;
}
//到站报点
int setTrainAD(char*msg,int startByte){
	char status = 0x52;
	const wxChar *arrival0,*departure0,*arrival,*departure;
	short number =0,size=0,deta=0,detd =0;
	int i =startByte;
	Train* t;

	msg[i++] = status;
	number = short(sendADMsg.size());
    i = twoByte2single(msg,number,i);//列车总数

	for(int x=0;x<sendADMsg.size();x++){
		t = sendADMsg[x];
	    size = short(strlen(t->name));
		i = twoByte2single(msg,size,i);  //车次号长度
		strcpy(msg+i,t->name);//车次号
		i+=size;

		size = short(strlen(t->pre_stop));//车站名称长度
		i = twoByte2single(msg,size,i);  //
		strcpy(msg+i,t->pre_stop);//车站
		i+=size;

		if(t->position){
		i = fourByte2single(msg,int(t->position->km),i);//公里标
		}else
		{
			i = fourByte2single(msg,int(t->lastkm),i);//公里标
		}
		msg[i++] = t->ADflag;
		

		arrival0 = format_time(t->nxtstop->arrival0);//计划到达时间
		msg[i++] = char(atoi(arrival0+1));
		msg[i++] = char(atoi(arrival0+4));
		msg[i++] = char(atoi(arrival0+7));

		if(t->ADflag==0||t->ADflag == 4||t->ADflag == 2){
		arrival = format_time(current_time);//实际到达时间
		msg[i++] = char(atoi(arrival+1));
		msg[i++] = char(atoi(arrival+4));
		msg[i++] = char(atoi(arrival+7));


		deta = (current_time - t->nxtstop->arrival0)/60;//到达偏移时间
		i = twoByte2single(msg,deta,i);
		}
		else{
		for(int x=0;x<5;x++)
			msg[i++] = 0;
		}
		
		departure0 = format_time(t->nxtstop->departure0);//计划发车时间
		msg[i++] = char(atoi(departure0+1));
		msg[i++] = char(atoi(departure0+4));
		msg[i++] = char(atoi(departure0+7));

		if(t->ADflag==1||t->ADflag==3){ //发车时和通过时
		departure = format_time(current_time);//实际发车时间
		msg[i++] = char(atoi(departure+1));
		msg[i++] = char(atoi(departure+4));
		msg[i++] = char(atoi(departure+7));


		detd = (current_time - t->nxtstop->departure0)/60;//发车偏移时间
		i = twoByte2single(msg,detd,i);
		}
		else{
		for(int x=0;x<5;x++)
			msg[i++] = 0;
		}
		if(t->ADflag == 4)//全部到达时
			msg[i++] = 1;
		else
			msg[i++] = 0;
		for(int x=0;x<3;x++)//预留字段
			msg[i++] = 0;
		
		if(t->ADflag == 4){
		do_command(wxT("t0"), false);
		do_command(wxT("fast"), false);
		do_command(wxT("fast"), false);
		do_command(wxT("fast"), false);
		}
	}
	return i-startByte;
}
//决策预报包
//所有列车速度位置数据
int setTrainPre(char*msg,int startByte){
	int i =startByte,travelled = 0,nextkm=0;
	const wxChar *nextTime,*planTime,*arrival0,*departure0;
	wxChar *station;
	short size =0,number = 0,pretime=0;
	Train *t;

	number = short(sendDecMsg.size());
    i = twoByte2single(msg,number,i);//列车总数

	for(int x=0;x<sendDecMsg.size();x++){
		t = sendDecMsg[x];
		size = short(strlen(t->name));
		i = twoByte2single(msg,size,i);  //车次号长度
		
		strcpy(msg+i,t->name);//车次号
		i+=size;

		msg[i++] = t->decflag;//决策标志位 1：进站决策 2：发车决策

		if(t->position->isstation)//是否站内
		{   travelled = 0;
			msg[i++] = 2;
			pretime = PRETIME2;
			nextkm = t->lastkm;
		}
		else{
			travelled =t->curve_travelled;
			msg[i++] = 1;
			pretime = PRETIME1;
			nextkm = t->nxtstop->km_post;
		}

		if(atoi(t->name+1)%2==0)//上下行
			msg[i++] =0;
		else
			msg[i++] =1;

		i= fourByte2single(msg,int(t->lastkm),i);//前方和后方车站公里标
		i= fourByte2single(msg,nextkm,i);

		i= fourByte2single(msg,int(travelled),i);
		
		i= fourByte2single(msg,int(t->curspeed),i);

		i= fourByte2single(msg,pretime,i);

		nextTime = format_time(t->estNextArrival);//预计时间
		msg[i++] = char(atoi(nextTime+1));
		msg[i++] = char(atoi(nextTime+4));
		msg[i++] = char(atoi(nextTime+7));

		//列车股道
		if(t->decflag == 1){
			planTime = format_time(t->nxtstop->departure0);//计划时间
			station = t->nxtstop->station;
		}else{
			planTime = format_time(t->nxtstop->arrival0);//计划时间
			station = t->position->station;
		}
		while(*station && *station != '@')
			station++;

		msg[i++] = char(atoi(planTime+1));//赋值计划时间
		msg[i++] = char(atoi(planTime+4));
		msg[i++] = char(atoi(planTime+7));
		msg[i++] = atoi(++station);       //赋值股道编号

		//for(int m=0;m<3;m++){//预留信息
		//msg[i++] = 0;
		//}
		number++;
		
		
	}
    
	return i-startByte;
}

int sendTrainMsg(char flag,char*msg1){
	char msg[512];
	const char *time;
	int size =0 ,startByte =14; //数据长度和开始字节
	memset(msg,0,sizeof(msg));

	msg[0] = msg[1] = 0xEF;
	msg[2] = flag;
	msg[3] = 1;//总包数 暂未分包，
	msg[4] = 0;
	msg[5] = 1;//当前包号
	msg[6] = 0;
	time = format_time(current_time);//时间
	msg[7] = char(atoi(time+1));//时
	msg[8] = char(atoi(time+4));//分
	msg[9] = char(atoi(time+7));//秒

	if(flag == 0x01)//数据包类型
	size = setTrainAD(msg,startByte);
	else if(flag == 0x02)
	size = setTrainVP(msg,startByte);
	else if(flag == 0x03)
	size = setTrainPre(msg,startByte);
	else if(flag == 0x04);
	else
		return 0;

	fourByte2single(msg,size,10);//数据长度
	msg[size+startByte] =msg[size+startByte+1]= 0xFD;//包尾
	//send_msg(msg,size+16);//发送数据
	
	if(flag==3){ //强化学习交互数据 暂时修改
	waitAction = 1;
	do_command(wxT("run"), false);
	}
	memcpy(msg1,msg,(size+16));
	return size+16;
}

void send_DMI(Train *t){
  char cmd,year,month,day,hour,minute,second;
  char msg[50];
  int size = 0,position,speed;
  unsigned short trainId;

  cmd = 0x55;
  trainId = atoi(t->name+1);
  year = month = day = hour = minute = second =0;
  position = int(t->temptravelled);
  speed = int(t->curspeed);

 /* TrainMessage *m;
  m=(TrainMessage *)malloc(sizeof(TrainMessage));*/
  memset(msg, 255, 20);

  msg[0] = cmd;
  msg[1] = packetId;
  //预留时间位//
  size += 2;
  trainId = atoi(t->name+1);
  size = twoByte2single(msg,trainId,size);
  size += 7;

  size = fourByte2single(msg,position,size);

  size = twoByte2single(msg,speed,size);
  
  send_udp2_msg(msg,size+2);

}



