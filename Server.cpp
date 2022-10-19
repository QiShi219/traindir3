#include "wx/wx.h"
#include "wx/splitter.h"
#include "wx/listctrl.h"
#include "wx/html/htmlwin.h"
#include "wx/image.h"		// for InitAllImageHandlers
#include "wx/filedlg.h"
#include "wx/ffile.h"
#include "wx/fs_zip.h"
#include "wx/sound.h"
#include "TDFile.h"
#include "TimeTblView.h"
#include "TrainInfoList.h"
#include "Canvas.h"
#include "AlertList.h"
#include "MainFrm.h"
#include "Traindir3.h"
#include "FontManager.h"
#include "Html.h"
#include "mongoose.h"
#include "SwitchBoard.h"
#include "Array.h"
#include "Server.h"
#include <wx/socket.h>
#include <wx/file.h>
#include <wx/stdpaths.h>
#include<fstream>

#include <string.h>
#define  recieveLen  40960
#include"message.h"//202038

extern	int server_port;		// command/log port - defined in Main.cpp
extern  int accuracy;
bool        atoAtEveryStation = 1;

int     udp_port = 8000;
extern  int     waitAto;   //等待通信接口接收ato曲线
extern  int     waitAction;//等待接口发送下一步动作（时刻表）

IntOption     http_server_port(wxT("httpPort"),
                               wxT("Listen on this HTTP port"),
                               wxT("Server"),
                               8999);
BoolOption      http_server_enabled(wxT("httpServerEnabled"),
                                    wxT("Enable listening for HTTP clients"),
                                    wxT("Server"),
                                    true);

extern	Traindir *traindir;

Servlet *services;

char MIME_TEXT[64] =  "text/plain; charset=Windows-1252";
char MIME_HTML[64] =  "text/html; charset=Windows-1252";
char MIME_XML[64]  =  "text/xml; charset=Windows-1252";
char MIME_JSON[64] =  "application/json; charset=Windows-1252";

bool	start_web_server();
void	stop_web_server();
void	do_command(const wxChar *cmd, bool sendToClients);
bool    ReceiveFile(wxSocketBase *sock1);
bool    ReceiveAto(wxSocketBase *sock1) ;//YangPX tcp
//bool    ReceiveAto(char *buff) ;//YangPX udp
bool  TimeWait(wxSocketBase *sock1);//lrg 20211017
extern int tcp_flag;//lrg 20211017
void    parse_udp(byte *buff);
extern  int     ato_flag;
//lrg 20211017 与matlab传输车站标识使用
extern const char name[31][10]={"秦","山","绥","葫","锦","盘","台","辽中","沈北","铁","开","昌","四","公","长","德","扶","双","哈","沈阳","沈南","辽","鞍",
"海城","营","盖州","鲅鱼","瓦房","普湾","大","盘锦"};

class ServerThread : public wxThread
{
public:
	ServerThread();
	~ServerThread();

	wxSocketBase *sock;
	//wxSocketBase *sock2;
	wxDatagramSocket *sockudp;

	ExitCode    Entry();


};

class ServerThread1 : public wxThread   //0106
{
public:
	ServerThread1();
	~ServerThread1();

	wxSocketBase *sock1;

	ExitCode    Entry();


};
class ServerThread2 : public wxThread   //20220119DMI
{
public:
	ServerThread2();
	~ServerThread2();

	wxSocketBase *sock;
	wxDatagramSocket *sockudp;

	ExitCode    Entry();


};

ServerThread *serverThread;
ServerThread1 *serverThread1; //ypx0106
ServerThread2 *serverThread2; //ypx0106

ServerThread::ServerThread()
{
	sock = 0;
}
ServerThread::~ServerThread()
{
}
ServerThread1::ServerThread1()
{
	sock1 = 0;
}

ServerThread1::~ServerThread1()
{
}
ServerThread2::ServerThread2()
{
	sock = 0;
}

ServerThread2::~ServerThread2()
{
}

extern	void	post_command(wxChar *cmd);

