#include <wx/sstream.h>
#include <wx/protocol/http.h>
#include "Traindir3.h"
#include "Options.h"

StringOption    user_name(wxT("userName"),
                          wxT("Name of the player"),
                          wxT("Server"),
                          wxT(""));

extern Traindir	*traindir;
extern  const wxChar	*fileName(const wxChar *p);

static
const Char	*locate(const wxChar *p, const wxChar *pattern)
{
	int l = wxStrlen(pattern);

	while(*p && wxStrncmp(p, pattern, l))
	    ++p;
	if(*p)
	    return p;
	return 0;
}

int	get_delay(Train *t)
{
	int	delay = 0;
#ifdef WIN32
	wxHTTP	get;

	if(!use_real_time)
	    return 0;

	get.SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
	get.SetTimeout(10); // 10 seconds of timeout instead of 10 minutes ...
 
	// this will wait until the user connects to the internet. It is important in case of dialup (or ADSL) connections
	while (!get.Connect(_T("mobile.viaggiatreno.it")))  // only the server, no pages here yet ...
	    wxSleep(5);
 
	traindir->IsMainLoopRunning(); // should return true
 
	wxChar buff[256];
	int i, j = 0;

	for(i = 0; t->name[i]; ++i) {
	    if(!wxIsdigit(t->name[i]))
		continue;
	    buff[j++] = t->name[i];
	}
	buff[j] = 0;

	wxChar	url[256];

	wxSnprintf(url, sizeof(url), wxT("/viaggiatreno/mobile/scheda?numeroTreno=%s&tipoRicerca=numero"), buff);
	// use _T("/") for index.html, index.php, default.asp, etc.
	wxInputStream *httpStream = get.GetInputStream(url);
 
	// wxLogVerbose( wxString(_T(" GetInputStream: ")) << get.GetResponse() << _T("-") << ((resStream)? _T("OK ") : _T("FAILURE ")) << get.GetError() );
 
	if (get.GetError() == wxPROTO_NOERR)
	{
	    wxString res;
	    wxStringOutputStream out_stream(&res);
	    httpStream->Read(out_stream);
	    //wxMessageBox(res);
 
	    const Char *p = res.c_str();
	    const Char *line = p;

	    if((p = locate(p, wxT("<!-- SITUAZIONE")))) {
		while(*p && wxStrncmp(p, wxT("minuti di ritardo"), 17)) {
		    ++p;
		    if(*p == '\n')
			line = p;
		}
		if(*p) {
		    for(p = line; *p && !wxIsdigit(*p); ++p);
		    if(wxIsdigit(*p))
			delay = wxAtoi(p);
		}
	    }

	    // wxLogVerbose( wxString(_T(" returned document length: ")) << res.Length() );
	}
	else
	{
	    wxMessageBox(_T("Unable to connect!"));
	}
 
	wxDELETE(httpStream);
	get.Close();
#endif
	return delay;
}


void    bstreet_send(const Char *url)
{
        if(user_name._sValue.Length() == 0)
            return;

	wxHTTP	get;
	get.SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
	get.SetTimeout(10); // 10 seconds of timeout instead of 10 minutes ...
 
	// this will wait until the user connects to the internet. It is important in case of dialup (or ADSL) connections
        if(!get.Connect(_T("www.backerstreet.com")))  // only the server, no pages here yet ...
	    return;
        wxInputStream *stream = get.GetInputStream(url);
        // we don't care about the result or any error
        wxDELETE(stream);
        get.Close();
}


void    bstreet_login()
{
        Char    url[256];
        wxStrcpy(url, wxT("/traindir/server/login.php?u="));
        wxStrcat(url, user_name._sValue.c_str());
        bstreet_send(url);
}

void    bstreet_logout()
{
        Char    url[256];
        wxStrcpy(url, wxT("/traindir/server/logout.php?u="));
        wxStrcat(url, user_name._sValue.c_str());
        bstreet_send(url);
}

