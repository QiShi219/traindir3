/*	Localize.cpp - Created by Giampiero Caprino

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
#include <stdlib.h>

#if !defined(__unix__) && !defined(__WXMAC__)
#include <malloc.h>
#endif

#include <memory.h>
#include <string.h>
#include "wx/ffile.h"
#include "Traindir3.h"
#include "Html.h"
#include "TDFile.h"
#include "localize.h"


/*	localized strings support (1.19)	*/

	const wxChar	*locale_name = wxT(".en");
struct	lstring {
	struct lstring *next;
	int	hash;
	wxChar	*en_string;
	wxChar	*loc_string;
};

struct	lstring *local_strings;

static wxChar	*linebuff;
static	int	maxline;

/*	TODO: need to rewrite getline() to support
 *	unicode localization catalogues. Also would
 *	be nice to support existing non-Unicode catalogues.
 */

static	const wxChar	*getline(TDFile *fp)
{
	int	i;
	wxChar	*buffpos;
	bool	notEOF;

	if(!linebuff) {
	    maxline = 256;
	    linebuff = (wxChar *)malloc(maxline * sizeof(linebuff[0]));
	    if (!linebuff)
		return NULL;
	}
	buffpos = linebuff;
	i = maxline;
	linebuff[maxline - 2] = '\n';
	while ((notEOF = fp->ReadLine(buffpos, i))) {
	    if(linebuff[maxline - 2] != '\n') {
	        i = maxline - 1;
		maxline += 256;
		linebuff = (wxChar *)realloc(linebuff, maxline * sizeof(linebuff[0]));
		if (!linebuff)
		    return NULL;
		buffpos = &linebuff[i];
		i = maxline - i;
		linebuff[maxline - 2] = '\n';
	    } else
		break;
	}
	if(!notEOF)
	    return 0;
	return linebuff;
}

/*	localized strings support (1.19)    */

void	set_full_file_name(wxString& fullpath, const wxChar *filename)
{
	if (!wxGetEnv(wxT("TDHOME"), &fullpath))
	    if (!wxGetEnv(wxT("HOME"), &fullpath)) {
#if !defined(__unix__)
		fullpath = wxT("C:");	// only disk that's definitely present
#else
		fullpath = wxT("/tmp");	// only user-writable directory that's definitely present
#endif
	    }
	fullpath += wxT('/');
	fullpath += filename;
}

int	strhash(const wxChar *s)
{
	int	h;

	for(h = 0; *s; h += *s++);	/* very poor man's hash algorithm */
	return h;
}

/*	convert "\n" into newline characters */

void	convert_newlines(wxChar *buff)
{
	int	i, j;

	for(i = j = 0; buff[i]; ++i, ++j)
	    if(buff[i] == '\\' && buff[i+1] == 'n') {
		buff[j] = '\n';
		++i;
	    } else if(buff[i] == '\\' && buff[i + 1] == 't') {
		buff[j] = '\t';
		++i;
	    } else
		buff[j] = buff[i];
	buff[j] = 0;
}

const wxChar	*localize(const wxChar *s)
{
	struct lstring *ls;
	int	h;

	if(!wxStrcmp(locale_name, wxT("en")) || !wxStrcmp(locale_name, wxT(".en")))
	    return s;
	h = strhash(s);
	for(ls = local_strings; ls; ls = ls->next) {
	    if(ls->hash == h && !wxStrcmp(ls->en_string, s))
		return ls->loc_string;
	}
	return s;
}

void	localizeArray(const Char *localized[], const Char *english[])
{
	int	i;

	for(i = 0; english[i]; ++i)
	    localized[i] = wxStrdup(LV(english[i]));
}

void	freeLocalizedArray(const Char *localized[])
{
	int	i;

	for(i = 0; localized[i]; ++i) {
	    free((void *) localized[i]);
	    localized[i] = 0;
	}
}

void	load_from_array(const LocalizeInfo *array)
{
	int	i;
	struct	lstring *ls;

	for(i = 0; array[i].english; ++i) {
	    ls = (struct lstring *)malloc(sizeof(struct lstring));
	    ls->en_string = wxStrdup(array[i].english);
	    ls->hash = strhash(ls->en_string);
	    ls->loc_string = wxStrdup(array[i].other);
	    ls->next = local_strings;
	    local_strings = ls;
	}
}

/*	Load all localized strings for 'locale'.
 *	Locale values should be in the standard
 *	2-character international country codes.
 *	By default, ".en" is ignored, since
 *	built-in strings are always in English.
 */

void	load_localized_strings(const wxChar *locale)
{
	wxChar		buff[512];
	wxString	name;
	struct	lstring *ls;
	const wxChar	*p;
	wxChar		*p1;

	if(!wxStrcmp(locale, wxT(".en")))
	    return;
	set_full_file_name(name, wxString(wxT("traindir")) + locale);
	TDFile	fp (name);
	if(!(fp.Load())) {
#if !defined( __WXMAC__ ) && !defined(__WXGTK__)
	    if(!wxStrcmp(locale, wxT(".es")))
		load_from_array(espanol);
#endif
	    if(!wxStrcmp(locale, wxT(".it")))
		load_from_array(italiano);
	    return;
	}
	while((p = getline(&fp))) {
	    if(*p == '#')	    /* comment */
		continue;
	    if(!(p1 = (wxChar *)wxStrstr(p, wxT("@@"))))
		continue;
	    wxStrcpy(buff, p1);

	    /*	isolate English string	*/

	    while(--p1 > p && (*p1 == ' ' || *p1 == '\t'));
	    p1[1] = 0;

	    p1 = wxStrstr(buff, wxT("@@")) + 2;
	    while(*p1 == ' ' || *p1 == '\t') ++p1;
	    convert_newlines(p1);

	    ls = (struct lstring *)malloc(sizeof(struct lstring));
	    ls->en_string = wxStrdup(p);
	    convert_newlines(ls->en_string);
	    ls->hash = strhash(ls->en_string);
	    ls->loc_string = wxStrdup(p1);
	    ls->next = local_strings;
	    local_strings = ls;
	}
}

void    escape_yaml(Char *out, Char *in)
{
        while(*in) {
            if(*in == '\n') {
                *out++ = '\\';
                *out++ = 'n';
                ++in;
            } else if(*in == '"') {
                *out++ = '\\';
                *out++ = '"';
                ++in;
            } else
                *out++ = *in++;
        }
        *out = 0;
}

void    get_localized_strings_yaml(wxString& out)
{
	struct	lstring *ls;
        Char    escaped[512];

        for(ls = local_strings; ls; ls = ls->next) {
            escape_yaml(escaped, ls->en_string);
            out.Append(wxT("- en: \"")).Append(escaped).Append(wxT("\"\n"));
            escape_yaml(escaped, ls->loc_string);
            out.Append(wxT("  loc: \"")).Append(escaped).Append(wxT("\"\n"));
        }
}
