/*	trsim.h - Created by Giampiero Caprino

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

#ifndef _TRSIM_H
#define _TRSIM_H

#include "color.h"
#include "defs.h"
#include "Options.h"
#include "Array.h"
#include "Shape.h"
#include <vector>
class	HtmlPage;
class	wxString;

#define	PLATFORM_SEP	wxT('@')

typedef enum {   //自动恢复的故障类型ypx
    NOERR =0,
	TRKFIND = 1,
	TRKSELECT = 2,
	SELECT = 3,
	ITINERARY =4,
}etype;


typedef enum {
	NOTRACK = 0,
	TRACK = 1,
	SWITCH = 2,
	PLATFORM = 3,
	TSIGNAL = 4,
	TRAIN = 5,
	TEXT = 6,
	LINK = 7,		/* not a real track - for the editor */
	IMAGE = 8,		/* for stations, bridges etc. */
	MACRO = 9,		/* editor only - not to be saved */
	ITIN = 10,		/* itinerary */
	TRIGGER = 11,		/* trigger point linked to track */
	MOVER = 12,		/* not a real track - for the editor */
        POWERTOOL = 13,         /* not a real track - for the editor */
        TRACKLENTOOL = 14       /* not a real track - for the editor */
} trktype;

typedef enum {
	NODIR = 0,
	W_E = 1,
	NW_SE = 2,
	SW_NE = 3,
	W_NE = 4,
	W_SE = 5,
	NW_E = 6,
	SW_E = 7,
	TRK_N_S = 8,
#define	E_W	NODIR

	signal_WEST_FLEETED = 9,
	signal_EAST_FLEETED = 10,
#define	N_S_W	signal_WEST_FLEETED
#define	N_S_E	signal_EAST_FLEETED
	SW_N = 11,
	NW_S = 12,
	SE_N = 13,
	NE_S = 14,
	N_S = 16,		/* must be 16 because of signals */
	S_N = 17,
	signal_SOUTH_FLEETED = 18,
	signal_NORTH_FLEETED = 19,
	XH_NW_SE = 20,
	XH_SW_NE = 21,
	X_X = 22,		/* X (no switch) */
	X_PLUS = 23,		/* + (no switch) */
	N_NE_S_SW = 24,		// no switch / |
	N_NW_S_SE = 25		// no switch \ |
} trkdir;

typedef enum {
	ST_FREE = 0,
	ST_BUSY = 1,
	ST_READY = 2,
	ST_WORK = 3,
	ST_GREEN = 4,
	ST_RED = 5,
	ST_WHITE = 6,
	ST_APPROACH_ROAD = 7,//lrg 20210220
} trkstat;

typedef enum {
	train_READY,
	train_RUNNING,
	train_STOPPED,
	train_DELAY,
	train_WAITING,
	train_DERAILED,			/* couldn't place on territory! */
	train_ARRIVED,			/* reached some destination */
        train_STARTING,                 // starting after a stop at signal
		train_STOP_OUTSTA,
	/*train_SHUNTING*/		/* going to next station at 30Km/h */
} trainstat;

struct TrackBase;

#include "TVector.h"
#include "TimeTable.h"

#define	NTTYPES	    10

#define	MAXNOTES    5

#define	MAX_FLASHING_ICONS  4

#define	TFLG_TURNED 1			/* train changed direction */
#define	TFLG_THROWN 2			/* switch was thrown */
#define	TFLG_WAITED 4			/* train waited at signal */
#define	TFLG_MERGING 8			/* train is shunting to merge with another train */
#define	TFLG_STRANDED 16		/* material left on track without engine */
#define	TFLG_WAITINGMERGE   32		/* another train is approaching us to be merged */
#define	TFLG_ENTEREDLATE    64		/* don't penalize for late arrivals */
#define	TFLG_GOTDELAYATSTOP 128		/* only select delay (or none) once */
#define	TFLG_SETLATEARRIVAL 256		/* only compute late arrival once */
#define	TFLG_SWAPHEADTAIL   512		/* swap loco and caboose icons */
#define TFLG_DONTSTOPSHUNTERS 1024      // don't stop here if train is shunting

