/*	TDFile.cpp - Created by Giampiero Caprino

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

#include "TDFile.h"
#include "wx/filename.h"
#include "wx/wfstream.h"
#include "wx/zipstrm.h"
//#include "wx/convauto.h"
#include <string.h>
#ifdef WIN32
#define strcasecmp stricmp
#endif
#include "trsim.h"

FileOption  searchPath(wxT("SearchPath"), wxT("Directories with signal scripts"),
                       wxT("Environment"), wxT(""));

class FileItem {
public:
	FileItem(const wxChar *item)
	{
	    name = item;
	    content = 0;
	    next = 0;
	    size = 0;
	}

	~FileItem()
	{
	    if(content)
		free(content);
	    content = 0;
	}


	FileItem    *next;
	wxString    name;
	int	    size;
	wxChar	    *content;
};

FileItem    *file_list;

void	FreeFileList(void)
{
	FileItem    *it;

	while((it = file_list)) {
	    file_list = it->next;
	    delete it;
	}
}


static	FileItem *AddFile(const wxString& name)
{
	FileItem    *it = new FileItem(name.c_str());
        it->name.Replace(wxT("\\"), wxT("/"), true);
	it->next = file_list;
	file_list = it;
	return it;
}

bool	ReadZipFile(const wxChar *path)
{
	wxFFileInputStream  dbStream(path);
	wxZipInputStream    zip(dbStream);

	wxZipEntry  *entry;
	wxString    entryName;
	FileItem    *it;
	int	    i;

	while(entry = zip.GetNextEntry()) {
	    entryName = entry->GetInternalName();
	    it = AddFile(entryName);
	    zip.OpenEntry(*entry);
	    it->size = zip.GetSize();

#if wxUSE_UNICODE
	    wxWritableCharBuffer tmpMB(it->size + 4);
	    if ((char *) tmpMB == NULL)
		return 0;
	    char *buffer = tmpMB;
	    for(i = 0; !zip.Eof() && i < it->size; buffer[i++] = zip.GetC());
	    buffer[i] = 0;
	    zip.CloseEntry();
	    /* wxConvAuto can't (as of wxWidgets 2.8.7) handle ISO-8859-1.  */
	    if (! (it->content = wxConvAuto().cMB2WX(tmpMB).release()))
		if (! (it->content = wxConvISO8859_1.cMB2WX(tmpMB).release()))
		    return 0;
#else
	    int	ch;
	    it->content = (wxChar *)malloc((it->size + 4) * sizeof(wxChar));
	    for(i = 0; !zip.Eof() && i < it->size; it->content[i++] = ch) {
		ch = zip.GetC();
		//if(ch < 0)
		//    break;
	    }
	    it->content[i] = 0;
#endif /* !wxUSE_UNICODE */
	}
	return 1;
}

