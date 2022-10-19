/*	Server.h - Created by Giampiero Caprino

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

#ifndef _SERVER_H
#define _SERVER_H

#define MAX_PACK_SIZE  102400

extern  char MIME_TEXT[];
extern  char MIME_HTML[];
extern  char MIME_XML[];
extern  char MIME_JSON[];

class Servlet {
public:
        Servlet(const char *path);
        virtual ~Servlet();

        virtual const char *getMimeType() const { return MIME_TEXT; };
        virtual bool    get(wxString& out, wxChar *url) { return false; }
        virtual bool    get(void **out, int& len, wxChar *url) { return false; }
        virtual bool    post(wxString& out, wxChar *url) { return false; }
        static  void    escape(Char *out, int maxlen, const Char *value);
        static  void    json(wxString& out, const wxChar *field, const wxChar *value, bool last = false);
        static  void    json(wxString& out, const wxChar *field, int value, bool last = false);
        static  void    json(Char *out, int outsize, const Char *name, const Char *value, const Char *sep, bool last = false);
        void            FromUrl(Char *dest, const Char *src);

	Servlet *_next;
        const char *_path;
};

void    registerWebService(Servlet *s);

#endif // _SERVER_H