class Track;				// scripted track. derived from TrackBase
class Signal;				// scripted signal. derived from TrackBase
class ExprValue;

struct Speedlim{

 Speedlim *next;

 int      LineId,KmBegin,KmEnd,speed;

};


struct TrackBase {
	TrackBase();
	virtual ~TrackBase() { };

	Track	*next;
	Track	*next1;		/* list of same type tracks */
	Pos	x, y;
	Pos	xsize, ysize;
	trktype	type;
	trkdir	direction;
	trkstat	status;
	Pos	wlinkx, wlinky;
	Pos	elinkx, elinky;
	char	isstation:2;
	char	switched:2;
	char	busy:2;
	char	fleeted:2;
	char	nowfleeted:2;
	char	norect:2;		/* switches have a rectangle around em*/
	char	fixedred:2;		/* signal is always red */
	char	nopenalty:2;		/* no penalty for train stopping at signal */
	char	noClickPenalty:2;	/* no penalty for un-necessary clicks */
	char	invisible:2;		/* object is not shown on layout */
	char	wtrigger:2;		/* westbound trigger linked */
	char	etrigger:2;		/* eastbound trigger linked */
	char	signalx:2;		/* use 'x' version when drawing signal */
	char	aspect_changed:2;	/* ignore script execution - TODO: remove */
	int	flags;			/* performance flags (TFLG_*) */
	wxChar	*station;
	void	*lock;
	short	speed[NTTYPES];
	int	icon;
	int	length;
	Signal	*wsignal;		/* signal controlling this track */
	Signal	*esignal;		/* signal controlling this track */
	Track	*controls;		/* track controlled by this signal */
	grcolor	fgcolor;
	void	*pixels;		/* for IMAGE pixmap */
	long	km;			/* station distance (in meters) */
    int    line_num;
	Char	*stateProgram;		/* 3.5: name of function describing state changes */
	const Char *_currentState;	/* 3.5: name of current state in state program */
        const Char *_prevState;         /* 3.8q: signal state before update loop */
	void	*_interpreterData;	/* 3.5: intermediate data for program interpreter */
	bool	_isFlashing;		/* 3.5: flashing signal */
	bool	_isShuntingSignal;	/* 3.5: only affects shunting trains */
	int	_nextFlashingIcon;	/* 3.5: index in list of icons when flashing */
	wxChar	*_flashingIcons[MAX_FLASHING_ICONS];	// 3.8: array of flashing icon names

	int	_fontIndex;		// 3.6: font selection for TEXT tracks 

        wxChar  *_lockedBy;             // 3.7q: signal is locked by other signal(s)
        bool    _intermediate;          // 3.8h: signal is intermediate
        int     _nReservations;         // 3.8h: number of trains still expected to pass this signal
        const Char *power;              // 3.9: motive power allowed (diesel, electric)
        //const Char *km_post;  		    //YangPX  公里标
        double  gauge;                  // 3.9: track gauge
        int     _maxPathHint;           // 3.9f: pre-allocate path vector

        Shape   *_shape;                // 3.9j: Shape-based drawing
        short   _leftDwellTime;         // 3.9j: delay until next state switch
        short   _dwellTime;             // 3.9j: dwell time for this element
        trkdir  _pathDir;               // 3.9n: direction of locked path (when not black)

	bool	GetPropertyValue(const wxChar *prop, ExprValue& result) { return false; }
	bool	SetPropertyValue(const wxChar *prop, ExprValue& val) { return false; }
        const Char *GetTypeName() const;
        bool    ToggleDwelling();
        bool    IsDwelling() const;
};

struct  TrainInfo {
        Char	entering_time[40];
        wxChar	leaving_time[20];
        wxChar	current_speed[20];
        wxChar	current_delay[20];
        wxChar	current_late[20];
        wxChar	current_status[250];
};

