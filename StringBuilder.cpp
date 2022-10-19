#include "StringBuilder.h"

#include <malloc.h>
#include <string.h>


StringItem::StringItem()
: _ptr(0), _next(0)
{
        Clear();
}

StringItem::StringItem(const SBChar *pChar)
: _ptr(0), _next(0), _buffPos(0)
{
        if(Append(pChar))
            return;
        int     l;
#if wxUSE_UNICODE
        l = wcslen(pChar);
        _ptr = (SBChar *)malloc((l + 1) * sizeof(SBChar));
        wcscpy(_ptr, pChar);
//        _ptr = wcsdup(pChar);
#else
        l = strlen(pChar);
        _ptr = strdup(pChar);
#endif
        _buffPos = l;
}

StringItem::~StringItem()
{
        Clear();
}

void    StringItem::Clear()
{
        if(_ptr)
            free(_ptr);
        _ptr = 0;
        _buffPos = 0;
}


bool    StringItem::Append(const SBChar *pChar)
{
#if wxUSE_UNICODE
        int     l = wcslen(pChar);
        if(l + _buffPos < STRITEMBUFFSIZE - 1) {
            wcscat(_buff, pChar);
            _buffPos += l;
            return true;
        }
#else
        int     l = strlen(pChar);
        if(l + _buffPos < STRITEMBUFFSIZE - 1) {
            strcat(_buff, pChar);
            _buffPos += l;
            return true;
        }
#endif
        return false;
}



//
//
//


StringBuilder::StringBuilder()
{
        Clear();
}

StringBuilder::StringBuilder(const SBChar *pChar)
{
        StringItem *item = new StringItem(pChar);
        _firstItem = item;
        _lastItem = item;
}


StringBuilder& StringBuilder::Append(const SBChar *pChar)
{
        StringItem *item = _lastItem;

        if(!item) {
            item = new StringItem(pChar);
            _firstItem = _lastItem = item;
            return *this;
        }
        if(item->Append(pChar))
            return *this;
        item = new StringItem(pChar);
        if(!_lastItem)
            _firstItem = _lastItem = item;
        else {
            _lastItem->_next = item;
            _lastItem = item;
        }
        return *this;
}

StringBuilder& StringBuilder::Append(SBChar c)
{
        SBChar  buff[4];

        buff[0] = c;
        buff[1] = 0;
        return Append(buff);
}


void    StringBuilder::Clear()
{
        while(_firstItem) {
            _lastItem = _firstItem->_next;
            delete _firstItem;
            _firstItem = _lastItem;
        }
}

SBChar  *StringBuilder::ToString()
{
        StringItem  *item;
        int     len = 0;
        SBChar  *str;

        for(item = _firstItem; item; item = item->_next) {
            len += item->Length();
        }
        len += 1;
        str = (SBChar *)malloc(len * sizeof(SBChar));
        len = 0;
        for(item = _firstItem; item; item = item->_next) {
            memcpy(str + len * sizeof(SBChar), item->GetPtr(), item->Length() * sizeof(SBChar));
            len += item->Length();
        }
        str[len] = 0;
        return str;
}
