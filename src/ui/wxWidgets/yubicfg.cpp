/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "YubiMixin.h" // for POLLING_INTERVAL
#include "./wxutils.h"

#include "core/StringX.h"
#include "core/PWScore.h"

#include "os/rand.h"
#include "os/unix/PWYubi.h"

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
  m_SKSizer = nullptr;
  m_SKCtrl = nullptr;
  m_ykstatus = nullptr;
////@end YubiCfgDlg member initialisation
  m_pollingTimer = new wxTimer(this, POLLING_TIMER_ID);
  m_present = !IsYubiInserted(); // lie to trigger correct actions in timer even
  m_yksernum = m_yksk = wxEmptyString;
  m_isSKHidden = true;
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
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("YubiKey Serial Number: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemDialog1, ID_YK_SERNUM, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
  itemBoxSizer3->Add(itemTextCtrl5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer6Static = new wxStaticBox(itemDialog1, wxID_ANY, _("YubiKey Secret Key (20 Byte Hex)"));
  m_SKSizer = new wxStaticBoxSizer(itemStaticBoxSizer6Static, wxVERTICAL);
  itemBoxSizer2->Add(m_SKSizer, 0, wxGROW|wxALL, 5);

  m_SKCtrl = new wxTextCtrl( itemDialog1, ID_YKSK, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
  m_SKSizer->Add(m_SKCtrl, 0, wxGROW|wxALL, 5);

  m_ykstatus = new wxStaticText( itemDialog1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_ykstatus->SetForegroundColour(wxColour(165, 42, 42));
  m_SKSizer->Add(m_ykstatus, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
  m_SKSizer->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton10 = new wxButton( itemDialog1, ID_YK_HIDESHOW, _("Hide"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer9->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton11 = new wxButton( itemDialog1, ID_YK_GENERATE, _("Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer9->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton12 = new wxButton( itemDialog1, ID_YK_SET, _("Set YubiKey"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer9->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer13 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* itemButton14 = new wxButton( itemDialog1, wxID_OK, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer13->AddButton(itemButton14);

  wxButton* itemButton15 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer13->AddButton(itemButton15);

  itemStdDialogButtonSizer13->Realize();

  // Set validators
  itemTextCtrl5->SetValidator( wxGenericValidator(& m_yksernum) );
  m_SKCtrl->SetValidator( wxGenericValidator(& m_yksk) );
////@end YubiCfgDlg content construction
  m_pollingTimer->Start(CYubiMixin::POLLING_INTERVAL);
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

wxBitmap YubiCfgDlg::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin YubiCfgDlg bitmap retrieval
  return wxNullBitmap;
////@end YubiCfgDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon YubiCfgDlg::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin YubiCfgDlg icon retrieval
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

void YubiCfgDlg::OnYkHideshowClick( wxCommandEvent& WXUNUSED(event) )
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

void YubiCfgDlg::OnYkGenerateClick( wxCommandEvent& WXUNUSED(event) )
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

void YubiCfgDlg::OnYkSetClick( wxCommandEvent& WXUNUSED(event) )
{
  Validate(); TransferDataFromWindow();
  m_ykstatus->SetLabel(wxEmptyString);
  StringX skStr(m_yksk.c_str());
  if (!skStr.empty()) {
    unsigned char yubi_sk_bin[YUBI_SK_LEN];
    HexStr2BinSK(skStr, yubi_sk_bin, YUBI_SK_LEN);
    PWYubi yk;
    if (yk.WriteSK(yubi_sk_bin, YUBI_SK_LEN)) { // 1. Update SK on Yubi.
      // 2. If YubiKey update succeeds, update in core.
      m_core.SetYubiSK(yubi_sk_bin);
      // 3. Write DB ASAP!
      m_core.WriteCurFile();
      trashMemory(yubi_sk_bin, YUBI_SK_LEN);
    } else {
      m_ykstatus->SetLabel(wxT("Failed to update YubiKey"));
    }
  }
}

void YubiCfgDlg::ShowSK()
{
  if (Validate() && TransferDataFromWindow()) {
    m_isSKHidden = false;
    FindWindow(ID_YK_HIDESHOW)->SetLabel(_("Hide"));
    ShowHideText(m_SKCtrl, m_yksk, m_SKSizer, true);
  }
}

void YubiCfgDlg::HideSK()
{
  if (Validate() && TransferDataFromWindow()) {
    m_isSKHidden = true;
    FindWindow(ID_YK_HIDESHOW)->SetLabel(_("Show"));
    ShowHideText(m_SKCtrl, m_yksk, m_SKSizer, false);
  }
}

void YubiCfgDlg::ReadYubiSN()
{
  PWYubi yk;
  unsigned int serial;
  if (!yk.GetSerial(serial)) {
    m_yksernum = wxEmptyString;
    m_ykstatus->SetLabel(yk.GetErrStr().c_str());
  } else {
    m_yksernum.Printf(wxT("%u"), serial);
    m_ykstatus->SetLabel(wxEmptyString);
  }
}

void YubiCfgDlg::yubiInserted(void)
{
  m_ykstatus->SetLabel(wxEmptyString);
  FindWindow(ID_YK_SERNUM)->Enable(true);
  FindWindow(ID_YKSK)->Enable(true);
  FindWindow(ID_YK_GENERATE)->Enable(true);
  FindWindow(ID_YK_SET)->Enable(true);
  if (m_core.GetYubiSK() != nullptr) {
    HideSK();
    m_yksk = BinSK2HexStr(m_core.GetYubiSK(), YUBI_SK_LEN).c_str();
  } else
    m_yksk = wxEmptyString;
  ReadYubiSN();
  FindWindow(ID_YK_SERNUM)->SetLabel(m_yksernum);
  Validate(); TransferDataToWindow();
}

void YubiCfgDlg::yubiRemoved(void)
{
  m_ykstatus->SetLabel(_("Please insert your YubiKey"));
  m_yksernum = m_yksk = wxEmptyString;
  ShowSK();
  FindWindow(ID_YK_SERNUM)->Enable(false);
  FindWindow(ID_YKSK)->Enable(false);
  FindWindow(ID_YK_GENERATE)->Enable(false);
  FindWindow(ID_YK_SET)->Enable(false);
}

bool YubiCfgDlg::IsYubiInserted() const
{
  const PWYubi yubi;
  return yubi.IsYubiInserted();
}
