/*	MotivePower.cpp - Created by Giampiero Caprino

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
#include "MotivePower.h"
#include "Options.h"

Array<const Char *> gMotivePowerCache;
const Char  *gEditorMotivePower;

IntOption       editor_gauge(wxT("editor_gauge"),
                             wxT("Default Track Gauge"),
                             wxT("Editor"), 0);

const Char *power_clean(const Char *p)
{
        static Char clean[128];
        int max = sizeof(clean) / sizeof(clean[0]);
        int x = 0;

        for(x = 0; *p; ++p) {
            if(*p == ' ')
                continue;
            if(*p == '\n')
                break;
            if(x < max - 1)
                clean[x++] = *p;
        }
        clean[x] = 0;
        clean[max - 1] = 0;
        return clean;
}

const Char    *power_find(const Char *p)
{
        for(int i = 0; i < gMotivePowerCache.Length(); ++i) {
            const Char *pc = gMotivePowerCache.At(i);
            if(!wxStrcmp(pc, p))
                return pc;
        }
        return 0;
}

const Char  *power_add(const Char *pwr)
{
        const Char *pc = wxStrdup(pwr);
        gMotivePowerCache.Add(pc);
        return pc;
}

const Char  *power_parse(const Char *p)
{
        p = power_clean(p);
        const Char *pc = power_find(p);
        if(pc)
            return pc;
        return power_add(p);
}

void    power_select(const Char *pwr)
{
        const Char    *pc = power_find(pwr);
        if(!pc) {
            pc = power_add(pwr);
        }
        gEditorMotivePower = *pc ? pc : 0;
}

