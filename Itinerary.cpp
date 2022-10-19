/*	Itinerary.cpp - Created by Giampiero Caprino

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

#if !defined(__unix__) && !defined(__WXMAC__)
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include <math.h>
#include "wx/ffile.h"
#include "wx/textfile.h"
#include "Traindir3.h"
#include "Itinerary.h"

Itinerary *itineraries;

int	toggle_signal(Signal *t);

void	puzzle_check(Track *t);
extern	int enable_training;
extern  wxChar  *Train_Name;
Vector	*findPath(Track *trk, int dir);

Itinerary   *find_itinerary(const wxChar *name)
{
	Itinerary *it;

	for(it = itineraries; it; it = it->next)
	    if(!wxStrcmp(it->name, name))
		return it;
	return 0;
}

void	clear_visited(void)
{
	Itinerary *ip;

	for(ip = itineraries; ip; ip = ip->next)
	    ip->visited = 0;
}

void	delete_itinerary(Itinerary *ip)
{
	Itinerary *it, *oit;

	oit = 0;
	for(it = itineraries; it && ip != it; it = it->next)
	    oit = it;
	if(!it)
	    return;
	if(!oit)
	    itineraries = it->next;
	else
	    oit->next = it->next;
	free_itinerary(it);
}

void	delete_itinerary(const wxChar *name)
{
	Itinerary *it, *oit;

	oit = 0;
	for(it = itineraries; it && wxStrcmp(it->name, name); it = it->next)
	    oit = it;
	if(!it)
	    return;
	if(!oit)
	    itineraries = it->next;
	else
	    oit->next = it->next;
	free_itinerary(it);
}

void	free_itinerary(Itinerary *it)
{
	if(it->signame)
	    free(it->signame);
	if(it->endsig)
	    free(it->endsig);
	if(it->name)
	    free(it->name);
	if(it->sw)
	    free(it->sw);
	free(it);
}

void	add_itinerary(Itinerary *it, int x, int y, int sw)
{
	int	i;

	for(i = 0; i < it->nsects; ++i)
	    if(it->sw[i].x == x && it->sw[i].y == y) {
		it->sw[i].switched = sw;
		return;
	    }
	if(it->nsects >= it->maxsects) {
	    it->maxsects += 10;
	    if(!it->sw) {
		it->sw = (switin *)malloc(sizeof(switin) * it->maxsects);
	    } else {
		it->sw = (switin *)realloc(it->sw,
					sizeof(switin) * it->maxsects);
	    }
	}
	it->sw[it->nsects].x = x;
	it->sw[it->nsects].y = y;
	it->sw[it->nsects].switched = sw;
	++it->nsects;
}

void    delete_all_itineraries()
{
        Itinerary *it;

	while((it = itineraries)) {
	    itineraries = it->next;
	    free_itinerary(it);
	}
}

bool    Itinerary::TurnSwitches()
{
        int     el;
        Track   *trk;

        // turn every switch
        for(el = 0; el < nsects; ++el) {
	    trk = findSwitch(sw[el].x, sw[el].y);
	    if(!trk)
	        return false;
            sw[el].oldsw = trk->switched;
            trk->switched = sw[el].switched;
        }
        return true;
}

void    Itinerary::RestoreSwitches()
{
        int     el;
        Track   *trk;

        // turn every switch
        for(el = 0; el < nsects; ++el) {
	    trk = findSwitch(sw[el].x, sw[el].y);
	    if(!trk)
	        break;
            trk->switched = sw[el].oldsw;
        }
}

void    Itinerary::SetHasDwellingTracks(bool yn)
{
        _hasDwellingTracks = yn;
}

bool    Itinerary::HasDwellingTracks() const
{
        return _hasDwellingTracks;
}

// Checks that no switch controlled by the itinerary is locked by another path
// (i.e. they are all black)

int	check_itinerary(Itinerary *it)
{
	wxChar	*nextitin;
	Track	*t1;
	int	i;

	clear_visited();
agn:
	if(!it || it->visited)
	    return 0;
	for(i = 0; i < it->nsects; ++i) {
	    t1 = findSwitch(it->sw[i].x, it->sw[i].y);
	    if(!t1 || t1->fgcolor != conf.fgcolor || t1->IsDwelling())//打开进路时返回0
		return 0;
	    it->sw[i].oldsw = t1->switched;
	    if(it->sw[i].switched != t1->switched)
		if((t1 = findSwitch(t1->wlinkx, t1->wlinky)))
		    if(t1 && t1->fgcolor != conf.fgcolor || t1->IsDwelling())
			return 0;
	}
	if(!(nextitin = it->nextitin) || !*nextitin)
	    return 1;
	it->visited = 1;
	for(it = itineraries; it; it = it->next)
	    if(!wxStrcmp(it->name, nextitin))
		break;
	goto agn;
}

// Throws all switches controlled by the specified itinerary

void	toggle_itinerary(Itinerary *it)
{
	Track	*t1;
	int	i;
	Char	*nextitin;

	do {
	    for(i = 0; i < it->nsects; ++i) {
		t1 = findSwitch(it->sw[i].x, it->sw[i].y);
		if(it->sw[i].switched != t1->switched) {
		    t1->switched = !t1->switched;
		    change_coord(t1->x, t1->y);
		    if((t1 = findSwitch(t1->wlinkx, t1->wlinky))) {
			t1->switched = !t1->switched;
			change_coord(t1->x, t1->y);
		    }
		}
	    }
	    if(!(nextitin = it->nextitin) || !*nextitin)
		return;
	    for(it = itineraries; it; it = it->next)
		if(!wxStrcmp(it->name, nextitin))
		    break;
	} while(it);			/* always true */
}

