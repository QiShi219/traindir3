/*	Shape.h - Created by Giampiero Caprino

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

#ifndef _SHAPE_H
#define _SHAPE_H

class Shape
{
public:
        Shape();
        virtual ~Shape();
        void    Clean();

        int     GetWidth();
        int     GetHeight();
        void    *GetImage();
        unsigned char *GetRGBA();

        void    SetXpm(const char **xpm);

        TString *_id;
        void    *_img;
        void    *_png;
        char    **_xpm;
        unsigned char *_rgba;
        unsigned short _rgbaSize;
        bool    _builtin;
};


class ShapeManager
{
public:
        ShapeManager();
        virtual ~ShapeManager();

        virtual void    Clear();        // remove non-builtin shapes
        virtual void    ClearAll();     // remove all shapes

        Shape   *NewShape(const Char *id);
        Shape   *NewShape(const Char *id, const char **xpm);
        Shape   *FindShape(const Char *id);
        Shape   *LoadXpm(const Char *fname);

        ManagedArray<Shape *>   _shapes;
};

extern  ShapeManager shapeManager;

#endif // _SHAPE_H
