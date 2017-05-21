/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// FieldChangesDlg.h : header file
//

#pragma once

#include "PWDialog.h"

#include "SyncScrollRichEdit.h"
#include "ControlExtns.h"
#include "SecString.h"

#include "core\StringX.h"

class CItemData;

class CFieldchangesDlg : public CPWDialog
{
public:
  CFieldchangesDlg(CWnd *pParent, CItemData *pci, 
    const StringX &original_text, const StringX &new_text, const bool bIsNotes);
  virtual ~CFieldchangesDlg();

  // Dialog Data
  enum { IDD = IDD_FINDREPLACECHANGES };

protected:
  CEditExtn m_ex_group, m_ex_title, m_ex_username;

  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support;

  DECLARE_MESSAGE_MAP()

private:
  CSyncScrollRichEdit m_SSRE_Original_Text, m_SSRE_New_Text;

  StringX m_sxOriginal_Text, m_sxNew_Text;
  CItemData *m_pci;
  bool m_bIsNotes;
};