void *ServerThread::Entry()
{
	byte	buff[recieveLen];
	//char    buff1[1024];
	wxChar	line[128];
	int	nxt,rcv,ato;
	char *msg1;
	const char *length;
	
	const wxChar	*quit = wxT("quit");
	const wxChar	*newsch = wxT("newsch");
	const wxChar	*newato = wxT("newato");

	wxIPV4address addr;
	addr.AnyAddress(); 
	addr.Service(server_port);
	//addr.IPAddress();
	//wxSocketServer *srvr = new wxSocketServer(addr, wxSOCKET_NONE);		
	sockudp = new wxDatagramSocket(addr,wxSOCKET_REUSEADDR);
    //wxSOCKET_REUSEADDR);

	while(true) {
		    //sock->Read(buff, sizeof(buff)); 
			sockudp->Read(buff, sizeof(buff));

			if(buff[0]<=0)
				continue;
		    wxChar	msg1[128];
			
			int i = 0;
			
			while(buff[i]>0&&buff[i]<127){ msg1[i] = buff[i];i++;}
			msg1[i] = 0;

			if(!wxStrncmp(msg1, quit,4)) {
			    Exit(0);
			    return 0;
			}

			if(!wxStrncmp(msg1, newsch,6)){
				rcv=ReceiveFile(sock);
				if (!rcv)
					continue;
			}
			if(!wxStrncmp(msg1, "toggle",6)){
				atoAtEveryStation = !atoAtEveryStation;
					continue;
			}
			if(buff[0] == 0x3A){

				parse_udp(buff);
				continue;
		/*	ato=ReceiveAto(buff+7);
			if (!ato)
				break;*/
                 
			}
			traindir->AddAlert(msg1);
			post_command(msg1);
			nxt = 0;
			memset(buff,0,sizeof(recieveLen));
			continue;

			}

		   /* if(nxt < sizeof(line) - 2)
			line[nxt] = buff[0];
				
				nxt++;
	}*/
		//sock = 0;
	/*	}
	}*/
	printf("Done with thread.\n");
	Exit(0);
	return 0;
}
//tcp 下达指令 到站报点
void *ServerThread1::Entry()
{
    char	buff[1];
	//char    buff1[1024];
	wxChar	line[128];
	int	nxt,rcv,ato;
	char *msg1;
	const char *length;
	int wait_time;
	const wxChar	*quit = wxT("quit");
	const wxChar	*newsch = wxT("newsch");
	const wxChar	*newato = wxT("newato");

	wxIPV4address addr1;
	addr1.AnyAddress(); 
	addr1.Service(server_port);
	wxSocketServer *srvr1 = new wxSocketServer(addr1, wxSOCKET_REUSEADDR);



	while(true) {
		sock1 = srvr1->Accept();
		if(sock1 != 0) {
		nxt = 0;
		while(true) {
		    sock1->Read(buff, sizeof(buff));
		    if(sock1->Error())
			break;
		    int n = sock1->LastCount();
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
			if(!wxStrcmp(line, wxT("newsch"))){
				rcv=ReceiveFile(sock1);
				if (!rcv)
					break;
                 
			}
			/*if(!wxStrncmp(line, wxT("newato"),6)){
				ato=ReceiveAto(buff);
				if (!ato)
					break;
                 
			}*/
			if(!wxStrncmp(line, wxT("waittime"),8)){
				wait_time=TimeWait(sock1);
				if (!wait_time)
					break;
                 
			}
			traindir->AddAlert(line);
			post_command(line);
			nxt = 0;
			continue;
			}

		    if(nxt < sizeof(line) - 2)
			line[nxt] = buff[0];
				
				nxt++;
		}
		sock1 = 0;
		}
	}
	printf("Done with thread.\n");
	Exit(0);
	return 0;
}
//udp 北交王洪伟ato
void *ServerThread2::Entry()
{
	byte	buff[recieveLen];
	//char    buff1[1024];
	wxChar	line[128];
	int	nxt,rcv,ato;
	char *msg1;
	const char *length;
	
	const wxChar	*quit = wxT("quit");
	const wxChar	*newsch = wxT("newsch");
	const wxChar	*newato = wxT("newato");

	wxIPV4address addr;
	addr.AnyAddress(); 
	addr.Service(8000);//端口8000
	//addr.IPAddress();
	//wxSocketServer *srvr = new wxSocketServer(addr, wxSOCKET_NONE);		
	sockudp = new wxDatagramSocket(addr,wxSOCKET_REUSEADDR);
    //wxSOCKET_REUSEADDR);

	while(true) {
		    //sock->Read(buff, sizeof(buff)); 
			sockudp->Read(buff, sizeof(buff));

			if(buff[0]<=0)
				continue;
		    wxChar	msg1[128];
			
			int i = 0;
			
			while(buff[i]>0&&buff[i]<127){ msg1[i] = buff[i];i++;}
			msg1[i] = 0;

			if(!wxStrncmp(msg1, quit,4)) {
			    Exit(0);
			    return 0;
			}

			if(!wxStrncmp(msg1, newsch,6)){
				rcv=ReceiveFile(sock);
				if (!rcv)
					continue;
			}
			if(!wxStrncmp(msg1, "toggle",6)){
				atoAtEveryStation = !atoAtEveryStation;
					continue;
			}
			if(buff[0] == 0x3A){

				parse_udp(buff);
				continue;
		/*	ato=ReceiveAto(buff+7);
			if (!ato)
				break;*/
                 
			}
			traindir->AddAlert(msg1);
			post_command(msg1);
			nxt = 0;
			memset(buff,0,sizeof(recieveLen));
			continue;

			}
	printf("Done with thread.\n");
	Exit(0);
	return 0;
}


void	start_server_thread(void)
{
        wxSocketBase::Initialize();

	serverThread = new ServerThread();
	serverThread->Create();
	serverThread->Run();
	/*start_web_server();*/
}

void	start_server_thread1(void)
{
        wxSocketBase::Initialize();

	serverThread1 = new ServerThread1();
	serverThread1->Create();
	serverThread1->Run();
	/*start_web_server();*/
}
void	start_server_thread2(void)
{
        wxSocketBase::Initialize();

	serverThread2 = new ServerThread2();
	serverThread2->Create();
	serverThread2->Run();
	/*start_web_server();*/
}

