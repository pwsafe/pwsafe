/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AttProperties.cpp : implementation file
//

#include "stdafx.h"
#include "AttProperties.h"

// CAttProperties dialog

IMPLEMENT_DYNAMIC(CAttProperties, CPWDialog)

CAttProperties::CAttProperties(const st_AttProp &st_prop, CWnd *pParent /*=NULL*/)
: CPWDialog(CAttProperties::IDD, pParent), m_pcfont(NULL)
{
  m_name = st_prop.name;
  m_path = st_prop.path;
  m_desc = st_prop.desc;
  m_cdate = st_prop.cdate;
  m_adate = st_prop.adate;
  m_mdate = st_prop.mdate;
  m_ddate = st_prop.ddate;
  m_usize = st_prop.usize;
  m_comp = st_prop.comp;
  m_crc = st_prop.crc;
  m_odigest = st_prop.odigest;
}

CAttProperties::~CAttProperties()
{
  if (m_pcfont != NULL) {
    m_pcfont->DeleteObject();
    delete m_pcfont;
  }
}

void CAttProperties::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAttProperties, CPWDialog)
  ON_BN_CLICKED(IDOK, &CAttProperties::OnOK)
END_MESSAGE_MAP()

// CAttProperties message handlers

void CAttProperties::OnOK()
{
  CPWDialog::OnOK();
}

BOOL CAttProperties::OnInitDialog()
{
  LOGFONT lf;
  CFont cfont;

  // Set labels Bold
  GetFont()->GetLogFont(&lf);
  lf.lfWeight = 700;
  cfont.CreateFontIndirect(&lf);

  for (UINT uID = IDC_STATIC_ATT1; uID <= IDC_STATIC_ATT11; uID++) {
    GetDlgItem(uID)->SetFont(&cfont);
  }

  // Set CRC & Digest to Courier New (non-proportional font)
  CFont *m_pcfont = new CFont;
  m_pcfont->CreatePointFont(100, L"Courier New");
  ((CStatic *)GetDlgItem(IDC_ATTACHMENTCRC))->SetFont(m_pcfont);
  ((CStatic *)GetDlgItem(IDC_ATTACHMENTDIGEST))->SetFont(m_pcfont);

  // Fill in the data
  GetDlgItem(IDC_ATTACHMENTNAME)->SetWindowText(m_name);
  GetDlgItem(IDC_ATTACHMENTPATH)->SetWindowText(m_path);
  GetDlgItem(IDC_ATTACHMENTDESC)->SetWindowText(m_desc);
  GetDlgItem(IDC_ATTACHMENTCDATE)->SetWindowText(m_cdate);
  GetDlgItem(IDC_ATTACHMENTADATE)->SetWindowText(m_adate);
  GetDlgItem(IDC_ATTACHMENTMDATE)->SetWindowText(m_mdate);
  GetDlgItem(IDC_ATTACHMENTDDATE)->SetWindowText(m_ddate);
  GetDlgItem(IDC_ATTACHMENTUSIZE)->SetWindowText(m_usize);
  GetDlgItem(IDC_ATTACHMENTCOMP)->SetWindowText(m_comp);
  GetDlgItem(IDC_ATTACHMENTCRC)->SetWindowText(m_crc);
  GetDlgItem(IDC_ATTACHMENTDIGEST)->SetWindowText(m_odigest);

  return TRUE;
}
