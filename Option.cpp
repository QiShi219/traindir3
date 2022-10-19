#include "Traindir3.h"
#include "Option.h"


Option	*optionList, *optionLast;

Option::Option(const Char *name, int type, const Char *descr)
: _name(name), _descr(descr)
{
	_type = type;
	_ivalue = 0;

	// link this element in global option list
	if(!optionList)
	    optionList = this;
	else
	    optionLast->_next = this;
	optionLast = this;
}


Option::~Option()
{
#if 0
	Option	*prev = 0;
	Option	*p;

	// unlink this element from global option list

	for(p = optionList; p && p != this; p = p->_next)
	    prev = p;
	if(p) {
	    if(!prev)
		optionList = _next;
	    else
		optionList = prev->_next;
	}
#endif
}


const Char *Option::GetDescr() const
{
	return LV(_descr.c_str());
}


bool	Option::Match(const Char *in) const
{
	if(!wxStrncmp(in, _name, _name.length()))
	    return true;
	return false;
}


void	Option::Format(TString& out)
{
	Char	buff[256];

	if(_type == 'i')
	    wxSprintf(buff, "%s=%d", _name.c_str(), _ivalue);
	else
	    wxSprintf(buff, "%s=%s", _name.c_str(), _value.c_str());

}


void	Option::Load(const Char *in)
{
	in += _name.length();
	while(*in && *in != wxT('='))
	    in += 1;
	if(*in)
	    in += 1;
	if(_type == 'i')
	    _ivalue = wxAtoi(in);
	else
	    _value = in;
}


//
//
//


void	SaveOptions(FILE *fp, int prefix)
{
	Option	*p;
	TString	out;

	for(p = optionList; p; p = p->_next) {
	    p->Format(out);
	    if(prefix)
		fputc(prefix, fp);
	    fprintf(fp, "%s\n", out.c_str());
	}
}


bool	LoadOption(const Char *p)
{
	Option	*opt;

	for(opt = optionList; opt; opt = opt->_next) {
	    if(opt->Match(p)) {
		opt->Load(p);
		return true;
	    }
	}
	return false;
}