int	LoadFile(const wxChar *name, wxChar **dest)
{
	FileItem    *it;

        wxString    fname = name;
        fname.Replace(wxT("\\"), wxT("/"), true);
	for(it = file_list; it; it = it->next) {
	    const wxChar    *t = it->name.c_str();
            if(!wxStricmp(fname.c_str(), t))
		break;
	}
	if(it) {
	    *dest = (wxChar *) malloc(sizeof (wxChar) * (it->size + 4));
	    memcpy(*dest, it->content, sizeof (wxChar) * (it->size + 1));
	    return 1;
	}
	FILE    *fp;

	if(!(fp = wxFopen(name, wxT("rb")))) {
	    wxString    filename;
	    size_t    p;

#if 1
	    // search in provided directories
            const Char *pth = searchPath._sValue.c_str();
	    for(p = 0; p < searchPath._sValue.size(); ++p) {
		if(pth[p] == wxT(';')) {
		    if(filename.size() > 0) {
			filename += wxT('/');
			filename += name;
			if((fp = wxFopen(filename, wxT("rb"))))
			    goto found;
			filename = wxT("");
		    }
		} else
		    filename += pth[p];
	    }
	    if(filename.size() > 0) {
		filename += wxT('/');
		filename += name;
		if((fp = wxFopen(filename, wxT("rb"))))
		    goto found;
	    }
#else
            size_t    p1, len;
	    p = p1 = 0;
	    while(!searchPath.empty() && p1 != wxString::npos) {
		p1 = searchPath.find(wxT(';'), p);
		len = p1 == wxString::npos ? p1 : p1 - p;
		filename = searchPath.substr(p, len) + wxT('/') + name;
		if((fp = wxFopen(filename, wxT("rb"))))
		    goto found;
		p = p1;
	    }
#endif
	    return 0;
	}
found:
	fseek(fp, 0, 2);
	unsigned int	length = ftell(fp);
	rewind(fp);

#if wxUSE_UNICODE
	wxWritableCharBuffer tmpMB(length + 4);
	if ((char *) tmpMB == NULL)
	    return 0;
	if(fread(tmpMB, 1, length, fp) != length) {
	    fclose(fp);
	    return 0;
	}
	fclose(fp);
	((char *) tmpMB)[length] = 0;		// mark end of file
	((char *) tmpMB)[length+1] = 0;		// mark end of file
	/* wxConvAuto can't (as of wxWidgets 2.8.7) handle ISO-8859-1.  */
	if (! (*dest = wxConvAuto().cMB2WX(tmpMB).release()))
	    if (! (*dest = wxConvISO8859_1.cMB2WX(tmpMB).release()))
		return 0;
#else
	*dest = (wxChar *)malloc((length + 4) * sizeof(wxChar));
	if(fread(*dest, 1, length, fp) != length) {
	    fclose(fp);
	    return 0;
	}
	fclose(fp);
	(*dest)[length] = 0;		// mark end of file
#endif /* !wxUSE_UNICODE */
	return 1;
}


///////////////////////////////////////////////////////////////////////


TDFile::TDFile(const wxChar *fname)
{
	name = wxStrdup(fname);
	size = 0;
	content = 0;
	nextChar = 0;
}

TDFile::~TDFile()
{
	if(content)
	    free(content);
	content = 0;
	size = 0;
	nextChar = 0;
}

void    TDFile::SetName(const Char *newName)
{
        if(content)
            free(content);
        content = 0;
        size = 0;
        nextChar = 0;
        name = wxStrdup(newName);
}

bool	TDFile::Load()
{
	if(!LoadFile(name.GetFullPath(), &content))
	    return false;

	size = wxStrlen(content);
	nextChar = content;
	return true;
}

int	TDFile::LineCount()
{
	int	nLines = 0;
	int	i;

	for(i = 0; i < size; ++i)
	    if(content[i] == wxT('\n'))
		++nLines;
	return nLines;
}

bool	TDFile::ReadLine(wxChar *dest, int size)
{
	int	i;

	for(i = 0; *nextChar && *nextChar != wxT('\n') && i < size - 1; ) {
	    if(*nextChar != wxT('\r'))
		dest[i++] = *nextChar;
	    ++nextChar;
	}
	dest[i] = 0;
	if(*nextChar == wxT('\n'))
            ++nextChar;
	return i != 0 || *nextChar != 0;
}

void	TDFile::Rewind()
{
	nextChar = content;
}

size_t	TDFile::GetPos()
{
	return nextChar - content;
}

void	TDFile::SetPos(size_t pos)
{
	nextChar = &content[pos];
}

void	TDFile::SetExt(const wxChar *ext)
{
	if (ext[1])
	    name.SetExt(&ext[1]);
	else
	    name.SetEmptyExt();
}

void	TDFile::GetDirName(wxChar *dest, int size)
{
	wxFileName  nm(this->name);
	nm.Normalize(wxPATH_NORM_LONG|wxPATH_NORM_DOTS|wxPATH_NORM_TILDE|wxPATH_NORM_ABSOLUTE);
	wxString    dir = nm.GetPath();
	wxStrncpy(dest, dir.c_str(), size - 1);
	dest[size - 1] = 0;
}
