// FileDialogExt.h
//-----------------------------------------------------------------------------

class CFileDialogExt
   : public CFileDialog
{
   DECLARE_DYNAMIC(CFileDialogExt)

public:
   CFileDialogExt(BOOL bOpenFileDialog, // TRUE=FileOpen, FALSE=FileSaveAs
                  LPCTSTR lpszDefExt = NULL,
                  LPCTSTR lpszFileName = NULL,
                  DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                  LPCTSTR lpszFilter = NULL,
                  CWnd* pParentWnd = NULL);

   virtual BOOL OnFileNameOK();

protected:
   LPCTSTR m_ext;

   DECLARE_MESSAGE_MAP()
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
