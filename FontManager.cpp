
#include "FontManager.h"

FontManager fonts;

FontManager::FontManager()
{
}


FontManager::~FontManager()
{
}


int	FontManager::FindFont(int size, int family, int style, int weight, long color)
{
	int	i;
	FontEntry *font;
	FontEntry f(size, family, style, weight, color);

	for(i = 0; i < _fonts.size(); ++i) {
	    font = _fonts[i];
	    if(font->SameAs(f))
		return i;
	}
	return -1;
}



FontEntry *FontManager::FindFont(int index)
{
	if(index < 0 || index >= _fonts.size())
	    return 0;
	return _fonts[index];
}



int	FontManager::AddFont(int size, int family, int style, int weight, long color)
{
	int	i;

	if((i = FindFont(size, family, style, weight, color)) >= 0)
	    return i;
	FontEntry *f = new FontEntry(size, family, style, weight, color);
	_fonts.push_back(f);
	return _fonts.size() - 1;
}


//
//
//


bool	FontEntry::SameAs(const FontEntry& other)
{
	if(other._size != _size) return false;
	if(other._family != _family) return false;
	if(other._style != _style) return false;
	if(other._weight != _weight) return false;
	if(other._color != _color) return false;
	return true;
}

