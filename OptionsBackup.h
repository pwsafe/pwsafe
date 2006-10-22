#pragma once

// OptionsBackup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsBackup dialog

class COptionsBackup : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsBackup)

// Construction
public:
	COptionsBackup();
	~COptionsBackup();

// Dialog Data
	//{{AFX_DATA(COptionsBackup)
	enum { IDD = IDD_PS_BACKUP };
	CComboBox m_backupsuffix_cbox;
	CString m_userbackupprefix;
	CString m_userbackupsubdirectory;
	CString m_userbackupotherlocation;
	BOOL	m_saveimmediately;
	BOOL	m_backupbeforesave;
	int		m_backupprefix;
	int		m_backuplocation;
	int		m_maxnumincbackups;
	//}}AFX_DATA
	int		m_backupsuffix;
	int		m_BKSFX_to_Index[PWSprefs::maxBKSFX + 1];

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsBackup)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg BOOL OnKillActive();
	// Generated message map functions
	//{{AFX_MSG(COptionsBackup)
	afx_msg void OnBackupPrefix();
	afx_msg void OnBackupDirectory();
	afx_msg void OnBackupBeforeSave();
	afx_msg void OnBrowseForLocation();
	afx_msg void OnUserPrefixKillfocus();
	afx_msg void OnComboChanged();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	BOOL PreTranslateMessage(MSG* pMsg);
	void SetExample();

private:
	CToolTipCtrl* m_ToolTipCtrl;
};

