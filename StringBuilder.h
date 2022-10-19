#ifndef _STRINGBUILDER_H
#define _STRINGBUILDER_H

#include "wx/wx.h"
#include "defs.h"

#if wxUSE_UNICODE
typedef wchar_t SBChar;
#else
typedef Char    SBChar;
#endif

#define STRITEMBUFFSIZE 16

class StringItem {
public:
        StringItem();
        StringItem(const SBChar *pChar);
        ~StringItem();
        void            Clear();
        bool            Append(const SBChar *pChar);

        SBChar          *GetPtr() { return _ptr ? _ptr : _buff; };
        int             Length() const { return _buffPos; };

        SBChar          _buff[STRITEMBUFFSIZE];
        SBChar          *_ptr;
        int             _buffPos;   // next pos in _buff, if not using _ptr
        StringItem      *_next;
};

class StringBuilder {
public:
        StringBuilder();
        StringBuilder(const SBChar *pChar);
        StringBuilder(const StringBuilder& other);
        ~StringBuilder();

        StringBuilder&  Append(const SBChar *pChar);
        StringBuilder&  Append(const StringBuilder& other);
        StringBuilder&  Append(SBChar c);

        int             Length();
        SBChar          *ToString();

        void            Clear();

        StringItem      *_firstItem, *_lastItem;
};

#endif // _STRINGBUILDER
