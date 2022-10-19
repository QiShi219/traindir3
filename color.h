/*	color.h - Created by Giampiero Caprino

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

#ifndef _COLOR_H
#define _COLOR_H

typedef	int	grcolor;

extern	grcolor	color_white;
extern	grcolor	color_black;
extern	grcolor	color_green;
extern	grcolor	color_yellow;
extern	grcolor	color_red;
extern	grcolor	color_orange;
extern	grcolor	color_brown;
extern	grcolor	color_gray;
extern	grcolor	color_lightgray;
extern	grcolor	color_darkgray;
extern	grcolor	color_blue;
extern	grcolor	color_cyan;
extern  grcolor color_magenta;
//extern	grcolor	color_red1;
extern	grcolor	color_bise;//lrg 20210215
extern	grcolor	color_approach;//lrg 20200220


typedef enum {
	COL_BACKGROUND = 0,
	COL_TRACK = 1,
	COL_GRAPHBG = 2,
	COL_TRAIN1 = 3,
	COL_TRAIN2 = 4,
	COL_TRAIN3 = 5,
	COL_TRAIN4 = 6,
        COL_TRAIN5 = 7,
        COL_TRAIN6 = 8,
        COL_TRAIN7 = 9,
        COL_TRAIN8 = 10,
        COL_TRAIN9 = 11,
        COL_TRAIN10 = 12,
		COL_TRAIN11 = 13,//lrg 20210215
		COL_TRAIN12 = 14,//lrg 20200220
	MAXFIELDCOL
} fieldcolor;

extern	grcolor	fieldcolors[MAXFIELDCOL];

#endif /* _COLOR_H */
