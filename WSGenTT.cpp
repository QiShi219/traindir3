/*	WSGenTT.cpp - Created by Giampiero Caprino

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

#include <wx/thread.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include "Traindir3.h"
#include "trsim.h"
#include "Train.h"
#include "Itinerary.h"
#include "Array.h"
#include "TDFile.h"
#include "Server.h"
#include "Options.h"
#include "GraphView.h"
#include "PlatformGraphView.h"
#include "SwitchBoard.h"
#include "HostLock.h"


#include <wx/sstream.h>
#include <wx/protocol/http.h>

wxString getExecutableDir()
{
        wxString rootStr = wxStandardPaths::Get().GetExecutablePath();
#if !defined(__WXGTK__) && !defined(__WXMACOS__)
        if(rootStr.EndsWith(wxT("\\traindir3.exe")))
            rootStr = rootStr.SubString(0, rootStr.Length() - 14);
#else
        if(rootStr.EndsWith(wxT("traindir3")))
            rootStr = rootStr.SubString(0, rootStr.Length() - 10);
#endif
        return rootStr;
}

class WS_Recv : public Servlet
{
public:
        WS_Recv() : Servlet("/tt/recv.yaml") { };
        ~WS_Recv() { };

        bool    get(wxString& out, Char *args);
} ws_recv;

bool    WS_Recv::get(wxString& out, Char *args)
{
        Char    *p, *d;
        Char    value[128];
        Char    dest[512];
        Char    buff[512];
        Char    hostName[512];
        wxString res;

        FromUrl(buff, args);
        p = buff;
        for(d = value; *p && *p != '@'; )
            *d++ = *p++;
        *d = 0;
        wxString genTTdir = getExecutableDir();
        wxSprintf(dest, wxT("%s/gentt/trains/%s.html"), genTTdir.c_str(), value);
	wxString destUC(dest);
	char *destName = strdup(destUC.mb_str(wxConvUTF8));
        FILE *fp = fopen(destName, "r");
        if(fp) {
	    free(destName);
            int ch;
            Char chs[4];
            while((ch = fgetc(fp)) != -1) {
                chs[0] = ch;
                chs[1] = 0;
                chs[2] = 0;
                chs[3] = 0;
                res.append(chs);
            }
            fclose(fp);
            out.append(res);
            return true;
        }
        if(*p != '@') {
            out.append(wxT("error: no train name\n"));
	    free(destName);
            return true;
        }
        ++p;
        if(!wxStrncmp(p, wxT("http://"), 7))
            p += 7;
        for(d = hostName; *p && *p != '/'; )
            *d++ = *p++;
        *d = 0;
        if(!*p) {
            out.append(wxT("error: no host name\n"));
	    free(destName);
            return true;
        }

        wxHTTP get;
        get.SetHeader(_T("User-Agent"), _T("Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.87 Safari/537.36"));
        get.SetHeader(_T("Accept"), _T("Accept: text/html,application/xhtml+xml,application/xml"));
        get.SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
        get.SetTimeout(10); // 10 seconds of timeout instead of 10 minutes ...

        // this will wait until the user connects to the internet. It is important in case of dialup (or ADSL) connections
        // fahrphan.sbb.ch
        while (!get.Connect(hostName))  // only the server, no pages here yet ...
            wxSleep(5);

        if (!wxApp::IsMainLoopRunning()) { // should return true
            out.append(wxT("Not in main loop\n"));
        }
         
        // use _T("/") for index.html, index.php, default.asp, etc.
        wxInputStream *httpStream = get.GetInputStream(p);
         
        // wxLogVerbose( wxString(_T(" GetInputStream: ")) << get.GetResponse() << _T("-") << ((resStream)? _T("OK ") : _T("FAILURE ")) << get.GetError() );
         
        if (get.GetError() == wxPROTO_NOERR) {
            wxStringOutputStream out_stream(&res);
            httpStream->Read(out_stream);
         
            //wxMessageBox(res);
            // wxLogVerbose( wxString(_T(" returned document length: ")) << res.Length() );
            out.append(out_stream.GetString());
            fp = fopen(destName, "w");
            if(fp) {
                fwrite(res.c_str(), 1, res.Length(), fp);
                fclose(fp);
            }
        } else {
            out.append(wxT("Unable to connect"));
        }
        free(destName);
        wxDELETE(httpStream);
        get.Close();
        return true;
}


class WS_GetStations : public Servlet
{
public:
        WS_GetStations() : Servlet("/tt/stations.yaml") { };
        ~WS_GetStations() { };

        bool    get(wxString& out, Char *args);
} ws_getStations;

static Char dataSubDir[] = wxT("/gentt/stations");

bool    WS_GetStations::get(wxString& out, Char *args)
{
        //dataDir.append(dataSubDir);
        wxString dirName = getExecutableDir();
        dirName.append(wxT("/gentt/stations"));
        wxDir dir(dirName);
        if(!dir.IsOpened()) {
            out.append(wxT("error: Cannot open data Dir"));
            // deal with the error here - wxDir would already log an error message
            // explaining the exact reason of the failure
            return true;
        }
        wxString filename;
        bool cont = dir.GetFirst(&filename);
        out.append(wxT("files:\n"));
        while(cont) {
            out.append(wxT("- name: "));
            out.append(filename);
            out.append(wxT("\n  url: "));
            out.append(dataSubDir).append(wxT("/")).append(filename).append(wxT("\n"));
            cont = dir.GetNext(&filename);
        }
        return true;
}