int	green_itinerary(Itinerary *it, bool forShunt)
{
	Signal	*t1;
	Itinerary *ip;
	wxChar	*nextitin;
	int	i;
	Signal	*blocks[100];
	int	nxtblk;

	nxtblk = 0;
	for(ip = it; ip; ) {
	    if(!(t1 = findSignalNamed(ip->signame)))
		return 0;
	    if(t1->status == ST_GREEN)
		return 0;
	    blocks[nxtblk++] = t1;
	    if(!(nextitin = ip->nextitin) || !*nextitin)
		break;			// done
	    for(ip = itineraries; ip; ip = ip->next)
		if(!wxStrcmp(ip->name, nextitin))
		    break;
	}

	// all signals are red, try to turn them green

        for(i = 0; i < nxtblk; ++i) {
            if(!forShunt) {
                if(!toggle_signal_auto_itinerary(blocks[i],1))//lrg 20210221
		    break;		// line block is busy
            } else {
                if(!blocks[i]->ToggleForShunting())
                    break;
            }
        }

		//train_it_save(it);//lrg 20210221

	if(i >= nxtblk)			// success!
	    return 1;
        while(--i >= 0) {		// undo signal toggling
            if(!forShunt)
	        toggle_signal(blocks[i]);
            else
                blocks[i]->ToggleForShunting();
        }
	return 0;
}

void	itinerary_selected(Track *t, bool forShunt)
{
	size_t	    namelen;
	Itinerary   *it;

	if(t->station && *t->station == '@') {	    // use script
	    t->OnClicked();
	    return;
	}
	Char *nameend = wxStrrchr(t->station, '@');

        if(nameend) {
	    namelen = nameend - t->station;
            while(namelen > 0 && t->station[namelen-1] == ' ')
                --namelen;
        } else
	    namelen = wxStrlen(t->station);
	for(it = itineraries; it; it = it->next) {
	    if(!wxStrncmp(it->name, t->station, namelen) &&
		  wxStrlen(it->name) == namelen)
 		break;
	}
	if(it)
            it->Select(forShunt);
	if(enable_training) {
	    puzzle_check(t);
	}
}


void	try_itinerary(int sx, int sy, int ex, int ey)
{
	Itinerary *it = 0;
	Signal	*t1, *t2;

	t1 = findSignal(sx, sy);
	t2 = findSignal(ex, ey);
	if(!t1 || !t2)
	    return;
	if(t1->station && *t1->station && t2->station && *t2->station) {
	    for(it = itineraries; it; it = it->next)
		if(!wxStrcmp(it->signame, t1->station) &&
			!wxStrcmp(it->endsig, t2->station))
		    break;
	}
//	if(!it) {
//	    Itinerary *it = find_from_to(t1, t2);
//	    return;
//	}
        it->Select(false);
}


