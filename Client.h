#include "wx/wx.h"
#include "TDFile.h"
#include "TimeTblView.h"
#include "TrainInfoList.h"
#include "Canvas.h"
#include "AlertList.h"
#include "MainFrm.h"
#include "Traindir3.h"
#include "FontManager.h"
#include "Html.h"
#include <wx/socket.h>

class ClientThread : public wxThread
{
public:
	ClientThread();
	~ClientThread();

	void	SetAddr(wxChar *host, int port);
	void	Send(wxChar *cmd);

	wxChar	*_host;
	int	_port;
	wxSocketClient *sock;
	ExitCode    Entry();
};

