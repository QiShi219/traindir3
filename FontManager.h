#ifndef _FONTMANAGER_H
#define _FONTMANAGER_H


#include <wx/font.h>
#include <vector>


class FontEntry {
public:
	FontEntry();
	FontEntry(int size, int family, int style, int weight, long color)
	    : _id(0),
	    _size(size),
	    _family(family),
	    _style(style),
	    _weight(weight),
	    _color(color)
	{
	}

	~FontEntry()
	{
	}

	int	_id;
	int	_size, _family, _style, _weight;
	long	_color;

	bool	SameAs(const FontEntry& other);
};

typedef std::vector<FontEntry *>    FontPool;

class FontManager {
public:
	FontManager();
	~FontManager();

	int	AddFont(int size, int family, int style, int weight, long color);

	FontEntry *FindFont(int index);

	int	FindFont(int size, int family, int style, int weight, long color);

	FontPool    _fonts;
};

extern	FontManager fonts;

#endif // _FONTMANAGER_H
