/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file yubicfg.cpp
* 
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include <iomanip>
#include <sstream>

#include <wx/timer.h>
#include "yubicfg.h"

#include "core/StringX.h"
#include "core/PWScore.h"

#include "os/rand.h"

using namespace std;

////@begin XPM images
////@end XPM images


/*!
 * YubiCfgDlg type definition
 */

IMPLEMENT_CLASS( YubiCfgDlg, wxDialog )


/*!
 * YubiCfgDlg event table definition
 */

BEGIN_EVENT_TABLE( YubiCfgDlg, wxDialog )

////@begin YubiCfgDlg event table entries
  EVT_BUTTON( ID_YK_HIDESHOW, YubiCfgDlg::OnYkHideshowClick )

  EVT_BUTTON( ID_YK_GENERATE, YubiCfgDlg::OnYkGenerateClick )

  EVT_BUTTON( ID_YK_SET, YubiCfgDlg::OnYkSetClick )

////@end YubiCfgDlg event table entries
EVT_TIMER(POLLING_TIMER_ID, YubiCfgDlg::OnPollingTimer)
END_EVENT_TABLE()


/*!
 * YubiCfgDlg constructors
 */

YubiCfgDlg::YubiCfgDlg( wxWindow* parent, PWScore &core, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_core(core)
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * YubiCfgDlg creator
 */

bool YubiCfgDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin YubiCfgDlg creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end YubiCfgDlg creation
  HideSK();
  return true;
}


/*!
 * YubiCfgDlg destructor
 */

YubiCfgDlg::~YubiCfgDlg()
{
////@begin YubiCfgDlg destruction
////@end YubiCfgDlg destruction
  delete m_pollingTimer;
}


/*!
 * Member initialisation
 */

void YubiCfgDlg::Init()
{
////@begin YubiCfgDlg member initialisation
////@end YubiCfgDlg member initialisation
  m_pollingTimer = new wxTimer(this, POLLING_TIMER_ID);
  m_yksernum = m_yksk = wxT("");
  m_isSKHidden = true;
  m_present = !IsYubiInserted(); // lie to trigger correct actions in timer even
}

static StringX BinSN2Str(const unsigned char *snstr)
{
  unsigned int sn = 0;
  sn = (snstr[0] << 24) | (snstr[1] << 16);
  sn |= (snstr[2] << 8) | snstr[3];
  wostringstream os;
  os << sn;
  return StringX(os.str().c_str());
}

static StringX BinSK2HexStr(const unsigned char *sk, int len)
{
  wostringstream os;
  os << setw(2);
  os << setfill(L'0');
  for (int i = 0; i < len; i++) {
    os << hex << setw(2) << int(sk[i]);
    if (i != len - 1)
      os << " ";
  }
  return StringX(os.str().c_str());
}

static void HexStr2BinSK(const StringX &str, unsigned char *sk, int len)
{
  wistringstream is(str.c_str());
  is >> hex;
  int i = 0;
  int b;
  while ((is >> b ) && i < len) {
    sk[i++] = (unsigned char)b;
  }
}


/*!
 * Control creation for YubiCfgDlg
 */