struct  Train;

class   AssignInfo {
public:
        Train	*assign_tr;     // train to be assigned
        int     assign_to;      // train receiving the rolling stock (index into assign_list[])
        Track   *track;         // pointer to station this train is stopped at
        Array<Train *> candidates;

        AssignInfo() {
            assign_tr = 0;
            assign_to = 0;
            track = 0;
        }
};

#define	MAX_DELAY 10
#define DELAY_CHAR  '!'

typedef struct	_delay {
	short	nDelays;	/* how many entries are in prob[] and seconds[] */
	short	nSeconds;	/* # of seconds selected for this delay (from seconds[] */
	short	prob[MAX_DELAY];/* probability[i] from 0=never to 100=always */
	short	seconds[MAX_DELAY];/* nseconds per each probability */
	/* TODO: add a script to evaluate the probability and/or nseconds */
} TDDelay;

typedef struct	_trainstop {
	struct	_trainstop *next; 
	wxChar	*station;	/* stop at this station */
	long	arrival;	/* scheduled arrival time */
	long	departure;	/* scheduled departure time */
	long	arrival0;	/* 计划到达 */
	long	departure0;	/* 计划离开 */
	long	arrival_actual;	/* 实际到达 *///lrg 20211018
	long	departure_actual;	/* 实际离开 *///lrg 20211018
	int     delta1;      //停站随机扰动
	int     delta2;     //运行随机扰动
	long	minstop;	/* minimum number of sec. stopping at station */
	char	stopped;	/* we did indeed stop here */
	char	late;		/* we were late arriving here */
	double  km_post;    // 停站公里标
	int     line;       //所在线路编号
	int	delay;		/* delay arriving at this station */
	TDDelay	*depDelay;	/* random departure delay, if any */
} TrainStop;
//YangPX 调试按计划行车
typedef struct	_ATO{
   struct  _ATO *next;
   wxChar	*station;
   double    t;
   double    v;
   double    s;
   long    a;
}ATOturn;
typedef struct TrainMessage{
	TrainMessage();
	~TrainMessage();
	TrainMessage *next;
	wxChar *name;
	unsigned char cmd;
	int distance,time,initialSpeed,terminalSpeed,accuracy,start,end,spdNum;
	int spdLimit[5][3];


}TrainMessage;

struct Train {
	Train();
	~Train();

        void    Get(TrainInfo& info);
        bool    Get(AssignInfo& info);
        const Char *GetStatusName() const;

	Train	*next;
	wxChar	*name;		/* train name or number */
	trainstat status;	/* status: running, waiting etc. */
	trkdir	sdirection;	/* starting direction: W_E or E_W */
	trkdir	direction;	/* current direction: W_E or E_W */
	int	timein;		/* time it shows up on territory */
#define tailentry timein	/* overload timein/out in t->tail structure */
#define tailexit timeout
	int	timeout;	/* time it should be out of territory */
	int timeout0;
	wxChar	*entrance, *exit;
	wxChar	*exited;	/* if wrongdest, where we exited */
        wxChar  *alternateExits;// 3.9r: no penalties for using an alternateExit
        wxChar  *alternateEntries;// 3.9r: avoid delayed entries, if possible
	int	timeexited;	/* when we exited */
	wxChar	*notes[MAXNOTES];
	char	nnotes;
	char	wrongdest;	/* train arrived at wrong destination */
	char	type;		/* train type */
	char	_gotDelay;	/* we computed a delay upon entry in the territory */
        int     _lastUpdate;    /* when we last updated this train's status */
	short	_inDelay;	/* the computed delay, in minutes */
	short	newsched;	/* must update schedule window for this train */
//	unsigned short	curspeed;   /* current speed */
	unsigned short	maxspeed;   /* absolute maximum speed 时刻表中的最大速度*/
	unsigned short	curmaxspeed;/* current (absolute or track) maximum speed 在speedlimit中更新*/
	unsigned short	speedlimit; /* last speed limit seen */
	unsigned short	timelate;   /* minutes late arriving at all stations */
	unsigned short	timedelay;  /* minutes late entering territory */
	unsigned short	timered;/* minutes stopped at red signal */
        short   startDelay;     // seconds to wait until starting after stop (runtime - set from train schedule or train type)
        short   myStartDelay;   // seconds this train should wait (from schedule file)
	double	trackpos;	/* how much of lengthy tracks we travelled */
        double  curspeed;       // 3.8r: support different acceleration rates
        double  accelRate;      // 3.8r: set from train schedule or train type