void	kill_server_thread()
{
	stop_web_server();
	if(!serverThread)
	    return;
	serverThread->Kill();
	serverThread1->Kill();
	serverThread2->Kill();
	//delete serverThread;
}


//void	send_msg1(wxChar *msg,int send_len)
//{
//	if(!serverThread1 || !serverThread1->sock1 || !serverThread1->sock2->IsConnected())
//	    return;
//	if(send_len == 0)
//	serverThread1->sock2->Write(msg, wxStrlen(msg));
//	else
//	serverThread1->sock2->Write(msg, send_len);
//	
//}

void	send_msg(wxChar *msg,int send_len)
{
	if(!serverThread1 || !serverThread1->sock1 || !serverThread1->sock1->IsConnected())
	    return;
	serverThread1->sock1->Write(msg, send_len);
}
void	send_udp1_msg(wxChar *msg,int send_len,int flag)
{
	//if(!serverThread || !serverThread->sock || !serverThread->sock->IsConnected())sockudp
	wxIPV4address addr;
	//addr.Hostname("219.216.103.248");
	
	addr.Hostname("219.216.80.15");
	if(flag == 0)
	addr.Service(6502);
	else
	addr.Service(7001);

	if(!serverThread || !serverThread->sock || !serverThread->sock->IsConnected())
	{
		if(!serverThread || !serverThread->sockudp)
	    return;
	}
	//serverThread->sock->Write(msg, wxStrlen(msg));
	if(send_len == 0)
	serverThread->sockudp->SendTo(addr,msg, wxStrlen(msg));
	else
	serverThread->sockudp->SendTo(addr,msg, send_len);
}
void	send_udp2_msg(wxChar *msg,int send_len)
{
	//if(!serverThread || !serverThread->sock || !serverThread->sock->IsConnected())sockudp
	wxIPV4address addr;
	//addr.Hostname("219.216.103.248");
	
	addr.Hostname("219.216.103.196");
	addr.Service(7001);
	

	if(!serverThread2 || !serverThread2->sock || !serverThread2->sock->IsConnected())
	{
		if(!serverThread2 || !serverThread2->sockudp)
	    return;
	}
	if(send_len > 0)
	serverThread2->sockudp->SendTo(addr,msg, send_len);
	else
		serverThread2->sockudp->SendTo(addr,msg, wxStrlen(msg));
	
}

void insert_file(wxString& str, const Char *fname)
{
	Char  buff[1024];
        TDFile file(fname);

        if(file.Load()) {
	    while(file.ReadLine(buff, sizeof(buff)/sizeof(buff[0]))) {
	        str.Append(buff);
	        str.Append(wxT("\n"));
	    }
        }
}

void send_refresh(mg_connection *conn, wxChar *urlBase)
{
        wxString str;

        str.Append(wxT("<html><head>\n"));
        str.Append(wxT("<META HTTP-EQUIV=\"refresh\" CONTENT=\"1;URL="));
        str.Append(urlBase);
        str.Append(wxT("\">\n"));
        str.Append(wxT("</head><body>Refreshing...</body></html>\n"));
        const char *mimeType = "text/html";
        char *content = strdup(str.mb_str(wxConvUTF8));
        int contentLength = strlen(content);
        mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                        "Cache: no-cache\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %d\r\n"
                        "\r\n",
                        mimeType,
                        contentLength);
        mg_write(conn, content, contentLength);
        free(content);
}

volatile bool   server_command_done;

void    wait_command(Char *cmd)
{
        server_command_done = false;
        post_command(cmd);
        do {
#if defined(__WXMAC__) || defined(__unix__)
	    usleep(100000);
#else
            Sleep(100);
#endif
        } while(!server_command_done);
}


Char *convToUCode(const char *p)
{
#if defined(__WXMAC__) || defined(__unix__) || wxUSE_UNICODE
	int i;

        wxWritableCharBuffer tmpMB(strlen(p) + 4);
        if ((char *) tmpMB == NULL)
	    return 0;
        char *buffer = tmpMB;
        for(i = 0; *p; buffer[i++] = *p++);
        buffer[i] = 0;
        Char *content;
        /* wxConvAuto can't (as of wxWidgets 2.8.7) handle ISO-8859-1.  */
        if (! (content = wxConvAuto().cMB2WX(tmpMB).release()))
	    if (! (content = wxConvISO8859_1.cMB2WX(tmpMB).release()))
	        return 0;
        return content;
#else
        return strdup(p);
#endif
}

char *convToByte(const Char *p)
{
	wxString str(p);
	return strdup(str.mb_str(wxConvUTF8));
}

//char    curdir[512];