void YubiCfgDlg::CreateControls()
{    
////@begin YubiCfgDlg content construction
  YubiCfgDlg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, 0, 0);

  wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("YubiKey Serial Number: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemDialog1, ID_YK_SERNUM, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
  itemBoxSizer3->Add(itemTextCtrl5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer6Static = new wxStaticBox(itemDialog1, wxID_ANY, _("YubiKey Secret Key (20 Byte Hex)"));
  wxStaticBoxSizer* itemStaticBoxSizer6 = new wxStaticBoxSizer(itemStaticBoxSizer6Static, wxVERTICAL);
  itemBoxSizer2->Add(itemStaticBoxSizer6, 0, wxGROW|wxALL, 5);

  wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemDialog1, ID_YKSK, _("Please insert your YubiKey"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
  itemStaticBoxSizer6->Add(itemTextCtrl7, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer6->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton9 = new wxButton( itemDialog1, ID_YK_HIDESHOW, _("Hide"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton10 = new wxButton( itemDialog1, ID_YK_GENERATE, _("Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton11 = new wxButton( itemDialog1, ID_YK_SET, _("Set YubiKey"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer12 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer12->AddButton(itemButton13);

  wxButton* itemButton14 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer12->AddButton(itemButton14);

  itemStdDialogButtonSizer12->Realize();

  // Set validators
  itemTextCtrl5->SetValidator( wxGenericValidator(& m_yksernum) );
  itemTextCtrl7->SetValidator( wxGenericValidator(& m_yksk) );
////@end YubiCfgDlg content construction
  m_pollingTimer->Start(250); // check for Yubikey every 250ms.
}


/*!
 * Should we show tooltips?
 */

bool YubiCfgDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap YubiCfgDlg::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin YubiCfgDlg bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end YubiCfgDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon YubiCfgDlg::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin YubiCfgDlg icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end YubiCfgDlg icon retrieval
}

void YubiCfgDlg::OnPollingTimer(wxTimerEvent &evt)
{
  if (evt.GetId() == POLLING_TIMER_ID) {
    // If an operation is pending, check if it has completed

    // No HMAC operation is pending - check if one and only one key is present
    bool inserted = IsYubiInserted();
    // call relevant callback if something's changed
    if (inserted != m_present) {
      m_present = inserted;
      if (m_present)
        yubiInserted();
      else
        yubiRemoved();
    }
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_HIDESHOW
 */

void YubiCfgDlg::OnYkHideshowClick( wxCommandEvent& event )
{
  if (m_isSKHidden) {
    ShowSK();
  } else {
    HideSK();
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_GENERATE
 */

void YubiCfgDlg::OnYkGenerateClick( wxCommandEvent& event )
{
  unsigned char yubi_sk_bin[YUBI_SK_LEN];
  pws_os::GetRandomData(yubi_sk_bin, YUBI_SK_LEN);
  m_yksk = BinSK2HexStr(yubi_sk_bin, YUBI_SK_LEN).c_str();
  trashMemory(yubi_sk_bin, YUBI_SK_LEN);
  Validate(); TransferDataToWindow();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_SET
 */

void YubiCfgDlg::OnYkSetClick( wxCommandEvent& event )
{
#ifdef NOTYET
  UpdateData(TRUE);  
  StringX skStr = m_yksk;
  
  FindWindow(IDC_YUBI_API)->ShowWindow(SW_HIDE); // in case of retry
  if (!skStr.empty()) {
    unsigned char yubi_sk_bin[YUBI_SK_LEN];
    HexStr2BinSK(skStr, yubi_sk_bin, YUBI_SK_LEN);
    int rc;
    if ((rc = WriteYubiSK(yubi_sk_bin)) == YKLIB_OK) { // 1. Update SK on Yubi.
      // 2. If YubiKey update succeeds, update in core.
      m_core.SetYubiSK(yubi_sk_bin);
      // 3. Write DB ASAP!
      m_core.WriteCurFile();
      trashMemory(yubi_sk_bin, YUBI_SK_LEN);
    } else {
      const CString err = _T("Failed to update YubiKey");
      FindWindow(IDC_YUBI_API)->ShowWindow(SW_SHOW);
      FindWindow(IDC_YUBI_API)->SetLabel(err);
    }
  }
#else
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_SET in YubiCfgDlg.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_SET in YubiCfgDlg.
#endif
}

void YubiCfgDlg::ShowSK()
{
  m_isSKHidden = false;
  FindWindow(ID_YK_HIDESHOW)->SetLabel(_("Hide"));
#ifdef NOTYET
  m_ex_YubiSK.SetSecure(false);

  // Remove password character so that the password is displayed
  m_ex_YubiSK.SetPasswordChar(0);
  m_ex_YubiSK.Invalidate();
#endif
}

void YubiCfgDlg::HideSK()
{
  m_isSKHidden = true;
  FindWindow(ID_YK_HIDESHOW)->SetLabel(_("Show"));
#ifdef NOTYET
  m_ex_YubiSK.SetSecure(true);

  // Set password character so that the password is not displayed
  m_ex_YubiSK.SetPasswordChar(PSSWDCHAR);
  m_ex_YubiSK.Invalidate();
#endif
}

void YubiCfgDlg::ReadYubiSN()
{
#ifdef NOTYET
  CYkLib yk;
  BYTE buffer[128];
  YKLIB_RC rc;
  STATUS status;
  CSingleLock singeLock(&m_mutex);

  memset(&status, 0, sizeof(status));
  singeLock.Lock();
  rc = yk.openKey();
  if (rc != YKLIB_OK) goto fail;
  rc = yk.readSerialBegin();
  if (rc != YKLIB_OK) goto fail;
  // Wait for response completion
  rc = yk.waitForCompletion(YKLIB_MAX_SERIAL_WAIT, buffer, sizeof(DWORD));
  if (rc != YKLIB_OK) goto fail;
  m_yksernem = BinSN2Str(buffer).c_str();
  rc = yk.closeKey();
  return; // good return
 fail:
#endif
  m_yksernum = _("Error reading YubiKey");
}

int YubiCfgDlg::WriteYubiSK(const unsigned char *yubi_sk_bin)
{
#ifdef NOTYET
  CYkLib yk;
  YKLIB_RC rc;
  STATUS status;
  CONFIG config;
  CSingleLock singeLock(&m_mutex);

  memset(&status, 0, sizeof(status));
  memset(&config, 0, sizeof(config));
  config.tktFlags = TKTFLAG_CHAL_RESP;
  config.cfgFlags = CFGFLAG_CHAL_HMAC | CFGFLAG_HMAC_LT64 | CFGFLAG_CHAL_BTN_TRIG;
  config.extFlags = EXTFLAG_SERIAL_API_VISIBLE;
  yk.setKey160(&config, yubi_sk_bin);
  singeLock.Lock();
  rc = yk.openKey();
  if (rc != YKLIB_OK) goto fail;
  rc = yk.writeConfigBegin(1, &config, NULL);
  if (rc != YKLIB_OK) goto fail;
  // Wait for response completion
  rc = yk.waitForCompletion(YKLIB_MAX_WRITE_WAIT);
  if (rc != YKLIB_OK) goto fail;
 fail:
  return rc;
#else
  return 0;
#endif
}

void YubiCfgDlg::yubiInserted(void)
{
  FindWindow(ID_YK_SERNUM)->Enable(true);
  FindWindow(ID_YKSK)->Enable(true);
  FindWindow(ID_YK_GENERATE)->Enable(true);
  FindWindow(ID_YK_SET)->Enable(true);
  if (m_core.GetYubiSK() != NULL) {
    HideSK();
    m_yksk = BinSK2HexStr(m_core.GetYubiSK(), YUBI_SK_LEN).c_str();
  } else 
    m_yksk = _("");
  ReadYubiSN();
  FindWindow(ID_YK_SERNUM)->SetLabel(m_yksernum);
  Validate(); TransferDataToWindow();
}

void YubiCfgDlg::yubiRemoved(void)
{
  m_yksernum = _("");
  m_yksk = _("Please insert your YubiKey");
  ShowSK();
  Validate(); TransferDataToWindow();
  FindWindow(ID_YK_SERNUM)->Enable(false);
  FindWindow(ID_YKSK)->Enable(false);
  FindWindow(ID_YK_GENERATE)->Enable(false);
  FindWindow(ID_YK_SET)->Enable(false);
}

bool YubiCfgDlg::IsYubiInserted() const
{
#ifdef NOTYET
  CSingleLock singeLock(&m_mutex);
  CYkLib yk;
  singeLock.Lock();
  return (yk.enumPorts() == 1);
#else
  return false;
#endif
}

