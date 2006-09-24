#pragma once

// CProperties dialog

#include "resource.h"

class CProperties : public CDialog
{
	DECLARE_DYNAMIC(CProperties)

public:
	CProperties(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProperties();

// Dialog Data
	enum { IDD = IDD_PROPERTIES };
	CString m_database;
	CString m_numgroups;
	CString m_numentries;
	CString m_whenlastsaved;
	CString m_wholastsaved;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOK();
	virtual BOOL OnInitDialog();
};