void *event_handler(enum mg_event event,
	struct mg_connection *conn,
	const struct mg_request_info *request_info)
{
	static void* done = (void *)"done";
	const char* mimeType = "text/plain";
	const char* content = "Not found";
	int contentLength = strlen(content);
        wxString str;
	wxChar  buff[512];

//        getcwd(buff, sizeof(buff));

//#ifndef __WXMAC__
	if (event == MG_NEW_REQUEST) {
//	    traindir->AddAlert(request_info->uri);
	    if (strncmp(request_info->uri, "/switchboard/", 13) == 0) {
		if(strcmp(request_info->request_method, "GET") != 0) {
error:
		    // send error (we only care about HTTP GET)
		    mg_printf(conn, "HTTP/1.1 %d Error (%s)\r\n\r\n%s",
			500,
			"we only care about HTTP GET",
			"we only care about HTTP GET");
		    // return not null means we handled the request
		    return done;
		}

		const char *host = mg_get_header(conn, "Host");

		wxChar  urlBase[512];
		wxChar  cmdBuff[512];
		Char *uri;
		uri = convToUCode(request_info->uri + 13);
	        if(!uri)
		    goto error;
		wxStrncpy(buff, uri, sizeof(buff)/sizeof(buff[0]));	// isolate switchboard name
		Char *p = wxStrchr(buff, wxT('/'));
		if(p)
		    *p = 0;
		// handle your GET request to /hello
		SwitchBoard *sw = FindSwitchBoard(buff);
		if(sw) {
		    Char *base = convToUCode(host);
		    wxSnprintf(urlBase, sizeof(urlBase)/sizeof(urlBase[0]), wxT("http://%s/switchboard/%s"), base, buff);
		    free(base);
		    const Char *p = uri;
		    p = wxStrchr(p, '/');
		    if(p) ++p;
	            if (p && wxStrncmp(p, wxT("command/"), 8) == 0) {
                        do_command(p + 8, true);        // this is dangerous since we are not in the UI thread!
                        send_refresh(conn, urlBase);
			free(uri);
                        return done;
                    }
		    if(p && *p) {
			Char *pp;
		        int x = wxStrtol(p, &pp, 10);
		        if(*pp == '/') ++pp;
		        int y = wxStrtol(pp, &pp, 10);
                        wxSnprintf(cmdBuff, sizeof(cmdBuff)/sizeof(cmdBuff[0]), wxT("switch %d,r%d %s"), x, y, buff);
                        wait_command(cmdBuff);
		        //sw->Select(x, y);     // don't do this since we are not in the GUI thread
                        send_refresh(conn, urlBase);
			free(uri);
                        return done;
		    }
                    if(request_info->query_string && request_info->query_string[0]) {
			p = convToUCode(request_info->query_string);
                        if(*p == 'i' && p[1] == '=')
                            p += 2;
                        wxSnprintf(cmdBuff, sizeof(cmdBuff)/sizeof(cmdBuff[0]), wxT("switch '%s' %s"), p, buff);
                        wait_command(cmdBuff);
                        send_refresh(conn, urlBase);
			free(uri);
			free((void *)p);
                        return done;
                    }
		    //free(uri);
		    str.Append(wxT("<html>\n"));
                    insert_file(str, wxT("tdstyle.css"));
		    str.Append(wxT("<body>\n"));
                    ///insert_file(str, wxT("body_header.html"));
                    wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]),
			wxT("<form name=\"iform\" method=\"get\" action=\"http://%s/switchboard/%s\">"),
			host, sw->_fname.c_str(), sw->_name.c_str());
                    str.Append(buff);
                    str.Append(wxT("<input type=\"text\" name=\"i\"></form><br>\n"));
		    SwitchBoard *ss;
		    str.Append(wxT("<ul class=\"navbar\">\n<br /><br />"));
		    for(ss = switchBoards; ss; ss = ss->_next) {
			if(!wxStrcmp(ss->_name.c_str(), sw->_name.c_str())) {
			    str.Append(wxT("<li class='selected'>"));
			    str.Append(ss->_name);
			    str.Append(wxT("</li>\n"));
			} else {
			    str.Append(wxT("<li class='other'>"));
			    base = convToUCode(host);
			    wxString sss;
			    sss.Printf(wxT("<a href=\"http://%s/switchboard/%s\">%s</a>"),
                                base, ss->_fname.c_str(), ss->_name.c_str());
			    str.Append(sss.c_str());
			    free(base);
/*
			    wxSnprintf(buff, sizeof(buff)/sizeof(buff[0]),
				wxT("<a href=\"http://%s/switchboard/%s\">%s</a>"),
				host, ss->_fname.c_str(), ss->_name.c_str());
			    str.Append(buff);
*/
			    str.Append(wxT("</li>\n"));
			}
		    }
		    str.Append(wxT("</ul>\n"));
		    sw->toHTML(str, urlBase);
//		    str.Append(wxT("<p><a href=\""));
//		    str.Append(urlBase);
//		    str.Append(wxT("\">Refresh</a>\n"));
                    ///insert_file(str, wxT("body_footer.html"));
                   // str.Append(wxT("<body OnLoad=\"document.iform.i.focus();\">\n"));
		   // str.Append(wxT("</body></html>\n"));
                    ///str.Append(wxT("<body>\n"));
                    ///str.Append(wxT("</body>\n"));
		    mimeType = "text/html";
		    content = strdup(str.mb_str(wxConvUTF8));
		    contentLength = strlen(content);
		}
		mg_printf(conn,
		    "HTTP/1.1 200 OK\r\n"
		    "Cache: no-cache\r\n"
		    "Content-Type: %s\r\n"
		    "Content-Length: %d\r\n"
		    "\r\n",
		    mimeType,
		    contentLength);
		mg_write(conn, content, contentLength);
		if(sw)
		    free((void *)content);
		return done;
	    }
	    // in this example i only handle /hello
