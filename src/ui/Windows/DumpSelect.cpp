/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file DumpSelect.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "DumpSelect.h"

#include <signal.h>
#include <eh.h>

// CDumpSelect dialog

IMPLEMENT_DYNAMIC(CDumpSelect, CPWDialog)

CDumpSelect::CDumpSelect(CWnd* pParent /*=NULL*/)
  : CPWDialog(CDumpSelect::IDD, pParent), m_dumptype(0)
{
}

CDumpSelect::~CDumpSelect()
{
}

void CDumpSelect::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  DDX_Radio(pDX, IDC_DUMP_WIN32, m_dumptype);
}

BEGIN_MESSAGE_MAP(CDumpSelect, CDialog)
  ON_BN_CLICKED(IDOK, OnBnClickedOK)
  ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()

/*
  For Radio buttons to work properly here, the following defines 
  must be contiguous in "resource.h" with IDC_DUMP_WIN32 first:

    IDC_DUMP_WIN32
    IDC_DUMP_TERMINATE
    IDC_DUMP_UNEXPECTED
    IDC_DUMP_INVALIDPARAMETER
    IDC_DUMP_SIGILL
    IDC_DUMP_SIGTERM
    IDC_DUMP_SIGABRT

  Fault handlers for errors during memory allocation and calls to
  pure virtual functions are not available for testing.
*/

// CDumpSelect message handlers

// The following is to stop the optimizing compiler removing
// the 'designed' error.
#pragma optimize("", off)

void CDumpSelect::OnBnClickedOK()
{
  char *p, c;

  UpdateData(TRUE);
  switch (m_dumptype) {
    case 0: // IDC_DUMP_WIN32
      // Win32FaultHandler
      //Create an error
      p = (char *)0xBADC0DE; // Ummm!!, a dirty bad thing
      // Create a access violation
      c = *p;
      break;
    case 1: // IDC_DUMP_TERMINATE
      // terminate
      terminate();
      break;
    case 2: // IDC_DUMP_UNEXPECTED
      // unexpected
      unexpected();
      break;
    case 3: // IDC_DUMP_INVALIDPARAMETER
      // C++ invalid_parameter
      p = NULL;
      printf(p);
      break;
    case 4: // IDC_DUMP_SIGILL
      // Raise signal an illegal instruction
      raise(SIGILL);
      break;
    case 5: // IDC_DUMP_SIGTERM
      // Raise signal for a program termination
      raise(SIGTERM);
      break;
    case 6: // IDC_DUMP_SIGABRT
      // Raise signal for an abnormal program termination
      raise(SIGABRT);
      break;
    default:
      break;
  }

  CPWDialog::OnOK();
}

//Put things back!
#pragma optimize("", on)

void CDumpSelect::OnBnClickedCancel()
{
  CPWDialog::OnCancel();
}
