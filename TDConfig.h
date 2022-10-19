#ifndef _TDCONFIG_H
#define _TDCONFIG_H

#include "wx/textfile.h"

#define MAX_CONFIG_SECT	40

class TDConfig {
public:
	TDConfig();
	~TDConfig();

	bool	Load(const char *fname);
	bool	Save(const char *fname);
	void	Close();

	bool	FindSection(const char *name);

	bool	PushSection(const char *name);
	void	PopSection();

	bool	GetInt(const char *name, int& result);
	bool	GetString(const char *name, wxString& result);

	void	StartSection(const char *name);
	void	PutString(const char *var, const wxString& value);
	void	PutInt(const char *var, int value);


	wxTextFile  m_file;
	int	m_start, m_end;

	int	m_nSaved;
	int	m_savedStart[MAX_CONFIG_SECT];
	int	m_savedEnd[MAX_CONFIG_SECT];
};

#endif // _TDCONFIG_H
