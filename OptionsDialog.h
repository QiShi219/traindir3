/*	OptionsDialog.h - Created by Giampiero Caprino

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

#ifndef _OPTIONSDIALOG_H
#define _OPTIONSDIALOG_H

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/radiobox.h>
#include "Options.h"
#include "SkinColorsDialog.h"

#define	NUM_OPTIONS	21

class FileRow
{
public:
        FileRow() { };
        ~FileRow() { };

        Option      *_option;
        wxTextCtrl  *_path;
        wxButton    *_button;
};

class OptionsDialog : public wxDialog
{
public:
	OptionsDialog(wxWindow *parent);
	~OptionsDialog();
	void	OnColorChoice(wxCommandEvent& event);
        void	OnPath1(wxCommandEvent& event);
	void	OnPath2(wxCommandEvent& event);
	void	OnPath3(wxCommandEvent& event);
            void    EnterPath(FileRow &row);
            void    EnterDirPath(FileRow &row);

        wxBoxSizer *AddFileRow(wxPanel *page, Option& option, FileRow& out, int id);

        wxPanel *CreatePage2();
        wxPanel *CreatePage3();
        wxPanel *CreatePage4();

	int	ShowModal();

	wxCheckBox  *m_boxes[NUM_OPTIONS + 2];

        FileRow     _alert, _enter, _search;

        SkinElementColor    m_background;
	SkinElementColor    m_freeTrack;
	SkinElementColor    m_reservedTrack;
	SkinElementColor    m_reservedShunting;
	SkinElementColor    m_occupiedTrack;
	SkinElementColor    m_occupiedTrack1;
	SkinElementColor    m_workingTrack;
	SkinElementColor    m_outline;
	SkinElementColor    m_text;

        wxCheckBox  *_httpServerEnabled;
        wxTextCtrl  *_httpPort;
        wxTextCtrl  *_userName;

        DECLARE_EVENT_TABLE()
};

class SelectPowerDialog : public wxDialog
{
public:
	SelectPowerDialog(wxWindow *parent);
	~SelectPowerDialog();

	int	ShowModal();

        wxComboBox  *m_power;
	//wxTextCtrl  *line_num;
};

class SetTrackLengthDialog : public wxDialog
{
public:
	SetTrackLengthDialog(wxWindow *parent);
	~SetTrackLengthDialog();
	int	ShowModal();
        wxTextCtrl *m_length;
};
extern  SetTrackLengthDialog *gSetTrackLengthDialog;

extern  IntOption       editor_gauge;

#endif // _OPTIONSDIALOG_H
