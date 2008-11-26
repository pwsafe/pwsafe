/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/// \file YubiKeyDlg.cpp
//

#include "stdafx.h"
#include <atlhttp.h>
#include <string> // for msg verification
#include <sstream>
#include "YubiKeyDlg.h"
#include "corelib/Util.h"
#include "corelib/UTF8Conv.h"
#include "corelib/sha1.h"
#include "corelib/hmac.h"

using namespace std;

//static const char *AuthServer = "http://api.yubico.com/wsapi/verify?";
static const char *AuthServer = "http://63.146.69.105/wsapi/verify?";
static const char *OurID ="708";

// API Key:	San67hskXHG7Ya3pi0JSw9AEqX0=
static unsigned char apiKey[20] = {0x49, 0xa9, 0xfa, 0xee, 0x1b,
                                   0x24, 0x5c, 0x71, 0xbb, 0x61,
                                   0xad, 0xe9, 0x8b, 0x42, 0x52,
                                   0xc3, 0xd0, 0x04, 0xa9, 0x7d};

// YubiKeyDlg dialog

IMPLEMENT_DYNAMIC(CYubiKeyDlg, CPWDialog)

CYubiKeyDlg::CYubiKeyDlg(CWnd* pParent /*=NULL*/)
: CPWDialog(CYubiKeyDlg::IDD, pParent), m_YKpubID(_T("")),
  m_YKinfo(_T("")), m_otp(_T("")), m_YKstatus(_T(""))
{
}

CYubiKeyDlg::~CYubiKeyDlg()
{
}

BOOL CYubiKeyDlg::OnInitDialog() 
{
  if (m_YKpubID.IsEmpty()) {
    m_YKinfo = _T("To enable YubiKey support, activate your YubiKey below");
  } else {
    m_YKinfo = _T("This database is associated with YubiKey \"");
    m_YKinfo += m_YKpubID;
    m_YKinfo += _T("\"\r\nTo disable YubiKey support, click OK.\r\n");
    m_YKinfo += _T("To change the YubiKey, activate your new YubiKey below");
  }
  CPWDialog::OnInitDialog();
  return TRUE;
}

void CYubiKeyDlg::DoDataExchange(CDataExchange* pDX)
{
   CPWDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_YK_OTP, m_otp);
   DDV_MaxChars(pDX, m_otp, 44);
   DDX_Text(pDX, IDC_YK_STATUS, m_YKstatus);
   DDX_Text(pDX, IDC_YK_INFO, m_YKinfo);
}


BEGIN_MESSAGE_MAP(CYubiKeyDlg, CPWDialog)
   ON_BN_CLICKED(IDOK, &CYubiKeyDlg::OnOk)
END_MESSAGE_MAP()


// CYubiKeyDlg message handlers

void CYubiKeyDlg::OnOk()
{
  // validate OTP, blablabla
  UpdateData(TRUE); // get data from control
  if (m_otp.IsEmpty()) { // an empty string is fine,
    //                      means that we don't want to use YubiKey.
    m_YKpubID = m_otp;
    CPWDialog::OnOK();
    return;
  }
  if (VerifyOTP(m_YKstatus)) {
    m_YKpubID = m_otp.Left(12);
    CPWDialog::OnOK();
  }
  else // verify failed, show why
    UpdateData(FALSE);
}

static bool VerifySig(const string &msg, const string &h)
{
  BYTE *hv = new BYTE[2*h.length()]; // a bit too much, who cares?
  size_t hlen = 0;
  CString H(h.c_str()); // kludge to workaround char/wchar_t pain
  PWSUtil::Base64Decode(LPCTSTR(H), hv, hlen);
  if (hlen != HMAC<SHA1>::HASHLEN)
    return false;
  HMAC<SHA1> hmac(apiKey, sizeof(apiKey));
  hmac.Update(reinterpret_cast<const unsigned char *>(msg.c_str()),
              msg.length());
  unsigned char hcal[HMAC<SHA1>::HASHLEN];
  hmac.Final(hcal);
  bool retval = (std::memcmp(hcal, hv, HMAC<SHA1>::HASHLEN) == 0);
  delete[] hv;
  return retval;
}

static stringT SignReq(const string &msg)
{
  HMAC<SHA1> hmac(apiKey, sizeof(apiKey));
  hmac.Update(reinterpret_cast<const unsigned char *>(msg.c_str()),
              msg.length());
  unsigned char hcal[HMAC<SHA1>::HASHLEN];
  hmac.Final(hcal);
  return PWSUtil::Base64Encode(hcal, HMAC<SHA1>::HASHLEN);
}

bool CYubiKeyDlg::VerifyOTP(CString &error)
{
  error = _T("");
  CAtlHttpClient client;
  CAtlNavigateData navData;
  // XXX TBD - support possible proxy definitions
  // ?? How to get system defaults ??
  // m_client->SetProxy( m_proxy, m_proxy_port );
  error = _T("Sending authentication request");

  CUTF8Conv conv;
  const unsigned char *otp_str;
  int otp_strlen;
  conv.ToUTF8(LPCTSTR(m_otp), otp_str, otp_strlen);
  bool retval = false;
  string req("id="); req += OurID;
  req += "&otp=";
  req += reinterpret_cast<const char *>(otp_str);
  stringT req_sig = SignReq(req);
  req += "&h=";

  StringX Req;
  conv.FromUTF8(reinterpret_cast<const unsigned char *>(req.c_str()),
                req.size(), Req);
  Req += req_sig.c_str();
  CString urlStr(AuthServer);
  urlStr += Req.c_str();

  if (client.Navigate(urlStr, &navData)) {
    if (client.GetStatus() == 200) { // 200 = successful HTTP transaction
      int bodyLen = client.GetBodyLength();
      char *body = new char[bodyLen+1];
      memcpy( body, client.GetBody(), bodyLen );
      body[bodyLen] = 0;

      string text(body);
      delete[] body;
      /**
       * A typical response is of the form:
       *  0         1         2         
       *  012345678901234567890123456789
       * "h=fiuyV7/F7ql48I0Qt2wt5mZbrPA=
       *  t=2008-10-29T14:57:42
       *  status=OK
       *  Each line ends with \n and all but the last are fixed length
       */
      istringstream is(text);
      string line, rsp_hash, rsp_timestamp, rsp_status;
      while (getline(is, line)) {
        if (line.find("h=") == 0)
          rsp_hash = line.substr(2);
        else if (line.find("t=") == 0)
          rsp_timestamp = line; //.substr(2);
        else if (line.find("status=") == 0)
          rsp_status = line; //.substr(7);
        else if (line.empty())
          continue;
        else {
          TRACE(_T("Unexpected value in response\n"));
          error = _T("Malformed response");
          goto done;
        }
      } // parse response

      string msg;
      msg = rsp_status; msg += "&"; msg += rsp_timestamp;

      if (!VerifySig(msg, rsp_hash)) {
        error = _T("Signature verification failed!");
        goto done;
      }

      error = rsp_status.c_str();
      retval = (rsp_status.find("OK") != string::npos);
    } else {// successful transaction
      error.Format(_T("Request failed: %d"), client.GetStatus());
    }
  } else { // !m_client.Navigate()
    CString str;
    error.Format( _T("request failed - status code: %d"),
                  client.GetStatus() );
  }
 done:
  return retval;
}