void    bstreet_playing()
{
        Char    url[256];
        wxSnprintf(url, sizeof(url)/sizeof(Char),
            wxT("/traindir/server/nowplaying.php?u=%s&s=%s&d=%d"),
            user_name._sValue.c_str(),
            fileName(current_project.c_str()),
            run_day);
        bstreet_send(url);
}


void    bstreet_getlinks()
{
        if(user_name._sValue.Length() == 0)
            return;
	wxHTTP	get;
	get.SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
	get.SetTimeout(10); // 10 seconds of timeout instead of 10 minutes ...
 
	// this will wait until the user connects to the internet. It is important in case of dialup (or ADSL) connections
        Char    url[256];
        if(!get.Connect(_T("www.backerstreet.com")))  // only the server, no pages here yet ...
	    return;
        wxSnprintf(url, sizeof(url)/sizeof(Char),
            wxT("/traindir/server/links.php?scenario=%s"),
            fileName(current_project.c_str()));
        wxInputStream *stream = get.GetInputStream(url);
        wxString res;
        wxStringOutputStream out_stream(&res);
        stream->Read(out_stream);
        //wxMessageBox(res);

        const Char *p = res.c_str();
        // parse Path file
        wxDELETE(stream);
        get.Close();
}

extern  long    current_time;

void    prepareTrainName(Char *dest, const Char *src)
{
        while(*src) {
            if(*src == ' ') {
                *dest++ = '%';
                *dest++ = '2';
                *dest++ = '0';
            } else
                *dest++ = *src;
            ++src;
        }
        *dest = 0;
}

void    bstreet_trainexited(Train *trn)
{
        if(user_name._sValue.Length() == 0)
            return;
	long arrtime = trn->timeout;
	if(arrtime < trn->timein)
	    arrtime += 24 * 60 * 60;
	long minlate = (current_time - arrtime) / 60;
        Char    url[256];
        Char    tname[256];
        prepareTrainName(tname, trn->name);
        wxSnprintf(url, sizeof(url)/sizeof(Char),
            wxT("/traindir/server/exited.php?&s=%s&t=%s&f=%s&x=%d&d=%d&v=%d"),
            fileName(current_project.c_str()),
            tname,
            trn->exited ? trn->exited : trn->exit,  // wrong exit or correct exit
            minlate,
            run_day,
            (int)trn->curspeed);
        bstreet_send(url);
}


int     bstreet_enterdelay(Train *trn, bool *changed)
{
        int delay = 0;
        if(user_name._sValue.Length() == 0)
            return 0;

        Char    url[256];
        Char    tname[256];
        prepareTrainName(tname, trn->name);
        //http://backerstreet.com/traindir/server/entering.php?s=bartSF.trk&t=01DCM2SUN_merged_2075&f=OAK1&d=64
        wxSnprintf(url, sizeof(url)/sizeof(Char),
            wxT("/traindir/server/entering.php?&s=%s&t=%s&f=%s&d=%d"),
            fileName(current_project.c_str()),
            tname,
            trn->entrance,
            run_day);
	wxHTTP	get;
	get.SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
	get.SetTimeout(10); // 10 seconds of timeout instead of 10 minutes ...
 
	// this will wait until the user connects to the internet. It is important in case of dialup (or ADSL) connections
        if(!get.Connect(_T("www.backerstreet.com")))  // only the server, no pages here yet ...
	    return 0;
        wxInputStream *stream = get.GetInputStream(url);
        // we don't care about the result or any error
	if (get.GetError() == wxPROTO_NOERR) {
	    wxString res;
	    wxStringOutputStream out_stream(&res);
	    stream->Read(out_stream);
	    //wxMessageBox(res);
 
	    const Char *line = res.c_str();
	    const Char *p;
            if((p = wxStrchr(line, '#'))) {
                delay = wxAtoi(p + 1);
                *changed = true;
            }
        }
        wxDELETE(stream);
        get.Close();
        return delay;
}