//	    mg_printf(conn, "HTTP/1.1 %d Error (%s)\r\n\r\n%s",
//		500, /* This the error code you want to send back*/
//		"Invalid Request.",
//		"Invalid Request.");
//	    return done;
            Servlet *s;
            for(s = services; s; s = s->_next) {
                if(!strncmp(request_info->uri, s->_path, strlen(s->_path))) {
		    Char *uri = 0;
                    if(request_info->query_string) {
                        uri = convToUCode(request_info->query_string);
	                if(!uri)
		            goto error;
		        wxStrncpy(buff, uri, sizeof(buff)/sizeof(buff[0]));
                    } else if(request_info->uri + strlen(s->_path)) {
                        uri = convToUCode(request_info->uri + strlen(s->_path));
	                if(!uri)
		            goto error;
		        wxStrncpy(buff, uri, sizeof(buff)/sizeof(buff[0]));
                    } else
                        buff[0] = 0;
		    if(!strcmp(request_info->request_method, "GET")) {
                        int len;
                        void *binout;
                        if (s->get(&binout, len, buff)) {
                            mimeType = s->getMimeType();
                            contentLength = len;
                            content = (char *)binout;
                            goto binary_output;
                        }
                        if(!s->get(str, buff)) {
                            goto error;
                        }
                        /*
                        sprintf(buff, "C:\\eclipse-js\\workspace\\TrainDirectorClient\\%s", request_info->uri);
                        FILE *fp = fopen(buff, "w");
                        if(fp) {
                            fwrite(str.c_str(), 1, str.length(), fp);
                            fclose(fp);
                        }
                        return 0;
                        */
                    } else if(!strcmp(request_info->request_method, "POST")) {
                        if(!s->post(str, buff)) {
                            goto error;
                        }
                    } else {
		        // send error (we only care about HTTP GET and POST
bad_method:
		        mg_printf(conn, "HTTP/1.1 %d Error (%s)\r\n\r\n%s",
			    405,
			    "Method not allowed",
			    "Method not allowed");
		        // return not null means we handled the request
		        return done;
                    }
	            mimeType = s->getMimeType();
	            content = strdup(str.mb_str(wxConvUTF8));
	            contentLength = strlen(content);
		    mg_printf(conn,
		        "HTTP/1.1 200 OK\r\n"
		        "Cache: no-cache\r\n"
		        "Content-Type: %s\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Content-Length: %d\r\n"
		        "\r\n",
		        mimeType,
		        contentLength);
		    mg_write(conn, content, contentLength);
	            free((void *)content);
		    return done;
binary_output:
#if 0
                    mg_printf(conn,
		        "HTTP/1.1 206 Partial Content\r\n"
		        "Cache: no-cache\r\n"
		        "Content-Type: %s\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Content-Range: 10/100"
                        "Content-Length: %d\r\n"
		        "\r\n",
		        mimeType,
		        contentLength);
		    mg_write(conn, content, contentLength);
                    wxSleep(10);
#endif
		    mg_printf(conn,
		        "HTTP/1.1 200 OK\r\n"
		        "Cache: no-cache\r\n"
		        "Content-Type: %s\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Content-Length: %d\r\n"
		        "\r\n",
		        mimeType,
		        contentLength);
		    mg_write(conn, content, contentLength);
	            free((void *)content);
		    return done;
                }
            }
	}
//#endif

	// No suitable handler found, mark as not processed. Mongoose will
	// try to serve the request.
	return NULL;
}

/* Initialize HTTP layer */
static struct mg_context *ctx;

bool	start_web_server()
{
        char port_number[20];
	/* Default options for the HTTP server */
	const char *options[] = {
	    "listening_ports", "8081",
	    "num_threads", "2",
////	    "enable_keep_alive", "yes",
            "document_root", "C:\\TrainDir",
	    NULL
	};

        const Char *root;
        wxString rootStr = wxStandardPaths::Get().GetExecutablePath();
        root = rootStr.c_str();
        if(wxGetenv(wxT("TDHOME"))) {
	    rootStr = wxGetenv(wxT("TDHOME"));
        }
        options[5] = strdup(rootStr.mb_str(wxConvUTF8));
        //options[5] = wxStrdup(root);
#if !defined(__WXGTK__) && !defined(__WXMAC__)
        char *p = (char *)strstr(options[5], "\\traindir3.exe");
#else
#ifdef __WXMAC__
        char *p = (char *)strstr(options[5] + strlen(options[5]) - 3, "td3");
#else
	// ends with traindir3?
        char *p = (char *)strstr(options[5] + strlen(options[5]) - 9, "traindir3");
#endif
#endif
        if(p)
            *p = 0;
        sprintf(port_number, "%d", http_server_port._iValue);
        options[1] = port_number;
//        options[5] = "C:\\eclipse-js\\workspace\\TrainDirectorClient";
	ctx = mg_start(&event_handler, NULL, options);
	if(ctx == NULL) {
	    return false;
	}
	return true;
}

