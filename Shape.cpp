/*	Shape.cpp - Created by Giampiero Caprino

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

#include "Traindir3.h"
#include "trsim.h"

void    *get_pixmap_file(const wxChar *fname, bool setShape);
extern	TDSkin	*curSkin;


ShapeManager::ShapeManager()
{
}


ShapeManager::~ShapeManager()
{
}


void    ShapeManager::ClearAll(void)
{
        _shapes.Clean();
}


void    ShapeManager::Clear(void)
{
        int     i;
        Shape   *shape;

        for(i = _shapes.Length(); --i >= 0; ) {
            shape = _shapes.At(i);
            if(shape->_builtin)
                continue;
            _shapes.RemoveAt(i);
        }
}


Shape   *ShapeManager::FindShape(const Char *id)
{
        int     i;
        Shape   *shape;

        for(i = _shapes.Length(); --i >= 0; ) {
            shape = _shapes.At(i);
            if(!wxStrcmp(shape->_id->c_str(), id))
                return shape;
        }
        return 0;
}

Shape   *ShapeManager::NewShape(const Char *id)
{
        Shape   *shape = new Shape();
        shape->_id = new TString(id);
        _shapes.Add(shape);
        return shape;
}


Shape   *ShapeManager::NewShape(const Char *id, const char **xpm)
{
        Shape *shp = NewShape(id);
        shp->SetXpm(xpm);
        return shp;
}


Shape   *ShapeManager::LoadXpm(const Char *fname)
{
        // TODO: use just file name instead of path
        Shape   *shape = FindShape(fname);
        if(shape)
            return shape;
        void *img = get_pixmap_file(fname, true); // will call NewShape(fname)
        if(!img)
            return 0;
        return FindShape(fname);
}


/////////////////////////////////////////


Shape::Shape()
{
        _id = 0;
        _img = 0;
        _rgba = 0;
        _png = 0;
        _xpm = 0;
        _builtin = false;
}

Shape::~Shape()
{
        Clean();
}

void    Shape::Clean()
{
        if(_img)
            delete _img;
        _img = 0;
        if(_rgba)
            free(_rgba);
        _rgba = 0;
        if(_png)
            free(_png);
        _png = 0;
        if(_xpm) {
            int i;
            for(i = 0; _xpm[i]; ++i)
                free(_xpm[i]);
            free(_xpm);
        }
        _xpm = 0;
        if(_id)
            delete _id;
        _id = 0;
}


void    *Shape::GetImage()
{
        if(!_img) {
            if(!_xpm) // TODO: create empty image
                return 0;
            _img = new wxImage(_xpm);
        }
        return _img;
}


void    Shape::SetXpm(const char **xpm)
{
        int     i;

        for(i = 0; xpm[i]; ++i);
        _xpm = (char **)calloc(i + 1, sizeof(char *));
        for(i = 0; xpm[i]; ++i)
            _xpm[i] = strdup(xpm[i]);
        // TODO: extract width and heigh from xpm data
}


int     Shape::GetWidth()
{
        // TODO: cache it locally
        wxImage *img = (wxImage *)GetImage();
        return img->GetWidth();
}


int     Shape::GetHeight()
{
        // TODO: cache it locally
        wxImage *img = (wxImage *)GetImage();
        return img->GetHeight();
}


unsigned char *Shape::GetRGBA()
{
        int     w;
        int     h;
        int     x;
        int     y;

        if(_rgba)
            return _rgba;
        wxImage *img = (wxImage *)GetImage();
        w = img->GetWidth();
        h = img->GetHeight();
        _rgbaSize = (w + 1) * h;
        _rgba = (unsigned char *)malloc(sizeof(unsigned char *) * _rgbaSize);
        unsigned char *dst = _rgba;
        for(y = 0; y < h; ++y) {
            for(x = 0; x < w; ++x) {
                // TODO: use img->GetData(), which is faster
                dst[0] = img->GetRed(x, y);
                dst[1] = img->GetGreen(x, y);
                dst[2] = img->GetBlue(x, y);
                dst[3] = 0xff;  // all pixels are opaque
                if(img->HasMask()) { // unless they were defined with color 'none'
                    if(img->GetMaskRed() == dst[0] && img->GetMaskGreen() == dst[1] &&
                        img->GetMaskBlue() == dst[2]) {
                            // if so, use our background color
                            dst[0] = (curSkin->background >> 16) & 0xff;
                            dst[1] = (curSkin->background >> 8) & 0xff;
                            dst[2] =  curSkin->background & 0xff;
                    }
                }
                dst += 4;
            }
        }
        return _rgba;
}
