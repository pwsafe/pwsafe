/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// YubiKey.cpp
//-----------------------------------------------------------------------------

#include <string> // for msg verification
#include "StringXStream.h"
#include "UTF8Conv.h"
#include "Util.h"
#include "sha1.h"
#include "hmac.h"
#include "os/http.h"

#include "YubiKey.h"

using namespace std;

//static const char *AuthServer = "http://api.yubico.com/wsapi/verify?";
static const char *AuthServer = "http://63.146.69.105/wsapi/verify?";
static const char *OurID ="708";

// API Key:	San67hskXHG7Ya3pi0JSw9AEqX0=
static unsigned char apiKey[20] = {0x49, 0xa9, 0xfa, 0xee, 0x1b,
                                   0x24, 0x5c, 0x71, 0xbb, 0x61,
                                   0xad, 0xe9, 0x8b, 0x42, 0x52,
                                   0xc3, 0xd0, 0x04, 0xa9, 0x7d};


stringT YubiKeyAuthenticator::SignReq(const string &msg)
{
  HMAC<SHA1> hmac(apiKey, sizeof(apiKey));
  hmac.Update(reinterpret_cast<const unsigned char *>(msg.c_str()),
              msg.length());
  unsigned char hcal[HMAC<SHA1>::HASHLEN];
  hmac.Final(hcal);
  return PWSUtil::Base64Encode(hcal, HMAC<SHA1>::HASHLEN);
}

bool YubiKeyAuthenticator::VerifySig(const string &msg, const string &h)
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

bool YubiKeyAuthenticator::VerifyOTP(const stringT &otp)
{
  m_error = _T("");
  pws_os::HttpClient client;
  m_error = _T("Sending authentication request");

  CUTF8Conv conv;
  const unsigned char *otp_str;
  int otp_strlen;
  conv.ToUTF8(otp.c_str(), otp_str, otp_strlen);
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

  if (client.Navigate(LPCTSTR(urlStr))) {
    if (client.GetStatus() == 200) { // 200 = successful HTTP transaction
      string text = client.GetBody();

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
      string line, rsp_hash, rsp_timestamp, rsp_status, rsp_info;
      while (getline(is, line)) {
        if (line.find("h=") == 0)
          rsp_hash = line.substr(2);
        else if (line.find("t=") == 0)
          rsp_timestamp = line; //.substr(2);
        else if (line.find("status=") == 0)
          rsp_status = line; //.substr(7);
        else if (line.find("info=") == 0)
          rsp_info = line; //.substr(7);
        else if (line.empty())
          continue;
        else {
          TRACE(_T("Unexpected value in response\n"));
          m_error = _T("Malformed response");
          goto done;
        }
      } // parse response

      string msg;
      if (!rsp_info.empty()) {
        msg = rsp_info; msg += "&";
      }
      msg += rsp_status; msg += "&"; msg += rsp_timestamp;

      if (!VerifySig(msg, rsp_hash)) {
        m_error = _T("Signature verification failed!");
        goto done;
      }

      m_error = _T(""); // rsp_status
      retval = (rsp_status.find("OK") != string::npos);
      if (!retval) {
        m_error = _T("Authentication failure: ");
        StringX statX; // pesky conversions...
        conv.FromUTF8(reinterpret_cast<const unsigned char *>(rsp_status.c_str()),
                rsp_status.size(), statX);
        m_error += statX.c_str();
      }
    } else { // successful transaction
      ostringstreamT os(m_error);
      os << _T("Request failed: ") << client.GetStatus();
    }
  } else { // !m_client.Navigate()
    ostringstreamT os(m_error);
    os << _T("request failed - status code: ") << client.GetStatus();
  }
 done:
  return retval;
}