void	stop_web_server() {
	if(ctx)
	    mg_stop(ctx);
}



//
//
//

void    registerWebService(Servlet *s)
{
        s->_next = services;
	services = s;
}

Servlet::Servlet(const char *path)
{
        _path = strdup(path);
        registerWebService(this);
}

Servlet::~Servlet()
{
        free((void *)_path);
}

void    Servlet::escape(Char *escaped, int maxlen, const Char *value)
{
        int         d = 0;

        if(value) {
            for(int i = 0; value[i]; ++i) {
                if(d >= maxlen - 3)
                    break;
                if(value[i] == '"' || value[i] == '\\')
                    escaped[d++] = '\\';
                escaped[d++] = value[i];
            }
        }
        escaped[d] = 0;
}


void    Servlet::json(wxString& out, const wxChar *field, const wxChar *value, bool last)
{
        Char        escaped[1024];
        wxString    str;

        escape(escaped, sizeof(escaped) / sizeof(escaped[0]), value);
        str.Printf(wxT("\"%s\": \"%s\"%s"), field, escaped, last ? wxT("\n") : wxT(",\n"));
        out.append(str);
}

void    Servlet::json(wxString& out, const wxChar *field, int value, bool last)
{
        wxString    str;
        str.Printf(wxT("\"%s\": %d"), field, value);
        out.append(str);
        out.append(last ? wxT("\n") : wxT(",\n"));
}

void    Servlet::json(Char *out, int outsize, const Char *name, const Char *value, const Char *sep, bool last)
{
        int     x;
        for(x = 0; out[x]; ++x);
        while(x < outsize - 5 && *sep) {
            out[x++] = *sep++;
        }
        out[x++] = '"';
        while(x < outsize - 1 && *name) {
            out[x++] = *name++;
        }
        out[x++] = '"';
        out[x++] = ':';
        out[x++] = ' ';
        out[x++] = '"';
        while(x < outsize - 1 && *value) {
            if(*value == '"')
                out[x++] = '\\';
            out[x++] = *value++;
        }
        out[x++] = '"';
        if(!last) {
            out[x++] = ',';
            out[x++] = '\n';
        }
        out[x] = 0;
}

void    Servlet::FromUrl(Char *dest, const Char *src)
{
        int x, y;
        while(*src) {
            if(*src != '%') {
                *dest++ = *src++;
                continue;
            }
            x = *++src;
            if(!x)
                break;
            if(x >= 'A' && x <= 'F')
                x = x - 'A' + 10;
            else if(x >= 'a' && x <= 'f')
                x = x - 'a' + 10;
            else
                x -= '0';
            y = *++src;
            if(!y)
                break;
            if(y >= 'A' && y <= 'F')
                y = y - 'A' + 10;
            else if(y >= 'a' && y <= 'f')
                y = y - 'a' + 10;
            else
                y -= '0';
            *dest++ = (x << 4) | y;
            ++src;
        }
        *dest = 0;
        dest[1] = 0;
}


bool  ReceiveFile(wxSocketBase *sock1)  //tcpip by YangPX
{
char buff[1],allreceive[5120];
int length,index=0;
Message::MsgHead *msgHead;


FILE *fp;
fp = fopen( "D:/received.sch","w");

if (fp==NULL)
   return false;


  memset(buff,0,sizeof(char));

    int i=0;
	///过滤最开始的0
	while(true){


		sock1->Read(buff, sizeof(buff));

		if(!wxStrncmp(buff, wxT("#"), 1)){
		fwrite(buff,sizeof buff[0],sizeof(buff),fp);
		break;
	  }
	}
	
   
	while(true)

	{
   
		sock1->Read(buff, sizeof(buff));


 		/*if(!i++)
			continue;*/
        
        if((byte)buff[0] == 0x24)
			break;

		 int n = sock1->LastCount();
		    if(!n)
			break;
		 
		if(sock1->Error())
			break;

		fwrite(buff,sizeof buff[0],sizeof(buff),fp);

	  memset(buff,0,sizeof(char));

	}


  fclose(fp);

  sock1=0;
  waitAction = 0;
  
  //do_command(wxT("newsch"), false);
  do_command(wxT("run"), false);
  return true;

}