//lrg 20210220
long int * itiner_number_y(wxChar *it)
{
	
	const int endsign_num_length = 6;
	int j = 0;
	int tempj = 0;
	long int start_zuobiao[2];
	long int starts_number = 0;
	wxChar *it_startsign;
	wxChar startsign_num[endsign_num_length];
	wxChar startsign_num_y[endsign_num_length];

	for(it_startsign = it;it_startsign;it_startsign++,j++)
	{
		if((*it_startsign != '(') && (*it_startsign != ','))
		{
			startsign_num[j-1] = *it_startsign;
		}
		if(*it_startsign == ',')//表示itinerary结尾的横坐标读取完毕
		{
			startsign_num[j-1] = '\0';
			j = 0;
			it_startsign++;

			for(;*it_startsign != ')';it_startsign++,tempj++)
			{
				startsign_num_y[tempj] = *it_startsign;
			}
			startsign_num_y[tempj] = '\0';
			tempj = 0;
			break;
		}
		
	}
	start_zuobiao[0] = atol(startsign_num);
	start_zuobiao[1] = atol(startsign_num_y);
	return start_zuobiao;
}


//lrg  20210217  找到触发itinerary的列车
void train_it_save(Itinerary *it)
{
	Train	*t;
	Train *tempt;
	int	i;
	int j = 0;
	int temi;
	int initdis_it = 100;//设置触发itinerary列车坐标与itinerary起始坐标的间距  lrg 20210217 
	int initdis_it_y = 100;
	long int *temp_lint;
	
	long int ends_number[2];
	long int starts_number[2];

	//wxChar *it_startsign;
	//wxChar startsign_num[endsign_num_length];

	wxChar	*nextitin;
	int a= 0;
	
	temp_lint = itiner_number_y(it->signame);
	starts_number[0] = *(temp_lint++);
	starts_number[1] = *temp_lint;

	temp_lint = itiner_number_y(it->endsig);
	ends_number[0] = *(temp_lint++);
	ends_number[1] = *temp_lint;
	
	wxChar it_name_first[20];
	wxChar *itit;
	wxChar tlast_it_first[20];
	int tempj;
	wxStrcpy(it_name_first, it->name);
	it_name_first[2] = '\0';
	int ai;

	
	//找到触发该itinerary的列车车次   lrg20210218
	//for(t = schedule; t; t = t->next) 
	//{
	//	if((starts_number[0] < ends_number[0]) && (t->status != train_READY) && (t->status != train_ARRIVED)&& (t->last_it != 0) &&(t->position->x < ends_number[0]))
	//	{
	//		wxStrcpy(tlast_it_first, t->last_it);
	//		tlast_it_first[2] = '\0';

	//		if(!wxStrncmp(tlast_it_first,it_name_first,2))
	//		{
	//			if((starts_number[0] - t->position->x < initdis_it)  && (t->position->x <= t->stoppoint->x) && (starts_number[0] - t->position->x >= 0))
	//			{
	//				initdis_it = starts_number[0] - t->position->x;
	//				tempt = t;
	//			}
	//		}
	//	}

	//	if((starts_number[0] > ends_number[0]) && (t->status != train_READY) && (t->status != train_ARRIVED)&& (t->last_it != 0) &&(t->position->x > starts_number[0]))
	//	{
	//		wxStrcpy(tlast_it_first, t->last_it);
	//		tlast_it_first[2] = '\0';

	//		if(!wxStrncmp(tlast_it_first,it_name_first,2))
	//		{
	//			if((t->position->x - starts_number[0] < initdis_it) && (t->position->x >= t->stoppoint->x) && (t->position->x - starts_number[0] >= 0))
	//			{
	//				initdis_it = t->position->x - starts_number[0];
	//				tempt = t;
	//			}
	//		}
	//	}
	//	
	//}
	
	Train *temp_sched;
	//将对应的itinerayr终点横坐标赋值给触发itinerary的列车  lrg20210218
	//for(temp_sched = schedule;temp_sched;temp_sched = temp_sched->next)
	//{
	//	//if(temp_sched == tempt)
	//	
	//	if(!wxStrcmp(tempt->name,temp_sched->name))
	//	{
	//		temp_sched->it_starts = starts_number[0];
	//		temp_sched->it_ends = ends_number[0];
	//		break;
	//	}
	//}

	for(temp_sched = schedule;temp_sched;temp_sched = temp_sched->next)
	{
		//if(temp_sched == tempt)
		
		if(temp_sched->itflag&&!wxStrcmp(Train_Name,temp_sched->name))
		{
			temp_sched->it_starts = starts_number[0];
			temp_sched->it_ends = ends_number[0];
			temp_sched->itflag = false;
			break;
		}
	}
	Train_Name = 0;

}




