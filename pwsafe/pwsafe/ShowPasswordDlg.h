// ShowPasswordDlg.h
//-----------------------------------------------------------------------------
// Currently only compiled into the Pocket PC build, but available to the
// desktop build if required.
//-----------------------------------------------------------------------------

#include "corelib/PwsPlatform.h"
#include "corelib/MyString.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
  #include "pocketpc/PwsPopupDialog.h"
  #define SUPERCLASS	CPwsPopupDialog
#else
  #include "resource.h"
  #define SUPERCLASS	CDialog
#endif

class CShowPasswordDlg : public SUPERCLASS
{
	CMyString	m_Password;
	CMyString	m_Title;
	CString		m_Message;

public:
	typedef SUPERCLASS		super;

	enum { IDD = IDD_SHOW_PASSWORD };

	CShowPasswordDlg(CWnd* pParent = NULL);   // standard constructor

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnCancel();
	afx_msg void OnOK();

public:
	void SetPassword( CMyString &pw );
	void SetTitle( CMyString &title );

	DECLARE_MESSAGE_MAP()
};

#undef SUPERCLASS

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
