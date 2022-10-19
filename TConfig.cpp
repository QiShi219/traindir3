/*	TConfig.cpp - Created by Giampiero Caprino

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

#include "wx/wx.h"
#include "TConfig.h"
#include "trsim.h"

TConfig::TConfig()
{
	m_start = m_end = -1;
	m_nSaved = 0;
}

TConfig::~TConfig()
{
}

bool	TConfig::Load(const wxChar *fname)
{
	m_start = m_end = -1;
	m_nSaved = 0;

	if(!wxFile::Exists(fname))
	    return false;
	if(!m_file.Open(fname))
	    return false;
	return true;
}

void	TConfig::Close()
{
	m_file.Write(wxTextFileType_Dos);
	m_file.Close();
}

bool	TConfig::Save(const wxChar *fname)
{
	if(wxFile::Exists(fname) && m_file.Open(fname))
	    m_file.Clear();
	else if(!m_file.Create(fname))
	    return false;
	return true;
}

bool	TConfig::FindSection(const wxChar *name)
{
	wxString    header;
	unsigned int i;

	header = wxT("[");
	header += name;
	header += wxT("]");
	m_start = m_end = -1;
	for(i = 0; i < m_file.GetLineCount(); ++i) {
	    if(m_file[i] == header) {
		m_start = i + 1;
		m_end = m_file.GetLineCount();
	    } else if(m_start != -1 && m_file[i][0] == '[') {
		m_end = i;
		return true;
	    }
	}
	return m_start != -1;
}

bool	TConfig::PushSection(const wxChar *name)
{
	if(m_nSaved >= MAX_CONFIG_SECT)
	    return false;
	m_savedStart[m_nSaved] = m_start;
	m_savedEnd[m_nSaved] = m_end;
	if(!FindSection(name))
	    return false;
	++m_nSaved;
	return true;
}

void	TConfig::PopSection()
{
	if(m_nSaved > 0)
	    --m_nSaved;
	m_start = m_savedStart[m_nSaved];
	m_end = m_savedEnd[m_nSaved];
}

bool	TConfig::GetInt(const wxChar *var, int& result)
{
	int	i;

	for(i = m_start; i < m_end; ++i) {
	    wxString tmp;
	    if(m_file[i].StartsWith(var, &tmp)) {
	        long r;
	        bool ret;
		tmp = tmp.AfterFirst(wxT('='));
		tmp.Trim(false);
		tmp.Trim(true);
		ret = tmp.ToLong(&r);
		result = r;
		return ret;
	    }
	}
	return false;
}

bool	TConfig::GetString(const wxChar *var, wxString& result)
{
	int	i;

	for(i = m_start; i < m_end; ++i) {
	    wxString tmp;
	    if(m_file[i].StartsWith(var, &tmp)) {
		result = tmp.AfterFirst(wxT('='));
		result.Trim(false);
		return true;
	    }
	}
	return false;
}

bool    TConfig::Get(Option *option)
{
        wxString value;

        if(!GetString(option->_name.c_str(), value))
            return false;
        option->Set(value.c_str());
        return true;
}

bool    TConfig::Get(Option& option)
{
        return Get(&option);
}


void	TConfig::StartSection(const wxChar *name)
{
	m_file.AddLine(wxString(wxT('[')) + name + wxT(']'));
}

void	TConfig::PutString(const wxChar *var, const wxChar *value)
{
	m_file.AddLine(wxString(var) + wxT(" = ") + value);
}

void	TConfig::PutInt(const wxChar *var, int value)
{
	m_file.AddLine(wxString::Format(wxT("%s = %d"), var, value));
}

void    TConfig::Put(Option *option)
{
        if(!option->_sValue.empty())
            PutString(option->_name, option->_sValue.c_str());
}

void    TConfig::Put(Option& option)
{
        Put(&option);
}

void    TConfig::Put(BoolOption& option)
{
        PutInt(option._name, option._iValue);
}

void    TConfig::Put(IntOption& option)
{
        PutInt(option._name, option._iValue);
}