        double  acceleration;      //20200330 by YangPX 实时变化加速度
	ATOturn   *ATOturn;            //20200412 by YangPX for 列控调试
	double  *CalculatedSpeedCurveValue;
	int estNextArrival; //下一站预计到达时间
	char decflag;
	TrainStop *stops;	/* list of scheduled stops */
	//TrainStop *stops0;  //计划图
	TrainStop *laststop;	/* last in list of scheduled stops 倒数第二站*/
    TrainStop *nxtstop;  //  当前行驶的下一站
    wxChar	*pre_stop;   // 当前行驶上一站
	wxChar  *passstp;    //经过站
	wxChar  *last_it;
	etype     errtype;
	int stop_flag;  //  列车停止的瞬间
	bool cross_flag;  //列车进入过路站瞬间
    bool  itflag; //列车是否触发了进路  
	float sta_waittime;//用于保存调度算法传输过来的晚点列车在晚点车站的等待时间
	long int it_starts;//列车所进入itinerary的起止横坐标   lrg 20210218
	long int it_ends;
	int selectline_flag;//设置列车是否进行转线操作(是否执行selectline)的标志位 lrg20210221
	short	length;		/* current train length in meters */
	short	entryLength;	/* original train length in meters (for Restart) */
//	short	pathpos;	/* index into path[] of train head */
	Vector	*path;		/* track elements to be travelled by train head */
	Vector	*path1;
    Vector	*checkbusy;   //ypx  for atp
	Train	*tail;	/* descriptor of train's end (if length != 0) */
	double	pathtravelled;	/* meters travelled in current path */
	double  temptravelled;    //绘图用 
	double  sta_travelled;  //停车站间行驶距离
	double  curve_travelled;  //每段曲线已走距离，每计算一次曲线则置零
	double  curposition;
	double  lastkm;        //上一站公里标
	int     lastline_No;      //上一站线路编号
    int 	run_time;  //站间已经过时间
	int     stop_to_run;
	Speedlim   *speed_lim;  //列车限速
	Signal  *nextsig;           //ypx
	char    ADflag;//到发标志位
	double	disttostop;	/* distance until next stop */
	double	disttoslow;	/* distance until next speed limit signal */
	Track	*stoppoint;
	Track	*slowpoint;
	Track	*position;	/* where the train is in the territory */
	int	timedep;	/* expected time of departure from station */
	Vector	*fleet;		/* list of signals waiting for tail to pass */
	Char	*waitfor;	/* must wait for this train to exit territory*/
	int 	waittime;	/* how many minutes after waitee has arrived we depart */
	Char	*stock;		/* next train which uses this train's stock */
	Shape	*epix, *wpix;	/* indexes to east and west pixmaps */
	Shape	*ecarpix, *wcarpix;/* indexes of east abd west car pixmaps (if length != 0) */
	Track	*outof;		/* ignore this station when checking shunting */
	Track	*stopping;	/* we are stopping/stopped at this station */
	trainstat oldstatus;
	char	arrived;	/* if true we are just shunting */
	char	shunting;
	char	days;		/* which day this train is running */
	char	needfindstop;	/* terrible hack! */
        char    isExternal;     /* does not run in this scenario */
	short	flags;		/* performance flags (TFLG_*) */
	Train	*merging;	/* will merge with this train */
	TDDelay	*entryDelay;	/* random entry time delay, if any */
        const Char *power;      // 3.9: motive power of locomotive
        double  gauge;          // 3.9: track gauge

