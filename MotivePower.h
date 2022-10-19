/*	MotivePower.h - Created by Giampiero Caprino

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

#ifndef _MOTIVEPOWER_H
#define _MOTIVEPOWER_H

#include "Array.h"
#include "defs.h"

/*      3.9: support motive power specifications in tracks and trains
 *
 *      Motive power is simply specified as a string to represent the
 *      characteristics of the power used on a track or by the train's locomotive.
 *      For example, one can specify the voltage of the electric line
 *      used for a certain stretch of track.
 *      When motive power is specified both on tracks (layout) and on
 *      trains (schedule), the runner will check if the train can enter
 *      a new block.
 *      No power specified for a train means that the train can travel
 *      on any track (e.g. it's diesel-powered).
 */

class Track;

extern  Array<const Char *> gMotivePowerCache;
extern  const Char    *gEditorMotivePower;          // currently selected power for laying down tracks

const Char  *power_find(const Char *p);             // return Char * from cache
void        power_select(const Char *pwr);          // set editorMotivePower from cache or add new entry in cache
const Char  *power_clean(const Char *p);            // remove all spacces from input string, return static Char[]
const Char  *power_add(const Char *p);              // remove all spacces from input string, return static Char[]
const Char  *power_parse(const Char *p);            // power_clean(p) + add to cache if not there
bool        power_specified(const Track *layout);   // return whether any track in layout specify a motive power

extern  bool    powerSpecified;

#endif // _MOTIVEPOWER_H
