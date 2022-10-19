#include "wx/wx.h"
#include "AlertList.h"
#include "Traindir3.h"
#include <wx/socket.h>
#include "Client.h"

extern	void	client_command(ClientThread *src, wxChar *cmd);

ClientThread *clientThread;

ClientThread::ClientThread()
{
	sock = 0;
}

ClientThread::~ClientThread()
{
}

extern	void	post_command(wxChar *cmd);

void *ClientThread::Entry()
{
	char	buff[1];
	wxChar	line[128];
	int	nxt;

	wxIPV4address addr;
	addr.Hostname(_host);
	addr.Service(_port);
	sock = new wxSocketClient();
	if(!sock->Connect(addr, true)) {
	    wxPrintf(wxT("failed to connect %s\n"), _host);
	    return 0;
	}
	while(true) {
	    nxt = 0;
	    while(true) {
		sock->Read(buff, sizeof(buff));
		if(sock->Error())
		    break;
		int n = sock->LastCount();
		if(!n)
		    continue;
		if(buff[0] == '\r' || buff[0] == '\n') {
		    if(!nxt)
			continue;
		    line[nxt] = 0;
		    if(!wxStrcmp(line, wxT("quit"))) {
			Exit(0);
			return 0;
		    }
		    client_command(this, line);
		    nxt = 0;
		    continue;
		}
		if(nxt < sizeof(line) - 2)
		    line[nxt++] = buff[0];
	    }
	}
	printf("Done with thread.\n");
	Exit(0);
	return 0;
}

void	ClientThread::SetAddr(wxChar *host, int port)
{
	_host = host;
	_port = port;
}

void	ClientThread::Send(wxChar *cmd)
{
	if(!this || !sock || !sock->IsConnected())
	    return;
	sock->Write(cmd, wxStrlen(cmd));
}

void	create_client_thread(void)
{
        wxSocketBase::Initialize();

	clientThread = new ClientThread();
	clientThread->Create();
}

void	kill_client_thread()
{
	if(!clientThread)
	    return;
	clientThread->Kill();
	//delete clientThread;
	clientThread = 0;
}

wxSocketClient *client;

void	connect_to_client(wxChar *host, int port)
{
	if(!clientThread)
	    create_client_thread();
	clientThread->SetAddr(host, port);
	clientThread->Run();	    // connect

//	wxIPV4address addr;
//	addr.AnyAddress();
//	addr.Service(8901);
//	client = new wxSocketClient();
}

void	client_send_msg(wxChar *msg)
{
//	if(!serverThread || !serverThread->sock || !serverThread->sock->IsConnected())
//	    return;
//	serverThread->sock->Write(msg, wxStrlen(msg));
}
