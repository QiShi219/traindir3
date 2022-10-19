#include "CSV.h"

CSVColumn::CSVColumn(int index, Char *name)
{
        _index = index;
        _name = wxStrdup(name);
        _next = 0;
}


CSVColumn::~CSVColumn()
{
        if(_name)
            free(_name);
}


CSVFile::CSVFile(Char *path)
{
        _path = wxStrdup(path);
        _nCols = 0;
        _firstCol = _lastCol = 0;
        _file = new TDFile(path);
}


CSVFile::~CSVFile()
{
        free(_path);
        while(_firstCol) {
            CSVColumn *next = _firstCol->_next;
            delete _firstCol;
            _firstCol = next;
        }
        _firstCol = _lastCol = 0;
        _nCols = 0;
}


bool    CSVFile::Load()
{
        return _file->Load();
}

bool    CSVFile::ReadLine()
{
        // Read a line,
	if(!_file->ReadLine(_line, sizeof(_line)/sizeof(Char)))
            return false;

        // split values into colPtrs[]

        int i;
        Char *p;
        for(p = _line, i = 0; *p; ) {
            _colPtrs[i++] = p;
            while(*p && *p != wxT(','))
                ++p;
            if(!*p)
                break;
            *p++ = 0;
        }
        _colPtrs[i] = 0;
        return true;
}

bool    CSVFile::ReadColumns()
{
        if(!ReadLine())
            return false;

        int     i;
        for(i = 0; i < MAX_CSV_COL && _colPtrs[i]; ++i) {
            CSVColumn *col = new CSVColumn(i, _colPtrs[i]);
            if(!_firstCol)
                _firstCol = col;
            else
                _lastCol->_next = col;
            _lastCol = col;
        }
        return true;
}

CSVColumn       *CSVFile::FindColumn(const Char *name)
{
        CSVColumn *col;

        for(col = _firstCol; col; col = col->_next) {
            if(!wxStrcmp(col->_name, name))
                return col;
        }
        return 0;
}

void    CSVFile::GetColumn(wxString& value, CSVColumn *col)
{
        if(col && _colPtrs[col->_index])
            value = _colPtrs[col->_index];
}

void    CSVFile::GetColumn(int& value, CSVColumn *col)
{
        if(col && _colPtrs[col->_index]) {
            value = wxAtoi(_colPtrs[col->_index]);
        }
}

void    CSVFile::GetColumnHex(int& value, CSVColumn *col)
{
        if(col && _colPtrs[col->_index]) {
            wxSscanf(_colPtrs[col->_index], wxT("%x"), &value);
        }
}

void    CSVFile::GetColumn(double& value, CSVColumn *col)
{
        if(col && _colPtrs[col->_index]) {
            value = wxAtof(_colPtrs[col->_index]);
        }
}