	// start to use C++ methods to make the code cleaner

	Char	*stateProgram;
	void	*_interpreterData;

	// REMEMBER TO CLEAR NEW FIELDS IN CONSTRUCTOR!

        bool    CanTravelOn(Vector *path);  // 3.9: check motive power on path

        // 3.9r: allow alternate entry/exits
        bool    IsAllowedAlternate(const Char *name, const Char *alternates);
        wxString& GetNameAndAlternates(wxString& out, const Char *name, const Char *alternates);

	void	SetTooltip();	// print extended info about the train

	void	ParseProgram();

	bool	GetPropertyValue(const wxChar *prop, ExprValue& result);
	bool	SetPropertyValue(const wxChar *prop, ExprValue& val);

	void	OnStart();

	void	OnStopped();

	void	OnWaiting(Signal *sig);

	void	OnEntry();

	void	OnExit();

	void	OnArrived();

        void    OnMerged();

	void	OnAssign();

	void	OnReverse();

	void	OnShunt();
};

class Itinerary;

typedef struct _textlist {
	struct _textlist *next;
	wxChar	*txt;
} TextList;

typedef struct _Trig {
	struct _Trig *next;
	int	type;
	wxChar	*action;
	Pos	wlinkx, wlinky;
	Pos	elinkx, elinky;
} Trigger;

struct station_sched {
	struct station_sched *next;
	Train	*tr;
	wxChar	*stopname;	    /* in case different platform */
	long	arrival, departure;
	char	transit;
};

class StationSchedule {
public:
        StationSchedule() { _items = 0; _ignorePlatforms = false; }
        ~StationSchedule() { Free(); }
        void    Build(const Char *station);
        void    Free();

        bool    _ignorePlatforms;
        struct station_sched *_items;
};

class StationList {
public:
        StationList() { _items = 0; _nItems = _maxItems = 0; _ignorePlatforms = false; }
        ~StationList() { Free(); }
        void    Add(Track *t);
        void    Build(bool ignorePlatforms);
        void    Free();

        bool    _ignorePlatforms;
        int     _nItems;
        int     _maxItems;
        Track   **_items;
};

typedef struct _Path {
	struct _Path *next;
	wxChar	*from;
	wxChar	*to;
	wxChar	*enter;
	long	times[NTTYPES];
} Path;

extern	Path	*paths;

class TDSkin {
public:
	TDSkin();
	~TDSkin();

	TDSkin	*next;
	Char	*name;
	int	free_track;		// default: black
	int	reserved_track;		// default: green
	int	reserved_shunting;	// default: white
	int	occupied_track;		// default: orange
	int	working_track;		// default: blue
	int	background;		// default: gray
	int	outline;		// default: dark_gray
	int	text;			// default: black
};

extern	TDSkin	*skin_list;
extern	TDSkin	*curSkin;
extern	TDSkin	*defaultSkin;


/*	Performance data	*/

typedef struct	{
	int	wrong_dest;
	int	late_trains;
	int	thrown_switch;		/* incorrectly thrown switches */
	int	cleared_signal;		/* incorrectly cleared signals */
	int	denied;			/* command denied */
	int	turned_train;
	int	waiting_train;
	int	wrong_platform;
	int	ntrains_late;
	int	ntrains_wrong;
	int	nmissed_stops;
	int	wrong_assign;
} perf;

extern	perf	perf_vals,		/* values for each performance type */
	perf_tot;			/* totals for each performance type */

struct	_conf {
	int	gridxbase, gridybase;
	int	gridxsize, gridysize;

	grcolor	gridcolor;
	grcolor	txtbgcolor;	/* for dialogues */
	grcolor	fgcolor;
	grcolor	linkcolor;	/* links signals and entry/exit */
	grcolor	linkcolor2;	/* links tracks */
};
extern	struct _conf 	conf;

