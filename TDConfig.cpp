#include "wx/wx.h"
#include "TDConfig.h"

TDConfig::TDConfig()
{
	m_start = m_end = -1;
	m_nSaved = 0;
}

TDConfig::~TDConfig()
{
}

bool	TDConfig::Load(const char *fname)
{
	m_start = m_end = -1;
	m_nSaved = 0;

	if(!m_file.Open(fname))
	    return false;
	return true;
}

void	TDConfig::Close()
{
	m_file.Write(wxTextFileType_Dos);
	m_file.Close();
}

bool	TDConfig::Save(const char *fname)
{
	if(m_file.Open(fname))
	    m_file.Clear();
	else if(!m_file.Create(fname))
	    return false;
	return true;
}

bool	TDConfig::FindSection(const char *name)
{
	wxString    header;
	unsigned int i;

	header = "[";
	header += name;
	header += "]";
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

bool	TDConfig::PushSection(const char *name)
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

void	TDConfig::PopSection()
{
	if(m_nSaved > 0)
	    --m_nSaved;
	m_start = m_savedStart[m_nSaved];
	m_end = m_savedEnd[m_nSaved];
}

bool	TDConfig::GetInt(const char *var, int& result)
{
	int	i;

	for(i = m_start; i < m_end; ++i) {
	    const char    *p = m_file[i].c_str();
	    if(!strncmp(p, var, strlen(var))) {
		p += strlen(var);
		while(*p == ' ' || *p == '\t') ++p;
		if(*p == '=') ++p;
		while(*p == ' ' || *p == '\t') ++p;
		result = atoi(p);
		return true;
	    }
	}
	return false;
}

bool	TDConfig::GetString(const char *var, wxString& result)
{
	int	i;

	for(i = m_start; i < m_end; ++i) {
	    const char    *p = m_file[i].c_str();
	    if(!strncmp(p, var, strlen(var))) {
		p += strlen(var);
		while(*p == ' ' || *p == '\t') ++p;
		if(*p == '=') ++p;
		while(*p == ' ' || *p == '\t') ++p;
		result = p;
		return true;
	    }
	}
	return false;
}

void	TDConfig::StartSection(const char *name)
{
	char	buff[256];

	sprintf(buff, "[%s]", name);
	m_file.AddLine(buff);
}

void	TDConfig::PutString(const char *var, const wxString& value)
{
	char	buff[256];

	sprintf(buff, "%s = %s", var, value.c_str());
	m_file.AddLine(buff);
}

void	TDConfig::PutInt(const char *var, int value)
{
	char	buff[256];

	sprintf(buff, "%s = %d", var, value);
	m_file.AddLine(buff);
}
