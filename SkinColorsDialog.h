/*	TrackDialog.h - Created by Giampiero Caprino

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

#ifndef _SKINCOLORSDIALOG_H
#define _SKINCOLORSDIALOG_H

#include <wx/dialog.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include "Options.h"

class SkinElementColor {
public:
	SkinElementColor();
	~SkinElementColor();

	wxTextCtrl   *m_r, *m_g, *m_b;
	wxButton     *m_rgbSelector;
	wxStaticText *m_label;
        ColorOption  *m_option;
};

class SkinColorsDialog : public wxDialog
{
public:
	SkinColorsDialog(wxWindow *parent, TDSkin *skn);
	~SkinColorsDialog();

	int	ShowModal();
	void	OnColorChoice(wxCommandEvent& event);

	SkinElementColor    m_background;
	SkinElementColor    m_freeTrack;
	SkinElementColor    m_reservedTrack;
	SkinElementColor    m_reservedShunting;
	SkinElementColor    m_occupiedTrack;
	SkinElementColor    m_workingTrack;
	SkinElementColor    m_outline;
	SkinElementColor    m_text;

	TDSkin		    *m_skin;

	DECLARE_EVENT_TABLE()
};

#endif // _SKINCOLORSDIALOG_H
