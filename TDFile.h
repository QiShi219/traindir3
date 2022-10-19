/*	TDfile.h - Created by Giampiero Caprino

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

#ifndef _TDFILE_H
#define _TDFILE_H
#include "wx/filename.h"

bool	ReadZipFile(const wxChar *path);
int	LoadFile(const wxChar *name, wxChar **dest);

class TDFile {
public:
	TDFile(const wxChar *fname);
	~TDFile();

	bool	Load();
	void	Rewind();
	size_t	GetPos();
	void	SetPos(size_t pos);
	int	LineCount();
	bool	ReadLine(wxChar *dest, int size);
	void	SetExt(const wxChar *ext);
	void	GetDirName(wxChar *dest, int size);
        void    SetName(const wxChar *name);

	wxFileName	name;
	wxChar	*content;
	wxChar	*nextChar;
	int	size;
};

#endif // _TDFILE_H
