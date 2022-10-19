/*	Options.cpp - Created by Giampiero Caprino

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
#include "Options.h"

Option *OptionManager::_first;
Option *OptionManager::_last;
int OptionManager::_nOptions;

int myAtoi(const Char *p)
{
	int val = 0;
	while(*p >= '0' && *p <= '9')
	    val = (val * 10) + (*p++ - '0');
	return val;
}

OptionManager::OptionManager()
{
}

OptionManager::~OptionManager()
{
}

void OptionManager::Register(Option *opt)
{
        if(!_first)
            _first = opt;
        else
            _last->_next = opt;
        _last = opt;
        opt->_next = 0;
}


Option::Option(OptionType type, Char *name, Char *descr, Char *cat, Char *defValue)
{
        _name = name;
        _descr = descr;
        _category = cat;
        _type = type;
        OptionManager::Register(this);
        Set(defValue);
}

Option::Option(OptionType type, Char *name, Char *descr, Char *cat, int defValue)
{
        _name = name;
        _descr = descr;
        _category = cat;
        _type = type;
        OptionManager::Register(this);
        Set(defValue);
}



Option::~Option()
{
}

void Option::Set(const Char *value)
{
        _sValue = value;
        if(value[0] >= wxT('0') && value[0] <= wxT('9'))
            _iValue = myAtoi(value);
        else
            _iValue = 0;
}

void Option::Set(int value)
{
        _iValue = value;
        _sValue.sprintf(wxT("%d"), value);
}

bool Option::Match(Char *name, Char *value)
{
        if(_name.CmpNoCase(name))
            return false;
        _sValue = value;
        if(value[0] >= wxT('0') && value[0] <= wxT('9'))
            _iValue = myAtoi(value);
        else
            _iValue = 0;
        return true;
}

bool Option::Save(FILE *fp)
{
        wxFprintf(fp, wxT("%s = %s\n"), _name.c_str(), _sValue.c_str());
        return false;
}

IntOption::IntOption(Char *name, Char *descr, Char *cat, int defValue)
: Option(OPTION_INT, name, descr, cat, defValue)
{
}

IntOption::~IntOption()
{
}

StringOption::StringOption(Char *name, Char *descr, Char *cat, Char *defValue)
: Option(OPTION_STRING, name, descr, cat, defValue)
{
}

StringOption::~StringOption()
{
}

BoolOption::BoolOption(Char *name, Char *descr, Char *cat, bool defValue)
: Option(OPTION_BOOL, name, descr, cat, defValue)
{
}

BoolOption::~BoolOption()
{
}

void BoolOption::Set(bool value)
{
        _sValue = value ? wxT("1") : wxT("0");
        _iValue = value ? 1 : 0;
}

void BoolOption::Set(const Char *value)
{
        _sValue = value;
        _iValue = wxAtoi(value) ? 1 : 0;
}

bool BoolOption::Match(Char *name, Char *value)
{
        if(!Option::Match(name, value))
            return false;
        if(_iValue != 0)
            _iValue = 1;
        return true;
}

FileOption::FileOption(Char *name, Char *descr, Char *cat, Char *defValue)
: Option(OPTION_FILE, name, descr, cat, defValue)
{
}

FileOption::~FileOption()
{
}

ColorOption::ColorOption(Char *name, Char *descr, Char *cat, Char *defValue)
: Option(OPTION_COLOR, name, descr, cat, defValue)
{
}

ColorOption::~ColorOption()
{
}

bool ColorOption::Match(Char *name, Char *value)
{
        if(!Option::Match(name, value))
            return false;
        Char *p;
        _iValue = (wxStrtol(value, &p, 0) & 0xFF) << 16;
        _iValue |= (wxStrtol(p, &p, 0) & 0xFF) << 8;
        _iValue |= (wxStrtol(p, &p, 0) & 0xFF);
        return true;
}

