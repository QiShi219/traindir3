/*	TConfig.h - Created by Giampiero Caprino

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

#ifndef _TCONFIG_H
#define _TCONFIG_H

#include "wx/textfile.h"
#include "Options.h"

#define MAX_CONFIG_SECT	40

class TConfig {
public:
	TConfig();
	~TConfig();

	bool	Load(const wxChar *fname);
	bool	Save(const wxChar *fname);
	void	Close();

	bool	FindSection(const wxChar *name);

	bool	PushSection(const wxChar *name);
	void	PopSection();

	bool	GetInt(const wxChar *name, int& result);
	bool	GetString(const wxChar *name, wxString& result);
        bool    Get(Option *option);
        bool    Get(Option& option);

	void	StartSection(const wxChar *name);
	void	PutString(const wxChar *var, const wxChar *value);
	void	PutInt(const wxChar *var, int value);
        void    Put(Option *option);
        void    Put(Option& option);
        void    Put(IntOption& option);
        void    Put(BoolOption& option);

	wxTextFile  m_file;
	int	m_start, m_end;

	int	m_nSaved;
	int	m_savedStart[MAX_CONFIG_SECT];
	int	m_savedEnd[MAX_CONFIG_SECT];
};

#endif // _TCONFIG_H