typedef struct _pxmap {
	wxChar	*name;
	void	*pixels;
} pxmap;
extern  pxmap   *pixmaps;
extern  int     npixmaps, maxpixmaps;
extern	pxmap	*carpixmaps;
extern	int	ncarpixmaps, maxcarpixmaps;

extern	TextList *track_info;
extern	BoolOption show_canceled;
extern	BoolOption show_arrived;
extern	BoolOption alert_last_on_top;

	void	init_sim(void);
	void	init_pmaps(void);
	Track	*load_field(const wxChar *name);
    Speedlim *load_spdlim(const wxChar *name);  //ypx
	void	clean_field(Track *layout);
	Track	*find_track(Track *layout, int x, int y);
	void	remove_ext(wxChar *buff);
	Train	*load_trains(const wxChar *name);
	Train	*update_trains(const wxChar *name);
	int	save_layout(const wxChar *name, Track *layout);
	void	save_schedule_status(HtmlPage& out);
	void	show_schedule_status(HtmlPage& out);
	wxChar	**get_all_station_list(void);
	void	invalidate_field(void);
	void	reset_clip_rect(void);
	int	macro_select(void);
	void	macro_place(int x, int y);
	void	track_place(int x, int y);
	void	track_properties(int x, int y);
	void	clean_trains(Train *sched);
	void	create_train(void);
	void	assign_train(Train *old, Train *newtr);
	void	do_alert(const wxChar *msg);
	void	update_labels(void);
	void	update_schedule(Train *t);
	void	repaint_labels(bool force = false);
	void	repaint_field(void);
	int	create_tgraph(void);
	void	tgraph_paint(void);
	int	create_schedule(int assign);
	void	update_alert(void);
	void	update_layout(void);
	void	update_button(const wxChar *btn, const wxChar *lbl);
	void	repaint_all(void);
	int	do_assign_dialog(Train *tr);
const	wxChar	*train_status(Train *t);
	void	do_station_list_print(const wxChar *name, HtmlPage& dest);
	void	print_entry_exit_stations(HtmlPage& page);
	void	print_track_info(HtmlPage& page);
	void	train_print(Train *, HtmlPage& page);
const	wxChar	*train_next_stop(Train *t, int *final);
	Train	*sort_schedule(Train *sched);
	void	fill_schedule(Train *sched, int assign);
	void	reset_schedule(void);
	void	link_signals(Track *layout);
	void	do_itinerary_dialog(int x, int y);
	int	set_itin_name(Itinerary *it, wxChar *name, wxChar *nextit);
	int	set_track_properties(Track *t, wxChar *len,
			wxChar *speed, wxChar *station, wxChar *dist,
			wxChar *wlink, wxChar *elink,wxChar *num);
	void	show_itinerary(void);
	void	click_time(void);

	Track	*track_new(void);
	TrainStop *stop_new(void);
	Train	*train_new(void);
	void	track_delete(Track *);

	wxChar	*format_time(long);
	const wxChar *format_day(int);
	long	scan_time(wxChar **);
	int	parse_time(wxChar **);
	wxChar	*parse_km(Track *t, const wxChar *p);
	Track	**get_station_list(void);
	Track	**get_entry_list(void);
	void	print_train_info(Train *t);
	void	train_draw(Track *t, Train *trn);
	void	layout_paint(Track *t);
	void	trains_paint(Train *t);
	void	track_paint(Track *t);
	void	*get_pixmap(const char **map);
	void	*get_pixmap_file(const wxChar *fname);
	void	delete_pixmap(void *p);
	void	free_pixmaps(void);
	int	get_pixmap_index(const wxChar *mapname);
        void	get_text_size(const wxChar *txt, Coord& sz);
	void	show_table(void);
	void	hide_table(void);
	void	show_tooltable(void);
	void	hide_tooltable(void);

	void	trainsim_cmd(const wxChar *);
	void	start_stop(void);
	void	main_quit_cmd(void);
	void	pointer_at(Coord cell);
	void	track_selected(int x, int y);
	void	track_selected1(int x, int y);
	int	track_shift_selected(Coord& pos);
	int	track_control_selected(Coord& pos);
	void	make_timer(int msec);

	Track	*findTrackType(int x, int y, trktype type);
