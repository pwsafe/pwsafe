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
#include "YubiKeyDlg.h"
#include "corelib/Util.h"
#include "corelib/sha1.h"
#include "corelib/hmac.h"

using namespace std;

static const CString AuthServer(_T("http://api.yubico.com/wsapi/verify?"));
static const CString OurID(_T("708"));

// YubiKeyDlg dialog

IMPLEMENT_DYNAMIC(CYubiKeyDlg, CPWDialog)

CYubiKeyDlg::CYubiKeyDlg(CWnd* pParent /*=NULL*/)
: CPWDialog(CYubiKeyDlg::IDD, pParent), m_YKpubID(_T("")),
  m_otp(_T("")), m_YKstatus(_T(""))
{
}

CYubiKeyDlg::~CYubiKeyDlg()
{
}

void CYubiKeyDlg::DoDataExchange(CDataExchange* pDX)
{
   CPWDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_YK_PUBID, m_YKpubID);
   DDV_MaxChars(pDX, m_YKpubID, 44);
   DDX_Text(pDX, IDC_YK_STATUS, m_YKstatus);
}


BEGIN_MESSAGE_MAP(CYubiKeyDlg, CPWDialog)
   ON_BN_CLICKED(IDOK, &CYubiKeyDlg::OnOk)
END_MESSAGE_MAP()


// CYubiKeyDlg message handlers

void CYubiKeyDlg::OnOk()
{
  // validate OTP, blablabla
  UpdateData(TRUE); // get data from control
  if (m_YKpubID.IsEmpty()) { // an empty string is fine,
    //                          means that we don't want to use YubiKey.
    CPWDialog::OnOK();
    return;
  }
  m_otp = m_YKpubID;
  if (VerifyOTP(m_YKstatus))
    CPWDialog::OnOK();
  else // verify failed, show why
    UpdateData(FALSE);
}

static bool VerifySig(const string &msg, const string &h)
{
  // API Key:	San67hskXHG7Ya3pi0JSw9AEqX0=
  static unsigned char apiKey[20] = {0x49, 0xa9, 0xfa, 0xee, 0x1b,
                                     0x24, 0x5c, 0x71, 0xbb, 0x61,
                                     0xad, 0xe9, 0x8b, 0x42, 0x52,
                                     0xc3, 0xd0, 0x04, 0xa9, 0x7d};
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

bool CYubiKeyDlg::VerifyOTP(CString &error)
{
  error = _T("");
  CAtlHttpClient client;
  CAtlNavigateData navData;
  // XXX TBD - support possible proxy definitions
  // ?? How to get system defaults ??
  // m_client->SetProxy( m_proxy, m_proxy_port );
  error = _T("Sending authentication request");

  bool retval = false;
  CString urlStr(AuthServer);
  urlStr += _T("id="); urlStr += OurID;
  urlStr += _T("&otp=");
  urlStr += m_otp;

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
       *  t=2008-10-29T14:57:42Z0127
       *  status=OK
       *  Each line ends with \r\n and all but the last are fixed length
       */
      string hash_str = text.substr(0, 30);
      if (hash_str[0] != 'h' ||
          hash_str[1] != '=' ||
          hash_str[29] != '=') {
        TRACE(_T("Bad or missing hash substring in response\n"));
        error = _T("Malformed response");
        goto done;
      }
      hash_str = hash_str.substr(2, 28); // throw away "h="

      string time_str = text.substr(32, 26);
      if (time_str[0] != 't' ||
          time_str[1] != '=') { // can add more checks here...
        TRACE(_T("Bad or missing time substring in response\n"));
        error = _T("Malformed response");
        goto done;
      }

      size_t status_ind = text.find("status=");
      if (status_ind == string::npos) {
        TRACE(_T("Bad or missing status substring in response\n"));
        error = _T("Malformed response");
        goto done;
      }
      string status_str = text.substr(status_ind, text.length() - status_ind);

      string msg;
      msg = status_str; msg += "&"; msg += time_str;

      if (!VerifySig(msg, hash_str)) {
        error = _T("Signature verification failed!");
        goto done;
      }

      error = status_str.c_str();
      retval = (status_str.find("OK") != string::npos);
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
