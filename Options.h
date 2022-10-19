/*	Options.h - Created by Giampiero Caprino

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

#ifndef _OPTIONS_H
#define _OPTIONS_H

#include <stdio.h>
#include "defs.h"

enum OptionType {
        OPTION_STRING,
        OPTION_INT,
        OPTION_BOOL,
        OPTION_FILE,
        OPTION_COLOR
};

class Option {
public:
        Option(OptionType type, Char *name, Char *descr, Char *cat, Char *defValue);
        Option(OptionType type, Char *name, Char *descr, Char *cat, int defValue);
        virtual ~Option();
        
        virtual void Set(const Char *value);
        virtual void Set(int value);
        virtual bool Match(Char *name, Char *value);
        virtual bool Save(FILE *fp);

        Option  *_next;

        wxString _name;
        wxString _descr;
        wxString _category;
        wxString _sValue;       // STRING, FILE
        int      _iValue;       // INT, BOOL, COLOR
        OptionType _type;
};

class IntOption : public Option {
public:
        IntOption(Char *name, Char *descr, Char *cat, int defValue);
        virtual ~IntOption();
};

class StringOption : public Option {
public:
        StringOption(Char *name, Char *descr, Char *cat, Char *defValue);
        virtual ~StringOption();

};

class BoolOption : public Option {
public:
        BoolOption(Char *name, Char *descr, Char *cat, bool defValue);
        virtual ~BoolOption();
        virtual void Set(bool value);
        virtual void Set(const Char *value);
        virtual bool Match(Char *name, Char *value);
};

class FileOption : public Option {
public:
        FileOption(Char *name, Char *descr, Char *cat, Char *defValue);
        virtual ~FileOption();
};

class ColorOption : public Option {
public:
        ColorOption(Char *name, Char *descr, Char *cat, Char *defValue);
        virtual ~ColorOption();
        virtual bool Match(Char *name, Char *value);
};


class OptionManager {
public:
        OptionManager();
        virtual ~OptionManager();

        static void Register(Option *opt);

        static void Save(FILE *fp);
        static bool Load(FILE *fp);

        static Option *_first, *_last;
        static int  _nOptions;
};

extern  OptionManager optionManager;

#endif // _OPTIONS_H
