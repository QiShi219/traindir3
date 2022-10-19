/*	Itinerary.h - Created by Giampiero Caprino

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

#ifndef _ITINERARY_H
#define _ITINERARY_H

typedef struct {
	int	x, y;		/* coordinate of the switch */
	int	switched;	/* whether to automatically throw the switch */
	int	oldsw;		/* old status */
} switin;

class Itinerary {
public:
	Itinerary *next;
	int	visited;	/* flag to avoid endless loop */
	wxChar	*name;		/* name of itinerary */
	wxChar	*signame;	/* name of start signal */
	wxChar	*endsig;	/* name of end signal */
	wxChar	*nextitin;	/* next itinerary automatically activated */
	int	nsects, maxsects;/* sections are signal-to-signal */
	switin	*sw;
        bool    _hasDwellingTracks;

	char	*_iconSelected;
	char	*_iconDeselected;

	bool	CanSelect();	// itinerary can be selected (check signals and switches)
	bool	Select(bool forShunt);
	bool    Find(bool forShunt);//逐一打开行程
	bool  Find_special(bool forShunt,const wxChar *it_name);//lrg 20211220
	bool	IsSelected();	// itinerary is currently selected (use iconSelected/iconDeselected)
	bool	Deselect(bool checkOnly);	// deselect itinerary
        bool    TurnSwitches(); // save old switches states and turn switches for itinerary
        void    RestoreSwitches(); // turn switches according to saved state

	// script support
	void	OnInit();
        void    OnRestart();
	void	OnClick();	// itinerary is enabled
	void	OnCanceled();	// itinerary is canceled

	bool	GetPropertyValue(const wxChar *prop, ExprValue& result);
        void    SetHasDwellingTracks(bool yn);
        bool    HasDwellingTracks() const;

	void	*_interpreterData;
};

class	Track;

Itinerary   *find_itinerary(const wxChar *name);

void	itinerary_selected(Track *t, bool forShunt);

void    delete_all_itineraries();
void	delete_itinerary(Itinerary *it);
void	delete_itinerary(const wxChar *name);
void	free_itinerary(Itinerary *it);
void	add_itinerary(Itinerary *it, int x, int y, int switched);
void	try_itinerary(int sx, int sy, int ex, int ey);
void	sort_itineraries();


extern	Itinerary *itineraries;

long int * itiner_number_y(wxChar *it);//lrg 20210218
void train_it_save(Itinerary *it);//lrg 20210219

#endif // _ITINERARY_H
