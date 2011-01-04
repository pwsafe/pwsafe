/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ChangeAliasPswd.cpp : implementation file
//

#include "stdafx.h"
#include "ChangeAliasPswd.h"

// CChangeAliasPswd dialog

IMPLEMENT_DYNAMIC(CChangeAliasPswd, CPWDialog)

CChangeAliasPswd::CChangeAliasPswd(CWnd* pParent /*=NULL*/)
	: CPWDialog(CChangeAliasPswd::IDD, pParent)
{
}

CChangeAliasPswd::~CChangeAliasPswd()
{
}

void CChangeAliasPswd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

  DDX_Text(pDX, IDC_STATIC_BASE, m_BaseEntry);
}

BEGIN_MESSAGE_MAP(CChangeAliasPswd, CDialog)
  ON_BN_CLICKED(IDC_CHANGEBASEPSWD, OnChangeBasePswd)
  ON_BN_CLICKED(IDC_CHANGEALIASPSWD, OnChangeAliasPswd)
END_MESSAGE_MAP()

// CChangeAliasPswd message handlers

void CChangeAliasPswd::OnChangeBasePswd()
{
  CPWDialog::EndDialog(CHANGEBASE);
}

void CChangeAliasPswd::OnChangeAliasPswd()
{
  CPWDialog::EndDialog(CHANGEALIAS);
}