bool	Itinerary::Select(bool forShunt)
{
	Itinerary *it = this;
	Track	*t1;
	int	i;
	wxChar	*nextitin;

	//train_it_save(it); //lrg 20210221

	if(!check_itinerary(it))
	    return false;
	toggle_itinerary(it); 
	if(green_itinerary(it, forShunt))
	    return true;		/* success */

	    
	/* error - restore switches status */
err:
	for(i = 0; i < it->nsects; ++i) {
	    t1 = findSwitch(it->sw[i].x, it->sw[i].y);
	    if(!t1)
		continue;
	    if(it->sw[i].switched == it->sw[i].oldsw)
		continue;
	    t1->switched = !t1->switched;
	    change_coord(t1->x, t1->y);
	    if((t1 = findSwitch(t1->wlinkx, t1->wlinky))) {
		t1->switched = !t1->switched;
		change_coord(t1->x, t1->y);
	    }
	}
	if(!(nextitin = it->nextitin) || !*nextitin)
	    return false;
	for(it = itineraries; it; it = it->next)
	    if(!wxStrcmp(it->name, nextitin))
		break;
	if(it)
	    goto err;
	return true;
}


int	sameStation_s1(const wxChar *s1, const wxChar *s2);
//逐一开启行程  lrg 20211213修改，需要指定股道停车
bool  Itinerary::Find_special(bool forShunt,const wxChar *it_name)
{
	Itinerary *it = this;
    Itinerary *it0;
	it0=it;
    it=it->next;
	it=it->next;
	size_t	namelen;

	Train *temp_sched;
	for(temp_sched = schedule;temp_sched;temp_sched = temp_sched->next)
	{	
		if(!wxStrcmp(Train_Name,temp_sched->name))
		{
			for(TrainStop *stops1=temp_sched->stops;stops1;stops1=stops1->next)
				if(sameStation_s1(stops1->station,it_name))
				{
					namelen = wxStrlen(stops1->station);
					Char *nameend1 = wxStrchr(stops1->station,'@');
					namelen = namelen-wxStrlen(nameend1)+4;
					/*wxChar *temp=(wxChar *)it_name;*/
					char temp[30];
					memset(temp,0,sizeof(char)*30);
					wxStrncpy(temp,it_name,namelen);
					Char *nameend = wxStrrchr(stops1->station,'@');
					nameend = nameend + 1;
					char a = nameend[0];
					//temp[wxStrlen(temp)-1]=a;
					//temp[wxStrlen(temp)]='\0';
					temp[namelen]=a;
					temp[namelen+1]='\0';
					Itinerary *it1;
					for(it1 = itineraries; it1; it1 = it1->next)
						if(!wxStrcmp(it1->name,temp) && !wxStrncmp(it1->next->name,temp,wxStrlen(temp)))
						{
							it = it1;
							temp_sched->last_it = it->name;
							break;
						}
					break;
				}
			break;
		}
	}

	if(check_itinerary(it))
	{
		if(wxStrncmp(it->name, it0->name, 4))
			return false;
		toggle_itinerary(it);
		green_itinerary(it, forShunt);
		if(!check_itinerary(it))
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
}



//逐一开启行程 
bool  Itinerary::Find(bool forShunt)
{
	Itinerary *it = this;
    Itinerary *it0;
	it0=it;
    it=it->next;
	it=it->next;
    //train_it_save(it);

	while(check_itinerary(it))
	{
		if(wxStrncmp(it->name, it0->name, 4))
      return false;
	toggle_itinerary(it);
	if(green_itinerary(it, forShunt))
	  break;
	it=it->next;
    it=it->next;
	}
	if(!check_itinerary(it))
	    return false;
}

bool	Itinerary::IsSelected()
{
	return Deselect(true);
}


bool	Itinerary::CanSelect()
{
    	Signal	*sig;
        int     el;
	Track   *trk;

	sig = findSignalNamed(signame);
	if(!sig)
	    return false;

	Vector	*path;

	if(!sig->controls)
	    return false;
	if(sig->IsClear())
	    return false;

        TurnSwitches();
	path = findPath(sig->controls, sig->direction);
        if(!path) {
            RestoreSwitches();
	    return false;
        }
	int nel = path->_size;
	bool failed = false;
	// check that every element in the path is clear
	for(el = 0; el < nel; ++el) {
	    trk = path->TrackAt(el);
            if(trk->fgcolor != conf.fgcolor) {
                failed = true;
                break;
	    }
	}
	Vector_delete(path);
        RestoreSwitches();
	return !failed;
}


bool	Itinerary::Deselect(bool checkOnly)
{
	Signal	*sig;

	sig = findSignalNamed(signame);
	if(!sig)
	    return false;

	Vector	*path;

	if(!sig->controls)
	    return false;
	if(!sig->IsClear())	// maybe a train entered the block or the
	    return false;	// path is occupied in the opposite direction

	path = findPath(sig->controls, sig->direction);
	if(!path)
	    return false;
	int nel = path->_size;
	int el;
	Track *trk;
	bool failed = false;
	// check that every element in the path is still clear
	for(el = 0; el < nel; ++el) {
	    trk = path->TrackAt(el);
	    if(trk->fgcolor != color_green && trk->fgcolor != color_white) {
		failed = true;
		break;
	    }
	}
	if(!failed) {
	    // check that every switch is in the right position
	    for(el = 0; el < nsects; ++el) {
		trk = findSwitch(sw[el].x, sw[el].y);
		if(!trk) {
		    failed = true;
		    break;
		}
		if(trk->switched != sw[el].switched) {
		    failed = true;
		    break;
		}
	    }
	}
	if(!failed) {
            if(!checkOnly) {		// OK to undo the itinerary
		toggle_signal(sig);
            }
	}

	Vector_delete(path);
	return !failed;
}



static int ByName(const void *pa, const void *pb)
{
	Itinerary   *ia = *(Itinerary **)pa;
	Itinerary   *ib = *(Itinerary **)pb;

	return wxStricmp(ia->name, ib->name);
}

void	sort_itineraries()
{
	Itinerary   **its;
	Itinerary   *it;
	int	    i, n;

	if(!itineraries)
	    return;
	n = 0;
	for(it = itineraries; it; it = it->next)
	    ++n;
	its = (Itinerary **)malloc(sizeof(Itinerary *) * n);
	n = 0;
	for(it = itineraries; it; it = it->next)
	    its[n++] = it;
	qsort(its, n, sizeof(Itinerary *), ByName);
	for(i = 0; i < n - 1; ++i)
	    its[i]->next = its[i + 1];
	its[i]->next = 0;
	itineraries = its[0];
	free(its);
}

//
//
//
//


#include "tdscript.h"

class ItinInterpreterData : public InterpreterData {
public:
	ItinInterpreterData()
	{
	    _onInit = 0;
	    _onRestart = 0;
	    _onClick = 0;
	}

	~ItinInterpreterData()
	{
	    if(_onInit)
		delete _onInit;
	    if(_onRestart)
		delete _onRestart;
	    if(_onClick)
		delete _onClick;
	};

	Statement *_onInit;	// list of actions (statements)
	Statement *_onRestart;
	Statement *_onClick;

	bool	Evaluate(ExprNode *expr, ExprValue& result);
};

bool	Itinerary::GetPropertyValue(const wxChar *prop, ExprValue& result)
{
	if(!wxStrcmp(prop, wxT("name"))) {
	    result._op = String;
	    result._txt = this->name;
	    return true;
	}
	return false;
}


void	Itinerary::OnInit()
{
	if(_interpreterData) {
	    ItinInterpreterData& interp = *(ItinInterpreterData *)_interpreterData;
	    if(interp._onInit) {
		interp._itinerary = this;
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("Itinerary::OnInit(%s)"), this->name);
		Trace(expr_buff);
		interp.Execute(interp._onInit);
	    }
	}
}

void	Itinerary::OnRestart()
{
	if(_interpreterData) {
	    ItinInterpreterData& interp = *(ItinInterpreterData *)_interpreterData;
	    if(interp._onRestart) {
		interp._itinerary = this;
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("Itinerary::OnRestart(%s)"), this->name);
		Trace(expr_buff);
		interp.Execute(interp._onRestart);
	    }
	}
}

void	Itinerary::OnClick()
{
	if(_interpreterData) {
	    ItinInterpreterData& interp = *(ItinInterpreterData *)_interpreterData;
	    if(interp._onClick) {
		interp._itinerary = this;
		wxSnprintf(expr_buff, sizeof(expr_buff)/sizeof(wxChar), wxT("Itinerary::OnClick(%s)"), this->name);
		Trace(expr_buff);
		interp.Execute(interp._onClick);
		return;
	    }
	}
}