bool  TimeWait(wxSocketBase *sock1)  //tcpip by LRG  20211017
{
 char buff[1];
 char buffer[50];//保存算法端发送的车次号及停站时间
 char buffer1[50];
  
	int tempi = 0;
	int tempj=0;
	int temp1=0;
	int temp2=0;
char late_train[10];
char late_time[10];
char late_sta[10];
int num[10];//假设传输信息个数不多于11个，共10个空格
int temp1_num= 0;
int temp2_num=0;
    memset(buff,0,sizeof(char));

int end_flag = 0;//整次传输结束标志
while(!end_flag)
{
   while(true)
   {
	temp1_num++;//
	sock1->Read(buff, sizeof(buff));
	if(!wxStrncmp(buff, wxT(" "), 1))//
		num[temp2_num++]=temp1_num;//
	if(!wxStrncmp(buff, wxT(";"), 1))//规定;视为单次传输结束
	{
		num[temp2_num++]=temp1_num;//
			break;
	}
	if(!wxStrncmp(buff, wxT("@"), 1))//规定@视为整次传输结束
	{
		num[temp2_num++]=temp1_num;//
		end_flag = 1;
			break;
	}

	buffer[tempi] = buff[0];
	buffer1[tempi] = buff[0];
	tempi=tempi+1;
	memset(buff,0,sizeof(char));
	
   }
	
	strncpy(late_train,buffer,num[0]-1);//保存车次号
	for(int tempk =num[0];tempk<num[1]-1;tempk++)
		late_time[tempk-num[0]]=buffer[tempk];
	for(int tempm =num[1];tempm<tempi;tempm++)
	{
		late_sta[tempm-num[1]]=buffer[tempm];
	}
	strcpy(late_sta,name[atoi(late_sta)]);

	
	Train *temp_sch;
	TrainStop * temp_tstop;
	for(temp_sch = schedule;temp_sch;temp_sch = temp_sch->next)
	{
		if(!wxStrncmp(temp_sch->name,late_train,num[0]-1))//找到调度算法针对的晚点车辆
		{//这里将buff值赋给schedule指针以保存
			temp_sch->sta_waittime = atoi(late_time);
			temp_sch->timedep = current_time + (int)(temp_sch->sta_waittime*60);
			temp_sch->nxtstop->departure_actual = temp_sch->timedep;
			temp_sch->sta_waittime =0;
			break;
		}
	}
	memset(buff,0,sizeof(char));
	temp1_num = 0;
	temp2_num = 0;
	memset(buffer,0,sizeof(char)*50);
	memset(buffer1,0,sizeof(char)*50);
	memset(late_train,0,sizeof(char)*10);
	memset(late_time,0,sizeof(char)*10);
	memset(late_sta,0,sizeof(char)*10);
	memset(num,0,sizeof(int)*10);
	tempi = 0;
	tempj=0;
	temp1=0;
	temp2=0;
}


 //  while(true)
 //  {
	//temp1_num++;//
	//sock1->Read(buff, sizeof(buff));
	//if(!wxStrncmp(buff, wxT(" "), 1))//
	//	num[temp2_num++]=temp1_num;//
	//if(!wxStrncmp(buff, wxT("@"), 1))//规定@视为传输结束
	//{
	//	num[temp2_num++]=temp1_num;//
	//		break;
	//}

	//buffer[tempi] = buff[0];
	//buffer1[tempi] = buff[0];
	//tempi=tempi+1;
	//memset(buff,0,sizeof(char));
	//
 //  }
	//
	//strncpy(late_train,buffer,num[0]-1);//保存车次号
	//for(int tempk =num[0];tempk<num[1]-1;tempk++)
	//	late_time[tempk-num[0]]=buffer[tempk];
	//for(int tempm =num[1];tempm<tempi;tempm++)
	//{
	//	late_sta[tempm-num[1]]=buffer[tempm];
	//}
	//strcpy(late_sta,name[atoi(late_sta)]);


	////for (tempj=0;tempj<50;tempj++)
	////{
	////	if(buffer[tempj]=='&')
	////		temp1 = tempj;//保存第一个间隔位置
	////	if(buffer[tempj]=='$')
	////	{temp2 = tempj;//保存第二个间隔位置
	////	 break;//一共就只有两个间隔标识
	////	}
	////}
	////	strncpy(late_train,buffer,temp1);//保存车次号
	////	for(int tempk =temp1+1;tempk<temp2;tempk++)
	////		late_time[tempk-(temp1+1)]=buffer[tempk];
	////	for(int tempm =temp2+1;tempm<tempi;tempm++)
	////	{
	////		late_sta[tempm-(temp2+1)]=buffer[tempm];
	////	}
	////	strcpy(late_sta,name[atoi(late_sta)]);

	//
	//Train *temp_sch;
	//TrainStop * temp_tstop;
	//for(temp_sch = schedule;temp_sch;temp_sch = temp_sch->next)
	//{
	//	if(!wxStrncmp(temp_sch->name,late_train,num[0]-1))//找到调度算法针对的晚点车辆
	//	{//这里将buff值赋给schedule指针以保存
	//		temp_sch->sta_waittime = atoi(late_time);
	//		temp_sch->timedep = current_time + (int)(schedule->sta_waittime*60);
	//		temp_sch->nxtstop->departure_actual = temp_sch->timedep;
	//		temp_sch->sta_waittime =0;
	//		break;
	//	}
	//}
  sock1=0;
  tcp_flag = 0;

  if(!buff)
	return false;
  else
	return true;

}
//
//bool  TimeWait(wxSocketBase *sock1)  //tcpip by LRG  20211017
//{
// char buff[1];
// char buffer[50];//保存算法端发送的车次号及停站时间
// char buffer1[50];
//  
//	int tempi = 0;
//	int tempj=0;
//	int temp1=0;
//	int temp2=0;
//char late_train[10];
//char late_time[10];
//char late_sta[10];
//int num[10];//假设传输信息个数不多于11个，共10个空格
//int temp1_num= 0;
//int temp2_num=0;
//    memset(buff,0,sizeof(char));
//
//   while(true)
//   {
//	temp1_num++;//
//	sock1->Read(buff, sizeof(buff));
//	if(!wxStrncmp(buff, wxT(" "), 1))//
//		num[temp2_num++]=temp1_num;//
//	if(!wxStrncmp(buff, wxT("@"), 1))//规定@视为传输结束
//	{
//		num[temp2_num++]=temp1_num;//
//			break;
//	}
//
//	buffer[tempi] = buff[0];
//	buffer1[tempi] = buff[0];
//	tempi=tempi+1;
//	memset(buff,0,sizeof(char));
//	
//   }
//	
//	strncpy(late_train,buffer,num[0]-1);//保存车次号
//	for(int tempk =num[0];tempk<num[1]-1;tempk++)
//		late_time[tempk-num[0]]=buffer[tempk];
//	for(int tempm =num[1];tempm<tempi;tempm++)
//	{
//		late_sta[tempm-num[1]]=buffer[tempm];
//	}
//	strcpy(late_sta,name[atoi(late_sta)]);
//
//
//	//for (tempj=0;tempj<50;tempj++)
//	//{
//	//	if(buffer[tempj]=='&')
//	//		temp1 = tempj;//保存第一个间隔位置
//	//	if(buffer[tempj]=='$')
//	//	{temp2 = tempj;//保存第二个间隔位置
//	//	 break;//一共就只有两个间隔标识
//	//	}
//	//}
//	//	strncpy(late_train,buffer,temp1);//保存车次号
//	//	for(int tempk =temp1+1;tempk<temp2;tempk++)
//	//		late_time[tempk-(temp1+1)]=buffer[tempk];
//	//	for(int tempm =temp2+1;tempm<tempi;tempm++)
//	//	{
//	//		late_sta[tempm-(temp2+1)]=buffer[tempm];
//	//	}
//	//	strcpy(late_sta,name[atoi(late_sta)]);
//
//	
//	Train *temp_sch;
//	TrainStop * temp_tstop;
//	for(temp_sch = schedule;temp_sch;temp_sch = temp_sch->next)
//	{
//		if(!wxStrncmp(temp_sch->name,late_train,num[0]-1))//找到调度算法针对的晚点车辆
//		{//这里将buff值赋给schedule指针以保存
//			temp_sch->sta_waittime = atoi(late_time);
//			temp_sch->timedep = current_time + (int)(schedule->sta_waittime*60);
//			temp_sch->nxtstop->departure_actual = temp_sch->timedep;
//			temp_sch->sta_waittime =0;
//			break;
//		}
//	}
//  sock1=0;
//  tcp_flag = 0;
//
//  if(!buff)
//	return false;
//  else
//	return true;
//
//}

