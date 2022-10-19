#ifndef _OPTION_H
#define _OPTION_H

class Option {
public:
	Option(const Char *name, int type, const Char *descr);
	~Option();

	bool	Match(const Char *in) const;
	void	Format(TString& out);
	void	Load(const Char *in);
	int	IntVal() const { return _ivalue; }
	const TString& TextVal() const { return _value; }
	const Char *GetDescr() const;

	int	    _type;	// type: 'i' = integer, 's' = string
	int	    _ivalue;	// integer value if type == 'i'
	TString	    _value;	// string value if type == 's'
	TString	    _name;	// internal name
	TString	    _descr;	// short description for menus
	TString	    _help;	// long description for help
	Option	    *_next;	// in list of options
};

extern	Option	*optionList;	// head of list of options

void	SaveOptions(FILE *fp, int prefix);  // save all options in optionList
bool	LoadOption(const Char *p);	    // check if 'p' is a valid option, and set its value

#endif // _OPTION_H