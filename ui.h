/*	ui.h - Created by Giampiero Caprino

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

#ifndef _UI_H
#define	_UI_H

struct tr_rect {
	int	left, top;
	int	right, bottom;
};

struct VLines {
	long	x0, y0;
	long	x1, y1;
};

typedef enum {
	SEG_N = 0,
	SEG_NE = 1,
	SEG_E = 2,
	SEG_SE = 3,
	SEG_S = 4,
	SEG_SW = 5,
	SEG_W = 6,
	SEG_NW = 7,
	SEG_END = 8
} SegDir;

typedef struct {
	wxChar	*text;
	wxChar	oldtext[256];
	void	*handle;
} TrLabel;

struct edittools {
	int	type;
	int	direction;
	int	x, y;
	Track	*trk;
};

struct clist {
	wxChar	*title;
	wxChar	**headers;
	int	*col_width;
	wxChar	*(*col_string)(int row, int col, void *ptr);
};

extern	struct tr_rect cliprect;
extern	struct edittools *tooltbl;
extern	TrLabel	labelList[];	/* for messages, scores etc. */

#define ANSWER_NO 0
#define	ANSWER_YES 1

void	init_pmaps(void);
void	tr_fillrect(int x, int y);
void	draw_layout(int x, int y, VLines *lns, int color);
void	draw_pixmap(int x, int y, void *pmap);
void	get_pixmap_size(void *map, Coord& sz);

void	car_draw(Track *trk, Train *trn);
void	alert_beep(void);
const wxChar	*train_status0(Train *t, int full);
void	gr_update_schedule(Train *t, int i);
void	check_layout_error(void);
void	hide_itinerary();
int	ask(const wxChar *msg);
void	create_path_window(void);
int	cont(const wxChar *msg);
void	track_info_dialogue();
void	new_status_position();

void	change_coord(int x, int  y, int w = 3, int h = 1);
void	do_replay();
void	enter_beep();

void	getcolor_rgb(grcolor col, int *r, int *g, int *b);
void	draw_link(int x1, int y1, int x2, int y2, grcolor col);
void	draw_layout_text1(int x, int y, const wxChar *txt, int size);
void	draw_layout_text_font(int x, int y, const wxChar *txt, int fontIndex);
void	draw_text_with_foreground(int x, int y, const wxChar *txt, int size, int fgcolor);
void	draw_text_with_background(int x, int y, const wxChar *txt, int size, int bgcolor);
void    draw_mid_point(int x, int y, int dx, int dy, int col);
void	select_tool(int);

#endif /* _UI_H */