void  parse_udp(byte *buff){
	int m=0;
	int h = 1,x = 0,sum = 0,i = 0;

	int flag,train,len;
	Train *t;
	byte trainCount;
	short int dataCount,trainId;
	double data[recieveLen];

	//memset(data,0,sizeof(data));

	trainCount = buff[3];
	buff = &buff[4];


	for(int i=0;i<trainCount;i++){
		double  *CurveValue;	
		trainId = (double)(buff[0]+buff[1]*256);
		dataCount = (double)(buff[2]+buff[3]*256);
		CurveValue = new double[accuracy*dataCount];
		buff += 4;

		/*for(int i =0;i<dataCount;i++){
			data[i] = (double)(buff[2*i]*256+buff[2*i+1]);
		}*/
		for(t = schedule; t; t = t->next){
			train = atoi(t->name+1);
			if(train == trainId){
				for(int j =0;j<dataCount;j++){
				CurveValue[accuracy*j] = (double)(buff[2*j]+buff[2*j+1]*256);
				CurveValue[accuracy*(j+1)] = 5;//最后一点+accuracy 设置为末速度为5(写在循环里是因为写在外面j变量已经释放了，此种写法也可保证最后一点为末速度)
				}
				t->CalculatedSpeedCurveValue = CurveValue;
				break;
			}
		}
		buff += 2*dataCount;
		
	}
	waitAto = 0;
    do_command(wxT("run"), false);
}

bool  ReceiveAto(char *buff)  //tcpip by YangPX  20200424
{
//char buff[1];
int length,flag;

remove("D:/ato.txt");
FILE *fp;
fp = fopen( "D:/ato.txt", "w+");

if (fp==NULL)
   return false;
  
  ato_flag=1;

  /*memset(buff,0,sizeof(char)*MAX_PACK_SIZE);*/
   memset(buff,0,sizeof(char));

    int i=0;
   
	while(buff[0]>0)

	{    
		if(!wxStrncmp(buff, wxT("end"), 3)){	
		break;
		}
		fwrite(buff,sizeof buff[0],sizeof(buff[0]),fp);
		buff++;
	}


  fclose(fp);

  return true;

}

