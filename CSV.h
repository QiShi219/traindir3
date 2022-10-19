#ifndef _CSV_H
#define _CSV_H

#include "wx/filename.h"
#include "TDFile.h"
#include "trsim.h"

#define MAX_CSV_COL 40

class CSVColumn {
public:
        CSVColumn(int index, Char *name);
        ~CSVColumn();

        CSVColumn *_next;
        Char    *_name;
        int     _index;
};

class CSVFile {
public:
        CSVFile();
        CSVFile(Char *path);
        ~CSVFile();

        bool    Load();
        bool    ReadLine();
        bool    ReadColumns();
        CSVColumn *FindColumn(const Char *name);
        void    GetColumn(wxString& value, CSVColumn *col);
        void    GetColumn(int& value, CSVColumn *col);
        void    GetColumnHex(int& value, CSVColumn *col);
        void    GetColumn(double& value, CSVColumn *col);

        Char            *_path;
        CSVColumn       *_firstCol, *_lastCol;
        TDFile          *_file;
        Char            _line[512];
        Char            *_colPtrs[MAX_CSV_COL];
        int             _nCols;
};

#endif // _CSV_H
