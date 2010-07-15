#ifndef __IMPORTTEXTDLG_H__
#define __IMPORTTEXTDLG_H__

#include <wx/dialog.h> // Base class: wxDialog
#include <wx/grid.h>

class wxCollapsiblePane;
class wxBoxSizer;
class wxSizerFlags;

class CImportTextDlg : public wxDialog {

  DECLARE_CLASS( CImportTextDlg )
  DECLARE_EVENT_TABLE()

public:
  CImportTextDlg(wxWindow* parent);
  virtual ~CImportTextDlg();

  void CreateControls();

protected:
  virtual wxSize DoGetBestSize() const;

private:
  wxCollapsiblePane* CreateFileFormatsPane(wxBoxSizer* sizer);
  wxCollapsiblePane* CreateParsingOptionsPane(wxBoxSizer* dlgSizer);

};

class ImportTextGrid : public wxGrid
{
public:
  ImportTextGrid(wxWindow* parent, wxWindowID id) : wxGrid(parent, id) {}
  
  DECLARE_CLASS(ImportTextGrid)
  
protected:
    virtual wxSize DoGetBestSize() const;
};


class ImportTextGridTable : public wxGridTableBase
{
  DECLARE_CLASS(ImportTextGridTable)
  
  wxArrayString m_lines;
  
public:
  /// Constructors
  ImportTextGridTable(const wxString& textfile);

  /// Destructor
  ~ImportTextGridTable(){}

  /// overrides from wxGridTableBase
  virtual int GetNumberRows() ;
  virtual int GetNumberCols();
  virtual bool IsEmptyCell(int row, int col);
  virtual wxString GetValue(int row, int col);
  //override this to suppress the row numbering in the grid
  //virtual wxString GetRowLabelValue(int row);
  virtual wxString GetColLabelValue(int col);
  virtual void SetValue(int row, int col, const wxString& value){}
  //virtual bool DeleteRows(size_t pos, size_t numRows);
  //virtual bool InsertRows(size_t pos, size_t numRows);
  //virtual bool AppendRows(size_t numRows = 1);
  
  ///optional overrides
  //virtual void Clear();
};

#endif // __IMPORTTEXTDLG_H__