#define	findTrack(x, y) findTrackType(x, y, TRACK)
#define	findSwitch(x, y) findTrackType(x, y, SWITCH)
#define	findSignal(x, y) (Signal *)findTrackType(x, y, TSIGNAL)
#define	findPlatform(x, y) findTrackType(x, y, PLATFORM)
#define	findText(x, y) findTrackType(x, y, TEXT)
#define	findImage(x, y) findTrackType(x, y, IMAGE)

	Track	*findLinkTo(int x, int y);
	Track	*findTriggerTo(int x, int y);
	Train	*findTrain(int x, int y);
	Train	*findTrainNamed(const wxChar *p);
	Track	*findStation(const wxChar *name);
	Track	*findStationNamed(const wxChar *name);
	Signal	*findSignalNamed(const wxChar *name);
	TrainStop *findStop(Train *trn, Track *trk);
	int	sameStation(const wxChar *s1, const wxChar *s2);
	Track	*track_walkwest(Track *, trkdir *);
	Track	*track_walkeast(Track *, trkdir *);
	Track	*track_walkwest(Track *, trkdir *);
	Track	*swtch_walkeast(Track *, trkdir *);
	Track	*swtch_walkwest(Track *, trkdir *);

	void	default_prefs(void);

	void	shunt_train(Train *);
	void	split_train(Train *, int length);
	void	remove_from_stranded_list(Train *);
	void	skip_to_next_event();

	Vector	*new_Vector(int hint = 0);
	void	Vector_addElement(Vector *v, Track *e, int flag);
	Track	*Vector_elementAt(Vector *v, int i);
	int	Vector_flagAt(Vector *v, int i);
	void	Vector_empty(Vector *v);
	void	Vector_delete(Vector *v);
	void	Vector_deleteElement(Vector *v, int del);
	int	Vector_find(Vector *v, Track *trk);
	void	Vector_merge(Vector *dest, Vector *src);
	void	Vector_Print(Vector *v, wxChar * str, short pos);
	void	Vector_dump(Train *tr, wxChar *suff);
	int	sameSubPath(Vector *v1, Vector *v2);
	Vector	*appendPath(Vector *oldpath, Vector *newelems);
	int	setShuntingPath(Vector *path, int direction, Train *shunting);
	void	colorPathStart(Vector *path, int state, int end);
	int	toggle_signal_auto_itinerary(Signal *t, int do_log);//lrg 20210220


	void	open_all_signals(void);
	void	decelerate_train(Train *t, long val);
	void	accelerate_train(Train *t, long val);
	bool	reverse_train(Train *t);
	bool	is_canceled(Train *t);
	int	save_game(const wxChar *p);
	void	restore_game(const wxChar *p);
	void	check_delayed_entries(Train *sched);
	void	check_layout_errors(void);
	Track	*load_field_tracks(const wxChar *fname, Itinerary **itinList);

	Train	*findTail(int x, int y);
	Train	*findStranded(int x, int y);
	Train	*findStrandedTail(int x, int y);
	Itinerary *find_from_to(Track *t1, Track *t2);
	void	link_all_tracks(Track *layout);
	void	link_all_tracks();

	void	add_to_script(Track *t);
	void	append_to_script(const wxChar *txt);
	bool	parse_script();
	int	ask_number(const wxChar *title, const wxChar *question);
	void	NameWithoutPlatform(wxString& out, const Char *name);
	void	try_itinerary(int sx, int sy, int ex, int ey);
	void	itinerary_selected(Track *t, bool forShunt);

extern	Train	*trains;	/* list of all trains */
extern	Train	*lasttrain;	/* last in list of all trains */
extern	Itinerary *itineraries;

class	wxFFile;
class	TDFile;
extern	wxFFile	flog;
extern	TDFile	*frply;

extern  TrainInfo   train_info;

extern	wxChar	status_line[1024];

extern	const wxChar	*disp_columns[9];

extern	int	editing;
extern	int	editing_itinerary;
extern	int	running;
extern	int	run_points;
extern	int	total_delay;
extern	int	total_late;
extern	int	late_data[60 * 24];	// minutes late accumulated at each minute of the day
extern	int	time_mult;
extern	long	current_time;
extern	long	start_time;
extern	int	cur_time_mult;
extern  short   startDelay[NTTYPES];
extern  double  accelRate[NTTYPES];     // 3.8r: variable acceleration rate for each type

/*	User options	    */

extern	int	signal_traditional;
extern	BoolOption show_speeds;
extern	BoolOption show_blocks;
extern	BoolOption show_seconds;
extern	BoolOption show_icons;
extern	int	show_tooltip;
extern	int	run_point_base;
extern	BoolOption beep_on_alert;
extern	BoolOption beep_on_enter;
extern	int	terse_status;
extern	int	status_on_top;
extern	BoolOption show_grid;
extern	int	auto_link;
extern	int	link_to_left;
extern	int	hard_counters;
extern	int	show_links;
extern	int	show_scripts;
extern  int     save_prefs; 
extern	bool	bShowCoord;
extern	BoolOption trace_script;
extern	int	use_real_time;
extern	int	random_delays;
extern	int	play_synchronously;
extern	int	swap_head_tail;	    // swap head and tail icons when reversing train
extern  BoolOption draw_train_names;
extern  BoolOption small_train_names;
extern  BoolOption no_train_names_colors;
extern  BoolOption no_train_names_background;

extern	Track	*layout;
extern  Speedlim *speedlim;

extern	int	current_macro;
extern	Track	**macros;
extern	int	nmacros, maxmacros;

extern	Train	*schedule;
extern	Train	*stranded;	/* list of rolling stock left behind
				 * by engine after a split operation.
				 */
extern	wxChar	*script_text;	// program-oriented control

extern	wxChar	alert_msg[1024];
extern	wxChar	current_msg[256];
extern	int	run_day;
extern	int	platform_schedule;

extern	void	(*track_properties_dialog)(Track *);
extern	void	(*signal_properties_dialog)(Signal *);
extern	void	switch_properties_dialog(Track *);
extern	void	(*trigger_properties_dialog)(Track *);
extern	void	(*performance_dialog)(void);
extern	void	(*options_dialog)(void);
extern	void	(*select_day_dialog)(void);
extern	void	(*train_info_dialog)(Train *);
extern	void	(*assign_dialog)(Train *);
extern	void	(*station_sched_dialog)(const wxChar *);
extern	void	(*itinerary_dialog)(Itinerary *);
extern	void	(*about_dialog)(void);

extern	int	ntrains_running;
extern  int     ntrains_starting;
extern	int	ntrains_arrived;
extern	int	ntrains_waiting;
extern	int	ntrains_stopped;
extern	int	ntrains_ready;

extern	int	total_track_number; /* to prevent endless loops in findPath() */

extern	Track	*signal_list,
		*track_list,
		*text_list,
		*switch_list;
extern  Track   *track_array[XNCELLS][YNCELLS];

extern	struct edittools *tooltbl;
extern	Track	*tool_layout;

extern	int	is_windows;

extern	const wxChar	*version, *host;

extern	wxString	program_name;
extern	wxString	current_project;
extern	FileOption      searchPath;
extern	wxChar		tooltipString[1024];

#include "TSignal.h"

void	signal_unlock(Signal *trk);
void	fill_itinerary(Itinerary *it, Signal *tr);
bool	start_running_now(Train *tr);
bool	can_start_running_now(Train *tr);

struct RunInfoStr {
        Char    time_msg[128];
        Char    points_msg[128];
};
extern  RunInfoStr gRunInfoStr;

void    printRunInfo(RunInfoStr& out);

#include "ui.h"

#endif /* _TRSIM_H */

